#ifndef __extensions_h
#define __extensions_h

#include <X11/Xlib.h>
#ifdef    XKB
#include <X11/XKBlib.h>
#endif
#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif
#ifdef    XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#include <glib.h>

/*! Does the display have the XKB extension? */
extern gboolean extensions_xkb;
/*! Base for events for the XKB extension */
extern int extensions_xkb_event_basep;

/*! Does the display have the Shape extension? */
extern gboolean extensions_shape;
/*! Base for events for the Shape extension */
extern int extensions_shape_event_basep;

/*! Does the display have the Xinerama extension? */
extern gboolean extensions_xinerama;
/*! Base for events for the Xinerama extension */
extern int extensions_xinerama_event_basep;

void extensions_query_all();
  
#endif
