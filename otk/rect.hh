// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __rect_hh
#define   __rect_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <vector>

namespace otk {

class Rect {
public:
  inline Rect(void) : _x1(0), _y1(0), _x2(0), _y2(0) { }
  inline Rect(int __x, int __y, unsigned int __w, unsigned int __h)
    : _x1(__x), _y1(__y), _x2(__w + __x - 1), _y2(__h + __y - 1) { }
  inline explicit Rect(const XRectangle& xrect)
    : _x1(xrect.x), _y1(xrect.y), _x2(xrect.width + xrect.x - 1),
      _y2(xrect.height + xrect.y - 1) { }

  inline int left(void) const { return _x1; }
  inline int top(void) const { return _y1; }
  inline int right(void) const { return _x2; }
  inline int bottom(void) const { return _y2; }

  inline int x(void) const { return _x1; }
  inline int y(void) const { return _y1; }
  void setX(int __x);
  void setY(int __y);
  void setPos(int __x, int __y);

  inline unsigned int width(void) const { return _x2 - _x1 + 1; }
  inline unsigned int height(void) const { return _y2 - _y1 + 1; }
  void setWidth(unsigned int __w);
  void setHeight(unsigned int __h);
  void setSize(unsigned int __w, unsigned int __h);

  void setRect(int __x, int __y, unsigned int __w, unsigned int __h);

  void setCoords(int __l, int __t, int __r, int __b);

  inline bool operator==(const Rect &a)
  { return _x1 == a._x1 && _y1 == a._y1 && _x2 == a._x2 && _y2 == a._y2; }
  inline bool operator!=(const Rect &a) { return ! operator==(a); }

  Rect operator|(const Rect &a) const;
  Rect operator&(const Rect &a) const;
  inline Rect &operator|=(const Rect &a) { *this = *this | a; return *this; }
  inline Rect &operator&=(const Rect &a) { *this = *this & a; return *this; }

  inline bool valid(void) const { return _x2 > _x1 && _y2 > _y1; }

  bool intersects(const Rect &a) const;
  bool contains(int __x, int __y) const;
  bool contains(const Rect &a) const;

private:
  int _x1, _y1, _x2, _y2;
};

typedef std::vector<Rect> RectList;

}

#endif // __rect_hh
