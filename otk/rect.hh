// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __rect_hh
#define   __rect_hh

extern "C" {
#include <X11/Xlib.h>
}

#include "point.hh"
#include <vector>

namespace otk {

//! The Rect class defines a rectangle in the plane.
class Rect {
public:
  //! Constructs an invalid Rect
  inline Rect(void) : _x1(0), _y1(0), _x2(0), _y2(0) { }
  //! Constructs a Rect
  /*!
    @param x The x component of the point defining the top left corner of the 
             rectangle
    @param y The y component of the point defining the top left corner of the
             rectangle
    @param w The width of the rectangle
    @param h The height of the rectangle
  */
  inline Rect(int x, int y, int w, int h)
    : _x1(x), _y1(y), _x2(w + x - 1), _y2(h + y - 1) { }
  //! Constructs a Rect from 2 Point objects
  /*!
    @param location The point defining the top left corner of the rectangle
    @param size The width and height of the rectangle
  */
  inline Rect(const Point &location, const Point &size)
    : _x1(location.x()), _y1(location.y()),
      _x2(size.x() + location.x() - 1), _y2(size.y() + location.y() - 1) { }
  //! Constructs a Rect from another Rect
  /*!
    @param rect The rectangle from which to construct this new one
  */
  inline Rect(const Rect &rect)
    : _x1(rect._x1), _y1(rect._y1), _x2(rect._x2), _y2(rect._y2) { }
  //! Constructs a Rect from an XRectangle
  inline explicit Rect(const XRectangle& xrect)
    : _x1(xrect.x), _y1(xrect.y), _x2(xrect.width + xrect.x - 1),
      _y2(xrect.height + xrect.y - 1) { }

  //! Returns the left coordinate of the Rect. Identical to Rect::x.
  inline int left(void) const { return _x1; }
  //! Returns the top coordinate of the Rect. Identical to Rect::y.
  inline int top(void) const { return _y1; }
  //! Returns the right coordinate of the Rect
  inline int right(void) const { return _x2; }
  //! Returns the bottom coordinate of the Rect
  inline int bottom(void) const { return _y2; }

  //! The x component of the point defining the top left corner of the Rect
  inline int x(void) const { return _x1; }
  //! The y component of the point defining the top left corner of the Rect
  inline int y(void) const { return _y1; }
  //! Returns the Point that defines the top left corner of the rectangle
  inline Point location() const { return Point(_x1, _y1); }

  //! Sets the x coordinate of the Rect.
  /*!
    @param x The new x component of the point defining the top left corner of
             the rectangle
  */
  void setX(int x);
  //! Sets the y coordinate of the Rect.
  /*!
    @param y The new y component of the point defining the top left corner of
             the rectangle
  */
  void setY(int y);
  //! Sets the x and y coordinates of the Rect.
  /*!
    @param x The new x component of the point defining the top left corner of
             the rectangle
    @param y The new y component of the point defining the top left corner of
             the rectangle
  */
  void setPos(int x, int y);
  //! Sets the x and y coordinates of the Rect.
  /*!
    @param location The point defining the top left corner of the rectangle.
  */
  void setPos(const Point &location);

  //! The width of the Rect
  inline int width(void) const { return _x2 - _x1 + 1; }
  //! The height of the Rect
  inline int height(void) const { return _y2 - _y1 + 1; }
  //! Returns the size of the Rect
  inline Point size() const { return Point(_x2 - _x1 + 1, _y2 - _y1 + 1); }

  //! Sets the width of the Rect
  /*!
    @param w The new width of the rectangle
  */
  void setWidth(int w);
  //! Sets the height of the Rect
  /*!
    @param h The new height of the rectangle
  */
  void setHeight(int h);
  //! Sets the size of the Rect.
  /*!
    @param w The new width of the rectangle
    @param h The new height of the rectangle
  */
  void setSize(int w, int h);
  //! Sets the size of the Rect.
  /*!
    @param size The new size of the rectangle
  */
  void setSize(const Point &size);

  //! Sets the position and size of the Rect
  /*!
    @param x The new x component of the point defining the top left corner of
             the rectangle
    @param y The new y component of the point defining the top left corner of
             the rectangle
    @param w The new width of the rectangle
    @param h The new height of the rectangle
   */
  void setRect(int x, int y, int w, int h);
  //! Sets the position and size of the Rect
  /*!
    @param location The new point defining the top left corner of the rectangle
    @param size The new size of the rectangle
   */
  void setRect(const Point &location, const Point &size);

