// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __widget_hh
#define __widget_hh

#include "rect.hh"
#include "point.hh"
#include "rendertexture.hh"
#include "renderstyle.hh"
#include "eventdispatcher.hh"
#include "display.hh"
#include "surface.hh"

extern "C" {
#include <assert.h>
}

#include <string>
#include <list>

namespace otk {

class Widget : public EventHandler {

public:

  enum Direction { Horizontal, Vertical };

  typedef std::list<Widget *> WidgetList;

  Widget(Widget *parent, Direction = Horizontal);
  Widget(EventDispatcher *event_dispatcher, RenderStyle *style,
         Direction direction = Horizontal, Cursor cursor = 0,
         int bevel_width = 1, bool override_redirect = false);

  virtual ~Widget();

  virtual void update();

  void exposeHandler(const XExposeEvent &e);
  void configureHandler(const XConfigureEvent &e);

  inline Window window(void) const { return _window; }
  inline const Widget *parent(void) const { return _parent; }
  inline const WidgetList &children(void) const { return _children; }
  inline unsigned int screen(void) const { return _screen; }
  inline const Rect &rect(void) const { return _rect; }

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
  virtual void unfocus(void);

  inline bool hasGrabbedMouse(void) const { return _grabbed_mouse; }
  bool grabMouse(void);
  void ungrabMouse(void);

  inline bool hasGrabbedKeyboard(void) const { return _grabbed_keyboard; }
  bool grabKeyboard(void);
  void ungrabKeyboard(void);

  inline RenderTexture *texture(void) const { return _texture; }
  virtual void setTexture(RenderTexture *texture)
    { _texture = texture; _dirty = true; }

  inline const RenderColor *borderColor(void) const { return _bcolor; }
  virtual void setBorderColor(const RenderColor *color) {
    assert(color); _bcolor = color;
    XSetWindowBorder(**display, _window, color->pixel());
  }

  inline int borderWidth(void) const { return _bwidth; }
  void setBorderWidth(int width) {
    _bwidth = width;
    XSetWindowBorderWidth(**display, _window, width);
  }

  virtual void addChild(Widget *child, bool front = false);
  virtual void removeChild(Widget *child);

  inline bool isStretchableHorz(void) const { return _stretchable_horz; }
  void setStretchableHorz(bool s_horz = true) { _stretchable_horz = s_horz; }

  inline bool isStretchableVert(void) const { return _stretchable_vert; }
  void setStretchableVert(bool s_vert = true)  { _stretchable_vert = s_vert; }

  inline Cursor cursor(void) const { return _cursor; }
  void setCursor(Cursor cursor) {
    _cursor = cursor;
    XDefineCursor(**display, _window, _cursor);
  }

  inline int bevelWidth(void) const { return _bevel_width; }
  void setBevelWidth(int bevel_width)
    { assert(bevel_width > 0); _bevel_width = bevel_width; }

  inline Direction direction(void) const { return _direction; }
  void setDirection(Direction dir) { _direction = dir; }

  inline RenderStyle *style(void) const { return _style; }
  virtual void setStyle(RenderStyle *style);

  inline long eventMask(void) const { return _event_mask; }
  void setEventMask(long e);

  inline EventDispatcher *eventDispatcher(void)
    { return _event_dispatcher; }
  void setEventDispatcher(EventDispatcher *disp);

protected:
  
  bool _dirty;
  bool _focused;

  virtual void adjust(void);
  virtual void create(bool override_redirect = false);
  virtual void adjustHorz(void);
  virtual void adjustVert(void);
  virtual void internalResize(int width, int height);
  virtual void render(void);
  virtual void renderForeground() {} // for overriding

  Window _window;

  Widget *_parent;
  WidgetList _children;

  RenderStyle *_style;
  Direction _direction;
  Cursor _cursor;
  int _bevel_width;
  int _ignore_config;

  bool _visible;

  bool _grabbed_mouse;
  bool _grabbed_keyboard;

  bool _stretchable_vert;
  bool _stretchable_horz;

  RenderTexture *_texture;
  Pixmap _bg_pixmap;
  unsigned int _bg_pixel;

  const RenderColor *_bcolor;
  unsigned int _bwidth;

  Rect _rect;
  unsigned int _screen;

  bool _fixed_width;
  bool _fixed_height;

  long _event_mask;

  Surface *_surface;

  EventDispatcher *_event_dispatcher;
};

}

#endif // __widget_hh
