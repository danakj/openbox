#include "widget.hh"
#include "display.hh"
#include "assassin.hh"
#include "screeninfo.hh"

#include <algorithm>
#include <iostream>

namespace otk {

OtkWidget::OtkWidget(OtkWidget *parent, Direction direction)
  : _parent(parent), _style(parent->getStyle()), _direction(direction),
    _cursor(parent->getCursor()), _bevel_width(parent->getBevelWidth()),
    _ignore_config(0),
    _visible(false), _focused(false), _grabbed_mouse(false),
    _grabbed_keyboard(false), _stretchable_vert(false),
    _stretchable_horz(false), _texture(0), _bg_pixmap(0), _bg_pixel(0),
    _screen(parent->getScreen()), _fixed_width(false), _fixed_height(false),
    _dirty(false)
{
  parent->addChild(this);
  create();
}

OtkWidget::OtkWidget(Style *style, Direction direction,
                     Cursor cursor, int bevel_width)
  : _parent(0), _style(style), _direction(direction), _cursor(cursor),
    _bevel_width(bevel_width), _ignore_config(0), _visible(false),
    _focused(false), _grabbed_mouse(false), _grabbed_keyboard(false),
    _stretchable_vert(false), _stretchable_horz(false), _texture(0),
    _bg_pixmap(0), _bg_pixel(0), _screen(style->getScreen()),
    _fixed_width(false), _fixed_height(false), _dirty(false)
{
  assert(style);
  create();
}

OtkWidget::~OtkWidget()
{
  if (_visible)
    hide();

  std::for_each(_children.begin(), _children.end(), PointerAssassin());

  if (_parent)
    _parent->removeChild(this);

  XDestroyWindow(otk::OBDisplay::display, _window);
}

void OtkWidget::create(void)
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

void OtkWidget::setWidth(int w)
{
  assert(w > 0);
  _fixed_width = true;  
  setGeometry(_rect.x(), _rect.y(), w, _rect.height());
}

void OtkWidget::setHeight(int h)
{
  assert(h > 0);
  _fixed_height = true;
  setGeometry(_rect.x(), _rect.y(), _rect.width(), h);
}

void OtkWidget::move(const Point &to)
{
  move(to.x(), to.y());
}

void OtkWidget::move(int x, int y)
{
  _rect.setPos(x, y);
  XMoveWindow(otk::OBDisplay::display, _window, x, y);
  _ignore_config++;
}

void OtkWidget::resize(const Point &to)
{
  resize(to.x(), to.y());
}

void OtkWidget::resize(int w, int h)
{
  assert(w > 0 && h > 0);
  _fixed_width = _fixed_height = true;
  setGeometry(_rect.x(), _rect.y(), w, h);
}

void OtkWidget::setGeometry(const Rect &new_geom)
{
  setGeometry(new_geom.x(), new_geom.y(), new_geom.width(), new_geom.height());
}
 
void OtkWidget::setGeometry(const Point &topleft, int width, int height)
{
  setGeometry(topleft.x(), topleft.y(), width, height);
}

void OtkWidget::setGeometry(int x, int y, int width, int height)
{
  _rect = Rect(x, y, width, height);
  _dirty = true;

  XMoveResizeWindow(otk::OBDisplay::display, _window, x, y, width, height);
  _ignore_config++;
}

void OtkWidget::show(void)
{
  if (_visible)
    return;

  // make sure the internal state isn't mangled
  if (_dirty)
    update();

  OtkWidgetList::iterator it = _children.begin(), end = _children.end();
  for (; it != end; ++it)
    (*it)->show();

  XMapWindow(otk::OBDisplay::display, _window);
  _visible = true;
}

void OtkWidget::hide(void)
{
  if (! _visible)
    return;

  OtkWidgetList::iterator it = _children.begin(), end = _children.end();
  for (; it != end; ++it)
    (*it)->hide();

  XUnmapWindow(otk::OBDisplay::display, _window);
  _visible = false;
}

void OtkWidget::focus(void)
{
  if (! _visible)
    return;

  XSetInputFocus(otk::OBDisplay::display, _window, RevertToPointerRoot,
                 CurrentTime);
}

bool OtkWidget::grabMouse(void)
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

void OtkWidget::ungrabMouse(void)
{
  if (! _grabbed_mouse)
    return;

  XUngrabPointer(otk::OBDisplay::display, CurrentTime);
  _grabbed_mouse = false;
}

bool OtkWidget::grabKeyboard(void)
{
  Status ret = XGrabKeyboard(otk::OBDisplay::display, _window, True,
                             GrabModeSync, GrabModeAsync, CurrentTime);
  _grabbed_keyboard = (ret == GrabSuccess);
  return _grabbed_keyboard;

}

void OtkWidget::ungrabKeyboard(void)
{
  if (! _grabbed_keyboard)
    return;

  XUngrabKeyboard(otk::OBDisplay::display, CurrentTime);
  _grabbed_keyboard = false;
}

void OtkWidget::render(void)
{
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

void OtkWidget::adjust(void)
{
  if (_direction == Horizontal)
    adjustHorz();
  else
    adjustVert();
}

void OtkWidget::adjustHorz(void)
{
  if (_children.size() == 0)
    return;

  OtkWidget *tmp;
  OtkWidgetList::iterator it, end = _children.end();

  int tallest = 0;
  int width = _bevel_width;
  OtkWidgetList stretchable;

  for (it = _children.begin(); it != end; ++it) {
    tmp = *it;
    if (tmp->isStretchableVert() && _rect.height() > _bevel_width * 2)
      tmp->setHeight(_rect.height() - _bevel_width * 2);
    if (tmp->isStretchableHorz())
      stretchable.push_back(tmp);
    else
      width += tmp->_rect.width() + _bevel_width;

    if (tmp->_rect.height() > tallest)
      tallest = tmp->_rect.height();
  }

  if (stretchable.size() > 0) {
    OtkWidgetList::iterator str_it = stretchable.begin(),
      str_end = stretchable.end();

    int str_width = _rect.width() - width / stretchable.size();

    for (; str_it != str_end; ++str_it)
      (*str_it)->setWidth(str_width - _bevel_width);
  }

  OtkWidget *prev_widget = 0;

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

void OtkWidget::adjustVert(void)
{
  if (_children.size() == 0)
    return;

  OtkWidget *tmp;
  OtkWidgetList::iterator it, end = _children.end();

  int widest = 0;
  int height = _bevel_width;
  OtkWidgetList stretchable;

  for (it = _children.begin(); it != end; ++it) {
    tmp = *it;
    if (tmp->isStretchableHorz() && _rect.width() > _bevel_width * 2)
      tmp->setWidth(_rect.width() - _bevel_width * 2);
    if (tmp->isStretchableVert())
      stretchable.push_back(tmp);
    else
      height += tmp->_rect.height() + _bevel_width;

    if (tmp->_rect.width() > widest)
      widest = tmp->_rect.width();
  }

  if (stretchable.size() > 0) {
    OtkWidgetList::iterator str_it = stretchable.begin(),
      str_end = stretchable.end();

    int str_height = _rect.height() - height / stretchable.size();

    for (; str_it != str_end; ++str_it)
      (*str_it)->setHeight(str_height - _bevel_width);
  }

  OtkWidget *prev_widget = 0;

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

void OtkWidget::update(void)
{
  if (_dirty) {
    adjust();
    render();
    XClearWindow(OBDisplay::display, _window);
  }

  OtkWidgetList::iterator it = _children.begin(), end = _children.end();
  for (; it != end; ++it)
    (*it)->update();

  _dirty = false;
}

void OtkWidget::internalResize(int w, int h)
{
  assert(w > 0 && h > 0);

  if (! _fixed_width && ! _fixed_height)
    resize(w, h);
  else if (! _fixed_width)
    resize(w, _rect.height());
  else if (! _fixed_height)
    resize(_rect.width(), h);
}

void OtkWidget::addChild(OtkWidget *child, bool front)
{
  assert(child);
  if (front)
    _children.push_front(child);
  else
    _children.push_back(child);
}

void OtkWidget::removeChild(OtkWidget *child)
{
  assert(child);
  OtkWidgetList::iterator it, end = _children.end();
  for (it = _children.begin(); it != end; ++it) {
    if ((*it) == child)
      break;
  }

  if (it != _children.end())
    _children.erase(it);
}

bool OtkWidget::expose(const XExposeEvent &e)
{
  if (e.window == _window) {
    _dirty = true;
    update();
    return true;
  } else {
    OtkWidgetList::iterator it = _children.begin(), end = _children.end();
    for (; it != end; ++it)
      if ((*it)->expose(e))
        return true;
  }
  return false;
}

bool OtkWidget::configure(const XConfigureEvent &e)
{
  if (e.window == _window) {
    if (_ignore_config) {
      _ignore_config--;
    } else {
      std::cout << "configure\n";
      if (!(e.width == _rect.width() && e.height == _rect.height())) {
        _dirty = true;
        _rect.setSize(e.width, e.height);
      }
      update();
    }
    return true;
  } else {
    OtkWidgetList::iterator it = _children.begin(), end = _children.end();
    for (; it != end; ++it)
      if ((*it)->configure(e))
        return true;
  }
  return false;
}

}
