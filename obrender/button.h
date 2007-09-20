#ifndef __button_h
#define __button_h

#include "render.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glib.h>

struct _RrButton {
    const RrInstance *inst;

    /* reference count */
    gint ref;

    /* colors */
    RrColor *focused_unpressed_color;
    RrColor *unfocused_unpressed_color;
    RrColor *focused_pressed_color;
    RrColor *unfocused_pressed_color;
    RrColor *disabled_focused_color;
    RrColor *disabled_unfocused_color;
    RrColor *hover_focused_color;
    RrColor *hover_unfocused_color;
    RrColor *toggled_hover_focused_color;
    RrColor *toggled_hover_unfocused_color;
    RrColor *toggled_focused_pressed_color;
    RrColor *toggled_unfocused_pressed_color;
    RrColor *toggled_focused_unpressed_color;
    RrColor *toggled_unfocused_unpressed_color;
    
    /* masks */
    RrPixmapMask *mask;
    RrPixmapMask *pressed_mask;
    RrPixmapMask *disabled_mask;
    RrPixmapMask *hover_mask;
    RrPixmapMask *toggled_mask;
    RrPixmapMask *toggled_hover_mask;
    RrPixmapMask *toggled_pressed_mask;
   
    /* textures */
    RrAppearance *a_focused_unpressed;
    RrAppearance *a_unfocused_unpressed;
    RrAppearance *a_focused_pressed;
    RrAppearance *a_unfocused_pressed;
    RrAppearance *a_disabled_focused;
    RrAppearance *a_disabled_unfocused;
    RrAppearance *a_hover_focused;
    RrAppearance *a_hover_unfocused;
    RrAppearance *a_toggled_focused_unpressed;
    RrAppearance *a_toggled_unfocused_unpressed;
    RrAppearance *a_toggled_focused_pressed;
    RrAppearance *a_toggled_unfocused_pressed;
    RrAppearance *a_toggled_hover_focused;
    RrAppearance *a_toggled_hover_unfocused;

};

#endif /* __button_h */
