// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __font_hh
#define   __font_hh

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

  unsigned int measureString(const std::string &string,
                             bool utf8 = true) const;

  //! Draws a string into an XftDraw object
  /*!
    Be Warned: If you use an XftDraw object and a color, or a font from
    different screens, you WILL have unpredictable results! :)
  */
  void drawString(XftDraw *d, int x, int y, const Color &color,
                  const std::string &string, bool utf8 = true) const;
};

}

#endif // __font_hh
