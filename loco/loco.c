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

typedef struct {
    Window id;
    gint x, y, w, h;
    gboolean input_only;
    gboolean visible;
    gint type; /* XXX make this an enum */
} LocoWindow;

typedef struct _LocoList {
    struct _LocoList *next;
    struct _LocoList *prev;
    LocoWindow *window;
} LocoList;

static Window      loco_root;
/* Maps X Window ID -> LocoWindow* */
static GHashTable *window_map;
/* Maps X Window ID -> LocoList* which is in the stacking_top/bottom list */
static GHashTable *stacking_map;
/* From top to bottom */
static LocoList   *stacking_top;
static LocoList   *stacking_bottom;

/*
static void print_stacking()
{
    LocoList *it;

    for (it = stacking_top; it; it = it->next) {
        printf("0x%lx\n", it->window->id);
    }
}
*/

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

void composite_setup_window(LocoWindow *win)
{
    if (win->input_only) return;

    //XCompositeRedirectWindow(obt_display, win->id, CompositeRedirectAutomatic);
    /*something useful = */XDamageCreate(obt_display, win->id, XDamageReportRawRectangles);
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
    g_hash_table_insert(window_map, &lw->id, lw);
    /* new windows are at the top */
    it = loco_list_prepend(&stacking_top, &stacking_bottom, lw);
    g_hash_table_insert(stacking_map, &lw->id, it);

    composite_setup_window(lw);

    //print_stacking();
}

static void remove_window(LocoWindow *lw)
{
    printf("remove window 0x%lx\n", lw->id);

    LocoList *pos = find_stacking(lw->id);
    g_assert(pos);

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

}

static void hide_window(LocoWindow *lw, gboolean destroyed)
{
    /* if destroyed, then the window is no longer available */
    lw->visible = FALSE;
}

void COMPOSTER_RAWR(const XEvent *e, gpointer data)
{
    if (e->type == ConfigureNotify) {
        LocoWindow *lw;
        printf("Window 0x%lx moved or something\n", e->xconfigure.window);

        lw = find_window(e->xconfigure.window);
        if (lw) {
            LocoList *above, *pos;

            pos = find_stacking(e->xconfigure.window);
            above = find_stacking(e->xconfigure.above);

            g_assert(pos != NULL && pos->window != NULL);
            if (e->xconfigure.above && !above)
                printf("missing windows from the stacking list!!\n");

            lw->x = e->xconfigure.x;
            lw->y = e->xconfigure.y;
            lw->w = e->xconfigure.width;
            lw->h = e->xconfigure.height;
            printf("Window 0x%lx above 0x%lx\n", pos->window->id,
                   above ? above->window->id : 0);
            loco_list_move_before(&stacking_top, &stacking_bottom, pos, above);
        }
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
        if (lw) show_window(lw);
        else
            printf("map notify for unknown window 0x%lx\n",
                   e->xmap.window);
    }
    else if (e->type == UnmapNotify) {
        LocoWindow *lw = find_window(e->xunmap.window);
        if (lw) hide_window(lw, FALSE);
        else
            printf("unmap notify for unknown window 0x%lx\n",
                   e->xunmap.window);
    }
    else if (e->type == obt_display_extension_damage_basep + XDamageNotify) {
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

static guint window_hash(Window *w) { return *w; }
static gboolean window_comp(Window *w1, Window *w2) { return *w1 == *w2; }

void loco_set_mainloop(gint screen_num, ObtMainLoop *loop)
{
    int w, h;
    XVisualInfo *vi;
    GLXContext cont;
    int config[] =
        { GLX_DEPTH_SIZE, 1, GLX_DOUBLEBUFFER, GLX_RGBA, None };

    loco_root = obt_root(screen_num);

    vi = glXChooseVisual(obt_display, screen_num, config);
    cont = glXCreateContext(obt_display, vi, NULL, GL_TRUE);
    if (cont == NULL)
        printf("context creation failed\n");
    glXMakeCurrent(obt_display, loco_root, cont);

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
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glXSwapBuffers(obt_display, loco_root);
    obt_main_loop_x_add(loop, COMPOSTER_RAWR, NULL, NULL);
    window_map = g_hash_table_new((GHashFunc)window_hash,
                                  (GEqualFunc)window_comp);
    stacking_map = g_hash_table_new((GHashFunc)window_hash,
                                    (GEqualFunc)window_comp);
    stacking_top = stacking_bottom = NULL;

    find_all_windows(screen_num);
}

void loco_shutdown(void)
{
}
