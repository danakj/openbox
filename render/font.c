#include "font.h"
#include "theme.h"
#include "kernel/geom.h"
#include "kernel/gettext.h"
#define _(str) gettext(str)

#include <X11/Xft/Xft.h>
#include <glib.h>
#include <string.h>

#define ELIPSES "..."
#define ELIPSES_LENGTH(font, shadow, offset) \
    (font->elipses_length + (shadow ? offset : 0))

void font_startup(void)
{
#ifdef DEBUG
    int version;
#endif /* DEBUG */
    if (!XftInit(0)) {
        g_warning(_("Couldn't initialize Xft.\n"));
        exit(3);
    }
#ifdef DEBUG
    version = XftGetVersion();
    g_message("Using Xft %d.%d.%d (Built against %d.%d.%d).",
              version / 10000 % 100, version / 100 % 100, version % 100,
              XFT_MAJOR, XFT_MINOR, XFT_REVISION);
#endif
}

static void measure_height(RrFont *f)
{
    XGlyphInfo info;

    /* measure an elipses */
    XftTextExtentsUtf8(RrDisplay(f->inst), f->xftfont,
                       (FcChar8*)ELIPSES, strlen(ELIPSES), &info);
    f->elipses_length = (signed) info.xOff;
}

RrFont *font_open(const RrInstance *inst, char *fontstring)
{
    RrFont *out;
    XftFont *xf;
    
    if ((xf = XftFontOpenName(RrDisplay(inst), RrScreen(inst), fontstring))) {
        out = g_new(RrFont, 1);
        out->inst = inst;
        out->xftfont = xf;
        measure_height(out);
        return out;
    }
    g_warning(_("Unable to load font: %s\n"), fontstring);
    g_warning(_("Trying fallback font: %s\n"), "sans");

    if ((xf = XftFontOpenName(RrDisplay(inst), RrScreen(inst), "sans"))) {
        out = g_new(RrFont, 1);
        out->inst = inst;
        out->xftfont = xf;
        measure_height(out);
        return out;
    }
    g_warning(_("Unable to load font: %s\n"), "sans");
    g_warning(_("Aborting!.\n"));

    exit(3); /* can't continue without a font */
}

void font_close(RrFont *f)
{
    if (f) {
        XftFontClose(RrDisplay(f->inst), f->xftfont);
        g_free(f);
    }
}

void font_measure_full(RrFont *f, char *str, int shadow, int offset,
                       int *x, int *y)
{
    XGlyphInfo info;

    XftTextExtentsUtf8(RrDisplay(f->inst), f->xftfont,
                       (FcChar8*)str, strlen(str), &info);

    *x = (signed) info.xOff + (shadow ? ABS(offset) : 0);
    *y = info.height + (shadow ? ABS(offset) : 0);
}

int font_measure_string(RrFont *f, char *str, int shadow, int offset)
{
    int x, y;
    font_measure_full (f, str, shadow, offset, &x, &y);
    return x;
}

int font_height(RrFont *f, int shadow, int offset)
{
    return f->xftfont->ascent + f->xftfont->descent + (shadow ? offset : 0);
}

int font_max_char_width(RrFont *f)
{
    return (signed) f->xftfont->max_advance_width;
}

void font_draw(XftDraw *d, RrTextureText *t, Rect *area)
{
    int x,y,w,h;
    XftColor c;
    GString *text;
    int mw, em, mh;
    size_t l;
    gboolean shortened = FALSE;

    /* center vertically */
    y = area->y +
        (area->height - font_height(t->font, t->shadow, t->offset)) / 2;
    w = area->width;
    h = area->height;

    text = g_string_new(t->string);
    l = g_utf8_strlen(text->str, -1);
    font_measure_full(t->font, text->str, t->shadow, t->offset, &mw, &mh);
    while (l && mw > area->width) {
        shortened = TRUE;
        /* remove a character from the middle */
        text = g_string_erase(text, l-- / 2, 1);
        em = ELIPSES_LENGTH(t->font, t->shadow, t->offset);
        /* if the elipses are too large, don't show them at all */
        if (em > area->width)
            shortened = FALSE;
        font_measure_full(t->font, text->str, t->shadow, t->offset, &mw, &mh);
        mw += em;
    }
    if (shortened) {
        text = g_string_insert(text, (l + 1) / 2, ELIPSES);
        l += 3;
    }
    if (!l) return;

    switch (t->justify) {
    case RR_JUSTIFY_LEFT:
        x = area->x;
        break;
    case RR_JUSTIFY_RIGHT:
        x = area->x + (w - mw);
        break;
    case RR_JUSTIFY_CENTER:
        x = area->x + (w - mw) / 2;
        break;
    }

    l = strlen(text->str); /* number of bytes */

    if (t->shadow) {
        if (t->tint >= 0) {
            c.color.red = 0;
            c.color.green = 0;
            c.color.blue = 0;
            c.color.alpha = 0xffff * t->tint / 100; /* transparent shadow */
            c.pixel = BlackPixel(RrDisplay(t->font->inst),
                                 RrScreen(t->font->inst));
        } else {
            c.color.red = 0xffff * -t->tint / 100;
            c.color.green = 0xffff * -t->tint / 100;
            c.color.blue = 0xffff * -t->tint / 100;
            c.color.alpha = 0xffff * -t->tint / 100; /* transparent shadow */
            c.pixel = WhitePixel(RrDisplay(t->font->inst),
                                 RrScreen(t->font->inst));
        }  
        XftDrawStringUtf8(d, &c, t->font->xftfont, x + t->offset,
                          t->font->xftfont->ascent + y + t->offset,
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
