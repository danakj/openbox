#include "render.h"
#include "instance.h"
#include "font.h"
#include <stdlib.h>
#include <string.h>

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
    int w, h;
    GlftMeasureString(font->font, string, strlen(string), &w, &h);
    return w;
}

int RrFontHeight(struct RrFont *font)
{
    return GlftFontHeight(font->font);
}

int RrFontMaxCharWidth(struct RrFont *font)
{
    return GlftFontMaxCharWidth(font->font);
}
