// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Window.cc for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh at debian.org>
// Copyright (c) 1997 - 2000, 2002 Brad Hughes <bhughes at trolltech.com>
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

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <X11/Xatom.h>
#include <X11/keysym.h>

#ifdef HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#ifdef    DEBUG
#  ifdef    HAVE_STDIO_H
#    include <stdio.h>
#  endif // HAVE_STDIO_H
#endif // DEBUG

#ifdef HAVE_STDLIB_H
   #include <stdlib.h>
#endif // HAVE_STDLIB_H
}

#include "i18n.hh"
#include "blackbox.hh"
#include "Clientmenu.hh"
#include "Font.hh"
#include "GCCache.hh"
#include "Iconmenu.hh"
#include "Image.hh"
#include "Screen.hh"
#include "Toolbar.hh"
#include "Util.hh"
#include "Window.hh"
#include "Windowmenu.hh"
#include "Workspace.hh"
#include "Slit.hh"

using std::string;
using std::abs;

// change this to change what modifier keys openbox uses for mouse bindings
// for example: Mod1Mask | ControlMask
//          or: ControlMask| ShiftMask
const unsigned int ModMask = Mod1Mask;

/*
 * Initializes the class with default values/the window's set initial values.
 */
BlackboxWindow::BlackboxWindow(Blackbox *b, Window w, BScreen *s) {
  // fprintf(stderr, "BlackboxWindow size: %d bytes\n",
  // sizeof(BlackboxWindow));

#ifdef    DEBUG
  fprintf(stderr, "BlackboxWindow::BlackboxWindow(): creating 0x%lx\n", w);
#endif // DEBUG

  /*
    set timer to zero... it is initialized properly later, so we check
    if timer is zero in the destructor, and assume that the window is not
    fully constructed if timer is zero...
  */
  timer = 0;
  blackbox = b;
  client.window = w;
  screen = s;
  xatom = blackbox->getXAtom();

  if (! validateClient()) {
    delete this;
    return;
  }

  // fetch client size and placement
  XWindowAttributes wattrib;
  if (! XGetWindowAttributes(blackbox->getXDisplay(),
                             client.window, &wattrib) ||
      ! wattrib.screen || wattrib.override_redirect) {
#ifdef    DEBUG
    fprintf(stderr,
            "BlackboxWindow::BlackboxWindow(): XGetWindowAttributes failed\n");
#endif // DEBUG

    delete this;
    return;
  }

  // set the eventmask early in the game so that we make sure we get
  // all the events we are interested in
  XSetWindowAttributes attrib_set;
  attrib_set.event_mask = PropertyChangeMask | FocusChangeMask |
                          StructureNotifyMask;
  attrib_set.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask |
                                     ButtonMotionMask;
  XChangeWindowAttributes(blackbox->getXDisplay(), client.window,
                          CWEventMask|CWDontPropagate, &attrib_set);

  flags.moving = flags.resizing = flags.shaded = flags.visible =
    flags.iconic = flags.focused = flags.stuck = flags.modal =
    flags.send_focus_message = flags.shaped = flags.skip_taskbar =
    flags.skip_pager = flags.fullscreen = False;
  flags.maximized = 0;

  blackbox_attrib.workspace = window_number = BSENTINEL;

  blackbox_attrib.flags = blackbox_attrib.attrib = blackbox_attrib.stack
    = blackbox_attrib.decoration = 0l;
  blackbox_attrib.premax_x = blackbox_attrib.premax_y = 0;
  blackbox_attrib.premax_w = blackbox_attrib.premax_h = 0;

  frame.border_w = 1;
  frame.window = frame.plate = frame.title = frame.handle = None;
  frame.close_button = frame.iconify_button = frame.maximize_button = None;
  frame.right_grip = frame.left_grip = None;

  frame.ulabel_pixel = frame.flabel_pixel = frame.utitle_pixel =
  frame.ftitle_pixel = frame.uhandle_pixel = frame.fhandle_pixel =
    frame.ubutton_pixel = frame.fbutton_pixel = frame.pbutton_pixel =
    frame.uborder_pixel = frame.fborder_pixel = frame.ugrip_pixel =
    frame.fgrip_pixel = 0;
  frame.utitle = frame.ftitle = frame.uhandle = frame.fhandle = None;
  frame.ulabel = frame.flabel = frame.ubutton = frame.fbutton = None;
  frame.pbutton = frame.ugrip = frame.fgrip = None;

  decorations = Decor_Titlebar | Decor_Border | Decor_Handle |
                Decor_Iconify | Decor_Maximize;
  functions = Func_Resize | Func_Move | Func_Iconify | Func_Maximize;

  client.normal_hint_flags = 0;
  client.window_group = None;
  client.transient_for = 0;

  current_state = NormalState;

  /*
    get the initial size and location of client window (relative to the
    _root window_). This position is the reference point used with the
    window's gravity to find the window's initial position.
  */
  client.rect.setRect(wattrib.x, wattrib.y, wattrib.width, wattrib.height);
  client.old_bw = wattrib.border_width;

  lastButtonPressTime = 0;

  timer = new BTimer(blackbox, this);
  timer->setTimeout(blackbox->getAutoRaiseDelay());

  windowmenu = new Windowmenu(this);

  // get size, aspect, minimum/maximum size and other hints set by the
  // client

  if (! getBlackboxHints()) {
    getMWMHints();
    getNetWMHints();
  }

  getWMProtocols();
  getWMHints();
  getWMNormalHints();

  frame.window = createToplevelWindow();

  blackbox->saveWindowSearch(frame.window, this);
  
  frame.plate = createChildWindow(frame.window);
  blackbox->saveWindowSearch(frame.plate, this);

  // determine if this is a transient window
  getTransientInfo();

  // determine the window's type, so we can decide its decorations and
  // functionality, or if we should not manage it at all
  getWindowType();

  // adjust the window decorations/behavior based on the window type

  switch (window_type) {
  case Type_Desktop:
  case Type_Dock:
  case Type_Menu:
  case Type_Toolbar:
  case Type_Utility:
  case Type_Splash:
    // none of these windows are decorated or manipulated by the window manager
    decorations = 0;
    functions = 0;
    blackbox_attrib.workspace = 0;  // we do need to belong to a workspace
    flags.stuck = True;             // we show up on all workspaces
    break;

  case Type_Dialog:
    // dialogs cannot be maximized, and don't display a handle
    decorations &= ~(Decor_Maximize | Decor_Handle);
    functions &= ~Func_Maximize;
    break;

  case Type_Normal:
    // normal windows retain all of the possible decorations and functionality
    break;
  }
  
  setAllowedActions();

  // further adjeust the window's decorations/behavior based on window sizes
  if ((client.normal_hint_flags & PMinSize) &&
      (client.normal_hint_flags & PMaxSize) &&
      client.max_width <= client.min_width &&
      client.max_height <= client.min_height) {
    decorations &= ~(Decor_Maximize | Decor_Handle);
    functions &= ~(Func_Resize | Func_Maximize);
  }
  
  if (decorations & Decor_Titlebar)
    createTitlebar();

  if (decorations & Decor_Handle)
    createHandle();

  // apply the size and gravity hint to the frame

  upsize();

  bool place_window = True;
  if (blackbox->isStartup() || isTransient() ||
      client.normal_hint_flags & (PPosition|USPosition)) {
    applyGravity(frame.rect);

    if (blackbox->isStartup() || client.rect.intersects(screen->getRect()))
      place_window = False;
  }

  // add the window's strut. note this is done *after* placing the window.
  screen->addStrut(&client.strut);
  updateStrut();
  
#ifdef    SHAPE
  if (blackbox->hasShapeExtensions() && flags.shaped)
    configureShape();
#endif // SHAPE
  
  // get the window's title before adding it to the workspace
  getWMName();
  getWMIconName();

  if (blackbox_attrib.workspace >= screen->getWorkspaceCount())
    screen->getCurrentWorkspace()->addWindow(this, place_window);
  else
    screen->getWorkspace(blackbox_attrib.workspace)->
      addWindow(this, place_window);

  /*
    the server needs to be grabbed here to prevent client's from sending
    events while we are in the process of configuring their window.
    We hold the grab until after we are done moving the window around.
  */

  XGrabServer(blackbox->getXDisplay());

  associateClientWindow();

  blackbox->saveWindowSearch(client.window, this);

  if (! place_window) {
    // don't need to call configure if we are letting the workspace
    // place the window
    configure(frame.rect.x(), frame.rect.y(),
              frame.rect.width(), frame.rect.height());

  }

  positionWindows();

  XUngrabServer(blackbox->getXDisplay());

  // now that we know where to put the window and what it should look like
  // we apply the decorations
  decorate();

  grabButtons();

  XMapSubwindows(blackbox->getXDisplay(), frame.window);

  // this ensures the title, buttons, and other decor are properly displayed
  redrawWindowFrame();

  // preserve the window's initial state on first map, and its current state
  // across a restart
  unsigned long initial_state = current_state;
  if (! getState())
    current_state = initial_state;

  // get sticky state from our parent window if we've got one
  if (isTransient() && client.transient_for != (BlackboxWindow *) ~0ul &&
      client.transient_for->isStuck() != flags.stuck)
    flags.stuck = True;

  if (flags.shaded) {
    flags.shaded = False;
    initial_state = current_state;
    shade();

    /*
      At this point in the life of a window, current_state should only be set
      to IconicState if the window was an *icon*, not if it was shaded.
    */
    if (initial_state != IconicState)
      current_state = NormalState;
  }

  if (flags.stuck) {
    flags.stuck = False;
    stick();
  }

  if (flags.maximized && (functions & Func_Maximize))
    remaximize();
}


BlackboxWindow::~BlackboxWindow(void) {
#ifdef    DEBUG
  fprintf(stderr, "BlackboxWindow::~BlackboxWindow: destroying 0x%lx\n",
          client.window);
#endif // DEBUG

  if (! timer) // window not managed...
    return;

  screen->removeStrut(&client.strut);
  screen->updateAvailableArea();

  // We don't need to worry about resizing because resizing always grabs the X
  // server. This should only ever happen if using opaque moving.
  if (flags.moving)
    endMove();

  delete timer;

  delete windowmenu;

  if (client.window_group) {
    BWindowGroup *group = blackbox->searchGroup(client.window_group);
    if (group) group->removeWindow(this);
  }

  // remove ourselves from our transient_for
  if (isTransient()) {
    if (client.transient_for != (BlackboxWindow *) ~0ul) {
      client.transient_for->client.transientList.remove(this);
    }
    client.transient_for = (BlackboxWindow*) 0;
  }

  if (client.transientList.size() > 0) {
    // reset transient_for for all transients
    BlackboxWindowList::iterator it, end = client.transientList.end();
    for (it = client.transientList.begin(); it != end; ++it) {
      (*it)->client.transient_for = (BlackboxWindow*) 0;
    }
  }

  if (frame.title)
    destroyTitlebar();

  if (frame.handle)
    destroyHandle();

  if (frame.plate) {
    blackbox->removeWindowSearch(frame.plate);
    XDestroyWindow(blackbox->getXDisplay(), frame.plate);
  }

  if (frame.window) {
    blackbox->removeWindowSearch(frame.window);
    XDestroyWindow(blackbox->getXDisplay(), frame.window);
  }

  blackbox->removeWindowSearch(client.window);
}


/*
 * Creates a new top level window, with a given location, size, and border
 * width.
 * Returns: the newly created window
 */
Window BlackboxWindow::createToplevelWindow(void) {
  XSetWindowAttributes attrib_create;
  unsigned long create_mask = CWBackPixmap | CWBorderPixel | CWColormap |
                              CWOverrideRedirect | CWEventMask;

  attrib_create.background_pixmap = None;
  attrib_create.colormap = screen->getColormap();
  attrib_create.override_redirect = True;
  attrib_create.event_mask = ButtonPressMask | ButtonReleaseMask |
                             ButtonMotionMask | EnterWindowMask;

  return XCreateWindow(blackbox->getXDisplay(), screen->getRootWindow(),
                       0, 0, 1, 1, frame.border_w, screen->getDepth(),
                       InputOutput, screen->getVisual(), create_mask,
                       &attrib_create);
}


/*
 * Creates a child window, and optionally associates a given cursor with
 * the new window.
 */
Window BlackboxWindow::createChildWindow(Window parent, Cursor cursor) {
  XSetWindowAttributes attrib_create;
  unsigned long create_mask = CWBackPixmap | CWBorderPixel |
                              CWEventMask;

  attrib_create.background_pixmap = None;
  attrib_create.event_mask = ButtonPressMask | ButtonReleaseMask |
                             ButtonMotionMask | ExposureMask;

  if (cursor) {
    create_mask |= CWCursor;
    attrib_create.cursor = cursor;
  }

  return XCreateWindow(blackbox->getXDisplay(), parent, 0, 0, 1, 1, 0,
                       screen->getDepth(), InputOutput, screen->getVisual(),
                       create_mask, &attrib_create);
}


void BlackboxWindow::associateClientWindow(void) {
  XSetWindowBorderWidth(blackbox->getXDisplay(), client.window, 0);

  XChangeSaveSet(blackbox->getXDisplay(), client.window, SetModeInsert);

  XSelectInput(blackbox->getXDisplay(), frame.plate, SubstructureRedirectMask);

  /*
    note we used to grab around this call to XReparentWindow however the
    server is now grabbed before this method is called
  */
  unsigned long event_mask = PropertyChangeMask | FocusChangeMask |
                             StructureNotifyMask;
  XSelectInput(blackbox->getXDisplay(), client.window,
               event_mask & ~StructureNotifyMask);
  XReparentWindow(blackbox->getXDisplay(), client.window, frame.plate, 0, 0);
  XSelectInput(blackbox->getXDisplay(), client.window, event_mask);

  XRaiseWindow(blackbox->getXDisplay(), frame.plate);
  XMapSubwindows(blackbox->getXDisplay(), frame.plate);

#ifdef    SHAPE
  if (blackbox->hasShapeExtensions()) {
    XShapeSelectInput(blackbox->getXDisplay(), client.window,
                      ShapeNotifyMask);

    Bool shaped = False;
    int foo;
    unsigned int ufoo;

    XShapeQueryExtents(blackbox->getXDisplay(), client.window, &shaped,
                       &foo, &foo, &ufoo, &ufoo, &foo, &foo, &foo,
                       &ufoo, &ufoo);
    flags.shaped = shaped;
  }
#endif // SHAPE
}


void BlackboxWindow::decorate(void) {
  BTexture* texture;

  texture = &(screen->getWindowStyle()->b_focus);
  frame.fbutton = texture->render(frame.button_w, frame.button_w,
                                  frame.fbutton);
  if (! frame.fbutton)
    frame.fbutton_pixel = texture->color().pixel();

  texture = &(screen->getWindowStyle()->b_unfocus);
  frame.ubutton = texture->render(frame.button_w, frame.button_w,
                                  frame.ubutton);
  if (! frame.ubutton)
    frame.ubutton_pixel = texture->color().pixel();

  texture = &(screen->getWindowStyle()->b_pressed);
  frame.pbutton = texture->render(frame.button_w, frame.button_w,
                                  frame.pbutton);
  if (! frame.pbutton)
    frame.pbutton_pixel = texture->color().pixel();

  if (decorations & Decor_Titlebar) {
    texture = &(screen->getWindowStyle()->t_focus);
    frame.ftitle = texture->render(frame.inside_w, frame.title_h,
                                   frame.ftitle);
    if (! frame.ftitle)
      frame.ftitle_pixel = texture->color().pixel();

    texture = &(screen->getWindowStyle()->t_unfocus);
    frame.utitle = texture->render(frame.inside_w, frame.title_h,
                                   frame.utitle);
    if (! frame.utitle)
      frame.utitle_pixel = texture->color().pixel();

    XSetWindowBorder(blackbox->getXDisplay(), frame.title,
                     screen->getBorderColor()->pixel());

    decorateLabel();
  }

  if (decorations & Decor_Border) {
    frame.fborder_pixel = screen->getWindowStyle()->f_focus.pixel();
    frame.uborder_pixel = screen->getWindowStyle()->f_unfocus.pixel();
    blackbox_attrib.flags |= AttribDecoration;
    blackbox_attrib.decoration = DecorNormal;
  } else {
    blackbox_attrib.flags |= AttribDecoration;
    blackbox_attrib.decoration = DecorNone;
  }

  if (decorations & Decor_Handle) {
    texture = &(screen->getWindowStyle()->h_focus);
    frame.fhandle = texture->render(frame.inside_w, frame.handle_h,
                                    frame.fhandle);
    if (! frame.fhandle)
      frame.fhandle_pixel = texture->color().pixel();

    texture = &(screen->getWindowStyle()->h_unfocus);
    frame.uhandle = texture->render(frame.inside_w, frame.handle_h,
                                    frame.uhandle);
    if (! frame.uhandle)
      frame.uhandle_pixel = texture->color().pixel();

    texture = &(screen->getWindowStyle()->g_focus);
    frame.fgrip = texture->render(frame.grip_w, frame.handle_h, frame.fgrip);
    if (! frame.fgrip)
      frame.fgrip_pixel = texture->color().pixel();

    texture = &(screen->getWindowStyle()->g_unfocus);
    frame.ugrip = texture->render(frame.grip_w, frame.handle_h, frame.ugrip);
    if (! frame.ugrip)
      frame.ugrip_pixel = texture->color().pixel();

    XSetWindowBorder(blackbox->getXDisplay(), frame.handle,
                     screen->getBorderColor()->pixel());
    XSetWindowBorder(blackbox->getXDisplay(), frame.left_grip,
                     screen->getBorderColor()->pixel());
    XSetWindowBorder(blackbox->getXDisplay(), frame.right_grip,
                     screen->getBorderColor()->pixel());
  }

  XSetWindowBorder(blackbox->getXDisplay(), frame.window,
                   screen->getBorderColor()->pixel());
}


