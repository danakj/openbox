#ifndef __glft_h__
#define __glft_h__

#include <fontconfig/fontconfig.h>
#include <X11/Xlib.h>

/* initialization */

FcBool GlftInit();

/* fonts */

struct GlftFont;

struct GlftFont *GlftFontOpen(Display *d, int screen, const char *name);

void GlftFontClose(struct GlftFont *font);

/* rendering */

struct GlftColor {
    float r;
    float g;
    float b;
    float a;
};

/*! Renders a string in UTF-8 encoding */
void GlftRenderString(struct GlftFont *font,
                      const char *str,
                      int bytes,
                      struct GlftColor *color,
                      int x,
                      int y);

/* metrics */

/*! Measures a string in UTF-8 encoding */
void GlftMeasureString(struct GlftFont *font,
                       const char *str,
                       int bytes,
                       int *w,
                       int *h);

#endif
