// Geometry.cc for Openbox
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

#include "Geometry.h"

Point::Point() : m_x(0), m_y(0) {
}

Point::Point(const Point &point) : m_x(point.m_x), m_y(point.m_y) {
}

Point::Point(const int x, const int y) : m_x(x), m_y(y) {
}

void Point::setX(const int x) {
  m_x = x;
}

void Point::setY(const int y) {
  m_y = y;
}

Size::Size() : m_w(0), m_h(0) {
}

Size::Size(const Size &size) : m_w(size.m_w), m_h(size.m_h) {
}

Size::Size(const unsigned int w, const unsigned int h) : m_w(w), m_h(h) {
}

void Size::setW(const unsigned int w) {
  m_w = w;
}

void Size::setH(const unsigned int h) {
  m_h = h;
}

Rect::Rect() : m_origin(0, 0), m_size(0, 0) {
}

Rect::Rect(const Point &origin, const Size &size) : m_origin(origin),
  m_size(size) {
}

Rect::Rect(const int x, const int y, const unsigned int w, const unsigned int h)
  : m_origin(x, y), m_size(w, h) {
}

void Rect::setSize(const Size &size) {
  m_size = size;
}

void Rect::setOrigin(const Point &origin) {
  m_origin = origin;
}

void Rect::setX(const int x) {
  m_origin.setX(x);
}

void Rect::setY(const int y) {
  m_origin.setY(y);
}

void Rect::setW(unsigned int w) {
  m_size.setW(w);
}

void Rect::setH(unsigned int h) {
  m_size.setH(h);
}

bool Rect::Intersect(const Rect &r) const {
  return
    (x() < (r.x()+r.w()) ) &&
    ( (x()+w()) > r.x()) &&
    (y() < (r.y()+r.h()) ) &&
    ( (y()+h()) > r.y());
}
