#ifndef __glft_render_h
#define __glft_render_h

#include <ft2build.h>
#include FT_FREETYPE_H

#include "font.h"

void GlftRenderGlyph(FT_Face face, struct GlftGlyph *g);

#endif
