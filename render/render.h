#ifndef __render_h
#define __render_h

#include <X11/Xlib.h>
#define _XFT_NO_COMPAT_ /* no Xft 1 API */
#include <X11/Xft/Xft.h>
#include <glib.h>
#include "color.h"
#include "../kernel/geom.h"

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
    Background_Pyramid,
    Background_Elliptic
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
    color_rgb *primary;
    color_rgb *secondary;
    color_rgb *border_color;
    gboolean interlaced;
    gboolean border;
    struct Appearance *parent;
    int parentx;
    int parenty;
    pixel32 *pixel_data;
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
    XftFont *xftfont;
    int height;
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
    color_rgb *color;
    char *string;
} TextureText;   

typedef struct {
    Pixmap mask;
    guint w, h;
    char *data;
} pixmap_mask;

typedef struct TextureMask {
    color_rgb *color;
    pixmap_mask *mask;
} TextureMask;

typedef struct TextureRGBA {
    int width;
    int height;
    unsigned long *data;
/* cached scaled so we don't have to scale often */
    int cwidth;
    int cheight;
    unsigned long *cache;
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
    Pixmap pixmap;
    XftDraw *xftdraw;
} Appearance;

extern Visual *render_visual;
extern int render_depth;
extern Colormap render_colormap;

void (*paint)(Window win, Appearance *l);

void render_startup(void);
void init_appearance(Appearance *l);
void x_paint(Window win, Appearance *l);
void render_shutdown(void);
Appearance *appearance_new(SurfaceType type, int numtex);
Appearance *appearance_copy(Appearance *a);
void appearance_free(Appearance *a);
void truecolor_startup(void);
void pseudocolor_startup(void);
void pixel32_to_pixmap(pixel32 *in, Pixmap out, int x, int y, int w, int h);

void appearance_minsize(Appearance *l, Size *s);

#endif /*__render_h*/
