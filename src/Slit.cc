// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Slit.cc for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
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

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <X11/keysym.h>
}

#include "i18n.hh"
#include "blackbox.hh"
#include "Image.hh"
#include "Screen.hh"
#include "Slit.hh"
#include "Toolbar.hh"


Slit::Slit(BScreen *scr) {
  screen = scr;
  blackbox = screen->getBlackbox();

  on_top = screen->isSlitOnTop();
  hidden = do_auto_hide = screen->doSlitAutoHide();

  display = screen->getBaseDisplay()->getXDisplay();
  frame.window = frame.pixmap = None;

  timer = new BTimer(blackbox, this);
  timer->setTimeout(blackbox->getAutoRaiseDelay());

  slitmenu = new Slitmenu(this);

  XSetWindowAttributes attrib;
  unsigned long create_mask = CWBackPixmap | CWBackPixel | CWBorderPixel |
                              CWColormap | CWOverrideRedirect | CWEventMask;
  attrib.background_pixmap = None;
  attrib.background_pixel = attrib.border_pixel =
    screen->getBorderColor()->pixel();
  attrib.colormap = screen->getColormap();
  attrib.override_redirect = True;
  attrib.event_mask = SubstructureRedirectMask | ButtonPressMask |
                      EnterWindowMask | LeaveWindowMask;

  frame.rect.setSize(1, 1);

  frame.window =
    XCreateWindow(display, screen->getRootWindow(),
                  frame.rect.x(), frame.rect.y(),
                  frame.rect.width(), frame.rect.height(),
                  screen->getBorderWidth(), screen->getDepth(), InputOutput,
                  screen->getVisual(), create_mask, &attrib);
  blackbox->saveSlitSearch(frame.window, this);

  screen->addStrut(&strut);

  reconfigure();
}


Slit::~Slit(void) {
  delete timer;

  delete slitmenu;

  screen->getImageControl()->removeImage(frame.pixmap);

  blackbox->removeSlitSearch(frame.window);

  XDestroyWindow(display, frame.window);
}


void Slit::addClient(Window w) {
  if (! blackbox->validateWindow(w))
    return;

  SlitClient *client = new SlitClient;
  client->client_window = w;

  XWMHints *wmhints = XGetWMHints(display, w);

  if (wmhints) {
    if ((wmhints->flags & IconWindowHint) &&
        (wmhints->icon_window != None)) {
      // some dock apps use separate windows, we need to hide these
      XMoveWindow(display, client->client_window, screen->getWidth() + 10,
                  screen->getHeight() + 10);
      XMapWindow(display, client->client_window);

      client->icon_window = wmhints->icon_window;
      client->window = client->icon_window;
    } else {
      client->icon_window = None;
      client->window = client->client_window;
    }

    XFree(wmhints);
  } else {
    client->icon_window = None;
    client->window = client->client_window;
  }

  XWindowAttributes attrib;
  if (XGetWindowAttributes(display, client->window, &attrib)) {
    client->rect.setSize(attrib.width, attrib.height);
  } else {
    client->rect.setSize(64, 64);
  }

  XSetWindowBorderWidth(display, client->window, 0);

  XGrabServer(display);
  XSelectInput(display, frame.window, NoEventMask);
  XSelectInput(display, client->window, NoEventMask);
  XReparentWindow(display, client->window, frame.window, 0, 0);
  XMapRaised(display, client->window);
  XChangeSaveSet(display, client->window, SetModeInsert);
  XSelectInput(display, frame.window, SubstructureRedirectMask |
               ButtonPressMask | EnterWindowMask | LeaveWindowMask);
  XSelectInput(display, client->window, StructureNotifyMask |
               SubstructureNotifyMask | EnterWindowMask);

  XUngrabServer(display);

  clientList.push_back(client);

  blackbox->saveSlitSearch(client->client_window, this);
  blackbox->saveSlitSearch(client->icon_window, this);
  reconfigure();
}


