// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <stdio.h>
}

#include <assert.h>

#include "color.hh"
#include "display.hh"
#include "screeninfo.hh"

namespace otk {

Color::ColorCache Color::colorcache;
bool Color::cleancache = false;

Color::Color(unsigned int _screen)
  : allocated(false), r(-1), g(-1), b(-1), p(0), scrn(_screen)
{}

Color::Color(int _r, int _g, int _b, unsigned int _screen)
  : allocated(false), r(_r), g(_g), b(_b), p(0), scrn(_screen)
{}


Color::Color(const std::string &_name, unsigned int _screen)
  : allocated(false), r(-1), g(-1), b(-1), p(0), scrn(_screen),
    colorname(_name) {
  parseColorName();
}


Color::~Color(void) {
  deallocate();
}


void Color::setScreen(unsigned int _screen) {
  if (_screen == screen()) {
    // nothing to do
    return;
  }

  deallocate();

  scrn = _screen;

  if (! colorname.empty()) {
    parseColorName();
  }
}


unsigned long Color::pixel(void) const {
  if (! allocated) {
    // mutable
    Color *that = (Color *) this;
    that->allocate();
  }

  return p;
}


void Color::parseColorName(void) {
  if (colorname.empty()) {
    fprintf(stderr, "Color: empty colorname, cannot parse (using black)\n");
    setRGB(0, 0, 0);
  }

  if (scrn == ~(0u))
    scrn = DefaultScreen(**display);
  Colormap colormap = display->screenInfo(scrn)->colormap();

  // get rgb values from colorname
  XColor xcol;
  xcol.red = 0;
  xcol.green = 0;
  xcol.blue = 0;
  xcol.pixel = 0;

  if (! XParseColor(**display, colormap,
                    colorname.c_str(), &xcol)) {
    fprintf(stderr, "Color::allocate: color parse error: \"%s\"\n",
            colorname.c_str());
    setRGB(0, 0, 0);
    return;
  }

  setRGB(xcol.red >> 8, xcol.green >> 8, xcol.blue >> 8);
}


void Color::allocate(void) {
  if (scrn == ~(0u)) scrn = DefaultScreen(**display);
  Colormap colormap = display->screenInfo(scrn)->colormap();

  if (! isValid()) {
    if (colorname.empty()) {
      fprintf(stderr, "Color: cannot allocate invalid color (using black)\n");
      setRGB(0, 0, 0);
    } else {
      parseColorName();
    }
  }

  // see if we have allocated this color before
  RGB rgb(scrn, r, g, b);
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

  if (! XAllocColor(**display, colormap, &xcol)) {
    fprintf(stderr, "Color::allocate: color alloc error: rgb:%x/%x/%x\n",
            r, g, b);
    xcol.pixel = 0;
  }

  p = xcol.pixel;
  allocated = true;

  colorcache.insert(ColorCacheItem(rgb, PixelRef(p)));

  if (cleancache)
    doCacheCleanup();
}


void Color::deallocate(void) {
  if (! allocated)
    return;

  ColorCache::iterator it = colorcache.find(RGB(scrn, r, g, b));
  if (it != colorcache.end()) {
    if ((*it).second.count >= 1)
      (*it).second.count--;
  }

  if (cleancache)
    doCacheCleanup();

  allocated = false;
}


Color &Color::operator=(const Color &c) {
  deallocate();

  setRGB(c.r, c.g, c.b);
  colorname = c.colorname;
  scrn = c.scrn;
  return *this;
}


void Color::cleanupColorCache(void) {
  cleancache = true;
}


void Color::doCacheCleanup(void) {
  // ### TODO - support multiple displays!
  ColorCache::iterator it = colorcache.begin();
  if (it == colorcache.end()) {
    // nothing to do
    return;
  }

  unsigned long *pixels = new unsigned long[ colorcache.size() ];
  int i;
  unsigned count;

  for (i = 0; i < ScreenCount(**display); i++) {
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
      XFreeColors(**display, display->screenInfo(i)->colormap(),
                  pixels, count, 0);
  }

  delete [] pixels;
  cleancache = false;
}

}
