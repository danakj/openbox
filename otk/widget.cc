// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"
#include "widget.hh"
#include "display.hh"
#include "surface.hh"
#include "rendertexture.hh"
#include "rendercolor.hh"
#include "eventdispatcher.hh"
#include "screeninfo.hh"

#include <climits>
#include <cassert>
#include <algorithm>

namespace otk {

Widget::Widget(int screen, EventDispatcher *ed, Direction direction, int bevel,
               bool overrideredir)
  : _texture(0),
    _screen(screen),
    _parent(0),
    _window(0),
    _surface(0),
    _event_mask(ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
                ExposureMask | StructureNotifyMask),
    _alignment(RenderStyle::CenterJustify),
    _direction(direction),
    _max_size(UINT_MAX, UINT_MAX),
    _visible(false),
    _bordercolor(0),
    _borderwidth(0),
    _bevel(bevel),
    _dirty(true),
    _dispatcher(ed),
    _ignore_config(0)
{
  createWindow(overrideredir);
  _dispatcher->registerHandler(_window, this);
}

Widget::Widget(Widget *parent, Direction direction, int bevel)
  : _texture(0),
    _screen(parent->_screen),
    _parent(parent),
    _window(0),
    _surface(0),
    _event_mask(ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
                ExposureMask | StructureNotifyMask),
    _alignment(RenderStyle::CenterJustify),
    _direction(direction),
    _max_size(UINT_MAX, UINT_MAX),
    _visible(false),
    _bordercolor(0),
    _borderwidth(0),
    _bevel(bevel),
    _dirty(true),
    _dispatcher(parent->_dispatcher),
    _ignore_config(0)
{
  assert(parent);
  createWindow(false);
  parent->addChild(this);
  parent->layout();
  _dispatcher->registerHandler(_window, this);
}

Widget::~Widget()
{
  assert(_children.empty()); // this would be bad. theyd have a hanging _parent
  
  if (_surface) delete _surface;
  if (_parent) _parent->removeChild(this);

  _dispatcher->clearHandler(_window);
  XDestroyWindow(**display, _window);
}

void Widget::show(bool children)
{
  if (children) {
    std::list<Widget*>::iterator it , end = _children.end();
    for (it = _children.begin(); it != end; ++it)
      (*it)->show(true);
  }
  if (!_visible) {
    _visible = true;
    XMapWindow(**display, _window);
    update();
  }
}

void Widget::hide()
{
  if (_visible) {
    _visible = false;
    XUnmapWindow(**display, _window);
    if (_parent) _parent->layout();
  }
} 

void Widget::setEventMask(long e)
{
  XSelectInput(**display, _window, e);
  _event_mask = e;
}

void Widget::update()
{
  _dirty = true;
  if (parent())
    parent()->layout(); // relay-out us and our siblings
  else {
    render();
    layout();
  }
}

void Widget::moveresize(const Rect &r)
{
  unsigned int w, h;
  w = std::min(std::max(r.width(), minSize().width()), maxSize().width());
  h = std::min(std::max(r.height(), minSize().height()), maxSize().height());

  if (r.x() == area().x() && r.y() == area().y() &&
      w == area().width() && h == area().height()) {
    return; // no change, don't cause a big layout chain to occur!
  }
  
  internal_moveresize(r.x(), r.y(), w, h);

  update();
}

void Widget::internal_moveresize(int x, int y, unsigned w, unsigned int h)
{
  assert(w > 0);
  assert(h > 0);
  assert(_borderwidth >= 0);
  _dirty = true;
  XMoveResizeWindow(**display, _window, x, y,
                    w - _borderwidth * 2,
                    h - _borderwidth * 2);
  _ignore_config++;

  _area = Rect(x, y, w, h);
}

void Widget::setAlignment(RenderStyle::Justify a)
{
  _alignment = a;  
  layout();
}
  
void Widget::createWindow(bool overrideredir)
{
  const ScreenInfo *info = display->screenInfo(_screen);
  XSetWindowAttributes attrib;
  unsigned long mask = CWEventMask | CWBorderPixel;

  attrib.event_mask = _event_mask;
  attrib.border_pixel = (_bordercolor ?
                         _bordercolor->pixel():
                         BlackPixel(**display, _screen));

  if (overrideredir) {
    mask |= CWOverrideRedirect;
    attrib.override_redirect = true;
  }
  
  _window = XCreateWindow(**display, (_parent ?
                                      _parent->_window :
                                      RootWindow(**display, _screen)),
                          _area.x(), _area.y(),
                          _area.width(), _area.height(),
                          _borderwidth,
                          info->depth(),
                          InputOutput,
                          info->visual(),
                          mask,
                          &attrib);
  assert(_window != None);
  ++_ignore_config;
}

void Widget::setBorderWidth(int w)
{
  assert(w >= 0);
  if (!parent()) return; // top-level windows cannot have borders
  if (w == borderWidth()) return; // no change
  
  _borderwidth = w;
  XSetWindowBorderWidth(**display, _window, _borderwidth);

  calcDefaultSizes();
  update();
}

void Widget::setMinSize(const Size &s)
{
  _min_size = s;
  update();
}

void Widget::setMaxSize(const Size &s)
{
  _max_size = s;
  update();
}

void Widget::setBorderColor(const RenderColor *c)
{
  _bordercolor = c;
  XSetWindowBorder(**otk::display, _window,
                   c ? c->pixel() : BlackPixel(**otk::display, _screen));
}

void Widget::setBevel(int b)
{
  _bevel = b;
  calcDefaultSizes();
  layout();
}

void Widget::layout()
{
  if (_direction == Horizontal)
    layoutHorz();
  else
    layoutVert();
}

void Widget::layoutHorz()
{
  std::list<Widget*>::iterator it, end;

  // work with just the visible children
  std::list<Widget*> visible = _children;
  for (it = visible.begin(), end = visible.end(); it != end;) {
    std::list<Widget*>::iterator next = it; ++next;
    if (!(*it)->visible())
      visible.erase(it);
    it = next;
  }

  if (visible.empty()) return;

  if ((unsigned)(_borderwidth * 2 + _bevel * 2) > _area.width() ||
      (unsigned)(_borderwidth * 2 + _bevel * 2) > _area.height())
    return; // not worth laying anything out!
  
  int x, y; unsigned int w, h; // working area
  x = y = _bevel;
  w = _area.width() - _borderwidth * 2 - _bevel * 2;
  h = _area.height() - _borderwidth * 2 - _bevel * 2;

  int free = w - (visible.size() - 1) * _bevel;
  if (free < 0) free = 0;
  unsigned int each;
  
  std::list<Widget*> adjustable = visible;

  // find the 'free' space, and how many children will be using it
  for (it = adjustable.begin(), end = adjustable.end(); it != end;) {
    std::list<Widget*>::iterator next = it; ++next;
    free -= (*it)->minSize().width();
    if (free < 0) free = 0;
    if ((*it)->maxSize().width() - (*it)->minSize().width() <= 0)
      adjustable.erase(it);
    it = next;
  }
  // some widgets may have max widths that restrict them, find the 'true'
  // amount of free space after these widgets are not included
  if (!adjustable.empty()) {
    do {
      each = free / adjustable.size();
      for (it = adjustable.begin(), end = adjustable.end(); it != end;) {
        std::list<Widget*>::iterator next = it; ++next;
        unsigned int m = (*it)->maxSize().width() - (*it)->minSize().width();
        if (m > 0 && m < each) {
          free -= m;
          if (free < 0) free = 0;
          adjustable.erase(it);
          break; // if one is found to be fixed, then the free space needs to
                 // change, and the rest need to be reexamined
        }
        it = next;
      }
    } while (it != end && !adjustable.empty());
  }

  // place/size the widgets
  if (!adjustable.empty())
    each = free / adjustable.size();
  else
    each = 0;
  for (it = visible.begin(), end = visible.end(); it != end; ++it) {
    unsigned int w;
    // is the widget adjustable?
    std::list<Widget*>::const_iterator
      found = std::find(adjustable.begin(), adjustable.end(), *it);
    if (found != adjustable.end()) {
      // adjustable
      w = (*it)->minSize().width() + each;
    } else {
      // fixed
      w = (*it)->minSize().width();
    }
    // align it vertically
    int yy = y;
    unsigned int hh = std::max(std::min(h, (*it)->_max_size.height()),
                               (*it)->_min_size.height());
    if (hh < h) {
      switch(_alignment) {
      case RenderStyle::RightBottomJustify:
        yy += h - hh;
        break;
      case RenderStyle::CenterJustify:
        yy += (h - hh) / 2;
        break;
      case RenderStyle::LeftTopJustify:
        break;
      }
    }
    (*it)->internal_moveresize(x, yy, w, hh);
    (*it)->render();
    (*it)->layout();
    x += w + _bevel;
  }
}

void Widget::layoutVert()
{
  std::list<Widget*>::iterator it, end;

  // work with just the visible children
  std::list<Widget*> visible = _children;
  for (it = visible.begin(), end = visible.end(); it != end;) {
    std::list<Widget*>::iterator next = it; ++next;
    if (!(*it)->visible())
      visible.erase(it);
    it = next;
  }

  if (visible.empty()) return;

  if ((unsigned)(_borderwidth * 2 + _bevel * 2) > _area.width() ||
      (unsigned)(_borderwidth * 2 + _bevel * 2) > _area.height())
    return; // not worth laying anything out!
  
  int x, y; unsigned int w, h; // working area
  x = y = _bevel;
  w = _area.width() - _borderwidth * 2 - _bevel * 2;
  h = _area.height() - _borderwidth * 2 - _bevel * 2;

  int free = h - (visible.size() - 1) * _bevel;
  if (free < 0) free = 0;
  unsigned int each;

  std::list<Widget*> adjustable = visible;

  // find the 'free' space, and how many children will be using it
  for (it = adjustable.begin(), end = adjustable.end(); it != end;) {
    std::list<Widget*>::iterator next = it; ++next;
    free -= (*it)->minSize().height();
    if (free < 0) free = 0;
    if ((*it)->maxSize().height() - (*it)->minSize().height() <= 0)
      adjustable.erase(it);
    it = next;
  }
  // some widgets may have max heights that restrict them, find the 'true'
  // amount of free space after these widgets are not included
  if (!adjustable.empty()) {
    do {
      each = free / adjustable.size();
      for (it = adjustable.begin(), end = adjustable.end(); it != end;) {
        std::list<Widget*>::iterator next = it; ++next;
        unsigned int m = (*it)->maxSize().height() - (*it)->minSize().height();
        if (m > 0 && m < each) {
          free -= m;
          if (free < 0) free = 0;
          adjustable.erase(it);
          break; // if one is found to be fixed, then the free space needs to
                 // change, and the rest need to be reexamined
        }
        it = next;
      }
    } while (it != end && !adjustable.empty());
  }

  // place/size the widgets
  if (!adjustable.empty())
  each = free / adjustable.size();
  else
    each = 0;
  for (it = visible.begin(), end = visible.end(); it != end; ++it) {
    unsigned int h;
    // is the widget adjustable?
    std::list<Widget*>::const_iterator
      found = std::find(adjustable.begin(), adjustable.end(), *it);
    if (found != adjustable.end()) {
      // adjustable
      h = (*it)->minSize().height() + each;
    } else {
      // fixed
      h = (*it)->minSize().height();
    }
    // align it horizontally
    int xx = x;
    unsigned int ww = std::max(std::min(w, (*it)->_max_size.width()),
                               (*it)->_min_size.width());
    if (ww < w) {
      switch(_alignment) {
      case RenderStyle::RightBottomJustify:
        xx += w - ww;
        break;
      case RenderStyle::CenterJustify:
        xx += (w - ww) / 2;
        break;
      case RenderStyle::LeftTopJustify:
        break;
      }
    }
    (*it)->internal_moveresize(xx, y, ww, h);
    (*it)->render();
    (*it)->layout();
    y += h + _bevel; 
  }
}

void Widget::render()
{
  if (!_texture || !_dirty) return;
  if ((unsigned)_borderwidth * 2 > _area.width() ||
      (unsigned)_borderwidth * 2 > _area.height())
    return; // no surface to draw on
  
  Surface *s = new Surface(_screen, Size(_area.width() - _borderwidth * 2,
                                         _area.height() - _borderwidth * 2));
  display->renderControl(_screen)->drawBackground(*s, *_texture);

  renderForeground(*s); // for inherited types to render onto the _surface

  XSetWindowBackgroundPixmap(**display, _window, s->pixmap());
  XClearWindow(**display, _window);

  // delete the old surface *after* its pixmap isn't in use anymore
  if (_surface) delete _surface;

  _surface = s;

  _dirty = false;
}

void Widget::renderChildren()
{
  std::list<Widget*>::iterator it, end = _children.end();
  for (it = _children.begin(); it != end; ++it)
    (*it)->render();
}

void Widget::exposeHandler(const XExposeEvent &e)
{
  EventHandler::exposeHandler(e);
  XClearArea(**display, _window, e.x, e.y, e.width, e.height, false);
}

void Widget::configureHandler(const XConfigureEvent &e)
{
  if (_ignore_config) {
    _ignore_config--;
  } else {
    XEvent ev;
    ev.xconfigure.width = e.width;
    ev.xconfigure.height = e.height;
    while (XCheckTypedWindowEvent(**display, window(), ConfigureNotify, &ev));

    if (!((unsigned)ev.xconfigure.width == area().width() &&
          (unsigned)ev.xconfigure.height == area().height())) {
      _area = Rect(_area.position(), Size(e.width, e.height));
      update();
    }
  }
}

}