void BlackboxWindow::decorateLabel(void) {
  BTexture *texture;

  texture = &(screen->getWindowStyle()->l_focus);
  frame.flabel = texture->render(frame.label_w, frame.label_h, frame.flabel);
  if (! frame.flabel)
    frame.flabel_pixel = texture->color().pixel();

  texture = &(screen->getWindowStyle()->l_unfocus);
  frame.ulabel = texture->render(frame.label_w, frame.label_h, frame.ulabel);
  if (! frame.ulabel)
    frame.ulabel_pixel = texture->color().pixel();
}


void BlackboxWindow::createHandle(void) {
  frame.handle = createChildWindow(frame.window);
  blackbox->saveWindowSearch(frame.handle, this);

  frame.left_grip =
    createChildWindow(frame.handle, blackbox->getLowerLeftAngleCursor());
  blackbox->saveWindowSearch(frame.left_grip, this);

  frame.right_grip =
    createChildWindow(frame.handle, blackbox->getLowerRightAngleCursor());
  blackbox->saveWindowSearch(frame.right_grip, this);
}


void BlackboxWindow::destroyHandle(void) {
  if (frame.fhandle)
    screen->getImageControl()->removeImage(frame.fhandle);

  if (frame.uhandle)
    screen->getImageControl()->removeImage(frame.uhandle);

  if (frame.fgrip)
    screen->getImageControl()->removeImage(frame.fgrip);

  if (frame.ugrip)
    screen->getImageControl()->removeImage(frame.ugrip);

  blackbox->removeWindowSearch(frame.left_grip);
  blackbox->removeWindowSearch(frame.right_grip);

  XDestroyWindow(blackbox->getXDisplay(), frame.left_grip);
  XDestroyWindow(blackbox->getXDisplay(), frame.right_grip);
  frame.left_grip = frame.right_grip = None;

  blackbox->removeWindowSearch(frame.handle);
  XDestroyWindow(blackbox->getXDisplay(), frame.handle);
  frame.handle = None;
}


void BlackboxWindow::createTitlebar(void) {
  frame.title = createChildWindow(frame.window);
  frame.label = createChildWindow(frame.title);
  blackbox->saveWindowSearch(frame.title, this);
  blackbox->saveWindowSearch(frame.label, this);

  if (decorations & Decor_Iconify) createIconifyButton();
  if (decorations & Decor_Maximize) createMaximizeButton();
  if (decorations & Decor_Close) createCloseButton();
}


void BlackboxWindow::destroyTitlebar(void) {
  if (frame.close_button)
    destroyCloseButton();

  if (frame.iconify_button)
    destroyIconifyButton();

  if (frame.maximize_button)
    destroyMaximizeButton();

  if (frame.ftitle)
    screen->getImageControl()->removeImage(frame.ftitle);

  if (frame.utitle)
    screen->getImageControl()->removeImage(frame.utitle);

  if (frame.flabel)
    screen->getImageControl()->removeImage(frame.flabel);

  if( frame.ulabel)
    screen->getImageControl()->removeImage(frame.ulabel);

  if (frame.fbutton)
    screen->getImageControl()->removeImage(frame.fbutton);

  if (frame.ubutton)
    screen->getImageControl()->removeImage(frame.ubutton);

  if (frame.pbutton)
    screen->getImageControl()->removeImage(frame.pbutton);

  blackbox->removeWindowSearch(frame.title);
  blackbox->removeWindowSearch(frame.label);

  XDestroyWindow(blackbox->getXDisplay(), frame.label);
  XDestroyWindow(blackbox->getXDisplay(), frame.title);
  frame.title = frame.label = None;
}


void BlackboxWindow::createCloseButton(void) {
  if (frame.title != None) {
    frame.close_button = createChildWindow(frame.title);
    blackbox->saveWindowSearch(frame.close_button, this);
  }
}


void BlackboxWindow::destroyCloseButton(void) {
  blackbox->removeWindowSearch(frame.close_button);
  XDestroyWindow(blackbox->getXDisplay(), frame.close_button);
  frame.close_button = None;
}


void BlackboxWindow::createIconifyButton(void) {
  if (frame.title != None) {
    frame.iconify_button = createChildWindow(frame.title);
    blackbox->saveWindowSearch(frame.iconify_button, this);
  }
}


void BlackboxWindow::destroyIconifyButton(void) {
  blackbox->removeWindowSearch(frame.iconify_button);
  XDestroyWindow(blackbox->getXDisplay(), frame.iconify_button);
  frame.iconify_button = None;
}


void BlackboxWindow::createMaximizeButton(void) {
  if (frame.title != None) {
    frame.maximize_button = createChildWindow(frame.title);
    blackbox->saveWindowSearch(frame.maximize_button, this);
  }
}


void BlackboxWindow::destroyMaximizeButton(void) {
  blackbox->removeWindowSearch(frame.maximize_button);
  XDestroyWindow(blackbox->getXDisplay(), frame.maximize_button);
  frame.maximize_button = None;
}


void BlackboxWindow::positionButtons(bool redecorate_label) {
  string layout = blackbox->getTitlebarLayout();
  string parsed;

  bool hasclose, hasiconify, hasmaximize, haslabel;
  hasclose = hasiconify = hasmaximize = haslabel = false;

  string::const_iterator it, end;
  for (it = layout.begin(), end = layout.end(); it != end; ++it) {
    switch(*it) {
    case 'C':
      if (! hasclose && (decorations & Decor_Close)) {
        hasclose = true;
        parsed += *it;
      }
      break;
    case 'I':
      if (! hasiconify && (decorations & Decor_Iconify)) {
        hasiconify = true;
        parsed += *it;
      }
      break;
    case 'M':
      if (! hasmaximize && (decorations & Decor_Maximize)) {
        hasmaximize = true;
        parsed += *it;
      }
      break;
    case 'L':
      if (! haslabel) {
        haslabel = true;
        parsed += *it;
      }
    }
  }
  if (! hasclose && frame.close_button)
    destroyCloseButton();
  if (! hasiconify && frame.iconify_button)
    destroyIconifyButton();
  if (! hasmaximize && frame.maximize_button)
    destroyMaximizeButton();
  if (! haslabel)
    parsed += 'L';      // require that the label be in the layout

  const unsigned int bsep = frame.bevel_w + 1;  // separation between elements
  const unsigned int by = frame.bevel_w + 1;
  const unsigned int ty = frame.bevel_w;

  frame.label_w = frame.inside_w - bsep * 2 -
    (frame.button_w + bsep) * (parsed.size() - 1);

  unsigned int x = bsep;
  for (it = parsed.begin(), end = parsed.end(); it != end; ++it) {
    switch(*it) {
    case 'C':
      if (! frame.close_button) createCloseButton();
      XMoveResizeWindow(blackbox->getXDisplay(), frame.close_button, x, by,
                        frame.button_w, frame.button_w);
      x += frame.button_w + bsep;
      break;
    case 'I':
      if (! frame.iconify_button) createIconifyButton();
      XMoveResizeWindow(blackbox->getXDisplay(), frame.iconify_button, x, by,
                        frame.button_w, frame.button_w);
      x += frame.button_w + bsep;
      break;
    case 'M':
      if (! frame.maximize_button) createMaximizeButton();
      XMoveResizeWindow(blackbox->getXDisplay(), frame.maximize_button, x, by,
                        frame.button_w, frame.button_w);
      x += frame.button_w + bsep;
      break;
    case 'L':
      XMoveResizeWindow(blackbox->getXDisplay(), frame.label, x, ty,
                        frame.label_w, frame.label_h);
      x += frame.label_w + bsep;
      break;
    }
  }

  if (redecorate_label) decorateLabel();
  redrawLabel();
  redrawAllButtons();
}


void BlackboxWindow::reconfigure(void) {
  restoreGravity(client.rect);
  upsize();
  applyGravity(frame.rect);
  positionWindows();
  decorate();
  redrawWindowFrame();

  ungrabButtons();
  grabButtons();

  if (windowmenu) {
    windowmenu->move(windowmenu->getX(), frame.rect.y() + frame.title_h);
    windowmenu->reconfigure();
  }
}


void BlackboxWindow::grabButtons(void) {
  if (! screen->isSloppyFocus() || screen->doClickRaise())
    // grab button 1 for changing focus/raising
    blackbox->grabButton(Button1, 0, frame.plate, True, ButtonPressMask,
                         GrabModeSync, GrabModeSync, frame.plate, None,
                         screen->allowScrollLock());
  
  if (functions & Func_Move)
    blackbox->grabButton(Button1, ModMask, frame.window, True,
                         ButtonReleaseMask | ButtonMotionMask, GrabModeAsync,
                         GrabModeAsync, frame.window, None,
                         screen->allowScrollLock());
  if (functions & Func_Resize)
    blackbox->grabButton(Button3, ModMask, frame.window, True,
                         ButtonReleaseMask | ButtonMotionMask, GrabModeAsync,
                         GrabModeAsync, frame.window, None,
                         screen->allowScrollLock());
  // alt+middle lowers the window
  blackbox->grabButton(Button2, ModMask, frame.window, True,
                       ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
                       frame.window, None,
                       screen->allowScrollLock());
}


void BlackboxWindow::ungrabButtons(void) {
  blackbox->ungrabButton(Button1, 0, frame.plate);
  blackbox->ungrabButton(Button1, ModMask, frame.window);
  blackbox->ungrabButton(Button2, ModMask, frame.window);
  blackbox->ungrabButton(Button3, ModMask, frame.window);
}


void BlackboxWindow::positionWindows(void) {
  XMoveResizeWindow(blackbox->getXDisplay(), frame.window,
                    frame.rect.x(), frame.rect.y(), frame.inside_w,
                    (flags.shaded) ? frame.title_h : frame.inside_h);
  XSetWindowBorderWidth(blackbox->getXDisplay(), frame.window,
                        frame.border_w);
  XSetWindowBorderWidth(blackbox->getXDisplay(), frame.plate,
                        frame.mwm_border_w);
  XMoveResizeWindow(blackbox->getXDisplay(), frame.plate,
                    frame.margin.left - frame.mwm_border_w - frame.border_w,
                    frame.margin.top - frame.mwm_border_w - frame.border_w,
                    client.rect.width(), client.rect.height());
  XMoveResizeWindow(blackbox->getXDisplay(), client.window,
                    0, 0, client.rect.width(), client.rect.height());
  // ensure client.rect contains the real location
  client.rect.setPos(frame.rect.left() + frame.margin.left,
                     frame.rect.top() + frame.margin.top);

  if (decorations & Decor_Titlebar) {
    if (frame.title == None) createTitlebar();

    XSetWindowBorderWidth(blackbox->getXDisplay(), frame.title,
                          frame.border_w);
    XMoveResizeWindow(blackbox->getXDisplay(), frame.title, -frame.border_w,
                      -frame.border_w, frame.inside_w, frame.title_h);

    positionButtons();
    XMapSubwindows(blackbox->getXDisplay(), frame.title);
    XMapWindow(blackbox->getXDisplay(), frame.title);
  } else if (frame.title) {
    destroyTitlebar();
  }
  if (decorations & Decor_Handle) {
    if (frame.handle == None) createHandle();
    XSetWindowBorderWidth(blackbox->getXDisplay(), frame.handle,
                          frame.border_w);
    XSetWindowBorderWidth(blackbox->getXDisplay(), frame.left_grip,
                          frame.border_w);
    XSetWindowBorderWidth(blackbox->getXDisplay(), frame.right_grip,
                          frame.border_w);

    // use client.rect here so the value is correct even if shaded
    XMoveResizeWindow(blackbox->getXDisplay(), frame.handle,
                      -frame.border_w,
                      client.rect.height() + frame.margin.top +
                      frame.mwm_border_w - frame.border_w,
                      frame.inside_w, frame.handle_h);
    XMoveResizeWindow(blackbox->getXDisplay(), frame.left_grip,
                      -frame.border_w, -frame.border_w,
                      frame.grip_w, frame.handle_h);
    XMoveResizeWindow(blackbox->getXDisplay(), frame.right_grip,
                      frame.inside_w - frame.grip_w - frame.border_w,
                      -frame.border_w, frame.grip_w, frame.handle_h);

    XMapSubwindows(blackbox->getXDisplay(), frame.handle);
    XMapWindow(blackbox->getXDisplay(), frame.handle);
  } else if (frame.handle) {
    destroyHandle();
  }
  XSync(blackbox->getXDisplay(), False);
}


void BlackboxWindow::updateStrut(void) {
  unsigned long num = 4;
  unsigned long *data;
  if (! xatom->getValue(client.window, XAtom::net_wm_strut, XAtom::cardinal,
                        num, &data))
    return;
 
  if (num == 4) {
    client.strut.left = data[0];
    client.strut.right = data[1];
    client.strut.top = data[2];
    client.strut.bottom = data[3];

    screen->updateAvailableArea();
  }

  delete [] data;
}


void BlackboxWindow::getWindowType(void) {
  unsigned long val;
  if (xatom->getValue(client.window, XAtom::net_wm_window_type, XAtom::atom,
                      val)) {
    if (val == xatom->getAtom(XAtom::net_wm_window_type_desktop))
      window_type = Type_Desktop;
    else if (val == xatom->getAtom(XAtom::net_wm_window_type_dock))
      window_type = Type_Dock;
    else if (val == xatom->getAtom(XAtom::net_wm_window_type_toolbar))
      window_type = Type_Toolbar;
    else if (val == xatom->getAtom(XAtom::net_wm_window_type_menu))
      window_type = Type_Menu;
    else if (val == xatom->getAtom(XAtom::net_wm_window_type_utility))
      window_type = Type_Utility;
    else if (val == xatom->getAtom(XAtom::net_wm_window_type_splash))
      window_type = Type_Splash;
    else if (val == xatom->getAtom(XAtom::net_wm_window_type_dialog))
      window_type = Type_Dialog;
    else //if (val[0] == xatom->getAtom(XAtom::net_wm_window_type_normal))
      window_type = Type_Normal;
    return;
  }

  /*
   * the window type hint was not set, which means we either classify ourself
   * as a normal window or a dialog, depending on if we are a transient.
   */
  if (isTransient())
    window_type = Type_Dialog;

  window_type = Type_Normal;
}


