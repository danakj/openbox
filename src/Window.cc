// Window.cc for Openbox
// Copyright (c) 2001 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// stupid macros needed to access some functions in version 2 of the GNU C
// library
#ifndef   _GNU_SOURCE
#define   _GNU_SOURCE
#endif // _GNU_SOURCE

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include <X11/Xatom.h>
#include <X11/keysym.h>

#ifdef    HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#ifdef    DEBUG
#  ifdef    HAVE_STDIO_H
#    include <stdio.h>
#  endif // HAVE_STDIO_H
#endif // DEBUG

#include "i18n.h"
#include "openbox.h"
#include "Iconmenu.h"
#include "Screen.h"
#include "Toolbar.h"
#include "Window.h"
#include "Windowmenu.h"
#include "Workspace.h"
#ifdef    SLIT
#  include "Slit.h"
#endif // SLIT
#include "Util.h"

/*
 * Initializes the class with default values/the window's set initial values.
 */
OpenboxWindow::OpenboxWindow(Openbox &o, Window w, BScreen *s) : openbox(o) {
#ifdef    DEBUG
  fprintf(stderr, i18n->getMessage(WindowSet, WindowCreating,
		     "OpenboxWindow::OpenboxWindow(): creating 0x%lx\n"),
	  w);
#endif // DEBUG

  client.window = w;
  display = openbox.getXDisplay();

  openbox.grab();
  if (! validateClient()) return;

  // fetch client size and placement
  XWindowAttributes wattrib;
  if ((! XGetWindowAttributes(display, client.window, &wattrib)) ||
      (! wattrib.screen) || wattrib.override_redirect) {
#ifdef    DEBUG
    fprintf(stderr,
	    i18n->getMessage(WindowSet, WindowXGetWindowAttributesFail,
	       "OpenboxWindow::OpenboxWindow(): XGetWindowAttributes "
	       "failed\n"));
#endif // DEBUG

    openbox.ungrab();
    return;
  }

  if (s) {
    screen = s;
  } else {
    screen = openbox.searchScreen(RootWindowOfScreen(wattrib.screen));
    if (! screen) {
#ifdef    DEBUG
      fprintf(stderr, i18n->getMessage(WindowSet, WindowCannotFindScreen,
		      "OpenboxWindow::OpenboxWindow(): can't find screen\n"
		      "\tfor root window 0x%lx\n"),
	              RootWindowOfScreen(wattrib.screen));
#endif // DEBUG

      openbox.ungrab();
      return;
    }
  }

  flags.moving = flags.resizing = flags.shaded = flags.visible =
    flags.iconic = flags.transient = flags.focused =
    flags.stuck = flags.modal =  flags.send_focus_message =
    flags.shaped = flags.managed = False;
  flags.maximized = 0;

  openbox_attrib.workspace = workspace_number = window_number = -1;

  openbox_attrib.flags = openbox_attrib.attrib = openbox_attrib.stack
    = openbox_attrib.decoration = 0l;
  openbox_attrib.premax_x = openbox_attrib.premax_y = 0;
  openbox_attrib.premax_w = openbox_attrib.premax_h = 0;

  frame.window = frame.plate = frame.title = frame.handle = None;
  frame.close_button = frame.iconify_button = frame.maximize_button = None;
  frame.right_grip = frame.left_grip = None;

  frame.utitle = frame.ftitle = frame.uhandle = frame.fhandle = None;
  frame.ulabel = frame.flabel = frame.ubutton = frame.fbutton = None;
  frame.pbutton = frame.ugrip = frame.fgrip = None;

  decorations.titlebar = decorations.border = decorations.handle = True;
  decorations.iconify = decorations.maximize = decorations.menu = True;
  functions.resize = functions.move = functions.iconify =
    functions.maximize = True;
  functions.close = decorations.close = False;

  client.wm_hint_flags = client.normal_hint_flags = 0;
  client.transient_for = client.transient = 0;
  client.title = 0;
  client.title_len = 0;
  client.icon_title = 0;
  client.mwm_hint = (MwmHints *) 0;
  client.openbox_hint = (OpenboxHints *) 0;

  // get the initial size and location of client window (relative to the
  // _root window_). This position is the reference point used with the
  // window's gravity to find the window's initial position.
  client.x = wattrib.x;
  client.y = wattrib.y;
  client.width = wattrib.width;
  client.height = wattrib.height;
  client.old_bw = wattrib.border_width;

  windowmenu = 0;
  lastButtonPressTime = 0;
  image_ctrl = screen->getImageControl();

  timer = new BTimer(openbox, *this);
  timer->setTimeout(openbox.getAutoRaiseDelay());
  timer->fireOnce(True);

  getOpenboxHints();
  if (! client.openbox_hint)
    getMWMHints();

  // get size, aspect, minimum/maximum size and other hints set by the
  // client
  getWMProtocols();
  getWMHints();
  getWMNormalHints();

#ifdef    SLIT
  if (client.initial_state == WithdrawnState) {
    screen->getSlit()->addClient(client.window);
    openbox.ungrab();
    delete this;
    return;
  }
#endif // SLIT

  flags.managed = True;
  openbox.saveWindowSearch(client.window, this);

  // determine if this is a transient window
  Window win;
  if (XGetTransientForHint(display, client.window, &win)) {
    if (win && (win != client.window)) {
      OpenboxWindow *tr;
      if ((tr = openbox.searchWindow(win))) {
	while (tr->client.transient) tr = tr->client.transient;
	client.transient_for = tr;
	tr->client.transient = this;
	flags.stuck = client.transient_for->flags.stuck;
	flags.transient = True;
      } else if (win == client.window_group) {
	if ((tr = openbox.searchGroup(win, this))) {
	  while (tr->client.transient) tr = tr->client.transient;
	  client.transient_for = tr;
	  tr->client.transient = this;
	  flags.stuck = client.transient_for->flags.stuck;
	  flags.transient = True;
	}
      }
    }

    if (win == screen->getRootWindow()) flags.modal = True;
  }

  // adjust the window decorations based on transience and window sizes
  if (flags.transient)
    decorations.maximize = decorations.handle = functions.maximize = False;

  if ((client.normal_hint_flags & PMinSize) &&
      (client.normal_hint_flags & PMaxSize) &&
       client.max_width <= client.min_width &&
      client.max_height <= client.min_height) {
    decorations.maximize = decorations.handle =
      functions.resize = functions.maximize = False;
  }
  upsize();

  Bool place_window = True;
  if (openbox.isStartup() || flags.transient ||
      client.normal_hint_flags & (PPosition|USPosition)) {
    setGravityOffsets();

    if ((openbox.isStartup()) ||
	(frame.x >= 0 &&
	 (signed) (frame.y + frame.y_border) >= 0 &&
	 frame.x <= (signed) screen->size().w() &&
	 frame.y <= (signed) screen->size().h()))
      place_window = False;
  }

  frame.window = createToplevelWindow(frame.x, frame.y, frame.width,
				      frame.height,
				      frame.border_w);
  openbox.saveWindowSearch(frame.window, this);

  frame.plate = createChildWindow(frame.window);
  openbox.saveWindowSearch(frame.plate, this);

  if (decorations.titlebar) {
    frame.title = createChildWindow(frame.window);
    frame.label = createChildWindow(frame.title);
    openbox.saveWindowSearch(frame.title, this);
    openbox.saveWindowSearch(frame.label, this);
  }

  if (decorations.handle) {
    frame.handle = createChildWindow(frame.window);
    openbox.saveWindowSearch(frame.handle, this);

    frame.left_grip =
      createChildWindow(frame.handle, openbox.getLowerLeftAngleCursor());
    openbox.saveWindowSearch(frame.left_grip, this);

    frame.right_grip =
      createChildWindow(frame.handle, openbox.getLowerRightAngleCursor());
    openbox.saveWindowSearch(frame.right_grip, this);
  }

  associateClientWindow();

  if (! screen->sloppyFocus())
    openbox.grabButton(Button1, 0, frame.plate, True, ButtonPressMask,
        GrabModeSync, GrabModeSync, None, None);

  openbox.grabButton(Button1, Mod1Mask, frame.window, True,
      ButtonReleaseMask | ButtonMotionMask, GrabModeAsync,
      GrabModeAsync, None, openbox.getMoveCursor());
  openbox.grabButton(Button2, Mod1Mask, frame.window, True,
      ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
  openbox.grabButton(Button3, Mod1Mask, frame.window, True,
      ButtonReleaseMask | ButtonMotionMask, GrabModeAsync,
      GrabModeAsync, None, None);

  positionWindows();
  XRaiseWindow(display, frame.plate);
  XMapSubwindows(display, frame.plate);
  if (decorations.titlebar) XMapSubwindows(display, frame.title);
  XMapSubwindows(display, frame.window);

  if (decorations.menu)
    windowmenu = new Windowmenu(*this);

  decorate();

  if (workspace_number < 0 || workspace_number >= screen->getWorkspaceCount())
    screen->getCurrentWorkspace()->addWindow(this, place_window);
  else
    screen->getWorkspace(workspace_number)->addWindow(this, place_window);

  configure(frame.x, frame.y, frame.width, frame.height);

  if (flags.shaded) {
    flags.shaded = False;
    shade();
  }

  if (flags.maximized && functions.maximize) {
    unsigned int button = flags.maximized;
    flags.maximized = 0;
    maximize(button);
  }

  setFocusFlag(False);

  openbox.ungrab();
}


OpenboxWindow::~OpenboxWindow(void) {
  if (flags.moving || flags.resizing) {
    screen->hideGeometry();
    XUngrabPointer(display, CurrentTime);
  }

  if (workspace_number != -1 && window_number != -1)
    screen->getWorkspace(workspace_number)->removeWindow(this);
  else if (flags.iconic)
    screen->removeIcon(this);

  if (timer) {
    if (timer->isTiming()) timer->stop();
    delete timer;
  }

  if (windowmenu) delete windowmenu;

  if (client.title)
    delete [] client.title;

  if (client.icon_title)
    delete [] client.icon_title;

  if (client.mwm_hint)
    XFree(client.mwm_hint);

  if (client.openbox_hint)
    XFree(client.openbox_hint);

  if (client.window_group)
    openbox.removeGroupSearch(client.window_group);

  if (flags.transient && client.transient_for)
    client.transient_for->client.transient = client.transient;
  if (client.transient)
    client.transient->client.transient_for = client.transient_for;

  if (frame.close_button) {
    openbox.removeWindowSearch(frame.close_button);
    XDestroyWindow(display, frame.close_button);
  }

  if (frame.iconify_button) {
    openbox.removeWindowSearch(frame.iconify_button);
    XDestroyWindow(display, frame.iconify_button);
  }

  if (frame.maximize_button) {
    openbox.removeWindowSearch(frame.maximize_button);
    XDestroyWindow(display, frame.maximize_button);
  }

  if (frame.title) {
    if (frame.ftitle)
      image_ctrl->removeImage(frame.ftitle);

    if (frame.utitle)
      image_ctrl->removeImage(frame.utitle);

    if (frame.flabel)
      image_ctrl->removeImage(frame.flabel);

    if( frame.ulabel)
      image_ctrl->removeImage(frame.ulabel);

    openbox.removeWindowSearch(frame.label);
    openbox.removeWindowSearch(frame.title);
    XDestroyWindow(display, frame.label);
    XDestroyWindow(display, frame.title);
  }

  if (frame.handle) {
    if (frame.fhandle)
      image_ctrl->removeImage(frame.fhandle);

    if (frame.uhandle)
      image_ctrl->removeImage(frame.uhandle);

    if (frame.fgrip)
      image_ctrl->removeImage(frame.fgrip);

    if (frame.ugrip)
      image_ctrl->removeImage(frame.ugrip);

    openbox.removeWindowSearch(frame.handle);
    openbox.removeWindowSearch(frame.right_grip);
    openbox.removeWindowSearch(frame.left_grip);
    XDestroyWindow(display, frame.right_grip);
    XDestroyWindow(display, frame.left_grip);
    XDestroyWindow(display, frame.handle);
  }

  if (frame.fbutton)
    image_ctrl->removeImage(frame.fbutton);

  if (frame.ubutton)
    image_ctrl->removeImage(frame.ubutton);

  if (frame.pbutton)
    image_ctrl->removeImage(frame.pbutton);

  if (frame.plate) {
    openbox.removeWindowSearch(frame.plate);
    XDestroyWindow(display, frame.plate);
  }

  if (frame.window) {
    openbox.removeWindowSearch(frame.window);
    XDestroyWindow(display, frame.window);
  }

  if (flags.managed) {
    openbox.removeWindowSearch(client.window);
    screen->removeNetizen(client.window);
  }
}


/*
 * Creates a new top level window, with a given location, size, and border
 * width.
 * Returns: the newly created window
 */
Window OpenboxWindow::createToplevelWindow(int x, int y, unsigned int width,
					    unsigned int height,
					    unsigned int borderwidth)
{
  XSetWindowAttributes attrib_create;
  unsigned long create_mask = CWBackPixmap | CWBorderPixel | CWColormap |
                              CWOverrideRedirect | CWEventMask;

  attrib_create.background_pixmap = None;
  attrib_create.colormap = screen->getColormap();
  attrib_create.override_redirect = True;
  attrib_create.event_mask = ButtonPressMask | ButtonReleaseMask |
                             ButtonMotionMask | EnterWindowMask;

  return XCreateWindow(display, screen->getRootWindow(), x, y, width, height,
			borderwidth, screen->getDepth(), InputOutput,
			screen->getVisual(), create_mask,
			&attrib_create);
}


/*
 * Creates a child window, and optionally associates a given cursor with
 * the new window.
 */
Window OpenboxWindow::createChildWindow(Window parent, Cursor cursor) {
  XSetWindowAttributes attrib_create;
  unsigned long create_mask = CWBackPixmap | CWBorderPixel |
                              CWEventMask;

  attrib_create.background_pixmap = None;
  attrib_create.event_mask = ButtonPressMask | ButtonReleaseMask |
                             ButtonMotionMask | ExposureMask |
                             EnterWindowMask | LeaveWindowMask;

  if (cursor) {
    create_mask |= CWCursor;
    attrib_create.cursor = cursor;
  }

  return XCreateWindow(display, parent, 0, 0, 1, 1, 0, screen->getDepth(),
		       InputOutput, screen->getVisual(), create_mask,
		       &attrib_create);
}


void OpenboxWindow::associateClientWindow(void) {
  XSetWindowBorderWidth(display, client.window, 0);
  getWMName();
  getWMIconName();

  XChangeSaveSet(display, client.window, SetModeInsert);
  XSetWindowAttributes attrib_set;

  XSelectInput(display, frame.plate, NoEventMask);
  XReparentWindow(display, client.window, frame.plate, 0, 0);
  XSelectInput(display, frame.plate, SubstructureRedirectMask);

  XFlush(display);

  attrib_set.event_mask = PropertyChangeMask | StructureNotifyMask |
                          FocusChangeMask;
  attrib_set.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask |
                                     ButtonMotionMask;

  XChangeWindowAttributes(display, client.window, CWEventMask|CWDontPropagate,
                          &attrib_set);

#ifdef    SHAPE
  if (openbox.hasShapeExtensions()) {
    XShapeSelectInput(display, client.window, ShapeNotifyMask);

    int foo;
    unsigned int ufoo;

    XShapeQueryExtents(display, client.window, &flags.shaped, &foo, &foo,
		       &ufoo, &ufoo, &foo, &foo, &foo, &ufoo, &ufoo);

    if (flags.shaped) {
      XShapeCombineShape(display, frame.window, ShapeBounding,
			 frame.mwm_border_w, frame.y_border +
			 frame.mwm_border_w, client.window,
			 ShapeBounding, ShapeSet);

      int num = 1;
      XRectangle xrect[2];
      xrect[0].x = xrect[0].y = 0;
      xrect[0].width = frame.width;
      xrect[0].height = frame.y_border;

      if (decorations.handle) {
	xrect[1].x = 0;
	xrect[1].y = frame.y_handle;
	xrect[1].width = frame.width;
	xrect[1].height = frame.handle_h + frame.border_w;
	num++;
      }

      XShapeCombineRectangles(display, frame.window, ShapeBounding, 0, 0,
			      xrect, num, ShapeUnion, Unsorted);
    }
  }
#endif // SHAPE

  if (decorations.iconify) createIconifyButton();
  if (decorations.maximize) createMaximizeButton();
  if (decorations.close) createCloseButton();

  if (frame.ubutton) {
    if (frame.close_button)
      XSetWindowBackgroundPixmap(display, frame.close_button, frame.ubutton);
    if (frame.maximize_button)
      XSetWindowBackgroundPixmap(display, frame.maximize_button,
				 frame.ubutton);
    if (frame.iconify_button)
      XSetWindowBackgroundPixmap(display, frame.iconify_button, frame.ubutton);
  } else {
    if (frame.close_button)
      XSetWindowBackground(display, frame.close_button, frame.ubutton_pixel);
    if (frame.maximize_button)
      XSetWindowBackground(display, frame.maximize_button,
			   frame.ubutton_pixel);
    if (frame.iconify_button)
      XSetWindowBackground(display, frame.iconify_button, frame.ubutton_pixel);
  }
}


void OpenboxWindow::decorate(void) {
  Pixmap tmp = frame.fbutton;
  BTexture *texture = &(screen->getWindowStyle()->b_focus);
  if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
    frame.fbutton = None;
    frame.fbutton_pixel = texture->getColor()->getPixel();
  } else {
    frame.fbutton =
      image_ctrl->renderImage(frame.button_w, frame.button_h, texture);
  }
  if (tmp) image_ctrl->removeImage(tmp);

  tmp = frame.ubutton;
  texture = &(screen->getWindowStyle()->b_unfocus);
  if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
    frame.ubutton = None;
    frame.ubutton_pixel = texture->getColor()->getPixel();
  } else {
    frame.ubutton =
      image_ctrl->renderImage(frame.button_w, frame.button_h, texture);
  }
  if (tmp) image_ctrl->removeImage(tmp);

  tmp = frame.pbutton;
  texture = &(screen->getWindowStyle()->b_pressed);
  if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
    frame.pbutton = None;
    frame.pbutton_pixel = texture->getColor()->getPixel();
  } else {
    frame.pbutton =
      image_ctrl->renderImage(frame.button_w, frame.button_h, texture);
  }
  if (tmp) image_ctrl->removeImage(tmp);

  if (decorations.titlebar) {
    tmp = frame.ftitle;
    texture = &(screen->getWindowStyle()->t_focus);
    if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
      frame.ftitle = None;
      frame.ftitle_pixel = texture->getColor()->getPixel();
    } else {
      frame.ftitle =
        image_ctrl->renderImage(frame.width, frame.title_h, texture);
    }
    if (tmp) image_ctrl->removeImage(tmp);

    tmp = frame.utitle;
    texture = &(screen->getWindowStyle()->t_unfocus);
    if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
      frame.utitle = None;
      frame.utitle_pixel = texture->getColor()->getPixel();
    } else {
      frame.utitle =
        image_ctrl->renderImage(frame.width, frame.title_h, texture);
    }
    if (tmp) image_ctrl->removeImage(tmp);

    XSetWindowBorder(display, frame.title,
                     screen->getBorderColor()->getPixel());

    decorateLabel();
  }

  if (decorations.border) {
    frame.fborder_pixel = screen->getWindowStyle()->f_focus.getPixel();
    frame.uborder_pixel = screen->getWindowStyle()->f_unfocus.getPixel();
    openbox_attrib.flags |= AttribDecoration;
    openbox_attrib.decoration = DecorNormal;
  } else {
    openbox_attrib.flags |= AttribDecoration;
    openbox_attrib.decoration = DecorNone;
  }

  if (decorations.handle) {
    tmp = frame.fhandle;
    texture = &(screen->getWindowStyle()->h_focus);
    if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
      frame.fhandle = None;
      frame.fhandle_pixel = texture->getColor()->getPixel();
    } else {
      frame.fhandle =
        image_ctrl->renderImage(frame.width, frame.handle_h, texture);
    }
    if (tmp) image_ctrl->removeImage(tmp);

    tmp = frame.uhandle;
    texture = &(screen->getWindowStyle()->h_unfocus);
    if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
      frame.uhandle = None;
      frame.uhandle_pixel = texture->getColor()->getPixel();
    } else {
      frame.uhandle =
        image_ctrl->renderImage(frame.width, frame.handle_h, texture);
    }
    if (tmp) image_ctrl->removeImage(tmp);

    tmp = frame.fgrip;
    texture = &(screen->getWindowStyle()->g_focus);
    if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
      frame.fgrip = None;
      frame.fgrip_pixel = texture->getColor()->getPixel();
    } else {
      frame.fgrip =
        image_ctrl->renderImage(frame.grip_w, frame.grip_h, texture);
    }
    if (tmp) image_ctrl->removeImage(tmp);

    tmp = frame.ugrip;
    texture = &(screen->getWindowStyle()->g_unfocus);
    if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
      frame.ugrip = None;
      frame.ugrip_pixel = texture->getColor()->getPixel();
    } else {
      frame.ugrip =
        image_ctrl->renderImage(frame.grip_w, frame.grip_h, texture);
    }
    if (tmp) image_ctrl->removeImage(tmp);

    XSetWindowBorder(display, frame.handle,
                     screen->getBorderColor()->getPixel());
    XSetWindowBorder(display, frame.left_grip,
                     screen->getBorderColor()->getPixel());
    XSetWindowBorder(display, frame.right_grip,
                     screen->getBorderColor()->getPixel());
  }

  XSetWindowBorder(display, frame.window,
                   screen->getBorderColor()->getPixel());
}


