/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   composite.c for the Openbox window manager
   Copyright (c) 2010        Dana Jansens
   Copyright (c) 2010        Derek Foreman

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

#include "composite.h"
#include "config.h"
#include "obt/display.h"
#include "openbox.h"
#include "screen.h"
#include "event.h"
#include "geom.h"
#include "client.h"
#include "window.h"
#include "frame.h"
#include "geom.h"
#include "debug.h"
#include "gettext.h"
#include "obt/prop.h"

#include <X11/Xlib.h>
#include <glib.h>

#ifdef USE_COMPOSITING
#  include <GL/glew.h>
#  include <GL/glxew.h>
#  include <GL/gl.h>
#endif
#ifdef DEBUG
#  include <sys/time.h>
#endif

Window composite_overlay = None;
Atom   composite_cm_atom = None;

#ifdef USE_COMPOSITING
#define MAX_DEPTH 32

typedef struct _ObCompositeFBConfig {
    GLXFBConfig fbc; /* the fbconfig */
    gint tf;         /* texture format */
} ObCompositeFBConfig;

/*! Turn composite redirection on for a window */
static void composite_window_redir(struct _ObWindow *w);
/*! Turn composite redirection off for a window */
static void composite_window_unredir(struct _ObWindow *w);

static GLXContext composite_ctx = NULL;
static ObCompositeFBConfig pixmap_config[MAX_DEPTH + 1]; /* depth is index */
static guint composite_idle_source = 0;
static gboolean need_redraw = FALSE;
static Window composite_support_win = None;
#ifdef DEBUG
static gboolean composite_started = FALSE;
#endif

static gboolean composite(gpointer data);
#define composite_enabled() (!!composite_idle_source)

static inline void time_fix(struct timeval *tv)
{
    while (tv->tv_usec >= 1000000) {
        tv->tv_usec -= 1000000;
        ++tv->tv_sec;
    }
    while (tv->tv_usec < 0) {
        tv->tv_usec += 1000000;
        --tv->tv_sec;
    }
}

static void get_best_fbcon(GLXFBConfig *in, int count, int depth,
                           ObCompositeFBConfig *out)
{
    GLXFBConfig best = 0;
    XVisualInfo *vi;
    int i, value, alpha, stencil, depthb;
    gboolean rgba, db;

    rgba = FALSE;
    db = TRUE;
    stencil = G_MAXSHORT;
    depthb = G_MAXSHORT;

    for (i = 0; i < count; i++) {
        vi = glXGetVisualFromFBConfig(obt_display, in[i]);
        if (vi == NULL)
            continue;

        value = vi->depth;
        XFree(vi);

        if (value != depth)
            continue;

        glXGetFBConfigAttrib(obt_display, in[i], GLX_ALPHA_SIZE, &alpha);
        glXGetFBConfigAttrib(obt_display, in[i], GLX_BUFFER_SIZE, &value);

        /* the buffer size should equal the depth or else the buffer size minus
           the alpha size should */
        if (value != depth && value - alpha != depth) continue;

        value = 0;
        if (depth == 32) {
            glXGetFBConfigAttrib(obt_display, in[i],
                                  GLX_BIND_TO_TEXTURE_RGBA_EXT, &value);
            rgba = TRUE;
        }
        if (!value) {
            if (rgba) continue; /* a different one has rgba, prefer that */

            glXGetFBConfigAttrib(obt_display, in[i],
                                  GLX_BIND_TO_TEXTURE_RGB_EXT, &value);
        }
        if (!value) // neither bind to texture?  no dice
            continue;

        /* get no doublebuffer if possible */
        glXGetFBConfigAttrib(obt_display, in[i], GLX_DOUBLEBUFFER, &value);
        if (value && !db) continue;
        db = value;

        /* get the smallest stencil buffer */
        glXGetFBConfigAttrib(obt_display, in[i], GLX_STENCIL_SIZE, &value);
        if (value > stencil) continue;
        stencil = value;

        /* get the smallest depth buffer */
        glXGetFBConfigAttrib(obt_display, in[i], GLX_DEPTH_SIZE, &value);
        if (value > depthb) continue;
        depthb = value;

        best = in[i];
    }
    out->fbc = best;
    out->tf = rgba ? GLX_TEXTURE_FORMAT_RGBA_EXT : GLX_TEXTURE_FORMAT_RGB_EXT;
}