void Slit::removeClient(SlitClient *client, bool remap) {
  blackbox->removeSlitSearch(client->client_window);
  blackbox->removeSlitSearch(client->icon_window);
  clientList.remove(client);

  screen->removeNetizen(client->window);

  if (remap && blackbox->validateWindow(client->window)) {
    XGrabServer(display);
    XSelectInput(display, frame.window, NoEventMask);
    XSelectInput(display, client->window, NoEventMask);
    XReparentWindow(display, client->window, screen->getRootWindow(),
                    client->rect.x(), client->rect.y());
    XChangeSaveSet(display, client->window, SetModeDelete);
    XSelectInput(display, frame.window, SubstructureRedirectMask |
                 ButtonPressMask | EnterWindowMask | LeaveWindowMask);
    XUngrabServer(display);
  }

  delete client;
  client = (SlitClient *) 0;
}


struct SlitClientMatch {
  Window window;
  SlitClientMatch(Window w): window(w) {}
  inline bool operator()(const Slit::SlitClient* client) const {
    return (client->window == window);
  }
};


void Slit::removeClient(Window w, bool remap) {
  SlitClientList::iterator it = clientList.begin();
  const SlitClientList::iterator end = clientList.end();

  it = std::find_if(it, end, SlitClientMatch(w));
  if (it != end) {
    removeClient(*it, remap);
    reconfigure();
  }
}


void Slit::reconfigure(void) {
  SlitClientList::iterator it = clientList.begin();
  const SlitClientList::iterator end = clientList.end();
  SlitClient *client;

  unsigned int width = 0, height = 0;

  switch (screen->getSlitDirection()) {
  case Vertical:
    for (; it != end; ++it) {
      client = *it;
      height += client->rect.height() + screen->getBevelWidth();

      if (width < client->rect.width())
        width = client->rect.width();
    }

    if (width < 1)
      width = 1;
    else
      width += (screen->getBevelWidth() * 2);

    if (height < 1)
      height = 1;
    else
      height += screen->getBevelWidth();

    break;

  case Horizontal:
    for (; it != end; ++it) {
      client = *it;
      width += client->rect.width() + screen->getBevelWidth();

      if (height < client->rect.height())
        height = client->rect.height();
    }

    if (width < 1)
      width = 1;
    else
      width += screen->getBevelWidth();

    if (height < 1)
      height = 1;
    else
      height += (screen->getBevelWidth() * 2);

    break;
  }
  frame.rect.setSize(width, height);

  reposition();

  XSetWindowBorderWidth(display ,frame.window, screen->getBorderWidth());
  XSetWindowBorder(display, frame.window,
                   screen->getBorderColor()->pixel());

  if (clientList.empty())
    XUnmapWindow(display, frame.window);
  else
    XMapWindow(display, frame.window);

  BTexture *texture = &(screen->getToolbarStyle()->toolbar);
  frame.pixmap = texture->render(frame.rect.width(), frame.rect.height(),
                                 frame.pixmap);
  if (! frame.pixmap)
    XSetWindowBackground(display, frame.window, texture->color().pixel());
  else
    XSetWindowBackgroundPixmap(display, frame.window, frame.pixmap);

  XClearWindow(display, frame.window);

  it = clientList.begin();

  int x, y;

  switch (screen->getSlitDirection()) {
  case Vertical:
    x = 0;
    y = screen->getBevelWidth();

    for (; it != end; ++it) {
      client = *it;
      x = (frame.rect.width() - client->rect.width()) / 2;

      XMoveResizeWindow(display, client->window, x, y,
                        client->rect.width(), client->rect.height());
      XMapWindow(display, client->window);

      // for ICCCM compliance
      client->rect.setPos(x, y);

      XEvent event;
      event.type = ConfigureNotify;

      event.xconfigure.display = display;
      event.xconfigure.event = client->window;
      event.xconfigure.window = client->window;
      event.xconfigure.x = x;
      event.xconfigure.y = y;
      event.xconfigure.width = client->rect.width();
      event.xconfigure.height = client->rect.height();
      event.xconfigure.border_width = 0;
      event.xconfigure.above = frame.window;
      event.xconfigure.override_redirect = False;

      XSendEvent(display, client->window, False, StructureNotifyMask, &event);

      y += client->rect.height() + screen->getBevelWidth();
    }

    break;

  case Horizontal:
    x = screen->getBevelWidth();
    y = 0;

    for (; it != end; ++it) {
      client = *it;
      y = (frame.rect.height() - client->rect.height()) / 2;

      XMoveResizeWindow(display, client->window, x, y,
                        client->rect.width(), client->rect.height());
      XMapWindow(display, client->window);

      // for ICCCM compliance
      client->rect.setPos(x, y);

      XEvent event;
      event.type = ConfigureNotify;

      event.xconfigure.display = display;
      event.xconfigure.event = client->window;
      event.xconfigure.window = client->window;
      event.xconfigure.x = x;
      event.xconfigure.y = y;
      event.xconfigure.width = client->rect.width();
      event.xconfigure.height = client->rect.height();
      event.xconfigure.border_width = 0;
      event.xconfigure.above = frame.window;
      event.xconfigure.override_redirect = False;

      XSendEvent(display, client->window, False, StructureNotifyMask, &event);

      x += client->rect.width() + screen->getBevelWidth();
    }
    break;
  }

  slitmenu->reconfigure();
}