void OpenboxWindow::decorateLabel(void) {
  Pixmap tmp = frame.flabel;
  BTexture *texture = &(screen->getWindowStyle()->l_focus);
  if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
    frame.flabel = None;
    frame.flabel_pixel = texture->getColor()->getPixel();
  } else {
    frame.flabel =
      image_ctrl->renderImage(frame.label_w, frame.label_h, texture);
  }
  if (tmp) image_ctrl->removeImage(tmp);

  tmp = frame.ulabel;
  texture = &(screen->getWindowStyle()->l_unfocus);
  if (texture->getTexture() == (BImage_Flat | BImage_Solid)) {
    frame.ulabel = None;
    frame.ulabel_pixel = texture->getColor()->getPixel();
  } else {
    frame.ulabel =
      image_ctrl->renderImage(frame.label_w, frame.label_h, texture);
  }
  if (tmp) image_ctrl->removeImage(tmp);
}


void OpenboxWindow::createCloseButton(void) {
  if (decorations.close && frame.title != None) {
    frame.close_button = createChildWindow(frame.title);
    openbox.saveWindowSearch(frame.close_button, this);
  }
}


void OpenboxWindow::createIconifyButton(void) {
  if (decorations.iconify && frame.title != None) {
    frame.iconify_button = createChildWindow(frame.title);
    openbox.saveWindowSearch(frame.iconify_button, this);
  }
}


void OpenboxWindow::createMaximizeButton(void) {
  if (decorations.maximize && frame.title != None) {
    frame.maximize_button = createChildWindow(frame.title);
    openbox.saveWindowSearch(frame.maximize_button, this);
  }
}


void OpenboxWindow::positionButtons(Bool redecorate_label) {
  const char *format = openbox.getTitleBarLayout();
  const unsigned int bw = frame.bevel_w + 1;
  const unsigned int by = frame.bevel_w + 1;
  unsigned int bx = frame.bevel_w + 1;
  unsigned int bcount = strlen(format) - 1;

  if (!decorations.close)
    bcount--;
  if (!decorations.maximize)
    bcount--;
  if (!decorations.iconify)
    bcount--;
  frame.label_w = frame.width - bx * 2 - (frame.button_w + bw) * bcount;
  
  bool hasclose, hasiconify, hasmaximize;
  hasclose = hasiconify = hasmaximize = false;
 
  for (int i = 0; format[i] != '\0' && i < 4; i++) {
    switch(format[i]) {
    case 'C':
      if (decorations.close && frame.close_button != None) {
        XMoveResizeWindow(display, frame.close_button, bx, by,
                          frame.button_w, frame.button_h);
        XMapWindow(display, frame.close_button);
        XClearWindow(display, frame.close_button);
        bx += frame.button_w + bw;
        hasclose = true;
      } else if (frame.close_button)
        XUnmapWindow(display, frame.close_button);
      break;
    case 'I':
      if (decorations.iconify && frame.iconify_button != None) {
        XMoveResizeWindow(display, frame.iconify_button, bx, by,
                          frame.button_w, frame.button_h);
        XMapWindow(display, frame.iconify_button);
        XClearWindow(display, frame.iconify_button);
        bx += frame.button_w + bw;
        hasiconify = true;
      } else if (frame.close_button)
        XUnmapWindow(display, frame.close_button);
      break;
    case 'M':
      if (decorations.maximize && frame.maximize_button != None) {
        XMoveResizeWindow(display, frame.maximize_button, bx, by,
                          frame.button_w, frame.button_h);
        XMapWindow(display, frame.maximize_button);
        XClearWindow(display, frame.maximize_button);
        bx += frame.button_w + bw;
        hasmaximize = true;
      } else if (frame.close_button)
        XUnmapWindow(display, frame.close_button);
      break;
    case 'L':
      XMoveResizeWindow(display, frame.label, bx, by - 1,
                        frame.label_w, frame.label_h);
      bx += frame.label_w + bw;
      break;
    }
  }

  if (!hasclose) {
      openbox.removeWindowSearch(frame.close_button);
      XDestroyWindow(display, frame.close_button);     
  }
  if (!hasiconify) {
      openbox.removeWindowSearch(frame.iconify_button);
      XDestroyWindow(display, frame.iconify_button);
  }
  if (!hasmaximize) {
      openbox.removeWindowSearch(frame.maximize_button);
      XDestroyWindow(display, frame.maximize_button);                 
  }
  if (redecorate_label)
    decorateLabel();
  redrawLabel();
  redrawAllButtons();
}


void OpenboxWindow::reconfigure(void) {
  upsize();

  client.x = frame.x + frame.mwm_border_w + frame.border_w;
  client.y = frame.y + frame.y_border + frame.mwm_border_w +
	     frame.border_w;

  if (client.title) {
    if (i18n->multibyte()) {
      XRectangle ink, logical;
      XmbTextExtents(screen->getWindowStyle()->fontset,
		     client.title, client.title_len, &ink, &logical);
      client.title_text_w = logical.width;
    } else {
      client.title_text_w = XTextWidth(screen->getWindowStyle()->font,
				       client.title, client.title_len);
    }
    client.title_text_w += (frame.bevel_w * 4);
  }

  positionWindows();
  decorate();

  XClearWindow(display, frame.window);
  setFocusFlag(flags.focused);

  configure(frame.x, frame.y, frame.width, frame.height);

  if (! screen->sloppyFocus())
    openbox.grabButton(Button1, 0, frame.plate, True, ButtonPressMask,
        GrabModeSync, GrabModeSync, None, None);
  else
    openbox.ungrabButton(Button1, 0, frame.plate);

  if (windowmenu) {
    windowmenu->move(windowmenu->getX(), frame.y + frame.title_h);
    windowmenu->reconfigure();
  }
}


