// Geometry.h for Openbox
// Copyright (c) 2002 - 2002 ben Jansens (ben@orodu.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef   __geometry_h
#define   __geometry_h

class Point{
  int m_x, m_y;
public:
  Point();
  Point(const Point &point);
  Point(const int x, const int y);

  void setX(const int x);
  inline int x() const {
    return m_x;
  }

  void setY(const int y);
  inline int y() const {
    return m_y;
  }
};

class Size{
  unsigned int m_w, m_h;
public:
  Size();
  Size(const Size &size);
  Size(const unsigned int w, const unsigned int h);

  void setW(const unsigned int w);
  inline unsigned int w() const {
    return m_w;
  }

  void setH(const unsigned int h);
  inline unsigned int h() const {
    return m_h;
  }
};

class Rect{
  Point m_origin;
  Size m_size;
public:
  Rect();
  Rect(const Point &origin, const Size &size);
  Rect(const int x, const int y, const unsigned int w, const unsigned int h);
  
  void setSize(const Size &size);
  inline const Size &size() const {
    return const_cast<const Size &>(m_size);
  }
  
  void setOrigin(const Point &origin);
  inline const Point &origin() const {
    return const_cast<const Point &>(m_origin);
  }
  
  void setX(const int x);
  inline int x() const {
    return m_origin.x();
  }

  void setY(const int y);
  inline int y() const {
    return m_origin.y();
  }

  void setW(const unsigned int w);
  inline unsigned int w() const {
    return m_size.w();
  }

  void setH(const unsigned int h);
  inline unsigned int h() const {
    return m_size.h();
  }

  bool Intersect(const Rect &r) const;
};  

#endif // __geometry_h
