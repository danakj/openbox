// XScreen.cc for Openbox
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

#include "XScreen.h"
#include "Geometry.h"

XScreen::XScreen(const Display *display, const unsigned int number) {
  _display = display;
  _number = number;

  _root = RootWindow(_display, _number);
  _size = Size(WidthOfScreen(ScreenOfDisplay(_display, _number)),
               HeightOfScreen(ScreenOfDisplay(_display, _number)));
  setColorData();
}


/*
 * This sets up the _depth, _visual, and _colormap properties.
 */
void XScreen::setColorData() {
  _depth = DefaultDepth(_display, _number);
  _visual = (Visual *) 0;

  // search for a TrueColor Visual. If we can't find one, use the default
  // visual for the screen
  XVisualInfo vinfo_template, *vinfo_return;
  int vinfo_nitems;

  vinfo_template.screen = _number;
  vinfo_template.c_class = TrueColor;

  vinfo_return = XGetVisualInfo(_display, VisualScreenMask | VisualClassMask,
                                &vinfo_template, &vinfo_nitems);
  if (vinfo_return && vinfo_nitems > 0) {
    for (int i = 0; i < vinfo_nitems; i++)
      if (_depth < (vinfo_return + i)->depth) {
        _depth = (vinfo_return + i)->depth;
        _visual = (vinfo_return + i)->visual;
      }
    XFree(vinfo_return);
  }
  if (visual)
    _colormap = XCreateColormap(_display, _root, _visual, AllocNone);
  else {
    _visual = DefaultVisual(_display, _number);
    _colormap = DefaultColormap(_display, _number);
  }
}
