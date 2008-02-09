/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   loco.c for the Openbox window manager
   Copyright (c) 2008        Derek Foreman
   Copyright (c) 2008        Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include <stdio.h>
#include "obt/mainloop.h"
#include "obt/display.h"
#include "obt/mainloop.h"
#include "obt/prop.h"
#include <glib.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glxtokens.h>
/*
<dana> you want CreateNotify, DestroyNotify, MapNotify, UnmapNotify, and
          ConfigureNotify
          <dana> and then xdamage or whatever

*/

#define glError()                                      \
{                                                      \
    /*const GLchar *err_file = strrchr(err_path, '/');*/ \
    GLenum        gl_error = glGetError();             \
                                                       \
    /*++err_file;*/                                        \
                                                       \
    for (; (gl_error); gl_error = glGetError())        \
      fprintf(stderr, "%s: %d caught at line %u\n",    \
              __FUNCTION__, gl_error, __LINE__);       \
              /*(const GLchar*)gluErrorString(gl_error)*/ \
}


#define MAX_DEPTH 32
#define REFRESH_RATE (G_USEC_PER_SEC/60)

typedef struct {
    Window id;
    gint x, y, w, h;
    gint depth;
    gboolean input_only;
    gboolean visible;
    gint type; /* XXX make this an enum */
    GLuint texname;
    Damage damage;
    GLXPixmap glpixmap;
    gboolean damaged;
} LocoWindow;

typedef struct _LocoList {
    struct _LocoList *next;
    struct _LocoList *prev;
    LocoWindow *window;
} LocoList;

static Window      loco_root;
static Window      loco_overlay;
/* Maps X Window ID -> LocoWindow* */
static GHashTable *window_map;
/* Maps X Window ID -> LocoList* which is in the stacking_top/bottom list */
static GHashTable *stacking_map;
/* From top to bottom */
static LocoList   *stacking_top;
static LocoList   *stacking_bottom;
static VisualID visualIDs[MAX_DEPTH + 1];
static XVisualInfo *glxPixmapVisuals[MAX_DEPTH + 1];
static int loco_screen_num;
static gboolean full_redraw_required;
static gboolean redraw_required;

typedef void (*BindEXTFunc)(Display *, GLXDrawable, int, const int *);
typedef void (*ReleaseEXTFunc)(Display *, GLXDrawable, int);

BindEXTFunc    BindTexImageEXT;
ReleaseEXTFunc ReleaseTexImageEXT;

static void show_window(LocoWindow *lw);
static void remove_window(LocoWindow *lw);

/*
static void print_stacking()
{
    LocoList *it;

    for (it = stacking_top; it; it = it->next) {
        printf("0x%lx\n", it->window->id);
    }
}
*/

static glong timecompare(GTimeVal *a, GTimeVal *b)
{
    glong r;
    if ((r = a->tv_sec - b->tv_sec)) return r;
    return a->tv_usec - b->tv_usec;
}

static void timeadd(GTimeVal *t, glong microseconds)
{
    t->tv_usec += microseconds;
    while (t->tv_usec >= G_USEC_PER_SEC) {
        t->tv_usec -= G_USEC_PER_SEC;
        ++t->tv_sec;
    }
    while (t->tv_usec < 0) {
        t->tv_usec += G_USEC_PER_SEC;
        --t->tv_sec;
    }
}

int create_glxpixmap(LocoWindow *lw)
{
    XVisualInfo  *visinfo;
    Pixmap pixmap;
    static const int attrs[] =
        {
            GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
            None
        };

    static const int drawable_tfp_attrs[] =
        {
            GLX_CONFIG_CAVEAT, GLX_NONE,
            GLX_DOUBLEBUFFER, False,
            GLX_DEPTH_SIZE, 0,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            GLX_ALPHA_SIZE, 1,
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_BIND_TO_TEXTURE_RGBA_EXT, True, // additional for tfp
            None
        };
    int count;

    pixmap = XCompositeNameWindowPixmap(obt_display, lw->id);
    visinfo = glxPixmapVisuals[lw->depth];
    if (!visinfo) {
        printf("no visinfo for depth %d\n", lw->depth);
	return 0;
    }

    GLXFBConfig fbc;
    GLXFBConfig *fbconfigs = glXChooseFBConfig(obt_display,
                                               loco_screen_num,
                                               drawable_tfp_attrs, &count);

    int i;
    fbc = 0;
    for(i = 0; i < count; ++i ) {
        int value;
        glXGetFBConfigAttrib(obt_display,
                             fbconfigs[i], GLX_VISUAL_ID, &value);
        if(value == (int)visinfo->visualid)
        {
            fbc = fbconfigs[i];
            XFree(fbconfigs);
        }
    }

    if (!fbc) {
        printf("fbconfigless\n");
        return 0;
    }

    lw->glpixmap = glXCreatePixmap(obt_display, fbc, pixmap, attrs);
    return !!lw->glpixmap;
}

