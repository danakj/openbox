// -*- mode: C++; indent-tabs-mode: nil; -*-
// Font.hh for Blackbox - an X11 Window manager
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

#ifndef   __Font_hh
#define   __Font_hh

extern "C" {
#include <X11/Xlib.h>

#ifdef XFT
#  include <X11/Xft/Xft.h>
#endif
}

#include <assert.h>

#include <string>

class BGCCache;
class BGCCacheItem;
class BColor;

#include "Screen.hh"

class BFont {
  /*
   * static members
   */
private:
//  static bool         _antialias;
  static std::string  _fallback_font;

public:
//  inline static bool antialias(void) { return _antialias; }
//  inline static void setAntialias(bool a) { _antialias = a; }

  inline static std::string fallbackFont(void) { return _fallback_font; }
  inline static void setFallbackFont(const std::string &f)
    { _fallback_font = f; }

  /*
   * instance members
   */
private:
  Display          *_display;
  BScreen          *_screen;

  std::string       _name;
  bool              _simplename;  // true if not spec'd as a -*-* string
  int               _size;
  bool              _bold;
  bool              _italic;

#ifdef XFT
  XftFont          *_xftfont;
#endif
  
  // standard
  XFontStruct      *_font;
  // multibyte
  XFontSet          _fontset;
  XFontSetExtents  *_fontset_extents;

  std::string buildXlfdName(bool mb) const;

  bool init(const std::string &xlfd = "");
  bool createFont(void);
  bool parseFontString(const std::string &xlfd);
  
  mutable BGCCache *_cache;
  mutable BGCCacheItem *_item;

  bool              _valid;

public:
  BFont(Display *d, BScreen *screen, const std::string &family, int size,
        bool bold, bool italic);
  BFont(Display *d, BScreen *screen, const std::string &xlfd);
  virtual ~BFont();

  inline bool valid(void) const { return _valid; }

  inline std::string name(void) const { assert(_valid); return _name; }
  inline int size(void) const { assert(_valid); return _size / 10; }
  inline bool bold(void) const { assert(_valid); return _bold; }
  inline bool italic(void) const { assert(_valid); return _italic; }

  unsigned int height(void) const;
  unsigned int maxCharWidth(void) const;

  unsigned int measureString(const std::string &string) const;

  void drawString(Drawable d, int x, int y, const BColor &color,
                  const std::string &string) const;
};

#endif // __Font_hh