void BlackboxWindow::getWMName(void) {
  if (xatom->getValue(client.window, XAtom::net_wm_name,
                      XAtom::utf8, client.title) &&
      !client.title.empty()) {
    xatom->eraseValue(client.window, XAtom::net_wm_visible_name);
    return;
  }
  //fall through to using WM_NAME
  if (xatom->getValue(client.window, XAtom::wm_name, XAtom::ansi, client.title)
      && !client.title.empty()) {
    xatom->eraseValue(client.window, XAtom::net_wm_visible_name);
    return;
  }
  // fall back to an internal default
  client.title = i18n(WindowSet, WindowUnnamed, "Unnamed");
  xatom->setValue(client.window, XAtom::net_wm_visible_name, XAtom::utf8,
                  client.title);
}


void BlackboxWindow::getWMIconName(void) {
  if (xatom->getValue(client.window, XAtom::net_wm_icon_name,
                      XAtom::utf8, client.icon_title) && 
      !client.icon_title.empty()) {
    xatom->eraseValue(client.window, XAtom::net_wm_visible_icon_name);
    return;
  }
  //fall through to using WM_ICON_NAME
  if (xatom->getValue(client.window, XAtom::wm_icon_name, XAtom::ansi,
                      client.icon_title) && 
      !client.icon_title.empty()) {
    xatom->eraseValue(client.window, XAtom::net_wm_visible_icon_name);
    return;
  }
  // fall back to using the main name
  client.icon_title = client.title;
  xatom->setValue(client.window, XAtom::net_wm_visible_icon_name, XAtom::utf8,
                  client.icon_title);
}


/*
 * Retrieve which WM Protocols are supported by the client window.
 * If the WM_DELETE_WINDOW protocol is supported, add the close button to the
 * window's decorations and allow the close behavior.
 * If the WM_TAKE_FOCUS protocol is supported, save a value that indicates
 * this.
 */
void BlackboxWindow::getWMProtocols(void) {
  Atom *proto;
  int num_return = 0;

  if (XGetWMProtocols(blackbox->getXDisplay(), client.window,
                      &proto, &num_return)) {
    for (int i = 0; i < num_return; ++i) {
      if (proto[i] == xatom->getAtom(XAtom::wm_delete_window)) {
        decorations |= Decor_Close;
        functions |= Func_Close;
      } else if (proto[i] == xatom->getAtom(XAtom::wm_take_focus))
        flags.send_focus_message = True;
      else if (proto[i] == xatom->getAtom(XAtom::blackbox_structure_messages))
        screen->addNetizen(new Netizen(screen, client.window));
    }

    XFree(proto);
  }
}


/*
 * Gets the value of the WM_HINTS property.
 * If the property is not set, then use a set of default values.
 */
void BlackboxWindow::getWMHints(void) {
  focus_mode = F_Passive;

  // remove from current window group
  if (client.window_group) {
    BWindowGroup *group = blackbox->searchGroup(client.window_group);
    if (group) group->removeWindow(this);
  }
  client.window_group = None;

  XWMHints *wmhint = XGetWMHints(blackbox->getXDisplay(), client.window);
  if (! wmhint) {
    return;
  }

  if (wmhint->flags & InputHint) {
    if (wmhint->input == True) {
      if (flags.send_focus_message)
        focus_mode = F_LocallyActive;
    } else {
      if (flags.send_focus_message)
        focus_mode = F_GloballyActive;
      else
        focus_mode = F_NoInput;
    }
  }

  if (wmhint->flags & StateHint)
    current_state = wmhint->initial_state;

  if (wmhint->flags & WindowGroupHint) {
    client.window_group = wmhint->window_group;

    // add window to the appropriate group
    BWindowGroup *group = blackbox->searchGroup(client.window_group);
    if (! group) { // no group found, create it!
      new BWindowGroup(blackbox, client.window_group);
      group = blackbox->searchGroup(client.window_group);
    }
    if (group)
      group->addWindow(this);
  }

  XFree(wmhint);
}


/*
 * Gets the value of the WM_NORMAL_HINTS property.
 * If the property is not set, then use a set of default values.
 */
void BlackboxWindow::getWMNormalHints(void) {
  long icccm_mask;
  XSizeHints sizehint;

  client.min_width = client.min_height =
    client.width_inc = client.height_inc = 1;
  client.base_width = client.base_height = 0;
  client.win_gravity = NorthWestGravity;
#if 0
  client.min_aspect_x = client.min_aspect_y =
    client.max_aspect_x = client.max_aspect_y = 1;
#endif

  /*
    use the full screen, not the strut modified size. otherwise when the
    availableArea changes max_width/height will be incorrect and lead to odd
    rendering bugs.
  */
  const Rect& screen_area = screen->getRect();
  client.max_width = screen_area.width();
  client.max_height = screen_area.height();

  if (! XGetWMNormalHints(blackbox->getXDisplay(), client.window,
                          &sizehint, &icccm_mask))
    return;

  client.normal_hint_flags = sizehint.flags;

  if (sizehint.flags & PMinSize) {
    if (sizehint.min_width >= 0)
      client.min_width = sizehint.min_width;
    if (sizehint.min_height >= 0)
      client.min_height = sizehint.min_height;
  }

  if (sizehint.flags & PMaxSize) {
    if (sizehint.max_width > static_cast<signed>(client.min_width))
      client.max_width = sizehint.max_width;
    else
      client.max_width = client.min_width;

    if (sizehint.max_height > static_cast<signed>(client.min_height))
      client.max_height = sizehint.max_height;
    else
      client.max_height = client.min_height;
  }

  if (sizehint.flags & PResizeInc) {
    client.width_inc = sizehint.width_inc;
    client.height_inc = sizehint.height_inc;
  }

#if 0 // we do not support this at the moment
  if (sizehint.flags & PAspect) {
    client.min_aspect_x = sizehint.min_aspect.x;
    client.min_aspect_y = sizehint.min_aspect.y;
    client.max_aspect_x = sizehint.max_aspect.x;
    client.max_aspect_y = sizehint.max_aspect.y;
  }
#endif

  if (sizehint.flags & PBaseSize) {
    client.base_width = sizehint.base_width;
    client.base_height = sizehint.base_height;
  }

  if (sizehint.flags & PWinGravity)
    client.win_gravity = sizehint.win_gravity;
}


/*
 * Gets the NETWM hints for the class' contained window.
 */
void BlackboxWindow::getNetWMHints(void) {
  unsigned long workspace;

  if (xatom->getValue(client.window, XAtom::net_wm_desktop, XAtom::cardinal,
                      workspace)) {
    if (workspace == 0xffffffff)
      flags.stuck = True;
    else
      blackbox_attrib.workspace = workspace;
  }

  unsigned long *state;
  unsigned long num = (unsigned) -1;
  if (xatom->getValue(client.window, XAtom::net_wm_state, XAtom::atom,
                      num, &state)) {
    bool vert = False,
         horz = False;
    for (unsigned long i = 0; i < num; ++i) {
      if (state[i] == xatom->getAtom(XAtom::net_wm_state_modal))
        flags.modal = True;
      else if (state[i] == xatom->getAtom(XAtom::net_wm_state_shaded))
        flags.shaded = True;
      else if (state[i] == xatom->getAtom(XAtom::net_wm_state_skip_taskbar))
        flags.skip_taskbar = True;
      else if (state[i] == xatom->getAtom(XAtom::net_wm_state_skip_pager))
        flags.skip_pager = True;
      else if (state[i] == xatom->getAtom(XAtom::net_wm_state_fullscreen))
        flags.fullscreen = True;
      else if (state[i] == xatom->getAtom(XAtom::net_wm_state_hidden))
        setState(IconicState);
      else if (state[i] == xatom->getAtom(XAtom::net_wm_state_maximized_vert))
        vert = True;
      else if (state[i] == xatom->getAtom(XAtom::net_wm_state_maximized_horz))
        horz = True;
    }
    if (vert && horz)
      flags.maximized = 1;
    else if (vert)
      flags.maximized = 2;
    else if (horz)
      flags.maximized = 3;

    delete [] state;
  }
}


/*
 * Gets the MWM hints for the class' contained window.
 * This is used while initializing the window to its first state, and not
 * thereafter.
 * Returns: true if the MWM hints are successfully retreived and applied;
 * false if they are not.
 */
void BlackboxWindow::getMWMHints(void) {
  unsigned long num;
  MwmHints *mwm_hint;

  num = PropMwmHintsElements;
  if (! xatom->getValue(client.window, XAtom::motif_wm_hints,
                        XAtom::motif_wm_hints, num,
                        (unsigned long **)&mwm_hint))
    return;
  if (num < PropMwmHintsElements) {
    delete [] mwm_hint;
    return;
  }

  if (mwm_hint->flags & MwmHintsDecorations) {
    if (mwm_hint->decorations & MwmDecorAll) {
      decorations = Decor_Titlebar | Decor_Handle | Decor_Border |
                    Decor_Iconify | Decor_Maximize | Decor_Close;
    } else {
      decorations = 0;

      if (mwm_hint->decorations & MwmDecorBorder)
        decorations |= Decor_Border;
      if (mwm_hint->decorations & MwmDecorHandle)
        decorations |= Decor_Handle;
      if (mwm_hint->decorations & MwmDecorTitle)
        decorations |= Decor_Titlebar;
      if (mwm_hint->decorations & MwmDecorIconify)
        decorations |= Decor_Iconify;
      if (mwm_hint->decorations & MwmDecorMaximize)
        decorations |= Decor_Maximize;
    }
  }

  if (mwm_hint->flags & MwmHintsFunctions) {
    if (mwm_hint->functions & MwmFuncAll) {
      functions = Func_Resize | Func_Move | Func_Iconify | Func_Maximize |
                  Func_Close;
    } else {
      functions = 0;

      if (mwm_hint->functions & MwmFuncResize)
        functions |= Func_Resize;
      if (mwm_hint->functions & MwmFuncMove)
        functions |= Func_Move;
      if (mwm_hint->functions & MwmFuncIconify)
        functions |= Func_Iconify;
      if (mwm_hint->functions & MwmFuncMaximize)
        functions |= Func_Maximize;
      if (mwm_hint->functions & MwmFuncClose)
        functions |= Func_Close;
    }
  }
  delete [] mwm_hint;
}


/*
 * Gets the blackbox hints from the class' contained window.
 * This is used while initializing the window to its first state, and not
 * thereafter.
 * Returns: true if the hints are successfully retreived and applied; false if
 * they are not.
 */
bool BlackboxWindow::getBlackboxHints(void) {
  unsigned long num;
  BlackboxHints *blackbox_hint;

  num = PropBlackboxHintsElements;
  if (! xatom->getValue(client.window, XAtom::blackbox_hints,
                        XAtom::blackbox_hints, num,
                        (unsigned long **)&blackbox_hint))
    return False;
  if (num < PropBlackboxHintsElements) {
    delete [] blackbox_hint;
    return False;
  }

  if (blackbox_hint->flags & AttribShaded)
    flags.shaded = (blackbox_hint->attrib & AttribShaded);

  if ((blackbox_hint->flags & AttribMaxHoriz) &&
      (blackbox_hint->flags & AttribMaxVert))
    flags.maximized = (blackbox_hint->attrib &
                       (AttribMaxHoriz | AttribMaxVert)) ? 1 : 0;
  else if (blackbox_hint->flags & AttribMaxVert)
    flags.maximized = (blackbox_hint->attrib & AttribMaxVert) ? 2 : 0;
  else if (blackbox_hint->flags & AttribMaxHoriz)
    flags.maximized = (blackbox_hint->attrib & AttribMaxHoriz) ? 3 : 0;

  if (blackbox_hint->flags & AttribOmnipresent)
    flags.stuck = (blackbox_hint->attrib & AttribOmnipresent);

  if (blackbox_hint->flags & AttribWorkspace)
    blackbox_attrib.workspace = blackbox_hint->workspace;

  // if (blackbox_hint->flags & AttribStack)
  //   don't yet have always on top/bottom for blackbox yet... working
  //   on that

  if (blackbox_hint->flags & AttribDecoration) {
    switch (blackbox_hint->decoration) {
    case DecorNone:
      decorations = 0;
      break;

    case DecorTiny:
      decorations |= Decor_Titlebar | Decor_Iconify;
      decorations &= ~(Decor_Border | Decor_Handle | Decor_Maximize);
      functions &= ~(Func_Resize | Func_Maximize);

      break;

    case DecorTool:
      decorations |= Decor_Titlebar;
      decorations &= ~(Decor_Iconify | Decor_Border | Decor_Handle);
      functions &= ~(Func_Resize | Func_Maximize | Func_Iconify);

      break;

    case DecorNormal:
    default:
      decorations |= Decor_Titlebar | Decor_Border | Decor_Handle |
                     Decor_Iconify | Decor_Maximize;
      break;
    }

    reconfigure();
  }
  
  delete [] blackbox_hint;

  return True;
}


void BlackboxWindow::getTransientInfo(void) {
  if (client.transient_for &&
      client.transient_for != (BlackboxWindow *) ~0ul) {
    // reset transient_for in preparation of looking for a new owner
    client.transient_for->client.transientList.remove(this);
  }

  // we have no transient_for until we find a new one
  client.transient_for = 0;

  Window trans_for;
  if (! XGetTransientForHint(blackbox->getXDisplay(), client.window,
                             &trans_for)) {
    // transient_for hint not set
    return;
  }

  if (trans_for == client.window) {
    // wierd client... treat this window as a normal window
    return;
  }

  if (trans_for == None || trans_for == screen->getRootWindow()) {
    // this is an undocumented interpretation of the ICCCM. a transient
    // associated with None/Root/itself is assumed to be a modal root
    // transient.  we don't support the concept of a global transient,
    // so we just associate this transient with nothing, and perhaps
    // we will add support later for global modality.
    client.transient_for = (BlackboxWindow *) ~0ul;
    flags.modal = True;
    return;
  }

  client.transient_for = blackbox->searchWindow(trans_for);
  if (! client.transient_for &&
      client.window_group && trans_for == client.window_group) {
    // no direct transient_for, perhaps this is a group transient?
    BWindowGroup *group = blackbox->searchGroup(client.window_group);
    if (group) client.transient_for = group->find(screen);
  }

  if (! client.transient_for || client.transient_for == this) {
    // no transient_for found, or we have a wierd client that wants to be
    // a transient for itself, so we treat this window as a normal window
    client.transient_for = (BlackboxWindow*) 0;
    return;
  }

  // register ourselves with our new transient_for
  client.transient_for->client.transientList.push_back(this);
  flags.stuck = client.transient_for->flags.stuck;
}


BlackboxWindow *BlackboxWindow::getTransientFor(void) const {
  if (client.transient_for &&
      client.transient_for != (BlackboxWindow*) ~0ul)
    return client.transient_for;
  return 0;
}


/*
 * This function is responsible for updating both the client and the frame
 * rectangles.
 * According to the ICCCM a client message is not sent for a resize, only a
 * move.
 */
void BlackboxWindow::configure(int dx, int dy,
                               unsigned int dw, unsigned int dh) {
  bool send_event = ((frame.rect.x() != dx || frame.rect.y() != dy) &&
                     ! flags.moving);

  if (dw != frame.rect.width() || dh != frame.rect.height()) {
    frame.rect.setRect(dx, dy, dw, dh);
    frame.inside_w = frame.rect.width() - (frame.border_w * 2);
    frame.inside_h = frame.rect.height() - (frame.border_w * 2);

    if (frame.rect.right() <= 0 || frame.rect.bottom() <= 0)
      frame.rect.setPos(0, 0);

    client.rect.setCoords(frame.rect.left() + frame.margin.left,
                          frame.rect.top() + frame.margin.top,
                          frame.rect.right() - frame.margin.right,
                          frame.rect.bottom() - frame.margin.bottom);

#ifdef    SHAPE
    if (blackbox->hasShapeExtensions() && flags.shaped) {
      configureShape();
    }
#endif // SHAPE

    positionWindows();
    decorate();
    redrawWindowFrame();
  } else {
    frame.rect.setPos(dx, dy);

    XMoveWindow(blackbox->getXDisplay(), frame.window,
                frame.rect.x(), frame.rect.y());
    /*
      we may have been called just after an opaque window move, so even though
      the old coords match the new ones no ConfigureNotify has been sent yet.
      There are likely other times when this will be relevant as well.
    */
    if (! flags.moving) send_event = True;
  }

  if (send_event) {
    // if moving, the update and event will occur when the move finishes
    client.rect.setPos(frame.rect.left() + frame.margin.left,
                       frame.rect.top() + frame.margin.top);

    XEvent event;
    event.type = ConfigureNotify;

    event.xconfigure.display = blackbox->getXDisplay();
    event.xconfigure.event = client.window;
    event.xconfigure.window = client.window;
    event.xconfigure.x = client.rect.x();
    event.xconfigure.y = client.rect.y();
    event.xconfigure.width = client.rect.width();
    event.xconfigure.height = client.rect.height();
    event.xconfigure.border_width = client.old_bw;
    event.xconfigure.above = frame.window;
    event.xconfigure.override_redirect = False;

    XSendEvent(blackbox->getXDisplay(), client.window, False,
               StructureNotifyMask, &event);
    screen->updateNetizenConfigNotify(&event);
    XFlush(blackbox->getXDisplay());
  }
}