int bindPixmapToTexture(LocoWindow *lw)
{
    if (lw->glpixmap != None)
        return 1; /* already bound */

    if (!create_glxpixmap(lw))
        return 0; /* horrible failure */

/*
    if (screen->queryDrawable (screen->display->display,
			       texture->pixmap,
			       GLX_TEXTURE_TARGET_EXT,
			       &target))
    {
	fprintf (stderr, "%s: glXQueryDrawable failed\n", programName);

	glXDestroyGLXPixmap (screen->display->display, texture->pixmap);
	texture->pixmap = None;

	return FALSE;
    }
*/

    glBindTexture(GL_TEXTURE_2D, lw->texname);
glError();
    BindTexImageEXT(obt_display, lw->glpixmap, GLX_FRONT_LEFT_EXT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    return 1;
}

void releasePixmapFromTexture(LocoWindow *lw)
{
    if (lw->glpixmap) {
        glBindTexture(GL_TEXTURE_2D, lw->texname);

        ReleaseTexImageEXT(obt_display, lw->glpixmap, GLX_FRONT_LEFT_EXT);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void destroy_glxpixmap(LocoWindow *lw)
{
    if (lw->glpixmap) {
		obt_display_ignore_errors(TRUE);
        releasePixmapFromTexture(lw);
        glXDestroyGLXPixmap(obt_display, lw->glpixmap);
        obt_display_ignore_errors(FALSE);

        lw->glpixmap = None;
    }
}

static void full_composite(void)
{
    int ret;
    LocoList *it;

    /* XXX if (full_redraw_required) */
        glClear(GL_COLOR_BUFFER_BIT);

    for (it = stacking_bottom; it != stacking_top; it = it->prev) {
        if (it->window->input_only) continue;
        if (!it->window->visible)   continue;
        /* XXX if (!full_redraw_required && !it->window->damaged) continue; */

        if (!full_redraw_required) {
            /* XXX if the window is transparent, then clear the background
               behind it */
        }

        glBindTexture(GL_TEXTURE_2D, it->window->texname);
        ret = bindPixmapToTexture(it->window);
        glBegin(GL_QUADS);
        glColor3f(1.0, 1.0, 1.0);
        glVertex2i(it->window->x, it->window->y);
        glTexCoord2f(1, 0);
        glVertex2i(it->window->x + it->window->w, it->window->y);
        glTexCoord2f(1, 1);
        glVertex2i(it->window->x + it->window->w,
                   it->window->y + it->window->h);
        glTexCoord2f(0, 1);
        glVertex2i(it->window->x, it->window->y + it->window->h);
        glTexCoord2f(0, 0);
        glEnd();

        it->window->damaged = FALSE;
    }
    glXSwapBuffers(obt_display, loco_overlay);

    full_redraw_required = redraw_required = FALSE;
}

static LocoList* loco_list_prepend(LocoList **top, LocoList **bottom,
                                   LocoWindow *window)
{
    LocoList *n = g_new(LocoList, 1);
    n->window = window;

    n->prev = NULL;
    n->next = *top;
    if (n->next) n->next->prev = n;

    *top = n;
    if (!*bottom) *bottom = n;
    return n;
}

static void loco_list_delete_link(LocoList **top, LocoList **bottom,
                                  LocoList *pos)
{
    LocoList *prev = pos->prev;
    LocoList *next = pos->next;

    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;
    if (!next)
        *bottom = prev;
    if (!prev)
        *top = next;

    g_free(pos);
}

static void loco_list_move_before(LocoList **top, LocoList **bottom,
                                  LocoList *move, LocoList *before)
{
    LocoList *prev, *next;

    /* these won't move it anywhere */
    if (move == before || move->next == before) return;

    prev = move->prev;
    next = move->next;

    /* remove it from the list */
    if (next) next->prev = prev;
    else      *bottom = prev;
    if (prev) prev->next = next;
    else      *top = next;

    /* reinsert it */
    if (before) {
        move->next = before;
        move->prev = before->prev;
        move->next->prev = move;
        if (move->prev) move->prev->next = move;
    }
    else {
        /* after the bottom */
        move->prev = *bottom;
        move->next = NULL;
        if (move->prev) move->prev->next = move;
        *bottom = move;
    }

    if (!move->prev) *top = move;
}

/* Returns a LocoWindow structure */
static LocoWindow* find_window(Window window)
{
    return g_hash_table_lookup(window_map, &window);
}

/* Returns a node from the stacking_top/bottom list */
static LocoList* find_stacking(Window window)
{
    return g_hash_table_lookup(stacking_map, &window);
}

static void add_window(Window window)
{
    LocoWindow *lw;
    LocoList *it;
    XWindowAttributes attrib;

    printf("add window 0x%lx\n", window);

    if (!XGetWindowAttributes(obt_display, window, &attrib))
        return;

    lw = g_new0(LocoWindow, 1);
    lw->id = window;
    lw->input_only = attrib.class == InputOnly;
    lw->x = attrib.x;
    lw->y = attrib.y;
    lw->w = attrib.width;
    lw->h = attrib.height;
    lw->depth = attrib.depth;

    g_hash_table_insert(window_map, &lw->id, lw);
    /* new windows are at the top */
    it = loco_list_prepend(&stacking_top, &stacking_bottom, lw);
    g_hash_table_insert(stacking_map, &lw->id, it);

    if (!lw->input_only) {
        glGenTextures(1, &lw->texname);
        //  glTexImage2D(TARGET, 0, GL_RGB, lw->w, lw->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
        lw->damage = XDamageCreate(obt_display, window, XDamageReportNonEmpty);
    }

    //print_stacking();

    if (attrib.map_state != IsUnmapped)
        show_window(lw);
}

static void remove_window(LocoWindow *lw)
{
    printf("remove window 0x%lx\n", lw->id);

    LocoList *pos = find_stacking(lw->id);
    g_assert(pos);

    if (!lw->input_only) {
        glDeleteTextures(1, &lw->texname);
		obt_display_ignore_errors(TRUE);
		XDamageDestroy(obt_display, lw->damage);
        obt_display_ignore_errors(FALSE);
    }

    loco_list_delete_link(&stacking_top, &stacking_bottom, pos);
    g_hash_table_remove(stacking_map, &lw->id);
    g_hash_table_remove(window_map, &lw->id);

    g_free(lw);

    //print_stacking();
}

static void show_window(LocoWindow *lw)
{
    guint32 *type;
    guint i, ntype;

    lw->visible = TRUE;

    /* get the window's semantic type (for different effects!) */
    lw->type = 0; /* XXX set this to the default type */
    if (OBT_PROP_GETA32(lw->id, NET_WM_WINDOW_TYPE, ATOM, &type, &ntype)) {
        /* use the first value that we know about in the array */
        for (i = 0; i < ntype; ++i) {
            /* XXX SET THESE TO AN ENUM'S VALUES */
            if (type[i] == OBT_PROP_ATOM(NET_WM_WINDOW_TYPE_DESKTOP))
                lw->type = 1;
            if (type[i] == OBT_PROP_ATOM(NET_WM_WINDOW_TYPE_MENU))
                lw->type = 2;
            /* XXX there are more TYPES that need to be added to prop.h */
        }
        g_free(type);
    }

    full_redraw_required = redraw_required = TRUE;
}

static void hide_window(LocoWindow *lw, gboolean destroyed)
{
    /* if destroyed, then the window is no longer available */
    lw->visible = FALSE;
    destroy_glxpixmap(lw);

    full_redraw_required = redraw_required = TRUE;
}

static void configure_window(LocoWindow *lw, const XConfigureEvent *e)
{
    LocoList *above, *pos;

    pos = find_stacking(e->window);
    above = find_stacking(e->above);

    g_assert(pos != NULL && pos->window != NULL);
    if (e->above && !above)
        printf("missing windows from the stacking list!!\n");

    if ((lw->x != e->x) || (lw->y != e->y)) {
        lw->x = e->x;
        lw->y = e->y;

        full_redraw_required = redraw_required = TRUE;
    }
        
    if ((lw->w != e->width) || (lw->h != e->height)) {
        lw->w = e->width;
        lw->h = e->height;

        destroy_glxpixmap(lw);

        full_redraw_required = redraw_required = TRUE;
    }

    if (pos->next != above) {
        printf("Window 0x%lx above 0x%lx\n", pos->window->id,
               above ? above->window->id : 0);
        loco_list_move_before(&stacking_top, &stacking_bottom, pos, above);

        full_redraw_required = redraw_required = TRUE;
    }
}

static void damage_window(LocoWindow *lw)
{
    LocoList *it, *pos;

    pos = find_stacking(lw->id);

    /* XXX if it is transparent, then damage any windows below it that
       intersect */

    /* damage any windows above it that intersect */
    for (it = pos; it; it = it->prev) {
        const LocoWindow *lwa = it->window;
        if (lwa->x < lw->x + lw->w && lwa->x + lwa->w > lw->x &&
            lwa->y < lw->y + lw->h && lwa->y + lwa->h > lw->y)
        {
            it->window->damaged = TRUE;
        }
    }

    lw->damaged = TRUE;
    redraw_required = TRUE;
}

void COMPOSTER_RAWR(const XEvent *e, gpointer data)
{
    //g_print("COMPOSTER_RAWR() %d\n", e->type);

    if (e->type == ConfigureNotify) {
        LocoWindow *lw;
        printf("Window 0x%lx moved or something\n", e->xconfigure.window);

        lw = find_window(e->xconfigure.window);
        if (lw)
            configure_window(lw, &e->xconfigure);
        //print_stacking();
    }
    else if (e->type == CreateNotify) {
        add_window(e->xmap.window);
    }
    else if (e->type == DestroyNotify) {
        LocoWindow *lw = find_window(e->xdestroywindow.window);
        if (lw) {
            hide_window(lw, TRUE);
            remove_window(lw);
        }
        else
            printf("destroy notify for unknown window 0x%lx\n",
                   e->xdestroywindow.window);
    }
    else if (e->type == ReparentNotify) {
        if (e->xreparent.parent == loco_root)
            add_window(e->xreparent.window);
        else {
            LocoWindow *lw = find_window(e->xreparent.window);
            if (lw) {
                printf("window 0x%lx reparented from root\n", lw->id);
                hide_window(lw, FALSE);
                remove_window(lw);
            }
            else
                printf("reparent notify away from root for unknown window "
                       "0x%lx\n", e->xreparent.window);
        }
    }

    else if (e->type == MapNotify) {
        LocoWindow *lw = find_window(e->xmap.window);
        if (lw)
            show_window(lw);
        else
            printf("map notify for unknown window 0x%lx\n",
                   e->xmap.window);
    }
    else if (e->type == UnmapNotify) {
        LocoWindow *lw = find_window(e->xunmap.window);
        if (lw)
            hide_window(lw, FALSE);
        else
            printf("unmap notify for unknown window 0x%lx\n",
                   e->xunmap.window);
    }
    else if (e->type == obt_display_extension_damage_basep + XDamageNotify) {
        const XDamageNotifyEvent *de;
        LocoWindow *lw;

        de = (const XDamageNotifyEvent*)e;
        lw = find_window(de->drawable);
        if (lw) {
            damage_window(lw);
            /* mark the damage as fixed - we know about it now */
            XDamageSubtract(obt_display, lw->damage, None, None);
        }
        else if (de->drawable == loco_root) {
            XDamageSubtract(obt_display, de->damage, None, None);
            full_redraw_required = redraw_required = TRUE;
        }
    }
}

static void find_all_windows(gint screen)
{
    guint i, nchild;
    Window w, *children;

    if (!XQueryTree(obt_display, loco_root, &w, &w, &children, &nchild))
        nchild = 0;

    for (i = 0; i < nchild; ++i)
        if (children[i] != None) add_window(children[i]);

    if (children) XFree(children);
}

static gboolean compositor_timeout(gpointer data)
{
    if (redraw_required)
        full_composite();
    return TRUE; /* repeat */
}

static guint window_hash(Window *w) { return *w; }
static gboolean window_comp(Window *w1, Window *w2) { return *w1 == *w2; }

void loco_set_mainloop(gint screen_num, ObtMainLoop *loop)
{
    int db, stencil, depth;
    int i, j, value, count;
    int w, h;
    XVisualInfo *vi, tvis, *visinfo;
    GLXContext cont;
    int config[] =
        { GLX_DEPTH_SIZE, 1, GLX_DOUBLEBUFFER, GLX_RGBA, None };

    loco_screen_num = screen_num;
    loco_root = obt_root(screen_num);
    loco_overlay = XCompositeGetOverlayWindow(obt_display, loco_root);
XserverRegion region = XFixesCreateRegion(obt_display, NULL, 0);

    XFixesSetWindowShapeRegion (obt_display,
                                loco_overlay,
                                ShapeBounding,
                                0, 0,
                                0);
    XFixesSetWindowShapeRegion (obt_display,
                                loco_overlay,
                                ShapeInput,
                                0, 0,
                                region);

    XFixesDestroyRegion (obt_display, region);

    vi = glXChooseVisual(obt_display, screen_num, config);
    cont = glXCreateContext(obt_display, vi, NULL, GL_TRUE);
    if (cont == NULL)
        printf("context creation failed\n");
    glXMakeCurrent(obt_display, loco_overlay, cont);

    BindTexImageEXT = (BindEXTFunc)
        glXGetProcAddress((const guchar*)"glXBindTexImageEXT");
    ReleaseTexImageEXT = (ReleaseEXTFunc)
        glXGetProcAddress((const guchar*)"glXReleaseTexImageEXT");

    w = WidthOfScreen(ScreenOfDisplay(obt_display, screen_num));
    h = HeightOfScreen(ScreenOfDisplay(obt_display, screen_num));
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
printf("Setting up an orthographic projection of %dx%d\n", w, h);
    glOrtho(0, w, h, 0.0, -1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
glError();
    glXSwapBuffers(obt_display, loco_overlay);
    glClearColor(1.0, 0.0, 0.0, 1.0);
//    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
//    glEnable(GL_BLEND);
    db = 32767;
    stencil = 32767;
    depth = 32767;
    for (i = 0; i <= MAX_DEPTH; i++) {
        tvis.depth = i;
        visualIDs[i] = 0;
        visinfo = XGetVisualInfo(obt_display, VisualDepthMask, &tvis, &count);
        for (j = 0; j < count; j++) {
            glXGetConfig(obt_display, &visinfo[j], GLX_USE_GL, &value);
            if (!value)
                continue;

            glXGetConfig(obt_display, &visinfo[j], GLX_DOUBLEBUFFER, &value);
            if (value > db)
                continue;
            db = value;

            glXGetConfig(obt_display, &visinfo[j], GLX_STENCIL_SIZE, &value);
            if (value > stencil)
                continue;
            stencil = value;

            glXGetConfig(obt_display, &visinfo[j], GLX_DEPTH_SIZE, &value);
            if (value > depth)
                continue;
            depth = value;

            visualIDs[i] = visinfo[j].visualid;
        }
    }

    for (i = 0 ;i <= MAX_DEPTH; i++) {
        tvis.visualid = visualIDs[i];
        glxPixmapVisuals[i] = XGetVisualInfo(obt_display, VisualIDMask, &tvis, &count);
        if (glxPixmapVisuals[i])
            printf("supporting depth %d\n", i);
    }
    obt_main_loop_x_add(loop, COMPOSTER_RAWR, NULL, NULL);
    window_map = g_hash_table_new((GHashFunc)window_hash,
                                  (GEqualFunc)window_comp);
    stacking_map = g_hash_table_new((GHashFunc)window_hash,
                                    (GEqualFunc)window_comp);
    stacking_top = stacking_bottom = NULL;

    XGrabServer(obt_display);

    XCompositeRedirectSubwindows(obt_display, loco_root,
                                 CompositeRedirectManual);
    find_all_windows(screen_num);

    XUngrabServer(obt_display);

    full_redraw_required = redraw_required = TRUE;

    obt_main_loop_timeout_add(loop, REFRESH_RATE,
                              (GSourceFunc)compositor_timeout,
                              NULL, NULL, NULL);
}

void loco_shutdown(void)
{
}
