// -*- mode: C++; indent-tabs-mode: nil; -*-
// Image.hh for Blackbox - an X11 Window manager
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

#ifndef   __Image_hh
#define   __Image_hh

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include <list>

#include "Timer.hh"
#include "BaseDisplay.hh"
#include "Color.hh"

class BImageControl;
class BTexture;
class BImageCache;

class BImage {
private:
  BImageControl *control;
  bool interlaced;
  XColor *colors;

  BColor from, to;
  int red_offset, green_offset, blue_offset, red_bits, green_bits, blue_bits,
    ncolors, cpc, cpccpc;
  unsigned char *red, *green, *blue, *red_table, *green_table, *blue_table;
  unsigned int width, height, *xtable, *ytable;

  void TrueColorDither(unsigned int bit_depth, int bytes_per_line,
		       unsigned char *pixel_data);
  void PseudoColorDither(int bytes_per_line, unsigned char *pixel_data);
#ifdef ORDEREDPSEUDO
  void OrderedPseudoColorDither(int bytes_per_line, unsigned char *pixel_data);
#endif

  Pixmap renderPixmap(void);

  XImage *renderXImage(void);

  void invert(void);
  void bevel1(void);
  void bevel2(void);
  void dgradient(void);
  void egradient(void);
  void hgradient(void);
  void pgradient(void);
  void rgradient(void);
  void vgradient(void);
  void cdgradient(void);
  void pcgradient(void);


public:
  BImage(BImageControl *c, unsigned int w, unsigned int h);
  ~BImage(void);

  Pixmap render(const BTexture &texture);
  Pixmap render_solid(const BTexture &texture);
  Pixmap render_gradient(const BTexture &texture);

  // static methods for the builtin cache
  static unsigned long maximumCacheSize(void);
  static void setMaximumCacheSize(const unsigned long cache_max);

  static unsigned long cacheTimeout(void);
  static void setCacheTimeout(const unsigned long cache_timeout);

private:
  // global image cache
  static BImageCache *imagecache;
};


class BImageControl : public TimeoutHandler {
public:
  struct CachedImage {
    Pixmap pixmap;

    unsigned int count, width, height;
    unsigned long pixel1, pixel2, texture;
  };

  BImageControl(BaseDisplay *dpy, const ScreenInfo *scrn,
                bool _dither= False, int _cpc = 4,
                unsigned long cache_timeout = 300000l,
                unsigned long cmax = 200l);
  virtual ~BImageControl(void);

  inline BaseDisplay *getBaseDisplay(void) const { return basedisplay; }

  inline bool doDither(void) { return dither; }

  inline const ScreenInfo *getScreenInfo(void) { return screeninfo; }

  inline Window getDrawable(void) const { return window; }

  inline Visual *getVisual(void) { return screeninfo->getVisual(); }

  inline int getBitsPerPixel(void) const { return bits_per_pixel; }
  inline int getDepth(void) const { return screen_depth; }
  inline int getColorsPerChannel(void) const
    { return colors_per_channel; }

  unsigned long getSqrt(unsigned int x);

  Pixmap renderImage(unsigned int width, unsigned int height,
                     const BTexture &texture);

  void installRootColormap(void);
  void removeImage(Pixmap pixmap);
  void getColorTables(unsigned char **rmt, unsigned char **gmt,
                      unsigned char **bmt,
                      int *roff, int *goff, int *boff,
                      int *rbit, int *gbit, int *bbit);
  void getXColorTable(XColor **c, int *n);
  void getGradientBuffers(unsigned int w, unsigned int h,
                          unsigned int **xbuf, unsigned int **ybuf);
  void setDither(bool d) { dither = d; }
  void setColorsPerChannel(int cpc);

  virtual void timeout(void);

private:
  bool dither;
  BaseDisplay *basedisplay;
  const ScreenInfo *screeninfo;
#ifdef    TIMEDCACHE
  BTimer *timer;
#endif // TIMEDCACHE

  Colormap colormap;

  Window window;
  XColor *colors;
  int colors_per_channel, ncolors, screen_number, screen_depth,
    bits_per_pixel, red_offset, green_offset, blue_offset,
    red_bits, green_bits, blue_bits;
  unsigned char red_color_table[256], green_color_table[256],
    blue_color_table[256];
  unsigned int *grad_xbuffer, *grad_ybuffer, grad_buffer_width,
    grad_buffer_height;
  unsigned long *sqrt_table, cache_max;

  typedef std::list<CachedImage> CacheContainer;
  CacheContainer cache;

  Pixmap searchCache(const unsigned int width, const unsigned int height,
                     const unsigned long texture,
                     const BColor &c1, const BColor &c2);
};


#endif // __Image_hh

