#ifndef __render_font_h
#define __render_font_h

#include "glft/glft.h"

struct RrInstance;

struct RrFont {
    struct RrInstance *inst;
    struct GlftFont *font;
};

#endif
