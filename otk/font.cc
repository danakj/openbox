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
#include "display.hh"
#include "color.hh"
#include "screeninfo.hh"

extern "C" {
#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#include "gettext.h"
#define _(str) gettext(str)
}

namespace otk {

string      Font::_fallback_font = "fixed";
bool        Font::_xft_init      = false;

Font::Font(int screen_num, const string &fontstring,
             bool shadow, unsigned char offset, unsigned char tint)
  : _screen_num(screen_num),
    _fontstring(fontstring),
    _shadow(shadow),
    _offset(offset),
    _tint(tint),
    _xftfont(0)
{
  assert(screen_num >= 0);
  assert(tint <= CHAR_MAX);
  
  if (!_xft_init) {
    if (!XftInit(0)) {
      printf(_("Couldn't initialize Xft.\n\n"));
      ::exit(3);
    }
    int version = XftGetVersion();
    printf(_("Using Xft %d.%d.%d.\n"),
           version / 10000 % 100, version / 100 % 100, version % 100);
    _xft_init = true;
  }

  if ((_xftfont = XftFontOpenName(Display::display, _screen_num,
                                  _fontstring.c_str())))
    return;

  printf(_("Unable to load font: %s\n"), _fontstring.c_str());
  printf(_("Trying fallback font: %s\n"), _fallback_font.c_str());

  if ((_xftfont = XftFontOpenName(Display::display, _screen_num,
                                  _fallback_font.c_str())))
    return;

  printf(_("Unable to load font: %s\n"), _fallback_font.c_str());
  printf(_("Aborting!.\n"));

  ::exit(3); // can't continue without a font
}


Font::~Font(void)
{
  if (_xftfont)
    XftFontClose(Display::display, _xftfont);
}


void Font::drawString(XftDraw *d, int x, int y, const Color &color,
                       const string &string, bool utf8) const
{
  assert(d);

  if (_shadow) {
    XftColor c;
    c.color.red = 0;
    c.color.green = 0;
    c.color.blue = 0;
    c.color.alpha = _tint | _tint << 8; // transparent shadow
    c.pixel = BlackPixel(Display::display, _screen_num);

    if (utf8)
      XftDrawStringUtf8(d, &c, _xftfont, x + _offset,
                        _xftfont->ascent + y + _offset,
                        (FcChar8*)string.c_str(), string.size());
    else
      XftDrawString8(d, &c, _xftfont, x + _offset,
                     _xftfont->ascent + y + _offset,
                     (FcChar8*)string.c_str(), string.size());
  }
    
  XftColor c;
  c.color.red = color.red() | color.red() << 8;
  c.color.green = color.green() | color.green() << 8;
  c.color.blue = color.blue() | color.blue() << 8;
  c.pixel = color.pixel();
  c.color.alpha = 0xff | 0xff << 8; // no transparency in Color yet

  if (utf8)
    XftDrawStringUtf8(d, &c, _xftfont, x, _xftfont->ascent + y,
                      (FcChar8*)string.c_str(), string.size());
  else
    XftDrawString8(d, &c, _xftfont, x, _xftfont->ascent + y,
                   (FcChar8*)string.c_str(), string.size());

  return;
}


unsigned int Font::measureString(const string &string, bool utf8) const
{
  XGlyphInfo info;

  if (utf8)
    XftTextExtentsUtf8(Display::display, _xftfont,
                       (FcChar8*)string.c_str(), string.size(), &info);
  else
    XftTextExtents8(Display::display, _xftfont,
                    (FcChar8*)string.c_str(), string.size(), &info);

  return info.xOff + (_shadow ? _offset : 0);
}


unsigned int Font::height(void) const
{
  return _xftfont->height + (_shadow ? _offset : 0);
}


unsigned int Font::maxCharWidth(void) const
{
  return _xftfont->max_advance_width;
}

}