void OpenboxWindow::positionWindows(void) {
  XResizeWindow(display, frame.window, frame.width,
                ((flags.shaded) ? frame.title_h : frame.height));
  XSetWindowBorderWidth(display, frame.window, frame.border_w);
  XSetWindowBorderWidth(display, frame.plate, frame.mwm_border_w);
  XMoveResizeWindow(display, frame.plate, 0, frame.y_border,
                    client.width, client.height);
  XMoveResizeWindow(display, client.window, 0, 0, client.width, client.height);

  if (decorations.titlebar) {
    XSetWindowBorderWidth(display, frame.title, frame.border_w);
    XMoveResizeWindow(display, frame.title, -frame.border_w,
		      -frame.border_w, frame.width, frame.title_h);

    positionButtons();
  } else if (frame.title) {
    XUnmapWindow(display, frame.title);
  }
  if (decorations.handle) {
    XSetWindowBorderWidth(display, frame.handle, frame.border_w);
    XSetWindowBorderWidth(display, frame.left_grip, frame.border_w);
    XSetWindowBorderWidth(display, frame.right_grip, frame.border_w);

    XMoveResizeWindow(display, frame.handle, -frame.border_w,
                      frame.y_handle - frame.border_w,
		      frame.width, frame.handle_h);
    XMoveResizeWindow(display, frame.left_grip, -frame.border_w,
		      -frame.border_w, frame.grip_w, frame.grip_h);
    XMoveResizeWindow(display, frame.right_grip,
		      frame.width - frame.grip_w - frame.border_w,
                      -frame.border_w, frame.grip_w, frame.grip_h);
    XMapSubwindows(display, frame.handle);
  } else if (frame.handle) {
    XUnmapWindow(display, frame.handle);
  }
}


void OpenboxWindow::getWMName(void) {
  if (client.title) {
    delete [] client.title;
    client.title = (char *) 0;
  }

  XTextProperty text_prop;
  char **list;
  int num;

  if (XGetWMName(display, client.window, &text_prop)) {
    if (text_prop.value && text_prop.nitems > 0) {
      if (text_prop.encoding != XA_STRING) {
	text_prop.nitems = strlen((char *) text_prop.value);

	if ((XmbTextPropertyToTextList(display, &text_prop,
				       &list, &num) == Success) &&
	    (num > 0) && *list) {
	  client.title = bstrdup(*list);
	  XFreeStringList(list);
	} else {
	  client.title = bstrdup((char *) text_prop.value);
	}
      } else {
	client.title = bstrdup((char *) text_prop.value);
      }
      XFree((char *) text_prop.value);
    } else {
      client.title = bstrdup(i18n->getMessage(WindowSet, WindowUnnamed,
					      "Unnamed"));
    }
  } else {
    client.title = bstrdup(i18n->getMessage(WindowSet, WindowUnnamed,
					    "Unnamed"));
  }
  client.title_len = strlen(client.title);

  if (i18n->multibyte()) {
    XRectangle ink, logical;
    XmbTextExtents(screen->getWindowStyle()->fontset,
		   client.title, client.title_len, &ink, &logical);
    client.title_text_w = logical.width;
  } else {
    client.title_len = strlen(client.title);
    client.title_text_w = XTextWidth(screen->getWindowStyle()->font,
				     client.title, client.title_len);
  }

  client.title_text_w += (frame.bevel_w * 4);
}


void OpenboxWindow::getWMIconName(void) {
  if (client.icon_title) {
    delete [] client.icon_title;
    client.icon_title = (char *) 0;
  }

  XTextProperty text_prop;
  char **list;
  int num;

  if (XGetWMIconName(display, client.window, &text_prop)) {
    if (text_prop.value && text_prop.nitems > 0) {
      if (text_prop.encoding != XA_STRING) {
 	text_prop.nitems = strlen((char *) text_prop.value);

 	if ((XmbTextPropertyToTextList(display, &text_prop,
 				       &list, &num) == Success) &&
 	    (num > 0) && *list) {
 	  client.icon_title = bstrdup(*list);
 	  XFreeStringList(list);
 	} else {
 	  client.icon_title = bstrdup((char *) text_prop.value);
	}
      } else {
 	client.icon_title = bstrdup((char *) text_prop.value);
      }
      XFree((char *) text_prop.value);
    } else {
      client.icon_title = bstrdup(client.title);
    }
  } else {
    client.icon_title = bstrdup(client.title);
  }
}


/*
 * Retrieve which WM Protocols are supported by the client window.
 * If the WM_DELETE_WINDOW protocol is supported, add the close button to the
 * window's decorations and allow the close behavior.
 * If the WM_TAKE_FOCUS protocol is supported, save a value that indicates
 * this.
 */
void OpenboxWindow::getWMProtocols(void) {
  Atom *proto;
  int num_return = 0;

  if (XGetWMProtocols(display, client.window, &proto, &num_return)) {
    for (int i = 0; i < num_return; ++i) {
      if (proto[i] == openbox.getWMDeleteAtom())
	functions.close = decorations.close = True;
      else if (proto[i] == openbox.getWMTakeFocusAtom())
        flags.send_focus_message = True;
      else if (proto[i] == openbox.getOpenboxStructureMessagesAtom())
        screen->addNetizen(new Netizen(*screen, client.window));
    }

    XFree(proto);
  }
}


/*
 * Gets the value of the WM_HINTS property.
 * If the property is not set, then use a set of default values.
 */
void OpenboxWindow::getWMHints(void) {
  XWMHints *wmhint = XGetWMHints(display, client.window);
  if (! wmhint) {
    flags.visible = True;
    flags.iconic = False;
    focus_mode = F_Passive;
    client.window_group = None;
    client.initial_state = NormalState;
    return;
  }
  client.wm_hint_flags = wmhint->flags;
  if (wmhint->flags & InputHint) {
    if (wmhint->input == True) {
      if (flags.send_focus_message)
	focus_mode = F_LocallyActive;
      else
	focus_mode = F_Passive;
    } else {
      if (flags.send_focus_message)
	focus_mode = F_GloballyActive;
      else
	focus_mode = F_NoInput;
    }
  } else {
    focus_mode = F_Passive;
  }

  if (wmhint->flags & StateHint)
    client.initial_state = wmhint->initial_state;
  else
    client.initial_state = NormalState;

  if (wmhint->flags & WindowGroupHint) {
    if (! client.window_group) {
      client.window_group = wmhint->window_group;
      openbox.saveGroupSearch(client.window_group, this);
    }
  } else {
    client.window_group = None;
  }
  XFree(wmhint);
}


/*
 * Gets the value of the WM_NORMAL_HINTS property.
 * If the property is not set, then use a set of default values.
 */
void OpenboxWindow::getWMNormalHints(void) {
  long icccm_mask;
  XSizeHints sizehint;

  client.min_width = client.min_height =
    client.base_width = client.base_height =
    client.width_inc = client.height_inc = 1;
  client.max_width = screen->size().w();
  client.max_height = screen->size().h();
  client.min_aspect_x = client.min_aspect_y =
    client.max_aspect_x = client.max_aspect_y = 1;
  client.win_gravity = NorthWestGravity;

  if (! XGetWMNormalHints(display, client.window, &sizehint, &icccm_mask))
    return;

  client.normal_hint_flags = sizehint.flags;

  if (sizehint.flags & PMinSize) {
    client.min_width = sizehint.min_width;
    client.min_height = sizehint.min_height;
  }

  if (sizehint.flags & PMaxSize) {
    client.max_width = sizehint.max_width;
    client.max_height = sizehint.max_height;
  }

  if (sizehint.flags & PResizeInc) {
    client.width_inc = sizehint.width_inc;
    client.height_inc = sizehint.height_inc;
  }

  if (sizehint.flags & PAspect) {
    client.min_aspect_x = sizehint.min_aspect.x;
    client.min_aspect_y = sizehint.min_aspect.y;
    client.max_aspect_x = sizehint.max_aspect.x;
    client.max_aspect_y = sizehint.max_aspect.y;
  }

  if (sizehint.flags & PBaseSize) {
    client.base_width = sizehint.base_width;
    client.base_height = sizehint.base_height;
  }

  if (sizehint.flags & PWinGravity)
    client.win_gravity = sizehint.win_gravity;
}


/*
 * Gets the MWM hints for the class' contained window.
 * This is used while initializing the window to its first state, and not
 * thereafter.
 * Returns: true if the MWM hints are successfully retreived and applied; false
 * if they are not.
 */
void OpenboxWindow::getMWMHints(void) {
  int format;
  Atom atom_return;
  unsigned long num, len;

  int ret = XGetWindowProperty(display, client.window,
			       openbox.getMotifWMHintsAtom(), 0,
			       PropMwmHintsElements, False,
			       openbox.getMotifWMHintsAtom(), &atom_return,
			       &format, &num, &len,
			       (unsigned char **) &client.mwm_hint);

  if (ret != Success || !client.mwm_hint || num != PropMwmHintsElements)
    return;

  if (client.mwm_hint->flags & MwmHintsDecorations) {
    if (client.mwm_hint->decorations & MwmDecorAll) {
      decorations.titlebar = decorations.handle = decorations.border =
	decorations.iconify = decorations.maximize =
	decorations.close = decorations.menu = True;
    } else {
      decorations.titlebar = decorations.handle = decorations.border =
	decorations.iconify = decorations.maximize =
	decorations.close = decorations.menu = False;

      if (client.mwm_hint->decorations & MwmDecorBorder)
	decorations.border = True;
      if (client.mwm_hint->decorations & MwmDecorHandle)
	decorations.handle = True;
      if (client.mwm_hint->decorations & MwmDecorTitle)
	decorations.titlebar = True;
      if (client.mwm_hint->decorations & MwmDecorMenu)
	decorations.menu = True;
      if (client.mwm_hint->decorations & MwmDecorIconify)
	decorations.iconify = True;
      if (client.mwm_hint->decorations & MwmDecorMaximize)
	decorations.maximize = True;
    }
  }

  if (client.mwm_hint->flags & MwmHintsFunctions) {
    if (client.mwm_hint->functions & MwmFuncAll) {
      functions.resize = functions.move = functions.iconify =
	functions.maximize = functions.close = True;
    } else {
      functions.resize = functions.move = functions.iconify =
	functions.maximize = functions.close = False;

      if (client.mwm_hint->functions & MwmFuncResize)
	functions.resize = True;
      if (client.mwm_hint->functions & MwmFuncMove)
	functions.move = True;
      if (client.mwm_hint->functions & MwmFuncIconify)
	functions.iconify = True;
      if (client.mwm_hint->functions & MwmFuncMaximize)
	functions.maximize = True;
      if (client.mwm_hint->functions & MwmFuncClose)
	functions.close = True;
    }
  }
}


/*
 * Gets the openbox hints from the class' contained window.
 * This is used while initializing the window to its first state, and not
 * thereafter.
 * Returns: true if the hints are successfully retreived and applied; false if
 * they are not.
 */
void OpenboxWindow::getOpenboxHints(void) {
  int format;
  Atom atom_return;
  unsigned long num, len;

  int ret = XGetWindowProperty(display, client.window,
			       openbox.getOpenboxHintsAtom(), 0,
			       PropOpenboxHintsElements, False,
			       openbox.getOpenboxHintsAtom(), &atom_return,
			       &format, &num, &len,
			       (unsigned char **) &client.openbox_hint);
  if (ret != Success || !client.openbox_hint ||
      num != PropOpenboxHintsElements)
    return;

  if (client.openbox_hint->flags & AttribShaded)
    flags.shaded = (client.openbox_hint->attrib & AttribShaded);

  if ((client.openbox_hint->flags & AttribMaxHoriz) &&
      (client.openbox_hint->flags & AttribMaxVert))
    flags.maximized = (client.openbox_hint->attrib &
		       (AttribMaxHoriz | AttribMaxVert)) ? 1 : 0;
  else if (client.openbox_hint->flags & AttribMaxVert)
    flags.maximized = (client.openbox_hint->attrib & AttribMaxVert) ? 2 : 0;
  else if (client.openbox_hint->flags & AttribMaxHoriz)
    flags.maximized = (client.openbox_hint->attrib & AttribMaxHoriz) ? 3 : 0;

  if (client.openbox_hint->flags & AttribOmnipresent)
    flags.stuck = (client.openbox_hint->attrib & AttribOmnipresent);

  if (client.openbox_hint->flags & AttribWorkspace)
    workspace_number = client.openbox_hint->workspace;

  // if (client.openbox_hint->flags & AttribStack)
  //   don't yet have always on top/bottom for openbox yet... working
  //   on that

  if (client.openbox_hint->flags & AttribDecoration) {
    switch (client.openbox_hint->decoration) {
    case DecorNone:
      decorations.titlebar = decorations.border = decorations.handle =
	decorations.iconify = decorations.maximize =
	decorations.menu = False;
      functions.resize = functions.move = functions.iconify =
	functions.maximize = False;

      break;

    case DecorTiny:
      decorations.titlebar = decorations.iconify = decorations.menu =
	functions.move = functions.iconify = True;
      decorations.border = decorations.handle = decorations.maximize =
	functions.resize = functions.maximize = False;

      break;

    case DecorTool:
      decorations.titlebar = decorations.menu = functions.move = True;
      decorations.iconify = decorations.border = decorations.handle =
	decorations.maximize = functions.resize = functions.maximize =
	functions.iconify = False;

      break;

    case DecorNormal:
    default:
      decorations.titlebar = decorations.border = decorations.handle =
	decorations.iconify = decorations.maximize =
	decorations.menu = True;
      functions.resize = functions.move = functions.iconify =
	functions.maximize = True;

      break;
    }

    reconfigure();
  }
}


