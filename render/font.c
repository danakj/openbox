#include "font.h"
#include "color.h"
#include "mask.h"
#include "theme.h"
#include "gettext.h"

#include <X11/Xft/Xft.h>
#include <glib.h>
#include <string.h>

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
        g_warning(_("Couldn't initialize Xft.\n"));
        exit(EXIT_FAILURE);
    }
    FcNameRegisterObjectTypes(objs, (sizeof(objs) / sizeof(objs[0])));
}

static void measure_font(RrFont *f)
{
    XGlyphInfo info;

    /* measure an elipses */
    XftTextExtentsUtf8(RrDisplay(f->inst), f->xftfont,
                       (FcChar8*)ELIPSES, strlen(ELIPSES), &info);
    f->elipses_length = (signed) info.xOff;
}

static RrFont *openfont(const RrInstance *inst, char *fontstring)
{
    RrFont *out;
    FcPattern *pat, *match;
    XftFont *font;
    FcResult res;
    gint tint;

    if (!(pat = XftNameParse(fontstring)))
        return NULL;

    match = XftFontMatch(RrDisplay(inst), RrScreen(inst), pat, &res);
    if (!match)
        return NULL;

    out = g_new(RrFont, 1);
    out->inst = inst;

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

    measure_font(out);

    return out;
}

RrFont *RrFontOpen(const RrInstance *inst, char *fontstring)
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
    XGlyphInfo info;

    XftTextExtentsUtf8(RrDisplay(f->inst), f->xftfont,
                       (const FcChar8*)str, strlen(str), &info);

    *x = (signed) info.xOff + (f->shadow ? ABS(f->offset) : 0);
    *y = info.height + (f->shadow ? ABS(f->offset) : 0);
}

int RrFontMeasureString(const RrFont *f, const gchar *str)
{
    gint x, y;
    font_measure_full (f, str, &x, &y);
    return x;
}

int RrFontHeight(const RrFont *f)
{
    return f->xftfont->ascent + f->xftfont->descent +
        (f->shadow ? f->offset : 0);
}

int RrFontMaxCharWidth(const RrFont *f)
{
    return (signed) f->xftfont->max_advance_width;
}

void RrFontDraw(XftDraw *d, RrTextureText *t, RrRect *area)
{
    gint x,y,w,h;
    XftColor c;
    GString *text;
    gint mw, em, mh;
    size_t l;
    gboolean shortened = FALSE;

    /* center vertically */
    y = area->y +
        (area->height - RrFontHeight(t->font)) / 2;
    /* the +2 and -4 leave a small blank edge on the sides */
    x = area->x + 2;
    w = area->width - 4;
    h = area->height;

    text = g_string_new(t->string);
    l = g_utf8_strlen(text->str, -1);
    font_measure_full(t->font, text->str, &mw, &mh);
    while (l && mw > area->width) {
        shortened = TRUE;
        /* remove a character from the middle */
        text = g_string_erase(text, l-- / 2, 1);
        em = ELIPSES_LENGTH(t->font);
        /* if the elipses are too large, don't show them at all */
        if (em > area->width)
            shortened = FALSE;
        font_measure_full(t->font, text->str, &mw, &mh);
        mw += em;
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
        XftDrawStringUtf8(d, &c, t->font->xftfont, x + t->font->offset,
                          t->font->xftfont->ascent + y + t->font->offset,
                          (FcChar8*)text->str, l);
    }  
    c.color.red = t->color->r | t->color->r << 8;
    c.color.green = t->color->g | t->color->g << 8;
    c.color.blue = t->color->b | t->color->b << 8;
    c.color.alpha = 0xff | 0xff << 8; /* fully opaque text */
    c.pixel = t->color->pixel;

    XftDrawStringUtf8(d, &c, t->font->xftfont, x,
                      t->font->xftfont->ascent + y,
                      (FcChar8*)text->str, l);
    return;
}
