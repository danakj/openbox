#include <stdlib.h>
#include <X11/Xft/Xft.h>
#include "../kernel/openbox.h"
#include "font.h"

#include "../src/gettext.h"
#define _(str) gettext(str)

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

static void measure_height(ObFont *f)
{
    XGlyphInfo info;
    char *str;

    /* XXX add some extended UTF8 characters in here? */
    str = "12345678900-qwertyuiopasdfghjklzxcvbnm"
        "!@#$%^&*()_+QWERTYUIOPASDFGHJKLZXCVBNM"
        "`~[]\\;',./{}|:\"<>?";

    XftTextExtentsUtf8(ob_display, f->xftfont,
                       (FcChar8*)str, strlen(str), &info);
    f->height = (signed) info.height;
}

ObFont *font_open(char *fontstring)
{
    ObFont *out;
    XftFont *xf;
    
    if ((xf = XftFontOpenName(ob_display, ob_screen, fontstring))) {
        out = malloc(sizeof(ObFont));
        out->xftfont = xf;
        measure_height(out);
        return out;
    }
    g_warning(_("Unable to load font: %s\n"), fontstring);
    g_warning(_("Trying fallback font: %s\n"), "fixed");

    if ((xf = XftFontOpenName(ob_display, ob_screen, "fixed"))) {
        out = malloc(sizeof(ObFont));
        out->xftfont = xf;
        measure_height(out);
        return out;
    }
    g_warning(_("Unable to load font: %s\n"), "fixed");
    g_warning(_("Aborting!.\n"));

    exit(3); // can't continue without a font
}

void font_close(ObFont *f)
{
    XftFontClose(ob_display, f->xftfont);
}

int font_measure_string(ObFont *f, const char *str, int shadow, int offset)
{
    XGlyphInfo info;

    XftTextExtentsUtf8(ob_display, f->xftfont,
                       (FcChar8*)str, strlen(str), &info);

    return (signed) info.xOff + (shadow ? offset : 0);
}

int font_height(ObFont *f, int shadow, int offset)
{
    return f->height + (shadow ? offset : 0);
}

int font_max_char_width(ObFont *f)
{
    return (signed) f->xftfont->max_advance_width;
}

void font_draw(XftDraw *d, TextureText *t, int x, int y, int w, int h)
{
    XftColor c;

    /* accomidate for areas bigger/smaller than Xft thinks the font is tall */
    y += (h - t->font->xftfont->height) / 2;

    x += 3; /* XXX figure out X with justification */

    if (t->shadow) {
        c.color.red = 0;
        c.color.green = 0;
        c.color.blue = 0;
        c.color.alpha = t->tint | t->tint << 8; /* transparent shadow */
        c.pixel = BlackPixel(ob_display, ob_screen);
  
        XftDrawStringUtf8(d, &c, t->font->xftfont, x + t->offset,
                          t->font->xftfont->ascent + y + t->offset,
                          (FcChar8*)t->string, strlen(t->string));
    }  
    c.color.red = t->color->r | t->color->r << 8;
    c.color.green = t->color->g | t->color->g << 8;
    c.color.blue = t->color->b | t->color->b << 8;
    c.color.alpha = 0xff | 0xff << 8; /* fully opaque text */
    c.pixel = t->color->pixel;
                     
    XftDrawStringUtf8(d, &c, t->font->xftfont, x,
                      t->font->xftfont->ascent + y -
                      (t->font->xftfont->height - t->font->height) / 2,
                     (FcChar8*)t->string, strlen(t->string));
    return;
}
