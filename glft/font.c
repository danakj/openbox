#include "font.h"
#include "glft.h"
#include "init.h"
#include "debug.h"
#include <assert.h>
#include <stdlib.h>

struct GlftFont *GlftFontOpen(const char *name)
{
    struct GlftFont *font;
    FcPattern *pat;
    FcFontSet *set;
    double alpha;

    assert(init_done);

    pat = FcNameParse((const unsigned char*)name);
    assert(pat);
    set = FcFontSetCreate();
    if (!FcFontSetAdd(set, pat)) {
        FcPatternDestroy(pat);
        FcFontSetDestroy(set);
        GlftDebug("failed to load font\n");
        return NULL;
    }
    
    font = malloc(sizeof(struct GlftFont));
    font->set = set;
    if (FcResultMatch != FcPatternGetBool(pat, "shadow", 0, &font->shadow))
        font->shadow = FcFalse;
    if (FcResultMatch !=
        FcPatternGetInteger(pat, "shadowoffset", 0, &font->shadow_offset))
        font->shadow_offset = 2;
    if (FcResultMatch != FcPatternGetDouble(pat, "shadowalpha", 0, &alpha))
        alpha = 0.5;
    font->shadow_alpha = (float)alpha;
    return font;
}

void GlftFontClose(struct GlftFont *font)
{
    if (font) {
        FcFontSetDestroy(font->set);
        free(font);
    }
}