static gboolean composite_annex(void)
{
    gchar *astr;
    Window cm_owner;
    Time timestamp;
    XSetWindowAttributes attrib;

    g_assert(composite_support_win == None);

    attrib.override_redirect = TRUE;
    composite_support_win = XCreateWindow(obt_display, obt_root(ob_screen),
                                          -100, -100, 1, 1, 0,
                                          CopyFromParent, InputOnly,
                                          CopyFromParent,
                                          CWOverrideRedirect,
                                          &attrib);

    astr = g_strdup_printf("_NET_WM_CM_S%d", ob_screen);
    composite_cm_atom = XInternAtom(obt_display, astr, FALSE);
    g_free(astr);

    cm_owner = XGetSelectionOwner(obt_display, composite_cm_atom);
    if (cm_owner != None) return FALSE;

    timestamp = event_time();
    XSetSelectionOwner(obt_display, composite_cm_atom, composite_support_win,
                       timestamp);

    cm_owner = XGetSelectionOwner(obt_display, composite_cm_atom);
    if (cm_owner != composite_support_win) return FALSE;

    /* Send client message indicating that we are now the CM */
    obt_prop_message(ob_screen, obt_root(ob_screen), OBT_PROP_ATOM(MANAGER),
                     timestamp, composite_cm_atom, composite_support_win, 0, 0,
                     SubstructureNotifyMask);

    return TRUE;
}

gboolean composite_enable(void)
{
    int count, val, i;
    XWindowAttributes xa;
    XserverRegion xr;
    XVisualInfo tmp, *vi;
    GLXFBConfig *fbcs;

    if (composite_enabled()) return TRUE;

    g_assert(config_comp);

    /* Check for the required extensions in the server */
    if (!obt_display_extension_composite) {
        g_message(
            _("Failed to enable composite. The %s extension is missing."),
            "XComposite");
        composite_disable();
        return FALSE;
    }
    if (!obt_display_extension_damage) {
        g_message(
            _("Failed to enable composite. The %s extension is missing."),
            "XDamage");
        composite_disable();
        return FALSE;
    }
    if (!obt_display_extension_fixes) {
        g_message(
            _("Failed to enable composite. The %s extension is missing."),
            "XFixes");
        composite_disable();
        return FALSE;
    }

    /* Make sure the root window's visual is acceptable for our GLX needs
       and create a GLX context with it */

    if (!XGetWindowAttributes(obt_display, obt_root(ob_screen), &xa)) {
        g_message(_("Failed to enable composite. %s failed."),
                  "XGetWindowAttributes");
        composite_disable();
        return FALSE;
    }
    tmp.visualid = XVisualIDFromVisual(xa.visual);
    vi = XGetVisualInfo(obt_display, VisualIDMask, &tmp, &count);
    if (!count) {
        g_message(
            _("Failed to enable composite. Failed to get visual info."));
        composite_disable();
        return FALSE;
    }
    glXGetConfig(obt_display, vi, GLX_USE_GL, &val);
    if (!val) {
        g_message(_("Failed to enable composite. Visual is not GL capable"));
        XFree(vi);
        composite_disable();
        return FALSE;
    }
    glXGetConfig(obt_display, vi, GLX_DOUBLEBUFFER, &val);
    if (!val) {
        g_message(
            _("Failed to enable composite. Visual is not double buffered"));
        XFree(vi);
        composite_disable();
        return FALSE;
    }
    composite_ctx = glXCreateContext(obt_display, vi, NULL, True);
    XFree(vi);
    if (!composite_ctx) {
        g_message(
            _("Failed to enable composite. Unable to create GLX context"));
        composite_disable();
        return FALSE;
    }

    /* Attempt to take over as composite manager.  There can only be one. */

    if (!composite_annex()) {
        g_message(_("Failed to enable composite. Another composite manager is running."));
        composite_disable();
        return FALSE;
    }

    /* Set up the overlay window */

    composite_overlay = XCompositeGetOverlayWindow(obt_display,
                                                   obt_root(ob_screen));
    if (!composite_overlay) {
        g_message(_("Failed to enable composite. Unable to get overlay window from X server"));
        composite_disable();
        return FALSE;
    }
    xr = XFixesCreateRegion(obt_display, NULL, 0);
    XFixesSetWindowShapeRegion(obt_display, composite_overlay, ShapeBounding,
                               0, 0, 0);
    XFixesSetWindowShapeRegion(obt_display, composite_overlay, ShapeInput,
                               0, 0, xr);
    XFixesDestroyRegion(obt_display, xr);

    /* Need a current GL context before GLEW works. */

    glXMakeCurrent(obt_display, composite_overlay, composite_ctx);

    /* init GLEW */
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        g_message(_("Failed to enable composite.  GLEW init failed."));
        composite_disable();
        return FALSE;
    }

    /* Check for required GLX extensions */

    if (!GLXEW_EXT_texture_from_pixmap) {
        g_message(_("Failed to enable composite. %s is not present."),
                  "GLX_EXT_texture_from_pixmap");
        composite_disable();
        return FALSE;
    }

    /* Check for FBconfigs */
