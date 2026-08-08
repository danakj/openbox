/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   theme.h for the Openbox window manager
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

#ifndef __theme_h
#define __theme_h

#include "render.h"

G_BEGIN_DECLS

typedef struct _RrTheme RrTheme;

struct _RrTheme {
    const RrInstance *inst;

    /* style settings - fonts */
    RrFont *win_font_focused;
    RrFont *win_font_unfocused;
    RrFont *menu_title_font;
    RrFont *menu_font;
    RrFont *osd_font_hilite;
    RrFont *osd_font_unhilite;

    /* style settings - geometry */
    gint paddingx;
    gint paddingy;
    gint handle_height;
    gint fbwidth; /*!< frame border width */
    gint mbwidth; /*!< menu border width */
    gint obwidth; /*!< osd border width */
    gint ubwidth; /*!< undecorated frame border width */
    gint cbwidthx;
    gint cbwidthy;
    gint menu_overlap_x;
    gint menu_overlap_y;
    gint menu_sep_width;
    gint menu_sep_paddingx;
    gint menu_sep_paddingy;
    /* these ones are calculated, not set directly by the theme file */
    gint win_font_height;
    gint menu_title_font_height;
    gint menu_font_height;
    gint label_height;
    gint title_height;
    /*! ABI COMPATIBILITY FIELD -- do not move, rename, or remove.
        =========================================================
        This field must stay at exactly this position in the struct.
        RrTheme is a *public* struct (declared in this installed header,
        obrender/theme.h) that external programs compile against
        directly -- they don't just call our functions, they read
        theme->some_field at whatever byte offset their own compiled
        copy of this header says that field lives at. The shared
        library (libobrender.so.32) and any such external program
        (obconf is the known case, but there may be others: panels,
        theme switchers, previewers, anything linking obrender) do NOT
        get recompiled together -- they are separate packages, updated
        independently by the package manager.

        So: inserting a field *before* this point, or deleting this
        field outright, silently shifts the compiled-in offsets that
        every field *after* it is read at in every external binary
        that hasn't been rebuilt against the new header. Those programs
        don't get a compile error -- they just start reading the wrong
        bytes as soon as the new library is installed, misinterpreting
        e.g. an RrColor* pointer as some unrelated gint, and typically
        segfault almost immediately (this is exactly what happened to
        obconf during development of the button/titlebar geometry
        rework: replacing this field in place with two new ones,
        button_height and button_width, shifted every RrColor pointer
        and RrAppearance pointer field declared below it, and obconf
        crashed the instant it dereferenced one of those now-misaligned
        pointers).

        The fix, and the rule going forward: never insert or resize a
        field in the middle of this struct. New fields belonging to
        this stage of the work (kgrip, button_height, button_width)
        are instead appended after the last original field (see the
        end of this struct, past `name`) -- appending only grows the
        struct and cannot change any earlier field's offset, so old
        binaries keep reading everything before the append point
        correctly. button_size itself is kept alive (rather than
        deleted) and populated in theme.c as a mirror of button_height,
        so that legacy code which still reads a single square
        button_size -- like obconf's theme-preview button -- gets a
        sane value instead of whatever garbage now sits at that offset.
        It is not used anywhere in this codebase's own layout logic
        any more; use button_height/button_width for that. */
    gint button_size;
    gint grip_width;
    gint menu_title_label_height;
    gint menu_title_height;

    /* style settings - colors */
    RrColor *menu_border_color;
    RrColor *osd_border_color;
    RrColor *frame_focused_border_color;
    RrColor *frame_undecorated_focused_border_color;
    RrColor *frame_unfocused_border_color;
    RrColor *frame_undecorated_unfocused_border_color;
    RrColor *title_separator_focused_color;
    RrColor *title_separator_unfocused_color;
    RrColor *cb_focused_color;
    RrColor *cb_unfocused_color;
    RrColor *title_focused_color;
    RrColor *title_unfocused_color;
    RrColor *titlebut_focused_disabled_color;
    RrColor *titlebut_unfocused_disabled_color;
    RrColor *titlebut_focused_hover_color;
    RrColor *titlebut_unfocused_hover_color;
    RrColor *titlebut_focused_hover_toggled_color;
    RrColor *titlebut_unfocused_hover_toggled_color;
    RrColor *titlebut_focused_pressed_toggled_color;
    RrColor *titlebut_unfocused_pressed_toggled_color;
    RrColor *titlebut_focused_unpressed_toggled_color;
    RrColor *titlebut_unfocused_unpressed_toggled_color;
    RrColor *titlebut_focused_pressed_color;
    RrColor *titlebut_unfocused_pressed_color;
    RrColor *titlebut_focused_unpressed_color;
    RrColor *titlebut_unfocused_unpressed_color;
    RrColor *menu_title_color;
    RrColor *menu_sep_color;
    RrColor *menu_color;
    RrColor *menu_bullet_color;
    RrColor *menu_bullet_selected_color;
    RrColor *menu_selected_color;
    RrColor *menu_disabled_color;
    RrColor *menu_disabled_selected_color;
    RrColor *title_focused_shadow_color;
    gchar    title_focused_shadow_alpha;
    RrColor *title_unfocused_shadow_color;
    gchar    title_unfocused_shadow_alpha;
    RrColor *osd_text_active_color;
    RrColor *osd_text_inactive_color;
    RrColor *osd_text_active_shadow_color;
    RrColor *osd_text_inactive_shadow_color;
    gchar    osd_text_active_shadow_alpha;
    gchar    osd_text_inactive_shadow_alpha;
    RrColor *osd_pressed_color;
    RrColor *osd_unpressed_color;
    RrColor *osd_focused_color;
    RrColor *osd_pressed_lineart;
    RrColor *osd_focused_lineart;
    RrColor *menu_title_shadow_color;
    RrColor *menu_text_shadow_color;