  //! Sets the position of all 4 sides of the Rect
  /*!
    @param l The new left coordinate of the rectangle
    @param t The new top coordinate of the rectangle
    @param r The new right coordinate of the rectangle
    @param b The new bottom coordinate of the rectangle
   */
  void setCoords(int l, int t, int r, int b);
  //! Sets the position of all 4 sides of the Rect
  /*!
    @param tl The new point at the top left of the rectangle
    @param br The new point at the bottom right of the rectangle
   */
  void setCoords(const Point &tl, const Point &br);

  //! Determines if two Rect objects are equal
  /*!
    The rectangles are considered equal if they are in the same position and
    are the same size.
  */
  inline bool operator==(const Rect &a)
  { return _x1 == a._x1 && _y1 == a._y1 && _x2 == a._x2 && _y2 == a._y2; }
  //! Determines if two Rect objects are inequal
  /*!
    @see operator==
  */
  inline bool operator!=(const Rect &a) { return ! operator==(a); }

  //! Returns the union of two Rect objects
  /*!
    The union of the rectangles will consist of the maximimum area that the two
    rectangles can make up.
    @param a A second Rect object to form a union with.
   */
  Rect operator|(const Rect &a) const;
  //! Returns the intersection of two Rect objects
  /*!
    The intersection of the rectangles will consist of just the area where the
    two rectangles overlap.
    @param a A second Rect object to form an intersection with.
    @return The intersection between this Rect and the one passed to the
            function
  */
  Rect operator&(const Rect &a) const;
  //! Sets the Rect to the union of itself with another Rect object
  /*!
    The union of the rectangles will consist of the maximimum area that the two
    rectangles can make up.
    @param a A second Rect object to form a union with.
    @return The union between this Rect and the one passed to the function
   */
  inline Rect &operator|=(const Rect &a) { *this = *this | a; return *this; }
  //! Sets the Rect to the intersection of itself with another Rect object
  /*!
    The intersection of the rectangles will consist of just the area where the
    two rectangles overlap.
    @param a A second Rect object to form an intersection with.
  */
  inline Rect &operator&=(const Rect &a) { *this = *this & a; return *this; }

  //! Returns if the Rect is valid
  /*!
    A rectangle is valid only if its right and bottom coordinates are larger
    than its left and top coordinates (i.e. it does not have a negative width
    or height).
    @return true if the Rect is valid; otherwise, false
  */
  inline bool valid(void) const { return _x2 > _x1 && _y2 > _y1; }

  //! Determines if this Rect intersects another Rect
  /*!
    The rectangles intersect if any part of them overlaps.
    @param a Another Rect object to compare this Rect with
    @return true if the Rect objects overlap; otherwise, false
  */
  bool intersects(const Rect &a) const;
  //! Determines if this Rect contains a point
  /*!
    The rectangle contains the point if it falls within the rectangle's
    boundaries.
    @param x The x coordinate of the point to operate on
    @param y The y coordinate of the point to operate on
    @return true if the point is contained within this Rect; otherwise, false
  */
  bool contains(int x, int y) const;
  //! Determines if this Rect contains a point
  /*!
    The rectangle contains the point if it falls within the rectangle's
    boundaries.
    @param p The point to operate on
    @return true if the point is contained within this Rect; otherwise, false
  */
  bool contains(const Point &p) const;
  //! Determines if this Rect contains another Rect entirely
  /*!
    This rectangle contains the second rectangle if it is entirely within this
    rectangle's boundaries.
    @param a The Rect to test for containment inside of this Rect
    @return true if the second Rect is contained within this Rect; otherwise,
            false
  */
  bool contains(const Rect &a) const;

private:
  //! The left coordinate of the Rect
  int _x1;
  //! The top coordinate of the Rect
  int _y1;
  //! The right coordinate of the Rect
  int _x2;
  //! The bottom coordinate of the Rect
  int _y2;
};

//! A list for Rect objects
typedef std::vector<Rect> RectList;

}

#endif // __rect_hh
