// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

extern "C" {
#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE
}

#include "openbox.hh"
#include "frame.hh"
#include "client.hh"
#include "otk/display.hh"

#include <string>
#include <iostream> // TEMP

namespace ob {

OBFrame::OBFrame(OBClient *client, otk::Style *style)
  : otk::OtkWidget(Openbox::instance, style),
    _client(client),
    _screen(otk::OBDisplay::screenInfo(client->screen())),
    _plate(this),
    _titlebar(this),
    _button_close(&_titlebar),
    _button_iconify(&_titlebar),
    _button_max(&_titlebar),
    _button_stick(&_titlebar),
    _label(&_titlebar),
    _handle(this),
    _grip_left(&_handle),
    _grip_right(&_handle),
    _decorations(client->decorations())
{
  assert(client);
  assert(style);

  unmanaged();
  _titlebar.unmanaged();
  _button_close.unmanaged();
  _button_iconify.unmanaged();
  _button_max.unmanaged();
  _button_stick.unmanaged();
  _label.unmanaged();
  _handle.unmanaged();
  _grip_left.unmanaged();
  _grip_right.unmanaged();
  _plate.unmanaged();

  _plate.show();

  _button_close.setText("X");
  _button_iconify.setText("I");
  _button_max.setText("M");
  _button_stick.setText("S");
  _label.setText(_client->title());

  _style = 0;
  setStyle(style);

  grabClient();
}


OBFrame::~OBFrame()
{
  releaseClient(false);
}


void OBFrame::setStyle(otk::Style *style)
{
  assert(style);

  otk::OtkWidget::setStyle(style);
  // set the grips' textures
  _grip_left.setTexture(style->getGripFocus());
  _grip_left.setUnfocusTexture(style->getGripUnfocus());
  _grip_left.setPressedFocusTexture(style->getGripFocus());
  _grip_left.setPressedUnfocusTexture(style->getGripUnfocus());
  _grip_right.setTexture(style->getGripFocus());
  _grip_right.setUnfocusTexture(style->getGripUnfocus());
  _grip_right.setPressedFocusTexture(style->getGripFocus());
  _grip_right.setPressedUnfocusTexture(style->getGripUnfocus());

  _titlebar.setTexture(style->getTitleFocus());
  _titlebar.setUnfocusTexture(style->getTitleUnfocus());
  _handle.setTexture(style->getHandleFocus());
  _handle.setUnfocusTexture(style->getHandleUnfocus());
  
  // if a style was previously set, then 'replace' is true, cause we're
  // replacing a style
  bool replace = (_style);

  if (replace) {
    // XXX: do shit here whatever
    // XXX: save the position based on gravity
  }
  
  _style = style;

  // XXX: change when focus changes!
  XSetWindowBorder(otk::OBDisplay::display, _plate.getWindow(),
                   _style->getFrameFocus()->color().pixel());

  XSetWindowBorder(otk::OBDisplay::display, getWindow(),
                   _style->getBorderColor()->pixel());
  XSetWindowBorder(otk::OBDisplay::display, _titlebar.getWindow(),
                   _style->getBorderColor()->pixel());
  XSetWindowBorder(otk::OBDisplay::display, _grip_left.getWindow(),
                   _style->getBorderColor()->pixel());
  XSetWindowBorder(otk::OBDisplay::display, _grip_right.getWindow(),
                   _style->getBorderColor()->pixel());
  XSetWindowBorder(otk::OBDisplay::display, _handle.getWindow(),
                   _style->getBorderColor()->pixel());
  
  // if !replace, then adjust() will get called after the client is grabbed!
  if (replace)
    adjust(); // size/position everything
}


void OBFrame::adjust()
{
  // XXX: only if not overridden or something!!! MORE LOGIC HERE!!
  _decorations = _client->decorations();
  _decorations = 0xffffffff;
  
  int width;   // the width of the whole frame
  int bwidth;  // width to make borders
  int cbwidth; // width of the inner client border
  
  if (_decorations & OBClient::Decor_Border) {
    bwidth = _style->getBorderWidth();
    cbwidth = _style->getFrameWidth();
  } else
    bwidth = cbwidth = 0;
  _size.left = _size.top = _size.bottom = _size.right = cbwidth;
  width = _client->area().width() + cbwidth * 2;

  XSetWindowBorderWidth(otk::OBDisplay::display, _plate.getWindow(), cbwidth);

  XSetWindowBorderWidth(otk::OBDisplay::display, getWindow(), bwidth);
  XSetWindowBorderWidth(otk::OBDisplay::display, _titlebar.getWindow(),
                        bwidth);
  XSetWindowBorderWidth(otk::OBDisplay::display, _grip_left.getWindow(),
                        bwidth);
  XSetWindowBorderWidth(otk::OBDisplay::display, _grip_right.getWindow(),
                        bwidth);
  XSetWindowBorderWidth(otk::OBDisplay::display, _handle.getWindow(), bwidth);

  if (_decorations & OBClient::Decor_Titlebar) {
    // set the titlebar size
    _titlebar.setGeometry(-bwidth,
                          -bwidth,
                          width,
                          (_style->getFont().height() +
                           _style->getBevelWidth() * 2));
    _size.top += _titlebar.height() + bwidth;

    // set the label size
    _label.setGeometry(0, _style->getBevelWidth(),
                       width, _style->getFont().height());
    // set the buttons sizes
    if (_decorations & OBClient::Decor_Iconify)
      _button_iconify.setGeometry(0, _style->getBevelWidth() + 1,
                                  _label.height() - 2,
                                  _label.height() - 2);
    if (_decorations & OBClient::Decor_Maximize)
      _button_max.setGeometry(0, _style->getBevelWidth() + 1,
                              _label.height() - 2,
                              _label.height() - 2);
    if (_decorations & OBClient::Decor_Sticky)
      _button_stick.setGeometry(0, _style->getBevelWidth() + 1,
                                _label.height() - 2,
                                _label.height() - 2);
    if (_decorations & OBClient::Decor_Close)
      _button_close.setGeometry(0, _style->getBevelWidth() + 1,
                                _label.height() - 2,
                                _label.height() - 2);

    // separation between titlebar elements
    const int sep = _style->getBevelWidth() + 1;

    std::string layout = "SLIMC"; // XXX: get this from somewhere
    // XXX: it is REQUIRED that by this point, the string only has one of each
    // possible letter, all of the letters are valid, and L exists somewhere in
    // the string!

    // the size of the label. this ASSUMES the layout has only buttons other
    // that the ONE LABEL!!
    // adds an extra sep so that there's a space on either side of the
    // titlebar.. note: x = sep, below.
    _label.setWidth(width - sep * 2 - 
                    (_button_iconify.width() + sep) * (layout.size() - 1));

    int x = sep;
    for (int i = 0, len = layout.size(); i < len; ++i) {
      switch (layout[i]) {
      case 'I':
        _button_iconify.move(x, _button_iconify.getRect().y());
        x += _button_iconify.width();
        break;
      case 'L':
        _label.move(x, _label.getRect().y());
        x += _label.width();
        break;
      case 'M':
        _button_max.move(x, _button_max.getRect().y());
        x += _button_max.width();
        break;
      case 'S':
        _button_stick.move(x, _button_stick.getRect().y());
        x += _button_stick.width();
        break;
      case 'C':
        _button_close.move(x, _button_close.getRect().y());
        x += _button_close.width();
        break;
      default:
        assert(false); // the layout string is invalid!
      }
      x += sep;
    }
  }

  if (_decorations & OBClient::Decor_Handle) {
    _handle.setGeometry(-bwidth,
                        _size.top + _client->area().height() + cbwidth,
                        width, _style->getHandleWidth());
    _grip_left.setGeometry(-bwidth,
                           -bwidth,
                           // XXX: get a Point class in otk and use that for
                           // the 'buttons size' since theyre all the same
                           _button_iconify.width() * 2,
                           _handle.height());
    _grip_right.setGeometry(((_handle.getRect().right() + 1) -
                             _button_iconify.width() * 2),
                            -bwidth,
                            // XXX: get a Point class in otk and use that for
                            // the 'buttons size' since theyre all the same
                            _button_iconify.width() * 2,
                            _handle.height());
    _size.bottom += _handle.height() + bwidth;
  }
  

  // position/size all the windows

  resize(_size.left + _size.right + _client->area().width(),
         _size.top + _size.bottom + _client->area().height());

  _plate.setGeometry(_size.left - cbwidth, _size.top - cbwidth,
                     _client->area().width(), _client->area().height());

  // map/unmap all the windows
  if (_decorations & OBClient::Decor_Titlebar) {
    _label.show();
    if (_decorations & OBClient::Decor_Iconify)
      _button_iconify.show();
    else
      _button_iconify.hide();
    if (_decorations & OBClient::Decor_Maximize)
      _button_max.show();
    else
      _button_max.hide();
    if (_decorations & OBClient::Decor_Sticky)
      _button_stick.show();
    else
      _button_stick.hide();
    if (_decorations & OBClient::Decor_Close)
      _button_close.show();
    else
      _button_close.hide();
    _titlebar.show();
  } else {
    _titlebar.hide(true);
  }

  if (_decorations & OBClient::Decor_Handle)
    _handle.show(true);
  else
    _handle.hide(true);
  
  // XXX: more is gunna have to happen here

  adjustShape();

  update();
}


void OBFrame::adjustShape()
{
#ifdef SHAPE
  if (!_client->shaped()) {
    // clear the shape on the frame window
    XShapeCombineMask(otk::OBDisplay::display, getWindow(), ShapeBounding,
                      _size.left,
                      _size.top,
                      None, ShapeSet);
  } else {
    // make the frame's shape match the clients
    XShapeCombineShape(otk::OBDisplay::display, getWindow(), ShapeBounding,
                       _size.left,
                       _size.top,
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

    XShapeCombineRectangles(otk::OBDisplay::display, getWindow(),
                            ShapeBounding, 0, 0, xrect, num,
                            ShapeUnion, Unsorted);
  }
#endif // SHAPE
}


void OBFrame::grabClient()
{
  
  // select the event mask on the frame
  //XSelectInput(otk::OBDisplay::display, _window, SubstructureRedirectMask);

  // reparent the client to the frame
  XReparentWindow(otk::OBDisplay::display, _client->window(),
                  _plate.getWindow(), 0, 0);
  _client->ignore_unmaps++;

  // raise the client above the frame
  //XRaiseWindow(otk::OBDisplay::display, _client->window());
  // map the client so it maps when the frame does
  XMapWindow(otk::OBDisplay::display, _client->window());
  
  adjust();
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

  Window w = XCreateWindow(otk::OBDisplay::display, parent, 0, 0, 1, 1, 0,
                           _screen->getDepth(), InputOutput,
                           _screen->getVisual(), create_mask, &attrib_create);
  return w;
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
                       0, 0, 1, 1, 0,
                       _screen->getDepth(), InputOutput, _screen->getVisual(),
                       create_mask, &attrib_create);
}

}