void OpenboxWindow::configure(int dx, int dy,
                               unsigned int dw, unsigned int dh) {
  Bool send_event = (frame.x != dx || frame.y != dy);

  if ((dw != frame.width) || (dh != frame.height)) {
    if ((((signed) frame.width) + dx) < 0) dx = 0;
    if ((((signed) frame.height) + dy) < 0) dy = 0;

    frame.x = dx;
    frame.y = dy;
    frame.width = dw;
    frame.height = dh;

    downsize();

#ifdef    SHAPE
    if (openbox.hasShapeExtensions() && flags.shaped) {
      XShapeCombineShape(display, frame.window, ShapeBounding,
 		         frame.mwm_border_w, frame.y_border +
			 frame.mwm_border_w, client.window,
			 ShapeBounding, ShapeSet);

      int num = 1;
      XRectangle xrect[2];
      xrect[0].x = xrect[0].y = 0;
      xrect[0].width = frame.width;
      xrect[0].height = frame.y_border;

      if (decorations.handle) {
	xrect[1].x = 0;
	xrect[1].y = frame.y_handle;
	xrect[1].width = frame.width;
	xrect[1].height = frame.handle_h + frame.border_w;
	num++;
      }

      XShapeCombineRectangles(display, frame.window, ShapeBounding, 0, 0,
			      xrect, num, ShapeUnion, Unsorted);
    }
#endif // SHAPE

    XMoveWindow(display, frame.window, frame.x, frame.y);

    positionWindows();
    decorate();
    setFocusFlag(flags.focused);
    redrawAllButtons();
  } else {
    frame.x = dx;
    frame.y = dy;

    XMoveWindow(display, frame.window, frame.x, frame.y);

    if (! flags.moving) send_event = True;
  }

  if (send_event && ! flags.moving) {
    client.x = dx + frame.mwm_border_w + frame.border_w;
    client.y = dy + frame.y_border + frame.mwm_border_w +
               frame.border_w;

    XEvent event;
    event.type = ConfigureNotify;

    event.xconfigure.display = display;
    event.xconfigure.event = client.window;
    event.xconfigure.window = client.window;
    event.xconfigure.x = client.x;
    event.xconfigure.y = client.y;
    event.xconfigure.width = client.width;
    event.xconfigure.height = client.height;
    event.xconfigure.border_width = client.old_bw;
    event.xconfigure.above = frame.window;
    event.xconfigure.override_redirect = False;

    XSendEvent(display, client.window, True, NoEventMask, &event);

    screen->updateNetizenConfigNotify(&event);
  }
}


Bool OpenboxWindow::setInputFocus(void) {
  if (((signed) (frame.x + frame.width)) < 0) {
    if (((signed) (frame.y + frame.y_border)) < 0)
      configure(frame.border_w, frame.border_w, frame.width, frame.height);
    else if (frame.y > (signed) screen->size().h())
      configure(frame.border_w, screen->size().h() - frame.height,
                frame.width, frame.height);
    else
      configure(frame.border_w, frame.y + frame.border_w,
                frame.width, frame.height);
  } else if (frame.x > (signed) screen->size().w()) {
    if (((signed) (frame.y + frame.y_border)) < 0)
      configure(screen->size().w() - frame.width, frame.border_w,
                frame.width, frame.height);
    else if (frame.y > (signed) screen->size().h())
      configure(screen->size().w() - frame.width,
	        screen->size().h() - frame.height, frame.width, frame.height);
    else
      configure(screen->size().w() - frame.width,
                frame.y + frame.border_w, frame.width, frame.height);
  }

  openbox.grab();
  if (! validateClient()) return False;

  Bool ret = False;

  if (client.transient && flags.modal) {
    ret = client.transient->setInputFocus();
  } else if (! flags.focused) {
    if (focus_mode == F_LocallyActive || focus_mode == F_Passive)
      XSetInputFocus(display, client.window,
		     RevertToPointerRoot, CurrentTime);
    else
      XSetInputFocus(display, screen->getRootWindow(),
		     RevertToNone, CurrentTime);

    openbox.setFocusedWindow(this);

    if (flags.send_focus_message) {
      XEvent ce;
      ce.xclient.type = ClientMessage;
      ce.xclient.message_type = openbox.getWMProtocolsAtom();
      ce.xclient.display = display;
      ce.xclient.window = client.window;
      ce.xclient.format = 32;
      ce.xclient.data.l[0] = openbox.getWMTakeFocusAtom();
      ce.xclient.data.l[1] = openbox.getLastTime();
      ce.xclient.data.l[2] = 0l;
      ce.xclient.data.l[3] = 0l;
      ce.xclient.data.l[4] = 0l;
      XSendEvent(display, client.window, False, NoEventMask, &ce);
    }

    if (screen->sloppyFocus() && screen->autoRaise())
      timer->start();

    ret = True;
  }

  openbox.ungrab();

  return ret;
}


void OpenboxWindow::iconify(void) {
  if (flags.iconic) return;

  if (windowmenu) windowmenu->hide();

  setState(IconicState);

  XSelectInput(display, client.window, NoEventMask);
  XUnmapWindow(display, client.window);
  XSelectInput(display, client.window,
               PropertyChangeMask | StructureNotifyMask | FocusChangeMask);

  XUnmapWindow(display, frame.window);
  flags.visible = False;
  flags.iconic = True;

  screen->getWorkspace(workspace_number)->removeWindow(this);

  if (flags.transient && client.transient_for &&
      !client.transient_for->flags.iconic) {
    client.transient_for->iconify();
  }
  screen->addIcon(this);

  if (client.transient && !client.transient->flags.iconic) {
    client.transient->iconify();
  }
}


void OpenboxWindow::deiconify(Bool reassoc, Bool raise) {
  if (flags.iconic || reassoc)
    screen->reassociateWindow(this, -1, False);
  else if (workspace_number != screen->getCurrentWorkspace()->getWorkspaceID())
    return;

  setState(NormalState);

  XSelectInput(display, client.window, NoEventMask);
  XMapWindow(display, client.window);
  XSelectInput(display, client.window,
               PropertyChangeMask | StructureNotifyMask | FocusChangeMask);

  XMapSubwindows(display, frame.window);
  XMapWindow(display, frame.window);

  if (flags.iconic && screen->focusNew()) setInputFocus();

  flags.visible = True;
  flags.iconic = False;

  if (reassoc && client.transient) client.transient->deiconify(True, False);

  if (raise)
    screen->getWorkspace(workspace_number)->raiseWindow(this);
}


void OpenboxWindow::close(void) {
  XEvent ce;
  ce.xclient.type = ClientMessage;
  ce.xclient.message_type = openbox.getWMProtocolsAtom();
  ce.xclient.display = display;
  ce.xclient.window = client.window;
  ce.xclient.format = 32;
  ce.xclient.data.l[0] = openbox.getWMDeleteAtom();
  ce.xclient.data.l[1] = CurrentTime;
  ce.xclient.data.l[2] = 0l;
  ce.xclient.data.l[3] = 0l;
  ce.xclient.data.l[4] = 0l;
  XSendEvent(display, client.window, False, NoEventMask, &ce);
}


void OpenboxWindow::withdraw(void) {
  flags.visible = False;
  flags.iconic = False;

  XUnmapWindow(display, frame.window);

  XSelectInput(display, client.window, NoEventMask);
  XUnmapWindow(display, client.window);
  XSelectInput(display, client.window,
               PropertyChangeMask | StructureNotifyMask | FocusChangeMask);

  if (windowmenu) windowmenu->hide();
}


void OpenboxWindow::maximize(unsigned int button) {
  // handle case where menu is open then the max button is used instead
  if (windowmenu && windowmenu->isVisible()) windowmenu->hide();

  if (flags.maximized) {
    flags.maximized = 0;

    openbox_attrib.flags &= ! (AttribMaxHoriz | AttribMaxVert);
    openbox_attrib.attrib &= ! (AttribMaxHoriz | AttribMaxVert);

    // when a resize is begun, maximize(0) is called to clear any maximization
    // flags currently set.  Otherwise it still thinks it is maximized.
    // so we do not need to call configure() because resizing will handle it
    if (!flags.resizing)
      configure(openbox_attrib.premax_x, openbox_attrib.premax_y,
		openbox_attrib.premax_w, openbox_attrib.premax_h);

    openbox_attrib.premax_x = openbox_attrib.premax_y = 0;
    openbox_attrib.premax_w = openbox_attrib.premax_h = 0;

    redrawAllButtons();
    setState(current_state);
    return;
  }

  // the following code is temporary and will be taken care of by Screen in the
  // future (with the NETWM 'strut')
  Rect space(0, 0, screen->size().w(), screen->size().h());
  if (! screen->fullMax()) {
#ifdef    SLIT
    Slit *slit = screen->getSlit();
    Toolbar *toolbar = screen->getToolbar();
    int tbarh = screen->hideToolbar() ? 0 :
      toolbar->getExposedHeight() + screen->getBorderWidth() * 2;
    bool tbartop;
    switch (toolbar->placement()) {
    case Toolbar::TopLeft:
    case Toolbar::TopCenter:
    case Toolbar::TopRight:
      tbartop = true;
      break;
    case Toolbar::BottomLeft:
    case Toolbar::BottomCenter:
    case Toolbar::BottomRight:
      tbartop = false;
      break;
    default:
      ASSERT(false);      // unhandled placement
    }
    if ((slit->direction() == Slit::Horizontal &&
         (slit->placement() == Slit::TopLeft ||
          slit->placement() == Slit::TopRight)) ||
        slit->placement() == Slit::TopCenter) {
      // exclude top
      if (tbartop) {
        space.setY(slit->area().y());
        space.setH(space.h() - space.y());
      } else
        space.setH(space.h() - tbarh);
      space.setY(space.y() + slit->area().h() + screen->getBorderWidth() * 2);
      space.setH(space.h() - (slit->area().h() + screen->getBorderWidth() * 2));
    } else if ((slit->direction() == Slit::Vertical &&
              (slit->placement() == Slit::TopRight ||
               slit->placement() == Slit::BottomRight)) ||
             slit->placement() == Slit::CenterRight) {
      // exclude right
      space.setW(space.w() - (slit->area().w() + screen->getBorderWidth() * 2));
      if (tbartop)
        space.setY(space.y() + tbarh);
      space.setH(space.h() - tbarh);
    } else if ((slit->direction() == Slit::Horizontal &&
              (slit->placement() == Slit::BottomLeft ||
               slit->placement() == Slit::BottomRight)) ||
             slit->placement() == Slit::BottomCenter) {
      // exclude bottom
      space.setH(space.h() - (screen->size().h() - slit->area().y()));
    } else {// if ((slit->direction() == Slit::Vertical &&
      //      (slit->placement() == Slit::TopLeft ||
      //       slit->placement() == Slit::BottomLeft)) ||
      //     slit->placement() == Slit::CenterLeft)
      // exclude left
      space.setX(slit->area().w() + screen->getBorderWidth() * 2);
      space.setW(space.w() - (slit->area().w() + screen->getBorderWidth() * 2));
      if (tbartop)
        space.setY(space.y() + tbarh);
      space.setH(space.h() - tbarh);
    }
#else // !SLIT
    Toolbar *toolbar = screen->getToolbar();
    int tbarh = screen->hideToolbar() ? 0 :
      toolbar->getExposedHeight() + screen->getBorderWidth() * 2;
    switch (toolbar->placement()) {
    case Toolbar::TopLeft:
    case Toolbar::TopCenter:
    case Toolbar::TopRight:
      space.setY(toolbar->getExposedHeight());
      space.setH(space.h() - toolbar->getExposedHeight());
      break;
    case Toolbar::BottomLeft:
    case Toolbar::BottomCenter:
    case Toolbar::BottomRight:
      space.setH(space.h() - tbarh);
      break;
    default:
      ASSERT(false);      // unhandled placement
    }
#endif // SLIT
  }

  openbox_attrib.premax_x = frame.x;
  openbox_attrib.premax_y = frame.y;
  openbox_attrib.premax_w = frame.width;
  openbox_attrib.premax_h = frame.height;

  unsigned int dw = space.w(),
               dh = space.h();
  dw -= frame.border_w * 2;
  dw -= frame.mwm_border_w * 2;
  dw -= client.base_width;

  dh -= frame.border_w * 2;
  dh -= frame.mwm_border_w * 2;
  dh -= ((frame.handle_h + frame.border_w) * decorations.handle);
  dh -= client.base_height;
  dh -= frame.y_border;

  if (dw < client.min_width) dw = client.min_width;
  if (dh < client.min_height) dh = client.min_height;
  if (dw > client.max_width) dw = client.max_width;
  if (dh > client.max_height) dh = client.max_height;

  dw -= (dw % client.width_inc);
  dw += client.base_width;
  dw += frame.mwm_border_w * 2;

  dh -= (dh % client.height_inc);
  dh += client.base_height;
  dh += frame.y_border;
  dh += ((frame.handle_h + frame.border_w) * decorations.handle);
  dh += frame.mwm_border_w * 2;

  int dx = space.x() + ((space.w() - dw) / 2) - frame.border_w,
      dy = space.y() + ((space.h() - dh) / 2) - frame.border_w;

  switch(button) {
  case 1:
    openbox_attrib.flags |= AttribMaxHoriz | AttribMaxVert;
    openbox_attrib.attrib |= AttribMaxHoriz | AttribMaxVert;
    break;

  case 2:
    openbox_attrib.flags |= AttribMaxVert;
    openbox_attrib.attrib |= AttribMaxVert;

    dw = frame.width;
    dx = frame.x;
    break;

  case 3:
    openbox_attrib.flags |= AttribMaxHoriz;
    openbox_attrib.attrib |= AttribMaxHoriz;

    dh = frame.height;
    dy = frame.y;
    break;
  }

  if (flags.shaded) {
    openbox_attrib.flags ^= AttribShaded;
    openbox_attrib.attrib ^= AttribShaded;
    flags.shaded = False;
  }

  flags.maximized = button;

  configure(dx, dy, dw, dh);
  screen->getWorkspace(workspace_number)->raiseWindow(this);
  redrawAllButtons();
  setState(current_state);
}


