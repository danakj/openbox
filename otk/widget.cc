#include "widget.hh"
#include "display.hh"
#include "assassin.hh"
#include "screeninfo.hh"

#include <algorithm>

namespace otk {

OtkWidget::OtkWidget(OtkWidget *parent, Direction direction)
  : _parent(parent), _style(parent->getStyle()), _direction(direction),
    _cursor(parent->getCursor()), _bevel_width(parent->getBevelWidth()),
    _visible(false), _focused(false), _grabbed_mouse(false),
    _grabbed_keyboard(false), _stretchable_vert(false),
    _stretchable_horz(false), _texture(0), _bg_pixmap(0),
    _screen(parent->getScreen())
{
  parent->addChild(this);
  create();
}

OtkWidget::OtkWidget(Style *style, Direction direction,
                     Cursor cursor, int bevel_width)
  : _parent(0), _style(style), _direction(direction), _cursor(cursor),
    _bevel_width(bevel_width), _visible(false),
    _focused(false), _grabbed_mouse(false), _grabbed_keyboard(false),
    _stretchable_vert(false), _stretchable_horz(false), _texture(0),
    _bg_pixmap(0), _screen(style->getScreen())
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
}

void OtkWidget::move(const Point &to)
{
  move(to.x(), to.y());
}

void OtkWidget::move(int x, int y)
{
  _rect.setPos(x, y);
  XMoveWindow(otk::OBDisplay::display, _window, x, y);
}

void OtkWidget::resize(const Point &to)
{
  resize(to.x(), to.y());
}

void OtkWidget::resize(int x, int y)
{
  assert(x >= _rect.x() && y >= _rect.y());

  setGeometry(_rect.x(), _rect.y(), x - _rect.x(), y - _rect.y());
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

  fprintf(stderr, "Resizing to x: %d, y: %d, width: %d, height: %d\n",
          x, y, width, height);

  XMoveResizeWindow(otk::OBDisplay::display, _window, x, y, width, height);
  setTexture();
}

void OtkWidget::show(void)
{
  if (_visible)
    return;

  OtkWidgetList::iterator it = _children.begin(), end = _children.end();
  for (; it != end; ++it) {
    fprintf(stderr, "showing child\n");
    (*it)->show();
  }

  fprintf(stderr, "x: %d, y: %d, width: %d, height: %d\n",
          _rect.x(), _rect.y(), _rect.width(), _rect.height());

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

void OtkWidget::setTexture(BTexture *texture)
{
  if (!texture && !_texture)
    return;

  Pixmap old = _bg_pixmap;

  if (texture)
    _texture = texture;

  _bg_pixmap = _texture->render(_rect.width(), _rect.height(), _bg_pixmap);

  if (_bg_pixmap != old)
    XSetWindowBackgroundPixmap(otk::OBDisplay::display, _window, _bg_pixmap);
  
  //XSetWindowBackground(otk::OBDisplay::display, win, pix);
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
  OtkWidgetList::iterator it, end = _children.end();
  for (; it != end; ++it) {
    if ((*it) == child)
      break;
  }

  if (it != _children.end())
    _children.erase(it);
}

}
