// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Texture.hh for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh at debian.org>
// Copyright (c) 1997 - 2000, 2002 Bradley T Hughes <bhughes at trolltech.com>
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

#ifndef TEXTURE_HH
#define TEXTURE_HH

#include "Color.hh"
#include "Util.hh"
class BImageControl;

#include <string>

class BTexture {
public:
  enum Type {
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
    // inverted image
    Invert              = (1l<<15),
    // parent relative image
    Parent_Relative     = (1l<<16),
    // fake interlaced image
    Interlaced          = (1l<<17)
  };

  BTexture(const BaseDisplay * const _display = 0,
           unsigned int _screen = ~(0u), BImageControl* _ctrl = 0);
  BTexture(const std::string &_description,
           const BaseDisplay * const _display = 0,
           unsigned int _screen = ~(0u), BImageControl* _ctrl = 0);

  void setColor(const BColor &_color);
  void setColorTo(const BColor &_colorTo) { ct = _colorTo; }

  const BColor &color(void) const { return c; }
  const BColor &colorTo(void) const { return ct; }
  const BColor &lightColor(void) const { return lc; }
  const BColor &shadowColor(void) const { return sc; }

  unsigned long texture(void) const { return t; }
  void setTexture(const unsigned long _texture) { t  = _texture; }
  void addTexture(const unsigned long _texture) { t |= _texture; }

  BTexture &operator=(const BTexture &tt);
  inline bool operator==(const BTexture &tt)
  { return (c == tt.c && ct == tt.ct && lc == tt.lc &&
            sc == tt.sc && t == tt.t); }
  inline bool operator!=(const BTexture &tt)
  { return (! operator==(tt)); }

  const BaseDisplay *display(void) const { return dpy; }
  unsigned int screen(void) const { return scrn; }
  void setDisplay(const BaseDisplay * const _display,
                  const unsigned int _screen);
  void setImageControl(BImageControl* _ctrl) { ctrl = _ctrl; }
  const std::string &description(void) const { return descr; }
  void setDescription(const std::string &d);

  Pixmap render(const unsigned int width, const unsigned int height,
                const Pixmap old = 0);

private:
  BColor c, ct, lc, sc;
  std::string descr;
  unsigned long t;
  const BaseDisplay *dpy;
  BImageControl *ctrl;
  unsigned int scrn;
};

#endif // TEXTURE_HH
