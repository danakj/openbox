// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "basewidget.hh"
#include "display.hh"
#include "assassin.hh"
#include "screeninfo.hh"

#include <algorithm>

namespace otk {

OtkBaseWidget::OtkBaseWidget(OtkBaseWidget *parent)
  : OtkEventHandler(),
    _dirty(false),
    _parent(parent), _style(parent->getStyle()),
    _cursor(parent->getCursor()), _bevel_width(parent->getBevelWidth()),
    _ignore_config(0), _visible(false), _focused(false), _grabbed_mouse(false),
    _grabbed_keyboard(false), _texture(0), _bg_pixmap(0), _bg_pixel(0),
    _screen(parent->getScreen())
{
  assert(parent);
  parent->addChild(this);
  create();
}

OtkBaseWidget::OtkBaseWidget(Style *style, Cursor cursor, int bevel_width)
  : OtkEventHandler(),
    _dirty(false), _parent(0), _style(style), _cursor(cursor),
    _bevel_width(bevel_width), _ignore_config(0), _visible(false),
    _focused(false), _grabbed_mouse(false), _grabbed_keyboard(false),
    _texture(0), _bg_pixmap(0), _bg_pixel(0), _screen(style->getScreen())
{
  assert(style);
  create();
}

OtkBaseWidget::~OtkBaseWidget()
{
  if (_visible)
    hide();

  std::for_each(_children.begin(), _children.end(), PointerAssassin());

  if (_parent)
    _parent->removeChild(this);

  XDestroyWindow(otk::OBDisplay::display, _window);
}

void OtkBaseWidget::create(void)
{
  const ScreenInfo *scr_info = otk::OBDisplay::screenInfo(_screen);
  Window p_window = _parent ? _parent->getWindow() : scr_info->getRootWindow();

  _rect.setRect(0, 0, 1, 1); // just some initial values

  XSetWindowAttributes attrib_create;
  unsigned long create_mask = CWBackPixmap | CWBorderPixel | CWEventMask;

  attrib_create.background_pixmap = None;
  attrib_create.colormap = scr_info->getColormap();
  attrib_create.event_mask = ButtonPressMask | ButtonReleaseMask |
    ButtonMotionMask | ExposureMask | StructureNotifyMask;

  if (_cursor) {
    create_mask |= CWCursor;
    attrib_create.cursor = _cursor;
  }

  _window = XCreateWindow(otk::OBDisplay::display, p_window, _rect.x(),
                          _rect.y(), _rect.width(), _rect.height(), 0,
                          scr_info->getDepth(), InputOutput,
                          scr_info->getVisual(), create_mask, &attrib_create);
  _ignore_config++;
}

void OtkBaseWidget::setWidth(int w)
{
  assert(w > 0);
  setGeometry(_rect.x(), _rect.y(), w, _rect.height());
}

void OtkBaseWidget::setHeight(int h)
{
  assert(h > 0);
  setGeometry(_rect.x(), _rect.y(), _rect.width(), h);
}

void OtkBaseWidget::move(const Point &to)
{
  move(to.x(), to.y());
}

void OtkBaseWidget::move(int x, int y)
{
  _rect.setPos(x, y);
  XMoveWindow(otk::OBDisplay::display, _window, x, y);
  _ignore_config++;
}

void OtkBaseWidget::resize(const Point &to)
{
  resize(to.x(), to.y());
}

void OtkBaseWidget::resize(int w, int h)
{
  assert(w > 0 && h > 0);
  setGeometry(_rect.x(), _rect.y(), w, h);
}

void OtkBaseWidget::setGeometry(const Rect &new_geom)
{
  setGeometry(new_geom.x(), new_geom.y(), new_geom.width(), new_geom.height());
}
 
void OtkBaseWidget::setGeometry(const Point &topleft, int width, int height)
{
  setGeometry(topleft.x(), topleft.y(), width, height);
}

void OtkBaseWidget::setGeometry(int x, int y, int width, int height)
{
  _rect = Rect(x, y, width, height);
  _dirty = true;

  XMoveResizeWindow(otk::OBDisplay::display, _window, x, y, width, height);
  _ignore_config++;
}

void OtkBaseWidget::show(bool recursive)
{
  if (_visible)
    return;

  // make sure the internal state isn't mangled
  if (_dirty)
    update();

  if (recursive) {
    OtkBaseWidgetList::iterator it = _children.begin(), end = _children.end();
    for (; it != end; ++it)
      (*it)->show();
  }

  XMapWindow(otk::OBDisplay::display, _window);
  _visible = true;
}

void OtkBaseWidget::hide(bool recursive)
{
  if (! _visible)
    return;

  if (recursive) {
    OtkBaseWidgetList::iterator it = _children.begin(), end = _children.end();
    for (; it != end; ++it)
      (*it)->hide();
  }
  
  XUnmapWindow(otk::OBDisplay::display, _window);
  _visible = false;
}

void OtkBaseWidget::focus(void)
{
  if (! _visible)
    return;

  XSetInputFocus(otk::OBDisplay::display, _window, RevertToPointerRoot,
                 CurrentTime);
}

bool OtkBaseWidget::grabMouse(void)
{
  Status ret = XGrabPointer(otk::OBDisplay::display, _window, True,
                            (ButtonPressMask | ButtonReleaseMask |
                             ButtonMotionMask | EnterWindowMask |
                             LeaveWindowMask | PointerMotionMask),
                            GrabModeSync, GrabModeAsync, None, None,
                            CurrentTime);
  _grabbed_mouse = (ret == GrabSuccess);
  return _grabbed_mouse;
}

void OtkBaseWidget::ungrabMouse(void)
{
  if (! _grabbed_mouse)
    return;

  XUngrabPointer(otk::OBDisplay::display, CurrentTime);
  _grabbed_mouse = false;
}

bool OtkBaseWidget::grabKeyboard(void)
{
  Status ret = XGrabKeyboard(otk::OBDisplay::display, _window, True,
                             GrabModeSync, GrabModeAsync, CurrentTime);
  _grabbed_keyboard = (ret == GrabSuccess);
  return _grabbed_keyboard;

}

void OtkBaseWidget::ungrabKeyboard(void)
{
  if (! _grabbed_keyboard)
    return;

  XUngrabKeyboard(otk::OBDisplay::display, CurrentTime);
  _grabbed_keyboard = false;
}

void OtkBaseWidget::render(void)
{
  if (!_texture) return;
  
  _bg_pixmap = _texture->render(_rect.width(), _rect.height(), _bg_pixmap);

  if (_bg_pixmap)
    XSetWindowBackgroundPixmap(otk::OBDisplay::display, _window, _bg_pixmap);
  else {
    unsigned int pix = _texture->color().pixel();
    if (pix != _bg_pixel) {
      _bg_pixel = pix;
      XSetWindowBackground(otk::OBDisplay::display, _window, pix);
    }
  }
}

void OtkBaseWidget::update(void)
{
  if (_dirty) {
    render();
    XClearWindow(OBDisplay::display, _window);
  }

  OtkBaseWidgetList::iterator it = _children.begin(), end = _children.end();
  for (; it != end; ++it)
    (*it)->update();

  _dirty = false;
}

void OtkBaseWidget::addChild(OtkBaseWidget *child, bool front)
{
  assert(child);
  if (front)
    _children.push_front(child);
  else
    _children.push_back(child);
}

void OtkBaseWidget::removeChild(OtkBaseWidget *child)
{
  assert(child);
  OtkBaseWidgetList::iterator it, end = _children.end();
  for (it = _children.begin(); it != end; ++it) {
    if ((*it) == child)
      break;
  }

  if (it != _children.end())
    _children.erase(it);
}

void OtkBaseWidget::setStyle(Style *style)
{
  assert(style);
  _style = style;
  _dirty = true;
  OtkBaseWidgetList::iterator it, end = _children.end();
  for (it = _children.begin(); it != end; ++it)
    (*it)->setStyle(style);
}

void OtkBaseWidget::exposeHandler(const XExposeEvent &e)
{
  OtkEventHandler::exposeHandler(e);
  _dirty = true;
  update();
}

void OtkBaseWidget::configureHandler(const XConfigureEvent &e)
{
  OtkEventHandler::configureHandler(e);
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
