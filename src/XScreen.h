// XScreen.h for Openbox
// Copyright (c) 2002 - 2002 Ben Janens (ben at orodu.net)
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

#ifndef   __XScreen_h
#define   __XScreen_h

#include <X11/Xlib.h>
#include "Geometry.h"

class XDisplay;

class XScreen {
private:
  Display        *_display;
  unsigned int    _number;
  Visual         *_visual;
  Window          _root;
  Colormap        _colormap;
  int             _depth;
  Size            _size;

  void setColorData();
  
  // no copying!!
  XScreen(const XScreen &);
  XScreen& operator=(const XScreen&);

public:
  XScreen(const XDisplay *display, const unsigned int number);
  virtual ~XScreen();

  inline Visual *visual() const { return _visual; }
  inline Window rootWindow() const { return _root; }
  inline Colormap colormap() const { return _colormap; }
  inline unsigned int depth() const { return _depth; }
  inline unsigned int number() const { return _number; }
  inline const Size &size() const { return _size; }
};

#endif // __XScreen_h
