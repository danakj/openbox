#ifndef __render_font_h
#define __render_font_h

#include "glft/glft.h"

struct RrInstance;

struct RrFont {
    struct RrInstance *inst;
    struct GlftFont *font;

    int height;
    int elipses;
};

#define RrFontElipsesLength(f) ((f)->elipses)

void RrFontRenderString(struct RrSurface *sur, struct RrFont *font,
                        struct RrColor *color, enum RrLayout layout,
                        const char *string, int x, int y, int w, int h);

#endif
