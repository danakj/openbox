#ifndef __glft_h__
#define __glft_h__

#include <fontconfig/fontconfig.h>

/* initialization */

FcBool GlftInit();

/* fonts */

struct GlftFont;

struct GlftFont *GlftFontOpen(const char *name);

void GlftFontClose(struct GlftFont *font);

/* rendering */

/*! Renders a string in UTF-8 encoding */
void GlftRenderString(struct GlftFont *font,
                      const char *str,
                      int bytes,
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
