// -*- mode: C++; indent-tabs-mode: nil; -*-
// ImageControl.cc for Blackbox - an X11 Window manager
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

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_CTYPE_H
#  include <ctype.h>
#endif // HAVE_CTYPE_H

#include <X11/Xlib.h>
}

#include <algorithm>

#include "blackbox.hh"
#include "i18n.hh"
#include "BaseDisplay.hh"
#include "Color.hh"
#include "Image.hh"
#include "Texture.hh"

static unsigned long bsqrt(unsigned long x) {
  if (x <= 0) return 0;
  if (x == 1) return 1;

  unsigned long r = x >> 1;
  unsigned long q;

  while (1) {
    q = x / r;
    if (q >= r) return r;
    r = (r + q) >> 1;
  }
}

BImageControl *ctrl = 0;

BImageControl::BImageControl(BaseDisplay *dpy, const ScreenInfo *scrn,
                             bool _dither, int _cpc,
                             unsigned long cache_timeout,
                             unsigned long cmax)
{
  if (! ctrl) ctrl = this;

  basedisplay = dpy;
  screeninfo = scrn;
  setDither(_dither);
  setColorsPerChannel(_cpc);

  cache_max = cmax;
#ifdef    TIMEDCACHE
  if (cache_timeout) {
    timer = new BTimer(basedisplay, this);
    timer->setTimeout(cache_timeout);
    timer->start();
  } else {
    timer = (BTimer *) 0;
  }
#endif // TIMEDCACHE

  colors = (XColor *) 0;
  ncolors = 0;

  grad_xbuffer = grad_ybuffer = (unsigned int *) 0;
  grad_buffer_width = grad_buffer_height = 0;

  sqrt_table = (unsigned long *) 0;

  screen_depth = screeninfo->getDepth();
  window = screeninfo->getRootWindow();
  screen_number = screeninfo->getScreenNumber();
  colormap = screeninfo->getColormap();

  int count;
  XPixmapFormatValues *pmv = XListPixmapFormats(basedisplay->getXDisplay(),
                                                &count);
  if (pmv) {
    bits_per_pixel = 0;
    for (int i = 0; i < count; i++)
      if (pmv[i].depth == screen_depth) {
	bits_per_pixel = pmv[i].bits_per_pixel;
	break;
      }

    XFree(pmv);
  }

  if (bits_per_pixel == 0) bits_per_pixel = screen_depth;
  if (bits_per_pixel >= 24) setDither(False);

  red_offset = green_offset = blue_offset = 0;

  switch (getVisual()->c_class) {
  case TrueColor: {
    int i;

    // compute color tables
    unsigned long red_mask = getVisual()->red_mask,
      green_mask = getVisual()->green_mask,
      blue_mask = getVisual()->blue_mask;

    while (! (red_mask & 1)) { red_offset++; red_mask >>= 1; }
    while (! (green_mask & 1)) { green_offset++; green_mask >>= 1; }
    while (! (blue_mask & 1)) { blue_offset++; blue_mask >>= 1; }

    red_bits = 255 / red_mask;
    green_bits = 255 / green_mask;
    blue_bits = 255 / blue_mask;

    for (i = 0; i < 256; i++) {
      red_color_table[i] = i / red_bits;
      green_color_table[i] = i / green_bits;
      blue_color_table[i] = i / blue_bits;
    }
    break;
  }

  case PseudoColor:
  case StaticColor: {
    ncolors = colors_per_channel * colors_per_channel * colors_per_channel;

    if (ncolors > (1 << screen_depth)) {
      colors_per_channel = (1 << screen_depth) / 3;
      ncolors = colors_per_channel * colors_per_channel * colors_per_channel;
    }

    if (colors_per_channel < 2 || ncolors > (1 << screen_depth)) {
      fprintf(stderr,
	      i18n(ImageSet, ImageInvalidColormapSize,
                   "BImageControl::BImageControl: invalid colormap size %d "
                   "(%d/%d/%d) - reducing"),
	      ncolors, colors_per_channel, colors_per_channel,
	      colors_per_channel);

      colors_per_channel = (1 << screen_depth) / 3;
    }

    colors = new XColor[ncolors];
    if (! colors) {
      fprintf(stderr, i18n(ImageSet, ImageErrorAllocatingColormap,
                           "BImageControl::BImageControl: error allocating "
                           "colormap\n"));
      exit(1);
    }

    int i = 0, ii, p, r, g, b,

#ifdef ORDEREDPSEUDO
      bits = 256 / colors_per_channel;
#else // !ORDEREDPSEUDO
    bits = 255 / (colors_per_channel - 1);
#endif // ORDEREDPSEUDO

    red_bits = green_bits = blue_bits = bits;

    for (i = 0; i < 256; i++)
      red_color_table[i] = green_color_table[i] = blue_color_table[i] =
	i / bits;

    for (r = 0, i = 0; r < colors_per_channel; r++)
      for (g = 0; g < colors_per_channel; g++)
	for (b = 0; b < colors_per_channel; b++, i++) {
	  colors[i].red = (r * 0xffff) / (colors_per_channel - 1);
	  colors[i].green = (g * 0xffff) / (colors_per_channel - 1);
	  colors[i].blue = (b * 0xffff) / (colors_per_channel - 1);;
	  colors[i].flags = DoRed|DoGreen|DoBlue;
	}

    for (i = 0; i < ncolors; i++) {
      if (! XAllocColor(basedisplay->getXDisplay(), colormap, &colors[i])) {
	fprintf(stderr, i18n(ImageSet, ImageColorAllocFail,
                             "couldn't alloc color %i %i %i\n"),
		colors[i].red, colors[i].green, colors[i].blue);
	colors[i].flags = 0;
      } else {
	colors[i].flags = DoRed|DoGreen|DoBlue;
      }
    }

    XColor icolors[256];
    int incolors = (((1 << screen_depth) > 256) ? 256 : (1 << screen_depth));

    for (i = 0; i < incolors; i++)
      icolors[i].pixel = i;

    XQueryColors(basedisplay->getXDisplay(), colormap, icolors, incolors);
    for (i = 0; i < ncolors; i++) {
      if (! colors[i].flags) {
	unsigned long chk = 0xffffffff, pixel, close = 0;

	p = 2;
	while (p--) {
	  for (ii = 0; ii < incolors; ii++) {
	    r = (colors[i].red - icolors[i].red) >> 8;
	    g = (colors[i].green - icolors[i].green) >> 8;
	    b = (colors[i].blue - icolors[i].blue) >> 8;
	    pixel = (r * r) + (g * g) + (b * b);

	    if (pixel < chk) {
	      chk = pixel;
	      close = ii;
	    }

	    colors[i].red = icolors[close].red;
	    colors[i].green = icolors[close].green;
	    colors[i].blue = icolors[close].blue;

	    if (XAllocColor(basedisplay->getXDisplay(), colormap,
			    &colors[i])) {
	      colors[i].flags = DoRed|DoGreen|DoBlue;
	      break;
	    }
	  }
	}
      }
    }

    break;
  }

  case GrayScale:
  case StaticGray: {
    if (getVisual()->c_class == StaticGray) {
      ncolors = 1 << screen_depth;
    } else {
      ncolors = colors_per_channel * colors_per_channel * colors_per_channel;

      if (ncolors > (1 << screen_depth)) {
	colors_per_channel = (1 << screen_depth) / 3;
	ncolors =
	  colors_per_channel * colors_per_channel * colors_per_channel;
      }
    }

    if (colors_per_channel < 2 || ncolors > (1 << screen_depth)) {
      fprintf(stderr,
              i18n(ImageSet, ImageInvalidColormapSize,
	           "BImageControl::BImageControl: invalid colormap size %d "
	            "(%d/%d/%d) - reducing"),
	      ncolors, colors_per_channel, colors_per_channel,
	      colors_per_channel);

      colors_per_channel = (1 << screen_depth) / 3;
    }

    colors = new XColor[ncolors];
    if (! colors) {
      fprintf(stderr,
              i18n(ImageSet, ImageErrorAllocatingColormap,
                 "BImageControl::BImageControl: error allocating colormap\n"));
      exit(1);
    }

    int i = 0, ii, p, bits = 255 / (colors_per_channel - 1);
    red_bits = green_bits = blue_bits = bits;

    for (i = 0; i < 256; i++)
      red_color_table[i] = green_color_table[i] = blue_color_table[i] =
	i / bits;

    for (i = 0; i < ncolors; i++) {
      colors[i].red = (i * 0xffff) / (colors_per_channel - 1);
      colors[i].green = (i * 0xffff) / (colors_per_channel - 1);
      colors[i].blue = (i * 0xffff) / (colors_per_channel - 1);;
      colors[i].flags = DoRed|DoGreen|DoBlue;

      if (! XAllocColor(basedisplay->getXDisplay(), colormap,
			&colors[i])) {
	fprintf(stderr, i18n(ImageSet, ImageColorAllocFail,
			     "couldn't alloc color %i %i %i\n"),
		colors[i].red, colors[i].green, colors[i].blue);
	colors[i].flags = 0;
      } else {
	colors[i].flags = DoRed|DoGreen|DoBlue;
      }
    }

    XColor icolors[256];
    int incolors = (((1 << screen_depth) > 256) ? 256 :
		    (1 << screen_depth));

    for (i = 0; i < incolors; i++)
      icolors[i].pixel = i;

    XQueryColors(basedisplay->getXDisplay(), colormap, icolors, incolors);
    for (i = 0; i < ncolors; i++) {
      if (! colors[i].flags) {
	unsigned long chk = 0xffffffff, pixel, close = 0;

	p = 2;
	while (p--) {
	  for (ii = 0; ii < incolors; ii++) {
	    int r = (colors[i].red - icolors[i].red) >> 8;
	    int g = (colors[i].green - icolors[i].green) >> 8;
	    int b = (colors[i].blue - icolors[i].blue) >> 8;
	    pixel = (r * r) + (g * g) + (b * b);

	    if (pixel < chk) {
	      chk = pixel;
	      close = ii;
	    }

	    colors[i].red = icolors[close].red;
	    colors[i].green = icolors[close].green;
	    colors[i].blue = icolors[close].blue;

	    if (XAllocColor(basedisplay->getXDisplay(), colormap,
			    &colors[i])) {
	      colors[i].flags = DoRed|DoGreen|DoBlue;
	      break;
	    }
	  }
	}
      }
    }

    break;
  }

  default:
    fprintf(stderr,
            i18n(ImageSet, ImageUnsupVisual,
                 "BImageControl::BImageControl: unsupported visual %d\n"),
	    getVisual()->c_class);
    exit(1);
  }
}


