/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   font.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003        Ben Jansens
   Copyright (c) 2003        Derek Foreman

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

#include "font.h"
#include "color.h"
#include "mask.h"
#include "theme.h"
#include "geom.h"
#include "instance.h"
#include "gettext.h"

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#define OB_SHADOW "shadow"
#define OB_SHADOW_OFFSET "shadowoffset"
#define OB_SHADOW_ALPHA "shadowtint"

FcObjectType objs[] = {
    { OB_SHADOW,        FcTypeBool    },
    { OB_SHADOW_OFFSET, FcTypeInteger },
    { OB_SHADOW_ALPHA,  FcTypeInteger  }
};

static gboolean started = FALSE;

static void font_startup(void)
{
    if (!XftInit(0)) {
        g_warning(_("Couldn't initialize Xft."));
        exit(EXIT_FAILURE);
    }

    /* Here we are teaching xft about the shadow, shadowoffset & shadowtint */
    FcNameRegisterObjectTypes(objs, (sizeof(objs) / sizeof(objs[0])));
}

static void measure_font(const RrInstance *inst, RrFont *f)
{
    PangoFontMetrics *metrics;
    gchar *locale, *p;

    /* get the default language from the locale
       (based on gtk_get_default_language in gtkmain.c) */
    locale = g_strdup(setlocale(LC_CTYPE, NULL));
    if ((p = strchr(locale, '.'))) *p = '\0'; /* strip off the . */
    if ((p = strchr(locale, '@'))) *p = '\0'; /* strip off the @ */

    /* measure the ascent and descent */
    metrics = pango_context_get_metrics(inst->pango, f->font_desc,
                                        pango_language_from_string(locale));
    f->ascent = pango_font_metrics_get_ascent(metrics);
    f->descent = pango_font_metrics_get_descent(metrics);
    pango_font_metrics_unref(metrics);

    g_free(locale);
}

static RrFont *openfont(const RrInstance *inst, gchar *fontstring)
{
    /* This function is called for each font in the theme file. */
    /* It returns a pointer to a RrFont struct after filling it. */
    RrFont *out;
    FcPattern *pat;
    gint tint;
    gchar *sval;
    gint ival;

    if (!(pat = XftNameParse(fontstring)))
        return NULL;

    out = g_new(RrFont, 1);
    out->inst = inst;
    out->font_desc = pango_font_description_new();
    out->layout = pango_layout_new(inst->pango);

    /* get the data from the parsed xft string */

    /* get the family */
    if (FcPatternGetString(pat, "family", 0,
                           (FcChar8**)&sval) == FcResultMatch)
        pango_font_description_set_family(out->font_desc, sval);
    
    /* get the weight */
    if (FcPatternGetInteger(pat, "weight", 0, &ival) == FcResultMatch) {
        if (ival == FC_WEIGHT_LIGHT)
            pango_font_description_set_weight(out->font_desc,
                                              PANGO_WEIGHT_LIGHT);
        else if (ival == FC_WEIGHT_DEMIBOLD)
            pango_font_description_set_weight(out->font_desc,
                                              PANGO_WEIGHT_SEMIBOLD);
        else if (ival == FC_WEIGHT_BOLD)
            pango_font_description_set_weight(out->font_desc,
                                              PANGO_WEIGHT_BOLD);
        else if (ival == FC_WEIGHT_BLACK)
            pango_font_description_set_weight(out->font_desc,
                                              PANGO_WEIGHT_ULTRABOLD);
    }

    /* get the style/slant */
    if (FcPatternGetInteger(pat, "slant", 0, &ival) == FcResultMatch) {
        if (ival == FC_SLANT_ITALIC)
            pango_font_description_set_style(out->font_desc,
                                             PANGO_STYLE_ITALIC);
        else if (ival == FC_SLANT_OBLIQUE)
            pango_font_description_set_style(out->font_desc,
                                             PANGO_STYLE_OBLIQUE);
    }

    /* get the size */
    if (FcPatternGetInteger(pat, "size", 0, &ival) == FcResultMatch)
        pango_font_description_set_size(out->font_desc, ival * PANGO_SCALE);
    else if (FcPatternGetInteger(pat, "pixelsize", 0, &ival) == FcResultMatch)
        pango_font_description_set_absolute_size(out->font_desc,
                                                 ival * PANGO_SCALE);

    if (FcPatternGetBool(pat, OB_SHADOW, 0, &out->shadow) != FcResultMatch)
        out->shadow = FALSE;

    if (FcPatternGetInteger(pat, OB_SHADOW_OFFSET, 0, &out->offset) !=
        FcResultMatch)
        out->offset = 1;

    if (FcPatternGetInteger(pat, OB_SHADOW_ALPHA, 0, &tint) != FcResultMatch)
        tint = 25;
    if (tint > 100) tint = 100;
    else if (tint < -100) tint = -100;
    out->tint = tint;

    /* setup the layout */
    pango_layout_set_font_description(out->layout, out->font_desc);
    pango_layout_set_single_paragraph_mode(out->layout, TRUE);
    pango_layout_set_ellipsize(out->layout, PANGO_ELLIPSIZE_MIDDLE);

    /* get the ascent and descent */
    measure_font(inst, out);

    return out;
}