void OpenboxWindow::setWorkspace(int n) {
  workspace_number = n;

  openbox_attrib.flags |= AttribWorkspace;
  openbox_attrib.workspace = workspace_number;
}


void OpenboxWindow::shade(void) {
  if (!decorations.titlebar)
    return;

  if (flags.shaded) {
    XResizeWindow(display, frame.window, frame.width, frame.height);
    flags.shaded = False;
    openbox_attrib.flags ^= AttribShaded;
    openbox_attrib.attrib ^= AttribShaded;

    setState(NormalState);
  } else {
    XResizeWindow(display, frame.window, frame.width, frame.title_h);
    flags.shaded = True;
    openbox_attrib.flags |= AttribShaded;
    openbox_attrib.attrib |= AttribShaded;

    setState(IconicState);
  }
}


void OpenboxWindow::stick(void) {
  if (flags.stuck) {
    openbox_attrib.flags ^= AttribOmnipresent;
    openbox_attrib.attrib ^= AttribOmnipresent;

    flags.stuck = False;

    if (! flags.iconic)
      screen->reassociateWindow(this, -1, True);

    setState(current_state);
  } else {
    flags.stuck = True;

    openbox_attrib.flags |= AttribOmnipresent;
    openbox_attrib.attrib |= AttribOmnipresent;

    setState(current_state);
  }
}


void OpenboxWindow::setFocusFlag(Bool focus) {
  flags.focused = focus;

  if (decorations.titlebar) {
    if (flags.focused) {
      if (frame.ftitle)
        XSetWindowBackgroundPixmap(display, frame.title, frame.ftitle);
      else
        XSetWindowBackground(display, frame.title, frame.ftitle_pixel);
    } else {
      if (frame.utitle)
        XSetWindowBackgroundPixmap(display, frame.title, frame.utitle);
      else
        XSetWindowBackground(display, frame.title, frame.utitle_pixel);
    }
    XClearWindow(display, frame.title);

    redrawLabel();
    redrawAllButtons();
  }

  if (decorations.handle) {
    if (flags.focused) {
      if (frame.fhandle)
        XSetWindowBackgroundPixmap(display, frame.handle, frame.fhandle);
      else
        XSetWindowBackground(display, frame.handle, frame.fhandle_pixel);

      if (frame.fgrip) {
        XSetWindowBackgroundPixmap(display, frame.right_grip, frame.fgrip);
        XSetWindowBackgroundPixmap(display, frame.left_grip, frame.fgrip);
      } else {
        XSetWindowBackground(display, frame.right_grip, frame.fgrip_pixel);
        XSetWindowBackground(display, frame.left_grip, frame.fgrip_pixel);
      }
    } else {
      if (frame.uhandle)
        XSetWindowBackgroundPixmap(display, frame.handle, frame.uhandle);
      else
        XSetWindowBackground(display, frame.handle, frame.uhandle_pixel);

      if (frame.ugrip) {
        XSetWindowBackgroundPixmap(display, frame.right_grip, frame.ugrip);
        XSetWindowBackgroundPixmap(display, frame.left_grip, frame.ugrip);
      } else {
        XSetWindowBackground(display, frame.right_grip, frame.ugrip_pixel);
        XSetWindowBackground(display, frame.left_grip, frame.ugrip_pixel);
      }
    }
    XClearWindow(display, frame.handle);
    XClearWindow(display, frame.right_grip);
    XClearWindow(display, frame.left_grip);
  }

  if (decorations.border) {
    if (flags.focused)
      XSetWindowBorder(display, frame.plate, frame.fborder_pixel);
    else
      XSetWindowBorder(display, frame.plate, frame.uborder_pixel);
  }

  if (screen->sloppyFocus() && screen->autoRaise() && timer->isTiming())
    timer->stop();
}


void OpenboxWindow::installColormap(Bool install) {
  openbox.grab();
  if (! validateClient()) return;

  int i = 0, ncmap = 0;
  Colormap *cmaps = XListInstalledColormaps(display, client.window, &ncmap);
  XWindowAttributes wattrib;
  if (cmaps) {
    if (XGetWindowAttributes(display, client.window, &wattrib)) {
      if (install) {
	// install the window's colormap
	for (i = 0; i < ncmap; i++) {
	  if (*(cmaps + i) == wattrib.colormap)
	    // this window is using an installed color map... do not install
	    install = False;
	}
	// otherwise, install the window's colormap
	if (install)
	  XInstallColormap(display, wattrib.colormap);
      } else {
	// uninstall the window's colormap
	for (i = 0; i < ncmap; i++) {
	  if (*(cmaps + i) == wattrib.colormap)
	    // we found the colormap to uninstall
	    XUninstallColormap(display, wattrib.colormap);
	}
      }
    }

    XFree(cmaps);
  }

  openbox.ungrab();
}


void OpenboxWindow::setState(unsigned long new_state) {
  current_state = new_state;

  unsigned long state[2];
  state[0] = (unsigned long) current_state;
  state[1] = (unsigned long) None;
  XChangeProperty(display, client.window, openbox.getWMStateAtom(),
		  openbox.getWMStateAtom(), 32, PropModeReplace,
		  (unsigned char *) state, 2);

  XChangeProperty(display, client.window,
		  openbox.getOpenboxAttributesAtom(),
                  openbox.getOpenboxAttributesAtom(), 32, PropModeReplace,
                  (unsigned char *) &openbox_attrib,
		  PropOpenboxAttributesElements);
}


Bool OpenboxWindow::getState(void) {
  current_state = 0;

  Atom atom_return;
  Bool ret = False;
  int foo;
  unsigned long *state, ulfoo, nitems;

  if ((XGetWindowProperty(display, client.window, openbox.getWMStateAtom(),
			  0l, 2l, False, openbox.getWMStateAtom(),
			  &atom_return, &foo, &nitems, &ulfoo,
			  (unsigned char **) &state) != Success) ||
      (! state)) {
    openbox.ungrab();
    return False;
  }

  if (nitems >= 1) {
    current_state = (unsigned long) state[0];

    ret = True;
  }

  XFree((void *) state);

  return ret;
}


void OpenboxWindow::setGravityOffsets(void) {
  // x coordinates for each gravity type
  const int x_west = client.x;
  const int x_east = client.x + client.width - frame.width;
  const int x_center = client.x + client.width - frame.width/2;
  // y coordinates for each gravity type
  const int y_north = client.y;
  const int y_south = client.y + client.height - frame.height;
  const int y_center = client.y + client.height - frame.height/2;

  switch (client.win_gravity) {
  case NorthWestGravity:
  default:
    frame.x = x_west;
    frame.y = y_north;
    break;
  case NorthGravity:
    frame.x = x_center;
    frame.y = y_north;
    break;
  case NorthEastGravity:
    frame.x = x_east;
    frame.y = y_north;
    break;
  case SouthWestGravity:
    frame.x = x_west;
    frame.y = y_south;
    break;
  case SouthGravity:
    frame.x = x_center;
    frame.y = y_south;
    break;
  case SouthEastGravity:
    frame.x = x_east;
    frame.y = y_south;
    break;
  case WestGravity:
    frame.x = x_west;
    frame.y = y_center;
    break;
  case EastGravity:
    frame.x = x_east;
    frame.y = y_center;
    break;
  case CenterGravity:
    frame.x = x_center;
    frame.y = y_center;
    break;
  case ForgetGravity:
  case StaticGravity:
    frame.x = client.x - frame.mwm_border_w + frame.border_w;
    frame.y = client.y - frame.y_border - frame.mwm_border_w - frame.border_w;
    break;
  }
}


void OpenboxWindow::restoreAttributes(void) {
  if (! getState()) current_state = NormalState;

  Atom atom_return;
  int foo;
  unsigned long ulfoo, nitems;

  OpenboxAttributes *net;
  int ret = XGetWindowProperty(display, client.window,
			       openbox.getOpenboxAttributesAtom(), 0l,
			       PropOpenboxAttributesElements, False,
			       openbox.getOpenboxAttributesAtom(),
			       &atom_return, &foo, &nitems, &ulfoo,
			       (unsigned char **) &net);
  if (ret != Success || !net || nitems != PropOpenboxAttributesElements)
    return;

  openbox_attrib.flags = net->flags;
  openbox_attrib.attrib = net->attrib;
  openbox_attrib.decoration = net->decoration;
  openbox_attrib.workspace = net->workspace;
  openbox_attrib.stack = net->stack;
  openbox_attrib.premax_x = net->premax_x;
  openbox_attrib.premax_y = net->premax_y;
  openbox_attrib.premax_w = net->premax_w;
  openbox_attrib.premax_h = net->premax_h;

  XFree((void *) net);

  if (openbox_attrib.flags & AttribShaded &&
      openbox_attrib.attrib & AttribShaded) {
    int save_state =
      ((current_state == IconicState) ? NormalState : current_state);

    flags.shaded = False;
    shade();

    current_state = save_state;
  }

  if (((int) openbox_attrib.workspace != screen->getCurrentWorkspaceID()) &&
      ((int) openbox_attrib.workspace < screen->getWorkspaceCount())) {
    screen->reassociateWindow(this, openbox_attrib.workspace, True);

    if (current_state == NormalState) current_state = WithdrawnState;
  } else if (current_state == WithdrawnState) {
    current_state = NormalState;
  }

  if (openbox_attrib.flags & AttribOmnipresent &&
      openbox_attrib.attrib & AttribOmnipresent) {
    flags.stuck = False;
    stick();

    current_state = NormalState;
  }

  if ((openbox_attrib.flags & AttribMaxHoriz) ||
      (openbox_attrib.flags & AttribMaxVert)) {
    int x = openbox_attrib.premax_x, y = openbox_attrib.premax_y;
    unsigned int w = openbox_attrib.premax_w, h = openbox_attrib.premax_h;
    flags.maximized = 0;

    unsigned int m = False;
    if ((openbox_attrib.flags & AttribMaxHoriz) &&
        (openbox_attrib.flags & AttribMaxVert))
      m = (openbox_attrib.attrib & (AttribMaxHoriz | AttribMaxVert)) ? 1 : 0;
    else if (openbox_attrib.flags & AttribMaxVert)
      m = (openbox_attrib.attrib & AttribMaxVert) ? 2 : 0;
    else if (openbox_attrib.flags & AttribMaxHoriz)
      m = (openbox_attrib.attrib & AttribMaxHoriz) ? 3 : 0;

    if (m) maximize(m);

    openbox_attrib.premax_x = x;
    openbox_attrib.premax_y = y;
    openbox_attrib.premax_w = w;
    openbox_attrib.premax_h = h;
  }

  setState(current_state);
}


/*
 * The reverse of the setGravityOffsets function. Uses the frame window's
 * position to find the window's reference point.
 */
void OpenboxWindow::restoreGravity(void) {
  // x coordinates for each gravity type
  const int x_west = frame.x;
  const int x_east = frame.x + frame.width - client.width;
  const int x_center = frame.x + (frame.width/2) - client.width;
  // y coordinates for each gravity type
  const int y_north = frame.y;
  const int y_south = frame.y + frame.height - client.height;
  const int y_center = frame.y + (frame.height/2) - client.height;

  switch(client.win_gravity) {
  default:
  case NorthWestGravity:
    client.x = x_west;
    client.y = y_north;
    break;
  case NorthGravity:
    client.x = x_center;
    client.y = y_north;
    break;
  case NorthEastGravity:
    client.x = x_east;
    client.y = y_north;
    break;
  case SouthWestGravity:
    client.x = x_west;
    client.y = y_south;
    break;
  case SouthGravity:
    client.x = x_center;
    client.y = y_south;
    break;
  case SouthEastGravity:
    client.x = x_east;
    client.y = y_south;
    break;
  case WestGravity:
    client.x = x_west;
    client.y = y_center;
    break;
  case EastGravity:
    client.x = x_east;
    client.y = y_center;
    break;
  case CenterGravity:
    client.x = x_center;
    client.y = y_center;
    break;
  case ForgetGravity:
  case StaticGravity:
    client.x = frame.x + frame.mwm_border_w + frame.border_w;
    client.y = frame.y + frame.y_border + frame.mwm_border_w +
      frame.border_w;
    break;
  }
}


