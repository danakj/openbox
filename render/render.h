#ifndef __render_h
#define __render_h

#include <X11/Xlib.h>
#include <glib.h>
#include "color.h"
#include "kernel/geom.h"

typedef enum {
    Surface_Planar,
    Surface_Nonplanar
} SurfaceType;

typedef enum {
    Flat,
    Raised,
    Sunken
} ReliefType;

typedef enum {
    Bevel1,
    Bevel2
} BevelType;

typedef enum {
    Background_ParentRelative,
    Background_Solid,
    Background_Horizontal,
    Background_Vertical,
    Background_Diagonal,
    Background_CrossDiagonal,
    Background_PipeCross,
    Background_Rectangle,
    Background_Pyramid
} SurfaceColorType;

typedef enum {
    Bitmask,
    Text,
    RGBA,
    NoTexture
} TextureType;

struct Appearance;

typedef struct PlanarSurface {
    SurfaceColorType grad;
    ReliefType relief;
    BevelType bevel;
    struct RrRGB primary;
    struct RrRGB secondary;
    struct RrRGB border_color;
    struct RrRGB bevel_dark; 
    struct RrRGB bevel_light;
    gboolean interlaced;
    gboolean border;
    struct Appearance *parent;
    int parentx;
    int parenty;
} PlanarSurface;

typedef struct NonplanarSurface {
    int poo;
} NonplanarSurface;

typedef union {
    PlanarSurface planar;
    NonplanarSurface nonplanar;
} SurfaceData;

typedef struct Surface {
    SurfaceType type;
    SurfaceColorType colortype;
    SurfaceData data;
} Surface;

typedef struct {
    int height;
    int elipses_length;
} ObFont;

typedef enum {
    Justify_Center,
    Justify_Left,
    Justify_Right
} Justify;

typedef struct TextureText {
    ObFont *font;
    Justify justify;
    int shadow;
    char tint;
    unsigned char offset;
    struct RrRGB color;
    char *string;
} TextureText;   

typedef struct {
    Pixmap mask;
    guint w, h;
    char *data;
} pixmap_mask;

typedef struct TextureMask {
    struct RrRGB color;
    pixmap_mask *mask;
} TextureMask;

typedef struct TextureRGBA {
    guint width;
    guint height;
    pixel32 *data;
/* cached scaled so we don't have to scale often */
    guint cwidth;
    guint cheight;
    pixel32 *cache;
} TextureRGBA;

typedef union {
    TextureRGBA rgba;
    TextureText text;
    TextureMask mask;
} TextureData;

typedef struct Texture {
    Rect position;
    TextureType type;
    TextureData data;
} Texture;

typedef struct Appearance {
    Surface surface;
    Rect area;
    int textures;
    Texture *texture;
} Appearance;

extern Visual *render_visual;
extern XVisualInfo render_visual_info;
extern int render_depth;
extern Colormap render_colormap;

void (*paint)(Window win, Appearance *l);

void render_startup(void);
void init_appearance(Appearance *l);
void x_paint(Window win, Appearance *l);
void gl_paint(Window win, Appearance *l);
void render_shutdown(void);
Appearance *appearance_new(SurfaceType type, int numtex);
Appearance *appearance_copy(Appearance *a);
void appearance_free(Appearance *a);
void truecolor_startup(void);
void pseudocolor_startup(void);
void pixel32_to_pixmap(pixel32 *in, Pixmap out, int x, int y, int w, int h);

void appearance_minsize(Appearance *l, int *w, int *h);

gboolean render_pixmap_to_rgba(Pixmap pmap, Pixmap mask,
                               int *w, int *h, pixel32 **data);

#endif /*__render_h*/
