#include "font.h"

#include "../src/gettext.h"
#define _(str) gettext(str)

font_open(const std::string &fontstring,
             bool shadow, unsigned char offset, unsigned char tint)
{
  assert(screen_num >= 0);
  assert(tint <= CHAR_MAX);
  
  if (!_xft_init) {
    if (!XftInit(0)) {
      printf(_("Couldn't initialize Xft.\n\n"));
      ::exit(3);
    }
#ifdef DEBUG
    int version = XftGetVersion();
    printf("Using Xft %d.%d.%d (Built against %d.%d.%d).\n",
           version / 10000 % 100, version / 100 % 100, version % 100,
           XFT_MAJOR, XFT_MINOR, XFT_REVISION);
#endif
    _xft_init = true;
  }

  if ((_xftfont = XftFontOpenName(ob_display, _screen_num,
                                  fontstring)))
    return;

  printf(_("Unable to load font: %s\n"), _fontstring.c_str());
  printf(_("Trying fallback font: %s\n"), "fixed");

  if ((_xftfont = XftFontOpenName(ob_display, _screen_num,
                                  "fixed")))
    return;

  printf(_("Unable to load font: %s\n"), "fixed");
  printf(_("Aborting!.\n"));

  exit(3); // can't continue without a font
}


destroy_fonts(void)
{
  if (_xftfont)
    XftFontClose(ob_display, _xftfont);
}


int font_measure_string(const char *)
{
  XGlyphInfo info;

  if (string.utf8())
    XftTextExtentsUtf8(**display, _xftfont,
                       (FcChar8*)string.c_str(), string.bytes(), &info);
  else
    XftTextExtents8(ob_display, _xftfont,
                    (FcChar8*)string.c_str(), string.bytes(), &info);

  return (signed) info.xOff + (_shadow ? _offset : 0);
}


int font_height(void)
{
  return (signed) _xftfont->height + (_shadow ? _offset : 0);
}


int font_max_char_width(void)
{
  return (signed) _xftfont->max_advance_width;
}
