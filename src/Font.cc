// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Font.cc for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
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

#include "i18n.hh"
#include "Font.hh"
#include "Util.hh"
#include "GCCache.hh"
#include "Color.hh"

string      BFont::_fallback_font   = "fixed";

#ifdef XFT
BFont::BFont(Display *d, BScreen *screen, const string &family, int size,
             bool bold, bool italic, bool shadow, bool antialias) :
                                          _display(d),
                                          _screen(screen),
                                          _family(family),
                                          _simplename(False),
                                          _size(size),
                                          _bold(bold),
                                          _italic(italic),
                                          _antialias(antialias),
                                          _shadow(shadow),
                                          _xftfont(0),
                                          _font(0),
                                          _fontset(0),
                                          _fontset_extents(0) {
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

  _font = XLoadQueryFont(_display, buildXlfd().c_str()); 
  if (! _font)
    return; // failure

  _valid = True;
}
#endif


BFont::BFont(Display *d, BScreen *screen, const string &xlfd) :
                                       _display(d),
                                       _screen(screen),
#ifdef    XFT
                                       _antialias(False),
                                       _shadow(False),
                                       _xftfont(0),
#endif // XFT
                                       _font(0),
                                       _fontset(0),
                                       _fontset_extents(0) {
  string int_xlfd;
  if (xlfd.empty())
    int_xlfd = _fallback_font;
  else
    int_xlfd = xlfd;
  
  if ((_valid = createXFont(int_xlfd)))
    return; // success

  if (int_xlfd != _fallback_font) {
    // try the fallback
    cerr << "BFont::BFont(): couldn't load font '" << _family << "'" << endl <<
      "Falling back to default '" << _fallback_font << "'" << endl;

    if ((_valid = createXFont(_fallback_font)))
      return; // success
  }

  cerr << "BFont::BFont(): couldn't load font '" << _family << "'" << endl <<
    "Giving up!" << endl;
  return; // failure
}


bool BFont::createXFont(const std::string &xlfd) {
  /*
     Even though this is only used for font sets (multibyte), it is still parsed
     out so that the bold/italic/etc information is still available from the
     class when using non-multibyte.

     This is where _simplename, _bold, _italic, and _size are initialized, since
     they are not initialized in the constructor. This needs to occur before
     calling any Xlfd-building functions.
  */
  if (! parseXlfd(xlfd))
    return False;

  if (i18n.multibyte()) {
    char **missing, *def = "-";
    int nmissing;
 
    _fontset = XCreateFontSet(_display, buildMultibyteXlfd().c_str(),
                              &missing, &nmissing, &def);
    if (nmissing) XFreeStringList(missing);
    if (_fontset)
      _fontset_extents = XExtentsOfFontSet(_fontset);
    else
      return False;

    assert(_fontset_extents);
  }
    
  _font = XLoadQueryFont(_display, xlfd.c_str());
  if (! _font)
    return False;
  return True;
}


BFont::~BFont(void) {
#ifdef    XFT
  if (_xftfont)
    XftFontClose(_display, _xftfont);
#endif // XFT

  if (i18n.multibyte() && _fontset)
    XFreeFontSet(_display, _fontset);
  if (_font)
    XFreeFont(_display, _font);
}


/*
 * Takes _family, _size, _bold, _italic, etc and builds them into a full XLFD.
 */
string BFont::buildXlfd(void) const {
  if (_simplename) 
    return _family;

  string weight = _bold ? "bold" : "medium";
  string slant = _italic ? "i" : "r";
  string sizestr= _size ? itostring(_size * 10) : "*";

  return "-*-" + _family + "-" + weight + "-" + slant + "-*-*-*-" + sizestr +
      "-*-*-*-*-*-*";
}


/*
 * Takes _family, _size, _bold, _italic, etc and builds them into a full XLFD.
 */
string BFont::buildMultibyteXlfd(void) const {
  string weight = _bold ? "bold" : "medium";
  string slant = _italic ? "i" : "r";
  string sizestr= _size ? itostring(_size) : "*";

  return _family + ','
    + "-*-*-" + weight + "-" + slant + "-*-*-*-" + sizestr +
      "-*-*-*-*-*-*" + ','
    + "-*-*-*-*-*-*-*-" + sizestr + "-*-*-*-*-*-*" + ',' +
    + "*";
}


