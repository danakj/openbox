/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   font.c for the Openbox window manager
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
#include "gettext.h"

#include <X11/Xft/Xft.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

#define ELIPSES "..."
#define ELIPSES_LENGTH(font) \
    (font->elipses_length + (font->shadow ? font->offset : 0))

#define OB_SHADOW "shadow"
#define OB_SHADOW_OFFSET "shadowoffset"
#define OB_SHADOW_ALPHA "shadowtint"

FcObjectType objs[] = {
    { OB_SHADOW,        FcTypeBool    },
    { OB_SHADOW_OFFSET, FcTypeInteger },
    { OB_SHADOW_ALPHA,  FcTypeInteger  }
};

static PangoContext *context;
static gboolean started = FALSE;

static void font_startup(void)
{
    if (!XftInit(0)) {
        g_warning(_("Couldn't initialize Xft."));
        exit(EXIT_FAILURE);
    }

#ifdef USE_PANGO
    g_type_init();
    /* these will never be freed, but we will need them until we shut down anyway */
    context = pango_xft_get_context(RrDisplay(NULL), RrScreen(NULL));
#endif /* USE_PANGO */
    /* Here we are teaching xft about the shadow, shadowoffset & shadowtint */
    FcNameRegisterObjectTypes(objs, (sizeof(objs) / sizeof(objs[0])));
}

static void measure_font(RrFont *f)
{
    /* xOff, yOff is the normal spacing to the next glyph. */
    XGlyphInfo info;

    /* measure an elipses */
    XftTextExtentsUtf8(RrDisplay(f->inst), f->xftfont,
                       (FcChar8*)ELIPSES, strlen(ELIPSES), &info);
    f->elipses_length = (signed) info.xOff;
}

static RrFont *openfont(const RrInstance *inst, gchar *fontstring)
{
    /* This function is called for each font in the theme file. */
    /* It returns a pointer to a RrFont struct after filling it. */
    RrFont *out;
    FcPattern *pat, *match;
    XftFont *font;
    FcResult res;
    gint tint;
#ifdef USE_PANGO
    guchar *tmp_string = NULL;
    gint tmp_int;
#endif /* USE_PANGO */

    if (!(pat = XftNameParse(fontstring)))
        return NULL;

    match = XftFontMatch(RrDisplay(inst), RrScreen(inst), pat, &res);
    FcPatternDestroy(pat);
    if (!match)
        return NULL;

    out = g_new(RrFont, 1);
    out->inst = inst;
#ifdef USE_PANGO
    /*    printf("\n\n%s\n\n",fontstring);
          FcPatternPrint(match); */

    out->pango_font_description = pango_font_description_new();

    if (FcPatternGetString(match, "family", 0, &tmp_string) != FcResultTypeMismatch) {
        pango_font_description_set_family(out->pango_font_description, (gchar *)tmp_string);
        tmp_string = NULL;
    }
    if (FcPatternGetString(match, "style", 0, &tmp_string) != FcResultTypeMismatch) {
        /* Bold ? */
        if (!strcasecmp("bold", (gchar *)tmp_string)) {
            pango_font_description_set_weight(out->pango_font_description, PANGO_WEIGHT_BOLD);
        }
        /* Italic ? */
        else if (!strcasecmp("italic", (gchar *)tmp_string)) {
            pango_font_description_set_style(out->pango_font_description, PANGO_STYLE_ITALIC);
        }
        tmp_string = NULL;
    }

    if (FcPatternGetInteger(match, "pixelsize", 0, &tmp_int) != FcResultTypeMismatch) {
        /* TODO: is PANGO_SCALE correct ?? */
        pango_font_description_set_size(out->pango_font_description, tmp_int*PANGO_SCALE);
    }

    PangoLanguage *ln;
    PangoFontMetrics *metrics = pango_context_get_metrics(context, out->pango_font_description, ln = pango_language_from_string("en_US"));
    out->pango_ascent = pango_font_metrics_get_ascent(metrics);
    out->pango_descent = pango_font_metrics_get_descent(metrics);
    pango_font_metrics_unref(metrics);
#endif /* USE_PANGO */

    if (FcPatternGetBool(match, OB_SHADOW, 0, &out->shadow) != FcResultMatch)
        out->shadow = FALSE;

    if (FcPatternGetInteger(match, OB_SHADOW_OFFSET, 0, &out->offset) !=
        FcResultMatch)
        out->offset = 1;

    if (FcPatternGetInteger(match, OB_SHADOW_ALPHA, 0, &tint) != FcResultMatch)
        tint = 25;
    if (tint > 100) tint = 100;
    else if (tint < -100) tint = -100;
    out->tint = tint;

    font = XftFontOpenPattern(RrDisplay(inst), match);
    if (!font) {
        FcPatternDestroy(match);
        g_free(out);
        return NULL;
    } else
        out->xftfont = font;

#ifdef USE_PANGO
    /*        FcPatternDestroy(match); */
#endif /* USE_PANGO */
    measure_font(out);

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
        XftFontClose(RrDisplay(f->inst), f->xftfont);
        g_free(f);
    }
#ifdef USE_PANGO
    pango_font_description_free(f->pango_font_description);
#endif
}

