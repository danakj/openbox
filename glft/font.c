#include "font.h"
#include "glft.h"
#include "init.h"
#include "debug.h"
#include "render.h"
#include <assert.h>
#include <stdlib.h>
#include <GL/glx.h>

struct GHashTable *glyph_map = NULL;

#define FLOOR(x)    ((x) & -64)
#define CEIL(x)     (((x)+63) & -64)
#define TRUNC(x)    ((x) >> 6)
#define ROUND(x)    (((x)+32) & -64)

void dest_glyph_map_value(gpointer data)
{
    struct GlftGlyph *g = data;
    glDeleteLists(g->dlist, 1);    
    free(data);
}

struct GlftFont *GlftFontOpen(const char *name)
{
    struct GlftFont *font;
    FcPattern *pat, *match;
    FcResult res;
    double alpha, psize;
    FcBool hinting, autohint, advance;

    assert(init_done);

    pat = FcNameParse((const unsigned char*)name);
    assert(pat);

    /* XXX read our extended attributes here? (if failing below..) */

    FcConfigSubstitute(NULL, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    match = FcFontMatch(NULL, pat, &res);
    printf("Pattern ");
    FcPatternPrint(pat);
    printf("Match ");
    FcPatternPrint(match);
    FcPatternDestroy(pat);
    if (!match) {
        GlftDebug("failed to find matching font\n");
        return NULL;
    }
    
    font = malloc(sizeof(struct GlftFont));
    font->pat = match;
    font->ftflags = FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP;

    if (FcPatternGetString(match, FC_FILE, 0, (FcChar8**) &font->filename) !=
        FcResultMatch) {
        GlftDebug("error getting FC_FILE from pattern\n");
        goto openfail0;
    }

    switch (FcPatternGetInteger(match, FC_INDEX, 0, &font->index)) {
    case FcResultNoMatch:
        font->index = 0;
        break;
    case FcResultMatch:
        break;
    default:
        GlftDebug("error getting FC_INDEX from pattern\n");
        goto openfail0;
    }

    switch (FcPatternGetBool(match, FC_ANTIALIAS, 0, &font->antialias)) {
    case FcResultNoMatch:
        font->antialias = FcTrue;
        break;
    case FcResultMatch:
        break;
    default:
        GlftDebug("error getting FC_ANTIALIAS from pattern\n");
        goto openfail0;
    }

    switch (FcPatternGetBool(match, FC_HINTING, 0, &hinting)) {
    case FcResultNoMatch:
        hinting = FcTrue;
        break;
    case FcResultMatch:
        break;
    default:
        GlftDebug("error getting FC_HINTING from pattern\n");
        goto openfail0;
    }
    if (!hinting) font->ftflags |= FT_LOAD_NO_HINTING;

    switch (FcPatternGetBool(match, FC_AUTOHINT, 0, &autohint)) {
    case FcResultNoMatch:
        autohint = FcFalse;
        break;
    case FcResultMatch:
        break;
    default:
        GlftDebug("error getting FC_AUTOHINT from pattern\n");
        goto openfail0;
    }
    if (autohint) font->ftflags |= FT_LOAD_FORCE_AUTOHINT;

    switch (FcPatternGetBool(match, FC_GLOBAL_ADVANCE, 0, &advance)) {
    case FcResultNoMatch:
        advance = FcTrue;
        break;
    case FcResultMatch:
        break;
    default:
        GlftDebug("error getting FC_GLOBAL_ADVANCE from pattern\n");
        goto openfail0;
    }
    if (!advance) font->ftflags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;

    switch (FcPatternGetInteger(match, FC_SPACING, 0, &font->spacing)) {
    case FcResultNoMatch:
        font->spacing = FC_PROPORTIONAL;
        break;
    case FcResultMatch:
        break;
    default:
        GlftDebug("error getting FC_SPACING from pattern\n");
        goto openfail0;
    }

    switch (FcPatternGetBool(match, FC_MINSPACE, 0, &font->minspace)) {
    case FcResultNoMatch:
        font->minspace = FcFalse;
        break;
    case FcResultMatch:
        break;
    default:
        GlftDebug("error getting FC_MINSPACE from pattern\n");
        goto openfail0;
    }

    switch (FcPatternGetInteger(match, FC_CHAR_WIDTH, 0, &font->char_width)) {
    case FcResultNoMatch:
        font->char_width = 0;
        break;
    case FcResultMatch:
        if (font->char_width)
            font->spacing = FC_MONO;
        break;
    default:
        GlftDebug("error getting FC_CHAR_WIDTH from pattern\n");
        goto openfail0;
    }

    if (FcPatternGetDouble(match, FC_PIXEL_SIZE, 0, &psize) != FcResultMatch)
        goto openfail0;
    font->ftcharsize = (FT_F26Dot6) psize * 64;

    if (FcPatternGetBool(match, "shadow", 0, &font->shadow) != FcResultMatch)
        font->shadow = FcFalse;

    if (FcPatternGetInteger(match, "shadowoffset", 0, &font->shadow_offset) !=
        FcResultMatch)
        font->shadow_offset = 2;

    if (FcPatternGetDouble(match, "shadowalpha", 0, &alpha) != FcResultMatch)
        alpha = 0.5;
    font->shadow_alpha = (float)alpha;

    /* time to load the font! */

    if (FT_New_Face(ft_lib, font->filename, font->index, &font->face)) {
        GlftDebug("failed to open FT face\n");
        goto openfail0;
    }
    if (FT_Set_Char_Size(font->face, 0, font->ftcharsize, 0, 0)) {
        GlftDebug("failed to set char size on FT face\n");
        goto openfail0;
    }

    if (!FcPatternGetCharSet(match, FC_CHARSET, 0, &font->chars) !=
        FcResultMatch)
        font->chars = FcFreeTypeCharSet(font->face, FcConfigGetBlanks(NULL));
    if (!font->chars) {
        GlftDebug("failed to get a valid CharSet\n");
        goto openfail0;
    }

    font->glyph_map = g_hash_table_new_full(g_int_hash, g_int_equal, NULL,
                                            dest_glyph_map_value);

    if (font->char_width)
        font->max_advance_width = font->char_width; 
    else
        font->max_advance_width = font->face->size->metrics.max_advance >> 6;
    font->descent = -(font->face->size->metrics.descender >> 6);
    font->ascent = font->face->size->metrics.ascender >> 6;
    if (font->minspace) font->height = font->ascent + font->descent;
    else                font->height = font->face->size->metrics.height >> 6;

    return font;

openfail0:
    FcPatternDestroy(match);
    free(font);
    return NULL;
}

void GlftFontClose(struct GlftFont *font)
{
    if (font) {
        FT_Done_Face(font->face);
        FcPatternDestroy(font->pat);
        FcCharSetDestroy(font->chars);
        g_hash_table_destroy(font->glyph_map);
        free(font);
    }
}

struct GlftGlyph *GlftFontGlyph(struct GlftFont *font, const char *c)
{
    const char *n = g_utf8_next_char(c);
    int b = n - c;
    struct GlftGlyph *g;
    FcChar32 w;

