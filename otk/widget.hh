// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __widget_hh
#define __widget_hh

#include "eventhandler.hh"
#include "rect.hh"
#include "renderstyle.hh"

#include <list>
#include <algorithm>
#include <cassert>

namespace otk {

class Surface;
class RenderTexture;
class RenderColor;
class EventDispatcher;

class Widget : public EventHandler, public StyleNotify {
public:
  enum Direction { Horizontal, Vertical };

  Widget(int screen, EventDispatcher *ed, Direction direction = Horizontal,
         int bevel = 3, bool overrideredir = false);
  Widget(Widget *parent, Direction direction = Horizontal, int bevel = 3);
  virtual ~Widget();

  inline int screen() const { return _screen; }
  inline Window window() const { return _window; }
  inline Widget *parent() const { return _parent; }
  inline Direction direction() const { return _direction; }

  inline RenderStyle::Justify alignment() const { return _alignment; }
  void setAlignment(RenderStyle::Justify a);

  inline long eventMask() const { return _event_mask; }
  virtual void setEventMask(long e);
  
  inline const Rect& area() const { return _area; }
  inline Rect usableArea() const { return Rect(_area.position(),
                                               Size(_area.width() -
                                                    _borderwidth * 2,
                                                    _area.height() -
                                                    _borderwidth * 2));}
  inline const Size& minSize() const { return _min_size; }
  inline const Size& maxSize() const { return _max_size; }
  virtual void setMaxSize(const Size &s);

  virtual void show(bool children = false);
  virtual void hide();
  inline bool visible() const { return _visible; }

  virtual void update();
  virtual void refresh() { _dirty = true; render(); }
  
  virtual void setBevel(int b);
  inline int bevel() const { return _bevel; }

  void move(const Point &p)
    { moveresize(Rect(p, _area.size())); }
  void resize(const Size &s)
    { moveresize(Rect(_area.position(), s)); }
  /*!
    When a widget has a parent, this won't change the widget directly, but will
    just cause the parent to re-layout all its children.
  */
  virtual void moveresize(const Rect &r);

  inline const RenderColor *borderColor() const { return _bordercolor; }
  virtual void setBorderColor(const RenderColor *c);

  inline int borderWidth() const { return _borderwidth; }
  virtual void setBorderWidth(int w);

  const std::list<Widget*>& children() const { return _children; }

  virtual void exposeHandler(const XExposeEvent &e);
  virtual void configureHandler(const XConfigureEvent &e);
  virtual void styleChanged(const RenderStyle &) {}

protected:
  virtual void addChild(Widget *w) { assert(w); _children.push_back(w); }
  virtual void removeChild(Widget *w) { assert(w); _children.remove(w); }

  //! Find the default min/max sizes for the widget. Useful after the in-use
  //! style has changed.
  virtual void calcDefaultSizes() {};

  virtual void setMinSize(const Size &s);

  //! Arrange the widget's children
  virtual void layout();
  virtual void layoutHorz();
  virtual void layoutVert();
  virtual void render();
  virtual void renderForeground(Surface&) {};
  virtual void renderChildren();

  void createWindow(bool overrideredir);

  RenderTexture *_texture;
  
private:
  void internal_moveresize(int x, int y, unsigned w, unsigned int h);

  int _screen;
  Widget *_parent;
  Window _window;
  Surface *_surface;
  long _event_mask;
  
  RenderStyle::Justify _alignment;
  Direction _direction;
  Rect _area;
  //! This size is the size *inside* the border, so they won't match the
  //! actual size of the widget
  Size _min_size;
  //! This size is the size *inside* the border, so they won't match the
  //! actual size of the widget 
  Size _max_size;

  bool _visible;
  
  const RenderColor *_bordercolor;
  int _borderwidth;
  int _bevel;
  bool _dirty;

  std::list<Widget*> _children;

  EventDispatcher *_dispatcher;

  int _ignore_config;
};

}

#endif // __widget_hh