static void font_measure_full(const RrFont *f, const gchar *str,
                              gint *x, gint *y)
{
#ifdef USE_PANGO
    PangoLayout *pl;
    PangoRectangle rect;
    pl = pango_layout_new (context);
    pango_layout_set_text(pl, str, -1);
    pango_layout_set_font_description(pl, f->pango_font_description);
    pango_layout_set_single_paragraph_mode(pl, TRUE);
    pango_layout_get_pixel_extents(pl, NULL, &rect);
    *x = rect.width + (f->shadow ? ABS(f->offset) : 0);
    *y = rect.height + (f->shadow ? ABS(f->offset) : 0);
    g_object_unref(pl);

#else
    XGlyphInfo info;

    XftTextExtentsUtf8(RrDisplay(f->inst), f->xftfont,
                       (const FcChar8*)str, strlen(str), &info);

    *x = (signed) info.xOff + (f->shadow ? ABS(f->offset) : 0);
    *y = info.height + (f->shadow ? ABS(f->offset) : 0);
#endif /* USE_PANGO */
}

RrSize *RrFontMeasureString(const RrFont *f, const gchar *str)
{
    RrSize *size;
    size = g_new(RrSize, 1);
    font_measure_full (f, str, &size->width, &size->height);
    return size;
}

gint RrFontHeight(const RrFont *f)
{
#ifdef USE_PANGO
    return (f->pango_ascent
            + f->pango_descent
           ) / PANGO_SCALE +
           (f->shadow ? f->offset : 0);
#else
    return f->xftfont->ascent + f->xftfont->descent +
           (f->shadow ? f->offset : 0);
#endif
}

gint RrFontMaxCharWidth(const RrFont *f)
{
    return (signed) f->xftfont->max_advance_width;
}

#ifdef USE_PANGO
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
    int asc = f->pango_ascent;
    int ascdesc = asc + f->pango_descent;
    int space = height * PANGO_SCALE - ascdesc;
    int baseline = space / 2 + asc;
    return baseline / PANGO_SCALE;
}
#endif

void RrFontDraw(XftDraw *d, RrTextureText *t, RrRect *area)
{
    gint x,y,w,h;
    XftColor c;
    GString *text;
    gint mw, mh;
#ifndef USE_PANGO
    size_t l;
    gboolean shortened = FALSE;
#else
    PangoLayout *pl;
    PangoRectangle rect;

    pl = pango_layout_new (context);
#endif /* USE_PANGO */

    /* center vertically
     * for xft we pass the top edge of the text for positioning... */
#ifndef USE_PANGO
    y = area->y +
        (area->height - RrFontHeight(t->font)) / 2;
#else
    /* but for pango we pass the baseline, since different fonts have
     * different top edges. It looks stupid when the baseline of "normal"
     * text jumps up and down when a "strange" character is just added
     * to the end of the text */
    y = area->y +
        font_calculate_baseline(t->font, area->height);
#endif
    /* the +2 and -4 leave a small blank edge on the sides */
    x = area->x + 2;
    w = area->width - 4;
    h = area->height;

    text = g_string_new(t->string);
#ifndef USE_PANGO
    l = g_utf8_strlen(text->str, -1);
    font_measure_full(t->font, text->str, &mw, &mh);
    while (l && mw > area->width) {
        shortened = TRUE;
        /* remove a character from the middle */
        text = g_string_erase(text, l-- / 2, 1);
        /* if the elipses are too large, don't show them at all */
        if (ELIPSES_LENGTH(t->font) > area->width)
            shortened = FALSE;
        font_measure_full(t->font, text->str, &mw, &mh);
        mw += ELIPSES_LENGTH(t->font);
    }
    if (shortened) {
        text = g_string_insert(text, (l + 1) / 2, ELIPSES);
        l += 3;
    }
    if (!l) return;

    l = strlen(text->str); /* number of bytes */

#else
    pango_layout_set_text(pl, text->str, -1);
    pango_layout_set_font_description(pl, t->font->pango_font_description);
    pango_layout_set_single_paragraph_mode(pl, TRUE);
    pango_layout_set_width(pl, w * PANGO_SCALE);
    pango_layout_set_ellipsize(pl, PANGO_ELLIPSIZE_MIDDLE);
    /* This doesn't work with layout_line() of course */
/*    pango_layout_set_alignment(pl, (PangoAlignment)(t->justify)); */
    pango_layout_get_pixel_extents(pl, NULL, &rect);
    mw = rect.width;

#endif /* USE_PANGO */

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
#ifndef USE_PANGO
        XftDrawStringUtf8(d, &c, t->font->xftfont, x + t->font->offset,
                          t->font->xftfont->ascent + y + t->font->offset,
                          (FcChar8*)text->str, l);
#else /* USE_PANGO */
        /* see below... */
        pango_xft_render_layout_line(d, &c, pango_layout_get_line(pl, 0),
                                     (x + t->font->offset) * PANGO_SCALE,
                                     (y + t->font->offset) * PANGO_SCALE);
#endif /* USE_PANGO */
    }
    c.color.red = t->color->r | t->color->r << 8;
    c.color.green = t->color->g | t->color->g << 8;
    c.color.blue = t->color->b | t->color->b << 8;
    c.color.alpha = 0xff | 0xff << 8; /* fully opaque text */
    c.pixel = t->color->pixel;

#ifndef USE_PANGO
    XftDrawStringUtf8(d, &c, t->font->xftfont, x,
                      t->font->xftfont->ascent + y,
                      (FcChar8*)text->str, l);
#else /* USE_PANGO */
    /* This looks retarded, but layout_line() bases y on the baseline, while
     * layout() bases y on the top of the ink layout shit ass fucking crap.
     * We want the baseline to always be in the same place, thusly, we use
     * layout_line()
     * The actual line doesn't need to be freed */
    pango_xft_render_layout_line(d, &c, pango_layout_get_line(pl, 0),
                                 x * PANGO_SCALE, y * PANGO_SCALE);
    g_object_unref(pl);
#endif

    g_string_free(text, TRUE);
    return;
}
