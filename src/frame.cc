// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

extern "C" {
#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE
}

#include "frame.hh"
#include "openbox.hh"
#include "otk/display.hh"
#include "otk/surface.hh"

#include <string>
#include <cassert>

namespace ob {

const long Frame::event_mask;

Window createWindow(const otk::ScreenInfo *info, Window parent, 
                    unsigned long mask, XSetWindowAttributes *attrib)
{
  return XCreateWindow(**otk::display, parent, 0, 0, 1, 1, 0,
                       info->depth(), InputOutput, info->visual(),
                       mask, attrib);
                       
}

Frame::Frame(Client *client)
  : _client(client),
    _visible(false),
    _plate(0),
    _title(0),
    _label(0),
    _handle(0),
    _lgrip(0),
    _rgrip(0),
    _max(0),
    _desk(0),
    _iconify(0),
    _icon(0),
    _close(0),
    _frame_sur(0),
    _title_sur(0),
    _label_sur(0),
    _handle_sur(0),
    _grip_sur(0),
    _max_sur(0),
    _desk_sur(0),
    _iconify_sur(0),
    _icon_sur(0),
    _close_sur(0),
    _max_press(false),
    _desk_press(false),
    _iconify_press(false),
    _icon_press(false),
    _close_press(false),
    _press_button(0)
{
  assert(client);

  XSetWindowAttributes attrib;
  unsigned long mask;
  const otk::ScreenInfo *info = otk::display->screenInfo(client->screen());

  // create all of the decor windows (except title bar buttons)
  mask = CWOverrideRedirect | CWEventMask;
  attrib.event_mask = Frame::event_mask;
  attrib.override_redirect = true;
  _frame = createWindow(info, info->rootWindow(), mask, &attrib);

  mask = 0;
  _plate = createWindow(info, _frame, mask, &attrib);
  mask = CWEventMask;
  attrib.event_mask = (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
                       ExposureMask);
  _title = createWindow(info, _frame, mask, &attrib);
  _label = createWindow(info, _title, mask, &attrib);
  _max = createWindow(info, _title, mask, &attrib);
  _close = createWindow(info, _title, mask, &attrib);
  _desk = createWindow(info, _title, mask, &attrib);
  _icon = createWindow(info, _title, mask, &attrib);
  _iconify = createWindow(info, _title, mask, &attrib);
  _handle = createWindow(info, _frame, mask, &attrib);
  mask |= CWCursor;
  attrib.cursor = openbox->cursors().ll_angle;
  _lgrip = createWindow(info, _handle, mask, &attrib);
  attrib.cursor = openbox->cursors().lr_angle;
  _rgrip = createWindow(info, _handle, mask, &attrib);

  // the other stuff is shown based on decor settings
  XMapWindow(**otk::display, _plate);
  XMapWindow(**otk::display, _lgrip);
  XMapWindow(**otk::display, _rgrip);
  XMapWindow(**otk::display, _label);

  applyStyle(*otk::RenderStyle::style(_client->screen()));

  _layout = "NDITMC";

  // register all of the windows with the event dispatcher
  Window *w = allWindows();
  for (unsigned int i = 0; w[i]; ++i)
    openbox->registerHandler(w[i], this);
  delete [] w;
}

Frame::~Frame()
{
  // unregister all of the windows with the event dispatcher
  Window *w = allWindows();
  for (unsigned int i = 0; w[i]; ++i)
    openbox->clearHandler(w[i]);
  delete [] w;

  XDestroyWindow(**otk::display, _rgrip);
  XDestroyWindow(**otk::display, _lgrip);
  XDestroyWindow(**otk::display, _handle);
  XDestroyWindow(**otk::display, _max);
  XDestroyWindow(**otk::display, _icon);
  XDestroyWindow(**otk::display, _iconify);
  XDestroyWindow(**otk::display, _desk);
  XDestroyWindow(**otk::display, _close);
  XDestroyWindow(**otk::display, _label);
  XDestroyWindow(**otk::display, _title);
  XDestroyWindow(**otk::display, _frame);

  if (_frame_sur) delete _frame_sur;
  if (_title_sur) delete _title_sur;
  if (_label_sur) delete _label_sur;
  if (_handle_sur) delete _handle_sur;
  if (_grip_sur) delete _grip_sur;
  if (_max_sur) delete _max_sur;
  if (_desk_sur) delete _desk_sur;
  if (_iconify_sur) delete _iconify_sur;
  if (_icon_sur) delete _icon_sur;
  if (_close_sur) delete _close_sur;
}

void Frame::show()
{
  if (!_visible) {
    _visible = true;
    XMapWindow(**otk::display, _frame);
  }
}

void Frame::hide()
{
  if (_visible) {
    _visible = false;
    XUnmapWindow(**otk::display, _frame);
  }
}

void Frame::buttonPressHandler(const XButtonEvent &e)
{
  if (_press_button) return;
  _press_button = e.button;
  
  if (e.window == _max) {
    _max_press = true;
    renderMax();
  }
  if (e.window == _close) {
    _close_press = true;
    renderClose();
  }
  if (e.window == _desk) {
    _desk_press = true;
    renderDesk();
  }
  if (e.window == _iconify) {
    _iconify_press = true;
    renderIconify();
  }
  if (e.window == _icon) {
    _icon_press = true;
    renderIcon();
  }
}

void Frame::buttonReleaseHandler(const XButtonEvent &e)
{
  if (e.button != _press_button) return;
  _press_button = 0;

  if (e.window == _max) {
    _max_press = false;
    renderMax();
  }
  if (e.window == _close) {
    _close_press = false;
    renderClose();
  }
  if (e.window == _desk) {
    _desk_press = false;
    renderDesk();
  }
  if (e.window == _iconify) {
    _iconify_press = false;
    renderIconify();
  }
  if (e.window == _icon) {
    _icon_press = false;
    renderIcon();
  }
}

MouseContext::MC Frame::mouseContext(Window win) const
{
  if (win == _frame)  return MouseContext::Frame;
  if (win == _title ||
      win == _label)  return MouseContext::Titlebar;
  if (win == _handle) return MouseContext::Handle;
  if (win == _plate)  return MouseContext::Window;
  if (win == _lgrip ||
      win == _rgrip)  return MouseContext::Grip;
  if (win == _max)    return MouseContext::MaximizeButton;
  if (win == _close)  return MouseContext::CloseButton;
  if (win == _desk)   return MouseContext::AllDesktopsButton;
  if (win == _iconify)return MouseContext::IconifyButton;
  if (win == _icon)   return MouseContext::IconButton;
  return (MouseContext::MC) -1;
}

Window *Frame::allWindows() const
{
  Window *w = new Window[12 + 1];
  unsigned int i = 0;
  w[i++] = _frame;
  w[i++] = _plate;
  w[i++] = _title;
  w[i++] = _label;
  w[i++] = _handle;
  w[i++] = _lgrip;
  w[i++] = _rgrip;
  w[i++] = _max;
  w[i++] = _desk;
  w[i++] = _close;
  w[i++] = _icon;
  w[i++] = _iconify;
  w[i] = 0;
  return w;
}

void Frame::applyStyle(const otk::RenderStyle &style)
{
  // set static border colors
  XSetWindowBorder(**otk::display, _frame, style.frameBorderColor()->pixel());
  XSetWindowBorder(**otk::display, _title, style.frameBorderColor()->pixel());
  XSetWindowBorder(**otk::display, _handle, style.frameBorderColor()->pixel());
  XSetWindowBorder(**otk::display, _lgrip, style.frameBorderColor()->pixel());
  XSetWindowBorder(**otk::display, _rgrip, style.frameBorderColor()->pixel());

  // size all the fixed-size elements
  geom.font_height = style.labelFont()->height();
  if (geom.font_height < 1) geom.font_height = 1;
  geom.button_size = geom.font_height - 2;
  if (geom.button_size < 1) geom.button_size = 1;
  geom.handle_height = style.handleWidth();
  if (geom.handle_height < 1) geom.handle_height = 1;
  geom.bevel = style.bevelWidth();
  
  XResizeWindow(**otk::display, _lgrip, geom.grip_width(), geom.handle_height);
  XResizeWindow(**otk::display, _rgrip, geom.grip_width(), geom.handle_height);
  
  XResizeWindow(**otk::display, _max, geom.button_size, geom.button_size);
  XResizeWindow(**otk::display, _close, geom.button_size, geom.button_size);
  XResizeWindow(**otk::display, _desk, geom.button_size, geom.button_size);
  XResizeWindow(**otk::display, _iconify, geom.button_size, geom.button_size);
  XResizeWindow(**otk::display, _icon, geom.button_size, geom.button_size);
}

void Frame::styleChanged(const otk::RenderStyle &style)
{
  applyStyle(style);
  
  // size/position everything
  adjustSize();
  adjustPosition();
}

void Frame::adjustFocus()
{
  // XXX optimizations later...
  adjustSize();
}

void Frame::adjustTitle()
{
  // XXX optimizations later...
  adjustSize();
}

static void render(int screen, const otk::Size &size, Window win,
                   otk::Surface **surface,
                   const otk::RenderTexture &texture, bool freedata=true)
{
  otk::Surface *s = new otk::Surface(screen, size);
  otk::display->renderControl(screen)->drawBackground(*s, texture);
  XSetWindowBackgroundPixmap(**otk::display, win, s->pixmap());
  XClearWindow(**otk::display, win);
  if (*surface) delete *surface;
  if (freedata) s->freePixelData();
  *surface = s;
}

void Frame::adjustSize()
{
  _decorations = _client->decorations();
  const otk::RenderStyle *style = otk::RenderStyle::style(_client->screen());

  if (_decorations & Client::Decor_Border) {
    geom.bwidth = style->frameBorderWidth();
    geom.cbwidth = style->clientBorderWidth();
  } else {
    geom.bwidth = geom.cbwidth = 0;
  }
  _innersize.left = _innersize.top = _innersize.bottom = _innersize.right =
    geom.cbwidth;
  geom.width = _client->area().width() + geom.cbwidth * 2;
  assert(geom.width > 0);

  // set border widths
  XSetWindowBorderWidth(**otk::display, _plate, geom.cbwidth);
  XSetWindowBorderWidth(**otk::display, _frame, geom.bwidth);
  XSetWindowBorderWidth(**otk::display, _title, geom.bwidth);
  XSetWindowBorderWidth(**otk::display, _handle, geom.bwidth);
  XSetWindowBorderWidth(**otk::display, _lgrip, geom.bwidth);
  XSetWindowBorderWidth(**otk::display, _rgrip, geom.bwidth);
  
  // position/size and map/unmap all the windows

  if (_decorations & Client::Decor_Titlebar) {
    XMoveResizeWindow(**otk::display, _title, -geom.bwidth, -geom.bwidth,
                      geom.width, geom.title_height());
    _innersize.top += geom.title_height() + geom.bwidth;
    XMapWindow(**otk::display, _title);

    // layout the title bar elements
    layoutTitle();
  } else {
    XUnmapWindow(**otk::display, _title);
    // make all the titlebar stuff not render
    _decorations &= ~(Client::Decor_Icon | Client::Decor_Iconify |
                      Client::Decor_Maximize | Client::Decor_Close |
                      Client::Decor_AllDesktops);
  }

  if (_decorations & Client::Decor_Handle) {
    geom.handle_y = _innersize.top + _client->area().height() + geom.cbwidth;
    XMoveResizeWindow(**otk::display, _handle, -geom.bwidth, geom.handle_y,
                      geom.width, geom.handle_height);
    XMoveWindow(**otk::display, _lgrip, -geom.bwidth, -geom.bwidth);
    XMoveWindow(**otk::display, _rgrip,
                -geom.bwidth + geom.width - geom.grip_width(),
                -geom.bwidth);
    _innersize.bottom += geom.handle_height + geom.bwidth;
    XMapWindow(**otk::display, _handle);
  } else
    XUnmapWindow(**otk::display, _handle);
  
  XResizeWindow(**otk::display, _frame, geom.width,
                (_client->shaded() ? geom.title_height() :
                 _innersize.top + _innersize.bottom +
                 _client->area().height()));

  // do this in two steps because clients whose gravity is set to
  // 'Static' don't end up getting moved at all with an XMoveResizeWindow
  XMoveWindow(**otk::display, _plate, _innersize.left - geom.cbwidth,
              _innersize.top - geom.cbwidth);
  XResizeWindow(**otk::display, _plate, _client->area().width(),
                _client->area().height());

  _size.left   = _innersize.left + geom.bwidth;
  _size.right  = _innersize.right + geom.bwidth;
  _size.top    = _innersize.top + geom.bwidth;
  _size.bottom = _innersize.bottom + geom.bwidth;

  _area = otk::Rect(_area.position(), otk::Size(_client->area().width() +
                                                _size.left + _size.right,
                                                _client->area().height() +
                                                _size.top + _size.bottom));

  // render all the elements
  int screen = _client->screen();
  bool focus = _client->focused();
  if (_decorations & Client::Decor_Titlebar) {
    render(screen, otk::Size(geom.width, geom.title_height()), _title,
           &_title_sur, *(focus ? style->titlebarFocusBackground() :
                          style->titlebarUnfocusBackground()), false);
    
    renderLabel();
    renderMax();
    renderDesk();
    renderIconify();
    renderIcon();
    renderClose();
  }

  if (_decorations & Client::Decor_Handle) {
    render(screen, otk::Size(geom.width, geom.handle_height), _handle,
           &_handle_sur, *(focus ? style->handleFocusBackground() :
                           style->handleUnfocusBackground()));
    render(screen, otk::Size(geom.grip_width(), geom.handle_height), _lgrip,
           &_grip_sur, *(focus ? style->gripFocusBackground() :
                         style->gripUnfocusBackground()));
    XSetWindowBackgroundPixmap(**otk::display, _rgrip, _grip_sur->pixmap());
    XClearWindow(**otk::display, _rgrip);
  }

  XSetWindowBorder(**otk::display, _plate,
                   focus ? style->clientBorderFocusColor()->pixel() :
                   style->clientBorderUnfocusColor()->pixel());
  
  adjustShape();
}

void Frame::renderLabel()
{
  const otk::RenderStyle *style = otk::RenderStyle::style(_client->screen());
  const otk::RenderControl *control =
    otk::display->renderControl(_client->screen());
  const otk::Font *font = style->labelFont();

  otk::Surface *s = new otk::Surface(_client->screen(),
                                     otk::Size(geom.label_width,
                                               geom.label_height()));
  control->drawBackground(*s, *(_client->focused() ?
                                style->labelFocusBackground() :
                                style->labelUnfocusBackground()));

  otk::ustring t = _client->title(); // the actual text to draw
  int x = geom.bevel;                // x coord for the text

  if (x * 2 > geom.label_width) return; // no room at all

  // find a string that will fit inside the area for text
  otk::ustring::size_type text_len = t.size();
  int length;
  int maxsize = geom.label_width - geom.bevel * 2;
      
  do {
    t.resize(text_len);
    length = font->measureString(t);  // this returns an unsigned, so check < 0
    if (length < 0) length = maxsize; // if the string's that long just adjust
  } while (length > maxsize && text_len-- > 0);

  if (text_len <= 0) return; // won't fit anything

  // justify the text
  switch (style->labelTextJustify()) {
  case otk::RenderStyle::RightBottomJustify:
    x += maxsize - length;
    break;
  case otk::RenderStyle::CenterJustify:
    x += (maxsize - length) / 2;
    break;
  case otk::RenderStyle::LeftTopJustify:
    break;
  }
 
  control->drawString(*s, *font, x, 0,
                      *(_client->focused() ? style->textFocusColor() :
                        style->textUnfocusColor()), t);

  XSetWindowBackgroundPixmap(**otk::display, _label, s->pixmap());
  XClearWindow(**otk::display, _label);
  if (_label_sur) delete _label_sur;
  s->freePixelData();
  _label_sur = s;
}

static void renderButton(int screen, bool focus, bool press, Window win,
                         otk::Surface **sur, int butsize,
                         const otk::PixmapMask *mask)
{
  const otk::RenderStyle *style = otk::RenderStyle::style(screen);
  const otk::RenderControl *control = otk::display->renderControl(screen);
  otk::Surface *s = new otk::Surface(screen, otk::Size(butsize, butsize));

  const otk::RenderTexture *tx = (focus ?
                                  (press ?
                                   style->buttonPressFocusBackground() :
                                   style->buttonUnpressFocusBackground()) :
                                  (press ?
                                   style->buttonPressUnfocusBackground() :
                                   style->buttonUnpressUnfocusBackground()));
  const otk::RenderColor *maskcolor = (focus ?
                                       style->buttonFocusColor() :
                                       style->buttonUnfocusColor());
  control->drawBackground(*s, *tx);
  control->drawMask(*s, *maskcolor, *mask);

  XSetWindowBackgroundPixmap(**otk::display, win, s->pixmap());
  XClearWindow(**otk::display, win);
  if (*sur) delete *sur;
  s->freePixelData();
  *sur = s;
}

void Frame::renderMax()
{
  if (!(_decorations & Client::Decor_Maximize)) return;
  bool press = _max_press || _client->maxVert() || _client->maxHorz();
  renderButton(_client->screen(), _client->focused(), press, _max,
               &_max_sur, geom.button_size,
               otk::RenderStyle::style(_client->screen())->maximizeMask());
}

void Frame::renderDesk()
{
  if (!(_decorations & Client::Decor_AllDesktops)) return;
  bool press = _desk_press || _client->desktop() == 0xffffffff;
  renderButton(_client->screen(), _client->focused(), press, _desk,
               &_desk_sur, geom.button_size,
               otk::RenderStyle::style(_client->screen())->alldesktopsMask());
}

void Frame::renderIconify()
{
  if (!(_decorations & Client::Decor_Iconify)) return;
  renderButton(_client->screen(), _client->focused(), _iconify_press, _iconify,
               &_iconify_sur, geom.button_size,
               otk::RenderStyle::style(_client->screen())->iconifyMask());
}

void Frame::renderClose()
{
  if (!(_decorations & Client::Decor_Close)) return;
  renderButton(_client->screen(), _client->focused(), _close_press, _close,
               &_close_sur, geom.button_size,
               otk::RenderStyle::style(_client->screen())->closeMask());
}

void Frame::renderIcon()
{
  if (!(_decorations & Client::Decor_Icon)) return;
  const int screen = _client->screen();
  const otk::RenderControl *control = otk::display->renderControl(screen);

  otk::Surface *s = new otk::Surface(screen, otk::Size(geom.button_size,
                                                       geom.button_size));
  otk::pixel32 *dest = s->pixelData(), *src;
  int w = _title_sur->size().width();
  
  src = _title_sur->pixelData() + w * (geom.bevel + 1) + geom.icon_x;
  
  // get the background under the icon button
  for (int y = 0; y < geom.button_size; ++y, src += w - geom.button_size)
    for (int x = 0; x < geom.button_size; ++x, ++dest, ++src)
      *dest = *src;
  // draw the icon over it
  const Icon *icon = _client->icon(otk::Size(geom.button_size,
                                             geom.button_size));
  control->drawImage(*s, icon->w, icon->h, icon->data);

  XSetWindowBackgroundPixmap(**otk::display, _icon, s->pixmap());
  XClearWindow(**otk::display, _icon);
  if (_icon_sur) delete _icon_sur;
  _icon_sur = s;
}

void Frame::layoutTitle()
{
  // figure out whats being shown, and the width of the label
  geom.label_width = geom.width - geom.bevel * 2;
  bool n, d, i, t, m ,c;
  n = d = i = t = m = c = false;
  for (const char *l = _layout.c_str(); *l; ++l) {
    switch (*l) {
    case 'n':
    case 'N':
      if (!(_decorations & Client::Decor_Icon)) break;
      n = true;
      geom.label_width -= geom.button_size + geom.bevel;
      break;
    case 'd':
    case 'D':
      if (!(_decorations & Client::Decor_AllDesktops)) break;
      d = true;
      geom.label_width -= geom.button_size + geom.bevel;
      break;
    case 'i':
    case 'I':
      if (!(_decorations & Client::Decor_Iconify)) break;
      i = true;
      geom.label_width -= geom.button_size + geom.bevel;
      break;
    case 't':
    case 'T':
      t = true;
      break;
    case 'm':
    case 'M':
      if (!(_decorations & Client::Decor_Maximize)) break;
      m = true;
      geom.label_width -= geom.button_size + geom.bevel;
      break;
    case 'c':
    case 'C':
      if (!(_decorations & Client::Decor_Close)) break;
      c = true;
      geom.label_width -= geom.button_size + geom.bevel;
      break;
    }
  }
  if (geom.label_width < 1) geom.label_width = 1;

  XResizeWindow(**otk::display, _label, geom.label_width, geom.font_height);
  
  if (!n) {
    _decorations &= ~Client::Decor_Icon;
    XUnmapWindow(**otk::display, _icon);
  }
  if (!d) {
    _decorations &= ~Client::Decor_AllDesktops;
    XUnmapWindow(**otk::display, _desk);
  }
  if (!i) {
    _decorations &= ~Client::Decor_Iconify;
    XUnmapWindow(**otk::display, _iconify);
  }
  if (!t)
    XUnmapWindow(**otk::display, _label);
  if (!m) {
    _decorations &= ~Client::Decor_Maximize;
    XUnmapWindow(**otk::display, _max);
  }
  if (!c) {
    _decorations &= ~Client::Decor_Close;
    XUnmapWindow(**otk::display, _close);
  }

  int x = geom.bevel;
  for (const char *lc = _layout.c_str(); *lc; ++lc) {
    switch (*lc) {
    case 'n':
    case 'N':
      if (!n) break;
      geom.icon_x = x;
      XMapWindow(**otk::display, _icon);
      XMoveWindow(**otk::display, _icon, x, geom.bevel + 1);
      x += geom.button_size;
      break;
    case 'd':
    case 'D':
      if (!d) break;
      XMapWindow(**otk::display, _desk);
      XMoveWindow(**otk::display, _desk, x, geom.bevel + 1);
      x += geom.button_size;
      break;
    case 'i':
    case 'I':
      if (!i) break;
      XMapWindow(**otk::display, _iconify);
      XMoveWindow(**otk::display, _iconify, x, geom.bevel + 1);
      x += geom.button_size;
      break;
    case 't':
    case 'T':
      if (!t) break;
      XMapWindow(**otk::display, _label);
      XMoveWindow(**otk::display, _label, x, geom.bevel);
      x += geom.label_width;
      break;
    case 'm':
    case 'M':
      if (!m) break;
      XMapWindow(**otk::display, _max);
      XMoveWindow(**otk::display, _max, x, geom.bevel + 1);
      x += geom.button_size;
      break;
    case 'c':
    case 'C':
      if (!c) break;
      XMapWindow(**otk::display, _close);
      XMoveWindow(**otk::display, _close, x, geom.bevel + 1);
      x += geom.button_size;
      break;
    }
    x += geom.bevel;
  }
}

void Frame::adjustPosition()
{
  int x, y;
  x = _client->area().x();
  y = _client->area().y();
  clientGravity(x, y);
  XMoveWindow(**otk::display, _frame, x, y);
  _area = otk::Rect(otk::Point(x, y), _area.size());
}


void Frame::adjustShape()
{
#ifdef SHAPE
  if (!_client->shaped()) {
    // clear the shape on the frame window
    XShapeCombineMask(**otk::display, _frame, ShapeBounding,
                      _innersize.left,
                      _innersize.top,
                      None, ShapeSet);
  } else {
    // make the frame's shape match the clients
    XShapeCombineShape(**otk::display, _frame, ShapeBounding,
                       _innersize.left,
                       _innersize.top,
                       _client->window(), ShapeBounding, ShapeSet);

    int num = 0;
    XRectangle xrect[2];

    if (_decorations & Client::Decor_Titlebar) {
      xrect[0].x = -geom.bevel;
      xrect[0].y = -geom.bevel;
      xrect[0].width = geom.width + geom.bwidth * 2;
      xrect[0].height = geom.title_height() + geom.bwidth * 2;
      ++num;
    }

    if (_decorations & Client::Decor_Handle) {
      xrect[1].x = -geom.bevel;
      xrect[1].y = geom.handle_y;
      xrect[1].width = geom.width + geom.bwidth * 2;
      xrect[1].height = geom.handle_height + geom.bwidth * 2;
      ++num;
    }

    XShapeCombineRectangles(**otk::display, _frame,
                            ShapeBounding, 0, 0, xrect, num,
                            ShapeUnion, Unsorted);
  }
#endif // SHAPE
}

void Frame::adjustState()
{
  renderDesk();
  renderMax();
}

void Frame::adjustIcon()
{
  renderIcon();
}

void Frame::grabClient()
{
  // reparent the client to the frame
  XReparentWindow(**otk::display, _client->window(), _plate, 0, 0);
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
  XSelectInput(**otk::display, _plate, SubstructureRedirectMask);

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
                    otk::display->screenInfo(_client->screen())->rootWindow(),
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
