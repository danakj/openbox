#include "widget.hh"
#include "display.hh"
#include "assassin.hh"
#include "screeninfo.hh"

namespace otk {

OtkWidget::OtkWidget(OtkWidget *parent)
  : _parent(parent), _visible(false), _focused(false), _grabbed_mouse(false),
    _grabbed_keyboard(false), _stretchable_vert(false),
    _stretchable_horz(false), _texture(NULL), _screen(parent->getScreen()),
    _cursor(parent->getCursor())
{
  parent->addChild(this);
  create();
}

OtkWidget::OtkWidget(unsigned int screen, Cursor cursor = 0)
  : _parent(NULL), _visible(false), _focused(false), _grabbed_mouse(false),
    _grabbed_keyboard(false), _stretchable_vert(false),
    _stretchable_horz(false), _texture(NULL), _screen(screen),
    _cursor(cursor)
{
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


  _rect.setRect(10, 10, 20, 20);

  XSetWindowAttributes attrib_create;
  unsigned long create_mask = CWBackPixmap | CWBorderPixel | CWEventMask;

  attrib_create.background_pixmap = None;
  attrib_create.colormap = scr_info->getColormap();
  attrib_create.event_mask = ButtonPressMask | ButtonReleaseMask |
    ButtonMotionMask | ExposureMask;

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

  _rect.setWidth(x - _rect.x());
  _rect.setHeight(y - _rect.y());
}

void OtkWidget::setGeometry(const Rect &new_geom)
{
  setGeometry(new_geom.x(), new_geom.y(), new_geom.height(), new_geom.width());
}
 
void OtkWidget::setGeometry(const Point &topleft, int width, int height)
{
  setGeometry(topleft.x(), topleft.y(), width, height);
}

void OtkWidget::setGeometry(int x, int y, int width, int height)
{
  _rect = Rect(x, y, width, height);
  XMoveResizeWindow(otk::OBDisplay::display, _window, x, y, width, height);
}

void OtkWidget::show(void)
{
  if (_visible)
    return;

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

void OtkWidget::blur(void)
{
  // ?
}

bool OtkWidget::grabMouse(void)
{
  return true;
}

void OtkWidget::ungrabMouse(void)
{

}

bool OtkWidget::grabKeyboard(void)
{
  return true;
}

void OtkWidget::ungrabKeyboard(void)
{

}

void OtkWidget::setTexture(BTexture *texture)
{
  texture = texture;
}

void OtkWidget::addChild(OtkWidget *child)
{
  child = child;
}

void OtkWidget::removeChild(OtkWidget *child)
{
  child = child;
}

}