    FcUtf8ToUcs4((const FcChar8*) c, &w, b);
    
    g = g_hash_table_lookup(font->glyph_map, &w);
    if (!g) {
        if (FT_Load_Glyph(font->face, FcFreeTypeCharIndex(font->face, w),
                          font->ftflags)) {
            if (FT_Load_Glyph(font->face, 0, font->ftflags))
                return NULL;
            else {
                g = g_hash_table_lookup(font->glyph_map, &w);
            }
        }
    }
    if (!g) {
        assert(font->face->face_flags & FT_FACE_FLAG_SCALABLE);

        g = malloc(sizeof(struct GlftGlyph));
        g->w = w;
        g->dlist = glGenLists(1);

        GlftRenderGlyph(font->face, g->dlist);

        if (!(font->spacing == FC_PROPORTIONAL)) {
            g->width = font->max_advance_width;
        } else {
            /*g->width = TRUNC(ROUND(font->face->glyph->advance.x));*/
            g->width = font->face->glyph->metrics.width >> 6;
        }
        g->height = -(font->face->glyph->metrics.height >> 6);

        g_hash_table_insert(font->glyph_map, &g->w, &g);
    }

    return g;
}

void GlftMeasureString(struct GlftFont *font,
                       const char *str,
                       int bytes,
                       int *w,
                       int *h)
{
    const char *c;
    struct GlftGlyph *g;

    if (!g_utf8_validate(str, bytes, NULL)) {
        GlftDebug("Invalid UTF-8 in string\n");
        return;
    }

    *w = 0;
    *h = 0;

    c = str;
    while (c) {
        g = GlftFontGlyph(font, c);
        if (g) {
            *w += g->width;
            *h = MAX(g->height, *h);
        } else {
            *w += font->max_advance_width;
        }
        c = g_utf8_next_char(c);
        if (c - str > bytes) break;
    }
}