void Slit::updateStrut(void) {
  strut.top = strut.bottom = strut.left = strut.right = 0;

  if (! clientList.empty()) {
    switch (screen->getSlitDirection()) {
    case Vertical:
      switch (screen->getSlitPlacement()) {
      case TopCenter:
        strut.top = getY() + getExposedHeight() +
                    (screen->getBorderWidth() * 2);
        break;
      case BottomCenter:
        strut.bottom = screen->getHeight() - getY();
        break;
      case TopLeft:
      case CenterLeft:
      case BottomLeft:
        strut.left = getExposedWidth() + (screen->getBorderWidth() * 2);
        break;
      case TopRight:
      case CenterRight:
      case BottomRight:
        strut.right = getExposedWidth() + (screen->getBorderWidth() * 2);
        break;
      }
      break;
    case Horizontal:
      switch (screen->getSlitPlacement()) {
      case TopCenter:
      case TopLeft:
      case TopRight:
        strut.top = getY() + getExposedHeight() +
                    (screen->getBorderWidth() * 2);
        break;
      case BottomCenter:
      case BottomLeft:
      case BottomRight:
        strut.bottom = screen->getHeight() - getY();
        break;
      case CenterLeft:
        strut.left = getExposedWidth() + (screen->getBorderWidth() * 2);
        break;
      case CenterRight:
        strut.right = getExposedWidth() + (screen->getBorderWidth() * 2);
        break;
      }
      break;
    }
  }

  // update area with new Strut info
  screen->updateAvailableArea();
}


