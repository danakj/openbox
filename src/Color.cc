// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Color.cc for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh at debian.org>
// Copyright (c) 1997 - 2000, 2002 Bradley T Hughes <bhughes at trolltech.com>
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

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "Color.hh"
#include "BaseDisplay.hh"

extern "C" {
#include <stdio.h>
}


BColor::ColorCache BColor::colorcache;
bool BColor::cleancache = false;

BColor::BColor(const BaseDisplay * const _display, unsigned int _screen)
  : allocated(false), r(-1), g(-1), b(-1), p(0), dpy(_display), scrn(_screen)
{}

BColor::BColor(int _r, int _g, int _b,
               const BaseDisplay * const _display, unsigned int _screen)
  : allocated(false), r(_r), g(_g), b(_b), p(0), dpy(_display), scrn(_screen)
{}


BColor::BColor(const std::string &_name,
               const BaseDisplay * const _display, unsigned int _screen)
  : allocated(false), r(-1), g(-1), b(-1), p(0), dpy(_display), scrn(_screen),
    colorname(_name) {
  parseColorName();
}


BColor::~BColor(void) {
  deallocate();
}


void BColor::setDisplay(const BaseDisplay * const _display,
                        unsigned int _screen) {
  if (_display == display() && _screen == screen()) {
    // nothing to do
    return;
  }

  deallocate();

  dpy = _display;
  scrn = _screen;

  if (! colorname.empty()) {
    parseColorName();
  }
}


unsigned long BColor::pixel(void) const {
  if (! allocated) {
    // mutable
    BColor *that = (BColor *) this;
    that->allocate();
  }

  return p;
}


void BColor::parseColorName(void) {
  assert(dpy != 0);

  if (colorname.empty()) {
    fprintf(stderr, "BColor: empty colorname, cannot parse (using black)\n");
    setRGB(0, 0, 0);
  }

  if (scrn == ~(0u))
    scrn = DefaultScreen(display()->getXDisplay());
  Colormap colormap = display()->getScreenInfo(scrn)->getColormap();

  // get rgb values from colorname
  XColor xcol;
  xcol.red = 0;
  xcol.green = 0;
  xcol.blue = 0;
  xcol.pixel = 0;

  if (! XParseColor(display()->getXDisplay(), colormap,
                    colorname.c_str(), &xcol)) {
    fprintf(stderr, "BColor::allocate: color parse error: \"%s\"\n",
            colorname.c_str());
    setRGB(0, 0, 0);
    return;
  }

  setRGB(xcol.red >> 8, xcol.green >> 8, xcol.blue >> 8);
}


void BColor::allocate(void) {
  assert(dpy != 0);

  if (scrn == ~(0u)) scrn = DefaultScreen(display()->getXDisplay());
  Colormap colormap = display()->getScreenInfo(scrn)->getColormap();

  if (! isValid()) {
    if (colorname.empty()) {
      fprintf(stderr, "BColor: cannot allocate invalid color (using black)\n");
      setRGB(0, 0, 0);
    } else {
      parseColorName();
    }
  }

  // see if we have allocated this color before
  RGB rgb(display(), scrn, r, g, b);
  ColorCache::iterator it = colorcache.find(rgb);
  if (it != colorcache.end()) {
    // found
    allocated = true;
    p = (*it).second.p;
    (*it).second.count++;
    return;
  }

  // allocate color from rgb values
  XColor xcol;
  xcol.red =   r | r << 8;
  xcol.green = g | g << 8;
  xcol.blue =  b | b << 8;
  xcol.pixel = 0;

  if (! XAllocColor(display()->getXDisplay(), colormap, &xcol)) {
    fprintf(stderr, "BColor::allocate: color alloc error: rgb:%x/%x/%x\n",
            r, g, b);
    xcol.pixel = 0;
  }

  p = xcol.pixel;
  allocated = true;

  colorcache.insert(ColorCacheItem(rgb, PixelRef(p)));

  if (cleancache)
    doCacheCleanup();
}


void BColor::deallocate(void) {
  if (! allocated)
    return;

  assert(dpy != 0);

  ColorCache::iterator it = colorcache.find(RGB(display(), scrn, r, g, b));
  if (it != colorcache.end()) {
    if ((*it).second.count >= 1)
      (*it).second.count--;
  }

  if (cleancache)
    doCacheCleanup();

  allocated = false;
}


BColor &BColor::operator=(const BColor &c) {
  deallocate();

  setRGB(c.r, c.g, c.b);
  colorname = c.colorname;
  dpy = c.dpy;
  scrn = c.scrn;
  return *this;
}


void BColor::cleanupColorCache(void) {
  cleancache = true;
}


void BColor::doCacheCleanup(void) {
  // ### TODO - support multiple displays!
  ColorCache::iterator it = colorcache.begin();
  if (it == colorcache.end()) {
    // nothing to do
    return;
  }

  const BaseDisplay* const display = (*it).first.display;
  unsigned long *pixels = new unsigned long[ colorcache.size() ];
  unsigned int i, count;

  for (i = 0; i < display->getNumberOfScreens(); i++) {
    count = 0;
    it = colorcache.begin();

    while (it != colorcache.end()) {
      if ((*it).second.count != 0 || (*it).first.screen != i) {
        ++it;
        continue;
      }

      pixels[ count++ ] = (*it).second.p;
      ColorCache::iterator it2 = it;
      ++it;
      colorcache.erase(it2);
    }

    if (count > 0)
      XFreeColors(display->getXDisplay(),
                  display->getScreenInfo(i)->getColormap(),
                  pixels, count, 0);
  }

  delete [] pixels;
  cleancache = false;
}
