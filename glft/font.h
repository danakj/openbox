#ifndef __glft_font_h
#define __glft_font_h

#include <fontconfig/fontconfig.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fcfreetype.h>

#include <X11/Xlib.h>
#include <glib.h>

#define GLFT_SHADOW "shadow"
#define GLFT_SHADOW_OFFSET "shadowoffset"
#define GLFT_SHADOW_ALPHA "shadowalpha"

struct GlftFont {
    Display *display;
    int screen;

    FcPattern *pat;
    FcCharSet *chars;

    char *filename;
    int index;

    FT_Face face;
    FT_Int ftflags;
    FT_F26Dot6 ftcharsize;

    FcBool antialias;
    int spacing;
    FcBool minspace;
    int char_width;
    /* extended font attributes */
    FcBool shadow;
    int shadow_offset;
    float shadow_alpha;

    GHashTable *glyph_map;

    int kerning : 1;

    /* public shit */
    int ascent;
    int descent;
    int height;
    int max_advance_width;
};

struct GlftGlyph {
    /* The character in UCS-4 encoding */
    FcChar32 w;
    /* OpenGL texture for the character */
    unsigned int tnum;
    /* The FT_Face glyph */
    FT_UInt glyph;

    int x;
    int y;
    int width;
    int height;

    int padx, pady;
    int texw, texh;
    int left, yoff;
};

/*! Takes a character in UTF-8 encoding and returns an OpenGL display list
 for it */
struct GlftGlyph *GlftFontGlyph(struct GlftFont *font, const char *c);

int GlftFontAdvance(struct GlftFont *font,
                    struct GlftGlyph *left,
                    struct GlftGlyph *right);

#endif
