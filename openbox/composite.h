/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   composite.h for the Openbox window manager
   Copyright (c) 2010        Dana Jansens
   Copyright (c) 2010        Derek Foreman

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

#ifndef __composite_h
#define __composite_h

#ifdef USE_COMPOSITING
#include <glib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "geom.h"

#define MAX_DEPTH 32

typedef GLXPixmap (*CreatePixmapT)(Display *display,
                                   GLXFBConfig config,
                                   int attribute,
                                   int *value);

typedef void (*BindTexImageT)(Display *display,
                              GLXDrawable drawable,
                              int buffer,
                              int *attriblist);

typedef void (*ReleaseTexImageT)(Display *display,
                                 GLXDrawable drawable,
                                 int buffer);

typedef GLXFBConfig *(*GetFBConfigsT) (Display *display,
                                       int screen,
                                       int *nElements);

typedef int (*GetFBConfigAttribT) (Display *display,
                                   GLXFBConfig config,
                                   int attribute,
                                   int *value);

struct ObCompositor {
    CreatePixmapT CreatePixmap;
    BindTexImageT BindTexImage;
    ReleaseTexImageT ReleaseTexImage;
    GetFBConfigsT GetFBConfigs;
    GetFBConfigAttribT GetFBConfigAttrib;

    Window overlay;

    GLXContext ctx;
    struct ObCompositeFBConfig {
        GLXFBConfig fbc; /* the fbconfig */
        gint tf;         /* texture format */
    } PixmapConfig[MAX_DEPTH + 1]; // need a [32]

    gboolean need_redraw;
    const Rect *screendims;
};
#endif

void composite_startup(gboolean reconfig);
void composite_shutdown(gboolean reconfig);

#endif
