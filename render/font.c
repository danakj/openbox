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

static gboolean started = FALSE;

static void font_startup(void)
{
    if (!XftInit(0)) {
        g_warning(_("Couldn't initialize Xft."));
        exit(EXIT_FAILURE);
    }

#ifdef USE_PANGO
    g_type_init();
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
    gchar *tmp_string = NULL;
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
        pango_font_description_set_family(out->pango_font_description, tmp_string);
        tmp_string = NULL;
    }
    if (FcPatternGetString(match, "style", 0, &tmp_string) != FcResultTypeMismatch) {
        /* Bold ? */
        if (!strcasecmp("bold", tmp_string)) {
            pango_font_description_set_weight(out->pango_font_description, PANGO_WEIGHT_BOLD);
        }
        /* Italic ? */
        else if (!strcasecmp("italic", tmp_string)) {
            pango_font_description_set_style(out->pango_font_description, PANGO_STYLE_ITALIC);
        }
        tmp_string = NULL;
    }

    if (FcPatternGetInteger(match, "pixelsize", 0, &tmp_int) != FcResultTypeMismatch) {
        /* TODO: is PANGO_SCALE correct ?? */
        pango_font_description_set_size(out->pango_font_description, tmp_int*PANGO_SCALE);
    }
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
}

static void font_measure_full(const RrFont *f, const gchar *str,
                              gint *x, gint *y)
{
#ifdef USE_PANGO
    PangoContext *context;
    PangoLayout *pl;
    PangoRectangle rect;
    context = pango_xft_get_context (RrDisplay(f->inst), RrScreen(f->inst));
    pl = pango_layout_new (context);
    pango_layout_set_text(pl, str, -1);
    pango_layout_set_font_description(pl, f->pango_font_description);
    pango_layout_set_single_paragraph_mode(pl, TRUE);
    pango_layout_get_pixel_extents(pl, NULL, &rect);
    *x = rect.width + (f->shadow ? ABS(f->offset) : 0);
    *y = rect.height + (f->shadow ? ABS(f->offset) : 0);
    g_object_unref(pl);
    g_object_unref(context);

#else
    XGlyphInfo info;

    XftTextExtentsUtf8(RrDisplay(f->inst), f->xftfont,
                       (const FcChar8*)str, strlen(str), &info);

    *x = (signed) info.xOff + (f->shadow ? ABS(f->offset) : 0);
    *y = info.height + (f->shadow ? ABS(f->offset) : 0);
#endif /* USE_PANGO */
}

gint RrFontMeasureString(const RrFont *f, const gchar *str)
{
    gint x, y;
    font_measure_full (f, str, &x, &y);
    return x + 4;
}

gint RrFontHeight(const RrFont *f)
{
#ifndef USE_PANGO
    return f->xftfont->ascent + f->xftfont->descent +
        (f->shadow ? f->offset : 0);
#else /* USE_PANGO */
     /*
     PangoContext *context = pango_context_new ();
 
     PangoFontMetrics *metrics = pango_context_get_metrics(context, f->pango_font, NULL);
 
     gint result =  pango_font_metrics_get_ascent (metrics) +
         pango_font_metrics_get_descent(metrics) +
         (f->shadow ? f->offset : 0);
 
     pango_font_metrics_unref(metrics);
     g_object_unref(context);
     return result;
 */
    return f->xftfont->ascent + f->xftfont->descent +
        (f->shadow ? f->offset : 0);

#endif /* USE_PANGO */
}

gint RrFontMaxCharWidth(const RrFont *f)
{
    return (signed) f->xftfont->max_advance_width;
}

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
    PangoContext *context;

    context = pango_xft_get_context (RrDisplay(t->font->inst), RrScreen(t->font->inst));
    pl = pango_layout_new (context);
#endif /* USE_PANGO */

    /* center vertically */
    y = area->y +
        (area->height - RrFontHeight(t->font)) / 2;
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

    l = strlen(text->str); /* number of bytes */

#else
    pango_layout_set_text(pl, text->str, -1);
    pango_layout_set_font_description(pl, t->font->pango_font_description);
    pango_layout_set_single_paragraph_mode(pl, TRUE);
    pango_layout_set_width(pl, w * PANGO_SCALE);
    pango_layout_set_ellipsize(pl, PANGO_ELLIPSIZE_MIDDLE);
    pango_layout_set_alignment(pl, (PangoAlignment)(t->justify));
#endif /* USE_PANGO */

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
#ifndef USE_PANGO
        }
        XftDrawStringUtf8(d, &c, t->font->xftfont, x + t->font->offset,
                          t->font->xftfont->ascent + y + t->font->offset,
                          (FcChar8*)text->str, l);
    }
#else /* USE_PANGO */
        }
    pango_xft_render_layout(d, &c, pl, (x + t->font->offset) * PANGO_SCALE,
                            (y + t->font->offset) * PANGO_SCALE);
    }
#endif /* USE_PANGO */
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
    pango_xft_render_layout(d, &c, pl, x * PANGO_SCALE, y * PANGO_SCALE);
    g_object_unref(pl);
    g_object_unref(context);
#endif

    g_string_free(text, TRUE);
    return;
}
