// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __point_hh
#define __point_hh

namespace otk {

class Point {
  int _x, _y;
public:
  Point() : _x(0), _y(0) {}
  Point(int x, int y) : _x(x), _y(y) {}
  Point(const Point &p) : _x(p._x), _y(p._y) {}

  inline int x() const { return _x; }
  inline int y() const { return _y; }

  bool operator==(const Point &o) const { return _x == o._x && _y == o._y; }
  bool operator!=(const Point &o) const { return _x != o._x || _y != o._y; }
};

}

#endif // __point_hh
