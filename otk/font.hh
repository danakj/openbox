// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __font_hh
#define   __font_hh

#include "ustring.hh"
#include "truerendercontrol.hh"

extern "C" {
#include <X11/Xlib.h>
#define _XFT_NO_COMPAT_ // no Xft 1 API
#include <X11/Xft/Xft.h>
}

#include <cassert>

namespace otk {

class Color;
class Surface;

class Font {
  /*
   * static members
   */
private:
  static bool       _xft_init;

  int               _screen_num;

  std::string       _fontstring;

  bool              _shadow;
  unsigned char     _offset;
  unsigned char     _tint;

  XftFont          *_xftfont;

  bool createXftFont(void);
  
public:
  // loads an Xft font
  Font(int screen_num, const std::string &fontstring, bool shadow,
        unsigned char offset, unsigned char tint);
  virtual ~Font();

  inline const std::string &fontstring() const { return _fontstring; }

  int height() const;
  int maxCharWidth() const;

  int measureString(const ustring &string) const;

  // The RenderControl classes use the internal data to render the fonts, but
  // noone else needs it, so its private.
  friend class RenderControl;
};

}

#endif // __font_hh