void Slit::reposition(void) {
  // place the slit in the appropriate place
  switch (screen->getSlitPlacement()) {
  case TopLeft:
    frame.rect.setPos(0, 0);

    if (screen->getSlitDirection() == Vertical) {
      frame.x_hidden = screen->getBevelWidth() - screen->getBorderWidth()
                       - frame.rect.width();
      frame.y_hidden = 0;
    } else {
      frame.x_hidden = 0;
      frame.y_hidden = screen->getBevelWidth() - screen->getBorderWidth()
                       - frame.rect.height();
    }
    break;

  case CenterLeft:
    frame.rect.setPos(0, (screen->getHeight() - frame.rect.height()) / 2);

    frame.x_hidden = screen->getBevelWidth() - screen->getBorderWidth()
                     - frame.rect.width();
    frame.y_hidden = frame.rect.y();
    break;

  case BottomLeft:
    frame.rect.setPos(0, (screen->getHeight() - frame.rect.height()
                          - (screen->getBorderWidth() * 2)));

    if (screen->getSlitDirection() == Vertical) {
      frame.x_hidden = screen->getBevelWidth() - screen->getBorderWidth()
                       - frame.rect.width();
      frame.y_hidden = frame.rect.y();
    } else {
      frame.x_hidden = 0;
      frame.y_hidden = screen->getHeight() - screen->getBevelWidth()
                       - screen->getBorderWidth();
    }
    break;

  case TopCenter:
    frame.rect.setPos((screen->getWidth() - frame.rect.width()) / 2, 0);

    frame.x_hidden = frame.rect.x();
    frame.y_hidden = screen->getBevelWidth() - screen->getBorderWidth()
                     - frame.rect.height();
    break;

  case BottomCenter:
    frame.rect.setPos((screen->getWidth() - frame.rect.width()) / 2,
                      (screen->getHeight() - frame.rect.height()
                       - (screen->getBorderWidth() * 2)));
    frame.x_hidden = frame.rect.x();
    frame.y_hidden = screen->getHeight() - screen->getBevelWidth()
                     - screen->getBorderWidth();
    break;

  case TopRight:
    frame.rect.setPos((screen->getWidth() - frame.rect.width()
                       - (screen->getBorderWidth() * 2)), 0);

    if (screen->getSlitDirection() == Vertical) {
      frame.x_hidden = screen->getWidth() - screen->getBevelWidth()
                       - screen->getBorderWidth();
      frame.y_hidden = 0;
    } else {
      frame.x_hidden = frame.rect.x();
      frame.y_hidden = screen->getBevelWidth() - screen->getBorderWidth()
                       - frame.rect.height();
    }
    break;

  case CenterRight:
  default:
    frame.rect.setPos((screen->getWidth() - frame.rect.width()
                       - (screen->getBorderWidth() * 2)),
                      (screen->getHeight() - frame.rect.height()) / 2);

    frame.x_hidden = screen->getWidth() - screen->getBevelWidth()
                     - screen->getBorderWidth();
    frame.y_hidden = frame.rect.y();
    break;

  case BottomRight:
    frame.rect.setPos((screen->getWidth() - frame.rect.width()
                       - (screen->getBorderWidth() * 2)),
                      (screen->getHeight() - frame.rect.height()
                       - (screen->getBorderWidth() * 2)));

    if (screen->getSlitDirection() == Vertical) {
      frame.x_hidden = screen->getWidth() - screen->getBevelWidth()
                       - screen->getBorderWidth();
      frame.y_hidden = frame.rect.y();
    } else {
      frame.x_hidden = frame.rect.x();
      frame.y_hidden = screen->getHeight() - screen->getBevelWidth()
                       - screen->getBorderWidth();
    }
    break;
  }

  Rect tbar_rect = screen->getToolbar()->getRect();
  tbar_rect.setSize(tbar_rect.width() + (screen->getBorderWidth() * 2),
                    tbar_rect.height() + (screen->getBorderWidth() * 2));
  Rect slit_rect = frame.rect;
  slit_rect.setSize(slit_rect.width() + (screen->getBorderWidth() * 2),
                    slit_rect.height() + (screen->getBorderWidth() * 2));

  if (slit_rect.intersects(tbar_rect)) {
    Toolbar *tbar = screen->getToolbar();
    frame.y_hidden = frame.rect.y();

    int delta = tbar->getExposedHeight() + (screen->getBorderWidth() * 2);
    if (frame.rect.bottom() <= tbar_rect.bottom()) {
      delta = -delta;
    }
    frame.rect.setY(frame.rect.y() + delta);
    if (screen->getSlitDirection() == Vertical)
      frame.y_hidden += delta;
  }

  updateStrut();

  if (hidden)
    XMoveResizeWindow(display, frame.window, frame.x_hidden,
                      frame.y_hidden, frame.rect.width(), frame.rect.height());
  else
    XMoveResizeWindow(display, frame.window, frame.rect.x(), frame.rect.y(),
                      frame.rect.width(), frame.rect.height());
}