void OpenboxWindow::redrawLabel(void) {
  int dx = frame.bevel_w * 2, dlen = client.title_len;
  unsigned int l = client.title_text_w;

  if (flags.focused) {
    if (frame.flabel)
      XSetWindowBackgroundPixmap(display, frame.label, frame.flabel);
    else
      XSetWindowBackground(display, frame.label, frame.flabel_pixel);
  } else {
    if (frame.ulabel)
      XSetWindowBackgroundPixmap(display, frame.label, frame.ulabel);
    else
      XSetWindowBackground(display, frame.label, frame.ulabel_pixel);
  }
  XClearWindow(display, frame.label);

  if (client.title_text_w > frame.label_w) {
    for (; dlen >= 0; dlen--) {
      if (i18n->multibyte()) {
	XRectangle ink, logical;
	XmbTextExtents(screen->getWindowStyle()->fontset, client.title, dlen,
		       &ink, &logical);
	l = logical.width;
      } else {
	l = XTextWidth(screen->getWindowStyle()->font, client.title, dlen);
      }
      l += (frame.bevel_w * 4);

      if (l < frame.label_w)
	break;
    }
  }

  switch (screen->getWindowStyle()->justify) {
  case BScreen::RightJustify:
    dx += frame.label_w - l;
    break;

  case BScreen::CenterJustify:
    dx += (frame.label_w - l) / 2;
    break;
  }

  WindowStyle *style = screen->getWindowStyle();
  GC text_gc = (flags.focused) ? style->l_text_focus_gc :
    style->l_text_unfocus_gc;
  if (i18n->multibyte())
    XmbDrawString(display, frame.label, style->fontset, text_gc, dx,
		  (1 - style->fontset_extents->max_ink_extent.y),
		  client.title, dlen);
  else
    XDrawString(display, frame.label, text_gc, dx,
		(style->font->ascent + 1), client.title, dlen);
}


void OpenboxWindow::redrawAllButtons(void) {
  if (frame.iconify_button) redrawIconifyButton(False);
  if (frame.maximize_button) redrawMaximizeButton(flags.maximized);
  if (frame.close_button) redrawCloseButton(False);
}


void OpenboxWindow::redrawIconifyButton(Bool pressed) {
  if (! pressed) {
    if (flags.focused) {
      if (frame.fbutton)
        XSetWindowBackgroundPixmap(display, frame.iconify_button,
				   frame.fbutton);
      else
        XSetWindowBackground(display, frame.iconify_button,
			     frame.fbutton_pixel);
    } else {
      if (frame.ubutton)
        XSetWindowBackgroundPixmap(display, frame.iconify_button,
				   frame.ubutton);
      else
        XSetWindowBackground(display, frame.iconify_button,
			     frame.ubutton_pixel);
    }
  } else {
    if (frame.pbutton)
      XSetWindowBackgroundPixmap(display, frame.iconify_button, frame.pbutton);
    else
      XSetWindowBackground(display, frame.iconify_button, frame.pbutton_pixel);
  }
  XClearWindow(display, frame.iconify_button);

  XDrawRectangle(display, frame.iconify_button,
		 ((flags.focused) ? screen->getWindowStyle()->b_pic_focus_gc :
		  screen->getWindowStyle()->b_pic_unfocus_gc),
		 2, (frame.button_h - 5), (frame.button_w - 5), 2);
}


void OpenboxWindow::redrawMaximizeButton(Bool pressed) {
  if (! pressed) {
    if (flags.focused) {
      if (frame.fbutton)
        XSetWindowBackgroundPixmap(display, frame.maximize_button,
				   frame.fbutton);
      else
        XSetWindowBackground(display, frame.maximize_button,
			     frame.fbutton_pixel);
    } else {
      if (frame.ubutton)
        XSetWindowBackgroundPixmap(display, frame.maximize_button,
				   frame.ubutton);
      else
        XSetWindowBackground(display, frame.maximize_button,
			     frame.ubutton_pixel);
    }
  } else {
    if (frame.pbutton)
      XSetWindowBackgroundPixmap(display, frame.maximize_button,
				 frame.pbutton);
    else
      XSetWindowBackground(display, frame.maximize_button,
			   frame.pbutton_pixel);
  }
  XClearWindow(display, frame.maximize_button);

  XDrawRectangle(display, frame.maximize_button,
		 ((flags.focused) ? screen->getWindowStyle()->b_pic_focus_gc :
		  screen->getWindowStyle()->b_pic_unfocus_gc),
		 2, 2, (frame.button_w - 5), (frame.button_h - 5));
  XDrawLine(display, frame.maximize_button,
	    ((flags.focused) ? screen->getWindowStyle()->b_pic_focus_gc :
	     screen->getWindowStyle()->b_pic_unfocus_gc),
	    2, 3, (frame.button_w - 3), 3);
}


void OpenboxWindow::redrawCloseButton(Bool pressed) {
  if (! pressed) {
    if (flags.focused) {
      if (frame.fbutton)
        XSetWindowBackgroundPixmap(display, frame.close_button,
				   frame.fbutton);
      else
        XSetWindowBackground(display, frame.close_button,
			     frame.fbutton_pixel);
    } else {
      if (frame.ubutton)
        XSetWindowBackgroundPixmap(display, frame.close_button,
				   frame.ubutton);
      else
        XSetWindowBackground(display, frame.close_button,
			     frame.ubutton_pixel);
    }
  } else {
    if (frame.pbutton)
      XSetWindowBackgroundPixmap(display, frame.close_button, frame.pbutton);
    else
      XSetWindowBackground(display, frame.close_button, frame.pbutton_pixel);
  }
  XClearWindow(display, frame.close_button);

  XDrawLine(display, frame.close_button,
	    ((flags.focused) ? screen->getWindowStyle()->b_pic_focus_gc :
	     screen->getWindowStyle()->b_pic_unfocus_gc), 2, 2,
            (frame.button_w - 3), (frame.button_h - 3));
  XDrawLine(display, frame.close_button,
	    ((flags.focused) ? screen->getWindowStyle()->b_pic_focus_gc :
	     screen->getWindowStyle()->b_pic_unfocus_gc), 2,
	    (frame.button_h - 3),
            (frame.button_w - 3), 2);
}


void OpenboxWindow::mapRequestEvent(XMapRequestEvent *re) {
  if (re->window == client.window) {
#ifdef    DEBUG
    fprintf(stderr, i18n->getMessage(WindowSet, WindowMapRequest,
			     "OpenboxWindow::mapRequestEvent() for 0x%lx\n"),
            client.window);
#endif // DEBUG

    openbox.grab();
    if (! validateClient()) return;

    Bool get_state_ret = getState();
    if (! (get_state_ret && openbox.isStartup())) {
      if ((client.wm_hint_flags & StateHint) &&
          (! (current_state == NormalState || current_state == IconicState)))
        current_state = client.initial_state;
      else
        current_state = NormalState;
    } else if (flags.iconic) {
      current_state = NormalState;
    }

    switch (current_state) {
    case IconicState:
      iconify();
      break;

    case WithdrawnState:
      withdraw();
      break;

    case NormalState:
    case InactiveState:
    case ZoomState:
    default:
      deiconify(False);
      break;
    }

    openbox.ungrab();
  }
}


void OpenboxWindow::mapNotifyEvent(XMapEvent *ne) {
  if ((ne->window == client.window) && (! ne->override_redirect)
      && (flags.visible)) {
    openbox.grab();
    if (! validateClient()) return;

    if (decorations.titlebar) positionButtons();

    setState(NormalState);

    redrawAllButtons();

    if (flags.transient || screen->focusNew())
      setInputFocus();
    else
      setFocusFlag(False);

    flags.visible = True;
    flags.iconic = False;

    openbox.ungrab();
  }
}


void OpenboxWindow::unmapNotifyEvent(XUnmapEvent *ue) {
  if (ue->window == client.window) {
#ifdef    DEBUG
    fprintf(stderr, i18n->getMessage(WindowSet, WindowUnmapNotify,
			     "OpenboxWindow::unmapNotifyEvent() for 0x%lx\n"),
            client.window);
#endif // DEBUG

    openbox.grab();
    if (! validateClient()) return;

    XChangeSaveSet(display, client.window, SetModeDelete);
    XSelectInput(display, client.window, NoEventMask);

    XDeleteProperty(display, client.window, openbox.getWMStateAtom());
    XDeleteProperty(display, client.window,
		    openbox.getOpenboxAttributesAtom());

    XUnmapWindow(display, frame.window);
    XUnmapWindow(display, client.window);

    XEvent dummy;
    if (! XCheckTypedWindowEvent(display, client.window, ReparentNotify,
				 &dummy)) {
#ifdef    DEBUG
      fprintf(stderr, i18n->getMessage(WindowSet, WindowUnmapNotifyReparent,
		       "OpenboxWindow::unmapNotifyEvent(): reparent 0x%lx to "
		       "root.\n"), client.window);
#endif // DEBUG

      restoreGravity();
      XReparentWindow(display, client.window, screen->getRootWindow(),
		      client.x, client.y);
    }

    XFlush(display);

    openbox.ungrab();

    delete this;
  }
}


void OpenboxWindow::destroyNotifyEvent(XDestroyWindowEvent *de) {
  if (de->window == client.window) {
    XUnmapWindow(display, frame.window);

    delete this;
  }
}


void OpenboxWindow::propertyNotifyEvent(Atom atom) {
  openbox.grab();
  if (! validateClient()) return;

  switch(atom) {
  case XA_WM_CLASS:
  case XA_WM_CLIENT_MACHINE:
  case XA_WM_COMMAND:
    break;

  case XA_WM_TRANSIENT_FOR:
    // determine if this is a transient window
    Window win;
    if (XGetTransientForHint(display, client.window, &win)) {
      if (win && (win != client.window)) {
        if ((client.transient_for = openbox.searchWindow(win))) {
          client.transient_for->client.transient = this;
          flags.stuck = client.transient_for->flags.stuck;
          flags.transient = True;
        } else if (win == client.window_group) {
	  //jr This doesn't look quite right...
          if ((client.transient_for = openbox.searchGroup(win, this))) {
            client.transient_for->client.transient = this;
            flags.stuck = client.transient_for->flags.stuck;
            flags.transient = True;
          }
        }
      }

      if (win == screen->getRootWindow()) flags.modal = True;
    }

    // adjust the window decorations based on transience
    if (flags.transient)
      decorations.maximize = decorations.handle = functions.maximize = False;

    reconfigure();

    break;

  case XA_WM_HINTS:
    getWMHints();
    break;

  case XA_WM_ICON_NAME:
    getWMIconName();
    if (flags.iconic) screen->iconUpdate();
    break;

  case XA_WM_NAME:
    getWMName();

    if (decorations.titlebar)
      redrawLabel();

    if (! flags.iconic)
      screen->getWorkspace(workspace_number)->update();

    break;

  case XA_WM_NORMAL_HINTS: {
    getWMNormalHints();

    if ((client.normal_hint_flags & PMinSize) &&
        (client.normal_hint_flags & PMaxSize)) {
      if (client.max_width <= client.min_width &&
          client.max_height <= client.min_height)
        decorations.maximize = decorations.handle =
	    functions.resize = functions.maximize = False;
      else
        decorations.maximize = decorations.handle =
	    functions.resize = functions.maximize = True;
    }

    int x = frame.x, y = frame.y;
    unsigned int w = frame.width, h = frame.height;

    upsize();

    if ((x != frame.x) || (y != frame.y) ||
        (w != frame.width) || (h != frame.height))
      reconfigure();

    break;
  }

  default:
    if (atom == openbox.getWMProtocolsAtom()) {
      getWMProtocols();

      if (decorations.close && (! frame.close_button)) {
        createCloseButton();
        if (decorations.titlebar) positionButtons(True);
        if (windowmenu) windowmenu->reconfigure();
      }
    }

    break;
  }

  openbox.ungrab();
}


void OpenboxWindow::exposeEvent(XExposeEvent *ee) {
  if (frame.label == ee->window && decorations.titlebar)
    redrawLabel();
  else if (frame.close_button == ee->window)
    redrawCloseButton(False);
  else if (frame.maximize_button == ee->window)
    redrawMaximizeButton(flags.maximized);
  else if (frame.iconify_button == ee->window)
    redrawIconifyButton(False);
}


void OpenboxWindow::configureRequestEvent(XConfigureRequestEvent *cr) {
  if (cr->window == client.window) {
    openbox.grab();
    if (! validateClient()) return;

    int cx = frame.x, cy = frame.y;
    unsigned int cw = frame.width, ch = frame.height;

    if (cr->value_mask & CWBorderWidth)
      client.old_bw = cr->border_width;

    if (cr->value_mask & CWX)
      cx = cr->x - frame.mwm_border_w - frame.border_w;

    if (cr->value_mask & CWY)
      cy = cr->y - frame.y_border - frame.mwm_border_w -
        frame.border_w;

    if (cr->value_mask & CWWidth)
      cw = cr->width + (frame.mwm_border_w * 2);

    if (cr->value_mask & CWHeight)
      ch = cr->height + frame.y_border + (frame.mwm_border_w * 2) +
        (frame.border_w * decorations.handle) + frame.handle_h;

    if (frame.x != cx || frame.y != cy ||
        frame.width != cw || frame.height != ch)
      configure(cx, cy, cw, ch);

    if (cr->value_mask & CWStackMode) {
      switch (cr->detail) {
      case Above:
      case TopIf:
      default:
	if (flags.iconic) deiconify();
	screen->getWorkspace(workspace_number)->raiseWindow(this);
	break;

      case Below:
      case BottomIf:
	if (flags.iconic) deiconify();
	screen->getWorkspace(workspace_number)->lowerWindow(this);
	break;
      }
    }

    openbox.ungrab();
  }
}


