/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   theme.c for the Openbox window manager
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

#include "render.h"
#include "color.h"
#include "font.h"
#include "mask.h"
#include "theme.h"
#include "icon.h"
#include "obt/paths.h"

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*! Titlebar button width-to-height aspect ratio. Not yet theme-configurable;
    promote to an rc key (e.g. via READ_INT with fixed-point scaling) if a
    real-world theme ends up looking distorted with a fixed 1.5. */
#define WISTOH 1.5

/*! Discrete native-resolution glyph tiers, ordered ascending. At theme
    load time, once button_height is known, we pick a tier sized to
    leave a visible margin around the glyph (see pick_glyph_tier below)
    and upsize the default glyphs to that tier's data, in place, before
    RrPixmapMaskDraw ever runs.

    mask.c draws a glyph at its own native pixel size, centered in the
    button, with no scaling of any kind -- so the tier we pick *is*
    the rendered size. A single fixed source size can't serve both a
    small button and a large one well: too small and it's lost in a
    sea of margin on a big button, too large and it either doesn't fit
    (pick_glyph_tier already excludes any tier bigger than
    button_height, since mask.c doesn't clip -- an oversized native
    glyph would just draw past the button's edge) or leaves no margin
    at all. A small discrete set of sizes, picked per-button, covers
    that range without needing mask.c to do any scaling. Themes
    supplying their own .xbm are unaffected either way -- read_button_
    styles() only calls RrPixmapMaskNew() at whatever size the theme's
    own file declares, and pick_glyph_tier's result is only ever
    applied to masks still at the untouched 6x6 default (see
    upsize_mask_inplace's width/height guard below). */
/*! Replaces a mask's contents in place (same RrPixmapMask*, new pixel
    data), rather than freeing the struct and allocating a new one.
    This matters because read_button_styles() (much earlier in
    RrThemeNew) already copied the *pointer value* of e.g.
    btn->unpressed_mask directly into every button appearance's
    texture[0].data.mask.mask. Swapping btn->unpressed_mask to point at
    a freshly allocated struct leaves all of those appearances holding a
    dangling pointer
    to the just-freed old struct, which segfaults the moment any button
    is painted (found via gdb: SIGSEGV in XSetClipMask, called from
    RrPixmapMaskDraw on a freed RrPixmapMask). Mutating the existing
    struct's contents keeps every existing pointer to it valid. */
static void upsize_mask_inplace(const RrInstance *inst, RrPixmapMask *m,
                                const guchar *bigdata, gint bigw, gint bigh)
{
    if (!m || m->width != 6 || m->height != 6) return;
    XFreePixmap(RrDisplay(inst), m->mask);
    g_free(m->data);
    m->width = bigw;
    m->height = bigh;
    m->data = g_memdup(bigdata, (bigw + 7) / 8 * bigh);
    m->mask = XCreateBitmapFromData(RrDisplay(inst), RrRootWindow(inst),
                                    (const gchar*)bigdata, bigw, bigh);
}

static void upsize_button_masks(const RrInstance *inst, RrButton *btn,
                                const guchar *bigdata,
                                const guchar *bigdata_toggled,
                                gint bigw, gint bigh)
{
    /* Only touches masks that are still exactly the small 6x6 built-in
       fallback (i.e. the theme never provided its own .xbm for that
       specific button state) -- anything the theme customized, at any
       resolution, is left completely alone. */
    upsize_mask_inplace(inst, btn->unpressed_mask, bigdata, bigw, bigh);
    upsize_mask_inplace(inst, btn->pressed_mask, bigdata, bigw, bigh);
    upsize_mask_inplace(inst, btn->disabled_mask, bigdata, bigw, bigh);
    upsize_mask_inplace(inst, btn->hover_mask, bigdata, bigw, bigh);
    if (bigdata_toggled) {
        upsize_mask_inplace(inst, btn->unpressed_toggled_mask,
                            bigdata_toggled, bigw, bigh);
        upsize_mask_inplace(inst, btn->pressed_toggled_mask,
                            bigdata_toggled, bigw, bigh);
        upsize_mask_inplace(inst, btn->hover_toggled_mask,
                            bigdata_toggled, bigw, bigh);
    }
}

/*! One glyph set (all 5 buttons, 7 shapes -- max has a toggled/restore
    variant, desk has a toggled/all-desktops-active variant, close/
    shade/iconify don't) at a single native resolution. Generated by
    supersampled rendering + threshold-to-1-bit (4x oversample, drawn
    with anti-aliasing-friendly primitives, downsampled with Lanczos,
    then thresholded at 50% coverage) rather than drawn pixel-by-pixel
    by hand -- this is what actually gives each tier clean, properly
    placed edges instead of a naive blown-up copy of the 6x6 originals. */
