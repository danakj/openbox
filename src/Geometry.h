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
  void setSize(const unsigned int w, const unsigned int h);
  inline const Size &size() const {
    return const_cast<const Size &>(m_size);
  }
  
  void setOrigin(const Point &origin);
  void setOrigin(const int x, const int y);
  inline const Point &origin() const {
    return const_cast<const Point &>(m_origin);
  }
  
  void setLeft(const int x);
  inline int left() const {
    return m_origin.x();
  }
  
  void setX(const int x);
  inline int x() const {
    return m_origin.x();
  }

  void setTop(const int y);
  inline int top() const {
    return m_origin.y();
  }
  
  void setY(const int y);
  inline int y() const {
    return m_origin.y();
  }

  void setRight(const int left);
  inline int right() const {
    return m_origin.x()+m_size.w()-1;
  }
  
  void setW(const unsigned int w);
  inline unsigned int w() const {
    return m_size.w();
  }

  void setBottom(const int bottom);
  inline int bottom() const {
    return m_origin.y()+m_size.h()-1;
  }
  
  void setH(const unsigned int h);
  inline unsigned int h() const {
    return m_size.h();
  }

  bool Intersect(const Rect &r) const;
  // returns a rect that is this rect increased in size by the passed in amount
  Rect Inflate(const unsigned int i) const;
  Rect Inflate(const unsigned int iw, const unsigned int ih) const;
  Rect Inflate(const Size &i) const;
  // returns a rect that is this rect decreased in size by the passed in amount
  Rect Deflate(const unsigned int d) const;
  Rect Deflate(const unsigned int dw, const unsigned int dh) const;
  Rect Deflate(const Size &d) const;
  // returns a rect that is moved the amount specified
  Rect Translate(const int t) const;
  Rect Translate(const int tx, const int ty) const;
  Rect Translate(const Point &t) const;
};  

#endif // __geometry_h