void OpenboxWindow::buttonPressEvent(XButtonEvent *be) {
  openbox.grab();
  if (! validateClient())
    return;

  int stack_change = 1; // < 0 means to lower the window
                        // > 0 means to raise the window
                        // 0 means to leave it where it is
  
  // alt + left/right click begins interactively moving/resizing the window
  // when the mouse is moved
  if (be->state == Mod1Mask && (be->button == 1 || be->button == 3)) {
    frame.grab_x = be->x_root - frame.x - frame.border_w;
    frame.grab_y = be->y_root - frame.y - frame.border_w;
    if (be->button == 3) {
      if (screen->getWindowZones() == 4 &&
          be->y < (signed) frame.height / 2) {
        resize_zone = ZoneTop;
      } else {
        resize_zone = ZoneBottom;
      }
      if (screen->getWindowZones() >= 2 &&
          be->x < (signed) frame.width / 2) {
        resize_zone |= ZoneLeft;
      } else {
        resize_zone |= ZoneRight;
      }
    }
  // control + left click on the titlebar shades the window
  } else if (be->state == ControlMask && be->button == 1) {
    if (be->window == frame.title ||
        be->window == frame.label)
      shade();
  // left click
  } else if (be->state == 0 && be->button == 1) {
    if (windowmenu && windowmenu->isVisible())
        windowmenu->hide();

    if (be->window == frame.maximize_button) {
      redrawMaximizeButton(True);
    } else if (be->window == frame.iconify_button) {
      redrawIconifyButton(True);
    } else if (be->window == frame.close_button) {
      redrawCloseButton(True);
    } else if (be->window == frame.plate) {
      XAllowEvents(display, ReplayPointer, be->time);
    } else if (be->window == frame.title ||
               be->window == frame.label) {
      // shade the window when the titlebar is double clicked
      if ( (be->time - lastButtonPressTime) <=
            openbox.getDoubleClickInterval()) {
        lastButtonPressTime = 0;
        shade();
      } else {
        lastButtonPressTime = be->time;
      }
      // clicking and dragging on the titlebar moves the window, so on a click
      // we need to save the coords of the click in case the user drags
      frame.grab_x = be->x_root - frame.x - frame.border_w;
      frame.grab_y = be->y_root - frame.y - frame.border_w;
    } else if (be->window == frame.handle ||
               be->window == frame.left_grip ||
               be->window == frame.right_grip ||
               be->window == frame.window) {
      // clicking and dragging on the window's frame moves the window, so on a
      // click we need to save the coords of the click in case the user drags
      frame.grab_x = be->x_root - frame.x - frame.border_w;
      frame.grab_y = be->y_root - frame.y - frame.border_w;
      if (be->window == frame.left_grip)
        resize_zone = ZoneBottom | ZoneLeft;
      else if (be->window == frame.right_grip)
        resize_zone = ZoneBottom | ZoneRight;
    }
  // middle click
  } else if (be->state == 0 && be->button == 2) {
    if (be->window == frame.maximize_button) {
      redrawMaximizeButton(True);
    // a middle click anywhere on the window's frame except for on the buttons
    // will lower the window
    } else if (! (be->window == frame.iconify_button ||
                  be->window == frame.close_button) ) {
      stack_change = -1;
    }
  // right click
  } else if (be->state == 0 && be->button == 3) {
    if (be->window == frame.maximize_button) {
      redrawMaximizeButton(True);
    // a right click on the window's frame will show or hide the window's
    // windowmenu
    } else if (be->window == frame.title ||
               be->window == frame.label ||
               be->window == frame.handle ||
               be->window == frame.window) {
      int mx, my;
      if (windowmenu) {
        if (windowmenu->isVisible()) {
          windowmenu->hide();
        } else {
          // get the coords for the menu
          mx = be->x_root - windowmenu->getWidth() / 2;
          if (be->window == frame.title || be->window == frame.label) {
            my = frame.y + frame.title_h;
          } else if (be->window = frame.handle) {
            my = frame.y + frame.y_handle - windowmenu->getHeight();
          } else { // (be->window == frame.window)
            if (be->y <= (signed) frame.bevel_w) {
              my = frame.y + frame.y_border;
            } else {
              my = be->y_root - (windowmenu->getHeight() / 2);
            }
          }

          if (mx > (signed) (frame.x + frame.width -
              windowmenu->getWidth())) {
            mx = frame.x + frame.width - windowmenu->getWidth();
          } else if (mx < frame.x) {
            mx = frame.x;
          }

          if (my > (signed) (frame.y + frame.y_handle -
                windowmenu->getHeight())) {
            my = frame.y + frame.y_handle - windowmenu->getHeight();
          } else if (my < (signed) (frame.y +
              ((decorations.titlebar) ? frame.title_h : frame.y_border))) {
            my = frame.y +
              ((decorations.titlebar) ? frame.title_h : frame.y_border);
          }

          windowmenu->move(mx, my);
          windowmenu->show();
          XRaiseWindow(display, windowmenu->getWindowID());
          XRaiseWindow(display, windowmenu->getSendToMenu()->getWindowID());
          stack_change = 0;  // dont raise the window overtop of the menu
        }
      }
    }
  // mouse wheel up
  } else if (be->state == 0 && be->button == 4) {
    if ((be->window == frame.label ||
        be->window == frame.title) &&
        !flags.shaded)
      shade();
  // mouse wheel down
  } else if (be->state == 0 && be->button == 5) {
    if ((be->window == frame.label ||
        be->window == frame.title) &&
        flags.shaded)
      shade();
  }

  if (! (flags.focused || screen->sloppyFocus()) ) {
    setInputFocus();  // any click focus' the window in 'click to focus'
  }
  if (stack_change < 0) {
    screen->getWorkspace(workspace_number)->lowerWindow(this);
  } else if (stack_change > 0) {
    screen->getWorkspace(workspace_number)->raiseWindow(this);
  }
 
  openbox.ungrab();
}


void OpenboxWindow::buttonReleaseEvent(XButtonEvent *re) {
  openbox.grab();
  if (! validateClient())
    return;

  // alt + middle button released
  if (re->state == (Mod1Mask & Button2Mask) && re->button == 2) {
    if (re->window == frame.window) {
      XUngrabPointer(display, CurrentTime); // why? i dont know
    }
  // left button released
  } else if (re->button == 1) {
    if (re->window == frame.maximize_button) {
      if (re->state == Button1Mask && // only the left button was depressed
          (re->x >= 0) && ((unsigned) re->x <= frame.button_w) &&
          (re->y >= 0) && ((unsigned) re->y <= frame.button_h)) {
        maximize(re->button);
      } else {
        redrawMaximizeButton(False);
      }
    } else if (re->window == frame.iconify_button) {
      if (re->state == Button1Mask && // only the left button was depressed
          (re->x >= 0) && ((unsigned) re->x <= frame.button_w) &&
          (re->y >= 0) && ((unsigned) re->y <= frame.button_h)) {
        iconify();
      } else {
        redrawIconifyButton(False);
      }
    } else if (re->window == frame.close_button) {
      if (re->state == Button1Mask && // only the left button was depressed
          (re->x >= 0) && ((unsigned) re->x <= frame.button_w) &&
          (re->y >= 0) && ((unsigned) re->y <= frame.button_h)) {
          close();
      }
      //we should always redraw the close button. some applications
      //eg. acroread don't honour the close.
      redrawCloseButton(False);
    }
  // middle button released
  } else if (re->button == 2) {
    if (re->window == frame.maximize_button) {
      if (re->state == Button2Mask && // only the middle button was depressed
          (re->x >= 0) && ((unsigned) re->x <= frame.button_w) &&
          (re->y >= 0) && ((unsigned) re->y <= frame.button_h)) {
        maximize(re->button);
      } else {
        redrawMaximizeButton(False);
      }
    }
  // right button released
  } else if (re->button == 3) {
    if (re->window == frame.maximize_button) {
      if (re->state == Button3Mask && // only the right button was depressed
          (re->x >= 0) && ((unsigned) re->x <= frame.button_w) &&
          (re->y >= 0) && ((unsigned) re->y <= frame.button_h)) {
        maximize(re->button);
      } else {
        redrawMaximizeButton(False);
      }
    }
  }
  
  // when the window is being interactively moved, a button release stops the
  // move where it is
  if (flags.moving) {
    flags.moving = False;

    openbox.maskWindowEvents(0, (OpenboxWindow *) 0);
    if (!screen->opaqueMove()) {
      XDrawRectangle(display, screen->getRootWindow(), screen->getOpGC(),
                     frame.move_x, frame.move_y, frame.resize_w - 1,
                     frame.resize_h - 1);

      configure(frame.move_x, frame.move_y, frame.width, frame.height);
      openbox.ungrab();
    } else {
      configure(frame.x, frame.y, frame.width, frame.height);
    }
    screen->hideGeometry();
    XUngrabPointer(display, CurrentTime);
  // when the window is being interactively resized, a button release stops the
  // resizing
  } else if (flags.resizing) {
    flags.resizing = False;
    XDrawRectangle(display, screen->getRootWindow(), screen->getOpGC(),
                   frame.resize_x, frame.resize_y,
                   frame.resize_w - 1, frame.resize_h - 1);
    screen->hideGeometry();
    if (resize_zone & ZoneLeft) {
      left_fixsize();
    } else {  // when resizing with "Alt+Button3", the resize is the same as if
              // done with the right grip (the right side of the window is what
              // moves)
      right_fixsize();
    }
    // unset maximized state when resized after fully maximized
    if (flags.maximized == 1) {
        maximize(0);
    }
    configure(frame.resize_x, frame.resize_y,
              frame.resize_w - (frame.border_w * 2),
              frame.resize_h - (frame.border_w * 2));
    openbox.ungrab();
    XUngrabPointer(display, CurrentTime);
    resize_zone = 0;
  }

  openbox.ungrab();
}