/*
 * Takes a full X font name and parses it out so we know if we're bold, our
 * size, etc.
 */
bool BFont::parseXlfd(const string &xlfd) {
  if (xlfd.empty() || xlfd[0] != '-') {
    _family = xlfd;
    _simplename = True;
    _bold = False;
    _italic = False;
    _size = 0;
  } else {
    _simplename = False;
    string weight,
           slant,
           sizestr;
    int i = 0;

    string::const_iterator it = xlfd.begin(), end = xlfd.end();
    while(1) {
      string::const_iterator tmp = it;   // current string.begin()
      it = std::find(tmp, end, '-');     // look for comma between tmp and end
      if (i == 2) _family = string(tmp, it); // s[tmp:it]
      if (i == 3) weight = string(tmp, it);
      if (i == 4) slant = string(tmp, it);
      if (i == 7 && string(tmp, it) != "*") sizestr = string(tmp, it);
      if (sizestr.empty() &&
          i == 8 && string(tmp, it) != "*") sizestr = string(tmp, it);
      if (it == end || i >= 8)
        break;
      ++it;
      ++i;
    }
    if (i < 3)  // no name even! can't parse that
      return False;
    _bold = weight == "bold" || weight == "demibold";
    _italic = slant == "i" || slant == "o";
    _size = atoi(sizestr.c_str()) / 10;
  }
  
  // min/max size restrictions for sanity, but 0 is the font's "default size"
  if (_size && _size < 3)
    _size = 3;
  else if (_size > 97)
    _size = 97;

  return True;
}


void BFont::drawString(Drawable d, int x, int y, const BColor &color,
                       const string &string) const {
  assert(_valid);

#ifdef    XFT
  if (_xftfont) {
    XftDraw *draw = XftDrawCreate(_display, d, _screen->getVisual(),
                                  _screen->getColormap());
    assert(draw);

    if (_shadow) {
      XftColor c;
      c.color.red = 0;
      c.color.green = 0;
      c.color.blue = 0;
      c.color.alpha = 0x49 | 0x49 << 8; // transparent shadow
      c.pixel = BlackPixel(_display, _screen->getScreenNumber());

        
      XftDrawStringUtf8(draw, &c, _xftfont, x + 1, _xftfont->ascent + y + 1,
                        (XftChar8 *) string.c_str(), string.size());
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
#endif // XFT

  BPen pen(color, _font);

  if (i18n.multibyte())
    XmbDrawString(_display, d, _fontset, pen.gc(),
                  x, y - _fontset_extents->max_ink_extent.y,
                  string.c_str(), string.size());
  else
    XDrawString(_display, d, pen.gc(),
                x, _font->ascent + y,
                string.c_str(), string.size());
}


unsigned int BFont::measureString(const string &string) const {
  assert(_valid);

#ifdef    XFT
  if (_xftfont) {
    XGlyphInfo info;
    XftTextExtentsUtf8(_display, _xftfont, (XftChar8 *) string.c_str(),
                       string.size(), &info);
    return info.xOff + (_shadow ? 1 : 0);
  }
#endif // XFT

  if (i18n.multibyte()) {
    XRectangle ink, logical;
    XmbTextExtents(_fontset, string.c_str(), string.size(), &ink, &logical);
    return logical.width;
  } else {
    return XTextWidth(_font, string.c_str(), string.size());
  }
}


unsigned int BFont::height(void) const {
  assert(_valid);

#ifdef    XFT
  if (_xftfont)
    return _xftfont->height + (_shadow ? 1 : 0);
#endif // XFT

  if (i18n.multibyte())
    return _fontset_extents->max_ink_extent.height;
  else
    return _font->ascent + _font->descent;
}


unsigned int BFont::maxCharWidth(void) const {
  assert(_valid);

#ifdef    XFT
  if (_xftfont)
    return _xftfont->max_advance_width;
#endif // XFT

  if (i18n.multibyte())
    return _fontset_extents->max_logical_extent.width;
  else
    return _font->max_bounds.rbearing - _font->min_bounds.lbearing;
}