//XXX: Technically we should test for GL 1.3 before using this, but that
//disqualifies some drivers that support parts of GL 1.3 yet report GL 1.2
    fbcs = glXGetFBConfigs(obt_display, ob_screen, &count);
    if (!count) {
        g_message(_("Failed to enable composite. No valid FBConfigs."));
        composite_disable();
        return FALSE;
    }
    memset(&pixmap_config, 0, sizeof(pixmap_config));
    for (i = 1; i < MAX_DEPTH + 1; i++)
        get_best_fbcon(fbcs, count, i, &pixmap_config[i]);
    if (count) XFree(fbcs);

    printf("Best visual for 24bpp was 0x%lx\n",
           (gulong)pixmap_config[24].fbc);
    printf("Best visual for 32bpp was 0x%lx\n",
           (gulong)pixmap_config[32].fbc);

    /* We're good to go for composite ! */

    /* register our screen redraw callback */
    composite_idle_source = g_idle_add(composite, NULL);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glXSwapBuffers(obt_display, composite_overlay);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    composite_resize();
    window_foreach(composite_window_redir);

    return TRUE;
}

void composite_disable(void)
{
    if (composite_ctx) {
        obt_display_ignore_errors(TRUE);
        glXMakeCurrent(obt_display, None, NULL);
        obt_display_ignore_errors(FALSE);

        glXDestroyContext(obt_display, composite_ctx);
        composite_ctx = NULL;
    }

    if (composite_overlay) {
        XCompositeReleaseOverlayWindow(obt_display, composite_overlay);
        composite_overlay = None;
    }

    if (composite_support_win)
        XDestroyWindow(obt_display, composite_support_win);

    window_foreach(composite_window_unredir);

    g_source_remove(composite_idle_source);
    composite_idle_source = 0;
}

/*! This function will try enable composite if config_comp is TRUE.  At the
  end of this process, config_comp will be set to TRUE only if composite
  is enabled, and FALSE otherwise. */
void composite_startup(gboolean reconfig)
{
#ifdef DEBUG
    composite_started = TRUE;
#endif

    if (!reconfig) {
        if (ob_comp_indirect)
            setenv("LIBGL_ALWAYS_INDIRECT", "1", TRUE);
    }
}

void composite_shutdown(gboolean reconfig)
{
#ifdef DEBUG
    composite_started = FALSE;
#endif

    if (reconfig) return;

    if (composite_enabled())
        composite_disable();
}

void composite_resize(void)
{
    const Rect *a;

    if (!composite_enabled()) return;

    a = screen_physical_area_all_monitors();
    glOrtho(a->x, a->x + a->width, a->y + a->height, a->y, -100, 100);
}

