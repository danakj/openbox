// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "widget.hh"
#include "display.hh"
#include "assassin.hh"
#include "screeninfo.hh"

#include <algorithm>
#include <iostream>

namespace otk {

Widget::Widget(Widget *parent, Direction direction)
  : EventHandler(),
    _dirty(false), _focused(false),
    _parent(parent), _style(parent->style()), _direction(direction),
    _cursor(parent->cursor()), _bevel_width(parent->bevelWidth()),
    _ignore_config(0),
    _visible(false), _grabbed_mouse(false),
    _grabbed_keyboard(false), _stretchable_vert(false),
    _stretchable_horz(false), _texture(0), _bg_pixmap(0), _bg_pixel(0),
    _bcolor(0), _bwidth(0), _screen(parent->screen()), _fixed_width(false),
    _fixed_height(false), _event_dispatcher(parent->eventDispatcher())
{
  assert(parent);
  parent->addChild(this);
  create();
  _event_dispatcher->registerHandler(_window, this);
  setStyle(_style); // let the widget initialize stuff
}

Widget::Widget(EventDispatcher *event_dispatcher, Style *style,
                     Direction direction, Cursor cursor, int bevel_width,
                     bool override_redirect)
  : EventHandler(),
    _dirty(false),_focused(false),
    _parent(0), _style(style), _direction(direction), _cursor(cursor),
    _bevel_width(bevel_width), _ignore_config(0), _visible(false),
    _grabbed_mouse(false), _grabbed_keyboard(false),
    _stretchable_vert(false), _stretchable_horz(false), _texture(0),
    _bg_pixmap(0), _bg_pixel(0), _bcolor(0), _bwidth(0),
    _screen(style->getScreen()), _fixed_width(false), _fixed_height(false),
    _event_dispatcher(event_dispatcher)
{
  assert(event_dispatcher);
  assert(style);
  create(override_redirect);
  _event_dispatcher->registerHandler(_window, this);
  setStyle(_style); // let the widget initialize stuff
}

Widget::~Widget()
{
  if (_visible)
    hide();

  _event_dispatcher->clearHandler(_window);

  std::for_each(_children.begin(), _children.end(), PointerAssassin());

  if (_parent)
    _parent->removeChild(this);

  XDestroyWindow(Display::display, _window);
}

void Widget::create(bool override_redirect)
{
  const ScreenInfo *scr_info = Display::screenInfo(_screen);
  Window p_window = _parent ? _parent->window() : scr_info->rootWindow();

  _rect.setRect(0, 0, 1, 1); // just some initial values

  XSetWindowAttributes attrib_create;
  unsigned long create_mask = CWBackPixmap | CWBorderPixel | CWEventMask;

  attrib_create.background_pixmap = None;
  attrib_create.colormap = scr_info->colormap();
  attrib_create.event_mask = ButtonPressMask | ButtonReleaseMask |
    ButtonMotionMask | ExposureMask | StructureNotifyMask;

  if (override_redirect) {
    create_mask |= CWOverrideRedirect;
    attrib_create.override_redirect = true;
  }

  if (_cursor) {
    create_mask |= CWCursor;
    attrib_create.cursor = _cursor;
  }

  _window = XCreateWindow(Display::display, p_window, _rect.x(),
                          _rect.y(), _rect.width(), _rect.height(), 0,
                          scr_info->depth(), InputOutput,
                          scr_info->visual(), create_mask, &attrib_create);
  _ignore_config++;
}

void Widget::setWidth(int w)
{
  assert(w > 0);
  _fixed_width = true;  
  setGeometry(_rect.x(), _rect.y(), w, _rect.height());
}

void Widget::setHeight(int h)
{
  assert(h > 0);
  _fixed_height = true;
  setGeometry(_rect.x(), _rect.y(), _rect.width(), h);
}

void Widget::move(const Point &to)
{
  move(to.x(), to.y());
}

void Widget::move(int x, int y)
{
  _rect.setPos(x, y);
  XMoveWindow(Display::display, _window, x, y);
  _ignore_config++;
}

void Widget::resize(const Point &to)
{
  resize(to.x(), to.y());
}

void Widget::resize(int w, int h)
{
  assert(w > 0 && h > 0);
  _fixed_width = _fixed_height = true;
  setGeometry(_rect.x(), _rect.y(), w, h);
}

void Widget::setGeometry(const Rect &new_geom)
{
  setGeometry(new_geom.x(), new_geom.y(), new_geom.width(), new_geom.height());
}
 
void Widget::setGeometry(const Point &topleft, int width, int height)
{
  setGeometry(topleft.x(), topleft.y(), width, height);
}

void Widget::setGeometry(int x, int y, int width, int height)
{
  _rect = Rect(x, y, width, height);
  _dirty = true;

  XMoveResizeWindow(Display::display, _window, x, y, width, height);
  _ignore_config++;
}

void Widget::show(bool recursive)
{
  if (_visible)
    return;

  // make sure the internal state isn't mangled
  if (_dirty)
    update();

  if (recursive) {
    WidgetList::iterator it = _children.begin(), end = _children.end();
    for (; it != end; ++it)
      (*it)->show();
  }

  XMapWindow(Display::display, _window);
  _visible = true;
}

void Widget::hide(bool recursive)
{
  if (! _visible)
    return;

  if (recursive) {
    WidgetList::iterator it = _children.begin(), end = _children.end();
    for (; it != end; ++it)
      (*it)->hide();
  }
  
  XUnmapWindow(Display::display, _window);
  _visible = false;
}

void Widget::focus(void)
{
  _focused = true;
  
  Widget::WidgetList::iterator it = _children.begin(),
    end = _children.end();
  for (; it != end; ++it)
    (*it)->focus();
}

void Widget::unfocus(void)
{
  _focused = false;
  
  Widget::WidgetList::iterator it = _children.begin(),
    end = _children.end();
  for (; it != end; ++it)
    (*it)->unfocus();
}

bool Widget::grabMouse(void)
{
  Status ret = XGrabPointer(Display::display, _window, True,
                            (ButtonPressMask | ButtonReleaseMask |
                             ButtonMotionMask | EnterWindowMask |
                             LeaveWindowMask | PointerMotionMask),
                            GrabModeSync, GrabModeAsync, None, None,
                            CurrentTime);
  _grabbed_mouse = (ret == GrabSuccess);
  return _grabbed_mouse;
}

void Widget::ungrabMouse(void)
{
  if (! _grabbed_mouse)
    return;

  XUngrabPointer(Display::display, CurrentTime);
  _grabbed_mouse = false;
}

bool Widget::grabKeyboard(void)
{
  Status ret = XGrabKeyboard(Display::display, _window, True,
                             GrabModeSync, GrabModeAsync, CurrentTime);
  _grabbed_keyboard = (ret == GrabSuccess);
  return _grabbed_keyboard;

}

void Widget::ungrabKeyboard(void)
{
  if (! _grabbed_keyboard)
    return;

  XUngrabKeyboard(Display::display, CurrentTime);
  _grabbed_keyboard = false;
}

void Widget::render(void)
{
  if (!_texture) return;

  _bg_pixmap = _texture->render(_rect.width(), _rect.height(), _bg_pixmap);

  if (_bg_pixmap) {
    XSetWindowBackgroundPixmap(Display::display, _window, _bg_pixmap);
    _bg_pixel = None;
  } else {
    unsigned int pix = _texture->color().pixel();
    if (pix != _bg_pixel) {
      _bg_pixel = pix;
      XSetWindowBackground(Display::display, _window, pix);
    }
  }
}

void Widget::adjust(void)
{
  if (_direction == Horizontal)
    adjustHorz();
  else
    adjustVert();
}

void Widget::adjustHorz(void)
{
  if (_children.size() == 0)
    return;

  Widget *tmp;
  WidgetList::iterator it, end = _children.end();

  int tallest = 0;
  int width = _bevel_width;
  WidgetList stretchable;

  for (it = _children.begin(); it != end; ++it) {
    tmp = *it;
    if (tmp->isStretchableVert())
      tmp->setHeight(_rect.height() > _bevel_width * 2 ?
                     _rect.height() - _bevel_width * 2 : _bevel_width);
    if (tmp->isStretchableHorz())
      stretchable.push_back(tmp);
    else
      width += tmp->_rect.width() + _bevel_width;

    if (tmp->_rect.height() > tallest)
      tallest = tmp->_rect.height();
  }

  if (stretchable.size() > 0) {
    WidgetList::iterator str_it = stretchable.begin(),
      str_end = stretchable.end();

    int str_width = _rect.width() - width / stretchable.size();

    for (; str_it != str_end; ++str_it)
      (*str_it)->setWidth(str_width > _bevel_width ? str_width - _bevel_width
                          : _bevel_width);
  }

  Widget *prev_widget = 0;

  for (it = _children.begin(); it != end; ++it) {
    tmp = *it;
    int x, y;

    if (prev_widget)
      x = prev_widget->_rect.x() + prev_widget->_rect.width() + _bevel_width;
    else
      x = _rect.x() + _bevel_width;
    y = (tallest - tmp->_rect.height()) / 2 + _bevel_width;

    tmp->move(x, y);

    prev_widget = tmp;
  }

  internalResize(width, tallest + _bevel_width * 2);
}

void Widget::adjustVert(void)
{
  if (_children.size() == 0)
    return;

  Widget *tmp;
  WidgetList::iterator it, end = _children.end();

  int widest = 0;
  int height = _bevel_width;
  WidgetList stretchable;

  for (it = _children.begin(); it != end; ++it) {
    tmp = *it;
    if (tmp->isStretchableHorz())
      tmp->setWidth(_rect.width() > _bevel_width * 2 ?
                    _rect.width() - _bevel_width * 2 : _bevel_width);
    if (tmp->isStretchableVert())
      stretchable.push_back(tmp);
    else
      height += tmp->_rect.height() + _bevel_width;

    if (tmp->_rect.width() > widest)
      widest = tmp->_rect.width();
  }

  if (stretchable.size() > 0) {
    WidgetList::iterator str_it = stretchable.begin(),
      str_end = stretchable.end();

    int str_height = _rect.height() - height / stretchable.size();

    for (; str_it != str_end; ++str_it)
      (*str_it)->setHeight(str_height > _bevel_width ?
                           str_height - _bevel_width : _bevel_width);
  }

  Widget *prev_widget = 0;

  for (it = _children.begin(); it != end; ++it) {
    tmp = *it;
    int x, y;

    if (prev_widget)
      y = prev_widget->_rect.y() + prev_widget->_rect.height() + _bevel_width;
    else
      y = _rect.y() + _bevel_width;
    x = (widest - tmp->_rect.width()) / 2 + _bevel_width;

    tmp->move(x, y);

    prev_widget = tmp;
  }

  internalResize(widest + _bevel_width * 2, height);
}

void Widget::update(void)
{
  if (_dirty) {
    adjust();
    render();
    XClearWindow(Display::display, _window);
  }

  WidgetList::iterator it = _children.begin(), end = _children.end();
  for (; it != end; ++it)
    (*it)->update();

  _dirty = false;
}

void Widget::internalResize(int w, int h)
{
  assert(w > 0 && h > 0);

  if (! _fixed_width && ! _fixed_height)
    resize(w, h);
  else if (! _fixed_width)
    resize(w, _rect.height());
  else if (! _fixed_height)
    resize(_rect.width(), h);
}

void Widget::addChild(Widget *child, bool front)
{
  assert(child);
  if (front)
    _children.push_front(child);
  else
    _children.push_back(child);
}

void Widget::removeChild(Widget *child)
{
  assert(child);
  WidgetList::iterator it, end = _children.end();
  for (it = _children.begin(); it != end; ++it) {
    if ((*it) == child)
      break;
  }

  if (it != _children.end())
    _children.erase(it);
}

void Widget::setStyle(Style *style)
{
  assert(style);
  _style = style;
  _dirty = true;

  WidgetList::iterator it, end = _children.end();
  for (it = _children.begin(); it != end; ++it)
    (*it)->setStyle(style);
}


void Widget::setEventDispatcher(EventDispatcher *disp)
{
  if (_event_dispatcher)
    _event_dispatcher->clearHandler(_window);
  _event_dispatcher = disp;
  _event_dispatcher->registerHandler(_window, this);
}

void Widget::exposeHandler(const XExposeEvent &e)
{
  EventHandler::exposeHandler(e);
  _dirty = true;
  update();
}

void Widget::configureHandler(const XConfigureEvent &e)
{
  EventHandler::configureHandler(e);
  if (_ignore_config) {
    _ignore_config--;
  } else {
    if (!(e.width == _rect.width() && e.height == _rect.height())) {
      _dirty = true;
      _rect.setSize(e.width, e.height);
    }
    update();
  }
}

}
