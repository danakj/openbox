#include "rect.hh"

namespace otk {

void Rect::setX(int x) {
  _x2 += x - _x1;
  _x1 = x;
}


void Rect::setY(int y)
{
  _y2 += y - _y1;
  _y1 = y;
}


void Rect::setPos(int x, int y) {
  _x2 += x - _x1;
  _x1 = x;
  _y2 += y - _y1;
  _y1 = y;
}


void Rect::setWidth(unsigned int w) {
  _x2 = w + _x1 - 1;
}


void Rect::setHeight(unsigned int h) {
  _y2 = h + _y1 - 1;
}


void Rect::setSize(unsigned int w, unsigned int h) {
  _x2 = w + _x1 - 1;
  _y2 = h + _y1 - 1;
}


void Rect::setRect(int x, int y, unsigned int w, unsigned int h) {
  *this = Rect(x, y, w, h);
}


void Rect::setCoords(int l, int t, int r, int b) {
  _x1 = l;
  _y1 = t;
  _x2 = r;
  _y2 = b;
}


Rect Rect::operator|(const Rect &a) const {
  Rect b;

  b._x1 = std::min(_x1, a._x1);
  b._y1 = std::min(_y1, a._y1);
  b._x2 = std::max(_x2, a._x2);
  b._y2 = std::max(_y2, a._y2);

  return b;
}


Rect Rect::operator&(const Rect &a) const {
  Rect b;

  b._x1 = std::max(_x1, a._x1);
  b._y1 = std::max(_y1, a._y1);
  b._x2 = std::min(_x2, a._x2);
  b._y2 = std::min(_y2, a._y2);

  return b;
}


bool Rect::intersects(const Rect &a) const {
  return std::max(_x1, a._x1) <= std::min(_x2, a._x2) &&
         std::max(_y1, a._y1) <= std::min(_y2, a._y2);
}


bool Rect::contains(int x, int y) const {
  return x >= _x1 && x <= _x2 &&
         y >= _y1 && y <= _y2;
}


bool Rect::contains(const Rect& a) const {
  return a._x1 >= _x1 && a._x2 <= _x2 &&
         a._y1 >= _y1 && a._y2 <= _y2;
}

}