#ifdef SHAPE
void BlackboxWindow::configureShape(void) {
  XShapeCombineShape(blackbox->getXDisplay(), frame.window, ShapeBounding,
                     frame.margin.left - frame.border_w,
                     frame.margin.top - frame.border_w,
                     client.window, ShapeBounding, ShapeSet);

  int num = 0;
  XRectangle xrect[2];

  if (decorations & Decor_Titlebar) {
    xrect[0].x = xrect[0].y = -frame.border_w;
    xrect[0].width = frame.rect.width();
    xrect[0].height = frame.title_h + (frame.border_w * 2);
    ++num;
  }

  if (decorations & Decor_Handle) {
    xrect[1].x = -frame.border_w;
    xrect[1].y = frame.rect.height() - frame.margin.bottom +
                 frame.mwm_border_w - frame.border_w;
    xrect[1].width = frame.rect.width();
    xrect[1].height = frame.handle_h + (frame.border_w * 2);
    ++num;
  }

  XShapeCombineRectangles(blackbox->getXDisplay(), frame.window,
                          ShapeBounding, 0, 0, xrect, num,
                          ShapeUnion, Unsorted);
}
#endif // SHAPE


bool BlackboxWindow::setInputFocus(void) {
  if (flags.focused) return True;

  assert(flags.stuck ||  // window must be on the current workspace or sticky
         blackbox_attrib.workspace == screen->getCurrentWorkspaceID());

  /*
     We only do this check for normal windows and dialogs because other windows
     do this on purpose, such as kde's kicker, and we don't want to go moving
     it.
  */
  if (window_type == Type_Normal || window_type == Type_Dialog)
    if (! frame.rect.intersects(screen->getRect())) {
      // client is outside the screen, move it to the center
      configure((screen->getWidth() - frame.rect.width()) / 2,
                (screen->getHeight() - frame.rect.height()) / 2,
                frame.rect.width(), frame.rect.height());
    }

  if (client.transientList.size() > 0) {
    // transfer focus to any modal transients
    BlackboxWindowList::iterator it, end = client.transientList.end();
    for (it = client.transientList.begin(); it != end; ++it) {
      if ((*it)->flags.modal) return (*it)->setInputFocus();
    }
  }

  bool ret = True;
  if (focus_mode == F_LocallyActive || focus_mode == F_Passive) {
    XSetInputFocus(blackbox->getXDisplay(), client.window,
                   RevertToPointerRoot, CurrentTime);
  } else {
    /* we could set the focus to none, since the window doesn't accept focus,
     * but we shouldn't set focus to nothing since this would surely make
     * someone angry
     */
    ret = False;
  }

  if (flags.send_focus_message) {
    XEvent ce;
    ce.xclient.type = ClientMessage;
    ce.xclient.message_type = xatom->getAtom(XAtom::wm_protocols);
    ce.xclient.display = blackbox->getXDisplay();
    ce.xclient.window = client.window;
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = xatom->getAtom(XAtom::wm_take_focus);
    ce.xclient.data.l[1] = blackbox->getLastTime();
    ce.xclient.data.l[2] = 0l;
    ce.xclient.data.l[3] = 0l;
    ce.xclient.data.l[4] = 0l;
    XSendEvent(blackbox->getXDisplay(), client.window, False,
               NoEventMask, &ce);
    XFlush(blackbox->getXDisplay());
  }

  return ret;
}


void BlackboxWindow::iconify(void) {
  if (flags.iconic) return;

  // We don't need to worry about resizing because resizing always grabs the X
  // server. This should only ever happen if using opaque moving.
  if (flags.moving)
    endMove();
    
  if (windowmenu) windowmenu->hide();

  /*
   * we don't want this XUnmapWindow call to generate an UnmapNotify event, so
   * we need to clear the event mask on client.window for a split second.
   * HOWEVER, since X11 is asynchronous, the window could be destroyed in that
   * split second, leaving us with a ghost window... so, we need to do this
   * while the X server is grabbed
   */
  unsigned long event_mask = PropertyChangeMask | FocusChangeMask |
                             StructureNotifyMask;
  XGrabServer(blackbox->getXDisplay());
  XSelectInput(blackbox->getXDisplay(), client.window,
               event_mask & ~StructureNotifyMask);
  XUnmapWindow(blackbox->getXDisplay(), client.window);
  XSelectInput(blackbox->getXDisplay(), client.window, event_mask);
  XUngrabServer(blackbox->getXDisplay());

  XUnmapWindow(blackbox->getXDisplay(), frame.window);
  flags.visible = False;
  flags.iconic = True;

  setState(IconicState);

  screen->getWorkspace(blackbox_attrib.workspace)->removeWindow(this);
  if (flags.stuck) {
    for (unsigned int i = 0; i < screen->getNumberOfWorkspaces(); ++i)
      if (i != blackbox_attrib.workspace)
        screen->getWorkspace(i)->removeWindow(this, True);
  }

  if (isTransient()) {
    if (client.transient_for != (BlackboxWindow *) ~0ul &&
        ! client.transient_for->flags.iconic) {
      // iconify our transient_for
      client.transient_for->iconify();
    }
  }

  screen->addIcon(this);

  if (client.transientList.size() > 0) {
    // iconify all transients
    BlackboxWindowList::iterator it, end = client.transientList.end();
    for (it = client.transientList.begin(); it != end; ++it) {
      if (! (*it)->flags.iconic) (*it)->iconify();
    }
  }
  screen->updateStackingList();
}


void BlackboxWindow::show(void) {
  flags.visible = True;
  flags.iconic = False;

  current_state = (flags.shaded) ? IconicState : NormalState;
  setState(current_state);

  XMapWindow(blackbox->getXDisplay(), client.window);
  XMapSubwindows(blackbox->getXDisplay(), frame.window);
  XMapWindow(blackbox->getXDisplay(), frame.window);

#if 0
  int real_x, real_y;
  Window child;
  XTranslateCoordinates(blackbox->getXDisplay(), client.window,
                        screen->getRootWindow(),
                        0, 0, &real_x, &real_y, &child);
  fprintf(stderr, "%s -- assumed: (%d, %d), real: (%d, %d)\n", getTitle(),
          client.rect.left(), client.rect.top(), real_x, real_y);
  assert(client.rect.left() == real_x && client.rect.top() == real_y);
#endif
}


void BlackboxWindow::deiconify(bool reassoc, bool raise) {
  if (flags.iconic || reassoc)
    screen->reassociateWindow(this, BSENTINEL, False);
  else if (blackbox_attrib.workspace != screen->getCurrentWorkspaceID())
    return;

  show();

  // reassociate and deiconify all transients
  if (reassoc && client.transientList.size() > 0) {
    BlackboxWindowList::iterator it, end = client.transientList.end();
    for (it = client.transientList.begin(); it != end; ++it) {
      (*it)->deiconify(True, False);
    }
  }

  if (raise)
    screen->getWorkspace(blackbox_attrib.workspace)->raiseWindow(this);
}


void BlackboxWindow::close(void) {
  XEvent ce;
  ce.xclient.type = ClientMessage;
  ce.xclient.message_type =  xatom->getAtom(XAtom::wm_protocols);
  ce.xclient.display = blackbox->getXDisplay();
  ce.xclient.window = client.window;
  ce.xclient.format = 32;
  ce.xclient.data.l[0] = xatom->getAtom(XAtom::wm_delete_window);
  ce.xclient.data.l[1] = CurrentTime;
  ce.xclient.data.l[2] = 0l;
  ce.xclient.data.l[3] = 0l;
  ce.xclient.data.l[4] = 0l;
  XSendEvent(blackbox->getXDisplay(), client.window, False, NoEventMask, &ce);
  XFlush(blackbox->getXDisplay());
}


void BlackboxWindow::withdraw(void) {
  // We don't need to worry about resizing because resizing always grabs the X
  // server. This should only ever happen if using opaque moving.
  if (flags.moving)
    endMove();
    
  flags.visible = False;
  flags.iconic = False;

  setState(current_state);

  XUnmapWindow(blackbox->getXDisplay(), frame.window);

  XGrabServer(blackbox->getXDisplay());

  unsigned long event_mask = PropertyChangeMask | FocusChangeMask |
                             StructureNotifyMask;
  XSelectInput(blackbox->getXDisplay(), client.window,
               event_mask & ~StructureNotifyMask);
  XUnmapWindow(blackbox->getXDisplay(), client.window);
  XSelectInput(blackbox->getXDisplay(), client.window, event_mask);

  XUngrabServer(blackbox->getXDisplay());

  if (windowmenu) windowmenu->hide();
}


void BlackboxWindow::maximize(unsigned int button) {
  // We don't need to worry about resizing because resizing always grabs the X
  // server. This should only ever happen if using opaque moving.
  if (flags.moving)
    endMove();

  // handle case where menu is open then the max button is used instead
  if (windowmenu && windowmenu->isVisible()) windowmenu->hide();

  if (flags.maximized) {
    flags.maximized = 0;

    blackbox_attrib.flags &= ! (AttribMaxHoriz | AttribMaxVert);
    blackbox_attrib.attrib &= ! (AttribMaxHoriz | AttribMaxVert);

    /*
      when a resize finishes, maximize(0) is called to clear any maximization
      flags currently set.  Otherwise it still thinks it is maximized.
      so we do not need to call configure() because resizing will handle it
    */
    if (! flags.resizing)
      configure(blackbox_attrib.premax_x, blackbox_attrib.premax_y,
                blackbox_attrib.premax_w, blackbox_attrib.premax_h);

    blackbox_attrib.premax_x = blackbox_attrib.premax_y = 0;
    blackbox_attrib.premax_w = blackbox_attrib.premax_h = 0;

    redrawAllButtons(); // in case it is not called in configure()
    setState(current_state);
    return;
  }

  blackbox_attrib.premax_x = frame.rect.x();
  blackbox_attrib.premax_y = frame.rect.y();
  blackbox_attrib.premax_w = frame.rect.width();
  // use client.rect so that clients can be restored even if shaded
  blackbox_attrib.premax_h =
    client.rect.height() + frame.margin.top + frame.margin.bottom;

#ifdef    XINERAMA
  if (screen->isXineramaActive() && blackbox->doXineramaMaximizing()) {
    // find the area to use
    RectList availableAreas = screen->allAvailableAreas();
    RectList::iterator it, end = availableAreas.end();

    for (it = availableAreas.begin(); it != end; ++it)
      if (it->intersects(frame.rect)) break;
    if (it == end) // the window isn't inside an area
      it = availableAreas.begin(); // so just default to the first one

    frame.changing = *it;
  } else
#endif // XINERAMA
  frame.changing = screen->availableArea();

  switch(button) {
  case 1:
    blackbox_attrib.flags |= AttribMaxHoriz | AttribMaxVert;
    blackbox_attrib.attrib |= AttribMaxHoriz | AttribMaxVert;
    break;

  case 2:
    blackbox_attrib.flags |= AttribMaxVert;
    blackbox_attrib.attrib |= AttribMaxVert;

    frame.changing.setX(frame.rect.x());
    frame.changing.setWidth(frame.rect.width());
    break;

  case 3:
    blackbox_attrib.flags |= AttribMaxHoriz;
    blackbox_attrib.attrib |= AttribMaxHoriz;

    frame.changing.setY(frame.rect.y());
    frame.changing.setHeight(frame.rect.height());
    break;
  }

  constrain(TopLeft);

  if (flags.shaded) {
    blackbox_attrib.flags ^= AttribShaded;
    blackbox_attrib.attrib ^= AttribShaded;
    flags.shaded = False;
  }

  flags.maximized = button;

  configure(frame.changing.x(), frame.changing.y(),
            frame.changing.width(), frame.changing.height());
  if (flags.focused)
    screen->getWorkspace(blackbox_attrib.workspace)->raiseWindow(this);
  redrawAllButtons(); // in case it is not called in configure()
  setState(current_state);
}


// re-maximizes the window to take into account availableArea changes
void BlackboxWindow::remaximize(void) {
  if (flags.shaded) {
    // we only update the window's attributes otherwise we lose the shade bit
    switch(flags.maximized) {
    case 1:
      blackbox_attrib.flags |= AttribMaxHoriz | AttribMaxVert;
      blackbox_attrib.attrib |= AttribMaxHoriz | AttribMaxVert;
      break;

    case 2:
      blackbox_attrib.flags |= AttribMaxVert;
      blackbox_attrib.attrib |= AttribMaxVert;
      break;

    case 3:
      blackbox_attrib.flags |= AttribMaxHoriz;
      blackbox_attrib.attrib |= AttribMaxHoriz;
      break;
    }
    return;
  }

  // save the original dimensions because maximize will wipe them out
  int premax_x = blackbox_attrib.premax_x,
    premax_y = blackbox_attrib.premax_y,
    premax_w = blackbox_attrib.premax_w,
    premax_h = blackbox_attrib.premax_h;

  unsigned int button = flags.maximized;
  flags.maximized = 0; // trick maximize() into working
  maximize(button);

  // restore saved values
  blackbox_attrib.premax_x = premax_x;
  blackbox_attrib.premax_y = premax_y;
  blackbox_attrib.premax_w = premax_w;
  blackbox_attrib.premax_h = premax_h;
}


void BlackboxWindow::setWorkspace(unsigned int n) {
  blackbox_attrib.flags |= AttribWorkspace;
  blackbox_attrib.workspace = n;
  if (n == BSENTINEL) { // iconified window
    /*
       we set the workspace to 'all workspaces' so that taskbars will show the
       window. otherwise, it made uniconifying a window imposible without the
       blackbox workspace menu
    */
    n = 0xffffffff;
  }
  xatom->setValue(client.window, XAtom::net_wm_desktop, XAtom::cardinal, n);
}


void BlackboxWindow::shade(void) {
  if (flags.shaded) {
    XResizeWindow(blackbox->getXDisplay(), frame.window,
                  frame.inside_w, frame.inside_h);
    flags.shaded = False;
    blackbox_attrib.flags ^= AttribShaded;
    blackbox_attrib.attrib ^= AttribShaded;

    setState(NormalState);

    // set the frame rect to the normal size
    frame.rect.setHeight(client.rect.height() + frame.margin.top +
                         frame.margin.bottom);
  } else {
    if (! (decorations & Decor_Titlebar))
      return; // can't shade it without a titlebar!

    XResizeWindow(blackbox->getXDisplay(), frame.window,
                  frame.inside_w, frame.title_h);
    flags.shaded = True;
    blackbox_attrib.flags |= AttribShaded;
    blackbox_attrib.attrib |= AttribShaded;

    setState(IconicState);

    // set the frame rect to the shaded size
    frame.rect.setHeight(frame.title_h + (frame.border_w * 2));
  }
}


/*
 * (Un)Sticks a window and its relatives.
 */
