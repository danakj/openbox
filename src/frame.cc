// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "frame.hh"
#include "client.hh"
#include "otk/display.hh"

namespace ob {

OBFrame::OBFrame(const OBClient *client, const otk::Style *style)
  : _client(client),
    _screen(otk::OBDisplay::screenInfo(client->screen()))
{
  assert(client);
  assert(style);
  
  _style = 0;
  loadStyle(style);

  _window = createFrame();
  assert(_window);

  grabClient();
}


OBFrame::~OBFrame()
{
  releaseClient(false);
}


void OBFrame::loadStyle(const otk::Style *style)
{
  assert(style);

  // if a style was previously set, then 'replace' is true, cause we're
  // replacing a style
  // NOTE: if this is false, then DO NOT DO SHIT WITH _window, it doesnt exist
  bool replace = (_style);

  if (replace) {
    // XXX: do shit here whatever
  }
  
  _style = style;

  // XXX: load shit like this from the style!
  _size.left = _size.top = _size.bottom = _size.right = 2;

  if (replace) {
    XSetWindowBorderWidth(otk::OBDisplay::display, _window,
                          _style->getBorderWidth());

    // XXX: make everything redraw
  }
}


void OBFrame::resize()
{
  XResizeWindow(otk::OBDisplay::display, _window,
                _size.left + _size.right + _client->area().width(),
                _size.top + _size.bottom + _client->area().height());
  XMoveWindow(otk::OBDisplay::display, _client->window(),
              _size.left, _size.top);
  // XXX: more is gunna have to happen here
}


void OBFrame::shape()
{
  // XXX: if shaped, shape the frame to the client..
}


void OBFrame::grabClient()
{
  
  XGrabServer(otk::OBDisplay::display);

  // select the event mask on the frame
  XSelectInput(otk::OBDisplay::display, _window, SubstructureRedirectMask);

  // reparent the client to the frame
  XSelectInput(otk::OBDisplay::display, _client->window(),
               OBClient::event_mask & ~StructureNotifyMask);
  XReparentWindow(otk::OBDisplay::display, _client->window(), _window, 0, 0);
  XSelectInput(otk::OBDisplay::display, _client->window(),
               OBClient::event_mask);

  // raise the client above the frame
  XRaiseWindow(otk::OBDisplay::display, _client->window());
  // map the client so it maps when the frame does
  XMapWindow(otk::OBDisplay::display, _client->window());
  
  XUngrabServer(otk::OBDisplay::display);

  resize();
  shape();
}


void OBFrame::releaseClient(bool remap)
{
  // check if the app has already reparented its window to the root window
  XEvent ev;
  if (XCheckTypedWindowEvent(otk::OBDisplay::display, _client->window(),
                             ReparentNotify, &ev)) {
    remap = true; // XXX: why do we remap the window if they already
                  // reparented to root?
  } else {
    // according to the ICCCM - if the client doesn't reparent to
    // root, then we have to do it for them
    XReparentWindow(otk::OBDisplay::display, _client->window(),
                    _screen->getRootWindow(),
                    _client->area().x(), _client->area().y());
  }

  // if we want to remap the window, do so now
  if (remap)
    XMapWindow(otk::OBDisplay::display, _client->window());
}


Window OBFrame::createFrame()
{
  XSetWindowAttributes attrib_create;
  unsigned long create_mask = CWBackPixmap | CWBorderPixel | CWColormap |
                              CWOverrideRedirect | CWEventMask;

  attrib_create.background_pixmap = None;
  attrib_create.colormap = _screen->getColormap();
  attrib_create.override_redirect = True;
  attrib_create.event_mask = EnterWindowMask | LeaveWindowMask | ButtonPress;
  /*
    We catch button presses because other wise they get passed down to the
    root window, which will then cause root menus to show when you click the
    window's frame.
  */

  return XCreateWindow(otk::OBDisplay::display, _screen->getRootWindow(),
                       0, 0, 1, 1, _style->getBorderWidth(),
                       _screen->getDepth(), InputOutput, _screen->getVisual(),
                       create_mask, &attrib_create);
}

}
