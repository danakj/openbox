// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H
}

#include <iostream>
#include <algorithm>

using std::string;
using std::cerr;
using std::endl;

#include "font.hh"
#include "util.hh"
#include "gccache.hh"
#include "color.hh"

string      BFont::_fallback_font   = "fixed";

BFont::BFont(Display *d, BScreen *screen, const string &family, int size,
             bool bold, bool italic, bool shadow, unsigned char offset, 
             unsigned char tint, bool antialias) :
                                          _display(d),
                                          _screen(screen),
                                          _family(family),
                                          _simplename(False),
                                          _size(size),
                                          _bold(bold),
                                          _italic(italic),
                                          _antialias(antialias),
                                          _shadow(shadow),
                                          _offset(offset),
                                          _tint(tint),
                                          _xftfont(0) {
  _valid = False;

  _xftfont = XftFontOpen(_display, _screen->getScreenNumber(),
                         XFT_FAMILY, XftTypeString,  _family.c_str(),
                         XFT_SIZE,   XftTypeInteger, _size,
                         XFT_WEIGHT, XftTypeInteger, (_bold ?
                                                      XFT_WEIGHT_BOLD :
                                                      XFT_WEIGHT_MEDIUM),
                         XFT_SLANT,  XftTypeInteger, (_italic ?
                                                      XFT_SLANT_ITALIC :
                                                      XFT_SLANT_ROMAN),
                         XFT_ANTIALIAS, XftTypeBool, _antialias,
                         0);
  if (! _xftfont)
    return; // failure

  _valid = True;
}


BFont::~BFont(void) {
  if (_xftfont)
    XftFontClose(_display, _xftfont);
}


void BFont::drawString(Drawable d, int x, int y, const BColor &color,
                       const string &string) const {
  assert(_valid);

  XftDraw *draw = XftDrawCreate(_display, d, _screen->getVisual(),
                                _screen->getColormap());
  assert(draw);

  if (_shadow) {
    XftColor c;
    c.color.red = 0;
    c.color.green = 0;
    c.color.blue = 0;
    c.color.alpha = _tint | _tint << 8; // transparent shadow
    c.pixel = BlackPixel(_display, _screen->getScreenNumber());

    XftDrawStringUtf8(draw, &c, _xftfont, x + _offset,
                      _xftfont->ascent + y + _offset,
                      (XftChar8 *) string.c_str(),
                      string.size());
  }
    
  XftColor c;
  c.color.red = color.red() | color.red() << 8;
  c.color.green = color.green() | color.green() << 8;
  c.color.blue = color.blue() | color.blue() << 8;
  c.pixel = color.pixel();
  c.color.alpha = 0xff | 0xff << 8; // no transparency in BColor yet

  XftDrawStringUtf8(draw, &c, _xftfont, x, _xftfont->ascent + y,
                    (XftChar8 *) string.c_str(), string.size());

  XftDrawDestroy(draw);
  return;
}


unsigned int BFont::measureString(const string &string) const {
  assert(_valid);

  XGlyphInfo info;

  XftTextExtentsUtf8(_display, _xftfont, (XftChar8 *) string.c_str(),
                     string.size(), &info);

  return info.xOff + (_shadow ? _offset : 0);
}


unsigned int BFont::height(void) const {
  assert(_valid);

  return _xftfont->height + (_shadow ? _offset : 0);
}


unsigned int BFont::maxCharWidth(void) const {
  assert(_valid);

  return _xftfont->max_advance_width;
}
