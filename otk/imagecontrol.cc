// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

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

#include "display.hh"
#include "color.hh"
#include "image.hh"
#include "texture.hh"

namespace otk {

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

ImageControl *ctrl = 0;

ImageControl::ImageControl(TimerQueueManager *timermanager,
                             const ScreenInfo *scrn,
                             bool _dither, int _cpc,
                             unsigned long cache_timeout,
                             unsigned long cmax) {
  if (! ctrl) ctrl = this;

  screeninfo = scrn;
  setDither(_dither);
  setColorsPerChannel(_cpc);

  cache_max = cmax;
  if (cache_timeout) {
    timer = new Timer(timermanager, (TimeoutHandler)timeout, this);
    timer->setTimeout(cache_timeout);
    timer->start();
  } else {
    timer = (Timer *) 0;
  }

  colors = (XColor *) 0;
  ncolors = 0;

  grad_xbuffer = grad_ybuffer = (unsigned int *) 0;
  grad_buffer_width = grad_buffer_height = 0;

  sqrt_table = (unsigned long *) 0;

  screen_depth = screeninfo->depth();
  window = screeninfo->rootWindow();
  screen_number = screeninfo->screen();
  colormap = screeninfo->colormap();

  int count;
  XPixmapFormatValues *pmv = XListPixmapFormats(Display::display,
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
	      "ImageControl::ImageControl: invalid colormap size %d "
              "(%d/%d/%d) - reducing",
	      ncolors, colors_per_channel, colors_per_channel,
	      colors_per_channel);

      colors_per_channel = (1 << screen_depth) / 3;
    }