void BlackboxWindow::stick(void) {
  if (flags.stuck) {
    blackbox_attrib.flags ^= AttribOmnipresent;
    blackbox_attrib.attrib ^= AttribOmnipresent;

    flags.stuck = False;
    
    for (unsigned int i = 0; i < screen->getNumberOfWorkspaces(); ++i)
      if (i != blackbox_attrib.workspace)
        screen->getWorkspace(i)->removeWindow(this, True);

    if (! flags.iconic)
      screen->reassociateWindow(this, BSENTINEL, True);
    // temporary fix since sticky windows suck. set the hint to what we
    // actually hold in our data.
    xatom->setValue(client.window, XAtom::net_wm_desktop, XAtom::cardinal,
                    blackbox_attrib.workspace);

    setState(current_state);
  } else {
    flags.stuck = True;

    blackbox_attrib.flags |= AttribOmnipresent;
    blackbox_attrib.attrib |= AttribOmnipresent;

    // temporary fix since sticky windows suck. set the hint to a different
    // value than that contained in the class' data.
    xatom->setValue(client.window, XAtom::net_wm_desktop, XAtom::cardinal,
                    0xffffffff);
    
    for (unsigned int i = 0; i < screen->getNumberOfWorkspaces(); ++i)
      if (i != blackbox_attrib.workspace)
        screen->getWorkspace(i)->addWindow(this, False, True);

    setState(current_state);
  }
  // go up the chain
  if (isTransient() && client.transient_for != (BlackboxWindow *) ~0ul &&
      client.transient_for->isStuck() != flags.stuck)
    client.transient_for->stick();
  // go down the chain
  BlackboxWindowList::iterator it;
  const BlackboxWindowList::iterator end = client.transientList.end();
  for (it = client.transientList.begin(); it != end; ++it)
    if ((*it)->isStuck() != flags.stuck)
      (*it)->stick();
}


void BlackboxWindow::redrawWindowFrame(void) const {
  if (decorations & Decor_Titlebar) {
    if (flags.focused) {
      if (frame.ftitle)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.title, frame.ftitle);
      else
        XSetWindowBackground(blackbox->getXDisplay(),
                             frame.title, frame.ftitle_pixel);
    } else {
      if (frame.utitle)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.title, frame.utitle);
      else
        XSetWindowBackground(blackbox->getXDisplay(),
                             frame.title, frame.utitle_pixel);
    }
    XClearWindow(blackbox->getXDisplay(), frame.title);

    redrawLabel();
    redrawAllButtons();
  }

  if (decorations & Decor_Handle) {
    if (flags.focused) {
      if (frame.fhandle)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.handle, frame.fhandle);
      else
        XSetWindowBackground(blackbox->getXDisplay(),
                             frame.handle, frame.fhandle_pixel);

      if (frame.fgrip) {
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.left_grip, frame.fgrip);
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.right_grip, frame.fgrip);
      } else {
        XSetWindowBackground(blackbox->getXDisplay(),
                             frame.left_grip, frame.fgrip_pixel);
        XSetWindowBackground(blackbox->getXDisplay(),
                             frame.right_grip, frame.fgrip_pixel);
      }
    } else {
      if (frame.uhandle)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.handle, frame.uhandle);
      else
        XSetWindowBackground(blackbox->getXDisplay(),
                             frame.handle, frame.uhandle_pixel);

      if (frame.ugrip) {
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.left_grip, frame.ugrip);
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.right_grip, frame.ugrip);
      } else {
        XSetWindowBackground(blackbox->getXDisplay(),
                             frame.left_grip, frame.ugrip_pixel);
        XSetWindowBackground(blackbox->getXDisplay(),
                             frame.right_grip, frame.ugrip_pixel);
      }
    }
    XClearWindow(blackbox->getXDisplay(), frame.handle);
    XClearWindow(blackbox->getXDisplay(), frame.left_grip);
    XClearWindow(blackbox->getXDisplay(), frame.right_grip);
  }

  if (decorations & Decor_Border) {
    if (flags.focused)
      XSetWindowBorder(blackbox->getXDisplay(),
                       frame.plate, frame.fborder_pixel);
    else
      XSetWindowBorder(blackbox->getXDisplay(),
                       frame.plate, frame.uborder_pixel);
  }
}


void BlackboxWindow::setFocusFlag(bool focus) {
  // only focus a window if it is visible
  if (focus && !flags.visible)
    return;

  flags.focused = focus;

  redrawWindowFrame();

  if (screen->isSloppyFocus() && screen->doAutoRaise()) {
    if (isFocused()) timer->start();
    else timer->stop();
  }

  if (flags.focused)
    blackbox->setFocusedWindow(this);
 
  if (! flags.iconic) {
    // iconic windows arent in a workspace menu!
    if (flags.stuck)
      screen->getCurrentWorkspace()->setFocused(this, isFocused());
    else
      screen->getWorkspace(blackbox_attrib.workspace)->
        setFocused(this, flags.focused);
  }
}


void BlackboxWindow::installColormap(bool install) {
  int i = 0, ncmap = 0;
  Colormap *cmaps = XListInstalledColormaps(blackbox->getXDisplay(),
                                            client.window, &ncmap);
  if (cmaps) {
    XWindowAttributes wattrib;
    if (XGetWindowAttributes(blackbox->getXDisplay(),
                             client.window, &wattrib)) {
      if (install) {
        // install the window's colormap
        for (i = 0; i < ncmap; i++) {
          if (*(cmaps + i) == wattrib.colormap)
            // this window is using an installed color map... do not install
            install = False;
        }
        // otherwise, install the window's colormap
        if (install)
          XInstallColormap(blackbox->getXDisplay(), wattrib.colormap);
      } else {
        // uninstall the window's colormap
        for (i = 0; i < ncmap; i++) {
          if (*(cmaps + i) == wattrib.colormap)
            // we found the colormap to uninstall
            XUninstallColormap(blackbox->getXDisplay(), wattrib.colormap);
        }
      }
    }

    XFree(cmaps);
  }
}


void BlackboxWindow::setAllowedActions(void) {
  Atom actions[7];
  int num = 0;
  
  actions[num++] = xatom->getAtom(XAtom::net_wm_action_shade);
  actions[num++] = xatom->getAtom(XAtom::net_wm_action_change_desktop);
  actions[num++] = xatom->getAtom(XAtom::net_wm_action_close);

  if (functions & Func_Move)
    actions[num++] = xatom->getAtom(XAtom::net_wm_action_move);
  if (functions & Func_Resize)
    actions[num++] = xatom->getAtom(XAtom::net_wm_action_resize);
  if (functions & Func_Maximize) {
    actions[num++] = xatom->getAtom(XAtom::net_wm_action_maximize_horz);
    actions[num++] = xatom->getAtom(XAtom::net_wm_action_maximize_vert);
  }

  xatom->setValue(client.window, XAtom::net_wm_allowed_actions, XAtom::atom,
                  actions, num);
}


void BlackboxWindow::setState(unsigned long new_state) {
  current_state = new_state;

  unsigned long state[2];
  state[0] = current_state;
  state[1] = None;
  xatom->setValue(client.window, XAtom::wm_state, XAtom::wm_state, state, 2);
 
  xatom->setValue(client.window, XAtom::blackbox_attributes,
                  XAtom::blackbox_attributes, (unsigned long *)&blackbox_attrib,
                  PropBlackboxAttributesElements);

  Atom netstate[8];
  int num = 0;
  if (flags.modal)
    netstate[num++] = xatom->getAtom(XAtom::net_wm_state_modal);
  if (flags.shaded)
    netstate[num++] = xatom->getAtom(XAtom::net_wm_state_shaded);
  if (flags.iconic)
    netstate[num++] = xatom->getAtom(XAtom::net_wm_state_hidden);
  if (flags.skip_taskbar)
    netstate[num++] = xatom->getAtom(XAtom::net_wm_state_skip_taskbar);
  if (flags.skip_pager)
    netstate[num++] = xatom->getAtom(XAtom::net_wm_state_skip_pager);
  if (flags.fullscreen)
    netstate[num++] = xatom->getAtom(XAtom::net_wm_state_fullscreen);
  if (flags.maximized == 1 || flags.maximized == 2)
    netstate[num++] = xatom->getAtom(XAtom::net_wm_state_maximized_vert);
  if (flags.maximized == 1 || flags.maximized == 3)
    netstate[num++] = xatom->getAtom(XAtom::net_wm_state_maximized_horz);
  xatom->setValue(client.window, XAtom::net_wm_state, XAtom::atom,
                  netstate, num);
}


bool BlackboxWindow::getState(void) {
  bool ret = xatom->getValue(client.window, XAtom::wm_state, XAtom::wm_state,
                             current_state);
  if (! ret) current_state = 0;
  return ret;
}


void BlackboxWindow::restoreAttributes(void) {
  unsigned long num = PropBlackboxAttributesElements;
  BlackboxAttributes *net;
  if (! xatom->getValue(client.window, XAtom::blackbox_attributes,
                        XAtom::blackbox_attributes, num,
                        (unsigned long **)&net))
    return;
  if (num < PropBlackboxAttributesElements) {
    delete [] net;
    return;
  }

  if (net->flags & AttribShaded && net->attrib & AttribShaded) {
    flags.shaded = False;
    unsigned long orig_state = current_state;
    shade();

    /*
      At this point in the life of a window, current_state should only be set
      to IconicState if the window was an *icon*, not if it was shaded.
    */
    if (orig_state != IconicState)
      current_state = WithdrawnState;
 }

  if (net->workspace != screen->getCurrentWorkspaceID() &&
      net->workspace < screen->getWorkspaceCount())
    screen->reassociateWindow(this, net->workspace, True);

  if ((blackbox_attrib.workspace != screen->getCurrentWorkspaceID()) &&
      (blackbox_attrib.workspace < screen->getWorkspaceCount())) {
    // set to WithdrawnState so it will be mapped on the new workspace
    if (current_state == NormalState) current_state = WithdrawnState;
  } else if (current_state == WithdrawnState) {
    // the window is on this workspace and is Withdrawn, so it is waiting to
    // be mapped
    current_state = NormalState;
  }

  if (net->flags & AttribOmnipresent && net->attrib & AttribOmnipresent &&
      ! flags.stuck) {
    stick();

    // if the window was on another workspace, it was going to be hidden. this
    // specifies that the window should be mapped since it is sticky.
    if (current_state == WithdrawnState) current_state = NormalState;
  }

  if (net->flags & AttribMaxHoriz || net->flags & AttribMaxVert) {
    int x = net->premax_x, y = net->premax_y;
    unsigned int w = net->premax_w, h = net->premax_h;
    flags.maximized = 0;

    unsigned int m = 0;
    if ((net->flags & AttribMaxHoriz) &&
        (net->flags & AttribMaxVert))
      m = (net->attrib & (AttribMaxHoriz | AttribMaxVert)) ? 1 : 0;
    else if (net->flags & AttribMaxVert)
      m = (net->attrib & AttribMaxVert) ? 2 : 0;
    else if (net->flags & AttribMaxHoriz)
      m = (net->attrib & AttribMaxHoriz) ? 3 : 0;

    if (m) maximize(m);

    blackbox_attrib.premax_x = x;
    blackbox_attrib.premax_y = y;
    blackbox_attrib.premax_w = w;
    blackbox_attrib.premax_h = h;
  }

  if (net->flags & AttribDecoration) {
    switch (net->decoration) {
    case DecorNone:
      decorations = 0;

      break;

    default:
    case DecorNormal:
      decorations |= Decor_Titlebar | Decor_Handle | Decor_Border |
        Decor_Iconify | Decor_Maximize;

      break;

    case DecorTiny:
      decorations |= Decor_Titlebar | Decor_Iconify;
      decorations &= ~(Decor_Border | Decor_Handle | Decor_Maximize);

      break;

    case DecorTool:
      decorations |= Decor_Titlebar;
      decorations &= ~(Decor_Iconify | Decor_Border | Decor_Handle);

      break;
    }

    // sanity check the new decor
    if (! (functions & Func_Resize) || isTransient())
      decorations &= ~(Decor_Maximize | Decor_Handle);
    if (! (functions & Func_Maximize))
      decorations &= ~Decor_Maximize;

    if (decorations & Decor_Titlebar) {
      if (functions & Func_Close)   // close button is controlled by function
        decorations |= Decor_Close; // not decor type
    } else { 
      if (flags.shaded) // we can not be shaded if we lack a titlebar
        shade();
    }

    if (flags.visible && frame.window) {
      XMapSubwindows(blackbox->getXDisplay(), frame.window);
      XMapWindow(blackbox->getXDisplay(), frame.window);
    }

    reconfigure();
    setState(current_state);
  }

  // with the state set it will then be the map event's job to read the
  // window's state and behave accordingly

  delete [] net;
}


/*
 * Positions the Rect r according the the client window position and
 * window gravity.
 */
void BlackboxWindow::applyGravity(Rect &r) {
  // apply horizontal window gravity
  switch (client.win_gravity) {
  default:
  case NorthWestGravity:
  case SouthWestGravity:
  case WestGravity:
    r.setX(client.rect.x());
    break;

  case NorthGravity:
  case SouthGravity:
  case CenterGravity:
    r.setX(client.rect.x() - (frame.margin.left + frame.margin.right) / 2);
    break;

  case NorthEastGravity:
  case SouthEastGravity:
  case EastGravity:
    r.setX(client.rect.x() - frame.margin.left - frame.margin.right + 2);
    break;

  case ForgetGravity:
  case StaticGravity:
    r.setX(client.rect.x() - frame.margin.left);
    break;
  }

  // apply vertical window gravity
  switch (client.win_gravity) {
  default:
  case NorthWestGravity:
  case NorthEastGravity:
  case NorthGravity:
    r.setY(client.rect.y());
    break;

  case CenterGravity:
  case EastGravity:
  case WestGravity:
    r.setY(client.rect.y() - (frame.margin.top + frame.margin.bottom) / 2);
    break;

  case SouthWestGravity:
  case SouthEastGravity:
  case SouthGravity:
    r.setY(client.rect.y() - frame.margin.top - frame.margin.bottom + 2);
    break;

  case ForgetGravity:
  case StaticGravity:
    r.setY(client.rect.y() - frame.margin.top);
    break;
  }
}


/*
 * The reverse of the applyGravity function.
 *
 * Positions the Rect r according to the frame window position and
 * window gravity.
 */
void BlackboxWindow::restoreGravity(Rect &r) {
  // restore horizontal window gravity
  switch (client.win_gravity) {
  default:
  case NorthWestGravity:
  case SouthWestGravity:
  case WestGravity:
    r.setX(frame.rect.x());
    break;

  case NorthGravity:
  case SouthGravity:
  case CenterGravity:
    r.setX(frame.rect.x() + (frame.margin.left + frame.margin.right) / 2);
    break;

  case NorthEastGravity:
  case SouthEastGravity:
  case EastGravity:
    r.setX(frame.rect.x() + frame.margin.left + frame.margin.right - 2);
    break;

  case ForgetGravity:
  case StaticGravity:
    r.setX(frame.rect.x() + frame.margin.left);
    break;
  }

  // restore vertical window gravity
  switch (client.win_gravity) {
  default:
  case NorthWestGravity:
  case NorthEastGravity:
  case NorthGravity:
    r.setY(frame.rect.y());
    break;

  case CenterGravity:
  case EastGravity:
  case WestGravity:
    r.setY(frame.rect.y() + (frame.margin.top + frame.margin.bottom) / 2);
    break;

  case SouthWestGravity:
  case SouthEastGravity:
  case SouthGravity:
    r.setY(frame.rect.y() + frame.margin.top + frame.margin.bottom - 2);
    break;

  case ForgetGravity:
  case StaticGravity:
    r.setY(frame.rect.y() + frame.margin.top);
    break;
  }
}


void BlackboxWindow::redrawLabel(void) const {
  if (flags.focused) {
    if (frame.flabel)
      XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                 frame.label, frame.flabel);
    else
      XSetWindowBackground(blackbox->getXDisplay(),
                           frame.label, frame.flabel_pixel);
  } else {
    if (frame.ulabel)
      XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                 frame.label, frame.ulabel);
    else
      XSetWindowBackground(blackbox->getXDisplay(),
                           frame.label, frame.ulabel_pixel);
  }
  XClearWindow(blackbox->getXDisplay(), frame.label);

  WindowStyle *style = screen->getWindowStyle();

  int pos = frame.bevel_w * 2;
  style->doJustify(client.title.c_str(), pos, frame.label_w, frame.bevel_w * 4);
  style->font->drawString(frame.label, pos, 1,
                          (flags.focused ? style->l_text_focus :
                           style->l_text_unfocus),
                          client.title);
}


void BlackboxWindow::redrawAllButtons(void) const {
  if (frame.iconify_button) redrawIconifyButton(False);
  if (frame.maximize_button) redrawMaximizeButton(flags.maximized);
  if (frame.close_button) redrawCloseButton(False);
}