BImageControl::~BImageControl(void) {
  delete [] sqrt_table;

  delete [] grad_xbuffer;

  delete [] grad_ybuffer;

  if (colors) {
    unsigned long *pixels = new unsigned long [ncolors];

    int i;
    for (i = 0; i < ncolors; i++)
      *(pixels + i) = (*(colors + i)).pixel;

    XFreeColors(basedisplay->getXDisplay(), colormap, pixels, ncolors, 0);

    delete [] colors;
  }

  if (!cache.empty()) {
    //#ifdef DEBUG
    fprintf(stderr, i18n(ImageSet, ImagePixmapRelease,
		         "BImageContol::~BImageControl: pixmap cache - "
	                 "releasing %d pixmaps\n"), cache.size());
    //#endif
    CacheContainer::iterator it = cache.begin();
    const CacheContainer::iterator end = cache.end();
    for (; it != end; ++it) {
      XFreePixmap(basedisplay->getXDisplay(), (*it).pixmap);
    }
  }
#ifdef    TIMEDCACHE
  if (timer) {
    timer->stop();
    delete timer;
  }
#endif // TIMEDCACHE
}


Pixmap BImageControl::searchCache(const unsigned int width,
                                  const unsigned int height,
                                  const unsigned long texture,
                                  const BColor &c1, const BColor &c2) {
  if (cache.empty())
    return None;

  CacheContainer::iterator it = cache.begin();
  const CacheContainer::iterator end = cache.end();
  for (; it != end; ++it) {
    CachedImage& tmp = *it;
    if ((tmp.width == width) && (tmp.height == height) &&
        (tmp.texture == texture) && (tmp.pixel1 == c1.pixel()))
      if (texture & BTexture::Gradient) {
        if (tmp.pixel2 == c2.pixel()) {
          tmp.count++;
          return tmp.pixmap;
        }
      } else {
        tmp.count++;
        return tmp.pixmap;
      }
  }
  return None;
}