void OpenboxWindow::motionNotifyEvent(XMotionEvent *me) {
  if (!flags.resizing && (me->state & Button1Mask) && functions.move &&
      (frame.title == me->window || frame.label == me->window ||
       frame.handle == me->window || frame.window == me->window)) {
    if (! flags.moving) {
      XGrabPointer(display, me->window, False, Button1MotionMask |
                   ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
                   None, openbox.getMoveCursor(), CurrentTime);

      if (windowmenu && windowmenu->isVisible())
        windowmenu->hide();

      flags.moving = True;

      openbox.maskWindowEvents(client.window, this);

      if (! screen->opaqueMove()) {
        openbox.grab();

        frame.move_x = frame.x;
	frame.move_y = frame.y;
        frame.resize_w = frame.width + (frame.border_w * 2);
        frame.resize_h = ((flags.shaded) ? frame.title_h : frame.height) +
          (frame.border_w * 2);

	screen->showPosition(frame.x, frame.y);

	XDrawRectangle(display, screen->getRootWindow(), screen->getOpGC(),
		       frame.move_x, frame.move_y,
		       frame.resize_w - 1, frame.resize_h - 1);
      }
    } else {
      int dx = me->x_root - frame.grab_x, dy = me->y_root - frame.grab_y;

      dx -= frame.border_w;
      dy -= frame.border_w;

      int snap_distance = screen->edgeSnapThreshold();
      // width/height of the snapping window
      unsigned int snap_w = frame.width + (frame.border_w * 2);
      unsigned int snap_h = area().h() + (frame.border_w * 2);
      if (snap_distance) {
        int drx = screen->size().w() - (dx + snap_w);

        if (dx < drx && (dx > 0 && dx < snap_distance) ||
                        (dx < 0 && dx > -snap_distance) )
          dx = 0;
        else if ( (drx > 0 && drx < snap_distance) ||
                  (drx < 0 && drx > -snap_distance) )
          dx = screen->size().w() - snap_w;

        int dtty, dbby, dty, dby;
        switch (screen->getToolbar()->placement()) {
        case Toolbar::TopLeft:
        case Toolbar::TopCenter:
        case Toolbar::TopRight:
          dtty = screen->getToolbar()->getExposedHeight() +
	         frame.border_w;
          dbby = screen->size().h();
          break;

        default:
          dtty = 0;
	  dbby = screen->getToolbar()->area().y();
          break;
        }

        dty = dy - dtty;
        dby = dbby - (dy + snap_h);

        if ( (dy > 0 && dty < snap_distance) ||
            (dy < 0 && dty > -snap_distance) )
          dy = dtty;
        else if ( (dby > 0 && dby < snap_distance) ||
                 (dby < 0 && dby > -snap_distance) )
          dy = dbby - snap_h;
      }

      if (screen->opaqueMove()) {
	configure(dx, dy, frame.width, frame.height);
      } else {
	XDrawRectangle(display, screen->getRootWindow(), screen->getOpGC(),
		       frame.move_x, frame.move_y, frame.resize_w - 1,
		       frame.resize_h - 1);

	frame.move_x = dx;
	frame.move_y = dy;

	XDrawRectangle(display, screen->getRootWindow(), screen->getOpGC(),
		       frame.move_x, frame.move_y, frame.resize_w - 1,
		       frame.resize_h - 1);
      }

      screen->showPosition(dx, dy);
    }
  } else if (functions.resize &&
	     (((me->state & Button1Mask) && (me->window == frame.right_grip ||
					     me->window == frame.left_grip)) ||
	      (me->state & (Mod1Mask | Button3Mask) &&
	                                     me->window == frame.window))) {
    Bool left = resize_zone & ZoneLeft;

    if (! flags.resizing) {
      Cursor cursor;
      if (resize_zone & ZoneTop)
        cursor = (resize_zone & ZoneLeft) ?
          openbox.getUpperLeftAngleCursor() :
          openbox.getUpperRightAngleCursor();
      else
        cursor = (resize_zone & ZoneLeft) ?
          openbox.getLowerLeftAngleCursor() :
          openbox.getLowerRightAngleCursor();
      XGrabPointer(display, me->window, False, ButtonMotionMask |
                   ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None,
                   cursor, CurrentTime);

      flags.resizing = True;

      openbox.grab();

      int gx, gy;
      if (resize_zone & ZoneRight)
        frame.grab_x = me->x - screen->getBorderWidth();
      else
        frame.grab_x = me->x + screen->getBorderWidth();
      if (resize_zone & ZoneTop)
        frame.grab_y = me->y + screen->getBorderWidth() * 2;
      else
        frame.grab_y = me->y - screen->getBorderWidth() * 2;
      frame.resize_x = frame.x;
      frame.resize_y = frame.y;
      frame.resize_w = frame.width + (frame.border_w * 2);
      frame.resize_h = frame.height + (frame.border_w * 2);

      if (left)
        left_fixsize(&gx, &gy);
      else
	right_fixsize(&gx, &gy);

      screen->showGeometry(gx, gy);

      XDrawRectangle(display, screen->getRootWindow(), screen->getOpGC(),
		     frame.resize_x, frame.resize_y,
		     frame.resize_w - 1, frame.resize_h - 1);
    } else {
      XDrawRectangle(display, screen->getRootWindow(), screen->getOpGC(),
		     frame.resize_x, frame.resize_y,
		     frame.resize_w - 1, frame.resize_h - 1);

      int gx, gy;

      if (resize_zone & ZoneTop)
        frame.resize_h = frame.height - (me->y - frame.grab_y);
      else
        frame.resize_h = frame.height + (me->y - frame.grab_y);
      if (frame.resize_h < 1) frame.resize_h = 1;

      if (left) {
        frame.resize_x = me->x_root - frame.grab_x;
        if (frame.resize_x > (signed) (frame.x + frame.width))
          frame.resize_x = frame.resize_x + frame.width - 1;

        left_fixsize(&gx, &gy);
      } else {
	frame.resize_w = frame.width + (me->x - frame.grab_x);
	if (frame.resize_w < 1) frame.resize_w = 1;

	right_fixsize(&gx, &gy);
      }

      XDrawRectangle(display, screen->getRootWindow(), screen->getOpGC(),
		     frame.resize_x, frame.resize_y,
		     frame.resize_w - 1, frame.resize_h - 1);

      screen->showGeometry(gx, gy);
    }
  }
}


#ifdef    SHAPE
void OpenboxWindow::shapeEvent(XShapeEvent *) {
  if (openbox.hasShapeExtensions()) {
    if (flags.shaped) {
      openbox.grab();
      if (! validateClient()) return;
      XShapeCombineShape(display, frame.window, ShapeBounding,
			 frame.mwm_border_w, frame.y_border +
			 frame.mwm_border_w, client.window,
			 ShapeBounding, ShapeSet);

      int num = 1;
      XRectangle xrect[2];
      xrect[0].x = xrect[0].y = 0;
      xrect[0].width = frame.width;
      xrect[0].height = frame.y_border;

      if (decorations.handle) {
	xrect[1].x = 0;
	xrect[1].y = frame.y_handle;
	xrect[1].width = frame.width;
	xrect[1].height = frame.handle_h + frame.border_w;
	num++;
      }

      XShapeCombineRectangles(display, frame.window, ShapeBounding, 0, 0,
			      xrect, num, ShapeUnion, Unsorted);
      openbox.ungrab();
    }
  }
}
#endif // SHAPE


Bool OpenboxWindow::validateClient(void) {
  XSync(display, False);

  XEvent e;
  if (XCheckTypedWindowEvent(display, client.window, DestroyNotify, &e) ||
      XCheckTypedWindowEvent(display, client.window, UnmapNotify, &e)) {
    XPutBackEvent(display, &e);
    openbox.ungrab();

    return False;
  }

  return True;
}


void OpenboxWindow::restore(void) {
  XChangeSaveSet(display, client.window, SetModeDelete);
  XSelectInput(display, client.window, NoEventMask);

  restoreGravity();

  XUnmapWindow(display, frame.window);
  XUnmapWindow(display, client.window);

  XSetWindowBorderWidth(display, client.window, client.old_bw);
  XReparentWindow(display, client.window, screen->getRootWindow(),
                  client.x, client.y);
  XMapWindow(display, client.window);

  XFlush(display);
}


void OpenboxWindow::timeout(void) {
  screen->getWorkspace(workspace_number)->raiseWindow(this);
}


void OpenboxWindow::changeOpenboxHints(OpenboxHints *net) {
  if ((net->flags & AttribShaded) &&
      ((openbox_attrib.attrib & AttribShaded) !=
       (net->attrib & AttribShaded)))
    shade();

  if (flags.visible && // watch out for requests when we can not be seen
      (net->flags & (AttribMaxVert | AttribMaxHoriz)) &&
      ((openbox_attrib.attrib & (AttribMaxVert | AttribMaxHoriz)) !=
       (net->attrib & (AttribMaxVert | AttribMaxHoriz)))) {
    if (flags.maximized) {
      maximize(0);
    } else {
      int button = 0;

      if ((net->flags & AttribMaxHoriz) && (net->flags & AttribMaxVert))
        button = ((net->attrib & (AttribMaxHoriz | AttribMaxVert)) ?  1 : 0);
      else if (net->flags & AttribMaxVert)
        button = ((net->attrib & AttribMaxVert) ? 2 : 0);
      else if (net->flags & AttribMaxHoriz)
        button = ((net->attrib & AttribMaxHoriz) ? 3 : 0);

      maximize(button);
    }
  }

  if ((net->flags & AttribOmnipresent) &&
      ((openbox_attrib.attrib & AttribOmnipresent) !=
       (net->attrib & AttribOmnipresent)))
    stick();

  if ((net->flags & AttribWorkspace) &&
      (workspace_number != (signed) net->workspace)) {
    screen->reassociateWindow(this, net->workspace, True);

    if (screen->getCurrentWorkspaceID() != (signed) net->workspace) withdraw();
    else deiconify();
  }

  if (net->flags & AttribDecoration) {
    switch (net->decoration) {
    case DecorNone:
      decorations.titlebar = decorations.border = decorations.handle =
       decorations.iconify = decorations.maximize = decorations.menu = False;

      break;

    default:
    case DecorNormal:
      decorations.titlebar = decorations.border = decorations.handle =
       decorations.iconify = decorations.maximize = decorations.menu = True;

      break;

    case DecorTiny:
      decorations.titlebar = decorations.iconify = decorations.menu = True;
      decorations.border = decorations.handle = decorations.maximize = False;
 
      break;

    case DecorTool:
      decorations.titlebar = decorations.menu = functions.move = True;
      decorations.iconify = decorations.border = decorations.handle =
	decorations.maximize = False;

      break;
    }
    if (frame.window) {
      XMapSubwindows(display, frame.window);
      XMapWindow(display, frame.window);
    }

    reconfigure();
    setState(current_state);
  }
}


/*
 * Set the sizes of all components of the window frame
 * (the window decorations).
 * These values are based upon the current style settings and the client
 * window's dimentions.
 */
void OpenboxWindow::upsize(void) {
  frame.bevel_w = screen->getBevelWidth();

  if (decorations.border) {
    frame.border_w = screen->getBorderWidth();
    if (!flags.transient)
      frame.mwm_border_w = screen->getFrameWidth();
    else
      frame.mwm_border_w = 0;
  } else {
    frame.mwm_border_w = frame.border_w = 0;
  }

  if (decorations.titlebar) {
    // the height of the titlebar is based upon the height of the font being
    // used to display the window's title
    WindowStyle *style = screen->getWindowStyle();
    if (i18n->multibyte())
      frame.title_h = (style->fontset_extents->max_ink_extent.height +
		       (frame.bevel_w * 2) + 2);
    else
      frame.title_h = (style->font->ascent + style->font->descent +
		       (frame.bevel_w * 2) + 2);

    frame.label_h = frame.title_h - (frame.bevel_w * 2);
    frame.button_w = frame.button_h = (frame.label_h - 2);
    frame.y_border = frame.title_h + frame.border_w;
  } else {
    frame.title_h = 0;
    frame.label_h = 0;
    frame.button_w = frame.button_h = 0;
    frame.y_border = 0;
  }

  frame.border_h = client.height + frame.mwm_border_w * 2;

  if (decorations.handle) {
    frame.y_handle = frame.y_border + frame.border_h + frame.border_w;
    frame.grip_w = frame.button_w * 2;
    frame.grip_h = frame.handle_h = screen->getHandleWidth();
  } else {
    frame.y_handle = frame.y_border + frame.border_h;
    frame.handle_h = 0;
    frame.grip_w = frame.grip_h = 0;
  }
  
  frame.width = client.width + (frame.mwm_border_w * 2);
  frame.height = frame.y_handle + frame.handle_h;
}


/*
 * Set the size and position of the client window.
 * These values are based upon the current style settings and the frame
 * window's dimensions.
 */
void OpenboxWindow::downsize(void) {
  frame.y_handle = frame.height - frame.handle_h;
  frame.border_h = frame.y_handle - frame.y_border -
    (decorations.handle ? frame.border_w : 0);

  client.x = frame.x + frame.mwm_border_w + frame.border_w;
  client.y = frame.y + frame.y_border + frame.mwm_border_w + frame.border_w;

  client.width = frame.width - (frame.mwm_border_w * 2);
  client.height = frame.height - frame.y_border - (frame.mwm_border_w * 2)
    - frame.handle_h - (decorations.handle ? frame.border_w : 0);

  frame.y_handle = frame.border_h + frame.y_border + frame.border_w;
}


void OpenboxWindow::right_fixsize(int *gx, int *gy) {
  // calculate the size of the client window and conform it to the
  // size specified by the size hints of the client window...
  int dx = frame.resize_w - client.base_width - (frame.mwm_border_w * 2) -
    (frame.border_w * 2) + (client.width_inc / 2);
  int dy = frame.resize_h - frame.y_border - client.base_height -
    frame.handle_h - (frame.border_w * 3) - (frame.mwm_border_w * 2)
    + (client.height_inc / 2);

  if (dx < (signed) client.min_width) dx = client.min_width;
  if (dy < (signed) client.min_height) dy = client.min_height;
  if ((unsigned) dx > client.max_width) dx = client.max_width;
  if ((unsigned) dy > client.max_height) dy = client.max_height;

  dx /= client.width_inc;
  dy /= client.height_inc;

  if (gx) *gx = dx;
  if (gy) *gy = dy;

  dx = (dx * client.width_inc) + client.base_width;
  dy = (dy * client.height_inc) + client.base_height;

  frame.resize_w = dx + (frame.mwm_border_w * 2) + (frame.border_w * 2);
  frame.resize_h = dy + frame.y_border + frame.handle_h +
                   (frame.mwm_border_w * 2) +  (frame.border_w * 3);
  if (resize_zone & ZoneTop)
    frame.resize_y = frame.y + frame.height - frame.resize_h +
      screen->getBorderWidth() * 2;
}


void OpenboxWindow::left_fixsize(int *gx, int *gy) {
  // calculate the size of the client window and conform it to the
  // size specified by the size hints of the client window...
  int dx = frame.x + frame.width - frame.resize_x - client.base_width -
    (frame.mwm_border_w * 2) + (client.width_inc / 2);
  int dy = frame.resize_h - frame.y_border - client.base_height -
    frame.handle_h - (frame.border_w * 3) - (frame.mwm_border_w * 2)
    + (client.height_inc / 2);

  if (dx < (signed) client.min_width) dx = client.min_width;
  if (dy < (signed) client.min_height) dy = client.min_height;
  if ((unsigned) dx > client.max_width) dx = client.max_width;
  if ((unsigned) dy > client.max_height) dy = client.max_height;

  dx /= client.width_inc;
  dy /= client.height_inc;

  if (gx) *gx = dx;
  if (gy) *gy = dy;

  dx = (dx * client.width_inc) + client.base_width;
  dy = (dy * client.height_inc) + client.base_height;

  frame.resize_w = dx + (frame.mwm_border_w * 2) + (frame.border_w * 2);
  frame.resize_x = frame.x + frame.width - frame.resize_w +
                   (frame.border_w * 2);
  frame.resize_h = dy + frame.y_border + frame.handle_h +
                   (frame.mwm_border_w * 2) + (frame.border_w * 3);
  if (resize_zone & ZoneTop)
    frame.resize_y = frame.y + frame.height - frame.resize_h +
      screen->getBorderWidth() * 2;
  
}