void BlackboxWindow::redrawIconifyButton(bool pressed) const {
  if (! pressed) {
    if (flags.focused) {
      if (frame.fbutton)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.iconify_button, frame.fbutton);
      else
        XSetWindowBackground(blackbox->getXDisplay(),
                             frame.iconify_button, frame.fbutton_pixel);
    } else {
      if (frame.ubutton)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.iconify_button, frame.ubutton);
      else
        XSetWindowBackground(blackbox->getXDisplay(), frame.iconify_button,
                             frame.ubutton_pixel);
    }
  } else {
    if (frame.pbutton)
      XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                 frame.iconify_button, frame.pbutton);
    else
      XSetWindowBackground(blackbox->getXDisplay(),
                           frame.iconify_button, frame.pbutton_pixel);
  }
  XClearWindow(blackbox->getXDisplay(), frame.iconify_button);

  BPen pen((flags.focused) ? screen->getWindowStyle()->b_pic_focus :
           screen->getWindowStyle()->b_pic_unfocus);
  XDrawRectangle(blackbox->getXDisplay(), frame.iconify_button, pen.gc(),
                 2, (frame.button_w - 5), (frame.button_w - 5), 2);
}


void BlackboxWindow::redrawMaximizeButton(bool pressed) const {
  if (! pressed) {
    if (flags.focused) {
      if (frame.fbutton)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.maximize_button, frame.fbutton);
      else
        XSetWindowBackground(blackbox->getXDisplay(), frame.maximize_button,
                             frame.fbutton_pixel);
    } else {
      if (frame.ubutton)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                   frame.maximize_button, frame.ubutton);
      else
        XSetWindowBackground(blackbox->getXDisplay(), frame.maximize_button,
                             frame.ubutton_pixel);
    }
  } else {
    if (frame.pbutton)
      XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                 frame.maximize_button, frame.pbutton);
    else
      XSetWindowBackground(blackbox->getXDisplay(), frame.maximize_button,
                           frame.pbutton_pixel);
  }
  XClearWindow(blackbox->getXDisplay(), frame.maximize_button);

  BPen pen((flags.focused) ? screen->getWindowStyle()->b_pic_focus :
           screen->getWindowStyle()->b_pic_unfocus);
  XDrawRectangle(blackbox->getXDisplay(), frame.maximize_button, pen.gc(),
                 2, 2, (frame.button_w - 5), (frame.button_w - 5));
  XDrawLine(blackbox->getXDisplay(), frame.maximize_button, pen.gc(),
            2, 3, (frame.button_w - 3), 3);
}


void BlackboxWindow::redrawCloseButton(bool pressed) const {
  if (! pressed) {
    if (flags.focused) {
      if (frame.fbutton)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(), frame.close_button,
                                   frame.fbutton);
      else
        XSetWindowBackground(blackbox->getXDisplay(), frame.close_button,
                             frame.fbutton_pixel);
    } else {
      if (frame.ubutton)
        XSetWindowBackgroundPixmap(blackbox->getXDisplay(), frame.close_button,
                                   frame.ubutton);
      else
        XSetWindowBackground(blackbox->getXDisplay(), frame.close_button,
                             frame.ubutton_pixel);
    }
  } else {
    if (frame.pbutton)
      XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                                 frame.close_button, frame.pbutton);
    else
      XSetWindowBackground(blackbox->getXDisplay(),
                           frame.close_button, frame.pbutton_pixel);
  }
  XClearWindow(blackbox->getXDisplay(), frame.close_button);

  BPen pen((flags.focused) ? screen->getWindowStyle()->b_pic_focus :
           screen->getWindowStyle()->b_pic_unfocus);
  XDrawLine(blackbox->getXDisplay(), frame.close_button, pen.gc(),
            2, 2, (frame.button_w - 3), (frame.button_w - 3));
  XDrawLine(blackbox->getXDisplay(), frame.close_button, pen.gc(),
            2, (frame.button_w - 3), (frame.button_w - 3), 2);
}


void BlackboxWindow::mapRequestEvent(const XMapRequestEvent *re) {
  if (re->window != client.window)
    return;

#ifdef    DEBUG
  fprintf(stderr, "BlackboxWindow::mapRequestEvent() for 0x%lx\n",
          client.window);
#endif // DEBUG

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
    show();
    screen->getWorkspace(blackbox_attrib.workspace)->raiseWindow(this);
    if (isNormal()) {
      if (! blackbox->isStartup()) {
        XSync(blackbox->getXDisplay(), False); // make sure the frame is mapped
        if (screen->doFocusNew()|| (isTransient() && getTransientFor() &&
                                    getTransientFor()->isFocused())) {
          setInputFocus();
        }
        if (screen->getPlacementPolicy() == BScreen::ClickMousePlacement) {
          int x, y, rx, ry;
          Window c, r;
          unsigned int m;
          XQueryPointer(blackbox->getXDisplay(), screen->getRootWindow(),
                        &r, &c, &rx, &ry, &x, &y, &m);
          beginMove(rx, ry);
        }
      }
    }
    break;
  }
}


void BlackboxWindow::unmapNotifyEvent(const XUnmapEvent *ue) {
  if (ue->window != client.window)
    return;

#ifdef    DEBUG
  fprintf(stderr, "BlackboxWindow::unmapNotifyEvent() for 0x%lx\n",
          client.window);
#endif // DEBUG

  screen->unmanageWindow(this, False);
}


void BlackboxWindow::destroyNotifyEvent(const XDestroyWindowEvent *de) {
  if (de->window != client.window)
    return;

#ifdef    DEBUG
  fprintf(stderr, "BlackboxWindow::destroyNotifyEvent() for 0x%lx\n",
          client.window);
#endif // DEBUG

  screen->unmanageWindow(this, False);
}


void BlackboxWindow::reparentNotifyEvent(const XReparentEvent *re) {
  if (re->window != client.window || re->parent == frame.plate)
    return;

#ifdef    DEBUG
  fprintf(stderr, "BlackboxWindow::reparentNotifyEvent(): reparent 0x%lx to "
          "0x%lx.\n", client.window, re->parent);
#endif // DEBUG

  XEvent ev;
  ev.xreparent = *re;
  XPutBackEvent(blackbox->getXDisplay(), &ev);
  screen->unmanageWindow(this, True);
}


void BlackboxWindow::propertyNotifyEvent(const XPropertyEvent *pe) {
  if (pe->state == PropertyDelete)
    return;

#ifdef    DEBUG
  fprintf(stderr, "BlackboxWindow::propertyNotifyEvent(): for 0x%lx\n",
          client.window);
#endif

  switch(pe->atom) {
  case XA_WM_CLASS:
  case XA_WM_CLIENT_MACHINE:
  case XA_WM_COMMAND:
    break;

  case XA_WM_TRANSIENT_FOR: {
    // determine if this is a transient window
    getTransientInfo();

    // adjust the window decorations based on transience
    if (isTransient()) {
      decorations &= ~(Decor_Maximize | Decor_Handle);
      functions &= ~Func_Maximize;
      setAllowedActions();
    }

    reconfigure();
  }
    break;

  case XA_WM_HINTS:
    getWMHints();
    break;

  case XA_WM_ICON_NAME:
    getWMIconName();
    if (flags.iconic) screen->propagateWindowName(this);
    break;

  case XAtom::net_wm_name:
  case XA_WM_NAME:
    getWMName();

    if (decorations & Decor_Titlebar)
      redrawLabel();

    screen->propagateWindowName(this);
    break;

  case XA_WM_NORMAL_HINTS: {
    getWMNormalHints();

    if ((client.normal_hint_flags & PMinSize) &&
        (client.normal_hint_flags & PMaxSize)) {
      // the window now can/can't resize itself, so the buttons need to be
      // regrabbed.
      ungrabButtons();
      if (client.max_width <= client.min_width &&
          client.max_height <= client.min_height) {
        decorations &= ~(Decor_Maximize | Decor_Handle);
        functions &= ~(Func_Resize | Func_Maximize);
      } else {
        if (! isTransient()) {
          decorations |= Decor_Maximize | Decor_Handle;
          functions |= Func_Maximize;
        }
        functions |= Func_Resize;
      }
      grabButtons();
      setAllowedActions();
    }

    Rect old_rect = frame.rect;

    upsize();

    if (old_rect != frame.rect)
      reconfigure();

    break;
  }

  default:
    if (pe->atom == xatom->getAtom(XAtom::wm_protocols)) {
      getWMProtocols();

      if ((decorations & Decor_Close) && (! frame.close_button)) {
        createCloseButton();
        if (decorations & Decor_Titlebar) {
          positionButtons(True);
          XMapSubwindows(blackbox->getXDisplay(), frame.title);
        }
        if (windowmenu) windowmenu->reconfigure();
      }
    } else if (pe->atom == xatom->getAtom(XAtom::net_wm_strut)) {
      updateStrut();
    }

    break;
  }
}


void BlackboxWindow::exposeEvent(const XExposeEvent *ee) {
#ifdef DEBUG
  fprintf(stderr, "BlackboxWindow::exposeEvent() for 0x%lx\n", client.window);
#endif

  if (frame.label == ee->window && (decorations & Decor_Titlebar))
    redrawLabel();
  else if (frame.close_button == ee->window)
    redrawCloseButton(False);
  else if (frame.maximize_button == ee->window)
    redrawMaximizeButton(flags.maximized);
  else if (frame.iconify_button == ee->window)
    redrawIconifyButton(False);
}


void BlackboxWindow::configureRequestEvent(const XConfigureRequestEvent *cr) {
  if (cr->window != client.window || flags.iconic)
    return;

  if (cr->value_mask & CWBorderWidth)
    client.old_bw = cr->border_width;

  if (cr->value_mask & (CWX | CWY | CWWidth | CWHeight)) {
    Rect req = frame.rect;

    if (cr->value_mask & (CWX | CWY)) {
      if (cr->value_mask & CWX)
        client.rect.setX(cr->x);
      if (cr->value_mask & CWY)
        client.rect.setY(cr->y);

      applyGravity(req);
    }

    if (cr->value_mask & CWWidth)
      req.setWidth(cr->width + frame.margin.left + frame.margin.right);

    if (cr->value_mask & CWHeight)
      req.setHeight(cr->height + frame.margin.top + frame.margin.bottom);

    configure(req.x(), req.y(), req.width(), req.height());
  }

  if (cr->value_mask & CWStackMode && !isDesktop()) {
    switch (cr->detail) {
    case Below:
    case BottomIf:
      screen->getWorkspace(blackbox_attrib.workspace)->lowerWindow(this);
      break;

    case Above:
    case TopIf:
    default:
      screen->getWorkspace(blackbox_attrib.workspace)->raiseWindow(this);
      break;
    }
  }
}


void BlackboxWindow::buttonPressEvent(const XButtonEvent *be) {
#ifdef DEBUG
  fprintf(stderr, "BlackboxWindow::buttonPressEvent() for 0x%lx\n",
          client.window);
#endif

  if (frame.maximize_button == be->window && be->button <= 3) {
    redrawMaximizeButton(True);
  } else if (be->button == 1 || (be->button == 3 && be->state == ModMask)) {
    if (! flags.focused)
      setInputFocus();

    if (frame.iconify_button == be->window) {
      redrawIconifyButton(True);
    } else if (frame.close_button == be->window) {
      redrawCloseButton(True);
    } else if (frame.plate == be->window) {
      if (windowmenu && windowmenu->isVisible()) windowmenu->hide();

      screen->getWorkspace(blackbox_attrib.workspace)->raiseWindow(this);

      XAllowEvents(blackbox->getXDisplay(), ReplayPointer, be->time);
    } else {
      if (frame.title == be->window || frame.label == be->window) {
        if (((be->time - lastButtonPressTime) <=
             blackbox->getDoubleClickInterval()) ||
            (be->state == ControlMask)) {
          lastButtonPressTime = 0;
          shade();
        } else {
          lastButtonPressTime = be->time;
        }
      }

      if (windowmenu && windowmenu->isVisible()) windowmenu->hide();

      screen->getWorkspace(blackbox_attrib.workspace)->raiseWindow(this);
    }
  } else if (be->button == 2 && (be->window != frame.iconify_button) &&
             (be->window != frame.close_button)) {
    screen->getWorkspace(blackbox_attrib.workspace)->lowerWindow(this);
  } else if (windowmenu && be->button == 3 &&
             (frame.title == be->window || frame.label == be->window ||
              frame.handle == be->window || frame.window == be->window)) {
    if (windowmenu->isVisible()) {
      windowmenu->hide();
    } else {
      int mx = be->x_root - windowmenu->getWidth() / 2,
          my = be->y_root - windowmenu->getHeight() / 2;

      // snap the window menu into a corner/side if necessary
      int left_edge, right_edge, top_edge, bottom_edge;

      /*
         the " + (frame.border_w * 2) - 1" bits are to get the proper width
         and height of the menu, as the sizes returned by it do not include
         the borders.
       */
      left_edge = frame.rect.x();
      right_edge = frame.rect.right() -
        (windowmenu->getWidth() + (frame.border_w * 2) - 1);
      top_edge = client.rect.top() - (frame.border_w + frame.mwm_border_w);
      bottom_edge = client.rect.bottom() -
        (windowmenu->getHeight() + (frame.border_w * 2) - 1) +
        (frame.border_w + frame.mwm_border_w);

      if (mx < left_edge)
        mx = left_edge;
      if (mx > right_edge)
        mx = right_edge;
      if (my < top_edge)
        my = top_edge;
      if (my > bottom_edge)
        my = bottom_edge;

      windowmenu->move(mx, my);
      windowmenu->show();
      XRaiseWindow(blackbox->getXDisplay(), windowmenu->getWindowID());
      XRaiseWindow(blackbox->getXDisplay(),
                   windowmenu->getSendToMenu()->getWindowID());
    }
  // mouse wheel up
  } else if (be->button == 4) {
    if ((be->window == frame.label ||
         be->window == frame.title ||
         be->window == frame.maximize_button ||
         be->window == frame.iconify_button ||
         be->window == frame.close_button) &&
        ! flags.shaded)
      shade();
  // mouse wheel down
  } else if (be->button == 5) {
    if ((be->window == frame.label ||
         be->window == frame.title ||
         be->window == frame.maximize_button ||
         be->window == frame.iconify_button ||
         be->window == frame.close_button) &&
        flags.shaded)
      shade();
  }
}


void BlackboxWindow::buttonReleaseEvent(const XButtonEvent *re) {
#ifdef DEBUG
  fprintf(stderr, "BlackboxWindow::buttonReleaseEvent() for 0x%lx\n",
          client.window);
#endif

  if (re->window == frame.maximize_button &&
      re->button >= 1 && re->button <= 3) {
    if ((re->x >= 0 && re->x <= static_cast<signed>(frame.button_w)) &&
        (re->y >= 0 && re->y <= static_cast<signed>(frame.button_w))) {
      maximize(re->button);
    } else {
      redrawMaximizeButton(flags.maximized);
    }
  } else if (re->window == frame.iconify_button && re->button == 1) {
    if ((re->x >= 0 && re->x <= static_cast<signed>(frame.button_w)) &&
        (re->y >= 0 && re->y <= static_cast<signed>(frame.button_w))) {
      iconify();
    } else {
      redrawIconifyButton(False);
    }
  } else if (re->window == frame.close_button & re->button == 1) {
    if ((re->x >= 0 && re->x <= static_cast<signed>(frame.button_w)) &&
        (re->y >= 0 && re->y <= static_cast<signed>(frame.button_w)))
      close();
    redrawCloseButton(False);
  } else if (flags.moving) {
    endMove();
  } else if (flags.resizing) {
    endResize();
  } else if (re->window == frame.window) {
    if (re->button == 2 && re->state == ModMask)
      XUngrabPointer(blackbox->getXDisplay(), CurrentTime);
  }
}