Pixmap BImageControl::renderImage(unsigned int width, unsigned int height,
                                  const BTexture &texture) {
  if (texture.texture() & BTexture::Parent_Relative) return ParentRelative;

  Pixmap pixmap = searchCache(width, height, texture.texture(),
			      texture.color(), texture.colorTo());
  if (pixmap) return pixmap;

  BImage image(this, width, height);
  pixmap = image.render(texture);

  if (!pixmap)
    return None;

  CachedImage tmp;

  tmp.pixmap = pixmap;
  tmp.width = width;
  tmp.height = height;
  tmp.count = 1;
  tmp.texture = texture.texture();
  tmp.pixel1 = texture.color().pixel();

  if (texture.texture() & BTexture::Gradient)
    tmp.pixel2 = texture.colorTo().pixel();
  else
    tmp.pixel2 = 0l;

  cache.push_back(tmp);

  if (cache.size() > cache_max) {
#ifdef    DEBUG
    fprintf(stderr, i18n(ImageSet, ImagePixmapCacheLarge,
			 "BImageControl::renderImage: cache is large, "
			 "forcing cleanout\n"));
#endif // DEBUG

    timeout();
  }

  return pixmap;
}


void BImageControl::removeImage(Pixmap pixmap) {
  if (!pixmap)
    return;

  CacheContainer::iterator it = cache.begin();
  const CacheContainer::iterator end = cache.end();
  for (; it != end; ++it) {
    CachedImage &tmp = *it;
    if (tmp.pixmap == pixmap && tmp.count > 0)
      tmp.count--;
  }

#ifdef    TIMEDCACHE
  if (! timer)
#endif // TIMEDCACHE
    timeout();
}


