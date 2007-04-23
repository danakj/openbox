/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   popup.h for the Openbox window manager
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

#ifndef __popup_h
#define __popup_h

#include "window.h"
#include "render/render.h"
#include <glib.h>

struct _ObClientIcon;

#define POPUP_WIDTH 320
#define POPUP_HEIGHT 48

typedef struct _ObPopup      ObPopup;
typedef struct _ObIconPopup  ObIconPopup;
typedef struct _ObPagerPopup ObPagerPopup;

struct _ObPopup
{
    ObWindow obwin;
    Window bg;

    Window text;

    gboolean hasicon;
    RrAppearance *a_bg;
    RrAppearance *a_text;
    gint gravity;
    gint x;
    gint y;
    gint w;
    gint h;
    gboolean mapped;
 
    void (*draw_icon)(gint x, gint y, gint w, gint h, gpointer data);
    gpointer draw_icon_data;
};

struct _ObIconPopup
{
    ObPopup *popup;

    Window icon;
    RrAppearance *a_icon;
};

struct _ObPagerPopup
{
    ObPopup *popup;

    guint desks;
    guint curdesk;
    Window *wins;
    RrAppearance *hilight;
    RrAppearance *unhilight;
};

ObPopup *popup_new(gboolean hasicon);
void popup_free(ObPopup *self);

/*! Position the popup. The gravity rules are not the same X uses for windows,
  instead of the position being the top-left of the window, the gravity
  specifies which corner of the popup will be placed at the given coords.
  Static and Forget gravity are equivilent to NorthWest.
*/
void popup_position(ObPopup *self, gint gravity, gint x, gint y);
/*! Set the sizes for the popup. When set to 0, the size will be based on
  the text size. */
void popup_size(ObPopup *self, gint w, gint h);
void popup_size_to_string(ObPopup *self, gchar *text);

void popup_set_text_align(ObPopup *self, RrJustify align);

void popup_show(ObPopup *self, gchar *text);
void popup_hide(ObPopup *self);

RrAppearance *popup_icon_appearance(ObPopup *self);


ObIconPopup *icon_popup_new();
void icon_popup_free(ObIconPopup *self);

void icon_popup_show(ObIconPopup *self,
                     gchar *text, const struct _ObClientIcon *icon);
#define icon_popup_hide(p) popup_hide((p)->popup)
#define icon_popup_position(p, g, x, y) popup_position((p)->popup,(g),(x),(y))
#define icon_popup_size(p, w, h) popup_size((p)->popup,(w),(h))
#define icon_popup_size_to_string(p, s) popup_size_to_string((p)->popup,(s))
#define icon_popup_set_text_align(p, j) popup_set_text_align((p)->popup,(j))

ObPagerPopup *pager_popup_new();
void pager_popup_free(ObPagerPopup *self);

void pager_popup_show(ObPagerPopup *self, gchar *text, guint desk);
#define pager_popup_hide(p) popup_hide((p)->popup)
#define pager_popup_position(p, g, x, y) popup_position((p)->popup,(g),(x),(y))
#define pager_popup_size(p, w, h) popup_size((p)->popup,(w),(h))
#define pager_popup_size_to_string(p, s) popup_size_to_string((p)->popup,(s))
#define pager_popup_set_text_align(p, j) popup_set_text_align((p)->popup,(j))

#endif