void BlackboxWindow::beginMove(int x_root, int y_root) {
  assert(! (flags.resizing || flags.moving));

  /*
    Only one window can be moved/resized at a time. If another window is already
    being moved or resized, then stop it before whating to work with this one.
  */
  BlackboxWindow *changing = blackbox->getChangingWindow();
  if (changing && changing != this) {
    if (changing->flags.moving)
      changing->endMove();
    else // if (changing->flags.resizing)
      changing->endResize();
  }
  
  XGrabPointer(blackbox->getXDisplay(), frame.window, False,
               PointerMotionMask | ButtonReleaseMask,
               GrabModeAsync, GrabModeAsync,
               None, blackbox->getMoveCursor(), CurrentTime);

  if (windowmenu && windowmenu->isVisible())
    windowmenu->hide();

  flags.moving = True;
  blackbox->setChangingWindow(this);

  if (! screen->doOpaqueMove()) {
    XGrabServer(blackbox->getXDisplay());

    frame.changing = frame.rect;
    screen->showPosition(frame.changing.x(), frame.changing.y());

    XDrawRectangle(blackbox->getXDisplay(), screen->getRootWindow(),
                   screen->getOpGC(),
                   frame.changing.x(),
                   frame.changing.y(),
                   frame.changing.width() - 1,
                   frame.changing.height() - 1);
  }

  frame.grab_x = x_root - frame.rect.x() - frame.border_w;
  frame.grab_y = y_root - frame.rect.y() - frame.border_w;
}


void BlackboxWindow::doMove(int x_root, int y_root) {
  assert(flags.moving);
  assert(blackbox->getChangingWindow() == this);

  int dx = x_root - frame.grab_x, dy = y_root - frame.grab_y;
  dx -= frame.border_w;
  dy -= frame.border_w;

  if (screen->doWorkspaceWarping())
    if (doWorkspaceWarping(x_root, y_root, dx, dy))
      return;

  doWindowSnapping(dx, dy);

  if (screen->doOpaqueMove()) {
    configure(dx, dy, frame.rect.width(), frame.rect.height());
  } else {
    XDrawRectangle(blackbox->getXDisplay(), screen->getRootWindow(),
                   screen->getOpGC(),
                   frame.changing.x(),
                   frame.changing.y(),
                   frame.changing.width() - 1,
                   frame.changing.height() - 1);

    frame.changing.setPos(dx, dy);

    XDrawRectangle(blackbox->getXDisplay(), screen->getRootWindow(),
                   screen->getOpGC(),
                   frame.changing.x(),
                   frame.changing.y(),
                   frame.changing.width() - 1,
                   frame.changing.height() - 1);
  }

  screen->showPosition(dx, dy);
}


bool BlackboxWindow::doWorkspaceWarping(int x_root, int y_root,
                                        int dx, int dy) {
  // workspace warping
  bool warp = False;
  unsigned int dest = screen->getCurrentWorkspaceID();
  if (x_root <= 0) {
    warp = True;

    if (dest > 0) dest--;
    else dest = screen->getNumberOfWorkspaces() - 1;

  } else if (x_root >= screen->getRect().right()) {
    warp = True;

    if (dest < screen->getNumberOfWorkspaces() - 1) dest++;
    else dest = 0;
  }
  if (! warp)
    return false;

  endMove();
  bool focus = flags.focused; // had focus while moving?
  if (! flags.stuck)
    screen->reassociateWindow(this, dest, False);
  screen->changeWorkspaceID(dest);
  if (focus)
    setInputFocus();

  /*
     If the XWarpPointer is done after the configure, we can end up
     grabbing another window, so made sure you do it first.
     */
  int dest_x;
  if (x_root <= 0) {
    dest_x = screen->getRect().right() - 1;
    XWarpPointer(blackbox->getXDisplay(), None, 
                 screen->getRootWindow(), 0, 0, 0, 0,
                 dest_x, y_root);

    configure(dx + (screen->getRect().width() - 1), dy,
              frame.rect.width(), frame.rect.height());
  } else {
    dest_x = 0;
    XWarpPointer(blackbox->getXDisplay(), None, 
                 screen->getRootWindow(), 0, 0, 0, 0,
                 dest_x, y_root);

    configure(dx - (screen->getRect().width() - 1), dy,
              frame.rect.width(), frame.rect.height());
  }

  beginMove(dest_x, y_root);
  return true;
}


void BlackboxWindow::doWindowSnapping(int &dx, int &dy) {
  // how much resistance to edges to provide
  const int resistance_size = screen->getResistanceSize();

  // how far away to snap
  const int snap_distance = screen->getSnapThreshold();

  // how to snap windows
  const int snap_to_windows = screen->getWindowToWindowSnap();
  const int snap_to_edges = screen->getWindowToEdgeSnap();
  // the amount of space away from the edge to provide resistance/snap
//  const int snap_offset = screen->getSnapThreshold();

  // find the geomeetery where the moving window currently is
  const Rect &moving = screen->doOpaqueMove() ? frame.rect : frame.changing;

  // window corners
  const int wleft = dx,
           wright = dx + frame.rect.width() - 1,
             wtop = dy,
          wbottom = dy + frame.rect.height() - 1;

  if (snap_to_windows) {
    RectList rectlist;

    Workspace *w = screen->getWorkspace(getWorkspaceNumber());
    assert(w);

    // add windows on the workspace to the rect list
    const BlackboxWindowList& stack_list = w->getStackingList();
    BlackboxWindowList::const_iterator st_it, st_end = stack_list.end();
    for (st_it = stack_list.begin(); st_it != st_end; ++st_it)
      rectlist.push_back( (*st_it)->frameRect() );

    // add the toolbar and the slit to the rect list.
    // (only if they are not hidden)
    Toolbar *tbar = screen->getToolbar();
    Slit *slit = screen->getSlit();
    Rect tbar_rect, slit_rect;
    unsigned int bwidth = screen->getBorderWidth() * 2;

    if (! (screen->doHideToolbar() || tbar->isHidden())) {
      tbar_rect.setRect(tbar->getX(), tbar->getY(), tbar->getWidth() + bwidth,
                        tbar->getHeight() + bwidth);
      rectlist.push_back(tbar_rect);
    }

    if (! slit->isHidden()) {
      slit_rect.setRect(slit->getX(), slit->getY(), slit->getWidth() + bwidth,
                        slit->getHeight() + bwidth);
      rectlist.push_back(slit_rect);
    }

    RectList::const_iterator it, end = rectlist.end();
    for (it = rectlist.begin(); it != end; ++it) {
      bool snapped = False;
      const Rect &winrect = *it;

      if (snap_to_windows == BScreen::WindowResistance)
        // if the window is already over top of this snap target, then
        // resistance is futile, so just ignore it
        if (winrect.intersects(moving))
          continue;

      int dleft, dright, dtop, dbottom;

      // if the windows are in the same plane vertically
      if (wtop >= (signed)(winrect.y() - frame.rect.height() + 1) &&
          wtop < (signed)(winrect.y() + winrect.height() - 1)) {

        if (snap_to_windows == BScreen::WindowResistance) {
          dleft = wright - winrect.left();
          dright = winrect.right() - wleft;

          // snap left of other window?
          if (dleft >= 0 && dleft < resistance_size) {
            dx = winrect.left() - frame.rect.width();
            snapped = True;
          }
          // snap right of other window?
          else if (dright >= 0 && dright < resistance_size) {
            dx = winrect.right() + 1;
            snapped = True;
          }
        } else { // BScreen::WindowSnap
          dleft = abs(wright - winrect.left());
          dright = abs(wleft - winrect.right());

          // snap left of other window?
          if (dleft < snap_distance && dleft <= dright) {
            dx = winrect.left() - frame.rect.width();
            snapped = True;
          }
          // snap right of other window?
          else if (dright < snap_distance) {
            dx = winrect.right() + 1;
            snapped = True;
          }            
        }

        if (snapped) {
          if (screen->getWindowCornerSnap()) {
            // try corner-snap to its other sides
            if (snap_to_windows == BScreen::WindowResistance) {
              dtop = winrect.top() - wtop;
              dbottom = wbottom - winrect.bottom();
              if (dtop > 0 && dtop < resistance_size) {
                // if we're already past the top edge, then don't provide
                // resistance
                if (moving.top() >= winrect.top())
                  dy = winrect.top();
              } else if (dbottom > 0 && dbottom < resistance_size) {
                // if we're already past the bottom edge, then don't provide
                // resistance
                if (moving.bottom() <= winrect.bottom())
                  dy = winrect.bottom() - frame.rect.height() + 1;
              }
            } else { // BScreen::WindowSnap
              dtop = abs(wtop - winrect.top());
              dbottom = abs(wbottom - winrect.bottom());
              if (dtop < snap_distance && dtop <= dbottom)
                dy = winrect.top();
              else if (dbottom < snap_distance)
                dy = winrect.bottom() - frame.rect.height() + 1;
            }
          }

          continue;
        }
      }

      // if the windows are on the same plane horizontally
      if (wleft >= (signed)(winrect.x() - frame.rect.width() + 1) &&
          wleft < (signed)(winrect.x() + winrect.width() - 1)) {

        if (snap_to_windows == BScreen::WindowResistance) {
          dtop = wbottom - winrect.top();
          dbottom = winrect.bottom() - wtop;

          // snap top of other window?
          if (dtop >= 0 && dtop < resistance_size) {
            dy = winrect.top() - frame.rect.height();
            snapped = True;
          }
          // snap bottom of other window?
          else if (dbottom >= 0 && dbottom < resistance_size) {
            dy = winrect.bottom() + 1;
            snapped = True;
          }
        } else { // BScreen::WindowSnap
          dtop = abs(wbottom - winrect.top());
          dbottom = abs(wtop - winrect.bottom());

          // snap top of other window?
          if (dtop < snap_distance && dtop <= dbottom) {
            dy = winrect.top() - frame.rect.height();
            snapped = True;
          }
          // snap bottom of other window?
          else if (dbottom < snap_distance) {
            dy = winrect.bottom() + 1;
            snapped = True;
          }

        }

        if (snapped) {
          if (screen->getWindowCornerSnap()) {
            // try corner-snap to its other sides
            if (snap_to_windows == BScreen::WindowResistance) {
              dleft = winrect.left() - wleft;
              dright = wright - winrect.right();
              if (dleft > 0 && dleft < resistance_size) {
                // if we're already past the left edge, then don't provide
                // resistance
                if (moving.left() >= winrect.left())
                  dx = winrect.left();
              } else if (dright > 0 && dright < resistance_size) {
                // if we're already past the right edge, then don't provide
                // resistance
                if (moving.right() <= winrect.right())
                  dx = winrect.right() - frame.rect.width() + 1;
              }
            } else { // BScreen::WindowSnap
              dleft = abs(wleft - winrect.left());
              dright = abs(wright - winrect.right());
              if (dleft < snap_distance && dleft <= dright)
                dx = winrect.left();
              else if (dright < snap_distance)
                dx = winrect.right() - frame.rect.width() + 1;
            }
          }

          continue;
        }
      }
    }
  }

  if (snap_to_edges) {
    RectList rectlist;

    // snap to the screen edges (and screen boundaries for xinerama)
#ifdef    XINERAMA
    if (screen->isXineramaActive() && blackbox->doXineramaSnapping()) {
      rectlist.insert(rectlist.begin(),
                      screen->getXineramaAreas().begin(),
                      screen->getXineramaAreas().end());
    } else
#endif // XINERAMA
      rectlist.push_back(screen->getRect());

    RectList::const_iterator it, end = rectlist.end();
    for (it = rectlist.begin(); it != end; ++it) {
      const Rect &srect = *it;

      if (snap_to_edges == BScreen::WindowResistance) {
        // if we're not in the rectangle then don't snap to it.
        if (! srect.contains(moving))
          continue;
      } else { // BScreen::WindowSnap
        // if we're not in the rectangle then don't snap to it.
        if (! srect.intersects(Rect(wleft, wtop, frame.rect.width(),
                                    frame.rect.height())))
          continue;
      }

      if (snap_to_edges == BScreen::WindowResistance) {
      int dleft = srect.left() - wleft,
         dright = wright - srect.right(),
           dtop = srect.top() - wtop,
        dbottom = wbottom - srect.bottom();

        // snap left?
        if (dleft > 0 && dleft < resistance_size)
          dx = srect.left();
        // snap right?
        else if (dright > 0 && dright < resistance_size)
          dx = srect.right() - frame.rect.width() + 1;

        // snap top?
        if (dtop > 0 && dtop < resistance_size)
          dy = srect.top();
        // snap bottom?
        else if (dbottom > 0 && dbottom < resistance_size)
          dy = srect.bottom() - frame.rect.height() + 1;
      } else { // BScreen::WindowSnap
        int dleft = abs(wleft - srect.left()),
           dright = abs(wright - srect.right()),
             dtop = abs(wtop - srect.top()),
          dbottom = abs(wbottom - srect.bottom());

        // snap left?
        if (dleft < snap_distance && dleft <= dright)
          dx = srect.left();
        // snap right?
        else if (dright < snap_distance)
          dx = srect.right() - frame.rect.width() + 1;

        // snap top?
        if (dtop < snap_distance && dtop <= dbottom)
          dy = srect.top();
        // snap bottom?
        else if (dbottom < snap_distance)
          dy = srect.bottom() - frame.rect.height() + 1;
      }
    }
  }
}


void BlackboxWindow::endMove(void) {
  assert(flags.moving);
  assert(blackbox->getChangingWindow() == this);

  flags.moving = False;
  blackbox->setChangingWindow(0);

  if (! screen->doOpaqueMove()) {
    /* when drawing the rubber band, we need to make sure we only draw inside
     * the frame... frame.changing_* contain the new coords for the window,
     * so we need to subtract 1 from changing_w/changing_h every where we
     * draw the rubber band (for both moving and resizing)
     */
    XDrawRectangle(blackbox->getXDisplay(), screen->getRootWindow(),
                   screen->getOpGC(), frame.changing.x(), frame.changing.y(),
                   frame.changing.width() - 1, frame.changing.height() - 1);
      XUngrabServer(blackbox->getXDisplay());
  
      configure(frame.changing.x(), frame.changing.y(),
                frame.changing.width(), frame.changing.height());
  } else {
    configure(frame.rect.x(), frame.rect.y(),
              frame.rect.width(), frame.rect.height());
  }
  screen->hideGeometry();

  XUngrabPointer(blackbox->getXDisplay(), CurrentTime);

  // if there are any left over motions from the move, drop them now
  XSync(blackbox->getXDisplay(), false); // make sure we don't miss any
  XEvent e;
  while (XCheckTypedWindowEvent(blackbox->getXDisplay(), frame.window,
                                MotionNotify, &e));
}


void BlackboxWindow::beginResize(int x_root, int y_root, Corner dir) {
  assert(! (flags.resizing || flags.moving));

  /*
    Only one window can be moved/resized at a time. If another window is already
    being moved or resized, then stop it before whating to work with this one.
  */
  BlackboxWindow *changing = blackbox->getChangingWindow();
  if (changing && changing != this) {
    if (changing->flags.moving)
      changing->endMove();
    else // if (changing->flags.resizing)
      changing->endResize();
  }

  resize_dir = dir;

  Cursor cursor;
  Corner anchor;
  
  switch (resize_dir) {
  case BottomLeft:
    anchor = TopRight;
    cursor = blackbox->getLowerLeftAngleCursor();
    break;

  case BottomRight:
    anchor = TopLeft;
    cursor = blackbox->getLowerRightAngleCursor();
    break;

  case TopLeft:
    anchor = BottomRight;
    cursor = blackbox->getUpperLeftAngleCursor();
    break;

  case TopRight:
    anchor = BottomLeft;
    cursor = blackbox->getUpperRightAngleCursor();
    break;

  default:
    assert(false); // unhandled Corner
    return;        // unreachable, for the compiler
  }
  
  XGrabServer(blackbox->getXDisplay());
  XGrabPointer(blackbox->getXDisplay(), frame.window, False,
               PointerMotionMask | ButtonReleaseMask,
               GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime);

  flags.resizing = True;
  blackbox->setChangingWindow(this);

  int gw, gh;
  frame.changing = frame.rect;

  constrain(anchor,  &gw, &gh);

  XDrawRectangle(blackbox->getXDisplay(), screen->getRootWindow(),
                 screen->getOpGC(), frame.changing.x(), frame.changing.y(),
                 frame.changing.width() - 1, frame.changing.height() - 1);

  screen->showGeometry(gw, gh);
  
  frame.grab_x = x_root;
  frame.grab_y = y_root;
}


