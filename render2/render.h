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
#define RrColorRed(c) (c)->r
/*! Returns the green component for an RrColor */
#define RrColorGreen(c) (c)->g
/*! Returns the blue component for an RrColor */
#define RrColorBlue(c) (c)->b
/*! Returns the alpha component for an RrColor */
#define RrColorAlpha(c) (c)->a

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

#endif
