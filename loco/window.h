/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   window.h for the Openbox compositor
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

#ifndef loco__window_h
#define loco__window_h

#include "obt/display.h"
#include <X11/Xlib.h>
#include <GL/glx.h>

struct _LocoScreen;

typedef enum {
    a /* XXX fill this in */
} LocoWindowType;

typedef struct _LocoWindow {
    gint      ref;

    Window    id;
    gint      type; /* XXX make this an enum */

    struct _LocoScreen *screen;

    gint      x, y, w, h;

    gint      depth;

    gboolean  input_only;
    gboolean  visible;
    gboolean  damaged;
    gboolean  stale;      /* glpixmap is out of date */

    GLuint    texname;
    Damage    damage;
    Pixmap    pixmap;
    GLXPixmap glpixmap;
} LocoWindow;

LocoWindow* loco_window_new(Window xwin, struct _LocoScreen *screen);
void loco_window_ref(LocoWindow *lw);
void loco_window_unref(LocoWindow *lw);

void loco_window_show(LocoWindow *lw);
void loco_window_hide(LocoWindow *lw, gboolean destroyed);

void loco_window_configure(LocoWindow *lw, const XConfigureEvent *e);
void loco_window_damage(LocoWindow *lw);

void loco_window_update_pixmap(LocoWindow *lw);

#endif
