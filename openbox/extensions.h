#ifndef __extensions_h
#define __extensions_h

#include "geom.h"

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
#ifdef    XRANDR
#include <X11/extensions/Xrandr.h>
#endif
#ifdef    VIDMODE
#include <X11/extensions/xf86vmode.h>
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

/*! Does the display have the RandR extension? */
extern gboolean extensions_randr;
/*! Base for events for the Randr extension */
extern int extensions_randr_event_basep;

/*! Does the display have the VidMode extension? */
extern gboolean extensions_vidmode;
/*! Base for events for the VidMode extension */
extern int extensions_vidmode_event_basep;

void extensions_query_all();

void extensions_xinerama_screens(Rect **areas, guint *nxin);
  
#endif
