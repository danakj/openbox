#ifndef __glft_font_h
#define __glft_font_h

#include <fontconfig/fontconfig.h>

struct GlftFont {
    FcFontSet *set;

    /* extended font attributes */
    FcBool shadow;
    int shadow_offset;
    float shadow_alpha;
};

#endif
