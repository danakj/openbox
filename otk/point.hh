// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __point_hh
#define __point_hh

/*! @file point.hh
  @brief The Point class contains an x/y pair
*/

namespace otk {

//! The Point class is an x/y coordinate or size pair
class Point {
private:
  //! The x value
  int _x;
  //! The y value
  int _y;

public:
  //! Constructs a new Point with 0,0 values
  Point() : _x(0), _y(0) {}
  //! Constructs a new Point with given values
  Point(int x, int y) : _x(x), _y(y) {}

  //! Changes the x value to the new value specified
  void setX(int x) { _x = x; }
  //! Returns the x value
  int x() const { return _x; }

  //! Changes the y value to the new value specified
  void setY(int x) { _x = x; }
  //! Returns the y value
  int y() const { return _x; }

  //! Changes the x and y values
  void setPoint(int x, int y) { _x = x; _y = y; }
};

}

#endif /* __point_hh */
