// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __font_hh
#define   __font_hh

#include "userstring.hh"

extern "C" {
#include <X11/Xlib.h>
#define _XFT_NO_COMPAT_ // no Xft 1 API
#include <X11/Xft/Xft.h>
}

#include <assert.h>

#include <string>

namespace otk {

class Color;

class Font {
  /*
   * static members
   */
private:
  static std::string  _fallback_font;
  static bool         _xft_init;

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

  unsigned int height() const;
  unsigned int maxCharWidth() const;

  //! Measures the length of a string
  /*!
    @param string The string to measure, it should be UTF8 encoded.
  */
  unsigned int measureString(const userstring &string) const;

  //! Draws a string into an XftDraw object
  /*!
    Be Warned: If you use an XftDraw object and a color, or a font from
    different screens, you WILL have unpredictable results! :)
    @param d The drawable to render into.
    @param x The X offset onto the drawable at which to start drawing.
    @param x The Y offset onto the drawable at which to start drawing.
    @param color The color to use for drawing the text.
    @param string The string to draw, it should be UTF8 encoded.
  */
  void drawString(XftDraw *d, int x, int y, const Color &color,
                  const userstring &string) const;
};

}

#endif // __font_hh
