// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

extern "C" {
#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE
}

#include "frame.hh"
#include "client.hh"
#include "otk/display.hh"

#include <string>

namespace ob {

OBFrame::OBFrame(const OBClient *client, const otk::Style *style)
  : _client(client),
    _screen(otk::OBDisplay::screenInfo(client->screen()))
{
  assert(client);
  assert(style);
 
  _decorations = client->decorations();
 
  _style = 0;
  loadStyle(style);

  // create the base frame parent window
  _window = createFrame();
  assert(_window);

  // create all of the style element child windows
  _titlebar = createChild(_window, 0);
  assert(_titlebar);
  _button_iconify = createChild(_titlebar, 0);
  assert(_button_iconify);
  _button_max = createChild(_titlebar, 0);
  assert(_button_max);
  _button_stick = createChild(_titlebar, 0);
  assert(_button_stick);
  _button_close = createChild(_titlebar, 0);
  assert(_button_close);
  _label = createChild(_titlebar, 0);
  assert(_label);
  XMapSubwindows(otk::OBDisplay::display, _titlebar);

  _handle = createChild(_window, 0);
  assert(_handle);
  _grip_left = createChild(_handle, 0);
  assert(_grip_left);
  _grip_right = createChild(_handle, 0);
  assert(_grip_right);
  XMapSubwindows(otk::OBDisplay::display, _handle);
  
  grabClient();
}


OBFrame::~OBFrame()
{
  releaseClient(false);

  XDestroyWindow(otk::OBDisplay::display, _titlebar);
  XDestroyWindow(otk::OBDisplay::display, _button_iconify);
  XDestroyWindow(otk::OBDisplay::display, _button_max);
  XDestroyWindow(otk::OBDisplay::display, _button_stick);
  XDestroyWindow(otk::OBDisplay::display, _button_close);
  XDestroyWindow(otk::OBDisplay::display, _label);
  XDestroyWindow(otk::OBDisplay::display, _handle);
  XDestroyWindow(otk::OBDisplay::display, _grip_left);
  XDestroyWindow(otk::OBDisplay::display, _grip_right);
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

  if (replace) {
    update();
    
    XSetWindowBorderWidth(otk::OBDisplay::display, _window,
                          _style->getBorderWidth());

    // XXX: make everything redraw
  }
}


void OBFrame::update()
{
  // XXX: only if not overridden or something!!! MORE LOGIC HERE!!
  _decorations = _client->decorations();

  int width;   // the width of the client window and the border around it
  
  if (_decorations & OBClient::Decor_Border) {
    _size.left = _size.top = _size.bottom = _size.right =
      _style->getBorderWidth() + _style->getFrameWidth();
    width = _client->area().width() + _style->getFrameWidth() * 2;
  } else {
    _size.left = _size.top = _size.bottom = _size.right = 0;
    width = _client->area().width();
  }

  if (_decorations & OBClient::Decor_Titlebar) {
    _titlebar_area.setRect(0, 0, width,
                           (_style->getFont()->height() +
                            _style->getFrameWidth() * 2));
    _size.top += _titlebar_area.height() + _style->getBorderWidth();

    // set the label size
    _label_area.setRect(0, _style->getBevelWidth(),
                        width, (_titlebar_area.height() -
                                _style->getBevelWidth() * 2));
    // set the buttons sizes
    if (_decorations & OBClient::Decor_Iconify)
      _button_iconify_area.setRect(0, _style->getBevelWidth() + 1,
                                   _label_area.height() - 2,
                                   _label_area.height() - 2);
    if (_decorations & OBClient::Decor_Maximize)
      _button_max_area.setRect(0, _style->getBevelWidth() + 1,
                               _label_area.height() - 2,
                               _label_area.height() - 2);
    if (_decorations & OBClient::Decor_Sticky)
      _button_stick_area.setRect(0, _style->getBevelWidth() + 1,
                                 _label_area.height() - 2,
                                 _label_area.height() - 2);
    if (_decorations & OBClient::Decor_Close)
      _button_close_area.setRect(0, _style->getBevelWidth() + 1,
                                 _label_area.height() - 2,
                                 _label_area.height() - 2);

    // separation between titlebar elements
    const int sep = _style->getBevelWidth() + 1;

    std::string layout = "ILMC"; // XXX: get this from somewhere
    // XXX: it is REQUIRED that by this point, the string only has one of each
    // possible letter, all of the letters are valid, and L exists somewhere in
    // the string!

    int x = sep;
    for (int i = 0, len = layout.size(); i < len; ++i) {
      otk::Rect *area;
      switch (layout[i]) {
      case 'I':
        if (!(_decorations & OBClient::Decor_Iconify))
          continue; // skip it
        area = &_button_iconify_area;
        break;
      case 'L':
        area = &_label_area;
        break;
      case 'M':
        if (!(_decorations & OBClient::Decor_Maximize))
          continue; // skip it
        area = &_button_max_area;
        break;
      case 'S':
        if (!(_decorations & OBClient::Decor_Sticky))
          continue; // skip it
        area = &_button_stick_area;
        break;
      case 'C':
        if (!(_decorations & OBClient::Decor_Close))
          continue; // skip it
        area = &_button_close_area;
        break;
      default:
        assert(false); // the layout string is invalid!
        continue; // just to fuck with g++
      }
      area->setX(x);
      x += sep + area->width();
    }
  }

  if (_decorations & OBClient::Decor_Handle) {
    _handle_area.setRect(0, _size.top + _client->area().height() +
                         _style->getBorderWidth(),
                         width, _style->getHandleWidth());
    _grip_left_area.setRect(0,
                            _handle_area.y() + _handle_area.height() +
                            _style->getBorderWidth(),
                            // XXX: get a Point class in otk and use that for
                            // the 'buttons size' since theyre all the same
                            _button_iconify_area.width() * 2,
                            _handle_area.height());
    _grip_right_area.setRect(((_handle_area.right() + 1) -
                              _button_iconify_area.width() * 2),
                             _handle_area.y() + _handle_area.height() +
                             _style->getBorderWidth(),
                             // XXX: get a Point class in otk and use that for
                             // the 'buttons size' since theyre all the same
                             _button_iconify_area.width() * 2,
                             _handle_area.height());
    _size.bottom += _handle_area.height() + _style->getBorderWidth() * 2;
  }
  

  // position/size all the windows

  XResizeWindow(otk::OBDisplay::display, _window,
                _size.left + _size.right + _client->area().width(),
                _size.top + _size.bottom + _client->area().height());

  XMoveWindow(otk::OBDisplay::display, _client->window(),
              _size.left, _size.top);

  if (_decorations & OBClient::Decor_Titlebar) {
    XMoveResizeWindow(otk::OBDisplay::display, _titlebar,
                      _titlebar_area.x(), _titlebar_area.y(),
                      _titlebar_area.width(), _titlebar_area.height());
    XMoveResizeWindow(otk::OBDisplay::display, _label,
                      _label_area.x(), _label_area.y(),
                      _label_area.width(), _label_area.height());
    if (_decorations & OBClient::Decor_Iconify)
      XMoveResizeWindow(otk::OBDisplay::display, _button_iconify,
                        _button_iconify_area.x(), _button_iconify_area.y(),
                        _button_iconify_area.width(),
                        _button_iconify_area.height());
    if (_decorations & OBClient::Decor_Maximize)
      XMoveResizeWindow(otk::OBDisplay::display, _button_max,
                        _button_max_area.x(), _button_max_area.y(),
                        _button_max_area.width(),
                        _button_max_area.height());
    if (_decorations & OBClient::Decor_Sticky)
      XMoveResizeWindow(otk::OBDisplay::display, _button_stick,
                        _button_stick_area.x(), _button_stick_area.y(),
                        _button_stick_area.width(),
                        _button_stick_area.height());
    if (_decorations & OBClient::Decor_Close)
      XMoveResizeWindow(otk::OBDisplay::display, _button_close,
                        _button_close_area.x(), _button_close_area.y(),
                        _button_close_area.width(),
                        _button_close_area.height());
  }

  if (_decorations & OBClient::Decor_Handle) {
    XMoveResizeWindow(otk::OBDisplay::display, _handle,
                      _handle_area.x(), _handle_area.y(),
                      _handle_area.width(), _handle_area.height());
    XMoveResizeWindow(otk::OBDisplay::display, _grip_left,
                      _grip_left_area.x(), _grip_left_area.y(),
                      _grip_left_area.width(), _grip_left_area.height());
    XMoveResizeWindow(otk::OBDisplay::display, _grip_right,
                      _grip_right_area.x(), _grip_right_area.y(),
                      _grip_right_area.width(), _grip_right_area.height());
  }

  // map/unmap all the windows
  if (_decorations & OBClient::Decor_Titlebar) {
    XMapWindow(otk::OBDisplay::display, _titlebar);
    XMapWindow(otk::OBDisplay::display, _label);
    if (_decorations & OBClient::Decor_Iconify)
      XMapWindow(otk::OBDisplay::display, _button_iconify);
    else
      XUnmapWindow(otk::OBDisplay::display, _button_iconify);
    if (_decorations & OBClient::Decor_Maximize)
      XMapWindow(otk::OBDisplay::display, _button_max);
    else
      XUnmapWindow(otk::OBDisplay::display, _button_max);
    if (_decorations & OBClient::Decor_Sticky)
      XMapWindow(otk::OBDisplay::display, _button_stick);
    else
      XUnmapWindow(otk::OBDisplay::display, _button_stick);
    if (_decorations & OBClient::Decor_Close)
      XMapWindow(otk::OBDisplay::display, _button_close);
    else
      XUnmapWindow(otk::OBDisplay::display, _button_close);
  } else {
    XUnmapWindow(otk::OBDisplay::display, _titlebar);
    XUnmapWindow(otk::OBDisplay::display, _label);
    XUnmapWindow(otk::OBDisplay::display, _button_iconify);
    XUnmapWindow(otk::OBDisplay::display, _button_max);
    XUnmapWindow(otk::OBDisplay::display, _button_stick);
    XUnmapWindow(otk::OBDisplay::display, _button_close);
  }

  if (_decorations & OBClient::Decor_Handle) {
    XMapWindow(otk::OBDisplay::display, _handle);
    XMapWindow(otk::OBDisplay::display, _grip_left);
    XMapWindow(otk::OBDisplay::display, _grip_right);
  } else {
    XUnmapWindow(otk::OBDisplay::display, _handle);
    XUnmapWindow(otk::OBDisplay::display, _grip_left);
    XUnmapWindow(otk::OBDisplay::display, _grip_right);
  }
  
  // XXX: more is gunna have to happen here

  updateShape();
}


void OBFrame::updateShape()
{
#ifdef SHAPE
  if (!_client->shaped()) {
    // clear the shape on the frame window
    XShapeCombineMask(otk::OBDisplay::display, _window, ShapeBounding,
                      _size.left - 2,//frame.margin.left - frame.border_w,
                      _size.top - 2,//frame.margin.top - frame.border_w,
                      None, ShapeSet);
  } else {
    // make the frame's shape match the clients
    XShapeCombineShape(otk::OBDisplay::display, _window, ShapeBounding,
                       _size.left - 2,
                       _size.top - 2,
                       _client->window(), ShapeBounding, ShapeSet);

  int num = 0;
    XRectangle xrect[2];

    /*
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
    }*/

    XShapeCombineRectangles(otk::OBDisplay::display, _window,
                            ShapeBounding, 0, 0, xrect, num,
                            ShapeUnion, Unsorted);
  }
#endif // SHAPE
}


void OBFrame::grabClient()
{
  
  XGrabServer(otk::OBDisplay::display);

  // select the event mask on the frame
  XSelectInput(otk::OBDisplay::display, _window, SubstructureRedirectMask);

  // reparent the client to the frame
  XSelectInput(otk::OBDisplay::display, _client->window(),
               OBClient::event_mask & ~StructureNotifyMask);
  XReparentWindow(otk::OBDisplay::display, _client->window(), _window,
                  _size.left, _size.top);
  XSelectInput(otk::OBDisplay::display, _client->window(),
               OBClient::event_mask);

  // raise the client above the frame
  XRaiseWindow(otk::OBDisplay::display, _client->window());
  // map the client so it maps when the frame does
  XMapWindow(otk::OBDisplay::display, _client->window());
  
  XUngrabServer(otk::OBDisplay::display);

  update();
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


Window OBFrame::createChild(Window parent, Cursor cursor)
{
  XSetWindowAttributes attrib_create;
  unsigned long create_mask = CWBackPixmap | CWBorderPixel | CWEventMask;

  attrib_create.background_pixmap = None;
  attrib_create.event_mask = ButtonPressMask | ButtonReleaseMask |
                             ButtonMotionMask | ExposureMask;

  if (cursor) {
    create_mask |= CWCursor;
    attrib_create.cursor = cursor;
  }

  return XCreateWindow(otk::OBDisplay::display, parent, 0, 0, 1, 1, 0,
                       _screen->getDepth(), InputOutput, _screen->getVisual(),
                       create_mask, &attrib_create);
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