void Slit::shutdown(void) {
  while (! clientList.empty())
    removeClient(clientList.front());
}


void Slit::buttonPressEvent(XButtonEvent *e) {
  if (e->window != frame.window) return;

  if (e->button == Button1 && (! on_top)) {
    Window w[1] = { frame.window };
    screen->raiseWindows(w, 1);
  } else if (e->button == Button2 && (! on_top)) {
    XLowerWindow(display, frame.window);
  } else if (e->button == Button3) {
    if (! slitmenu->isVisible()) {
      int x, y;

      x = e->x_root - (slitmenu->getWidth() / 2);
      y = e->y_root - (slitmenu->getHeight() / 2);

      if (x < 0)
        x = 0;
      else if (x + slitmenu->getWidth() > screen->getWidth())
        x = screen->getWidth() - slitmenu->getWidth();

      if (y < 0)
        y = 0;
      else if (y + slitmenu->getHeight() > screen->getHeight())
        y = screen->getHeight() - slitmenu->getHeight();

      slitmenu->move(x, y);
      slitmenu->show();
    } else {
      slitmenu->hide();
    }
  }
}


void Slit::enterNotifyEvent(XCrossingEvent *) {
  if (! do_auto_hide)
    return;

  if (hidden) {
    if (! timer->isTiming()) timer->start();
  } else {
    if (timer->isTiming()) timer->stop();
  }
}


void Slit::leaveNotifyEvent(XCrossingEvent *) {
  if (! do_auto_hide)
    return;

  if (hidden) {
    if (timer->isTiming()) timer->stop();
  } else if (! slitmenu->isVisible()) {
    if (! timer->isTiming()) timer->start();
  }
}


void Slit::configureRequestEvent(XConfigureRequestEvent *e) {
  if (! blackbox->validateWindow(e->window))
    return;

  XWindowChanges xwc;

  xwc.x = e->x;
  xwc.y = e->y;
  xwc.width = e->width;
  xwc.height = e->height;
  xwc.border_width = 0;
  xwc.sibling = e->above;
  xwc.stack_mode = e->detail;

  XConfigureWindow(display, e->window, e->value_mask, &xwc);

  SlitClientList::iterator it = clientList.begin();
  const SlitClientList::iterator end = clientList.end();
  for (; it != end; ++it) {
    SlitClient *client = *it;
    if (client->window == e->window &&
        (static_cast<signed>(client->rect.width()) != e->width ||
         static_cast<signed>(client->rect.height()) != e->height)) {
      client->rect.setSize(e->width, e->height);

      reconfigure();
      return;
    }
  }
}


void Slit::timeout(void) {
  hidden = ! hidden;
  if (hidden)
    XMoveWindow(display, frame.window, frame.x_hidden, frame.y_hidden);
  else
    XMoveWindow(display, frame.window, frame.rect.x(), frame.rect.y());
}


void Slit::toggleAutoHide(void) {
  do_auto_hide = (do_auto_hide) ?  False : True;

  updateStrut();

  if (do_auto_hide == False && hidden) {
    // force the slit to be visible
    if (timer->isTiming()) timer->stop();
    timeout();
  }
}


void Slit::unmapNotifyEvent(XUnmapEvent *e) {
  removeClient(e->window);
}