static gboolean composite(gpointer data)
{
    struct timeval start, end, dif;
    GList *it;
    ObWindow *win;
    ObClient *client = NULL;

    if (!composite_enabled()) return FALSE;

/*    if (!need_redraw) return FALSE; */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/* XXX for (it = stacking_list_last; it; it = g_list_previous(it)) { */
    for (it = g_list_last(stacking_list); it; it = g_list_previous(it)) {
        gint d, x, y, w, h;

        win = it->data;
        if (win->type == OB_WINDOW_CLASS_PROMPT)
            continue;

        if (!win->mapped || !win->is_redir)
            continue;

        if (win->type == OB_WINDOW_CLASS_CLIENT) {
            client = WINDOW_AS_CLIENT(win);
            if (!client->frame->visible)
                continue;
        }

        d = window_depth(win);

        if (win->pixmap == None) {
            obt_display_ignore_errors(TRUE);
            win->pixmap = XCompositeNameWindowPixmap(obt_display,
                                                     window_redir(win));
            obt_display_ignore_errors(FALSE);
            if (obt_display_error_occured) {
                ob_debug_type(OB_DEBUG_CM,
                              "Error in XCompositeNameWindowPixmap for "
                              "window 0x%x of 0x%x",
                              window_redir(win), window_top(win));
                /* it can error but still return an ID, which will cause an
                   error to occur if you try to free it etc */
                if (win->pixmap) {
                    obt_display_ignore_errors(TRUE);
                    XFreePixmap(obt_display, win->pixmap);
                    obt_display_ignore_errors(FALSE);
                    win->pixmap = None;
                }
            }
        }
        if (win->pixmap == None)
            continue;

        if (win->gpixmap == None) {
            int attribs[] = {
                GLX_TEXTURE_FORMAT_EXT,
                pixmap_config[d].tf,
                GLX_TEXTURE_TARGET_EXT,
                GLX_TEXTURE_2D_EXT,
                None
            };
            obt_display_ignore_errors(TRUE);
            win->gpixmap = glXCreatePixmap(obt_display,
                                           pixmap_config[d].fbc,
                                           win->pixmap, attribs);
            obt_display_ignore_errors(FALSE);
            if (obt_display_error_occured)
                g_assert(0 && "ERROR CREATING GLX PIXMAP FROM NAMED PIXMAP");
        }
        if (win->gpixmap == None)
            continue;

        glBindTexture(GL_TEXTURE_2D, win->texture);
#ifdef DEBUG
        gettimeofday(&start, NULL);
#endif
        obt_display_ignore_errors(TRUE);
        glXBindTexImageEXT(obt_display, win->gpixmap, GLX_FRONT_LEFT_EXT,
                           NULL);
        obt_display_ignore_errors(FALSE);
        if (obt_display_error_occured)
            g_assert(0 && "ERROR BINDING GLX PIXMAP");
#ifdef DEBUG
        gettimeofday(&end, NULL);
        dif.tv_sec = end.tv_sec - start.tv_sec;
        dif.tv_usec = end.tv_usec - start.tv_usec;
        time_fix(&dif);
//printf("took %f ms\n", dif.tv_sec * 1000.0 + dif.tv_usec / 1000.0);
#endif

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        x = win->toparea.x + win->topborder + win->area.x;
        y = win->toparea.y + win->topborder + win->area.y;
        w = win->area.width;
        h = win->area.height;

        if (win->alpha && *win->alpha < 0xffffffff)
            glColor4ui(0xffffffff, 0xffffffff, 0xffffffff, *win->alpha);

        glBegin(GL_QUADS);
        if (!win->n_rects) {
            glTexCoord2f(0, 0);
            glVertex3f(x, y, 0.0);
            glTexCoord2f(0, 1);
            glVertex3f(x, y + h, 0.0);
            glTexCoord2f(1, 1);
            glVertex3f(x + w, y + h, 0.0);
            glTexCoord2f(1, 0);
            glVertex3f(x + w, y, 0.0);
        }
        else {
            gint i;
            /* the border is not included in the shape rect coords */
            const gint sb = window_top(win) == window_redir(win) ?
                win->topborder : 0;


            for (i = 0; i < win->n_rects; ++i) {
                const gint xb = win->rects[i].x + sb;
                const gint yb = win->rects[i].y + sb;
                const gint wb = win->rects[i].width;
                const gint hb = win->rects[i].height;

                glTexCoord2d((GLdouble)xb/w,
                             (GLdouble)yb/h);
                glVertex3f(x + xb, y + yb, 0.0f);

                glTexCoord2d((GLdouble)xb/w,
                             (GLdouble)(yb+hb)/h);
                glVertex3f(x + xb, y + yb + hb, 0.0f);

                glTexCoord2d((GLdouble)(xb+wb)/w,
                             (GLdouble)(yb+hb)/h);
                glVertex3f(x + xb + wb, y + yb + hb, 0.0f);

                glTexCoord2d((GLdouble)(xb+wb)/w,
                             (GLdouble)yb/h);
                glVertex3f(x + xb + wb, y + yb, 0.0f);
            }
        }
        glEnd();

        if (win->alpha && *win->alpha < 0xffffffff)
            glColor4f(1.0, 1.0, 1.0, 1.0);

        if (client && (client->frame->decorations & OB_FRAME_DECOR_BORDER)) {
            int a, b, c, d;
            a = client->frame->size.left;
            b = client->frame->size.right;
            c = client->frame->size.top;
            d = client->frame->size.bottom;
            if (client->frame->focused)
                glColor4f(ob_rr_theme->frame_focused_border_color->r/255.0,
                          ob_rr_theme->frame_focused_border_color->g/255.0,
                          ob_rr_theme->frame_focused_border_color->b/255.0,
                          0.5);
            else
                glColor4f(ob_rr_theme->frame_unfocused_border_color->r/255.0,
                          ob_rr_theme->frame_unfocused_border_color->g/255.0,
                          ob_rr_theme->frame_unfocused_border_color->b/255.0,
                          0.5);
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);
            glVertex3f(x - a, y - c, 0.0);
            glVertex3f(x + w, y - c, 0.0);
            glVertex3f(x + w, y, 0.0);
            glVertex3f(x - a, y, 0.0);

            glVertex3f(x + w, y - c, 0.0);
            glVertex3f(x + w + b, y - c, 0.0);
            glVertex3f(x + w + b, y + h + d, 0.0);
            glVertex3f(x + w, y + h + d, 0.0);

            glVertex3f(x - a, y + h + d, 0.0);
            glVertex3f(x + w, y + h + d, 0.0);
            glVertex3f(x + w, y + h, 0.0);
            glVertex3f(x - a, y + h, 0.0);

            glVertex3f(x - a, y, 0.0);
            glVertex3f(x, y, 0.0);
            glVertex3f(x, y + h, 0.0);
            glVertex3f(x - a, y + h, 0.0);
            glEnd();
            glEnable(GL_TEXTURE_2D);
            glColor4f(1.0, 1.0, 1.0, 1.0);
        }

        obt_display_ignore_errors(TRUE);
        glXReleaseTexImageEXT(obt_display, win->gpixmap, GLX_FRONT_LEFT_EXT);
        obt_display_ignore_errors(FALSE);
        if (obt_display_error_occured)
            g_assert(0 && "ERROR RELEASING GLX PIXMAP");
    }

    glXSwapBuffers(obt_display, composite_overlay);
    glFinish();

    if (ob_comp_indirect)
        g_usleep(1000);

