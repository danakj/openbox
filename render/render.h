#ifndef __render_h
#define __render_h

#define _XFT_NO_COMPAT_ /* no Xft 1 API */
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <glib.h>

typedef union  _RrTextureData      RrTextureData;
typedef struct _RrAppearance       RrAppearance;
typedef struct _RrSurface          RrSurface;
typedef struct _RrFont             RrFont;
typedef struct _RrTexture          RrTexture;
typedef struct _RrTextureMask      RrTextureMask;
typedef struct _RrTextureRGBA      RrTextureRGBA;
typedef struct _RrTextureText      RrTextureText;
typedef struct _RrPixmapMask       RrPixmapMask;
typedef struct _RrInstance         RrInstance;
typedef struct _RrColor            RrColor;

typedef guint32 RrPixel32;
typedef guint16 RrPixel16;

typedef enum {
    RR_RELIEF_FLAT,
    RR_RELIEF_RAISED,
    RR_RELIEF_SUNKEN
} RrReliefType;

typedef enum {
    RR_BEVEL_1,
    RR_BEVEL_2
} RrBevelType;

typedef enum {
    RR_SURFACE_NONE,
    RR_SURFACE_PARENTREL,
    RR_SURFACE_SOLID,
    RR_SURFACE_HORIZONTAL,
    RR_SURFACE_VERTICAL,
    RR_SURFACE_DIAGONAL,
    RR_SURFACE_CROSS_DIAGONAL,
    RR_SURFACE_PYRAMID
} RrSurfaceColorType;

typedef enum {
    RR_TEXTURE_NONE,
    RR_TEXTURE_MASK,
    RR_TEXTURE_TEXT,
    RR_TEXTURE_RGBA
} RrTextureType;

typedef enum {
    RR_JUSTIFY_LEFT,
    RR_JUSTIFY_CENTER,
    RR_JUSTIFY_RIGHT
} RrJustify;

struct _RrSurface {
    RrSurfaceColorType grad;
    RrReliefType relief;
    RrBevelType bevel;
    RrColor *primary;
    RrColor *secondary;
    RrColor *border_color;
    RrColor *bevel_dark; 
    RrColor *bevel_light;
    gboolean interlaced;
    gboolean border;
    RrAppearance *parent;
    gint parentx;
    gint parenty;
    RrPixel32 *RrPixel_data;
};

struct _RrTextureText {
    RrFont *font;
    RrJustify justify;
    gint shadow;
    gchar tint;
    guchar offset;
    RrColor *color;
    gchar *string;
};

struct _RrPixmapMask {
    const RrInstance *inst;
    Pixmap mask;
    gint width;
    gint height;
    gchar *data;
};

struct _RrTextureMask {
    RrColor *color;
    RrPixmapMask *mask;
};

struct _RrTextureRGBA {
    gint width;
    gint height;
    RrPixel32 *data;
/* cached scaled so we don't have to scale often */
    gint cwidth;
    gint cheight;
    RrPixel32 *cache;
};

union _RrTextureData {
    RrTextureRGBA rgba;
    RrTextureText text;
    RrTextureMask mask;
};

struct _RrTexture {
    RrTextureType type;
    RrTextureData data;
};

struct _RrAppearance {
    const RrInstance *inst;

    RrSurface surface;
    gint textures;
    RrTexture *texture;
    Pixmap pixmap;
    XftDraw *xftdraw;

    /* cached for internal use */
    gint w, h;
};

#if (G_BYTE_ORDER == G_BIG_ENDIAN)
#define RrDefaultRedOffset 0
#define RrDefaultGreenOffset 8
#define RrDefaultBlueOffset 16
#define RrDefaultAlphaOffset 24
#define RrEndian MSBFirst  
#else
#define RrDefaultAlphaOffset 24
#define RrDefaultRedOffset 16
#define RrDefaultGreenOffset 8
#define RrDefaultBlueOffset 0
#define RrEndian LSBFirst
#endif /* G_BYTE_ORDER == G_BIG_ENDIAN */

RrInstance* RrInstanceNew (Display *display, gint screen);
void        RrInstanceFree (RrInstance *inst);

Display* RrDisplay      (const RrInstance *inst);
gint     RrScreen       (const RrInstance *inst);
Window   RrRootWindow   (const RrInstance *inst);
Visual*  RrVisual       (const RrInstance *inst);
gint     RrDepth        (const RrInstance *inst);
Colormap RrColormap     (const RrInstance *inst);
gint     RrRedOffset    (const RrInstance *inst);
gint     RrGreenOffset  (const RrInstance *inst);
gint     RrBlueOffset   (const RrInstance *inst);
gint     RrRedShift     (const RrInstance *inst);
gint     RrGreenShift   (const RrInstance *inst);
gint     RrBlueShift    (const RrInstance *inst);
gint     RrRedMask      (const RrInstance *inst);
gint     RrGreenMask    (const RrInstance *inst);
gint     RrBlueMask     (const RrInstance *inst);
guint    RrPseudoBPC    (const RrInstance *inst);
XColor*  RrPseudoColors (const RrInstance *inst);

RrColor *RrColorNew   (const RrInstance *inst, gint r, gint g, gint b);
RrColor *RrColorParse (const RrInstance *inst, gchar *colorname);
void     RrColorFree  (RrColor *in);

RrAppearance *RrAppearanceNew  (const RrInstance *inst, gint numtex);
RrAppearance *RrAppearanceCopy (RrAppearance *a);
void          RrAppearanceFree (RrAppearance *a);

int RrFontMeasureString (const RrFont *f, const gchar *str,
                         gint shadow, gint offset);
int RrFontHeight        (const RrFont *f, gint shadow, gint offset);
int RrFontMaxCharWidth  (const RrFont *f);

void RrPaint   (RrAppearance *l, Window win, gint w, gint h);
void RrMinsize (RrAppearance *l, gint *w, gint *h);

gboolean RrPixmapToRGBA(const RrInstance *inst,
                        Pixmap pmap, Pixmap mask,
                        gint *w, gint *h, RrPixel32 **data);

#endif /*__render_h*/
