// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __Font_hh
#define   __Font_hh

extern "C" {
#include <X11/Xlib.h>

#include <X11/Xft/Xft.h>
}

#include <assert.h>

#include <string>

class BGCCache;
class BGCCacheItem;
class BColor;

#include "screen.hh"

class BFont {
  /*
   * static members
   */
private:
  static std::string  _fallback_font;

public:
  // the fallback is only used for X fonts, not for Xft fonts, since it is
  // assumed that X fonts will be the fallback from Xft.
  inline static std::string fallbackFont(void) { return _fallback_font; }
  inline static void setFallbackFont(const std::string &f)
    { _fallback_font = f; }

  /*
   * instance members
   */
private:
  Display          *_display;
  BScreen          *_screen;

  std::string       _family;
  bool              _simplename;  // true if not spec'd as a -*-* string
  int               _size;
  bool              _bold;
  bool              _italic;

  bool              _antialias;
  bool              _shadow;
  unsigned char     _offset;
  unsigned char     _tint;

  XftFont          *_xftfont;

  bool createXftFont(void);
  
  bool              _valid;

public:
  // loads an Xft font
  BFont(Display *d, BScreen *screen, const std::string &family, int size,
        bool bold, bool italic, bool shadow, unsigned char offset, 
        unsigned char tint, bool antialias = True);
  virtual ~BFont(void);

  inline bool valid(void) const { return _valid; }

  inline std::string family(void) const { assert(_valid); return _family; }
  inline int size(void) const { assert(_valid); return _size; }
  inline bool bold(void) const { assert(_valid); return _bold; }
  inline bool italic(void) const { assert(_valid); return _italic; }

  unsigned int height(void) const;
  unsigned int maxCharWidth(void) const;

  unsigned int measureString(const std::string &string) const;

  void drawString(Drawable d, int x, int y, const BColor &color,
                  const std::string &string) const;
};

#endif // __Font_hh
