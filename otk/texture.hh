// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef TEXTURE_HH
#define TEXTURE_HH

#include "color.hh"
#include "util.hh"

#include <string>

namespace otk {

class ImageControl;

class Texture {
public:
  enum Type {
    // No texture
    NoTexture           = (0),    
    // bevel options
    Flat                = (1l<<0),
    Sunken              = (1l<<1),
    Raised              = (1l<<2),
    // textures
    Solid               = (1l<<3),
    Gradient            = (1l<<4),
    // gradients
    Horizontal          = (1l<<5),
    Vertical            = (1l<<6),
    Diagonal            = (1l<<7),
    CrossDiagonal       = (1l<<8),
    Rectangle           = (1l<<9),
    Pyramid             = (1l<<10),
    PipeCross           = (1l<<11),
    Elliptic            = (1l<<12),
    // bevel types
    Bevel1              = (1l<<13),
    Bevel2              = (1l<<14),
    // flat border
    Border              = (1l<<15),
    // inverted image
    Invert              = (1l<<16),
    // parent relative image
    Parent_Relative     = (1l<<17),
    // fake interlaced image
    Interlaced          = (1l<<18)
  };

  Texture(unsigned int _screen = ~(0u), ImageControl* _ctrl = 0);
  Texture(const std::string &_description,
           unsigned int _screen = ~(0u), ImageControl* _ctrl = 0);

  void setColor(const Color &_color);
  void setColorTo(const Color &_colorTo) { ct = _colorTo; }
  void setBorderColor(const Color &_borderColor) { bc = _borderColor; }

  const Color &color(void) const { return c; }
  const Color &colorTo(void) const { return ct; }
  const Color &lightColor(void) const { return lc; }
  const Color &shadowColor(void) const { return sc; }
  const Color &borderColor(void) const { return bc; }

  unsigned long texture(void) const { return t; }
  void setTexture(const unsigned long _texture) { t  = _texture; }
  void addTexture(const unsigned long _texture) { t |= _texture; }

#ifndef SWIG
  Texture &operator=(const Texture &tt);
#endif
  inline bool operator==(const Texture &tt)
  { return (c == tt.c && ct == tt.ct && lc == tt.lc &&
            sc == tt.sc && t == tt.t); }
  inline bool operator!=(const Texture &tt)
  { return (! operator==(tt)); }

  unsigned int screen(void) const { return scrn; }
  void setScreen(const unsigned int _screen);
  void setImageControl(ImageControl* _ctrl) { ctrl = _ctrl; }
  const std::string &description(void) const { return descr; }
  void setDescription(const std::string &d);

  Pixmap render(const unsigned int width, const unsigned int height,
                const Pixmap old = 0);

private:
  Color c, ct, lc, sc, bc;
  std::string descr;
  unsigned long t;
  ImageControl *ctrl;
  unsigned int scrn;
};

}

#endif // TEXTURE_HH
