/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   extensions.h for the Openbox window manager
   Copyright (c) 2003        Ben Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#ifndef __extensions_h
#define __extensions_h

#include "geom.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h> /* shape.h uses Region which is in here */
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
extern gint extensions_xkb_event_basep;

/*! Does the display have the Shape extension? */
extern gboolean extensions_shape;
/*! Base for events for the Shape extension */
extern gint extensions_shape_event_basep;

/*! Does the display have the Xinerama extension? */
extern gboolean extensions_xinerama;
/*! Base for events for the Xinerama extension */
extern gint extensions_xinerama_event_basep;

/*! Does the display have the RandR extension? */
extern gboolean extensions_randr;
/*! Base for events for the Randr extension */
extern gint extensions_randr_event_basep;

/*! Does the display have the VidMode extension? */
extern gboolean extensions_vidmode;
/*! Base for events for the VidMode extension */
extern gint extensions_vidmode_event_basep;

void extensions_query_all();

void extensions_xinerama_screens(Rect **areas, guint *nxin);
  
#endif
