// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <stdio.h>
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
}

#include <assert.h>

#include "texture.hh"
#include "display.hh"
#include "image.hh"

using std::string;

namespace otk {

Texture::Texture(unsigned int _screen, ImageControl* _ctrl)
  : c(_screen), ct(_screen),
    lc(_screen), sc(_screen), bc(_screen), t(0),
    ctrl(_ctrl), scrn(_screen) { }


Texture::Texture(const string &d,unsigned int _screen, ImageControl* _ctrl)
  : c(_screen), ct(_screen),
    lc(_screen), sc(_screen), bc(_screen), t(0),
    ctrl(_ctrl), scrn(_screen) {
  setDescription(d);
}


void Texture::setColor(const Color &cc) {
  c = cc;
  c.setScreen(screen());

  unsigned char r, g, b, rr, gg, bb;

  // calculate the light color
  r = c.red();
  g = c.green();
  b = c.blue();
  rr = r + (r >> 1);
  gg = g + (g >> 1);
  bb = b + (b >> 1);
  if (rr < r) rr = ~0;
  if (gg < g) gg = ~0;
  if (bb < b) bb = ~0;
  lc = Color(rr, gg, bb, screen());

  // calculate the shadow color
  r = c.red();
  g = c.green();
  b = c.blue();
  rr = (r >> 2) + (r >> 1);
  gg = (g >> 2) + (g >> 1);
  bb = (b >> 2) + (b >> 1);
  if (rr > r) rr = 0;
  if (gg > g) gg = 0;
  if (bb > b) bb = 0;
  sc = Color(rr, gg, bb, screen());
}


void Texture::setDescription(const string &d) {
  descr.erase();
  descr.reserve(d.length());

  string::const_iterator it = d.begin(), end = d.end();
  for (; it != end; ++it)
    descr += tolower(*it);

  if (descr.find("parentrelative") != string::npos) {
    setTexture(Texture::Parent_Relative);
  } else {
    setTexture(0);

    if (descr.find("gradient") != string::npos) {
      addTexture(Texture::Gradient);
      if (descr.find("crossdiagonal") != string::npos)
        addTexture(Texture::CrossDiagonal);
      else if (descr.find("rectangle") != string::npos)
        addTexture(Texture::Rectangle);
      else if (descr.find("pyramid") != string::npos)
        addTexture(Texture::Pyramid);
      else if (descr.find("pipecross") != string::npos)
        addTexture(Texture::PipeCross);
      else if (descr.find("elliptic") != string::npos)
        addTexture(Texture::Elliptic);
      else if (descr.find("horizontal") != string::npos)
        addTexture(Texture::Horizontal);
      else if (descr.find("vertical") != string::npos)
        addTexture(Texture::Vertical);
      else
        addTexture(Texture::Diagonal);
    } else {
      addTexture(Texture::Solid);
    }

    if (descr.find("sunken") != string::npos)
      addTexture(Texture::Sunken);
    else if (descr.find("flat") != string::npos)
      addTexture(Texture::Flat);
    else
      addTexture(Texture::Raised);

    if (texture() & Texture::Flat) {
      if (descr.find("border") != string::npos)
        addTexture(Texture::Border);
    } else {
      if (descr.find("bevel2") != string::npos)
        addTexture(Texture::Bevel2);
      else
        addTexture(Texture::Bevel1);
    }

    if (descr.find("interlaced") != string::npos)
      addTexture(Texture::Interlaced);
  }
}

void Texture::setScreen(const unsigned int _screen) {
  if (_screen == screen()) {
    // nothing to do
    return;
  }

  scrn = _screen;
  c.setScreen(_screen);
  ct.setScreen(_screen);
  lc.setScreen(_screen);
  sc.setScreen(_screen);
  bc.setScreen(_screen);
}


Texture& Texture::operator=(const Texture &tt) {
  c  = tt.c;
  ct = tt.ct;
  lc = tt.lc;
  sc = tt.sc;
  bc = tt.bc;
  descr = tt.descr;
  t  = tt.t;
  scrn = tt.scrn;
  ctrl = tt.ctrl;

  return *this;
}


Pixmap Texture::render(const unsigned int width, const unsigned int height,
                        const Pixmap old) {
  assert(texture() != Texture::NoTexture);

//  if (texture() == (Texture::Flat | Texture::Solid))
//    return None;
  if (texture() == Texture::Parent_Relative)
    return ParentRelative;

  if (screen() == ~(0u))
    scrn = DefaultScreen(**display);

  assert(ctrl != 0);
  Pixmap ret = ctrl->renderImage(width, height, *this);

  if (old)
    ctrl->removeImage(old);

  return ret;
}

}
