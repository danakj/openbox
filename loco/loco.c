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

static void remove_window(Window window)
{
    LocoWindow *lw;

    printf("remove window 0x%lx\n", window);

    lw = g_hash_table_lookup(window_map, &window);
    if (lw) {
        LocoList *pos = g_hash_table_lookup(stacking_map, &window);
        g_assert(pos);

        loco_list_delete_link(&stacking_top, &stacking_bottom, pos);
        g_hash_table_remove(stacking_map, &window);
        g_hash_table_remove(window_map, &window);

        g_free(lw);
    }

    //print_stacking();
}

void COMPOSTER_RAWR(const XEvent *e, gpointer data)
{
    if (e->type == CreateNotify) {
        add_window(e->xmap.window);
    }
    else if (e->type == DestroyNotify) {
        remove_window(e->xdestroywindow.window);
    }
    else if (e->type == ReparentNotify) {
        if (e->xreparent.parent == loco_root)
            add_window(e->xreparent.window);
        else
            remove_window(e->xreparent.window);
    }
    else if (e->type == obt_display_extension_damage_basep + XDamageNotify) {
    }
    else if (e->type == ConfigureNotify) {
        LocoWindow *lw;
        printf("Window 0x%lx moved or something\n", e->xconfigure.window);

        lw = g_hash_table_lookup(window_map, &e->xconfigure.window);
        if (lw) {
            LocoList *above, *pos;

            pos = g_hash_table_lookup(stacking_map, &e->xconfigure.window);
            above = g_hash_table_lookup(stacking_map, &e->xconfigure.above);

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
