// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __rect_hh
#define __rect_hh

#include "point.hh"
#include "size.hh"

namespace otk {

class Rect {
  Point _p;
  Size _s;
public:
  Rect() : _p(), _s() {}
  Rect(const Point &p, const Size &s) : _p(p), _s(s) {}
  Rect(const Rect &r) : _p(r._p), _s(r._s) {}
  Rect(int x, int y, int w, int h)
    : _p(x, y), _s(w, h) {}

  inline int x() const { return _p.x(); }
  inline int y() const { return _p.y(); }
  inline int width() const { return _s.width(); }
  inline int height() const { return _s.height(); }

  inline int left() const { return _p.x(); }
  inline int top() const { return _p.y(); }
  inline int right() const { return _p.x() + _s.width() - 1; }
  inline int bottom() const { return _p.y() + _s.height() - 1; }

  inline const Point& position() const { return _p; }
  inline const Size& size() const { return _s; }

  bool operator==(const Rect &o) const { return _p == o._p && _s == o._s; }
  bool operator!=(const Rect &o) const { return _p != o._p || _s != o._s; }
};

}

#endif // __rect_hh
