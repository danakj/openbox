#include "render.h"
#include "instance.h"
#include "font.h"
#include <stdlib.h>

struct RrFont *RrFontOpen(struct RrInstance *inst, const char *fontstring)
{
    struct RrFont *font;

    font = malloc(sizeof(struct RrFont));
    font->inst = inst;
    font->font = GlftFontOpen(RrDisplay(inst), RrScreen(inst), fontstring);
    return font;
}

void RrFontClose(struct RrFont *font)
{
    if (font) {
        GlftFontClose(font->font);
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
