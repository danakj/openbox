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

namespace ob {

const long OBFrame::event_mask;

OBFrame::OBFrame(OBClient *client, otk::Style *style)
  : otk::OtkWidget(Openbox::instance, style),
    OBWidget(Type_Frame),
    _client(client),
    _screen(otk::OBDisplay::screenInfo(client->screen())),
    _plate(this, OBWidget::Type_Plate),
    _titlebar(this, OBWidget::Type_Titlebar),
    _button_close(&_titlebar, OBWidget::Type_CloseButton),
    _button_iconify(&_titlebar, OBWidget::Type_IconifyButton),
    _button_max(&_titlebar, OBWidget::Type_MaximizeButton),
    _button_stick(&_titlebar, OBWidget::Type_StickyButton),
    _label(&_titlebar, OBWidget::Type_Label),
    _handle(this, OBWidget::Type_Handle),
    _grip_left(&_handle, OBWidget::Type_LeftGrip),
    _grip_right(&_handle, OBWidget::Type_RightGrip),
    _decorations(client->decorations())
{
  assert(client);
  assert(style);

  XSelectInput(otk::OBDisplay::display, window(), OBFrame::event_mask);
  
  _grip_left.setCursor(Openbox::instance->cursors().ll_angle);
  _grip_right.setCursor(Openbox::instance->cursors().lr_angle);
  
  _label.setText(_client->title());

  _style = 0;
  setStyle(style);

  otk::OtkWidget::unfocus(); // stuff starts out appearing focused in otk
  
  _plate.show(); // the other stuff is shown based on decor settings
  
  grabClient();
}


OBFrame::~OBFrame()
{
  releaseClient(false);
}


void OBFrame::setTitle(const std::string &text)
{
  _label.setText(text);
  _label.update();
}


void OBFrame::setStyle(otk::Style *style)
{
  assert(style);

  otk::OtkWidget::setStyle(style);

  // if a style was previously set, then 'replace' is true, cause we're
  // replacing a style
  bool replace = (_style);

  if (replace) {
    // XXX: do shit here whatever
  }
  
  _style = style;

  setBorderColor(_style->getBorderColor());

  // if !replace, then adjust() will get called after the client is grabbed!
  if (replace) {
    // size/position everything
    adjustSize();
    adjustPosition();
  }
}


void OBFrame::focus()
{
  otk::OtkWidget::focus();
  update();
}


void OBFrame::unfocus()
{
  otk::OtkWidget::unfocus();
  update();
}


void OBFrame::adjust()
{
}


void OBFrame::adjustSize()
{
  // XXX: only if not overridden or something!!! MORE LOGIC HERE!!
  _decorations = _client->decorations();
  _decorations = 0xffffffff;
  
  int width;   // the width of the client and its border
  int bwidth;  // width to make borders
  int cbwidth; // width of the inner client border
  
  if (_decorations & OBClient::Decor_Border) {
    bwidth = _style->getBorderWidth();
    cbwidth = _style->getFrameWidth();
  } else
    bwidth = cbwidth = 0;
  _innersize.left = _innersize.top = _innersize.bottom = _innersize.right =
    cbwidth;
  width = _client->area().width() + cbwidth * 2;

  _plate.setBorderWidth(cbwidth);

  setBorderWidth(bwidth);
  _titlebar.setBorderWidth(bwidth);
  _grip_left.setBorderWidth(bwidth);
  _grip_right.setBorderWidth(bwidth);
  _handle.setBorderWidth(bwidth);
  
  if (_decorations & OBClient::Decor_Titlebar) {
    // set the titlebar size
    _titlebar.setGeometry(-bwidth,
                          -bwidth,
                          width,
                          (_style->getFont()->height() +
                           _style->getBevelWidth() * 2));
    _innersize.top += _titlebar.height() + bwidth;

    // set the label size
    _label.setGeometry(0, _style->getBevelWidth(),
                       width, _style->getFont()->height());
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
    int lwidth = width - sep * 2 -
      (_button_iconify.width() + sep) * (layout.size() - 1);
    // quick sanity check for really small windows. if this is needed, its
    // obviously not going to be displayed right...
    // XXX: maybe we should make this look better somehow? constraints?
    if (lwidth <= 0) lwidth = 1;
    _label.setWidth(lwidth);

    int x = sep;
    for (int i = 0, len = layout.size(); i < len; ++i) {
      switch (layout[i]) {
      case 'I':
        _button_iconify.move(x, _button_iconify.rect().y());
        x += _button_iconify.width();
        break;
      case 'L':
        _label.move(x, _label.rect().y());
        x += _label.width();
        break;
      case 'M':
        _button_max.move(x, _button_max.rect().y());
        x += _button_max.width();
        break;
      case 'S':
        _button_stick.move(x, _button_stick.rect().y());
        x += _button_stick.width();
        break;
      case 'C':
        _button_close.move(x, _button_close.rect().y());
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
                        _innersize.top + _client->area().height() + cbwidth,
                        width, _style->getHandleWidth());
    _grip_left.setGeometry(-bwidth,
                           -bwidth,
                           // XXX: get a Point class in otk and use that for
                           // the 'buttons size' since theyre all the same
                           _button_iconify.width() * 2,
                           _handle.height());
    _grip_right.setGeometry(((_handle.rect().right() + 1) -
                             _button_iconify.width() * 2),
                            -bwidth,
                            // XXX: get a Point class in otk and use that for
                            // the 'buttons size' since theyre all the same
                            _button_iconify.width() * 2,
                            _handle.height());
    _innersize.bottom += _handle.height() + bwidth;
  }
  

  // position/size all the windows

  resize(_innersize.left + _innersize.right + _client->area().width(),
         _innersize.top + _innersize.bottom + _client->area().height());

  _plate.setGeometry(_innersize.left - cbwidth, _innersize.top - cbwidth,
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

  _size.left   = _innersize.left + bwidth;
  _size.right  = _innersize.right + bwidth;
  _size.top    = _innersize.top + bwidth;
  _size.bottom = _innersize.bottom + bwidth;

  adjustShape();

  update();
}


void OBFrame::adjustPosition()
{
  int x, y;
  clientGravity(x, y);
  move(x, y);
}


void OBFrame::adjustShape()
{
#ifdef SHAPE
  int bwidth = (_decorations & OBClient::Decor_Border) ?
    _style->getBorderWidth() : 0;
  
  if (!_client->shaped()) {
    // clear the shape on the frame window
    XShapeCombineMask(otk::OBDisplay::display, window(), ShapeBounding,
                      _innersize.left,
                      _innersize.top,
                      None, ShapeSet);
  } else {
    // make the frame's shape match the clients
    XShapeCombineShape(otk::OBDisplay::display, window(), ShapeBounding,
                       _innersize.left,
                       _innersize.top,
                       _client->window(), ShapeBounding, ShapeSet);

    int num = 0;
    XRectangle xrect[2];

    if (_decorations & OBClient::Decor_Titlebar) {
      xrect[0].x = _titlebar.rect().x();
      xrect[0].y = _titlebar.rect().y();
      xrect[0].width = _titlebar.width() + bwidth * 2; // XXX: this is useless once the widget handles borders!
      xrect[0].height = _titlebar.height() + bwidth * 2;
      ++num;
    }

    if (_decorations & OBClient::Decor_Handle) {
      xrect[1].x = _handle.rect().x();
      xrect[1].y = _handle.rect().y();
      xrect[1].width = _handle.width() + bwidth * 2; // XXX: this is useless once the widget handles borders!
      xrect[1].height = _handle.height() + bwidth * 2;
      ++num;
    }

    XShapeCombineRectangles(otk::OBDisplay::display, window(),
                            ShapeBounding, 0, 0, xrect, num,
                            ShapeUnion, Unsorted);
  }
#endif // SHAPE
}


void OBFrame::grabClient()
{
  
  // reparent the client to the frame
  XReparentWindow(otk::OBDisplay::display, _client->window(),
                  _plate.window(), 0, 0);
  _client->ignore_unmaps++;

  // select the event mask on the client's parent (to receive config req's)
  XSelectInput(otk::OBDisplay::display, _plate.window(),
               SubstructureRedirectMask);

  // map the client so it maps when the frame does
  XMapWindow(otk::OBDisplay::display, _client->window());

  adjustSize();
  adjustPosition();
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
                    _screen->rootWindow(),
                    _client->area().x(), _client->area().y());
  }

  // if we want to remap the window, do so now
  if (remap)
    XMapWindow(otk::OBDisplay::display, _client->window());
}


void OBFrame::clientGravity(int &x, int &y)
{
  x = _client->area().x();
  y = _client->area().y();

  // horizontal
  switch (_client->gravity()) {
  default:
  case NorthWestGravity:
  case SouthWestGravity:
  case WestGravity:
    break;

  case NorthGravity:
  case SouthGravity:
  case CenterGravity:
    x -= (_size.left + _size.right) / 2;
    break;

  case NorthEastGravity:
  case SouthEastGravity:
  case EastGravity:
    x -= _size.left + _size.right;
    break;

  case ForgetGravity:
  case StaticGravity:
    x -= _size.left;
    break;
  }

  // vertical
  switch (_client->gravity()) {
  default:
  case NorthWestGravity:
  case NorthEastGravity:
  case NorthGravity:
    break;

  case CenterGravity:
  case EastGravity:
  case WestGravity:
    y -= (_size.top + _size.bottom) / 2;
    break;

  case SouthWestGravity:
  case SouthEastGravity:
  case SouthGravity:
    y -= _size.top + _size.bottom;
    break;

  case ForgetGravity:
  case StaticGravity:
    y -= _size.top;
    break;
  }
}


void OBFrame::frameGravity(int &x, int &y)
{
  x = rect().x();
  y = rect().y();
  
  // horizontal
  switch (_client->gravity()) {
  default:
  case NorthWestGravity:
  case WestGravity:
  case SouthWestGravity:
    break;
  case NorthGravity:
  case CenterGravity:
  case SouthGravity:
    x += (_size.left + _size.right) / 2;
    break;
  case NorthEastGravity:
  case EastGravity:
  case SouthEastGravity:
    x += _size.left + _size.right;
    break;
  case StaticGravity:
  case ForgetGravity:
    x += _size.left;
    break;
  }

  // vertical
  switch (_client->gravity()) {
  default:
  case NorthWestGravity:
  case WestGravity:
  case SouthWestGravity:
    break;
  case NorthGravity:
  case CenterGravity:
  case SouthGravity:
    y += (_size.top + _size.bottom) / 2;
    break;
  case NorthEastGravity:
  case EastGravity:
  case SouthEastGravity:
    y += _size.top + _size.bottom;
    break;
  case StaticGravity:
  case ForgetGravity:
    y += _size.top;
    break;
  }
}


}
