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

#endif
