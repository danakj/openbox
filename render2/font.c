#include "render.h"
#include "instance.h"
#include "surface.h"
#include "font.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#define ALPHAS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" \
               "1234567890`-=\\!@#$%^&*()~_+|[]{};':\",./<>?"
#define ELIPSES "..."

struct RrFont *RrFontOpen(struct RrInstance *inst, const char *fontstring)
{
    struct RrFont *font;
    int w, h;

    font = malloc(sizeof(struct RrFont));
    font->inst = inst;
    font->font = GlftFontOpen(RrDisplay(inst), RrScreen(inst), fontstring);

    GlftMeasureString(font->font, ELIPSES, strlen(ELIPSES), &w, &h);
    font->elipses = w;
    GlftMeasureString(font->font, ALPHAS, strlen(ALPHAS), &w, &h);
    font->height = h;

    return font;
}

void RrFontClose(struct RrFont *font)
{
    if (font) {
        GlftFontClose(font->font);
        free(font);
    }
}

int RrFontMeasureString(struct RrFont *font, const char *string)
{
    int w, h;
    GlftMeasureString(font->font, string, strlen(string), &w, &h);
    return w;
}

int RrFontHeight(struct RrFont *font)
{
    return GlftFontHeight(font->font);
}

int RrFontMaxCharWidth(struct RrFont *font)
{
    return GlftFontMaxCharWidth(font->font);
}

void RrFontRenderString(struct RrSurface *sur, struct RrFont *font,
                        struct RrColor *color, enum RrLayout layout,
                        const char *string, int x, int y, int w, int h)
{
    struct GlftColor col;
    int fh = RrFontHeight(font);
    int l, mw, mh;
    GString *text;
    int shortened = 0;

    switch (layout) {
    case RR_TOP_LEFT:
    case RR_TOP:
    case RR_TOP_RIGHT:
        y += h - fh;
        break;
    case RR_LEFT:
    case RR_CENTER:
    case RR_RIGHT:
        y += (h - fh) / 2;
        break;
    case RR_BOTTOM_LEFT:
    case RR_BOTTOM:
    case RR_BOTTOM_RIGHT:
        break;
    }

    text = g_string_new(string);
    l = g_utf8_strlen(text->str, -1);
    GlftMeasureString(font->font, text->str, strlen(text->str), &mw, &mh);
    if (font->elipses > w)
        l = 0; /* nothing fits.. */
    else {
        while (l && mw > w) {
            shortened = 1;
            /* remove a character from the middle */
            text = g_string_erase(text, l-- / 2, 1);
            /* if the elipses are too large, don't show them at all */
            GlftMeasureString(font->font, text->str, strlen(text->str),
                              &mw, &mh);
            mw += font->elipses;
        }
        if (shortened) {
            text = g_string_insert(text, (l + 1) / 2, ELIPSES);
            l += 3;
        }
    }
    if (!l) return;

    /* center in the font's height's area based on the measured height of the
       specific string */
    y += (fh - mh) / 2;

    switch (layout) {
    case RR_TOP_LEFT:
    case RR_LEFT:
    case RR_BOTTOM_LEFT:
        break;
    case RR_TOP:
    case RR_CENTER:
    case RR_BOTTOM:
        x += (w - mw) / 2;
        break;
    case RR_TOP_RIGHT:
    case RR_RIGHT:
    case RR_BOTTOM_RIGHT:
        x += w - mw;
        break;
    }

    col.r = color->r;
    col.g = color->g;
    col.b = color->b;
    col.a = color->a;
    GlftRenderString(font->font, text->str, strlen(text->str), &col, x, y);
}