    colors = new XColor[ncolors];
    if (! colors) {
      fprintf(stderr, "ImageControl::ImageControl: error allocating "
              "colormap\n");
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
      if (! XAllocColor(Display::display, colormap, &colors[i])) {
	fprintf(stderr, "couldn't alloc color %i %i %i\n",
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

    XQueryColors(Display::display, colormap, icolors, incolors);
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

	    if (XAllocColor(Display::display, colormap,
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
              "ImageControl::ImageControl: invalid colormap size %d "
              "(%d/%d/%d) - reducing",
	      ncolors, colors_per_channel, colors_per_channel,
	      colors_per_channel);

      colors_per_channel = (1 << screen_depth) / 3;
    }

    colors = new XColor[ncolors];
    if (! colors) {
      fprintf(stderr,
              "ImageControl::ImageControl: error allocating colormap\n");
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

      if (! XAllocColor(Display::display, colormap,
			&colors[i])) {
	fprintf(stderr, "couldn't alloc color %i %i %i\n",
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

    XQueryColors(Display::display, colormap, icolors, incolors);
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

	    if (XAllocColor(Display::display, colormap,
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
    fprintf(stderr, "ImageControl::ImageControl: unsupported visual %d\n",
	    getVisual()->c_class);
    exit(1);
  }
}


ImageControl::~ImageControl(void) {
  delete [] sqrt_table;

  delete [] grad_xbuffer;

  delete [] grad_ybuffer;

  if (colors) {
    unsigned long *pixels = new unsigned long [ncolors];

    for (int i = 0; i < ncolors; i++)
      *(pixels + i) = (*(colors + i)).pixel;

    XFreeColors(Display::display, colormap, pixels, ncolors, 0);

    delete [] colors;
  }

  if (! cache.empty()) {
    //#ifdef DEBUG
    fprintf(stderr, "ImageContol::~ImageControl: pixmap cache - "
            "releasing %d pixmaps\n", cache.size());
    //#endif
    CacheContainer::iterator it = cache.begin();
    const CacheContainer::iterator end = cache.end();
    for (; it != end; ++it)
      XFreePixmap(Display::display, it->pixmap);
  }
  if (timer) {
    timer->stop();
    delete timer;
  }
}


Pixmap ImageControl::searchCache(const unsigned int width,
                                  const unsigned int height,
                                  const unsigned long texture,
                                  const Color &c1, const Color &c2) {
  if (cache.empty())
    return None;

  CacheContainer::iterator it = cache.begin();
  const CacheContainer::iterator end = cache.end();
  for (; it != end; ++it) {
    CachedImage& tmp = *it;
    if (tmp.width == width && tmp.height == height &&
        tmp.texture == texture && tmp.pixel1 == c1.pixel())
      if (texture & Texture::Gradient) {
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


Pixmap ImageControl::renderImage(unsigned int width, unsigned int height,
                                  const Texture &texture) {
  if (texture.texture() & Texture::Parent_Relative) return ParentRelative;

  Pixmap pixmap = searchCache(width, height, texture.texture(),
			      texture.color(), texture.colorTo());
  if (pixmap) return pixmap;

  Image image(this, width, height);
  pixmap = image.render(texture);

  if (! pixmap)
    return None;

  CachedImage tmp;

  tmp.pixmap = pixmap;
  tmp.width = width;
  tmp.height = height;
  tmp.count = 1;
  tmp.texture = texture.texture();
  tmp.pixel1 = texture.color().pixel();

  if (texture.texture() & Texture::Gradient)
    tmp.pixel2 = texture.colorTo().pixel();
  else
    tmp.pixel2 = 0l;

  cache.push_back(tmp);

  if (cache.size() > cache_max) {
#ifdef    DEBUG
    fprintf(stderr, "ImageControl::renderImage: cache is large, "
      "forcing cleanout\n");
#endif // DEBUG

    timeout(this);
  }

  return pixmap;
}


void ImageControl::removeImage(Pixmap pixmap) {
  if (! pixmap)
    return;

  CacheContainer::iterator it = cache.begin();
  const CacheContainer::iterator end = cache.end();
  for (; it != end; ++it) {
    CachedImage &tmp = *it;
    if (tmp.pixmap == pixmap && tmp.count > 0)
      tmp.count--;
  }

  if (! timer)
    timeout(this);
}


void ImageControl::getColorTables(unsigned char **rmt, unsigned char **gmt,
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


void ImageControl::getXColorTable(XColor **c, int *n) {
  if (c) *c = colors;
  if (n) *n = ncolors;
}


void ImageControl::getGradientBuffers(unsigned int w,
				       unsigned int h,
				       unsigned int **xbuf,
				       unsigned int **ybuf)
{
  if (w > grad_buffer_width) {
    if (grad_xbuffer)
      delete [] grad_xbuffer;

    grad_buffer_width = w;

    grad_xbuffer = new unsigned int[grad_buffer_width * 3];
  }

  if (h > grad_buffer_height) {
    if (grad_ybuffer)
      delete [] grad_ybuffer;

    grad_buffer_height = h;

    grad_ybuffer = new unsigned int[grad_buffer_height * 3];
  }

  *xbuf = grad_xbuffer;
  *ybuf = grad_ybuffer;
}


void ImageControl::installRootColormap(void) {
  int ncmap = 0;
  Colormap *cmaps =
    XListInstalledColormaps(Display::display, window, &ncmap);

  if (cmaps) {
    bool install = True;
    for (int i = 0; i < ncmap; i++)
      if (*(cmaps + i) == colormap)
	install = False;

    if (install)
      XInstallColormap(Display::display, colormap);

    XFree(cmaps);
  }
}


void ImageControl::setColorsPerChannel(int cpc) {
  if (cpc < 2) cpc = 2;
  if (cpc > 6) cpc = 6;

  colors_per_channel = cpc;
}


unsigned long ImageControl::getSqrt(unsigned int x) {
  if (! sqrt_table) {
    // build sqrt table for use with elliptic gradient

    sqrt_table = new unsigned long[(256 * 256 * 2) + 1];

    for (int i = 0; i < (256 * 256 * 2); i++)
      *(sqrt_table + i) = bsqrt(i);
  }

  return (*(sqrt_table + x));
}


struct ZeroRefCheck {
  inline bool operator()(const ImageControl::CachedImage &image) const {
    return (image.count == 0);
  }
};

struct CacheCleaner {
  ZeroRefCheck ref_check;
  CacheCleaner() {}
  inline void operator()(const ImageControl::CachedImage& image) const {
    if (ref_check(image))
      XFreePixmap(Display::display, image.pixmap);
  }
};


void ImageControl::timeout(ImageControl *t) {
  CacheCleaner cleaner;
  std::for_each(t->cache.begin(), t->cache.end(), cleaner);
  t->cache.remove_if(cleaner.ref_check);
}

}
