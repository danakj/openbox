/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   button.c for the Openbox window manager
   Copyright (c) 2012        Mikael Magnusson
   Copyright (c) 2012        Dana Jansens

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

#include "render.h"
#include "instance.h"
#include "mask.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>

RrButton *RrButtonNew (const RrInstance *inst)
{
    RrButton *out = NULL;

    out = g_new(RrButton, 1);
    out->inst = inst;

    /* no need to alloc colors, set them null (for freeing later) */
    out->focused_unpressed_color = NULL;
    out->unfocused_unpressed_color = NULL;
    out->focused_pressed_color = NULL;
    out->unfocused_pressed_color = NULL;
    out->disabled_focused_color = NULL;
    out->disabled_unfocused_color = NULL;
    out->hover_focused_color = NULL;
    out->hover_unfocused_color = NULL;
    out->toggled_hover_focused_color = NULL;
    out->toggled_hover_unfocused_color = NULL;
    out->toggled_focused_pressed_color = NULL;
    out->toggled_unfocused_pressed_color = NULL;
    out->toggled_focused_unpressed_color = NULL;
    out->toggled_unfocused_unpressed_color = NULL;

    /* same with masks */
    out->mask = NULL;
    out->pressed_mask = NULL;
    out->disabled_mask = NULL;
    out->hover_mask = NULL;
    out->toggled_mask = NULL;
    out->toggled_hover_mask = NULL;
    out->toggled_pressed_mask = NULL;

    /* allocate appearances */
    out->a_focused_unpressed = RrAppearanceNew(inst, 1);
    out->a_unfocused_unpressed = RrAppearanceNew(inst, 1);
    out->a_focused_pressed = RrAppearanceNew(inst, 1);
    out->a_unfocused_pressed = RrAppearanceNew(inst, 1);
    out->a_disabled_focused = RrAppearanceNew(inst, 1);
    out->a_disabled_unfocused = RrAppearanceNew(inst, 1);
    out->a_hover_focused = RrAppearanceNew(inst, 1);
    out->a_hover_unfocused = RrAppearanceNew(inst, 1);
    out->a_toggled_focused_unpressed = RrAppearanceNew(inst, 1);
    out->a_toggled_unfocused_unpressed = RrAppearanceNew(inst, 1);
    out->a_toggled_focused_pressed = RrAppearanceNew(inst, 1);
    out->a_toggled_unfocused_pressed = RrAppearanceNew(inst, 1);
    out->a_toggled_hover_focused = RrAppearanceNew(inst, 1);
    out->a_toggled_hover_unfocused = RrAppearanceNew(inst, 1);

    return out;
}

void RrButtonFree(RrButton *b)
{
    /* colors */
    if (b->focused_unpressed_color) 
        RrColorFree(b->focused_unpressed_color);
    if (b->unfocused_unpressed_color) 
        RrColorFree(b->unfocused_unpressed_color);
    if (b->focused_pressed_color) 
        RrColorFree(b->focused_pressed_color);
    if (b->unfocused_pressed_color) 
        RrColorFree(b->unfocused_pressed_color);
    if (b->disabled_focused_color) 
        RrColorFree(b->disabled_focused_color);
    if (b->disabled_unfocused_color) 
        RrColorFree(b->disabled_unfocused_color);
    if (b->hover_focused_color) 
        RrColorFree(b->hover_focused_color);
    if (b->hover_unfocused_color) 
        RrColorFree(b->hover_unfocused_color);
    if (b->toggled_hover_focused_color) 
        RrColorFree(b->toggled_hover_focused_color);
    if (b->toggled_hover_unfocused_color) 
        RrColorFree(b->toggled_hover_unfocused_color);
    if (b->toggled_focused_pressed_color) 
        RrColorFree(b->toggled_focused_pressed_color);
    if (b->toggled_unfocused_pressed_color) 
        RrColorFree(b->toggled_unfocused_pressed_color);
    if (b->toggled_focused_unpressed_color) 
        RrColorFree(b->toggled_focused_unpressed_color);
    if (b->toggled_unfocused_unpressed_color) 
        RrColorFree(b->toggled_unfocused_unpressed_color);

    /* masks */
    if (b->mask) RrPixmapMaskFree(b->mask);
    if (b->pressed_mask) RrPixmapMaskFree(b->pressed_mask);
    if (b->disabled_mask) RrPixmapMaskFree(b->disabled_mask);
    if (b->hover_mask) RrPixmapMaskFree(b->hover_mask);
    if (b->toggled_mask) RrPixmapMaskFree(b->toggled_mask);
    if (b->toggled_hover_mask) RrPixmapMaskFree(b->toggled_hover_mask);
    if (b->toggled_pressed_mask) RrPixmapMaskFree(b->toggled_pressed_mask);

    /* appearances */
    RrAppearanceFree(b->a_focused_unpressed);
    RrAppearanceFree(b->a_unfocused_unpressed);
    RrAppearanceFree(b->a_focused_pressed);
    RrAppearanceFree(b->a_unfocused_pressed);
    RrAppearanceFree(b->a_disabled_focused);
    RrAppearanceFree(b->a_disabled_unfocused);
    RrAppearanceFree(b->a_hover_focused);
    RrAppearanceFree(b->a_hover_unfocused);
    RrAppearanceFree(b->a_toggled_focused_unpressed);
    RrAppearanceFree(b->a_toggled_unfocused_unpressed);
    RrAppearanceFree(b->a_toggled_focused_pressed);
    RrAppearanceFree(b->a_toggled_unfocused_pressed);
    RrAppearanceFree(b->a_toggled_hover_focused);
    RrAppearanceFree(b->a_toggled_hover_unfocused);
}
