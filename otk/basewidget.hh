#ifndef __basewidget_hh
#define __basewidget_hh

#include "rect.hh"
#include "point.hh"
#include "texture.hh"
#include "style.hh"
#include "display.hh"
#include "eventdispatcher.hh"

extern "C" {
#include <assert.h>
}

#include <list>

namespace otk {

class OtkBaseWidget : public OtkEventHandler {

public:

  typedef std::list<OtkBaseWidget *> OtkBaseWidgetList;

  OtkBaseWidget(OtkBaseWidget *parent);
  OtkBaseWidget(Style *style, Cursor cursor = 0, int bevel_width = 1);

  virtual ~OtkBaseWidget();

  //! Redraw the widget and all of its children
  virtual void update(void);

  void exposeHandler(const XExposeEvent &e);
  void configureHandler(const XConfigureEvent &e);

  inline Window getWindow(void) const { return _window; }
  inline const OtkBaseWidget *getParent(void) const { return _parent; }
  inline const OtkBaseWidgetList &getChildren(void) const { return _children; }
  inline unsigned int getScreen(void) const { return _screen; }
  inline const Rect &getRect(void) const { return _rect; }

  void move(const Point &to);
  void move(int x, int y);

  virtual void setWidth(int);
  virtual void setHeight(int);

  virtual int width() const { return _rect.width(); }
  virtual int height() const { return _rect.height(); }

  virtual void resize(const Point &to);
  virtual void resize(int x, int y);

  virtual void setGeometry(const Rect &new_geom);
  virtual void setGeometry(const Point &topleft, int width, int height);
  virtual void setGeometry(int x, int y, int width, int height);

  inline bool isVisible(void) const { return _visible; };
  virtual void show(bool recursive = false);
  virtual void hide(bool recursive = false);

  inline bool isFocused(void) const { return _focused; };
  virtual void focus(void);

  inline bool hasGrabbedMouse(void) const { return _grabbed_mouse; }
  bool grabMouse(void);
  void ungrabMouse(void);

  inline bool hasGrabbedKeyboard(void) const { return _grabbed_keyboard; }
  bool grabKeyboard(void);
  void ungrabKeyboard(void);

  inline BTexture *getTexture(void) const { return _texture; }
  virtual void setTexture(BTexture *texture)
  { _texture = texture; _dirty = true; }

  virtual void addChild(OtkBaseWidget *child, bool front = false);
  virtual void removeChild(OtkBaseWidget *child);

  inline Cursor getCursor(void) const { return _cursor; }
  void setCursor(Cursor cursor) {
    _cursor = cursor;
    XDefineCursor(OBDisplay::display, _window, _cursor);
  }

  inline int getBevelWidth(void) const { return _bevel_width; }
  void setBevelWidth(int bevel_width)
  { assert(bevel_width > 0); _bevel_width = bevel_width; }

  inline Style *getStyle(void) const { return _style; }
  virtual void setStyle(Style *style);

protected:

  //! If the widget needs to redraw itself. Watched by all subclasses too!
  bool _dirty;

  Window _window;

  OtkBaseWidget *_parent;
  OtkBaseWidgetList _children;

  Style *_style;
  Cursor _cursor;
  int _bevel_width;
  int _ignore_config;

  bool _visible;
  bool _focused;

  bool _grabbed_mouse;
  bool _grabbed_keyboard;

  BTexture *_texture;
  Pixmap _bg_pixmap;
  unsigned int _bg_pixel;

  Rect _rect;
  unsigned int _screen;

private:

  void create(void);
  void render(void);
};

}

#endif // __basewidget_hh
