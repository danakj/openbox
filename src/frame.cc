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
#include "python.hh"
#include "bindings.hh"
#include "otk/display.hh"

#include <string>

namespace ob {

const long Frame::event_mask;

Frame::Frame(Client *client, otk::RenderStyle *style)
  : otk::Widget(openbox, style, Horizontal, 0, 1, true),
    WidgetBase(WidgetBase::Type_Frame),
    _client(client),
    _screen(otk::display->screenInfo(client->screen())),
    _plate(this, WidgetBase::Type_Plate),
    _titlebar(this, WidgetBase::Type_Titlebar),
    _button_close(&_titlebar, WidgetBase::Type_CloseButton, client),
    _button_iconify(&_titlebar, WidgetBase::Type_IconifyButton, client),
    _button_max(&_titlebar, WidgetBase::Type_MaximizeButton, client),
    _button_alldesk(&_titlebar, WidgetBase::Type_AllDesktopsButton, client),
    _label(&_titlebar, WidgetBase::Type_Label),
    _handle(this, WidgetBase::Type_Handle),
    _grip_left(&_handle, WidgetBase::Type_LeftGrip, client),
    _grip_right(&_handle, WidgetBase::Type_RightGrip, client),
    _decorations(client->decorations())
{
  assert(client);
  assert(style);

  XSelectInput(**otk::display, _window, Frame::event_mask);

  _grip_left.setCursor(openbox->cursors().ll_angle);
  _grip_right.setCursor(openbox->cursors().lr_angle);
  
  _label.setText(_client->title());

  _style = 0;
  setStyle(style);

  otk::Widget::unfocus(); // stuff starts out appearing focused in otk
  
  _plate.show(); // the other stuff is shown based on decor settings
}


Frame::~Frame()
{
}


void Frame::setTitle(const otk::ustring &text)
{
  _label.setText(text);
  _label.update();
}


void Frame::setStyle(otk::RenderStyle *style)
{
  assert(style);

  // if a style was previously set, then 'replace' is true, cause we're
  // replacing a style
  bool replace = (_style);

  otk::Widget::setStyle(style);

  if (replace) {
    // XXX: do shit here whatever
  }
  
  _style = style;

  setBorderColor(_style->frameBorderColor());

  // if !replace, then adjust() will get called after the client is grabbed!
  if (replace) {
    // size/position everything
    adjustSize();
    adjustPosition();
  }
}


void Frame::focus()
{
  otk::Widget::focus();
  update();
}


void Frame::unfocus()
{
  otk::Widget::unfocus();
  update();
}


void Frame::adjust()
{
  // the party all happens in adjustSize
}


void Frame::adjustSize()
{
  // XXX: only if not overridden or something!!! MORE LOGIC HERE!!
  _decorations = _client->decorations();

  // true/false for whether to show each element of the titlebar
  bool tit_i = false, tit_m = false, tit_s = false, tit_c = false;
  int width;   // the width of the client and its border
  int bwidth;  // width to make borders
  int cbwidth; // width of the inner client border
  int fontheight = _style->labelFont()->height(); // height of the font
  int butsize = fontheight - 2; // width and height of the titlebar buttons
  const int bevel = _style->bevelWidth();
  
  if (_decorations & Client::Decor_Border) {
    bwidth = _style->frameBorderWidth();
    cbwidth = _style->clientBorderWidth();
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
  
  if (_decorations & Client::Decor_Titlebar) {
    // set the titlebar size
    _titlebar.setGeometry(-bwidth,
                          -bwidth,
                          width,
                          _style->labelFont()->height() + (bevel * 2));
    _innersize.top += _titlebar.height() + bwidth;

    // set the label size
    _label.setGeometry(0, bevel, width, fontheight);
    // set the buttons sizes
    if (_decorations & Client::Decor_Iconify)
      _button_iconify.setGeometry(0, bevel + 1, butsize, butsize);
    if (_decorations & Client::Decor_Maximize)
      _button_max.setGeometry(0, bevel + 1, butsize, butsize);
    if (_decorations & Client::Decor_AllDesktops)
      _button_alldesk.setGeometry(0, bevel + 1, butsize, butsize);
    if (_decorations & Client::Decor_Close)
      _button_close.setGeometry(0, bevel + 1, butsize, butsize);

    // separation between titlebar elements
    const int sep = bevel + 1;

    otk::ustring layout;
    if (!python_get_string("titlebar_layout", &layout))
      layout = "ILMC";

    // this code ensures that the string only has one of each possible
    // letter, all of the letters are valid, and L exists somewhere in the
    // string!
    bool tit_l = false;
  
    for (std::string::size_type i = 0; i < layout.size(); ++i) {
      switch (layout[i]) {
      case 'i':
      case 'I':
        if (!tit_i && (_decorations & Client::Decor_Iconify)) {
          tit_i = true;
          continue;
        }
        break;
      case 'l':
      case 'L':
        if (!tit_l) {
          tit_l = true;
          continue;
        }
        break;
      case 'm':
      case 'M':
        if (!tit_m && (_decorations & Client::Decor_Maximize)) {
          tit_m = true;
          continue;
        }
        break;
      case 'd':
      case 'D':
        if (!tit_s && (_decorations & Client::Decor_AllDesktops)) {
          tit_s = true;
          continue;
        }
        break;
      case 'c':
      case 'C':
        if (!tit_c && (_decorations & Client::Decor_Close)) {
          tit_c = true;
          continue;
        }
        break;
      }
      // if we get here then we don't want the letter, kill it
      layout.erase(i--, 1);
    }
    if (!tit_l)
      layout += "L";
    
    // the size of the label. this ASSUMES the layout has only buttons other
    // that the ONE LABEL!!
    // adds an extra sep so that there's a space on either side of the
    // titlebar.. note: x = sep, below.
    int lwidth = width - sep * 2 -
      (butsize + sep) * (layout.size() - 1);
    // quick sanity check for really small windows. if this is needed, its
    // obviously not going to be displayed right...
    // XXX: maybe we should make this look better somehow? constraints?
    if (lwidth <= 0) lwidth = 1;
    _label.setWidth(lwidth);

    int x = sep;
    for (std::string::size_type i = 0, len = layout.size(); i < len; ++i) {
      switch (layout[i]) {
      case 'i':
      case 'I':
        _button_iconify.move(x, _button_iconify.rect().y());
        x += _button_iconify.width();
        break;
      case 'l':
      case 'L':
        _label.move(x, _label.rect().y());
        x += _label.width();
        break;
      case 'm':
      case 'M':
        _button_max.move(x, _button_max.rect().y());
        x += _button_max.width();
        break;
      case 'd':
      case 'D':
        _button_alldesk.move(x, _button_alldesk.rect().y());
        x += _button_alldesk.width();
        break;
      case 'c':
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

  if (_decorations & Client::Decor_Handle) {
    _handle.setGeometry(-bwidth,
                        _innersize.top + _client->area().height() + cbwidth,
                        width, _style->handleWidth());
    _grip_left.setGeometry(-bwidth,
                           -bwidth,
                           butsize * 2,
                           _handle.height());
    _grip_right.setGeometry(((_handle.rect().right() + 1) -
                             butsize * 2),
                            -bwidth,
                            butsize * 2,
                            _handle.height());
    _innersize.bottom += _handle.height() + bwidth;
  }
  

  // position/size all the windows

  if (_client->shaded())
    resize(_innersize.left + _innersize.right + _client->area().width(),
           _titlebar.height());
  else
    resize(_innersize.left + _innersize.right + _client->area().width(),
           _innersize.top + _innersize.bottom + _client->area().height());

  _plate.setGeometry(_innersize.left - cbwidth, _innersize.top - cbwidth,
                     _client->area().width(), _client->area().height());

  // map/unmap all the windows
  if (_decorations & Client::Decor_Titlebar) {
    _label.show();
    if (tit_i)
      _button_iconify.show();
    else
      _button_iconify.hide();
    if (tit_m)
      _button_max.show();
    else
      _button_max.hide();
    if (tit_s)
      _button_alldesk.show();
    else
      _button_alldesk.hide();
    if (tit_c)
      _button_close.show();
    else
      _button_close.hide();
    _titlebar.show();
  } else {
    _titlebar.hide(true);
  }

  if (_decorations & Client::Decor_Handle)
    _handle.show(true);
  else
    _handle.hide(true);
  
  _size.left   = _innersize.left + bwidth;
  _size.right  = _innersize.right + bwidth;
  _size.top    = _innersize.top + bwidth;
  _size.bottom = _innersize.bottom + bwidth;

  adjustShape();

  update();
}


void Frame::adjustPosition()
{
  int x, y;
  x = _client->area().x();
  y = _client->area().y();
  clientGravity(x, y);
  move(x, y);
}


void Frame::adjustShape()
{
#ifdef SHAPE
  int bwidth = (_decorations & Client::Decor_Border) ?
    _style->frameBorderWidth() : 0;
  
  if (!_client->shaped()) {
    // clear the shape on the frame window
    XShapeCombineMask(**otk::display, _window, ShapeBounding,
                      _innersize.left,
                      _innersize.top,
                      None, ShapeSet);
  } else {
    // make the frame's shape match the clients
    XShapeCombineShape(**otk::display, _window, ShapeBounding,
                       _innersize.left,
                       _innersize.top,
                       _client->window(), ShapeBounding, ShapeSet);

    int num = 0;
    XRectangle xrect[2];

    if (_decorations & Client::Decor_Titlebar) {
      xrect[0].x = _titlebar.rect().x();
      xrect[0].y = _titlebar.rect().y();
      xrect[0].width = _titlebar.width() + bwidth * 2; // XXX: this is useless once the widget handles borders!
      xrect[0].height = _titlebar.height() + bwidth * 2;
      ++num;
    }

    if (_decorations & Client::Decor_Handle) {
      xrect[1].x = _handle.rect().x();
      xrect[1].y = _handle.rect().y();
      xrect[1].width = _handle.width() + bwidth * 2; // XXX: this is useless once the widget handles borders!
      xrect[1].height = _handle.height() + bwidth * 2;
      ++num;
    }

    XShapeCombineRectangles(**otk::display, window(),
                            ShapeBounding, 0, 0, xrect, num,
                            ShapeUnion, Unsorted);
  }
#endif // SHAPE
}


void Frame::adjustState()
{
  _button_alldesk.update();
  _button_max.update();
}


void Frame::grabClient()
{
  // reparent the client to the frame
  XReparentWindow(**otk::display, _client->window(), _plate.window(), 0, 0);
  /*
    When reparenting the client window, it is usually not mapped yet, since
    this occurs from a MapRequest. However, in the case where Openbox is
    starting up, the window is already mapped, so we'll see unmap events for
    it. There are 2 unmap events generated that we see, one with the 'event'
    member set the root window, and one set to the client, but both get handled
    and need to be ignored.
  */
  if (openbox->state() == Openbox::State_Starting)
    _client->ignore_unmaps += 2;

  // select the event mask on the client's parent (to receive config/map req's)
  XSelectInput(**otk::display, _plate.window(), SubstructureRedirectMask);

  // map the client so it maps when the frame does
  XMapWindow(**otk::display, _client->window());

  adjustSize();
  adjustPosition();
}


void Frame::releaseClient()
{
  XEvent ev;

  // check if the app has already reparented its window away
  if (XCheckTypedWindowEvent(**otk::display, _client->window(),
                             ReparentNotify, &ev)) {
    XPutBackEvent(**otk::display, &ev);
    // re-map the window since the unmanaging process unmaps it
    XMapWindow(**otk::display, _client->window());  
  } else {
    // according to the ICCCM - if the client doesn't reparent itself, then we
    // will reparent the window to root for them
    XReparentWindow(**otk::display, _client->window(),
                    _screen->rootWindow(),
                    _client->area().x(), _client->area().y());
  }
}


void Frame::clientGravity(int &x, int &y)
{
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


void Frame::frameGravity(int &x, int &y)
{
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