RrFont *RrFontOpen(const RrInstance *inst, gchar *fontstring)
{
    RrFont *out;

    if (!started) {
        font_startup();
        started = TRUE;
    }

    if ((out = openfont(inst, fontstring)))
        return out;
    g_warning(_("Unable to load font: %s\n"), fontstring);
    g_warning(_("Trying fallback font: %s\n"), "sans");

    if ((out = openfont(inst, "sans")))
        return out;
    g_warning(_("Unable to load font: %s\n"), "sans");

    return NULL;
}

void RrFontClose(RrFont *f)
{
    if (f) {
        g_object_unref(f->layout);
        pango_font_description_free(f->font_desc);
        g_free(f);
    }
}

static void font_measure_full(const RrFont *f, const gchar *str,
                              gint *x, gint *y)
{
    PangoRectangle rect;

    pango_layout_set_text(f->layout, str, -1);
    pango_layout_set_width(f->layout, -1);
    pango_layout_get_pixel_extents(f->layout, NULL, &rect);
    *x = rect.width + (f->shadow ? ABS(f->offset) : 0);
    *y = rect.height + (f->shadow ? ABS(f->offset) : 0);
}

RrSize *RrFontMeasureString(const RrFont *f, const gchar *str)
{
    RrSize *size;
    size = g_new(RrSize, 1);
    font_measure_full(f, str, &size->width, &size->height);
    return size;
}

gint RrFontHeight(const RrFont *f)
{
    return (f->ascent + f->descent) / PANGO_SCALE +
        (f->shadow ? f->offset : 0);
}

static inline int font_calculate_baseline(RrFont *f, gint height)
{
/* For my own reference:
 *   _________
 *  ^space/2  ^height     ^baseline
 *  v_________|_          |
 *            | ^ascent   |   _           _
 *            | |         |  | |_ _____ _| |_ _  _
 *            | |         |  |  _/ -_) \ /  _| || |
 *            | v_________v   \__\___/_\_\\__|\_, |
 *            | ^descent                      |__/
 *  __________|_v
 *  ^space/2  |
 *  V_________v
 */
    return (((height * PANGO_SCALE) /* height of the space in pango units */
             - (f->ascent + f->descent)) /* minus space taken up by text */
            / 2 /* divided by two -> half of the empty space (this is the top
                   of the text) */
            + f->ascent) /* now move down to the baseline */
        / PANGO_SCALE; /* back to pixels */
}

void RrFontDraw(XftDraw *d, RrTextureText *t, RrRect *area)
{
    gint x,y,w,h;
    XftColor c;
    gint mw;
    PangoRectangle rect;

    /* center the text vertically
       We do this centering based on the 'baseline' since different fonts have
       different top edges. It looks bad when the whole string is moved when 1
       character from a non-default language is included in the string */
    y = area->y +
        font_calculate_baseline(t->font, area->height);

    /* the +2 and -4 leave a small blank edge on the sides */
    x = area->x + 2;
    w = area->width - 4;
    h = area->height;

    pango_layout_set_text(t->font->layout, t->string, -1);
    pango_layout_set_width(t->font->layout, w * PANGO_SCALE);

    pango_layout_get_pixel_extents(t->font->layout, NULL, &rect);
    mw = rect.width;

    /* pango_layout_set_alignment doesn't work with 
       pango_xft_render_layout_line */
    switch (t->justify) {
    case RR_JUSTIFY_LEFT:
        break;
    case RR_JUSTIFY_RIGHT:
        x += (w - mw);
        break;
    case RR_JUSTIFY_CENTER:
        x += (w - mw) / 2;
        break;
    }

    if (t->font->shadow) {
        if (t->font->tint >= 0) {
            c.color.red = 0;
            c.color.green = 0;
            c.color.blue = 0;
            c.color.alpha = 0xffff * t->font->tint / 100;
            c.pixel = BlackPixel(RrDisplay(t->font->inst),
                                 RrScreen(t->font->inst));
        } else {
            c.color.red = 0xffff;
            c.color.green = 0xffff;
            c.color.blue = 0xffff;
            c.color.alpha = 0xffff * -t->font->tint / 100;
            c.pixel = WhitePixel(RrDisplay(t->font->inst),
                                 RrScreen(t->font->inst));
        }
        /* see below... */
        pango_xft_render_layout_line
            (d, &c, pango_layout_get_line(t->font->layout, 0),
             (x + t->font->offset) * PANGO_SCALE,
             (y + t->font->offset) * PANGO_SCALE);
    }

    c.color.red = t->color->r | t->color->r << 8;
    c.color.green = t->color->g | t->color->g << 8;
    c.color.blue = t->color->b | t->color->b << 8;
    c.color.alpha = 0xff | 0xff << 8; /* fully opaque text */
    c.pixel = t->color->pixel;

    /* layout_line() uses y to specify the baseline
       The line doesn't need to be freed, it's a part of the layout */
    pango_xft_render_layout_line
        (d, &c, pango_layout_get_line(t->font->layout, 0),
         x * PANGO_SCALE, y * PANGO_SCALE);
}
