// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Util.cc for Blackbox - an X11 Window manager
// Copyright (c) 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
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

#ifndef _BLACKBOX_UTIL_HH
#define _BLACKBOX_UTIL_HH

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <string>

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

private:
  int _x1, _y1, _x2, _y2;
};

/* XXX: this needs autoconf help */
const unsigned int BSENTINEL = 65535;

std::string expandTilde(const std::string& s);

void bexec(const std::string& command, const std::string& displaystring);

#ifndef   HAVE_BASENAME
std::string basename(const std::string& path);
#endif

std::string textPropertyToString(Display *display, XTextProperty& text_prop);

struct timeval; // forward declare to avoid the header
timeval normalizeTimeval(const timeval &tm);

struct PointerAssassin {
  template<typename T>
  inline void operator()(const T ptr) const {
    delete ptr;
  }
};

#endif