Slitmenu::Slitmenu(Slit *sl) : Basemenu(sl->screen) {
  slit = sl;

  setLabel(i18n(SlitSet, SlitSlitTitle, "Slit"));
  setInternalMenu();

  directionmenu = new Directionmenu(this);
  placementmenu = new Placementmenu(this);

  insert(i18n(CommonSet, CommonDirectionTitle, "Direction"),
         directionmenu);
  insert(i18n(CommonSet, CommonPlacementTitle, "Placement"),
         placementmenu);
  insert(i18n(CommonSet, CommonAlwaysOnTop, "Always on top"), 1);
  insert(i18n(CommonSet, CommonAutoHide, "Auto hide"), 2);

  update();

  if (slit->isOnTop()) setItemSelected(2, True);
  if (slit->doAutoHide()) setItemSelected(3, True);
}


Slitmenu::~Slitmenu(void) {
  delete directionmenu;
  delete placementmenu;
}


void Slitmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);
  if (! item) return;

  switch (item->function()) {
  case 1: { // always on top
    slit->on_top = ((slit->isOnTop()) ?  False : True);
    setItemSelected(2, slit->on_top);

    if (slit->isOnTop()) slit->screen->raiseWindows((Window *) 0, 0);
    break;
  }

  case 2: { // auto hide
    slit->toggleAutoHide();
    setItemSelected(3, slit->do_auto_hide);

    break;
  }
  } // switch
}


void Slitmenu::internal_hide(void) {
  Basemenu::internal_hide();
  if (slit->doAutoHide())
    slit->timeout();
}


void Slitmenu::reconfigure(void) {
  directionmenu->reconfigure();
  placementmenu->reconfigure();

  Basemenu::reconfigure();
}


Slitmenu::Directionmenu::Directionmenu(Slitmenu *sm)
  : Basemenu(sm->slit->screen) {

  setLabel(i18n(SlitSet, SlitSlitDirection, "Slit Direction"));
  setInternalMenu();

  insert(i18n(CommonSet, CommonDirectionHoriz, "Horizontal"),
         Slit::Horizontal);
  insert(i18n(CommonSet, CommonDirectionVert, "Vertical"),
         Slit::Vertical);

  update();

  if (getScreen()->getSlitDirection() == Slit::Horizontal)
    setItemSelected(0, True);
  else
    setItemSelected(1, True);
}


void Slitmenu::Directionmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);
  if (! item) return;

  getScreen()->saveSlitDirection(item->function());

  if (item->function() == Slit::Horizontal) {
    setItemSelected(0, True);
    setItemSelected(1, False);
  } else {
    setItemSelected(0, False);
    setItemSelected(1, True);
  }

  hide();
  getScreen()->getSlit()->reconfigure();
}


Slitmenu::Placementmenu::Placementmenu(Slitmenu *sm)
  : Basemenu(sm->slit->screen) {

  setLabel(i18n(SlitSet, SlitSlitPlacement, "Slit Placement"));
  setMinimumSublevels(3);
  setInternalMenu();

  insert(i18n(CommonSet, CommonPlacementTopLeft, "Top Left"),
         Slit::TopLeft);
  insert(i18n(CommonSet, CommonPlacementCenterLeft, "Center Left"),
         Slit::CenterLeft);
  insert(i18n(CommonSet, CommonPlacementBottomLeft, "Bottom Left"),
         Slit::BottomLeft);
  insert(i18n(CommonSet, CommonPlacementTopCenter, "Top Center"),
         Slit::TopCenter);
  insert("");
  insert(i18n(CommonSet, CommonPlacementBottomCenter, "Bottom Center"),
         Slit::BottomCenter);
  insert(i18n(CommonSet, CommonPlacementTopRight, "Top Right"),
         Slit::TopRight);
  insert(i18n(CommonSet, CommonPlacementCenterRight, "Center Right"),
         Slit::CenterRight);
  insert(i18n(CommonSet, CommonPlacementBottomRight, "Bottom Right"),
         Slit::BottomRight);

  update();
}


void Slitmenu::Placementmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);
  if (! (item && item->function())) return;

  getScreen()->saveSlitPlacement(item->function());
  hide();
  getScreen()->getSlit()->reconfigure();
}

