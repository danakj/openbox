#include "render.h"
#include "font.h"
#include <stdlib.h>

struct RrFont *RrFontOpen(struct RrInstance *inst, const char *fontstring)
{
    struct RrFont *font;

    font = malloc(sizeof(struct RrFont));
    font->inst = inst;
/*XXX    font->font = GlftFontOpen(fontstring);*/
    return font;
}

void RrFontClose(struct RrFont *font)
{
    if (font) {
/*XXX        GlftFontClose(font->font);*/
        free(font);
    }
}

int RrFontMeasureString(struct RrFont *font, const char *string)
{
    return 20;
}

int RrFontHeight(struct RrFont *font)
{
    return 8;
}

int RrFontMaxCharWidth(struct RrFont *font)
{
    return 5;
}
