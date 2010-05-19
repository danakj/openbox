/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   framerender.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

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

#include "frame.h"
#include "openbox.h"
#include "screen.h"
#include "client.h"
#include "framerender.h"
#include "obrender/theme.h"

void framerender_frame(ObFrame *self)
{
    if (frame_iconify_animating(self))
        return; /* delay redrawing until the animation is done */
    if (!self->need_render)
        return;
    if (!self->visible)
        return;
    self->need_render = FALSE;

    /* XXX draw everything in the frame */
    {
        gulong px;

        px = (self->focused ?
              RrColorPixel(ob_rr_theme->frame_focused_border_color) :
              RrColorPixel(ob_rr_theme->frame_unfocused_border_color));

        XSetWindowBackground(obt_display, self->window, px);
        XClearWindow(obt_display, self->window);
    }

    if (self->decorations & OB_FRAME_DECOR_TITLEBAR) {
        /* XXX draw the titlebar */
    }

    if (self->decorations & OB_FRAME_DECOR_HANDLE) {
        /* XXX draw the handle */
    }

    XFlush(obt_display);
}