void BlackboxWindow::doResize(int x_root, int y_root) {
  assert(flags.resizing);
  assert(blackbox->getChangingWindow() == this);

  XDrawRectangle(blackbox->getXDisplay(), screen->getRootWindow(),
                 screen->getOpGC(), frame.changing.x(), frame.changing.y(),
                 frame.changing.width() - 1, frame.changing.height() - 1);

  int gw, gh;
  Corner anchor;

  switch (resize_dir) {
  case BottomLeft:
    anchor = TopRight;
    frame.changing.setSize(frame.rect.width() - (x_root - frame.grab_x),
                           frame.rect.height() + (y_root - frame.grab_y));
    break;
  case BottomRight:
    anchor = TopLeft;
    frame.changing.setSize(frame.rect.width() + (x_root - frame.grab_x),
                           frame.rect.height() + (y_root - frame.grab_y));
    break;
  case TopLeft:
    anchor = BottomRight;
    frame.changing.setSize(frame.rect.width() - (x_root - frame.grab_x),
                           frame.rect.height() - (y_root - frame.grab_y));
    break;
  case TopRight:
    anchor = BottomLeft;
    frame.changing.setSize(frame.rect.width() + (x_root - frame.grab_x),
                           frame.rect.height() - (y_root - frame.grab_y));
    break;

  default:
    assert(false); // unhandled Corner
    return;        // unreachable, for the compiler
  }
  
  constrain(anchor, &gw, &gh);

  XDrawRectangle(blackbox->getXDisplay(), screen->getRootWindow(),
                 screen->getOpGC(), frame.changing.x(), frame.changing.y(),
                 frame.changing.width() - 1, frame.changing.height() - 1);

  screen->showGeometry(gw, gh);
}


void BlackboxWindow::endResize(void) {
  assert(flags.resizing);
  assert(blackbox->getChangingWindow() == this);

  XDrawRectangle(blackbox->getXDisplay(), screen->getRootWindow(),
                 screen->getOpGC(), frame.changing.x(), frame.changing.y(),
                 frame.changing.width() - 1, frame.changing.height() - 1);
  XUngrabServer(blackbox->getXDisplay());

  // unset maximized state after resized when fully maximized
  if (flags.maximized == 1)
    maximize(0);
  
  flags.resizing = False;
  blackbox->setChangingWindow(0);

  configure(frame.changing.x(), frame.changing.y(),
            frame.changing.width(), frame.changing.height());
  screen->hideGeometry();

  XUngrabPointer(blackbox->getXDisplay(), CurrentTime);
  
  // if there are any left over motions from the resize, drop them now
  XSync(blackbox->getXDisplay(), false); // make sure we don't miss any
  XEvent e;
  while (XCheckTypedWindowEvent(blackbox->getXDisplay(), frame.window,
                                MotionNotify, &e));
}


void BlackboxWindow::motionNotifyEvent(const XMotionEvent *me) {
#ifdef DEBUG
  fprintf(stderr, "BlackboxWindow::motionNotifyEvent() for 0x%lx\n",
          client.window);
#endif

  if (flags.moving) {
    doMove(me->x_root, me->y_root);
  } else if (flags.resizing) {
    doResize(me->x_root, me->y_root);
  } else {
    if (!flags.resizing && me->state & Button1Mask && (functions & Func_Move) &&
        (frame.title == me->window || frame.label == me->window ||
         frame.handle == me->window || frame.window == me->window)) {
      beginMove(me->x_root, me->y_root);
    } else if ((functions & Func_Resize) &&
               (me->state & Button1Mask && (me->window == frame.right_grip ||
                                            me->window == frame.left_grip)) ||
               (me->state & Button3Mask && me->state & ModMask &&
                me->window == frame.window)) {
      unsigned int zones = screen->getResizeZones();
      Corner corner;
      
      if (me->window == frame.left_grip) {
        corner = BottomLeft;
      } else if (me->window == frame.right_grip || zones == 1) {
        corner = BottomRight;
      } else {
        bool top;
        bool left = (me->x_root - frame.rect.x() <=
                     static_cast<signed>(frame.rect.width() / 2));
        if (zones == 2)
          top = False;
        else // (zones == 4)
          top = (me->y_root - frame.rect.y() <=
                 static_cast<signed>(frame.rect.height() / 2));
        corner = (top ? (left ? TopLeft : TopRight) :
                        (left ? BottomLeft : BottomRight));
      }

      beginResize(me->x_root, me->y_root, corner);
    }
  }
}


#ifdef    SHAPE
void BlackboxWindow::shapeEvent(XShapeEvent *) {
  if (blackbox->hasShapeExtensions() && flags.shaped) {
    configureShape();
  }
}
#endif // SHAPE


bool BlackboxWindow::validateClient(void) const {
  XSync(blackbox->getXDisplay(), False);

  XEvent e;
  if (XCheckTypedWindowEvent(blackbox->getXDisplay(), client.window,
                             DestroyNotify, &e) ||
      XCheckTypedWindowEvent(blackbox->getXDisplay(), client.window,
                             UnmapNotify, &e)) {
    XPutBackEvent(blackbox->getXDisplay(), &e);

    return False;
  }

  return True;
}


void BlackboxWindow::restore(bool remap) {
  XChangeSaveSet(blackbox->getXDisplay(), client.window, SetModeDelete);
  XSelectInput(blackbox->getXDisplay(), client.window, NoEventMask);
  XSelectInput(blackbox->getXDisplay(), frame.plate, NoEventMask);

  // do not leave a shaded window as an icon unless it was an icon
  if (flags.shaded && ! flags.iconic)
    setState(NormalState);

  restoreGravity(client.rect);

  XUnmapWindow(blackbox->getXDisplay(), frame.window);
  XUnmapWindow(blackbox->getXDisplay(), client.window);

  XSetWindowBorderWidth(blackbox->getXDisplay(), client.window, client.old_bw);

  XEvent ev;
  if (XCheckTypedWindowEvent(blackbox->getXDisplay(), client.window,
                             ReparentNotify, &ev)) {
    remap = True;
  } else {
    // according to the ICCCM - if the client doesn't reparent to
    // root, then we have to do it for them
    XReparentWindow(blackbox->getXDisplay(), client.window,
                    screen->getRootWindow(),
                    client.rect.x(), client.rect.y());
  }

  if (remap) XMapWindow(blackbox->getXDisplay(), client.window);
}


// timer for autoraise
void BlackboxWindow::timeout(void) {
  screen->getWorkspace(blackbox_attrib.workspace)->raiseWindow(this);
}


void BlackboxWindow::changeBlackboxHints(const BlackboxHints *net) {
  if ((net->flags & AttribShaded) &&
      ((blackbox_attrib.attrib & AttribShaded) !=
       (net->attrib & AttribShaded)))
    shade();

  if (flags.visible && // watch out for requests when we can not be seen
      (net->flags & (AttribMaxVert | AttribMaxHoriz)) &&
      ((blackbox_attrib.attrib & (AttribMaxVert | AttribMaxHoriz)) !=
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
      ((blackbox_attrib.attrib & AttribOmnipresent) !=
       (net->attrib & AttribOmnipresent)))
    stick();

  if ((net->flags & AttribWorkspace) &&
      (blackbox_attrib.workspace != net->workspace)) {
    screen->reassociateWindow(this, net->workspace, True);

    if (screen->getCurrentWorkspaceID() != net->workspace) {
      withdraw();
    } else {
      show();
      screen->getWorkspace(blackbox_attrib.workspace)->raiseWindow(this);
    }
  }

  if (net->flags & AttribDecoration) {
    switch (net->decoration) {
    case DecorNone:
      decorations = 0;

      break;

    default:
    case DecorNormal:
      decorations |= Decor_Titlebar | Decor_Border | Decor_Iconify;
  
      decorations = ((functions & Func_Resize) && !isTransient() ?
                     decorations | Decor_Handle :
                     decorations &= ~Decor_Handle);
      decorations = (functions & Func_Maximize ?
                     decorations | Decor_Maximize :
                     decorations &= ~Decor_Maximize);

      break;

    case DecorTiny:
      decorations |= Decor_Titlebar | Decor_Iconify;
      decorations &= ~(Decor_Border | Decor_Handle);
      
      decorations = (functions & Func_Maximize ?
                     decorations | Decor_Maximize :
                     decorations &= ~Decor_Maximize);

      break;

    case DecorTool:
      decorations |= Decor_Titlebar;
      decorations &= ~(Decor_Iconify | Decor_Border);

      decorations = ((functions & Func_Resize) && !isTransient() ?
                     decorations | Decor_Handle :
                     decorations &= ~Decor_Handle);
      decorations = (functions & Func_Maximize ?
                     decorations | Decor_Maximize :
                     decorations &= ~Decor_Maximize);

      break;
    }

    // we can not be shaded if we lack a titlebar
    if (flags.shaded && ! (decorations & Decor_Titlebar))
      shade();

    if (flags.visible && frame.window) {
      XMapSubwindows(blackbox->getXDisplay(), frame.window);
      XMapWindow(blackbox->getXDisplay(), frame.window);
    }

    reconfigure();
    setState(current_state);
  }
}


/*
 * Set the sizes of all components of the window frame
 * (the window decorations).
 * These values are based upon the current style settings and the client
 * window's dimensions.
 */
void BlackboxWindow::upsize(void) {
  frame.bevel_w = screen->getBevelWidth();

  if (decorations & Decor_Border) {
    frame.border_w = screen->getBorderWidth();
    if (! isTransient())
      frame.mwm_border_w = screen->getFrameWidth();
    else
      frame.mwm_border_w = 0;
  } else {
    frame.mwm_border_w = frame.border_w = 0;
  }

  if (decorations & Decor_Titlebar) {
    // the height of the titlebar is based upon the height of the font being
    // used to display the window's title
    WindowStyle *style = screen->getWindowStyle();
    frame.title_h = style->font->height() + (frame.bevel_w * 2) + 2;

    frame.label_h = frame.title_h - (frame.bevel_w * 2);
    frame.button_w = (frame.label_h - 2);

    // set the top frame margin
    frame.margin.top = frame.border_w + frame.title_h +
                       frame.border_w + frame.mwm_border_w;
  } else {
    frame.title_h = 0;
    frame.label_h = 0;
    frame.button_w = 0;

    // set the top frame margin
    frame.margin.top = frame.border_w + frame.mwm_border_w;
  }

  // set the left/right frame margin
  frame.margin.left = frame.margin.right = frame.border_w + frame.mwm_border_w;

  if (decorations & Decor_Handle) {
    frame.grip_w = frame.button_w * 2;
    frame.handle_h = screen->getHandleWidth();

    // set the bottom frame margin
    frame.margin.bottom = frame.border_w + frame.handle_h +
                          frame.border_w + frame.mwm_border_w;
  } else {
    frame.handle_h = 0;
    frame.grip_w = 0;

    // set the bottom frame margin
    frame.margin.bottom = frame.border_w + frame.mwm_border_w;
  }

  /*
    We first get the normal dimensions and use this to define the inside_w/h
    then we modify the height if shading is in effect.
    If the shade state is not considered then frame.rect gets reset to the
    normal window size on a reconfigure() call resulting in improper
    dimensions appearing in move/resize and other events.
  */
  unsigned int
    height = client.rect.height() + frame.margin.top + frame.margin.bottom,
    width = client.rect.width() + frame.margin.left + frame.margin.right;

  frame.inside_w = width - (frame.border_w * 2);
  frame.inside_h = height - (frame.border_w * 2);

  if (flags.shaded)
    height = frame.title_h + (frame.border_w * 2);
  frame.rect.setSize(width, height);
}


/*
 * Calculate the size of the client window and constrain it to the
 * size specified by the size hints of the client window.
 *
 * The logical width and height are placed into pw and ph, if they
 * are non-zero.  Logical size refers to the users perception of
 * the window size (for example an xterm resizes in cells, not in pixels).
 *
 * The physical geometry is placed into frame.changing_{x,y,width,height}.
 * Physical geometry refers to the geometry of the window in pixels.
 */
void BlackboxWindow::constrain(Corner anchor, int *pw, int *ph) {
  // frame.changing represents the requested frame size, we need to
  // strip the frame margin off and constrain the client size
  frame.changing.setCoords(frame.changing.left() + frame.margin.left,
                           frame.changing.top() + frame.margin.top,
                           frame.changing.right() - frame.margin.right,
                           frame.changing.bottom() - frame.margin.bottom);

  int dw = frame.changing.width(), dh = frame.changing.height(),
    base_width = (client.base_width) ? client.base_width : client.min_width,
    base_height = (client.base_height) ? client.base_height :
                                         client.min_height;

  // constrain
  if (dw < static_cast<signed>(client.min_width)) dw = client.min_width;
  if (dh < static_cast<signed>(client.min_height)) dh = client.min_height;
  if (dw > static_cast<signed>(client.max_width)) dw = client.max_width;
  if (dh > static_cast<signed>(client.max_height)) dh = client.max_height;

  dw -= base_width;
  dw /= client.width_inc;
  dh -= base_height;
  dh /= client.height_inc;

  if (pw) {
    if (client.width_inc == 1)
      *pw = dw + base_width;
    else
      *pw = dw;
  }
  if (ph) {
    if (client.height_inc == 1)
      *ph = dh + base_height;
    else
      *ph = dh;
  }

  dw *= client.width_inc;
  dw += base_width;
  dh *= client.height_inc;
  dh += base_height;

  frame.changing.setSize(dw, dh);

  // add the frame margin back onto frame.changing
  frame.changing.setCoords(frame.changing.left() - frame.margin.left,
                           frame.changing.top() - frame.margin.top,
                           frame.changing.right() + frame.margin.right,
                           frame.changing.bottom() + frame.margin.bottom);

  // move frame.changing to the specified anchor
  int dx = 0,
      dy = 0;
  switch (anchor) {
  case TopLeft:
    break;

  case TopRight:
    dx = frame.rect.right() - frame.changing.right();
    break;

  case BottomLeft:
    dy = frame.rect.bottom() - frame.changing.bottom();
    break;

  case BottomRight:
    dx = frame.rect.right() - frame.changing.right();
    dy = frame.rect.bottom() - frame.changing.bottom();
    break;

  default:
    assert(false);  // unhandled corner
  }
  frame.changing.setPos(frame.changing.x() + dx, frame.changing.y() + dy);
}


void WindowStyle::doJustify(const std::string &text, int &start_pos,
                            unsigned int max_length,
                            unsigned int modifier) const {
  size_t text_len = text.size();
  unsigned int length;

  do {
    length = font->measureString(string(text, 0, text_len)) + modifier;
  } while (length > max_length && text_len-- > 0);

  switch (justify) {
  case RightJustify:
    start_pos += max_length - length;
    break;

  case CenterJustify:
    start_pos += (max_length - length) / 2;
    break;

  case LeftJustify:
  default:
    break;
  }
}


BWindowGroup::BWindowGroup(Blackbox *b, Window _group)
  : blackbox(b), group(_group) {
  XWindowAttributes wattrib;
  if (! XGetWindowAttributes(blackbox->getXDisplay(), group, &wattrib)) {
    // group window doesn't seem to exist anymore
    delete this;
    return;
  }

  XSelectInput(blackbox->getXDisplay(), group,
               PropertyChangeMask | FocusChangeMask | StructureNotifyMask);

  blackbox->saveGroupSearch(group, this);
}


BWindowGroup::~BWindowGroup(void) {
  blackbox->removeGroupSearch(group);
}


BlackboxWindow *
BWindowGroup::find(BScreen *screen, bool allow_transients) const {
  BlackboxWindow *ret = blackbox->getFocusedWindow();

  // does the focus window match (or any transient_fors)?
  while (ret) {
    if (ret->getScreen() == screen && ret->getGroupWindow() == group) {
      if (ret->isTransient() && allow_transients) break;
      else if (! ret->isTransient()) break;
    }

    ret = ret->getTransientFor();
  }

  if (ret) return ret;

  // the focus window didn't match, look in the group's window list
  BlackboxWindowList::const_iterator it, end = windowList.end();
  for (it = windowList.begin(); it != end; ++it) {
    ret = *it;
    if (ret->getScreen() == screen && ret->getGroupWindow() == group) {
      if (ret->isTransient() && allow_transients) break;
      else if (! ret->isTransient()) break;
    }
  }

  return ret;
}
