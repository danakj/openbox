#ifndef __render_h__
#define __render_h__

#include <glib.h>
#include <X11/Xlib.h>

/*
#define RrBool gboolean
#define RrTrue 1
#define RrFalse 0
*/

struct RrInstance;

/*! Returns a struct to be used when calling members of the library.
  If the library fails to initialize, NULL is returned.
  @param display The X Display to use.
  @param screen The number of the screen to use.
*/
struct RrInstance *RrInit(Display *display,
                          int screen);

/*! Destroys an instance of the library.
  @param inst The instance to destroy.
*/
void RrDestroy(struct RrInstance *inst);

#endif
