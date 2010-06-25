/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   window.h for the Openbox window manager
   Copyright (c) 2003-2007   Dana Jansens

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

#ifndef __window_h
#define __window_h

#include "stacking.h"
#include "geom.h"

#include <X11/Xlib.h>
#include <glib.h>

#ifdef USE_COMPOSITING
#include <X11/extensions/Xdamage.h>
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#endif

typedef struct _ObWindow         ObWindow;
typedef struct _ObInternalWindow ObInternalWindow;

typedef enum {
    OB_WINDOW_CLASS_MENUFRAME,
    OB_WINDOW_CLASS_DOCK,
    OB_WINDOW_CLASS_CLIENT,
    OB_WINDOW_CLASS_INTERNAL,
    OB_WINDOW_CLASS_UNMANAGED,
    OB_WINDOW_CLASS_PROMPT
} ObWindowClass;

/* In order to be an ObWindow, you need to make this struct the top of your
   struct */
struct _ObWindow {
    ObWindowClass type;
    gsize bytes;

    /* abstract values implemented by subclasses */

    /*! Points to the top level Xwindow for the ObWindow */
    const Window *top;
    /*! Points to the topmost window that should be redirected for composite */
    const Window *redir;
    /*! Points to the stacking layer for the ObWindow */
    const ObStackingLayer *layer;
    /*! Points to the depth of the ObWindow */
    const int *depth;
    /*! Points to the alpha the ObWindow.
      0xffffffff is completely transparent, 0 is opaque.  If this is NULL,
      then the window will be considered opaque. */
    const guint32 *alpha;

#ifdef USE_COMPOSITING
    GLuint texture;
    Pixmap pixmap;
    GLXPixmap gpixmap;
    Damage damage;
    Rect area; /* area of the redirected window */
    Rect toparea; /* area of the top-level window */
    gint topborder; /* the border around the top-level window */
    XRectangle *rects; /* rects that make up the shape of the window */
    gint n_rects; /* number of objects in @rects */
    gboolean mapped;
    gboolean is_redir;
#endif
};

#define WINDOW_IS_MENUFRAME(win) \
    (((ObWindow*)win)->type == OB_WINDOW_CLASS_MENUFRAME)
#define WINDOW_IS_DOCK(win) \
    (((ObWindow*)win)->type == OB_WINDOW_CLASS_DOCK)
#define WINDOW_IS_CLIENT(win) \
    (((ObWindow*)win)->type == OB_WINDOW_CLASS_CLIENT)
#define WINDOW_IS_INTERNAL(win) \
    (((ObWindow*)win)->type == OB_WINDOW_CLASS_INTERNAL)
#define WINDOW_IS_UNMANAGED(win) \
    (((ObWindow*)win)->type == OB_WINDOW_CLASS_UNMANAGED)
#define WINDOW_IS_PROMPT(win) \
    (((ObWindow*)win)->type == OB_WINDOW_CLASS_PROMPT)

struct _ObMenu;
struct _ObDock;
struct _ObDockApp;
struct _ObClient;
struct _ObPrompt;

#define WINDOW_AS_MENUFRAME(win) ((struct _ObMenuFrame*)win)
#define WINDOW_AS_DOCK(win) ((struct _ObDock*)win)
#define WINDOW_AS_CLIENT(win) ((struct _ObClient*)win)
#define WINDOW_AS_INTERNAL(win) ((struct _ObInternalWindow*)win)
#define WINDOW_AS_UNMANAGED(win) ((struct _ObUnmanaged*)win)
#define WINDOW_AS_PROMPT(win) ((struct _ObPrompt*)win)

#define MENUFRAME_AS_WINDOW(menu) ((ObWindow*)menu)
#define DOCK_AS_WINDOW(dock) ((ObWindow*)dock)
#define CLIENT_AS_WINDOW(client) ((ObWindow*)client)
#define INTERNAL_AS_WINDOW(intern) ((ObWindow*)intern)
#define UNMANAGED_AS_WINDOW(um) ((ObWindow*)um)
#define PROMPT_AS_WINDOW(prompt) ((ObWindow*)prompt)

void window_startup (gboolean reconfig);
void window_shutdown(gboolean reconfig);

#define window_new(c, t) ((t*)window_new_size((c), sizeof(t)))
ObWindow* window_new_size(ObWindowClass type, gsize size);
/*! A subclass of ObWindow must call this to set these pointers during the
  initialization/creation phase, so that the ObWindow can be used */
void      window_set_abstract(ObWindow *self,
                              const Window *top,
                              const Window *redir,
                              const ObStackingLayer *layer,
                              const int *depth,
                              const guint32 *alpha);
/*! A subclass of ObWindow must call this before setting its top-level
  window, to the top-level window's initial position/size/border */
void      window_set_top_area(ObWindow *self, const Rect *r, gint border);
/*! A subclass of ObWindow must call this when it is going to be destroying
  itself, but _before_ it destroys the members it sets in
  window_set_abstract() */
void      window_cleanup(ObWindow *self);
void      window_free(ObWindow *self);

ObWindow* window_find  (Window xwin);
void      window_add   (Window *xwin, ObWindow *win);
void      window_remove(Window xwin);

typedef void (*ObWindowForeachFunc)(ObWindow *w);

void      window_foreach(ObWindowForeachFunc func);

#define window_top(w) (*((ObWindow*)w)->top)
#define window_redir(w) (*((ObWindow*)w)->redir)
#define window_layer(w) (*((ObWindow*)w)->layer)
#define window_area(w) (*((ObWindow*)w)->area)
#define window_depth(w) (*((ObWindow*)w)->depth)

void window_adjust_redir_shape(ObWindow *self);

/* Internal openbox-owned windows like the alt-tab popup */
struct _ObInternalWindow {
    ObWindow super;
    Window window;
    ObStackingLayer layer;
    int depth;
};

ObInternalWindow* window_internal_new(Window window, const Rect *area,
                                      gint border, gint depth);

void window_manage_all(void);
void window_manage(Window win);
void window_unmanage_all(void);

#endif
