#ifndef __focus_hh
#define __focus_hh

#include <string>
#include <list>

#include "rect.hh"
#include "point.hh"
#include "texture.hh"
#include "style.hh"

namespace otk {

class OtkWidget {

public:

  enum Direction { Horizontal, Vertical };

  typedef std::list<OtkWidget *> OtkWidgetList;

  OtkWidget(OtkWidget *parent, Direction = Horizontal);
  OtkWidget(Style *style, Direction direction = Horizontal,
            Cursor cursor = 0, int bevel_width = 1);

  virtual ~OtkWidget();

  void update(void);

  inline Window getWindow(void) const { return _window; }
  inline const OtkWidget *getParent(void) const { return _parent; }
  inline const OtkWidgetList &getChildren(void) const { return _children; }
  inline unsigned int getScreen(void) const { return _screen; }
  inline const Rect &getRect(void) const { return _rect; }

  void move(const Point &to);
  void move(int x, int y);

  virtual void setWidth(int);
  virtual void setHeight(int);

  virtual void resize(const Point &to);
  virtual void resize(int x, int y);

  virtual void setGeometry(const Rect &new_geom);
  virtual void setGeometry(const Point &topleft, int width, int height);
  virtual void setGeometry(int x, int y, int width, int height);

  inline bool isVisible(void) const { return _visible; };
  virtual void show(void);
  virtual void hide(void);

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

  virtual void addChild(OtkWidget *child, bool front = false);
  virtual void removeChild(OtkWidget *child);

  inline bool isStretchableHorz(void) const { return _stretchable_horz; }
  void setStretchableHorz(bool s_horz) { _stretchable_horz = s_horz; }

  inline bool isStretchableVert(void) const { return _stretchable_vert; }
  void setStretchableVert(bool s_vert)  { _stretchable_vert = s_vert; }

  inline Cursor getCursor(void) const { return _cursor; }

  inline int getBevelWidth(void) const { return _bevel_width; }
  void setBevelWidth(int bevel_width)
  { assert(bevel_width > 0); _bevel_width = bevel_width; }

  inline Direction getDirection(void) const { return _direction; }
  void setDirection(Direction dir) { _direction = dir; }

  inline Style *getStyle(void) const { return _style; }
  void setStyle(Style *style) { _style = style; }

private:

  void create(void);
  void adjust(void);
  void adjustHorz(void);
  void adjustVert(void);
  void internalResize(int width, int height);
  void render(void);

  Window _window;

  OtkWidget *_parent;
  OtkWidgetList _children;

  Style *_style;
  Direction _direction;
  Cursor _cursor;
  int _bevel_width;

  bool _visible;
  bool _focused;

  bool _grabbed_mouse;
  bool _grabbed_keyboard;

  bool _stretchable_vert;
  bool _stretchable_horz;

  BTexture *_texture;
  Pixmap _bg_pixmap;
  unsigned int _bg_pixel;

  Rect _rect;
  unsigned int _screen;

  bool _fixed_width;
  bool _fixed_height;

  bool _dirty;
};

}

#endif // __widget_hh
