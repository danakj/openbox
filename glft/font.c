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

#define GLFT_SHADOW "shadow"
#define GLFT_SHADOW_OFFSET "shadowoffset"
#define GLFT_SHADOW_ALPHA "shadowalpha"

void dest_glyph_map_value(gpointer key, gpointer val, gpointer data)
{
    struct GlftGlyph *g = val;
    glDeleteTextures(1, &g->tnum);    
    free(g);
}

static void GlftDefaultSubstitute(Display *d, int s, FcPattern *pat)
{
    FcValue v;
    double dpi;

    if (FcPatternGet(pat, FC_DPI, 0, &v) == FcResultNoMatch) {
        dpi = DisplayHeight(d, s) * 25.4 / (double)DisplayHeightMM(d, s);
        FcPatternAddDouble(pat, FC_DPI, dpi);
    }
    if (FcPatternGet(pat, FC_ANTIALIAS, 0, &v) == FcResultNoMatch) {
        FcPatternAddBool(pat, FC_ANTIALIAS, FcFalse);
        g_message("SETTING ANTIALIAS TRUE");
    }
    if (FcPatternGet(pat, FC_HINTING, 0, &v) == FcResultNoMatch)
        FcPatternAddBool(pat, FC_HINTING, FcTrue);
    if (FcPatternGet(pat, FC_AUTOHINT, 0, &v) == FcResultNoMatch)
        FcPatternAddBool(pat, FC_AUTOHINT, FcFalse);
    if (FcPatternGet(pat, FC_GLOBAL_ADVANCE, 0, &v) == FcResultNoMatch)
        FcPatternAddBool(pat, FC_GLOBAL_ADVANCE, FcTrue);
    if (FcPatternGet(pat, FC_SPACING, 0, &v) == FcResultNoMatch)
        FcPatternAddInteger(pat, FC_SPACING, FC_PROPORTIONAL);
    if (FcPatternGet(pat, FC_MINSPACE, 0, &v) == FcResultNoMatch)
        FcPatternAddBool(pat, FC_MINSPACE, FcTrue);
    if (FcPatternGet(pat, FC_CHAR_WIDTH, 0, &v) == FcResultNoMatch)
        FcPatternAddInteger(pat, FC_CHAR_WIDTH, 0);
    if (FcPatternGet(pat, GLFT_SHADOW, 0, &v) == FcResultNoMatch)
        FcPatternAddBool(pat, GLFT_SHADOW, FcFalse);
    if (FcPatternGet(pat, GLFT_SHADOW_OFFSET, 0, &v) == FcResultNoMatch)
        FcPatternAddInteger(pat, GLFT_SHADOW_OFFSET, 2);
    if (FcPatternGet(pat, GLFT_SHADOW_ALPHA, 0, &v) == FcResultNoMatch)
        FcPatternAddDouble(pat, GLFT_SHADOW_ALPHA, 0.5);

    FcDefaultSubstitute(pat);
}

struct GlftFont *GlftFontOpen(Display *d, int screen, const char *name)
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
    GlftDefaultSubstitute(d, screen, pat);

    match = FcFontMatch(NULL, pat, &res);
    FcPatternDestroy(pat);
    if (!match) {
        GlftDebug("failed to find matching font\n");
        return NULL;
    }
    
    font = malloc(sizeof(struct GlftFont));
    font->display = d;
    font->screen = screen;
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
        font->minspace = FcTrue;
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

    if (FcPatternGetBool(match, GLFT_SHADOW, 0, &font->shadow) != FcResultMatch)
        font->shadow = FcFalse;

    if (FcPatternGetInteger(match,GLFT_SHADOW_OFFSET,0,&font->shadow_offset) !=
        FcResultMatch)
        font->shadow_offset = 2;

    if (FcPatternGetDouble(match, GLFT_SHADOW_ALPHA,0,&alpha) != FcResultMatch)
        alpha = 0.5;
    font->shadow_alpha = (float)alpha;

    /* time to load the font! */

    if (FT_New_Face(ft_lib, font->filename, font->index, &font->face)) {
        GlftDebug("failed to open FT face\n");
        goto openfail0;
    }
    assert(FT_IS_SCALABLE(font->face));
    if (!FT_IS_SCALABLE(font->face)) {
        GlftDebug("got a non-scalable face");
        goto openfail1;
    }
    if (FT_Set_Char_Size(font->face, 0, font->ftcharsize, 0, 0)) {
        GlftDebug("failed to set char size on FT face\n");
        goto openfail1;
    }

    if (!FcPatternGetCharSet(match, FC_CHARSET, 0, &font->chars) !=
        FcResultMatch)
        font->chars = FcFreeTypeCharSet(font->face, FcConfigGetBlanks(NULL));
    if (!font->chars) {
        GlftDebug("failed to get a valid CharSet\n");
        goto openfail1;
    }

    if (font->char_width)
        font->max_advance_width = font->char_width; 
    else
        font->max_advance_width = font->face->size->metrics.max_advance >> 6;
    font->descent = -(font->face->size->metrics.descender >> 6);
    font->ascent = font->face->size->metrics.ascender >> 6;
    if (font->minspace) font->height = font->ascent + font->descent;
    else                font->height = font->face->size->metrics.height >> 6;

    font->kerning = FT_HAS_KERNING(font->face);

    font->glyph_map = g_hash_table_new(g_int_hash, g_int_equal);

    return font;

openfail1:
    FT_Done_Face(font->face);
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
        g_hash_table_foreach(font->glyph_map, dest_glyph_map_value, NULL);
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
        g = malloc(sizeof(struct GlftGlyph));
        g->w = w;
        glGenTextures(1, &g->tnum);

        GlftRenderGlyph(font->face, g);

        if (!(font->spacing == FC_PROPORTIONAL)) {
            g->width = font->max_advance_width;
        } else {
            g->width = TRUNC(ROUND(font->face->glyph->metrics.horiAdvance));
        }
        g->x = TRUNC(FLOOR(font->face->glyph->metrics.horiBearingX));
        g->y = TRUNC(CEIL(font->face->glyph->metrics.horiBearingY));
        g->height = TRUNC(ROUND(font->face->glyph->metrics.height));

        g_hash_table_insert(font->glyph_map, &g->w, g);
    }

    return g;
}

int GlftFontAdvance(struct GlftFont *font,
                    struct GlftGlyph *left,
                    struct GlftGlyph *right)
{
    FT_Vector v;
    int k = 0;

    if (left) k+= left->width;

    if (right) {
        k -= right->x;
        if (font->kerning) {
            FT_Get_Kerning(font->face, left->glyph, right->glyph,
                           FT_KERNING_UNFITTED, &v);
            k += v.x >> 6;
        }
    }

    return k;
}

int GlftFontHeight(struct GlftFont *font)
{
    return font->height;
}

int GlftFontMaxCharWidth(struct GlftFont *font)
{
    return font->max_advance_width;
}
