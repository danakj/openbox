#ifndef __render_h__
#define __render_h__

#include <glib.h>
#include <X11/Xlib.h>

/* initialization */

struct RrInstance;

/*! Returns a struct to be used when calling members of the library.
  If the library fails to initialize, NULL is returned.
  @param display The X Display to use.
  @param screen The number of the screen to use.
*/
struct RrInstance *RrInit(Display *display,
                          int screen);

/*! Destroys an instance of the library. The instance should not be used after
  calling this function.
  @param inst The instance to destroy.
*/
void RrDestroy(struct RrInstance *inst);


/* colors */

/*! A Color (including alpha component) for the Render library. This should be
  treated as an opaque data type, and be accessed only via the available
  functions. */
struct RrColor {
    /*! The red component. */
    float r;
    /*! The green component. */
    float g;
    /*! The blue component. */
    float b;
    /*! The alpha component. */
    float a;
};

/*! Returns the red component for an RrColor */
#define RrColorRed(c) (c).r
/*! Returns the green component for an RrColor */
#define RrColorGreen(c) (c).g
/*! Returns the blue component for an RrColor */
#define RrColorBlue(c) (c).b
/*! Returns the alpha component for an RrColor */
#define RrColorAlpha(c) (c).a
/*! Returns if an RrColor is non-opaque */
#define RrColorHasAlpha(c) ((c).a > 0.0000001)

/*! Sets the values of all components for an RrColor */
#define RrColorSet(c, w, x, y, z) (c)->r = (w), (c)->g = (x), \
                                  (c)->b = (y), (c)->a = z


/*! Gets color values from a colorname.
  @param inst An instance of the library
  @param colorname The name of the color.
  @param ret The RrColor to set the colorvalues in.
  @return nonzero if the colorname could be parsed; on error, it returns zero.
*/
int RrColorParse(struct RrInstance *inst, const char *colorname,
                 struct RrColor *ret);

/* fonts */

struct RrFont;

struct RrFont *RrFontOpen(struct RrInstance *inst, const char *fontstring);
void RrFontClose(struct RrFont *font);

int RrFontMeasureString(struct RrFont *font, const char *string);
int RrFontHeight(struct RrFont *font);
int RrFontMaxCharWidth(struct RrFont *font);

/* surfaces */

struct RrSurface;

enum RrSurfaceType {
    RR_SURFACE_PLANAR,
    RR_SURFACE_NONPLANAR
};

/*! The options available for the background of an RrSurface */
enum RrSurfaceColorType {
    /*! No rendering on the surface background, its contents will be
      undefined. */
    RR_SURFACE_NONE,
    /*! Solid color fill. */
    RR_SURFACE_SOLID,
    /*! Horizontal gradient. */
    RR_SURFACE_HORIZONTAL,
    /*! Vertical gradient. */
    RR_SURFACE_VERTICAL,
    /*! Diagonal (TL->BR) gradient. */
    RR_SURFACE_DIAGONAL,
    /*! Cross-Diagonal (TR->BL) gradient. */
    RR_SURFACE_CROSSDIAGONAL,
    /*! Pipecross gradient. */
    RR_SURFACE_PIPECROSS,
    /*! Rectangle gradient. */
    RR_SURFACE_RECTANGLE,
    /*! Pyramid gradient. */
    RR_SURFACE_PYRAMID
};

/*! Create a new RrSurface prototype that can't render. A prototype can be
 copied to a new RrSurface that can render. */
struct RrSurface *RrSurfaceNewProto(enum RrSurfaceType type,
                                    int numtex);
/*! Create a new top-level RrSurface for a Window. */
struct RrSurface *RrSurfaceNew(struct RrInstance *inst,
                               enum RrSurfaceType type,
                               Window win,
                               int numtex);
/*! Create a new RrSurface which is a child of another. */
struct RrSurface *RrSurfaceNewChild(enum RrSurfaceType type,
                                    struct RrSurface *parent,
                                    int numtex);
/*! Copy an RrSurface, creating a new top-level RrSurface for a Window. */
struct RrSurface *RrSurfaceCopy(struct RrInstance *inst,
                                struct RrSurface *sur,
                                Window win);
/*! Copy an RrSurface, creating a nwe RrSurface which is a child of another. */
struct RrSurface *RrSurfaceCopyChild(struct RrSurface *sur,
                                     struct RrSurface *parent);
void RrSurfaceFree(struct RrSurface *sur);

void RrSurfaceSetArea(struct RrSurface *sur,
                      int x,
                      int y,
                      int w,
                      int h);

Window RrSurfaceWindow(struct RrSurface *sur);

/* textures */

enum RrLayout {
    RR_TOP_LEFT,
    RR_TOP,
    RR_TOP_RIGHT,
    RR_LEFT,
    RR_CENTER,
    RR_RIGHT,
    RR_BOTTOM_LEFT,
    RR_BOTTOM,
    RR_BOTTOM_RIGHT
};

#ifndef __LONG64
typedef long RrData32;
#else
typedef int RrData32;
#endif

void RrTextureSetRGBA(struct RrSurface *sur,
                      int texnum,
                      RrData32 *data,
                      int x,
                      int y,
                      int w,
                      int h);
void RrTextureSetText(struct RrSurface *sur,
                      int texnum,
                      struct RrFont *font,
                      enum RrLayout layout,
                      const char *text);

/* drawing */

/*! Paints the surface, and all its children */
void RrSurfacePaint(struct RrSurface *sur);
/*! Paints the surface, and all its children, but only in the given area. */
void RrSurfacePaintArea(struct RrSurface *sur,
                        int x,
                        int y,
                        int w,
                        int h);

#endif