void BImageControl::getColorTables(unsigned char **rmt, unsigned char **gmt,
				   unsigned char **bmt,
				   int *roff, int *goff, int *boff,
                                   int *rbit, int *gbit, int *bbit) {
  if (rmt) *rmt = red_color_table;
  if (gmt) *gmt = green_color_table;
  if (bmt) *bmt = blue_color_table;

  if (roff) *roff = red_offset;
  if (goff) *goff = green_offset;
  if (boff) *boff = blue_offset;

  if (rbit) *rbit = red_bits;
  if (gbit) *gbit = green_bits;
  if (bbit) *bbit = blue_bits;
}


void BImageControl::getXColorTable(XColor **c, int *n) {
  if (c) *c = colors;
  if (n) *n = ncolors;
}


void BImageControl::getGradientBuffers(unsigned int w,
				       unsigned int h,
				       unsigned int **xbuf,
				       unsigned int **ybuf)
{
  if (w > grad_buffer_width) {
    if (grad_xbuffer) {
      delete [] grad_xbuffer;
    }

    grad_buffer_width = w;

    grad_xbuffer = new unsigned int[grad_buffer_width * 3];
  }

  if (h > grad_buffer_height) {
    if (grad_ybuffer) {
      delete [] grad_ybuffer;
    }

    grad_buffer_height = h;

    grad_ybuffer = new unsigned int[grad_buffer_height * 3];
  }

  *xbuf = grad_xbuffer;
  *ybuf = grad_ybuffer;
}


void BImageControl::installRootColormap(void) {
  int ncmap = 0;
  Colormap *cmaps =
    XListInstalledColormaps(basedisplay->getXDisplay(), window, &ncmap);

  if (cmaps) {
    bool install = True;
    for (int i = 0; i < ncmap; i++)
      if (*(cmaps + i) == colormap)
	install = False;

    if (install)
      XInstallColormap(basedisplay->getXDisplay(), colormap);

    XFree(cmaps);
  }
}


void BImageControl::setColorsPerChannel(int cpc) {
  if (cpc < 2) cpc = 2;
  if (cpc > 6) cpc = 6;

  colors_per_channel = cpc;
}


unsigned long BImageControl::getSqrt(unsigned int x) {
  if (! sqrt_table) {
    // build sqrt table for use with elliptic gradient

    sqrt_table = new unsigned long[(256 * 256 * 2) + 1];

    for (int i = 0; i < (256 * 256 * 2); i++)
      *(sqrt_table + i) = bsqrt(i);
  }

  return (*(sqrt_table + x));
}


struct ZeroRefCheck {
  inline bool operator()(const BImageControl::CachedImage &image) const {
    return (image.count == 0);
  }
};

struct CacheCleaner {
  Display *display;
  ZeroRefCheck ref_check;
  CacheCleaner(Display *d): display(d) {}
  inline void operator()(const BImageControl::CachedImage& image) const {
    if (ref_check(image))
      XFreePixmap(display, image.pixmap);
  }
};


void BImageControl::timeout(void) {
  CacheCleaner cleaner(basedisplay->getXDisplay());
  std::for_each(cache.begin(), cache.end(), cleaner);
  cache.remove_if(cleaner.ref_check);
}