typedef struct {
    gint size;
    const guchar *max_normal, *max_toggled, *close;
    const guchar *desk_normal, *desk_toggled;
    const guchar *shade, *iconify;
} ButtonGlyphTier;

        /* ---- 8x8 tier ---- */

        static const guchar max_normal8[] = {
            0x00, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x00
        };

        static const guchar max_toggled8[] = {
            0xf0, 0x90, 0x90, 0xe0, 0x07, 0x0f, 0x0f, 0x0f
        };

        static const guchar close8[] = {
            0x00, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x00
        };

        static const guchar desk_normal8[] = {
            0x00, 0x66, 0x66, 0x00, 0x00, 0x66, 0x66, 0x00
        };

        static const guchar desk_toggled8[] = {
            0x00, 0x00, 0x40, 0x20, 0x12, 0x0c, 0x00, 0x00
        };

        static const guchar shade8[] = {
            0x00, 0x7e, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar iconify8[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x00
        };


        /* ---- 16x16 tier ---- */

        static const guchar max_normal16[] = {
            0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60,
            0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60,
            0x06, 0x60, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00
        };

        static const guchar max_toggled16[] = {
            0x00, 0x00, 0x00, 0x00, 0xe0, 0x3f, 0xe0, 0x3f, 0x60, 0x30, 0xfc, 0x37,
            0xfc, 0x37, 0x6c, 0x36, 0x6c, 0x36, 0xec, 0x3f, 0xec, 0x3f, 0x0c, 0x06,
            0xfc, 0x07, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00
        };

        /* Small circular caps are added at each of the X's 4 endpoints,
           radius roughly half the stroke width, so the strokes end in
           a curve rather than a flat diagonal-cut edge. Visible at
           16/24/32; at 8px the stroke is only ~1-2px wide, too thin for
           the added radius to register, so close8 above has no cap. */
        static const guchar close16[] = {
            0x00, 0x00, 0x0c, 0x30, 0x1e, 0x78, 0x3e, 0x3c, 0x7c, 0x1e, 0xf8, 0x0f,
            0xf0, 0x07, 0xe0, 0x03, 0xe0, 0x07, 0xf0, 0x0f, 0x78, 0x1f, 0x3c, 0x3e,
            0x1e, 0x7c, 0x0e, 0x78, 0x04, 0x30, 0x00, 0x00
        };

        static const guchar desk_normal16[] = {
            0x00, 0x00, 0x00, 0x00, 0x38, 0x1c, 0x3c, 0x3c, 0x7c, 0x3e, 0x3c, 0x3c,
            0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x3c, 0x3c, 0x7c, 0x3e,
            0x3c, 0x3c, 0x38, 0x1c, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar desk_toggled16[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x38, 0x00, 0x3c,
            0x00, 0x1e, 0x00, 0x0e, 0x18, 0x0f, 0xbc, 0x07, 0xfc, 0x03, 0xf8, 0x01,
            0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar shade16[] = {
            0x00, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar iconify16[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0xfc, 0x3f,
            0xfc, 0x3f, 0xfc, 0x3f, 0x00, 0x00, 0x00, 0x00
        };


        /* ---- 24x24 tier ---- */

        static const guchar max_normal24[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0x3f, 0xfc, 0xff, 0x3f,
            0xfc, 0xff, 0x3f, 0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38,
            0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38,
            0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38,
            0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38, 0x1c, 0x00, 0x38, 0xfc, 0xff, 0x3f,
            0xfc, 0xff, 0x3f, 0xfc, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar max_toggled24[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x1f,
            0x00, 0xff, 0x1f, 0x00, 0xff, 0x1f, 0x00, 0x07, 0x1c, 0x00, 0x07, 0x1c,
            0xf8, 0xff, 0x1c, 0xf8, 0xff, 0x1c, 0xf8, 0xff, 0x1c, 0x38, 0xe7, 0x1c,
            0x38, 0xe7, 0x1c, 0x38, 0xff, 0x1f, 0x38, 0xff, 0x1f, 0x38, 0xff, 0x1f,
            0x38, 0xe0, 0x00, 0x38, 0xe0, 0x00, 0xf8, 0xff, 0x00, 0xf8, 0xff, 0x00,
            0xf8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        /* small circular caps at each endpoint, same as close16 above */
        static const guchar close24[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x38, 0x3c, 0x00, 0x3c,
            0x7c, 0x00, 0x3e, 0xf8, 0x00, 0x1f, 0xf0, 0x81, 0x0f, 0xe0, 0xc3, 0x07,
            0xc0, 0xe7, 0x03, 0x80, 0xff, 0x01, 0x00, 0xff, 0x00, 0x00, 0x7e, 0x00,
            0x00, 0x7e, 0x00, 0x00, 0xff, 0x00, 0x80, 0xff, 0x01, 0xc0, 0xe7, 0x03,
            0xe0, 0xc3, 0x07, 0xf0, 0x81, 0x0f, 0xf8, 0x00, 0x1f, 0x7c, 0x00, 0x3e,
            0x3c, 0x00, 0x3c, 0x1c, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar desk_normal24[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x07,
            0xf0, 0x81, 0x0f, 0xf8, 0xc3, 0x1f, 0xf8, 0xc3, 0x1f, 0xf8, 0xc3, 0x1f,
            0xf0, 0x81, 0x0f, 0xe0, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x07, 0xf0, 0x81, 0x0f,
            0xf8, 0xc3, 0x1f, 0xf8, 0xc3, 0x1f, 0xf8, 0xc3, 0x1f, 0xf0, 0x81, 0x0f,
            0xe0, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar desk_toggled24[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1f,
            0x00, 0x80, 0x0f, 0x00, 0x80, 0x0f, 0x00, 0xc0, 0x07, 0x00, 0xe0, 0x03,
            0x20, 0xf0, 0x01, 0xf0, 0xf0, 0x01, 0xf8, 0xf9, 0x00, 0xf0, 0x7f, 0x00,
            0xe0, 0x3f, 0x00, 0xc0, 0x3f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x09, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar shade24[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x1f,
            0xf8, 0xff, 0x1f, 0xf8, 0xff, 0x1f, 0xf8, 0xff, 0x1f, 0xf8, 0xff, 0x1f,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar iconify24[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf8, 0xff, 0x1f, 0xf8, 0xff, 0x1f, 0xf8, 0xff, 0x1f, 0xf8, 0xff, 0x1f,
            0xf8, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };


        /* ---- 32x32 tier ---- */

        static const guchar max_normal32[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f,
            0xf8, 0xff, 0xff, 0x1f, 0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e,
            0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e,
            0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e,
            0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e,
            0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e,
            0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e,
            0x78, 0x00, 0x00, 0x1e, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f,
            0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar max_toggled32[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0x0f, 0x00, 0xfc, 0xff, 0x0f,
            0x00, 0xfc, 0xff, 0x0f, 0x00, 0xfc, 0xff, 0x0f, 0x00, 0x3c, 0x00, 0x0f,
            0x00, 0x3c, 0x00, 0x0f, 0xf0, 0xff, 0x1f, 0x0f, 0xf0, 0xff, 0x3f, 0x0f,
            0xf0, 0xff, 0x3f, 0x0f, 0xf0, 0xff, 0x3f, 0x0f, 0xf0, 0x7c, 0x3e, 0x0f,
            0xf0, 0x3c, 0x3c, 0x0f, 0xf0, 0x3c, 0x3c, 0x0f, 0xf0, 0x7c, 0x3e, 0x0f,
            0xf0, 0xfc, 0xff, 0x0f, 0xf0, 0xfc, 0xff, 0x0f, 0xf0, 0xfc, 0xff, 0x0f,
            0xf0, 0xf8, 0xff, 0x0f, 0xf0, 0x00, 0x3c, 0x00, 0xf0, 0x00, 0x3c, 0x00,
            0xf0, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x3f, 0x00,
            0xf0, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        /* small circular caps at each endpoint, same as close16 above */
        static const guchar close32[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x78, 0x00, 0x00, 0x1e, 0xf8, 0x00, 0x00, 0x1f, 0xf8, 0x01, 0x80, 0x1f,
            0xf8, 0x03, 0xc0, 0x1f, 0xf0, 0x07, 0xe0, 0x0f, 0xe0, 0x0f, 0xf0, 0x07,
            0xc0, 0x1f, 0xf8, 0x03, 0x80, 0x3f, 0xfc, 0x01, 0x00, 0x7f, 0xfe, 0x00,
            0x00, 0xfe, 0x7f, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xf8, 0x1f, 0x00,
            0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf8, 0x1f, 0x00,
            0x00, 0xfc, 0x3f, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0x7f, 0xfe, 0x00,
            0x80, 0x3f, 0xfc, 0x01, 0xc0, 0x1f, 0xf8, 0x03, 0xe0, 0x0f, 0xf0, 0x07,
            0xf0, 0x07, 0xe0, 0x0f, 0xf8, 0x03, 0xc0, 0x1f, 0xfc, 0x01, 0x80, 0x1f,
            0xf8, 0x00, 0x00, 0x1f, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar desk_normal32[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0xe0, 0x01, 0xe0, 0x0f, 0xf0, 0x07,
            0xe0, 0x1f, 0xf8, 0x07, 0xf0, 0x1f, 0xf8, 0x0f, 0xf0, 0x1f, 0xf8, 0x0f,
            0xf0, 0x1f, 0xf8, 0x0f, 0xf0, 0x1f, 0xf8, 0x0f, 0xe0, 0x0f, 0xf0, 0x07,
            0xc0, 0x07, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0xe0, 0x03, 0xe0, 0x0f, 0xf0, 0x07,
            0xf0, 0x1f, 0xf8, 0x0f, 0xf0, 0x1f, 0xf8, 0x0f, 0xf0, 0x1f, 0xf8, 0x0f,
            0xf0, 0x1f, 0xf8, 0x0f, 0xe0, 0x1f, 0xf8, 0x07, 0xe0, 0x0f, 0xf0, 0x07,
            0x80, 0x07, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar desk_toggled32[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x80, 0x1f,
            0x00, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0xe0, 0x07,
            0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x00, 0xfc, 0x01,
            0x00, 0x00, 0xfe, 0x00, 0x80, 0x00, 0x7e, 0x00, 0xc0, 0x01, 0x3f, 0x00,
            0xe0, 0x83, 0x1f, 0x00, 0xf0, 0xc7, 0x1f, 0x00, 0xe0, 0xcf, 0x0f, 0x00,
            0xc0, 0xff, 0x07, 0x00, 0x80, 0xff, 0x03, 0x00, 0x00, 0xff, 0x03, 0x00,
            0x00, 0xfe, 0x01, 0x00, 0x00, 0xd8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar shade32[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
            0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
            0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        static const guchar iconify32[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
            0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
            0xf0, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

static const ButtonGlyphTier button_glyph_tiers[] = {
    {  8, max_normal8,  max_toggled8,  close8,
           desk_normal8,  desk_toggled8,  shade8,  iconify8 },
    { 16, max_normal16, max_toggled16, close16,
           desk_normal16, desk_toggled16, shade16, iconify16 },
    { 24, max_normal24, max_toggled24, close24,
           desk_normal24, desk_toggled24, shade24, iconify24 },
    { 32, max_normal32, max_toggled32, close32,
           desk_normal32, desk_toggled32, shade32, iconify32 },
};
#define N_BUTTON_GLYPH_TIERS \
    ((gint)(sizeof(button_glyph_tiers) / sizeof(button_glyph_tiers[0])))

/*! Returns the tier that fills the button best while keeping a margin
    around the glyph, or NULL to mean "leave the classic 6x6 default
    alone" (only when button_height is below even our smallest tier).

    mask.c draws a glyph at its native pixel size, centered in the
    button -- no scaling. So a tier's "fill" is just its own size, and
    MAX_FILL_RATIO caps how much of the button that's allowed to
    occupy, so a common theme doesn't end up with a glyph that fills
    the button edge-to-edge with no visible margin.

    If no tier fits under that cap (a very large button relative to
    our largest tier), we still pick the closest-fitting tier we have
    rather than falling back to the 6x6 default -- a bigger native
    source with more margin than intended still looks better than the
    tiny default centered in a lot of empty space. */
#define MAX_FILL_RATIO_NUM 7  /* rendered glyph may fill at most 70% of */
#define MAX_FILL_RATIO_DEN 10 /* the button; the remaining 30% is margin */
static const ButtonGlyphTier *pick_glyph_tier(gint button_height)
{
    const gint max_fill = (button_height * MAX_FILL_RATIO_NUM) /
                           MAX_FILL_RATIO_DEN;
    const ButtonGlyphTier *best_under_cap = NULL;
    const ButtonGlyphTier *best_over_cap = NULL;
    gint i;

    for (i = 0; i < N_BUTTON_GLYPH_TIERS; ++i) {
        gint size = button_glyph_tiers[i].size;
        if (size > button_height)
            break; /* sorted ascending; nothing further fits */
        if (size <= max_fill)
            best_under_cap = &button_glyph_tiers[i]; /* largest under cap */
        else if (!best_over_cap)
            best_over_cap = &button_glyph_tiers[i]; /* smallest overshoot */
    }

    /* prefer any tier that fits under the margin cap; only reach for
       the least-bad over-cap tier if literally nothing fit under it */
    return best_under_cap ? best_under_cap : best_over_cap;
}

struct fallbacks {
    RrAppearance *focused_disabled;
    RrAppearance *unfocused_disabled;
    RrAppearance *focused_hover;
    RrAppearance *unfocused_hover;
    RrAppearance *focused_unpressed;
    RrAppearance *focused_pressed;
    RrAppearance *unfocused_unpressed;
    RrAppearance *unfocused_pressed;
    RrAppearance *focused_hover_toggled;
    RrAppearance *unfocused_hover_toggled;
    RrAppearance *focused_unpressed_toggled;
    RrAppearance *focused_pressed_toggled;
    RrAppearance *unfocused_unpressed_toggled;
    RrAppearance *unfocused_pressed_toggled;
};

static XrmDatabase loaddb(const gchar *name, gchar **path);
static gboolean read_int(XrmDatabase db, const gchar *rname, gint *value);
static gboolean read_string(XrmDatabase db, const gchar *rname, gchar **value);
static gboolean read_color(XrmDatabase db, const RrInstance *inst,
                           const gchar *rname, RrColor **value);
static gboolean read_mask(const RrInstance *inst, const gchar *path,
                          const gchar *maskname, RrPixmapMask **value);
static gboolean read_appearance(XrmDatabase db, const RrInstance *inst,
                                const gchar *rname, RrAppearance *value,
                                gboolean allow_trans);
static int parse_inline_number(const char *p);
static RrPixel32* read_c_image(gint width, gint height, const guint8 *data);
static void set_default_appearance(RrAppearance *a);
static void read_button_styles(XrmDatabase db, const RrInstance *inst, 
                               gchar *path,
                               const RrTheme *theme, RrButton *btn, 
                               const gchar *btnname,
                               struct fallbacks *fbs,
                               guchar *normal_mask,
                               guchar *toggled_mask);

static RrFont *get_font(RrFont *target, RrFont **default_font,
                        const RrInstance *inst)
{
    if (target) {
        RrFontRef(target);
        return target;
    } else {
        /* Only load the default font once */
        if (*default_font) {
            RrFontRef(*default_font);
        } else {
            *default_font = RrFontOpenDefault(inst);
        }
        return *default_font;
    }
}

#define READ_INT(x_resstr, x_var, x_min, x_max, x_def) \
    if (!read_int(db, x_resstr, & x_var) || \
            x_var < x_min || x_var > x_max) \
        x_var = x_def;

#define READ_COLOR(x_resstr, x_var, x_def) \
    if (!read_color(db, inst, x_resstr, & x_var)) \
        x_var = x_def;

#define READ_COLOR_(x_res1, x_res2, x_var, x_def) \
    if (!read_color(db, inst, x_res1, & x_var) && \
        !read_color(db, inst, x_res2, & x_var)) \
        x_var = x_def;

#define READ_MASK_COPY(x_file, x_var, x_copysrc) \
    if (!read_mask(inst, path, x_file, & x_var)) \
        x_var = RrPixmapMaskCopy(x_copysrc);

#define READ_APPEARANCE(x_resstr, x_var, x_parrel) \
    if (!read_appearance(db, inst, x_resstr, x_var, x_parrel)) \
        set_default_appearance(x_var);

#define READ_APPEARANCE_COPY(x_resstr, x_var, x_parrel, x_defval) \
    if (!read_appearance(db, inst, x_resstr, x_var, x_parrel)) {\
        RrAppearanceFree(x_var); \
        x_var = RrAppearanceCopy(x_defval); }

#define READ_APPEARANCE_COPY_TEXTURES(x_resstr, x_var, x_parrel, x_defval, n_tex) \
    if (!read_appearance(db, inst, x_resstr, x_var, x_parrel)) {\
        RrAppearanceFree(x_var); \
        x_var = RrAppearanceCopy(x_defval); \
        RrAppearanceRemoveTextures(x_var); \
        RrAppearanceAddTextures(x_var, 5); }

#define READ_APPEARANCE_(x_res1, x_res2, x_var, x_parrel, x_defval) \
    if (!read_appearance(db, inst, x_res1, x_var, x_parrel) && \
        !read_appearance(db, inst, x_res2, x_var, x_parrel)) {\
        RrAppearanceFree(x_var); \
        x_var = RrAppearanceCopy(x_defval); }

RrTheme* RrThemeNew(const RrInstance *inst, const gchar *name,
                    gboolean allow_fallback,
                    RrFont *active_window_font, RrFont *inactive_window_font,
                    RrFont *menu_title_font, RrFont *menu_item_font,
                    RrFont *active_osd_font, RrFont *inactive_osd_font)
{
    XrmDatabase db = NULL;
    RrJustify winjust, mtitlejust;
    gchar *str;
    RrTheme *theme;
    RrFont *default_font = NULL;
    gchar *path;
    gint menu_overlap = 0;
    struct fallbacks fbs;

    if (name) {
        db = loaddb(name, &path);
        if (db == NULL) {
            g_message("Unable to load the theme '%s'", name);
            if (allow_fallback)
                g_message("Falling back to the default theme '%s'",
                          DEFAULT_THEME);
            /* fallback to the default theme */
            name = NULL;
        }
    }
    if (name == NULL) {
        if (allow_fallback) {
            db = loaddb(DEFAULT_THEME, &path);
            if (db == NULL) {
                g_message("Unable to load the theme '%s'", DEFAULT_THEME);
                return NULL;
            }
        } else
            return NULL;
    }

    /* initialize temp reading textures */
    fbs.focused_disabled = RrAppearanceNew(inst, 1);
    fbs.unfocused_disabled = RrAppearanceNew(inst, 1);
    fbs.focused_hover = RrAppearanceNew(inst, 1);
    fbs.unfocused_hover = RrAppearanceNew(inst, 1);
    fbs.focused_unpressed_toggled = RrAppearanceNew(inst, 1);
    fbs.unfocused_unpressed_toggled = RrAppearanceNew(inst, 1);
    fbs.focused_hover_toggled = RrAppearanceNew(inst, 1);
    fbs.unfocused_hover_toggled = RrAppearanceNew(inst, 1);
    fbs.focused_pressed_toggled = RrAppearanceNew(inst, 1);
    fbs.unfocused_pressed_toggled = RrAppearanceNew(inst, 1);
    fbs.focused_unpressed = RrAppearanceNew(inst, 1);
    fbs.focused_pressed = RrAppearanceNew(inst, 1);
    fbs.unfocused_unpressed = RrAppearanceNew(inst, 1);
    fbs.unfocused_pressed = RrAppearanceNew(inst, 1);

    /* initialize theme */
    theme = g_slice_new0(RrTheme);

    theme->inst = inst;
    theme->name = g_strdup(name ? name : DEFAULT_THEME);

    /* init buttons */
    theme->btn_max = RrButtonNew(inst);
    theme->btn_close = RrButtonNew(inst);
    theme->btn_desk = RrButtonNew(inst);
    theme->btn_shade = RrButtonNew(inst);
    theme->btn_iconify = RrButtonNew(inst);

    /* init appearances */
    theme->a_focused_grip = RrAppearanceNew(inst, 0);
    theme->a_unfocused_grip = RrAppearanceNew(inst, 0);
    theme->a_focused_title = RrAppearanceNew(inst, 0);
    theme->a_unfocused_title = RrAppearanceNew(inst, 0);
    theme->a_focused_label = RrAppearanceNew(inst, 1);
    theme->a_unfocused_label = RrAppearanceNew(inst, 1);
    theme->a_icon = RrAppearanceNew(inst, 1);
    theme->a_focused_handle = RrAppearanceNew(inst, 0);
    theme->a_unfocused_handle = RrAppearanceNew(inst, 0);
    theme->a_menu = RrAppearanceNew(inst, 0);
    theme->a_menu_title = RrAppearanceNew(inst, 0);
    theme->a_menu_text_title = RrAppearanceNew(inst, 1);
    theme->a_menu_normal = RrAppearanceNew(inst, 0);
    theme->a_menu_selected = RrAppearanceNew(inst, 0);
    theme->a_menu_disabled = RrAppearanceNew(inst, 0);
    /* a_menu_disabled_selected is copied from a_menu_selected */
    theme->a_menu_text_normal = RrAppearanceNew(inst, 1);
    theme->a_menu_text_selected = RrAppearanceNew(inst, 1);
    theme->a_menu_text_disabled = RrAppearanceNew(inst, 1);
    theme->a_menu_text_disabled_selected = RrAppearanceNew(inst, 1);
    theme->a_menu_bullet_normal = RrAppearanceNew(inst, 1);
    theme->a_menu_bullet_selected = RrAppearanceNew(inst, 1);
    theme->a_clear = RrAppearanceNew(inst, 0);
    theme->a_clear_tex = RrAppearanceNew(inst, 1);
    theme->osd_bg = RrAppearanceNew(inst, 0);
    theme->osd_hilite_label = RrAppearanceNew(inst, 1);
    theme->osd_hilite_bg = RrAppearanceNew(inst, 0);
    theme->osd_unhilite_label = RrAppearanceNew(inst, 1);
    theme->osd_unhilite_bg = RrAppearanceNew(inst, 0);
    theme->osd_unpressed_button = RrAppearanceNew(inst, 1);
    theme->osd_pressed_button = RrAppearanceNew(inst, 5);
    theme->osd_focused_button = RrAppearanceNew(inst, 5);

    /* load the font stuff */
    theme->win_font_focused = get_font(active_window_font,
                                       &default_font, inst);
    theme->win_font_unfocused = get_font(inactive_window_font,
                                         &default_font, inst);

    winjust = RR_JUSTIFY_LEFT;
    if (read_string(db, "window.label.text.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            winjust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            winjust = RR_JUSTIFY_CENTER;
    }

    theme->menu_title_font = get_font(menu_title_font, &default_font, inst);

    mtitlejust = RR_JUSTIFY_LEFT;
    if (read_string(db, "menu.title.text.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            mtitlejust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            mtitlejust = RR_JUSTIFY_CENTER;
    }

    theme->menu_font = get_font(menu_item_font, &default_font, inst);

    theme->osd_font_hilite = get_font(active_osd_font, &default_font, inst);
    theme->osd_font_unhilite = get_font(inactive_osd_font, &default_font,inst);

    /* load direct dimensions */
    READ_INT("menu.overlap", menu_overlap, -100, 100, 0);
    READ_INT("menu.overlap.x", theme->menu_overlap_x, -100, 100, menu_overlap);
    READ_INT("menu.overlap.y", theme->menu_overlap_y, -100, 100, menu_overlap);
    READ_INT("window.handle.width", theme->handle_height, 0, 100, 6);
    READ_INT("padding.width", theme->paddingx, 0, 100, 3);
    READ_INT("padding.height", theme->paddingy, 0, 100, theme->paddingx);
    READ_INT("border.width", theme->fbwidth, 0, 100, 1);
    READ_INT("menu.border.width", theme->mbwidth, 0, 100, theme->fbwidth);
    READ_INT("osd.border.width", theme->obwidth, 0, 100, theme->fbwidth);
    READ_INT("undecorated.border.width", theme->ubwidth, 0, 100, theme->fbwidth);
    READ_INT("menu.separator.width", theme->menu_sep_width, 1, 100, 1);
    READ_INT("menu.separator.padding.width", theme->menu_sep_paddingx, 0, 100, 6);
    READ_INT("menu.separator.padding.height", theme->menu_sep_paddingy, 0, 100, 3);
    READ_INT("window.client.padding.width", theme->cbwidthx, 0, 100, theme->paddingx);
    READ_INT("window.client.padding.height", theme->cbwidthy, 0, 100, theme->cbwidthx);
    /* room above the titlebar buttons reserved for the top-resize hover
       zone. kept small and theme-configurable; see frame.c's topresize
       and set_theme_statics() for how it drives button_height. */
    READ_INT("window.title.kgrip", theme->kgrip, 0, 2, 2);

    /* load colors */
    READ_COLOR_("window.active.border.color",
                "border.color",
                theme->frame_focused_border_color,
                RrColorNew(inst, 0, 0, 0));
    READ_COLOR("window.undecorated.active.border.color",
               theme->frame_undecorated_focused_border_color,
               RrColorCopy(theme->frame_focused_border_color));
    READ_COLOR("window.active.title.separator.color",
               theme->title_separator_focused_color,
               RrColorCopy(theme->frame_focused_border_color));

    READ_COLOR("window.inactive.border.color",
               theme->frame_unfocused_border_color,
               RrColorCopy(theme->frame_focused_border_color));

    READ_COLOR("window.undecorated.inactive.border.color",
               theme->frame_undecorated_unfocused_border_color,
               RrColorCopy(theme->frame_unfocused_border_color));

    READ_COLOR("window.inactive.title.separator.color",
               theme->title_separator_unfocused_color,
               RrColorCopy(theme->frame_unfocused_border_color));

    READ_COLOR("menu.border.color",
               theme->menu_border_color,
               RrColorCopy(theme->frame_focused_border_color));

    READ_COLOR("osd.border.color", 
               theme->osd_border_color,
               RrColorCopy(theme->frame_focused_border_color));

    READ_COLOR("window.active.client.color",
               theme->cb_focused_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("window.inactive.client.color",
               theme->cb_unfocused_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("window.active.label.text.color",
               theme->title_focused_color,
               RrColorNew(inst, 0x0, 0x0, 0x0));

    READ_COLOR("window.inactive.label.text.color",
               theme->title_unfocused_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR_("osd.active.label.text.color",
                "osd.label.text.color",
                theme->osd_text_active_color,
                RrColorCopy(theme->title_focused_color));

    READ_COLOR_("osd.inactive.label.text.color",
                "osd.label.text.color",
                theme->osd_text_inactive_color,
                RrColorCopy(theme->title_unfocused_color));

    READ_COLOR("window.active.button.unpressed.image.color",
               theme->titlebut_focused_unpressed_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("window.inactive.button.unpressed.image.color",
               theme->titlebut_unfocused_unpressed_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("window.active.button.pressed.image.color",
               theme->titlebut_focused_pressed_color,
               RrColorCopy(theme->titlebut_focused_unpressed_color));

    READ_COLOR("window.inactive.button.pressed.image.color",
               theme->titlebut_unfocused_pressed_color,
               RrColorCopy(theme->titlebut_unfocused_unpressed_color));

    READ_COLOR("window.active.button.disabled.image.color",
               theme->titlebut_focused_disabled_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("window.inactive.button.disabled.image.color",
               theme->titlebut_unfocused_disabled_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("window.active.button.hover.image.color",
               theme->titlebut_focused_hover_color,
               RrColorCopy(theme->titlebut_focused_unpressed_color));

    READ_COLOR("window.inactive.button.hover.image.color",
               theme->titlebut_unfocused_hover_color,
               RrColorCopy(theme->titlebut_unfocused_unpressed_color));

    READ_COLOR_("window.active.button.toggled.unpressed.image.color",
                "window.active.button.toggled.image.color",
                theme->titlebut_focused_unpressed_toggled_color,
                RrColorCopy(theme->titlebut_focused_pressed_color));

    READ_COLOR_("window.inactive.button.toggled.unpressed.image.color",
                "window.inactive.button.toggled.image.color",
                theme->titlebut_unfocused_unpressed_toggled_color,
                RrColorCopy(theme->titlebut_unfocused_pressed_color));

    READ_COLOR("window.active.button.toggled.hover.image.color",
               theme->titlebut_focused_hover_toggled_color,
               RrColorCopy(theme->titlebut_focused_unpressed_toggled_color));

    READ_COLOR("window.inactive.button.toggled.hover.image.color",
               theme->titlebut_unfocused_hover_toggled_color,
               RrColorCopy(theme->titlebut_unfocused_unpressed_toggled_color));

    READ_COLOR("window.active.button.toggled.pressed.image.color",
               theme->titlebut_focused_pressed_toggled_color,
               RrColorCopy(theme->titlebut_focused_pressed_color));

    READ_COLOR("window.inactive.button.toggled.pressed.image.color",
               theme->titlebut_unfocused_pressed_toggled_color,
               RrColorCopy(theme->titlebut_unfocused_pressed_color));

    READ_COLOR("menu.title.text.color",
               theme->menu_title_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("menu.items.text.color",
               theme->menu_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("menu.bullet.image.color",
               theme->menu_bullet_color,
               RrColorCopy(theme->menu_color));
   
    READ_COLOR("menu.items.disabled.text.color",
               theme->menu_disabled_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("menu.items.active.disabled.text.color",
               theme->menu_disabled_selected_color,
               RrColorCopy(theme->menu_disabled_color));

    READ_COLOR("menu.items.active.text.color",
               theme->menu_selected_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("menu.separator.color",
               theme->menu_sep_color,
               RrColorCopy(theme->menu_color));
    
    READ_COLOR("menu.bullet.selected.image.color", 
               theme->menu_bullet_selected_color,
               RrColorCopy(theme->menu_selected_color));

    READ_COLOR("osd.button.unpressed.text.color",
               theme->osd_unpressed_color,
               RrColorCopy(theme->osd_text_active_color));
    READ_COLOR("osd.button.pressed.text.color",
               theme->osd_pressed_color,
               RrColorCopy(theme->osd_text_active_color));
    READ_COLOR("osd.button.focused.text.color",
               theme->osd_focused_color,
               RrColorCopy(theme->osd_text_active_color));
    READ_COLOR("osd.button.pressed.box.color",
               theme->osd_pressed_lineart,
               RrColorCopy(theme->titlebut_focused_pressed_color));
    READ_COLOR("osd.button.focused.box.color",
               theme->osd_focused_lineart,
               RrColorCopy(theme->titlebut_focused_hover_color));
 
    /* load window buttons */

    /* bases: unpressed, pressed, disabled */
    READ_APPEARANCE("window.active.button.unpressed.bg", fbs.focused_unpressed, TRUE);
    READ_APPEARANCE("window.inactive.button.unpressed.bg", fbs.unfocused_unpressed, TRUE);
    READ_APPEARANCE("window.active.button.pressed.bg", fbs.focused_pressed, TRUE);
    READ_APPEARANCE("window.inactive.button.pressed.bg", fbs.unfocused_pressed, TRUE);
    READ_APPEARANCE("window.active.button.disabled.bg", fbs.focused_disabled, TRUE);
    READ_APPEARANCE("window.inactive.button.disabled.bg", fbs.unfocused_disabled, TRUE);

    /* hover */
    READ_APPEARANCE_COPY("window.active.button.hover.bg",
                         fbs.focused_hover, TRUE,
                         fbs.focused_unpressed);
    READ_APPEARANCE_COPY("window.inactive.button.hover.bg",
                         fbs.unfocused_hover, TRUE,
                         fbs.unfocused_unpressed);

    /* toggled unpressed */
    READ_APPEARANCE_("window.active.button.toggled.unpressed.bg",
                     "window.active.button.toggled.bg",
                     fbs.focused_unpressed_toggled, TRUE,
                     fbs.focused_pressed);
    READ_APPEARANCE_("window.inactive.button.toggled.unpressed.bg",
                     "window.inactive.button.toggled.bg",
                     fbs.unfocused_unpressed_toggled, TRUE,
                     fbs.unfocused_pressed);

    /* toggled pressed */
    READ_APPEARANCE_COPY("window.active.button.toggled.pressed.bg",
                         fbs.focused_pressed_toggled, TRUE,
                         fbs.focused_pressed);
    READ_APPEARANCE_COPY("window.inactive.button.toggled.pressed.bg",
                         fbs.unfocused_pressed_toggled, TRUE,
                         fbs.unfocused_pressed);

    /* toggled hover */
    READ_APPEARANCE_COPY("window.active.button.toggled.hover.bg",
                         fbs.focused_hover_toggled, TRUE,
                         fbs.focused_unpressed_toggled);
    READ_APPEARANCE_COPY("window.inactive.button.toggled.hover.bg",
                         fbs.unfocused_hover_toggled, TRUE,
                         fbs.unfocused_unpressed_toggled);


    /* now do individual buttons, if specified */

    /* max button */
    {
        guchar normal_mask[] =  { 0x3f, 0x3f, 0x21, 0x21, 0x21, 0x3f };
        guchar toggled_mask[] = { 0x3e, 0x22, 0x2f, 0x29, 0x39, 0x0f };
        read_button_styles(db, inst, path, theme, theme->btn_max, "max",
                           &fbs, normal_mask, toggled_mask);
    }

    /* close button */
    {
        guchar normal_mask[] = { 0x33, 0x3f, 0x1e, 0x1e, 0x3f, 0x33 };
        read_button_styles(db, inst, path, theme, theme->btn_close, "close",
                           &fbs, normal_mask, NULL);
    }

    /* all desktops button */
    {
        guchar normal_mask[] =  { 0x33, 0x33, 0x00, 0x00, 0x33, 0x33 };
        guchar toggled_mask[] = { 0x00, 0x1e, 0x1a, 0x16, 0x1e, 0x00 };
        read_button_styles(db, inst, path, theme, theme->btn_desk, "desk",
                           &fbs, normal_mask, toggled_mask);
    }

    /* shade button */
    {
        guchar normal_mask[] = { 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00 };
        read_button_styles(db, inst, path, theme, theme->btn_shade, "shade",
                           &fbs, normal_mask, normal_mask);
    }

    /* iconify button */
    {
        guchar normal_mask[] = { 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f };
        read_button_styles(db, inst, path, theme, theme->btn_iconify, "iconify",
                           &fbs, normal_mask, NULL);
    }

    /* submenu bullet mask */
    if (!read_mask(inst, path, "bullet.xbm", &theme->menu_bullet_mask))
    {
        guchar data[] = { 0x01, 0x03, 0x07, 0x0f, 0x07, 0x03, 0x01 };
        theme->menu_bullet_mask = RrPixmapMaskNew(inst, 4, 7, (gchar*)data);
    }

    /* up and down arrows */
    {
        guchar data[] = { 0xfe, 0x00, 0x7c, 0x00, 0x38, 0x00, 0x10, 0x00 };
        theme->down_arrow_mask = RrPixmapMaskNew(inst, 9, 4, (gchar*)data);
    }
    {
        guchar data[] = { 0x10, 0x00, 0x38, 0x00, 0x7c, 0x00, 0xfe, 0x00 };
        theme->up_arrow_mask = RrPixmapMaskNew(inst, 9, 4, (gchar*)data);
    }

    /* setup the default window icon */
    theme->def_win_icon = read_c_image(OB_DEFAULT_ICON_WIDTH,
                                       OB_DEFAULT_ICON_HEIGHT,
                                       OB_DEFAULT_ICON_pixel_data);
    theme->def_win_icon_w = OB_DEFAULT_ICON_WIDTH;
    theme->def_win_icon_h = OB_DEFAULT_ICON_HEIGHT;

    /* read the decoration textures */
    READ_APPEARANCE("window.active.title.bg", theme->a_focused_title, FALSE);
    READ_APPEARANCE("window.inactive.title.bg", theme->a_unfocused_title, FALSE);
    READ_APPEARANCE("window.active.label.bg", theme->a_focused_label, TRUE);
    READ_APPEARANCE("window.inactive.label.bg", theme->a_unfocused_label, TRUE);
    READ_APPEARANCE("window.active.handle.bg", theme->a_focused_handle, FALSE);
    READ_APPEARANCE("window.inactive.handle.bg",theme->a_unfocused_handle, FALSE);
    READ_APPEARANCE("window.active.grip.bg", theme->a_focused_grip, TRUE);
    READ_APPEARANCE("window.inactive.grip.bg", theme->a_unfocused_grip, TRUE);
    READ_APPEARANCE("menu.items.bg", theme->a_menu, FALSE);
    READ_APPEARANCE("menu.title.bg", theme->a_menu_title, TRUE);
    READ_APPEARANCE("menu.items.active.bg", theme->a_menu_selected, TRUE);

    theme->a_menu_disabled_selected =
        RrAppearanceCopy(theme->a_menu_selected);

    /* read appearances for non-decorations (on-screen-display) */
    if (!read_appearance(db, inst, "osd.bg", theme->osd_bg, FALSE))
    {
        RrAppearanceFree(theme->osd_bg);
        theme->osd_bg = RrAppearanceCopy(theme->a_focused_title);
    }
    if (!read_appearance(db, inst, "osd.active.label.bg",
                         theme->osd_hilite_label, TRUE) &&
        !read_appearance(db, inst, "osd.label.bg",
                         theme->osd_hilite_label, TRUE))
    {
        RrAppearanceFree(theme->osd_hilite_label);
        theme->osd_hilite_label = RrAppearanceCopy(theme->a_focused_label);
    }
    if (!read_appearance(db, inst, "osd.inactive.label.bg",
                         theme->osd_unhilite_label, TRUE))
    {
        RrAppearanceFree(theme->osd_unhilite_label);
        theme->osd_unhilite_label = RrAppearanceCopy(theme->a_unfocused_label);
    }
    /* osd_hilite_fg can't be parentrel */
    if (!read_appearance(db, inst, "osd.hilight.bg",
                         theme->osd_hilite_bg, FALSE))
    {
        RrAppearanceFree(theme->osd_hilite_bg);
        if (theme->a_focused_label->surface.grad != RR_SURFACE_PARENTREL)
            theme->osd_hilite_bg = RrAppearanceCopy(theme->a_focused_label);
        else
            theme->osd_hilite_bg = RrAppearanceCopy(theme->a_focused_title);
    }
    /* osd_unhilite_fg can't be parentrel either */
    if (!read_appearance(db, inst, "osd.unhilight.bg",
                         theme->osd_unhilite_bg, FALSE))
    {
        RrAppearanceFree(theme->osd_unhilite_bg);
        if (theme->a_unfocused_label->surface.grad != RR_SURFACE_PARENTREL)
            theme->osd_unhilite_bg=RrAppearanceCopy(theme->a_unfocused_label);
        else
            theme->osd_unhilite_bg=RrAppearanceCopy(theme->a_unfocused_title);
    }

    /* osd buttons */
    READ_APPEARANCE_COPY("osd.button.unpressed.bg", theme->osd_unpressed_button, TRUE, fbs.focused_unpressed);
    READ_APPEARANCE_COPY_TEXTURES("osd.button.pressed.bg", theme->osd_pressed_button, TRUE, fbs.focused_pressed, 5);
    READ_APPEARANCE_COPY_TEXTURES("osd.button.focused.bg", theme->osd_focused_button, TRUE, fbs.focused_unpressed, 5);

    theme->a_icon->surface.grad =
        theme->a_clear->surface.grad =
        theme->a_clear_tex->surface.grad =
        theme->a_menu_text_title->surface.grad =
        theme->a_menu_normal->surface.grad =
        theme->a_menu_disabled->surface.grad =
        theme->a_menu_text_normal->surface.grad =
        theme->a_menu_text_selected->surface.grad =
        theme->a_menu_text_disabled->surface.grad =
        theme->a_menu_text_disabled_selected->surface.grad =
        theme->a_menu_bullet_normal->surface.grad =
        theme->a_menu_bullet_selected->surface.grad = RR_SURFACE_PARENTREL;

    /* set up the textures */
    theme->a_focused_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_focused_label->texture[0].data.text.justify = winjust;
    theme->a_focused_label->texture[0].data.text.font=theme->win_font_focused;
    theme->a_focused_label->texture[0].data.text.color =
        theme->title_focused_color;

    if (read_string(db, "window.active.label.text.font", &str)) {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->a_focused_label->texture[0].data.text.shadow_offset_x = i;
            theme->a_focused_label->texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint="))) {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->title_focused_shadow_color = RrColorNew(inst, j, j, j);
            theme->title_focused_shadow_alpha = i;
        } else {
            theme->title_focused_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->title_focused_shadow_alpha = 50;
        }
    }

    theme->a_focused_label->texture[0].data.text.shadow_color = theme->title_focused_shadow_color;
    theme->a_focused_label->texture[0].data.text.shadow_alpha = theme->title_focused_shadow_alpha;

    theme->osd_hilite_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->osd_hilite_label->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    theme->osd_hilite_label->texture[0].data.text.font = theme->osd_font_hilite;
    theme->osd_hilite_label->texture[0].data.text.color = theme->osd_text_active_color;

    if (read_string(db, "osd.active.label.text.font", &str) ||
        read_string(db, "osd.label.text.font", &str))
    {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->osd_hilite_label->texture[0].data.text.shadow_offset_x = i;
            theme->osd_hilite_label->texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint="))) {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->osd_text_active_shadow_color = RrColorNew(inst, j, j, j);
            theme->osd_text_active_shadow_alpha = i;
        } else {
            theme->osd_text_active_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->osd_text_active_shadow_alpha = 50;
        }
    } else {
        /* inherit the font settings from the focused label */
        theme->osd_hilite_label->texture[0].data.text.shadow_offset_x =
            theme->a_focused_label->texture[0].data.text.shadow_offset_x;
        theme->osd_hilite_label->texture[0].data.text.shadow_offset_y =
            theme->a_focused_label->texture[0].data.text.shadow_offset_y;
        if (theme->title_focused_shadow_color)
            theme->osd_text_active_shadow_color =
                RrColorCopy(theme->title_focused_shadow_color);
        else
            theme->osd_text_active_shadow_color = RrColorNew(inst, 0, 0, 0);
        theme->osd_text_active_shadow_alpha =
            theme->title_focused_shadow_alpha;
    }

    theme->osd_hilite_label->texture[0].data.text.shadow_color =
        theme->osd_text_active_shadow_color;
    theme->osd_hilite_label->texture[0].data.text.shadow_alpha =
        theme->osd_text_active_shadow_alpha;

    theme->osd_unpressed_button->texture[0].type =
        theme->osd_pressed_button->texture[0].type =
        theme->osd_focused_button->texture[0].type =
        RR_TEXTURE_TEXT;

    theme->osd_unpressed_button->texture[0].data.text.justify =
        theme->osd_pressed_button->texture[0].data.text.justify =
        theme->osd_focused_button->texture[0].data.text.justify =
        RR_JUSTIFY_CENTER;

    theme->osd_unpressed_button->texture[0].data.text.font =
        theme->osd_pressed_button->texture[0].data.text.font =
        theme->osd_focused_button->texture[0].data.text.font =
        theme->osd_font_hilite;

    theme->osd_unpressed_button->texture[0].data.text.color =
        theme->osd_unpressed_color;
    theme->osd_pressed_button->texture[0].data.text.color =
        theme->osd_pressed_color;
    theme->osd_focused_button->texture[0].data.text.color =
        theme->osd_focused_color;

    theme->osd_pressed_button->texture[1].data.lineart.color =
        theme->osd_pressed_button->texture[2].data.lineart.color =
        theme->osd_pressed_button->texture[3].data.lineart.color =
        theme->osd_pressed_button->texture[4].data.lineart.color =
        theme->osd_pressed_lineart;

    theme->osd_focused_button->texture[1].data.lineart.color =
        theme->osd_focused_button->texture[2].data.lineart.color =
        theme->osd_focused_button->texture[3].data.lineart.color =
        theme->osd_focused_button->texture[4].data.lineart.color =
        theme->osd_focused_lineart;

    theme->a_unfocused_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_unfocused_label->texture[0].data.text.justify = winjust;
    theme->a_unfocused_label->texture[0].data.text.font = theme->win_font_unfocused;
    theme->a_unfocused_label->texture[0].data.text.color = theme->title_unfocused_color;

    if (read_string(db, "window.inactive.label.text.font", &str)) {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->a_unfocused_label->texture[0].data.text.shadow_offset_x = i;
            theme->a_unfocused_label->texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint="))) {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->title_unfocused_shadow_color = RrColorNew(inst, j, j, j);
            theme->title_unfocused_shadow_alpha = i;
        } else {
            theme->title_unfocused_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->title_unfocused_shadow_alpha = 50;
        }
    }

    theme->a_unfocused_label->texture[0].data.text.shadow_color =
        theme->title_unfocused_shadow_color;
    theme->a_unfocused_label->texture[0].data.text.shadow_alpha =
        theme->title_unfocused_shadow_alpha;

    theme->osd_unhilite_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->osd_unhilite_label->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    theme->osd_unhilite_label->texture[0].data.text.font =
        theme->osd_font_unhilite;
    theme->osd_unhilite_label->texture[0].data.text.color =
        theme->osd_text_inactive_color;

    if (read_string(db, "osd.inactive.label.text.font", &str)) {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->osd_unhilite_label->texture[0].data.text.shadow_offset_x=i;
            theme->osd_unhilite_label->texture[0].data.text.shadow_offset_y=i;
        }
        if ((p = strstr(str, "shadowtint="))) {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->osd_text_inactive_shadow_color = RrColorNew(inst, j, j, j);
            theme->osd_text_inactive_shadow_alpha = i;
        } else {
            theme->osd_text_inactive_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->osd_text_inactive_shadow_alpha = 50;
        }
    } else {
        /* inherit the font settings from the unfocused label */
        theme->osd_unhilite_label->texture[0].data.text.shadow_offset_x =
            theme->a_unfocused_label->texture[0].data.text.shadow_offset_x;
        theme->osd_unhilite_label->texture[0].data.text.shadow_offset_y =
            theme->a_unfocused_label->texture[0].data.text.shadow_offset_y;
        if (theme->title_unfocused_shadow_color)
            theme->osd_text_inactive_shadow_color =
                RrColorCopy(theme->title_unfocused_shadow_color);
        else
            theme->osd_text_inactive_shadow_color = RrColorNew(inst, 0, 0, 0);
        theme->osd_text_inactive_shadow_alpha =
            theme->title_unfocused_shadow_alpha;
    }

    theme->osd_unhilite_label->texture[0].data.text.shadow_color =
        theme->osd_text_inactive_shadow_color;
    theme->osd_unhilite_label->texture[0].data.text.shadow_alpha =
        theme->osd_text_inactive_shadow_alpha;

    theme->a_menu_text_title->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_menu_text_title->texture[0].data.text.justify = mtitlejust;
    theme->a_menu_text_title->texture[0].data.text.font = theme->menu_title_font;
    theme->a_menu_text_title->texture[0].data.text.color = theme->menu_title_color;

    if (read_string(db, "menu.title.text.font", &str)) {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->a_menu_text_title->texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_title->texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint="))) {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->menu_title_shadow_color = RrColorNew(inst, j, j, j);
        } else {
            theme->menu_title_shadow_color = RrColorNew(inst, 0, 0, 0);
            i = 50;
        }

        theme->a_menu_text_title->texture[0].data.text.shadow_color =
            theme->menu_title_shadow_color;
        theme->a_menu_text_title->texture[0].data.text.shadow_alpha =
            i;
    }

    theme->a_menu_text_normal->texture[0].type =
        theme->a_menu_text_selected->texture[0].type =
        theme->a_menu_text_disabled->texture[0].type =
        theme->a_menu_text_disabled_selected->texture[0].type =
        RR_TEXTURE_TEXT;
    theme->a_menu_text_normal->texture[0].data.text.justify =
        theme->a_menu_text_selected->texture[0].data.text.justify =
        theme->a_menu_text_disabled->texture[0].data.text.justify =
        theme->a_menu_text_disabled_selected->texture[0].data.text.justify =
        RR_JUSTIFY_LEFT;
    theme->a_menu_text_normal->texture[0].data.text.font =
        theme->a_menu_text_selected->texture[0].data.text.font =
        theme->a_menu_text_disabled->texture[0].data.text.font =
        theme->a_menu_text_disabled_selected->texture[0].data.text.font =
        theme->menu_font;
    theme->a_menu_text_normal->texture[0].data.text.color = theme->menu_color;
    theme->a_menu_text_selected->texture[0].data.text.color =
        theme->menu_selected_color;
    theme->a_menu_text_disabled->texture[0].data.text.color =
        theme->menu_disabled_color;
    theme->a_menu_text_disabled_selected->texture[0].data.text.color =
        theme->menu_disabled_selected_color;

    if (read_string(db, "menu.items.font", &str)) {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->a_menu_text_normal->
                texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_normal->
                texture[0].data.text.shadow_offset_y = i;
            theme->a_menu_text_selected->
                texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_selected->
                texture[0].data.text.shadow_offset_y = i;
            theme->a_menu_text_disabled->
                texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_disabled->
                texture[0].data.text.shadow_offset_y = i;
            theme->a_menu_text_disabled_selected->
                texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_disabled_selected->
                texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint="))) {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->menu_text_shadow_color = RrColorNew(inst, j, j, j);
        } else {
            theme->menu_text_shadow_color = RrColorNew(inst, 0, 0, 0);
            i = 50;
        }

        theme->a_menu_text_normal->texture[0].data.text.shadow_color =
            theme->a_menu_text_selected->texture[0].data.text.shadow_color =
            theme->a_menu_text_disabled->texture[0].data.text.shadow_color =
            theme->a_menu_text_disabled_selected->texture[0].data.text.shadow_color =
            theme->menu_text_shadow_color;

        theme->a_menu_text_normal->texture[0].data.text.shadow_alpha =
            theme->a_menu_text_selected->texture[0].data.text.shadow_alpha =
            theme->a_menu_text_disabled->texture[0].data.text.shadow_alpha =
            theme->a_menu_text_disabled_selected->texture[0].data.text.shadow_alpha =
            i;
    }

    theme->a_menu_bullet_normal->texture[0].type =
        theme->a_menu_bullet_selected->texture[0].type = RR_TEXTURE_MASK;
    theme->a_menu_bullet_normal->texture[0].data.mask.mask =
    theme->a_menu_bullet_selected->texture[0].data.mask.mask =
        theme->menu_bullet_mask;
    theme->a_menu_bullet_normal->texture[0].data.mask.color =
        theme->menu_bullet_color;
    theme->a_menu_bullet_selected->texture[0].data.mask.color =
        theme->menu_bullet_selected_color;

    g_free(path);
    XrmDestroyDatabase(db);

    /* set the font heights */
    theme->win_font_height = RrFontHeight(theme->win_font_focused,
        theme->a_focused_label->texture[0].data.text.shadow_offset_y);
    theme->win_font_height =
        MAX(theme->win_font_height,
            RrFontHeight(theme->win_font_focused,
                theme->a_unfocused_label->texture[0].data.text.shadow_offset_y));
    theme->menu_title_font_height = RrFontHeight(theme->menu_title_font,
        theme->a_menu_text_title->texture[0].data.text.shadow_offset_y);
    theme->menu_font_height = RrFontHeight(theme->menu_font,
        theme->a_menu_text_normal->texture[0].data.text.shadow_offset_y);

    /* calculate some last extents */
    {
        gint ft, fb, fl, fr, ut, ub, ul, ur;

        RrMargins(theme->a_focused_label, &fl, &ft, &fr, &fb);
        RrMargins(theme->a_unfocused_label, &ul, &ut, &ur, &ub);
        theme->label_height = theme->win_font_height + MAX(ft + fb, ut + ub);
        theme->label_height += theme->label_height % 2;

        /* this would be nice I think, since padding.width can now be 0,
           but it breaks frame.c horribly and I don't feel like fixing that
           right now, so if anyone complains, here is how to keep text from
           going over the title's bevel/border with a padding.width of 0 and a
           bevelless/borderless label
           RrMargins(theme->a_focused_title, &fl, &ft, &fr, &fb);
           RrMargins(theme->a_unfocused_title, &ul, &ut, &ur, &ub);
           theme->title_height = theme->label_height +
           MAX(MAX(theme->padding * 2, ft + fb),
           MAX(theme->padding * 2, ut + ub));
        */
        theme->title_height = theme->label_height + theme->paddingy * 2;

        RrMargins(theme->a_menu_title, &ul, &ut, &ur, &ub);
        theme->menu_title_label_height = theme->menu_title_font_height+ut+ub;
        theme->menu_title_height = theme->menu_title_label_height +
            theme->paddingy * 2;
    }
    /* button_height is derived from the titlebar height, leaving kgrip
       pixels of margin above and below the button (the margin above
       doubles as the top-resize hover strip, see frame.c's topresize).
       The MAX(1, ...) floor guards against a 0/negative XResizeWindow
       call on themes with a very small paddingy combined with a large
       kgrip. */
    theme->button_height = MAX(1, theme->title_height - 2 * theme->kgrip);
    theme->button_width = (gint)(theme->button_height * WISTOH + 0.5);
    /* ABI COMPATIBILITY: button_size no longer drives any of our own
       geometry (frame.c/framerender.c use button_height/button_width
       exclusively now), but the field itself is kept alive at its
       original offset in the struct -- see the long comment on
       button_size in theme.h for why that matters. We still assign it
       a real value here, rather than leaving it at 0/uninitialized,
       so that external programs still compiled against the stock
       RrTheme layout (obconf's theme-preview button being the known
       case) get a sane, if approximate, single square dimension
       instead of garbage. button_height is the closer analogue of the
       two new dimensions to what button_size used to mean. */
    theme->button_size = theme->button_height;
    theme->grip_width = 25;

    /* Now that button_height is known, upgrade the default (non-theme-
       provided) button glyphs to the largest native-resolution tier
       that still fits -- see pick_glyph_tier and the tier table
       above. pick_glyph_tier returns NULL if button_height is below
       even the smallest tier, in which case we leave the classic 6x6
       defaults (already loaded by read_button_styles() above) alone. */
    {
        const ButtonGlyphTier *gt = pick_glyph_tier(theme->button_height);
        if (gt) {
            upsize_button_masks(inst, theme->btn_max,
                                gt->max_normal, gt->max_toggled,
                                gt->size, gt->size);
            upsize_button_masks(inst, theme->btn_close,
                                gt->close, NULL, gt->size, gt->size);
            upsize_button_masks(inst, theme->btn_desk,
                                gt->desk_normal, gt->desk_toggled,
                                gt->size, gt->size);
            /* shade's toggled state reuses the same shape as its
               unpressed state (see the original 6x6 read_button_styles
               call a bit above, which passes normal_mask for both
               parameters) -- pass gt->shade for both here too, or the
               toggled/shaded-window icon would be left stuck at 6x6
               while the unpressed one upgrades. */
            upsize_button_masks(inst, theme->btn_shade,
                                gt->shade, gt->shade, gt->size, gt->size);
            upsize_button_masks(inst, theme->btn_iconify,
                                gt->iconify, NULL, gt->size, gt->size);
        }
    }

    RrAppearanceFree(fbs.focused_disabled);
    RrAppearanceFree(fbs.unfocused_disabled);
    RrAppearanceFree(fbs.focused_hover);
    RrAppearanceFree(fbs.unfocused_hover);
    RrAppearanceFree(fbs.focused_unpressed);
    RrAppearanceFree(fbs.focused_pressed);
    RrAppearanceFree(fbs.unfocused_unpressed);
    RrAppearanceFree(fbs.unfocused_pressed);
    RrAppearanceFree(fbs.focused_hover_toggled);
    RrAppearanceFree(fbs.unfocused_hover_toggled);
    RrAppearanceFree(fbs.focused_unpressed_toggled);
    RrAppearanceFree(fbs.focused_pressed_toggled);
    RrAppearanceFree(fbs.unfocused_unpressed_toggled);
    RrAppearanceFree(fbs.unfocused_pressed_toggled);

    return theme;
}

void RrThemeFree(RrTheme *theme)
{
    if (theme) {
        g_free(theme->name);

        RrButtonFree(theme->btn_max);
        RrButtonFree(theme->btn_close);
        RrButtonFree(theme->btn_desk);
        RrButtonFree(theme->btn_shade);
        RrButtonFree(theme->btn_iconify);

        RrColorFree(theme->menu_border_color);
        RrColorFree(theme->osd_border_color);
        RrColorFree(theme->frame_focused_border_color);
        RrColorFree(theme->frame_undecorated_focused_border_color);
        RrColorFree(theme->frame_unfocused_border_color);
        RrColorFree(theme->frame_undecorated_unfocused_border_color);
        RrColorFree(theme->title_separator_focused_color);
        RrColorFree(theme->title_separator_unfocused_color);
        RrColorFree(theme->cb_unfocused_color);
        RrColorFree(theme->cb_focused_color);
        RrColorFree(theme->title_focused_color);
        RrColorFree(theme->title_unfocused_color);
        RrColorFree(theme->titlebut_focused_disabled_color);
        RrColorFree(theme->titlebut_unfocused_disabled_color);
        RrColorFree(theme->titlebut_focused_hover_color);
        RrColorFree(theme->titlebut_unfocused_hover_color);
        RrColorFree(theme->titlebut_focused_hover_toggled_color);
        RrColorFree(theme->titlebut_unfocused_hover_toggled_color);
        RrColorFree(theme->titlebut_focused_pressed_toggled_color);
        RrColorFree(theme->titlebut_unfocused_pressed_toggled_color);
        RrColorFree(theme->titlebut_focused_unpressed_toggled_color);
        RrColorFree(theme->titlebut_unfocused_unpressed_toggled_color);
        RrColorFree(theme->titlebut_focused_pressed_color);
        RrColorFree(theme->titlebut_unfocused_pressed_color);
        RrColorFree(theme->titlebut_focused_unpressed_color);
        RrColorFree(theme->titlebut_unfocused_unpressed_color);
        RrColorFree(theme->menu_title_color);
        RrColorFree(theme->menu_sep_color);
        RrColorFree(theme->menu_color);
        RrColorFree(theme->menu_bullet_color);
        RrColorFree(theme->menu_bullet_selected_color);
        RrColorFree(theme->menu_selected_color);
        RrColorFree(theme->menu_disabled_color);
        RrColorFree(theme->menu_disabled_selected_color);
        RrColorFree(theme->title_focused_shadow_color);
        RrColorFree(theme->title_unfocused_shadow_color);
        RrColorFree(theme->osd_text_active_color);
        RrColorFree(theme->osd_text_inactive_color);
        RrColorFree(theme->osd_text_active_shadow_color);
        RrColorFree(theme->osd_text_inactive_shadow_color);
        RrColorFree(theme->osd_pressed_color);
        RrColorFree(theme->osd_unpressed_color);
        RrColorFree(theme->osd_focused_color);
        RrColorFree(theme->osd_pressed_lineart);
        RrColorFree(theme->osd_focused_lineart);
        RrColorFree(theme->menu_title_shadow_color);
        RrColorFree(theme->menu_text_shadow_color);

        g_free(theme->def_win_icon);
        
        RrPixmapMaskFree(theme->menu_bullet_mask);
        RrPixmapMaskFree(theme->down_arrow_mask);
        RrPixmapMaskFree(theme->up_arrow_mask);

        RrFontClose(theme->win_font_focused);
        RrFontClose(theme->win_font_unfocused);
        RrFontClose(theme->menu_title_font);
        RrFontClose(theme->menu_font);
        RrFontClose(theme->osd_font_hilite);
        RrFontClose(theme->osd_font_unhilite);

        RrAppearanceFree(theme->a_focused_grip);
        RrAppearanceFree(theme->a_unfocused_grip);
        RrAppearanceFree(theme->a_focused_title);
        RrAppearanceFree(theme->a_unfocused_title);
        RrAppearanceFree(theme->a_focused_label);
        RrAppearanceFree(theme->a_unfocused_label);
        RrAppearanceFree(theme->a_icon);
        RrAppearanceFree(theme->a_focused_handle);
        RrAppearanceFree(theme->a_unfocused_handle);
        RrAppearanceFree(theme->a_menu);
        RrAppearanceFree(theme->a_menu_title);
        RrAppearanceFree(theme->a_menu_text_title);
        RrAppearanceFree(theme->a_menu_normal);
        RrAppearanceFree(theme->a_menu_selected);
        RrAppearanceFree(theme->a_menu_disabled);
        RrAppearanceFree(theme->a_menu_disabled_selected);
        RrAppearanceFree(theme->a_menu_text_normal);
        RrAppearanceFree(theme->a_menu_text_selected);
        RrAppearanceFree(theme->a_menu_text_disabled);
        RrAppearanceFree(theme->a_menu_text_disabled_selected);
        RrAppearanceFree(theme->a_menu_bullet_normal);
        RrAppearanceFree(theme->a_menu_bullet_selected);
        RrAppearanceFree(theme->a_clear);
        RrAppearanceFree(theme->a_clear_tex);
        RrAppearanceFree(theme->osd_bg);
        RrAppearanceFree(theme->osd_hilite_bg);
        RrAppearanceFree(theme->osd_hilite_label);
        RrAppearanceFree(theme->osd_unhilite_bg);
        RrAppearanceFree(theme->osd_unhilite_label);
        RrAppearanceFree(theme->osd_pressed_button);
        RrAppearanceFree(theme->osd_unpressed_button);
        RrAppearanceFree(theme->osd_focused_button);

        g_slice_free(RrTheme, theme);
    }
}

static XrmDatabase loaddb(const gchar *name, gchar **path)
{
    GSList *it;
    XrmDatabase db = NULL;
    gchar *s;

    if (name[0] == '/') {
        s = g_build_filename(name, "openbox-3", "themerc", NULL);
        if ((db = XrmGetFileDatabase(s)))
            *path = g_path_get_dirname(s);
        g_free(s);
    } else {
        ObtPaths *p;

        p = obt_paths_new();

        /* XXX backwards compatibility, remove me sometime later */
        s = g_build_filename(g_get_home_dir(), ".themes", name,
                             "openbox-3", "themerc", NULL);
        if ((db = XrmGetFileDatabase(s)))
            *path = g_path_get_dirname(s);
        g_free(s);

        for (it = obt_paths_data_dirs(p); !db && it; it = g_slist_next(it))
        {
            s = g_build_filename(it->data, "themes", name,
                                 "openbox-3", "themerc", NULL);
            if ((db = XrmGetFileDatabase(s)))
                *path = g_path_get_dirname(s);
            g_free(s);
        }

        obt_paths_unref(p);
    }

    if (db == NULL) {
        s = g_build_filename(name, "themerc", NULL);
        if ((db = XrmGetFileDatabase(s)))
            *path = g_path_get_dirname(s);
        g_free(s);
    }

    return db;
}

static gchar *create_class_name(const gchar *rname)
{
    gchar *rclass = g_strdup(rname);
    gchar *p = rclass;

    while (TRUE) {
        *p = toupper(*p);
        p = strchr(p+1, '.');
        if (p == NULL) break;
        ++p;
        if (*p == '\0') break;
    }
    return rclass;
}

static gboolean read_int(XrmDatabase db, const gchar *rname, gint *value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype, *end;
    XrmValue retvalue;

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        *value = (gint)strtol(retvalue.addr, &end, 10);
        if (end != retvalue.addr)
            ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_string(XrmDatabase db, const gchar *rname, gchar **value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype;
    XrmValue retvalue;

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        g_strstrip(retvalue.addr);
        *value = retvalue.addr;
        ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_color(XrmDatabase db, const RrInstance *inst,
                           const gchar *rname, RrColor **value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype;
    XrmValue retvalue;

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        RrColor *c;

        /* retvalue.addr is inside the xrdb database so we can't destroy it
           but we can edit it in place, as g_strstrip does. */
        g_strstrip(retvalue.addr);
        c = RrColorParse(inst, retvalue.addr);
        if (c != NULL) {
            *value = c;
            ret = TRUE;
        }
    }

    g_free(rclass);
    return ret;
}

static gboolean read_mask(const RrInstance *inst, const gchar *path,
                          const gchar *maskname, RrPixmapMask **value)
{
    gboolean ret = FALSE;
    gchar *s;
    gint hx, hy; /* ignored */
    guint w, h;
    guchar *b;

    s = g_build_filename(path, maskname, NULL);
    if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) == BitmapSuccess) {
        ret = TRUE;
        *value = RrPixmapMaskNew(inst, w, h, (gchar*)b);
        XFree(b);
    }
    g_free(s);

    return ret;
}

static void parse_appearance(gchar *tex, RrSurfaceColorType *grad,
                             RrReliefType *relief, RrBevelType *bevel,
                             gboolean *interlaced, gboolean *border,
                             gboolean allow_trans)
{
    gchar *t;

    /* convert to all lowercase */
    for (t = tex; *t != '\0'; ++t)
        *t = g_ascii_tolower(*t);

    if (allow_trans && strstr(tex, "parentrelative") != NULL) {
        *grad = RR_SURFACE_PARENTREL;
    } else {
        if (strstr(tex, "gradient") != NULL) {
            if (strstr(tex, "crossdiagonal") != NULL)
                *grad = RR_SURFACE_CROSS_DIAGONAL;
            else if (strstr(tex, "pyramid") != NULL)
                *grad = RR_SURFACE_PYRAMID;
            else if (strstr(tex, "mirrorhorizontal") != NULL)
                *grad = RR_SURFACE_MIRROR_HORIZONTAL;
            else if (strstr(tex, "horizontal") != NULL)
                *grad = RR_SURFACE_HORIZONTAL;
            else if (strstr(tex, "splitvertical") != NULL)
                *grad = RR_SURFACE_SPLIT_VERTICAL;
            else if (strstr(tex, "vertical") != NULL)
                *grad = RR_SURFACE_VERTICAL;
            else
                *grad = RR_SURFACE_DIAGONAL;
        } else {
            *grad = RR_SURFACE_SOLID;
        }
    }

    if (strstr(tex, "sunken") != NULL)
        *relief = RR_RELIEF_SUNKEN;
    else if (strstr(tex, "flat") != NULL)
        *relief = RR_RELIEF_FLAT;
    else if (strstr(tex, "raised") != NULL)
        *relief = RR_RELIEF_RAISED;
    else
        *relief = (*grad == RR_SURFACE_PARENTREL) ?
                  RR_RELIEF_FLAT : RR_RELIEF_RAISED;

    *border = FALSE;
    if (*relief == RR_RELIEF_FLAT) {
        if (strstr(tex, "border") != NULL)
            *border = TRUE;
    } else {
        if (strstr(tex, "bevel2") != NULL)
            *bevel = RR_BEVEL_2;
        else
            *bevel = RR_BEVEL_1;
    }

    if (strstr(tex, "interlaced") != NULL)
        *interlaced = TRUE;
    else
        *interlaced = FALSE;
}

static gboolean read_appearance(XrmDatabase db, const RrInstance *inst,
                                const gchar *rname, RrAppearance *value,
                                gboolean allow_trans)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *cname, *ctoname, *bcname, *icname, *hname, *sname;
    gchar *csplitname, *ctosplitname;
    gchar *rettype;
    XrmValue retvalue;
    gint i;

    cname = g_strconcat(rname, ".color", NULL);
    ctoname = g_strconcat(rname, ".colorTo", NULL);
    bcname = g_strconcat(rname, ".border.color", NULL);
    icname = g_strconcat(rname, ".interlace.color", NULL);
    hname = g_strconcat(rname, ".highlight", NULL);
    sname = g_strconcat(rname, ".shadow", NULL);
    csplitname = g_strconcat(rname, ".color.splitTo", NULL);
    ctosplitname = g_strconcat(rname, ".colorTo.splitTo", NULL);

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        parse_appearance(retvalue.addr,
                         &value->surface.grad,
                         &value->surface.relief,
                         &value->surface.bevel,
                         &value->surface.interlaced,
                         &value->surface.border,
                         allow_trans);
        if (!read_color(db, inst, cname, &value->surface.primary))
            value->surface.primary = RrColorNew(inst, 0, 0, 0);
        if (!read_color(db, inst, ctoname, &value->surface.secondary))
            value->surface.secondary = RrColorNew(inst, 0, 0, 0);
        if (value->surface.border)
            if (!read_color(db, inst, bcname,
                            &value->surface.border_color))
                value->surface.border_color = RrColorNew(inst, 0, 0, 0);
        if (value->surface.interlaced)
            if (!read_color(db, inst, icname,
                            &value->surface.interlace_color))
                value->surface.interlace_color = RrColorNew(inst, 0, 0, 0);
        if (read_int(db, hname, &i) && i >= 0)
            value->surface.bevel_light_adjust = i;
        if (read_int(db, sname, &i) && i >= 0 && i <= 256)
            value->surface.bevel_dark_adjust = i;

        if (value->surface.grad == RR_SURFACE_SPLIT_VERTICAL) {
            gint r, g, b;

            if (!read_color(db, inst, csplitname,
                            &value->surface.split_primary))
            {
                r = value->surface.primary->r;
                r += r >> 2;
                g = value->surface.primary->g;
                g += g >> 2;
                b = value->surface.primary->b;
                b += b >> 2;
                if (r > 0xFF) r = 0xFF;
                if (g > 0xFF) g = 0xFF;
                if (b > 0xFF) b = 0xFF;
                value->surface.split_primary = RrColorNew(inst, r, g, b);
            }

            if (!read_color(db, inst, ctosplitname,
                            &value->surface.split_secondary))
            {
                r = value->surface.secondary->r;
                r += r >> 4;
                g = value->surface.secondary->g;
                g += g >> 4;
                b = value->surface.secondary->b;
                b += b >> 4;
                if (r > 0xFF) r = 0xFF;
                if (g > 0xFF) g = 0xFF;
                if (b > 0xFF) b = 0xFF;
                value->surface.split_secondary = RrColorNew(inst, r, g, b);
            }
        }

        ret = TRUE;
    }

    g_free(ctosplitname);
    g_free(csplitname);
    g_free(sname);
    g_free(hname);
    g_free(icname);
    g_free(bcname);
    g_free(ctoname);
    g_free(cname);
    g_free(rclass);
    return ret;
}

static int parse_inline_number(const char *p)
{
    int neg = 1;
    int res = 0;
    if (*p == '-') {
        neg = -1;
        ++p;
    }
    for (; isdigit(*p); ++p)
        res = res * 10 + *p - '0';
    res *= neg;
    return res;
}

static void set_default_appearance(RrAppearance *a)
{
    a->surface.grad = RR_SURFACE_SOLID;
    a->surface.relief = RR_RELIEF_FLAT;
    a->surface.bevel = RR_BEVEL_1;
    a->surface.interlaced = FALSE;
    a->surface.border = FALSE;
    a->surface.primary = RrColorNew(a->inst, 0, 0, 0);
    a->surface.secondary = RrColorNew(a->inst, 0, 0, 0);
}

/* Reads the output from gimp's C-Source file format into valid RGBA data for
   an RrTextureRGBA. */
static RrPixel32* read_c_image(gint width, gint height, const guint8 *data)
{
    RrPixel32 *im, *p;
    gint i;

    p = im = g_memdup(data, width * height * sizeof(RrPixel32));

    for (i = 0; i < width * height; ++i) {
        guchar a = ((*p >> 24) & 0xff);
        guchar b = ((*p >> 16) & 0xff);
        guchar g = ((*p >>  8) & 0xff);
        guchar r = ((*p >>  0) & 0xff);

        *p = ((r << RrDefaultRedOffset) +
              (g << RrDefaultGreenOffset) +
              (b << RrDefaultBlueOffset) +
              (a << RrDefaultAlphaOffset));
        p++;
    }

    return im;
}

static void read_button_styles(XrmDatabase db, const RrInstance *inst, 
                               gchar *path,
                               const RrTheme *theme, RrButton *btn, 
                               const gchar *btnname,
                               struct fallbacks *fbs,
                               guchar *normal_mask,
                               guchar *toggled_mask)
{
    gchar name[128], name2[128];
    gboolean userdef = TRUE;

    g_snprintf(name, 128, "%s.xbm", btnname);
    if (!read_mask(inst, path, name, &btn->unpressed_mask) && normal_mask)
    {
        btn->unpressed_mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)normal_mask);
        userdef = FALSE;
    }
    g_snprintf(name, 128, "%s_toggled.xbm", btnname);
    if (toggled_mask && !read_mask(inst, path, name, &btn->unpressed_toggled_mask))
    {
        if (userdef)
            btn->unpressed_toggled_mask = RrPixmapMaskCopy(btn->unpressed_mask);
        else
            btn->unpressed_toggled_mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)toggled_mask);
    }
#define READ_BUTTON_MASK_COPY(type, fallback) \
    g_snprintf(name, 128, "%s_" #type ".xbm", btnname); \
    READ_MASK_COPY(name, btn->type##_mask, fallback);

    READ_BUTTON_MASK_COPY(pressed, btn->unpressed_mask);
    READ_BUTTON_MASK_COPY(disabled, btn->unpressed_mask);
    READ_BUTTON_MASK_COPY(hover, btn->unpressed_mask);
    if (toggled_mask) {
        g_snprintf(name, 128, "%s_toggled_pressed.xbm", btnname);
        READ_MASK_COPY(name, btn->pressed_toggled_mask, btn->unpressed_toggled_mask);
        g_snprintf(name, 128, "%s_toggled_hover.xbm", btnname);
        READ_MASK_COPY(name, btn->hover_toggled_mask, btn->unpressed_toggled_mask);
    }

#define READ_BUTTON_APPEARANCE(typedots, type, fallback) \
    g_snprintf(name, 128, "window.active.button.%s." typedots ".image.color", btnname); \
    READ_COLOR(name, btn->focused_##type##_color, RrColorCopy(theme->titlebut_focused_##type##_color)); \
    g_snprintf(name, 128, "window.inactive.button.%s." typedots ".image.color", btnname); \
    READ_COLOR(name, btn->unfocused_##type##_color, RrColorCopy(theme->titlebut_unfocused_##type##_color)); \
    if (fallback) { \
        g_snprintf(name, 128, "window.active.button.%s." typedots ".bg", btnname); \
        g_snprintf(name2, 128, "window.active.button.%s.toggled.bg", btnname); \
        READ_APPEARANCE_(name, name2, btn->a_focused_##type, TRUE, fbs->focused_##type); \
        g_snprintf(name, 128, "window.inactive.button.%s." typedots ".bg", btnname); \
        g_snprintf(name2, 128, "window.inactive.button.%s.toggled.bg", btnname); \
        READ_APPEARANCE_(name, name2, btn->a_unfocused_##type, TRUE, fbs->unfocused_##type); \
    } else { \
        g_snprintf(name, 128, "window.active.button.%s." typedots ".bg", btnname); \
        READ_APPEARANCE_COPY(name, btn->a_focused_##type, TRUE, fbs->focused_##type); \
        g_snprintf(name, 128, "window.inactive.button.%s." typedots ".bg", btnname); \
        READ_APPEARANCE_COPY(name, btn->a_unfocused_##type, TRUE, fbs->unfocused_##type); \
    } \
    btn->a_unfocused_##type->texture[0].typ##e = \
      btn->a_focused_##type->texture[0].typ##e = \
        RR_TEXTURE_MASK; \
    btn->a_unfocused_##type->texture[0].data.mask.mask = \
      btn->a_focused_##type->texture[0].data.mask.mask = \
        btn->type##_mask; \
    btn->a_unfocused_##type->texture[0].data.mask.color = \
        btn->unfocused_##type##_color; \
    btn->a_focused_##type->texture[0].data.mask.color = \
        btn->focused_##type##_color;

    READ_BUTTON_APPEARANCE("unpressed", unpressed, 0);
    READ_BUTTON_APPEARANCE("pressed", pressed, 0);
    READ_BUTTON_APPEARANCE("disabled", disabled, 0);
    READ_BUTTON_APPEARANCE("hover", hover, 0);
    if (toggled_mask) {
        READ_BUTTON_APPEARANCE("toggled.unpressed", unpressed_toggled, 1);
        READ_BUTTON_APPEARANCE("toggled.pressed", pressed_toggled, 0);
        READ_BUTTON_APPEARANCE("toggled.hover", hover_toggled, 0);
    }
}
