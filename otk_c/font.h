// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __font_h
#define   __font_h

#include <X11/Xlib.h>
#define _XFT_NO_COMPAT_ // no Xft 1 API
#include <X11/Xft/Xft.h>
#include <Python.h>

extern PyTypeObject OtkFont_Type;

struct OtkColor;
struct ScreenInfo;

#define OTKFONTHEIGHT(font) (font->xftfont->height + \
                             (font->shadow ? font->offset : 0))
#define OTKFONTMAXCHARWIDTH(font) (font->xftfont->max_advance_width)

typedef struct OtkFont {
  PyObject_HEAD
  int               screen;
  Bool              shadow;
  unsigned char     offset;
  unsigned char     tint;
  XftFont          *xftfont;
} OtkFont;

void OtkFont_Initialize();

PyObject *OtkFont_New(int screen, const char *fontstring, Bool shadow,
		      unsigned char offset, unsigned char tint);

int OtkFont_MeasureString(OtkFont *self, const char *string);//, Bool utf8);

//! Draws a string into an XftDraw object
/*!
  Be Warned: If you use an XftDraw object and a color, or a font from
  different screens, you WILL have unpredictable results! :)
*/
void OtkFont_DrawString(OtkFont *self, XftDraw *d, int x, int y,
			struct OtkColor *color, const char *string);//, Bool utf8);

/*
  bool createXftFont(void);
  
public:
  // loads an Xft font
  BFont(int screen_num, const std::string &fontstring, bool shadow,
        unsigned char offset, unsigned char tint);
  virtual ~BFont();

  inline const std::string &fontstring() const { return _fontstring; }

  unsigned int height() const;
  unsigned int maxCharWidth() const;

  unsigned int measureString(const std::string &string,
                             bool utf8 = false) const;

};

}
*/
#endif // __font_h
