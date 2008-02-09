/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   screen.h for the Openbox compositor
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

#ifndef loco__screen_h
#define loco__screen_h

#include <X11/Xlib.h>
#include <glib.h>
#include <GL/glx.h>

#define LOCO_SCREEN_MAX_DEPTH 32

struct _LocoWindow;
struct _LocoList;

typedef void (*BindEXTFunc)(Display *, GLXDrawable, int, const int *);
typedef void (*ReleaseEXTFunc)(Display *, GLXDrawable, int);

typedef struct _LocoScreen {
    gint         ref;

    gint         number;

    //struct _Window *root;
    Window          root;
    Window          overlay;

    GLXFBConfig  glxFBConfig[LOCO_SCREEN_MAX_DEPTH + 1];

    gboolean     redraw;

    /* Maps X Window ID -> LocoList* which is in the stacking_top/bottom list
     */
    GHashTable       *stacking_map;
    GHashTable       *stacking_map_ptr;
    /* The stacking list goes from top-most to bottom-most window */
    struct _LocoList *stacking_top;
    struct _LocoList *stacking_bottom;

    BindEXTFunc    bindTexImageEXT;
    ReleaseEXTFunc releaseTexImageEXT;
} LocoScreen;

LocoScreen* loco_screen_new(gint number);
void loco_screen_ref(LocoScreen *sc);
void loco_screen_unref(LocoScreen *sc);

void loco_screen_add_window(LocoScreen *sc, struct _LocoWindow *lw);
void loco_screen_zombie_window(LocoScreen *sc, struct _LocoWindow *lw);
void loco_screen_remove_window(LocoScreen *sc, struct _LocoWindow *lw);

struct _LocoWindow* loco_screen_find_window(LocoScreen *sc, Window xwin);
struct _LocoList* loco_screen_find_stacking(LocoScreen *sc, Window xwin);
struct _LocoList* loco_screen_find_stacking_ptr(LocoScreen *sc,
                                                struct _LocoWindow *lw);

void loco_screen_redraw(LocoScreen *sc);

void loco_screen_redraw_done(LocoScreen *sc);

#endif
