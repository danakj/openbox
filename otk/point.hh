// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __point_hh
#define __point_hh

/*! @file point.hh
*/

namespace otk {

class Point {
private:
  int _x, _y;

public:
  Point() : _x(0), _y(0) {}
  Point(int x, int y) : _x(x), _y(y) {}

  void setX(int x) { _x = x; }
  void x() const { return _x; }

  void setY(int x) { _x = x; }
  void y() const { return _x; }
};

}

#endif /* __point_hh */