    /* style settings - pics */
    RrPixel32 *def_win_icon; /* RGBA */
    gint       def_win_icon_w;
    gint       def_win_icon_h;

    /* style settings - masks */
    RrPixmapMask *menu_bullet_mask; /* submenu pointer */
#if 0
    RrPixmapMask *menu_toggle_mask; /* menu boolean */
#endif

    RrPixmapMask *down_arrow_mask;
    RrPixmapMask *up_arrow_mask;

    /* buttons */
    RrButton *btn_max;
    RrButton *btn_close;
    RrButton *btn_desk;
    RrButton *btn_shade;
    RrButton *btn_iconify;

    /* global appearances */
    RrAppearance *a_focused_grip;
    RrAppearance *a_unfocused_grip;
    RrAppearance *a_focused_title;
    RrAppearance *a_unfocused_title;
    RrAppearance *a_focused_label;
    RrAppearance *a_unfocused_label;
    /* always parentrelative, so no focused/unfocused */
    RrAppearance *a_icon;
    RrAppearance *a_focused_handle;
    RrAppearance *a_unfocused_handle;
    RrAppearance *a_menu_text_title;
    RrAppearance *a_menu_title;
    RrAppearance *a_menu;
    RrAppearance *a_menu_normal;
    RrAppearance *a_menu_selected;
    RrAppearance *a_menu_disabled;
    RrAppearance *a_menu_disabled_selected;
    RrAppearance *a_menu_text_normal;
    RrAppearance *a_menu_text_disabled;
    RrAppearance *a_menu_text_disabled_selected;
    RrAppearance *a_menu_text_selected;
    RrAppearance *a_menu_bullet_normal;
    RrAppearance *a_menu_bullet_selected;
    RrAppearance *a_clear;     /* clear with no texture */
    RrAppearance *a_clear_tex; /* clear with a texture */

    RrAppearance *osd_bg; /* can never be parent relative */
    RrAppearance *osd_hilite_bg; /* can never be parent relative */
    RrAppearance *osd_hilite_label; /* can be parent relative */
    RrAppearance *osd_unhilite_bg; /* can never be parent relative */
    RrAppearance *osd_unhilite_label; /* can be parent relative */
    RrAppearance *osd_pressed_button;
    RrAppearance *osd_unpressed_button;
    RrAppearance *osd_focused_button;

    gchar *name;

    /* =====================================================================
       APPEND-ONLY ZONE -- new fields from here on, nothing above this line.
       =====================================================================
       `name` above is the last field of the original, upstream RrTheme
       struct (see the ABI comment on button_size, further up, for the
       full explanation of why this matters). Every field added below
       was deliberately placed *after* the original struct's end rather
       than inserted among the existing fields, specifically so that
       external programs already compiled against the stock layout --
       obconf being the concrete example that broke -- go on reading
       every pre-existing field (all the RrColor and RrAppearance pointers
       etc. above) at the exact offsets they were compiled for, even
       though this library's struct is now physically larger than
       theirs. Those programs simply don't know these new fields exist,
       which is fine: they never try to read past `name`.

       Any future geometry work should keep following this same rule:
       append new fields here, don't insert them earlier in the struct,
       and don't repurpose/resize an existing field's type. If a field
       genuinely needs to be removed, keep it in place (like
       button_size) and just stop relying on it internally, rather than
       deleting it outright. */

    /*! Room at the top of the titlebar, above the buttons, that acts as
      a hover zone for the top-resize cursor. Read directly from the
      theme file (clamped), like paddingx/paddingy. */
    gint kgrip;
    /*! Real (non-square) button dimensions. button_width is derived
        from button_height via the WISTOH constant in theme.c. */
    gint button_height;
    gint button_width;
};

/*! The font values are all optional. If a NULL is used for any of them, then
  the default font will be used. */
RrTheme* RrThemeNew(const RrInstance *inst, const gchar *theme,
                    gboolean allow_fallback,
                    RrFont *active_window_font, RrFont *inactive_window_font,
                    RrFont *menu_title_font, RrFont *menu_item_font,
                    RrFont *active_osd_font, RrFont *inactive_osd_font);
void RrThemeFree(RrTheme *theme);

G_END_DECLS

#endif