#ifdef DEBUG
    {
        GLenum gler;
        while ((gler = glGetError()) != GL_NO_ERROR) {
            printf("gl error %d\n", gler);
        }
    }
#endif

    need_redraw = 0;
    return TRUE;
}

static void composite_window_redir(ObWindow *w)
{
    if (!composite_enabled()) return;

    if (w->is_redir) return;

    XCompositeRedirectWindow(obt_display, window_redir(w),
                             CompositeRedirectManual);
    w->is_redir = TRUE;
}

static void composite_window_unredir(ObWindow *w)
{
    if (!w->redir) return;

    XCompositeUnredirectWindow(obt_display, window_redir(w),
                               CompositeRedirectManual);
    w->is_redir = FALSE;
}

void composite_window_setup(ObWindow *w)
{
    if (w->type == OB_WINDOW_CLASS_PROMPT) return;

#ifdef DEBUG
    g_assert(composite_started);
#endif

    obt_display_ignore_errors(TRUE);
    w->damage = XDamageCreate(obt_display, window_top(w),
                              XDamageReportNonEmpty);
    obt_display_ignore_errors(FALSE);
    glGenTextures(1, &w->texture);

    composite_window_redir(w);
}

void composite_window_cleanup(ObWindow *w)
{
    if (w->type == OB_WINDOW_CLASS_PROMPT) return;

#ifdef DEBUG
    g_assert(composite_started);
#endif

    composite_window_unredir(w);
    composite_window_invalid(w);
    if (w->damage) {
        XDamageDestroy(obt_display, w->damage);
        w->damage = None;
    }
    if (w->texture) {
        glDeleteTextures(1, &w->texture);
        w->texture = 0;
    }
}

void composite_window_invalid(ObWindow *w)
{
    if (w->gpixmap) {
        glXDestroyPixmap(obt_display, w->gpixmap);
        w->gpixmap = None;
    }
    if (w->pixmap) {
        XFreePixmap(obt_display, w->pixmap);
        w->pixmap = None;
    }
}
        
#else
void composite_startup        (gboolean boiv) {;(void)(biov);}
void composite_shutdown       (gboolean boiv) {;(void)(biov);}
void composite                (void)          {}
void composite_resize         (void)          {}
void composite_disable        (void)          {}
void composite_window_setup   (ObWindow *w)   {}
void composite_window_cleanup (ObWindow *w)   {}
void composite_window_invalid (ObWindow *w)   {}

void composite_enable(void)
{
    g_message(
        _("Unable to use compositing. Openbox was compiled without it."));
}
#endif
