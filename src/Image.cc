// Image.cc for Openbox
// Copyright (c) 2001 Sean 'Shaleh' Perry <shaleh@debian.org>
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

// stupid macros needed to access some functions in version 2 of the GNU C
// library
#ifndef   _GNU_SOURCE
#define   _GNU_SOURCE
#endif // _GNU_SOURCE

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "i18n.h"
#include "BaseDisplay.h"
#include "Image.h"

#ifdef    HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif // HAVE_SYS_TYPES_H

#ifndef u_int32_t
#  ifdef uint_32_t
typedef uint32_t u_int32_t;
#  else
#    ifdef __uint32_t
typedef __uint32_t u_int32_t;
#    else
typedef unsigned int u_int32_t;
#    endif
#  endif
#endif

#ifdef    STDC_HEADERS
#  include <stdlib.h>
#  include <string.h>
#endif // STDC_HEADERS

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_CTYPE_H
#  include <ctype.h>
#endif // HAVE_CTYPE_H

#include <algorithm>
using namespace std;

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


BImage::BImage(BImageControl *c, unsigned int w, unsigned int h) {
  control = c;

  width = ((signed) w > 0) ? w : 1;
  height = ((signed) h > 0) ? h : 1;

  red = new unsigned char[width * height];
  green = new unsigned char[width * height];
  blue = new unsigned char[width * height];

  xtable = ytable = (unsigned int *) 0;

  cpc = control->getColorsPerChannel();
  cpccpc = cpc * cpc;

  control->getColorTables(&red_table, &green_table, &blue_table,
                          &red_offset, &green_offset, &blue_offset,
                          &red_bits, &green_bits, &blue_bits);

  if (control->getVisual()->c_class != TrueColor)
    control->getXColorTable(&colors, &ncolors);
}


BImage::~BImage(void) {
  if (red) delete [] red;
  if (green) delete [] green;
  if (blue) delete [] blue;
}


Pixmap BImage::render(BTexture *texture) {
  if (texture->getTexture() & BImage_ParentRelative)
    return ParentRelative;
  else if (texture->getTexture() & BImage_Solid)
    return render_solid(texture);
  else if (texture->getTexture() & BImage_Gradient)
    return render_gradient(texture);

  return None;
}


Pixmap BImage::render_solid(BTexture *texture) {
  Pixmap pixmap = XCreatePixmap(control->getBaseDisplay()->getXDisplay(),
				control->getDrawable(), width,
				height, control->getDepth());
  if (pixmap == None) {
    fprintf(stderr, i18n->getMessage(ImageSet, ImageErrorCreatingSolidPixmap,
		       "BImage::render_solid: error creating pixmap\n"));
    return None;
  }

  XGCValues gcv;
  GC gc, hgc, lgc;

  gcv.foreground = texture->getColor()->getPixel();
  gcv.fill_style = FillSolid;
  gc = XCreateGC(control->getBaseDisplay()->getXDisplay(), pixmap,
		 GCForeground | GCFillStyle, &gcv);

  gcv.foreground = texture->getHiColor()->getPixel();
  hgc = XCreateGC(control->getBaseDisplay()->getXDisplay(), pixmap,
		  GCForeground, &gcv);

  gcv.foreground = texture->getLoColor()->getPixel();
  lgc = XCreateGC(control->getBaseDisplay()->getXDisplay(), pixmap,
		  GCForeground, &gcv);

  XFillRectangle(control->getBaseDisplay()->getXDisplay(), pixmap, gc, 0, 0,
		 width, height);

#ifdef    INTERLACE
  if (texture->getTexture() & BImage_Interlaced) {
    gcv.foreground = texture->getColorTo()->getPixel();
    GC igc = XCreateGC(control->getBaseDisplay()->getXDisplay(), pixmap,
		       GCForeground, &gcv);

    register unsigned int i = 0;
    for (; i < height; i += 2)
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, igc,
		0, i, width, i);

    XFreeGC(control->getBaseDisplay()->getXDisplay(), igc);
  }
#endif // INTERLACE


  if (texture->getTexture() & BImage_Bevel1) {
    if (texture->getTexture() & BImage_Raised) {
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, lgc,
                0, height - 1, width - 1, height - 1);
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, lgc,
                width - 1, height - 1, width - 1, 0);

      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, hgc,
                0, 0, width - 1, 0);
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, hgc,
                0, height - 1, 0, 0);
    } else if (texture->getTexture() & BImage_Sunken) {
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, hgc,
                0, height - 1, width - 1, height - 1);
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, hgc,
                width - 1, height - 1, width - 1, 0);

      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, lgc,
                0, 0, width - 1, 0);
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, lgc,
                0, height - 1, 0, 0);
    }
  } else if (texture->getTexture() & BImage_Bevel2) {
    if (texture->getTexture() & BImage_Raised) {
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, lgc,
                1, height - 3, width - 3, height - 3);
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, lgc,
                width - 3, height - 3, width - 3, 1);

      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, hgc,
                1, 1, width - 3, 1);
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, hgc,
                1, height - 3, 1, 1);
    } else if (texture->getTexture() & BImage_Sunken) {
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, hgc,
                1, height - 3, width - 3, height - 3);
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, hgc,
                width - 3, height - 3, width - 3, 1);

      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, lgc,
                1, 1, width - 3, 1);
      XDrawLine(control->getBaseDisplay()->getXDisplay(), pixmap, lgc,
                1, height - 3, 1, 1);
    }
  }

  XFreeGC(control->getBaseDisplay()->getXDisplay(), gc);
  XFreeGC(control->getBaseDisplay()->getXDisplay(), hgc);
  XFreeGC(control->getBaseDisplay()->getXDisplay(), lgc);

  return pixmap;
}


Pixmap BImage::render_gradient(BTexture *texture) {
 int inverted = 0;

#ifdef    INTERLACE
  interlaced = texture->getTexture() & BImage_Interlaced;
#endif // INTERLACE

  if (texture->getTexture() & BImage_Sunken) {
    from = texture->getColorTo();
    to = texture->getColor();

    if (! (texture->getTexture() & BImage_Invert)) inverted = 1;
  } else {
    from = texture->getColor();
    to = texture->getColorTo();

    if (texture->getTexture() & BImage_Invert) inverted = 1;
  }

  control->getGradientBuffers(width, height, &xtable, &ytable);

  if (texture->getTexture() & BImage_Diagonal) dgradient();
  else if (texture->getTexture() & BImage_Elliptic) egradient();
  else if (texture->getTexture() & BImage_Horizontal) hgradient();
  else if (texture->getTexture() & BImage_Pyramid) pgradient();
  else if (texture->getTexture() & BImage_Rectangle) rgradient();
  else if (texture->getTexture() & BImage_Vertical) vgradient();
  else if (texture->getTexture() & BImage_CrossDiagonal) cdgradient();
  else if (texture->getTexture() & BImage_PipeCross) pcgradient();

  if (texture->getTexture() & BImage_Bevel1) bevel1();
  else if (texture->getTexture() & BImage_Bevel2) bevel2();

  if (inverted) invert();

  Pixmap pixmap = renderPixmap();

  return pixmap;

}


XImage *BImage::renderXImage(void) {
  XImage *image =
    XCreateImage(control->getBaseDisplay()->getXDisplay(),
                 control->getVisual(), control->getDepth(), ZPixmap, 0, 0,
                 width, height, 32, 0);

  if (! image) {
    fprintf(stderr, i18n->getMessage(ImageSet, ImageErrorCreatingXImage,
		       "BImage::renderXImage: error creating XImage\n"));
    return (XImage *) 0;
  }

  // insurance policy
  image->data = (char *) 0;

  unsigned char *d = new unsigned char[image->bytes_per_line * (height + 1)];
  register unsigned int x, y, dithx, dithy, r, g, b, o, er, eg, eb, offset;

  unsigned char *pixel_data = d, *ppixel_data = d;
  unsigned long pixel;

  o = image->bits_per_pixel + ((image->byte_order == MSBFirst) ? 1 : 0);

  if (control->doDither() && width > 1 && height > 1) {
    unsigned char dither4[4][4] = { {0, 4, 1, 5},
                                    {6, 2, 7, 3},
                                    {1, 5, 0, 4},
                                    {7, 3, 6, 2} };

#ifdef    ORDEREDPSEUDO
    unsigned char dither8[8][8] = { { 0,  32, 8,  40, 2,  34, 10, 42 },
                                    { 48, 16, 56, 24, 50, 18, 58, 26 },
                                    { 12, 44, 4,  36, 14, 46, 6,  38 },
                                    { 60, 28, 52, 20, 62, 30, 54, 22 },
                                    { 3,  35, 11, 43, 1,  33, 9,  41 },
                                    { 51, 19, 59, 27, 49, 17, 57, 25 },
                                    { 15, 47, 7,  39, 13, 45, 5,  37 },
                                    { 63, 31, 55, 23, 61, 29, 53, 21 } };
#endif // ORDEREDPSEUDO

    switch (control->getVisual()->c_class) {
    case TrueColor:
      // algorithm: ordered dithering... many many thanks to rasterman
      // (raster@rasterman.com) for telling me about this... portions of this
      // code is based off of his code in Imlib
      for (y = 0, offset = 0; y < height; y++) {
	dithy = y & 0x3;

	for (x = 0; x < width; x++, offset++) {
          dithx = x & 0x3;
          r = red[offset];
          g = green[offset];
          b = blue[offset];

          er = r & (red_bits - 1);
          eg = g & (green_bits - 1);
          eb = b & (blue_bits - 1);

          r = red_table[r];
          g = green_table[g];
          b = blue_table[b];

          if ((dither4[dithy][dithx] < er) && (r < red_table[255])) r++;
          if ((dither4[dithy][dithx] < eg) && (g < green_table[255])) g++;
          if ((dither4[dithy][dithx] < eb) && (b < blue_table[255])) b++;

	  pixel = (r << red_offset) | (g << green_offset) | (b << blue_offset);

          switch (o) {
	  case  8: //  8bpp
	    *pixel_data++ = pixel;
	    break;

          case 16: // 16bpp LSB
            *pixel_data++ = pixel;
	    *pixel_data++ = pixel >> 8;
            break;

          case 17: // 16bpp MSB
	    *pixel_data++ = pixel >> 8;
	    *pixel_data++ = pixel;
            break;

	  case 24: // 24bpp LSB
	    *pixel_data++ = pixel;
	    *pixel_data++ = pixel >> 8;
	    *pixel_data++ = pixel >> 16;
	    break;

          case 25: // 24bpp MSB
            *pixel_data++ = pixel >> 16;
            *pixel_data++ = pixel >> 8;
            *pixel_data++ = pixel;
            break;

          case 32: // 32bpp LSB
            *pixel_data++ = pixel;
            *pixel_data++ = pixel >> 8;
            *pixel_data++ = pixel >> 16;
            *pixel_data++ = pixel >> 24;
            break;

          case 33: // 32bpp MSB
            *pixel_data++ = pixel >> 24;
            *pixel_data++ = pixel >> 16;
            *pixel_data++ = pixel >> 8;
            *pixel_data++ = pixel;
            break;
          }
	}

	pixel_data = (ppixel_data += image->bytes_per_line);
      }

      break;

    case StaticColor:
    case PseudoColor: {
#ifndef   ORDEREDPSEUDO
      short *terr,
	*rerr = new short[width + 2],
	*gerr = new short[width + 2],
	*berr = new short[width + 2],
	*nrerr = new short[width + 2],
	*ngerr = new short[width + 2],
	*nberr = new short[width + 2];
      int rr, gg, bb, rer, ger, ber;
      int dd = 255 / control->getColorsPerChannel();

      for (x = 0; x < width; x++) {
	*(rerr + x) = *(red + x);
	*(gerr + x) = *(green + x);
	*(berr + x) = *(blue + x);
      }

      *(rerr + x) = *(gerr + x) = *(berr + x) = 0;
#endif // ORDEREDPSEUDO

      for (y = 0, offset = 0; y < height; y++) {
#ifdef    ORDEREDPSEUDO
        dithy = y & 7;

        for (x = 0; x < width; x++, offset++) {
          dithx = x & 7;

          r = red[offset];
          g = green[offset];
          b = blue[offset];

          er = r & (red_bits - 1);
          eg = g & (green_bits - 1);
          eb = b & (blue_bits - 1);

          r = red_table[r];
          g = green_table[g];
          b = blue_table[b];

          if ((dither8[dithy][dithx] < er) && (r < red_table[255])) r++;
          if ((dither8[dithy][dithx] < eg) && (g < green_table[255])) g++;
          if ((dither8[dithy][dithx] < eb) && (b < blue_table[255])) b++;

          pixel = (r * cpccpc) + (g * cpc) + b;
          *(pixel_data++) = colors[pixel].pixel;
        }

        pixel_data = (ppixel_data += image->bytes_per_line);
      }
#else // !ORDEREDPSEUDO
      if (y < (height - 1)) {
	int i = offset + width;
	for (x = 0; x < width; x++, i++) {
	  *(nrerr + x) = *(red + i);
	  *(ngerr + x) = *(green + i);
	  *(nberr + x) = *(blue + i);
	}

	*(nrerr + x) = *(red + (--i));
	*(ngerr + x) = *(green + i);
	*(nberr + x) = *(blue + i);
      }

      for (x = 0; x < width; x++) {
	rr = rerr[x];
	gg = gerr[x];
	bb = berr[x];

	if (rr > 255) rr = 255; else if (rr < 0) rr = 0;
	if (gg > 255) gg = 255; else if (gg < 0) gg = 0;
	if (bb > 255) bb = 255; else if (bb < 0) bb = 0;

	r = red_table[rr];
	g = green_table[gg];
	b = blue_table[bb];

	rer = rerr[x] - r*dd;
	ger = gerr[x] - g*dd;
	ber = berr[x] - b*dd;

	pixel = (r * cpccpc) + (g * cpc) + b;
	*pixel_data++ = colors[pixel].pixel;

	r = rer >> 1;
	g = ger >> 1;
	b = ber >> 1;
	rerr[x+1] += r;
	gerr[x+1] += g;
	berr[x+1] += b;
	nrerr[x] += r;
	ngerr[x] += g;
	nberr[x] += b;
      }

      offset += width;

      pixel_data = (ppixel_data += image->bytes_per_line);

      terr = rerr;
      rerr = nrerr;
      nrerr = terr;

      terr = gerr;
      gerr = ngerr;
      ngerr = terr;

      terr = berr;
      berr = nberr;
      nberr = terr;
    }

    delete [] rerr;
    delete [] gerr;
    delete [] berr;
    delete [] nrerr;
    delete [] ngerr;
    delete [] nberr;
#endif // ORDEREDPSUEDO

    break; }

    default:
      fprintf(stderr, i18n->getMessage(ImageSet, ImageUnsupVisual,
			 "BImage::renderXImage: unsupported visual\n"));
      delete [] d;
      XDestroyImage(image);
      return (XImage *) 0;
    }
  } else {
    switch (control->getVisual()->c_class) {
    case StaticColor:
    case PseudoColor:
      for (y = 0, offset = 0; y < height; y++) {
        for (x = 0; x < width; x++, offset++) {
  	  r = red_table[red[offset]];
          g = green_table[green[offset]];
	  b = blue_table[blue[offset]];

	  pixel = (r * cpccpc) + (g * cpc) + b;
	  *pixel_data++ = colors[pixel].pixel;
        }

        pixel_data = (ppixel_data += image->bytes_per_line);
      }

      break;

  case TrueColor:
    for (y = 0, offset = 0; y < height; y++) {
      for (x = 0; x < width; x++, offset++) {
	r = red_table[red[offset]];
	g = green_table[green[offset]];
	b = blue_table[blue[offset]];

	pixel = (r << red_offset) | (g << green_offset) | (b << blue_offset);

        switch (o) {
	case  8: //  8bpp
	  *pixel_data++ = pixel;
	  break;

        case 16: // 16bpp LSB
          *pixel_data++ = pixel;
          *pixel_data++ = pixel >> 8;
          break;

        case 17: // 16bpp MSB
          *pixel_data++ = pixel >> 8;
          *pixel_data++ = pixel;
          break;

        case 24: // 24bpp LSB
          *pixel_data++ = pixel;
          *pixel_data++ = pixel >> 8;
          *pixel_data++ = pixel >> 16;
          break;

        case 25: // 24bpp MSB
          *pixel_data++ = pixel >> 16;
          *pixel_data++ = pixel >> 8;
          *pixel_data++ = pixel;
          break;

        case 32: // 32bpp LSB
          *pixel_data++ = pixel;
          *pixel_data++ = pixel >> 8;
          *pixel_data++ = pixel >> 16;
          *pixel_data++ = pixel >> 24;
          break;

        case 33: // 32bpp MSB
          *pixel_data++ = pixel >> 24;
          *pixel_data++ = pixel >> 16;
          *pixel_data++ = pixel >> 8;
          *pixel_data++ = pixel;
          break;
        }
      }

      pixel_data = (ppixel_data += image->bytes_per_line);
    }

    break;

  case StaticGray:
  case GrayScale:
    for (y = 0, offset = 0; y < height; y++) {
      for (x = 0; x < width; x++, offset++) {
	r = *(red_table + *(red + offset));
	g = *(green_table + *(green + offset));
	b = *(blue_table + *(blue + offset));

	g = ((r * 30) + (g * 59) + (b * 11)) / 100;
	*pixel_data++ = colors[g].pixel;
      }

      pixel_data = (ppixel_data += image->bytes_per_line);
    }

    break;

  default:
    fprintf(stderr, i18n->getMessage(ImageSet, ImageUnsupVisual,
		       "BImage::renderXImage: unsupported visual\n"));
    delete [] d;
    XDestroyImage(image);
    return (XImage *) 0;
  }
}

  image->data = (char *) d;
  return image;
}


Pixmap BImage::renderPixmap(void) {
  Pixmap pixmap =
    XCreatePixmap(control->getBaseDisplay()->getXDisplay(),
                  control->getDrawable(), width, height, control->getDepth());

  if (pixmap == None) {
    fprintf(stderr, i18n->getMessage(ImageSet, ImageErrorCreatingPixmap,
	                     "BImage::renderPixmap: error creating pixmap\n"));
    return None;
  }

  XImage *image = renderXImage();

  if (! image) {
    XFreePixmap(control->getBaseDisplay()->getXDisplay(), pixmap);
    return None;
  } else if (! image->data) {
    XDestroyImage(image);
    XFreePixmap(control->getBaseDisplay()->getXDisplay(), pixmap);
    return None;
  }

  XPutImage(control->getBaseDisplay()->getXDisplay(), pixmap,
	    DefaultGC(control->getBaseDisplay()->getXDisplay(),
		      control->getScreenInfo()->getScreenNumber()),
            image, 0, 0, 0, 0, width, height);

  if (image->data) {
    delete [] image->data;
    image->data = NULL;
  }

  XDestroyImage(image);

  return pixmap;
}


void BImage::bevel1(void) {
  if (width > 2 && height > 2) {
    unsigned char *pr = red, *pg = green, *pb = blue;

    register unsigned char r, g, b, rr ,gg ,bb;
    register unsigned int w = width, h = height - 1, wh = w * h;

    while (--w) {
      r = *pr;
      rr = r + (r >> 1);
      if (rr < r) rr = ~0;
      g = *pg;
      gg = g + (g >> 1);
      if (gg < g) gg = ~0;
      b = *pb;
      bb = b + (b >> 1);
      if (bb < b) bb = ~0;

      *pr = rr;
      *pg = gg;
      *pb = bb;

      r = *(pr + wh);
      rr = (r >> 2) + (r >> 1);
      if (rr > r) rr = 0;
      g = *(pg + wh);
      gg = (g >> 2) + (g >> 1);
      if (gg > g) gg = 0;
      b = *(pb + wh);
      bb = (b >> 2) + (b >> 1);
      if (bb > b) bb = 0;

      *((pr++) + wh) = rr;
      *((pg++) + wh) = gg;
      *((pb++) + wh) = bb;
    }

    r = *pr;
    rr = r + (r >> 1);
    if (rr < r) rr = ~0;
    g = *pg;
    gg = g + (g >> 1);
    if (gg < g) gg = ~0;
    b = *pb;
    bb = b + (b >> 1);
    if (bb < b) bb = ~0;

    *pr = rr;
    *pg = gg;
    *pb = bb;

    r = *(pr + wh);
    rr = (r >> 2) + (r >> 1);
    if (rr > r) rr = 0;
    g = *(pg + wh);
    gg = (g >> 2) + (g >> 1);
    if (gg > g) gg = 0;
    b = *(pb + wh);
    bb = (b >> 2) + (b >> 1);
    if (bb > b) bb = 0;

    *(pr + wh) = rr;
    *(pg + wh) = gg;
    *(pb + wh) = bb;

    pr = red + width;
    pg = green + width;
    pb = blue + width;

    while (--h) {
      r = *pr;
      rr = r + (r >> 1);
      if (rr < r) rr = ~0;
      g = *pg;
      gg = g + (g >> 1);
      if (gg < g) gg = ~0;
      b = *pb;
      bb = b + (b >> 1);
      if (bb < b) bb = ~0;

      *pr = rr;
      *pg = gg;
      *pb = bb;

      pr += width - 1;
      pg += width - 1;
      pb += width - 1;

      r = *pr;
      rr = (r >> 2) + (r >> 1);
      if (rr > r) rr = 0;
      g = *pg;
      gg = (g >> 2) + (g >> 1);
      if (gg > g) gg = 0;
      b = *pb;
      bb = (b >> 2) + (b >> 1);
      if (bb > b) bb = 0;

      *(pr++) = rr;
      *(pg++) = gg;
      *(pb++) = bb;
    }

    r = *pr;
    rr = r + (r >> 1);
    if (rr < r) rr = ~0;
    g = *pg;
    gg = g + (g >> 1);
    if (gg < g) gg = ~0;
    b = *pb;
    bb = b + (b >> 1);
    if (bb < b) bb = ~0;

    *pr = rr;
    *pg = gg;
    *pb = bb;

    pr += width - 1;
    pg += width - 1;
    pb += width - 1;

    r = *pr;
    rr = (r >> 2) + (r >> 1);
    if (rr > r) rr = 0;
    g = *pg;
    gg = (g >> 2) + (g >> 1);
    if (gg > g) gg = 0;
    b = *pb;
    bb = (b >> 2) + (b >> 1);
    if (bb > b) bb = 0;

    *pr = rr;
    *pg = gg;
    *pb = bb;
  }
}


void BImage::bevel2(void) {
  if (width > 4 && height > 4) {
    unsigned char r, g, b, rr ,gg ,bb, *pr = red + width + 1,
      *pg = green + width + 1, *pb = blue + width + 1;
    unsigned int w = width - 2, h = height - 1, wh = width * (height - 3);

    while (--w) {
      r = *pr;
      rr = r + (r >> 1);
      if (rr < r) rr = ~0;
      g = *pg;
      gg = g + (g >> 1);
      if (gg < g) gg = ~0;
      b = *pb;
      bb = b + (b >> 1);
      if (bb < b) bb = ~0;

      *pr = rr;
      *pg = gg;
      *pb = bb;

      r = *(pr + wh);
      rr = (r >> 2) + (r >> 1);
      if (rr > r) rr = 0;
      g = *(pg + wh);
      gg = (g >> 2) + (g >> 1);
      if (gg > g) gg = 0;
      b = *(pb + wh);
      bb = (b >> 2) + (b >> 1);
      if (bb > b) bb = 0;

      *((pr++) + wh) = rr;
      *((pg++) + wh) = gg;
      *((pb++) + wh) = bb;
    }

    pr = red + width;
    pg = green + width;
    pb = blue + width;

    while (--h) {
      r = *pr;
      rr = r + (r >> 1);
      if (rr < r) rr = ~0;
      g = *pg;
      gg = g + (g >> 1);
      if (gg < g) gg = ~0;
      b = *pb;
      bb = b + (b >> 1);
      if (bb < b) bb = ~0;

      *(++pr) = rr;
      *(++pg) = gg;
      *(++pb) = bb;

      pr += width - 3;
      pg += width - 3;
      pb += width - 3;

      r = *pr;
      rr = (r >> 2) + (r >> 1);
      if (rr > r) rr = 0;
      g = *pg;
      gg = (g >> 2) + (g >> 1);
      if (gg > g) gg = 0;
      b = *pb;
      bb = (b >> 2) + (b >> 1);
      if (bb > b) bb = 0;

      *(pr++) = rr;
      *(pg++) = gg;
      *(pb++) = bb;

      pr++; pg++; pb++;
    }
  }
}


void BImage::invert(void) {
  register unsigned int i, j, wh = (width * height) - 1;
  unsigned char tmp;

  for (i = 0, j = wh; j > i; j--, i++) {
    tmp = *(red + j);
    *(red + j) = *(red + i);
    *(red + i) = tmp;

    tmp = *(green + j);
    *(green + j) = *(green + i);
    *(green + i) = tmp;

    tmp = *(blue + j);
    *(blue + j) = *(blue + i);
    *(blue + i) = tmp;
  }
}


void BImage::dgradient(void) {
  // diagonal gradient code was written by Mike Cole <mike@mydot.com>
  // modified for interlacing by Brad Hughes

  float drx, dgx, dbx, dry, dgy, dby, yr = 0.0, yg = 0.0, yb = 0.0,
    xr = (float) from->getRed(),
    xg = (float) from->getGreen(),
    xb = (float) from->getBlue();
  unsigned char *pr = red, *pg = green, *pb = blue;
  unsigned int w = width * 2, h = height * 2, *xt = xtable, *yt = ytable;

  register unsigned int x, y;

  dry = drx = (float) (to->getRed() - from->getRed());
  dgy = dgx = (float) (to->getGreen() - from->getGreen());
  dby = dbx = (float) (to->getBlue() - from->getBlue());

  // Create X table
  drx /= w;
  dgx /= w;
  dbx /= w;

  for (x = 0; x < width; x++) {
    *(xt++) = (unsigned char) (xr);
    *(xt++) = (unsigned char) (xg);
    *(xt++) = (unsigned char) (xb);

    xr += drx;
    xg += dgx;
    xb += dbx;
  }

  // Create Y table
  dry /= h;
  dgy /= h;
  dby /= h;

  for (y = 0; y < height; y++) {
    *(yt++) = ((unsigned char) yr);
    *(yt++) = ((unsigned char) yg);
    *(yt++) = ((unsigned char) yb);

    yr += dry;
    yg += dgy;
    yb += dby;
  }

  // Combine tables to create gradient

#ifdef    INTERLACE
  if (! interlaced) {
#endif // INTERLACE

    // normal dgradient
    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        *(pr++) = *(xt++) + *(yt);
        *(pg++) = *(xt++) + *(yt + 1);
        *(pb++) = *(xt++) + *(yt + 2);
      }
    }

#ifdef    INTERLACE
  } else {
    // faked interlacing effect
    unsigned char channel, channel2;

    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        if (y & 1) {
          channel = *(xt++) + *(yt);
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pr++) = channel2;

          channel = *(xt++) + *(yt + 1);
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pg++) = channel2;

          channel = *(xt++) + *(yt + 2);
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pb++) = channel2;
        } else {
          channel = *(xt++) + *(yt);
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pr++) = channel2;

          channel = *(xt++) + *(yt + 1);
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pg++) = channel2;

          channel = *(xt++) + *(yt + 2);
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pb++) = channel2;
        }
      }
    }
  }
#endif // INTERLACE

}


void BImage::hgradient(void) {
  float drx, dgx, dbx,
    xr = (float) from->getRed(),
    xg = (float) from->getGreen(),
    xb = (float) from->getBlue();
  unsigned char *pr = red, *pg = green, *pb = blue;

  register unsigned int x, y;

  drx = (float) (to->getRed() - from->getRed());
  dgx = (float) (to->getGreen() - from->getGreen());
  dbx = (float) (to->getBlue() - from->getBlue());

  drx /= width;
  dgx /= width;
  dbx /= width;

#ifdef    INTERLACE
  if (interlaced && height > 2) {
    // faked interlacing effect
    unsigned char channel, channel2;

    for (x = 0; x < width; x++, pr++, pg++, pb++) {
      channel = (unsigned char) xr;
      channel2 = (channel >> 1) + (channel >> 2);
      if (channel2 > channel) channel2 = 0;
      *pr = channel2;

      channel = (unsigned char) xg;
      channel2 = (channel >> 1) + (channel >> 2);
      if (channel2 > channel) channel2 = 0;
      *pg = channel2;

      channel = (unsigned char) xb;
      channel2 = (channel >> 1) + (channel >> 2);
      if (channel2 > channel) channel2 = 0;
      *pb = channel2;


      channel = (unsigned char) xr;
      channel2 = channel + (channel >> 3);
      if (channel2 < channel) channel2 = ~0;
      *(pr + width) = channel2;

      channel = (unsigned char) xg;
      channel2 = channel + (channel >> 3);
      if (channel2 < channel) channel2 = ~0;
      *(pg + width) = channel2;

      channel = (unsigned char) xb;
      channel2 = channel + (channel >> 3);
      if (channel2 < channel) channel2 = ~0;
      *(pb + width) = channel2;

      xr += drx;
      xg += dgx;
      xb += dbx;
    }

    pr += width;
    pg += width;
    pb += width;

    int offset;

    for (y = 2; y < height; y++, pr += width, pg += width, pb += width) {
      if (y & 1) offset = width; else offset = 0;

      memcpy(pr, (red + offset), width);
      memcpy(pg, (green + offset), width);
      memcpy(pb, (blue + offset), width);
    }
  } else {
#endif // INTERLACE

    // normal hgradient
    for (x = 0; x < width; x++) {
      *(pr++) = (unsigned char) (xr);
      *(pg++) = (unsigned char) (xg);
      *(pb++) = (unsigned char) (xb);

      xr += drx;
      xg += dgx;
      xb += dbx;
    }

    for (y = 1; y < height; y++, pr += width, pg += width, pb += width) {
      memcpy(pr, red, width);
      memcpy(pg, green, width);
      memcpy(pb, blue, width);
    }

#ifdef    INTERLACE
  }
#endif // INTERLACE

}


void BImage::vgradient(void) {
  float dry, dgy, dby,
    yr = (float) from->getRed(),
    yg = (float) from->getGreen(),
    yb = (float) from->getBlue();
  unsigned char *pr = red, *pg = green, *pb = blue;

  register unsigned int y;

  dry = (float) (to->getRed() - from->getRed());
  dgy = (float) (to->getGreen() - from->getGreen());
  dby = (float) (to->getBlue() - from->getBlue());

  dry /= height;
  dgy /= height;
  dby /= height;

#ifdef    INTERLACE
  if (interlaced) {
    // faked interlacing effect
    unsigned char channel, channel2;

    for (y = 0; y < height; y++, pr += width, pg += width, pb += width) {
      if (y & 1) {
        channel = (unsigned char) yr;
        channel2 = (channel >> 1) + (channel >> 2);
        if (channel2 > channel) channel2 = 0;
        memset(pr, channel2, width);

        channel = (unsigned char) yg;
        channel2 = (channel >> 1) + (channel >> 2);
        if (channel2 > channel) channel2 = 0;
        memset(pg, channel2, width);

        channel = (unsigned char) yb;
        channel2 = (channel >> 1) + (channel >> 2);
        if (channel2 > channel) channel2 = 0;
        memset(pb, channel2, width);
      } else {
        channel = (unsigned char) yr;
        channel2 = channel + (channel >> 3);
        if (channel2 < channel) channel2 = ~0;
        memset(pr, channel2, width);

        channel = (unsigned char) yg;
        channel2 = channel + (channel >> 3);
        if (channel2 < channel) channel2 = ~0;
        memset(pg, channel2, width);

        channel = (unsigned char) yb;
        channel2 = channel + (channel >> 3);
        if (channel2 < channel) channel2 = ~0;
        memset(pb, channel2, width);
      }

      yr += dry;
      yg += dgy;
      yb += dby;
    }
  } else {
#endif // INTERLACE

    // normal vgradient
    for (y = 0; y < height; y++, pr += width, pg += width, pb += width) {
      memset(pr, (unsigned char) yr, width);
      memset(pg, (unsigned char) yg, width);
      memset(pb, (unsigned char) yb, width);

      yr += dry;
      yg += dgy;
      yb += dby;
    }

#ifdef    INTERLACE
  }
#endif // INTERLACE

}


void BImage::pgradient(void) {
  // pyramid gradient -  based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Openbox by Brad Hughes

  float yr, yg, yb, drx, dgx, dbx, dry, dgy, dby,
    xr, xg, xb;
  int rsign, gsign, bsign;
  unsigned char *pr = red, *pg = green, *pb = blue;
  unsigned int tr = to->getRed(), tg = to->getGreen(), tb = to->getBlue(),
    *xt = xtable, *yt = ytable;

  register unsigned int x, y;

  dry = drx = (float) (to->getRed() - from->getRed());
  dgy = dgx = (float) (to->getGreen() - from->getGreen());
  dby = dbx = (float) (to->getBlue() - from->getBlue());

  rsign = (drx < 0) ? -1 : 1;
  gsign = (dgx < 0) ? -1 : 1;
  bsign = (dbx < 0) ? -1 : 1;

  xr = yr = (drx / 2);
  xg = yg = (dgx / 2);
  xb = yb = (dbx / 2);

  // Create X table
  drx /= width;
  dgx /= width;
  dbx /= width;

  for (x = 0; x < width; x++) {
    *(xt++) = (unsigned char) ((xr < 0) ? -xr : xr);
    *(xt++) = (unsigned char) ((xg < 0) ? -xg : xg);
    *(xt++) = (unsigned char) ((xb < 0) ? -xb : xb);

    xr -= drx;
    xg -= dgx;
    xb -= dbx;
  }

  // Create Y table
  dry /= height;
  dgy /= height;
  dby /= height;

  for (y = 0; y < height; y++) {
    *(yt++) = ((unsigned char) ((yr < 0) ? -yr : yr));
    *(yt++) = ((unsigned char) ((yg < 0) ? -yg : yg));
    *(yt++) = ((unsigned char) ((yb < 0) ? -yb : yb));

    yr -= dry;
    yg -= dgy;
    yb -= dby;
  }

  // Combine tables to create gradient

#ifdef    INTERLACE
  if (! interlaced) {
#endif // INTERLACE

    // normal pgradient
    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        *(pr++) = (unsigned char) (tr - (rsign * (*(xt++) + *(yt))));
        *(pg++) = (unsigned char) (tg - (gsign * (*(xt++) + *(yt + 1))));
        *(pb++) = (unsigned char) (tb - (bsign * (*(xt++) + *(yt + 2))));
      }
    }

#ifdef    INTERLACE
  } else {
    // faked interlacing effect
    unsigned char channel, channel2;

    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        if (y & 1) {
          channel = (unsigned char) (tr - (rsign * (*(xt++) + *(yt))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pr++) = channel2;

          channel = (unsigned char) (tg - (gsign * (*(xt++) + *(yt + 1))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pg++) = channel2;

          channel = (unsigned char) (tb - (bsign * (*(xt++) + *(yt + 2))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pb++) = channel2;
        } else {
          channel = (unsigned char) (tr - (rsign * (*(xt++) + *(yt))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pr++) = channel2;

          channel = (unsigned char) (tg - (gsign * (*(xt++) + *(yt + 1))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pg++) = channel2;

          channel = (unsigned char) (tb - (bsign * (*(xt++) + *(yt + 2))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pb++) = channel2;
        }
      }
    }
  }
#endif // INTERLACE
}


void BImage::rgradient(void) {
  // rectangle gradient -  based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Openbox by Brad Hughes

  float drx, dgx, dbx, dry, dgy, dby, xr, xg, xb, yr, yg, yb;
  int rsign, gsign, bsign;
  unsigned char *pr = red, *pg = green, *pb = blue;
  unsigned int tr = to->getRed(), tg = to->getGreen(), tb = to->getBlue(),
    *xt = xtable, *yt = ytable;

  register unsigned int x, y;

  dry = drx = (float) (to->getRed() - from->getRed());
  dgy = dgx = (float) (to->getGreen() - from->getGreen());
  dby = dbx = (float) (to->getBlue() - from->getBlue());

  rsign = (drx < 0) ? -2 : 2;
  gsign = (dgx < 0) ? -2 : 2;
  bsign = (dbx < 0) ? -2 : 2;

  xr = yr = (drx / 2);
  xg = yg = (dgx / 2);
  xb = yb = (dbx / 2);

  // Create X table
  drx /= width;
  dgx /= width;
  dbx /= width;

  for (x = 0; x < width; x++) {
    *(xt++) = (unsigned char) ((xr < 0) ? -xr : xr);
    *(xt++) = (unsigned char) ((xg < 0) ? -xg : xg);
    *(xt++) = (unsigned char) ((xb < 0) ? -xb : xb);

    xr -= drx;
    xg -= dgx;
    xb -= dbx;
  }

  // Create Y table
  dry /= height;
  dgy /= height;
  dby /= height;

  for (y = 0; y < height; y++) {
    *(yt++) = ((unsigned char) ((yr < 0) ? -yr : yr));
    *(yt++) = ((unsigned char) ((yg < 0) ? -yg : yg));
    *(yt++) = ((unsigned char) ((yb < 0) ? -yb : yb));

    yr -= dry;
    yg -= dgy;
    yb -= dby;
  }

  // Combine tables to create gradient

#ifdef    INTERLACE
  if (! interlaced) {
#endif // INTERLACE

    // normal rgradient
    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        *(pr++) = (unsigned char) (tr - (rsign * max(*(xt++), *(yt))));
        *(pg++) = (unsigned char) (tg - (gsign * max(*(xt++), *(yt + 1))));
        *(pb++) = (unsigned char) (tb - (bsign * max(*(xt++), *(yt + 2))));
      }
    }

#ifdef    INTERLACE
  } else {
    // faked interlacing effect
    unsigned char channel, channel2;

    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        if (y & 1) {
          channel = (unsigned char) (tr - (rsign * max(*(xt++), *(yt))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pr++) = channel2;

          channel = (unsigned char) (tg - (gsign * max(*(xt++), *(yt + 1))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pg++) = channel2;

          channel = (unsigned char) (tb - (bsign * max(*(xt++), *(yt + 2))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pb++) = channel2;
        } else {
          channel = (unsigned char) (tr - (rsign * max(*(xt++), *(yt))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pr++) = channel2;

          channel = (unsigned char) (tg - (gsign * max(*(xt++), *(yt + 1))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pg++) = channel2;

          channel = (unsigned char) (tb - (bsign * max(*(xt++), *(yt + 2))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pb++) = channel2;
        }
      }
    }
  }
#endif // INTERLACE
}


void BImage::egradient(void) {
  // elliptic gradient -  based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Openbox by Brad Hughes

  float drx, dgx, dbx, dry, dgy, dby, yr, yg, yb, xr, xg, xb;
  int rsign, gsign, bsign;
  unsigned char *pr = red, *pg = green, *pb = blue;
  unsigned int *xt = xtable, *yt = ytable,
    tr = (unsigned long) to->getRed(),
    tg = (unsigned long) to->getGreen(),
    tb = (unsigned long) to->getBlue();

  register unsigned int x, y;

  dry = drx = (float) (to->getRed() - from->getRed());
  dgy = dgx = (float) (to->getGreen() - from->getGreen());
  dby = dbx = (float) (to->getBlue() - from->getBlue());

  rsign = (drx < 0) ? -1 : 1;
  gsign = (dgx < 0) ? -1 : 1;
  bsign = (dbx < 0) ? -1 : 1;

  xr = yr = (drx / 2);
  xg = yg = (dgx / 2);
  xb = yb = (dbx / 2);

  // Create X table
  drx /= width;
  dgx /= width;
  dbx /= width;

  for (x = 0; x < width; x++) {
    *(xt++) = (unsigned long) (xr * xr);
    *(xt++) = (unsigned long) (xg * xg);
    *(xt++) = (unsigned long) (xb * xb);

    xr -= drx;
    xg -= dgx;
    xb -= dbx;
  }

  // Create Y table
  dry /= height;
  dgy /= height;
  dby /= height;

  for (y = 0; y < height; y++) {
    *(yt++) = (unsigned long) (yr * yr);
    *(yt++) = (unsigned long) (yg * yg);
    *(yt++) = (unsigned long) (yb * yb);

    yr -= dry;
    yg -= dgy;
    yb -= dby;
  }

  // Combine tables to create gradient

#ifdef    INTERLACE
  if (! interlaced) {
#endif // INTERLACE

    // normal egradient
    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        *(pr++) = (unsigned char)
          (tr - (rsign * control->getSqrt(*(xt++) + *(yt))));
        *(pg++) = (unsigned char)
          (tg - (gsign * control->getSqrt(*(xt++) + *(yt + 1))));
        *(pb++) = (unsigned char)
          (tb - (bsign * control->getSqrt(*(xt++) + *(yt + 2))));
      }
    }

#ifdef    INTERLACE
  } else {
    // faked interlacing effect
    unsigned char channel, channel2;

    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        if (y & 1) {
          channel = (unsigned char)
            (tr - (rsign * control->getSqrt(*(xt++) + *(yt))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pr++) = channel2;

          channel = (unsigned char)
            (tg - (gsign * control->getSqrt(*(xt++) + *(yt + 1))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pg++) = channel2;

          channel = (unsigned char)
            (tb - (bsign * control->getSqrt(*(xt++) + *(yt + 2))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pb++) = channel2;
        } else {
          channel = (unsigned char)
            (tr - (rsign * control->getSqrt(*(xt++) + *(yt))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pr++) = channel2;

          channel = (unsigned char)
          (tg - (gsign * control->getSqrt(*(xt++) + *(yt + 1))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pg++) = channel2;

          channel = (unsigned char)
            (tb - (bsign * control->getSqrt(*(xt++) + *(yt + 2))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pb++) = channel2;
        }
      }
    }
  }
#endif // INTERLACE
}


void BImage::pcgradient(void) {
  // pipe cross gradient -  based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Openbox by Brad Hughes

  float drx, dgx, dbx, dry, dgy, dby, xr, xg, xb, yr, yg, yb;
  int rsign, gsign, bsign;
  unsigned char *pr = red, *pg = green, *pb = blue;
  unsigned int *xt = xtable, *yt = ytable,
    tr = to->getRed(),
    tg = to->getGreen(),
    tb = to->getBlue();

  register unsigned int x, y;

  dry = drx = (float) (to->getRed() - from->getRed());
  dgy = dgx = (float) (to->getGreen() - from->getGreen());
  dby = dbx = (float) (to->getBlue() - from->getBlue());

  rsign = (drx < 0) ? -2 : 2;
  gsign = (dgx < 0) ? -2 : 2;
  bsign = (dbx < 0) ? -2 : 2;

  xr = yr = (drx / 2);
  xg = yg = (dgx / 2);
  xb = yb = (dbx / 2);

  // Create X table
  drx /= width;
  dgx /= width;
  dbx /= width;

  for (x = 0; x < width; x++) {
    *(xt++) = (unsigned char) ((xr < 0) ? -xr : xr);
    *(xt++) = (unsigned char) ((xg < 0) ? -xg : xg);
    *(xt++) = (unsigned char) ((xb < 0) ? -xb : xb);

    xr -= drx;
    xg -= dgx;
    xb -= dbx;
  }

  // Create Y table
  dry /= height;
  dgy /= height;
  dby /= height;

  for (y = 0; y < height; y++) {
    *(yt++) = ((unsigned char) ((yr < 0) ? -yr : yr));
    *(yt++) = ((unsigned char) ((yg < 0) ? -yg : yg));
    *(yt++) = ((unsigned char) ((yb < 0) ? -yb : yb));

    yr -= dry;
    yg -= dgy;
    yb -= dby;
  }

  // Combine tables to create gradient

#ifdef    INTERLACE
  if (! interlaced) {
#endif // INTERLACE

    // normal pcgradient
    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        *(pr++) = (unsigned char) (tr - (rsign * min(*(xt++), *(yt))));
        *(pg++) = (unsigned char) (tg - (gsign * min(*(xt++), *(yt + 1))));
        *(pb++) = (unsigned char) (tb - (bsign * min(*(xt++), *(yt + 2))));
      }
    }

#ifdef    INTERLACE
  } else {
    // faked interlacing effect
    unsigned char channel, channel2;

    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        if (y & 1) {
          channel = (unsigned char) (tr - (rsign * min(*(xt++), *(yt))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pr++) = channel2;

          channel = (unsigned char) (tg - (bsign * min(*(xt++), *(yt + 1))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pg++) = channel2;

          channel = (unsigned char) (tb - (gsign * min(*(xt++), *(yt + 2))));
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pb++) = channel2;
        } else {
          channel = (unsigned char) (tr - (rsign * min(*(xt++), *(yt))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pr++) = channel2;

          channel = (unsigned char) (tg - (gsign * min(*(xt++), *(yt + 1))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pg++) = channel2;

          channel = (unsigned char) (tb - (bsign * min(*(xt++), *(yt + 2))));
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pb++) = channel2;
        }
      }
    }
  }
#endif // INTERLACE
}


void BImage::cdgradient(void) {
  // cross diagonal gradient -  based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Openbox by Brad Hughes

  float drx, dgx, dbx, dry, dgy, dby, yr = 0.0, yg = 0.0, yb = 0.0,
    xr = (float) from->getRed(),
    xg = (float) from->getGreen(),
    xb = (float) from->getBlue();
  unsigned char *pr = red, *pg = green, *pb = blue;
  unsigned int w = width * 2, h = height * 2, *xt, *yt;

  register unsigned int x, y;

  dry = drx = (float) (to->getRed() - from->getRed());
  dgy = dgx = (float) (to->getGreen() - from->getGreen());
  dby = dbx = (float) (to->getBlue() - from->getBlue());

  // Create X table
  drx /= w;
  dgx /= w;
  dbx /= w;

  for (xt = (xtable + (width * 3) - 1), x = 0; x < width; x++) {
    *(xt--) = (unsigned char) xb;
    *(xt--) = (unsigned char) xg;
    *(xt--) = (unsigned char) xr;

    xr += drx;
    xg += dgx;
    xb += dbx;
  }

  // Create Y table
  dry /= h;
  dgy /= h;
  dby /= h;

  for (yt = ytable, y = 0; y < height; y++) {
    *(yt++) = (unsigned char) yr;
    *(yt++) = (unsigned char) yg;
    *(yt++) = (unsigned char) yb;

    yr += dry;
    yg += dgy;
    yb += dby;
  }

  // Combine tables to create gradient

#ifdef    INTERLACE
  if (! interlaced) {
#endif // INTERLACE

    // normal cdgradient
    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        *(pr++) = *(xt++) + *(yt);
        *(pg++) = *(xt++) + *(yt + 1);
        *(pb++) = *(xt++) + *(yt + 2);
      }
    }

#ifdef    INTERLACE
  } else {
    // faked interlacing effect
    unsigned char channel, channel2;

    for (yt = ytable, y = 0; y < height; y++, yt += 3) {
      for (xt = xtable, x = 0; x < width; x++) {
        if (y & 1) {
          channel = *(xt++) + *(yt);
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pr++) = channel2;

          channel = *(xt++) + *(yt + 1);
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pg++) = channel2;

          channel = *(xt++) + *(yt + 2);
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          *(pb++) = channel2;
        } else {
          channel = *(xt++) + *(yt);
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pr++) = channel2;

          channel = *(xt++) + *(yt + 1);
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pg++) = channel2;

          channel = *(xt++) + *(yt + 2);
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          *(pb++) = channel2;
        }
      }
    }
  }
#endif // INTERLACE
}


BImageControl::BImageControl(BaseDisplay *dpy, ScreenInfo *scrn, Bool _dither,
                             int _cpc, unsigned long cache_timeout,
                             unsigned long cmax)
{
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
  } else
    timer = (BTimer *) 0;
#endif // TIMEDCACHE

  colors = (XColor *) 0;
  ncolors = 0;

  grad_xbuffer = grad_ybuffer = (unsigned int *) 0;
  grad_buffer_width = grad_buffer_height = 0;

  sqrt_table = (unsigned long *) 0;

  screen_depth = screeninfo->getDepth();
  window = screeninfo->getRootWindow();
  screen_number = screeninfo->getScreenNumber();

  int count;
  XPixmapFormatValues *pmv = XListPixmapFormats(basedisplay->getXDisplay(),
                                                &count);
  colormap = screeninfo->getColormap();

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
  case TrueColor:
    {
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
    }

    break;

  case PseudoColor:
  case StaticColor:
    {
      ncolors = colors_per_channel * colors_per_channel * colors_per_channel;

      if (ncolors > (1 << screen_depth)) {
	colors_per_channel = (1 << screen_depth) / 3;
	ncolors = colors_per_channel * colors_per_channel * colors_per_channel;
      }

      if (colors_per_channel < 2 || ncolors > (1 << screen_depth)) {
	fprintf(stderr, i18n->getMessage(ImageSet, ImageInvalidColormapSize,
                      "BImageControl::BImageControl: invalid colormap size %d "
		           "(%d/%d/%d) - reducing"),
                ncolors, colors_per_channel, colors_per_channel,
                colors_per_channel);

        colors_per_channel = (1 << screen_depth) / 3;
      }

      colors = new XColor[ncolors];
      if (! colors) {
	fprintf(stderr, i18n->getMessage(ImageSet,
	                                 ImageErrorAllocatingColormap,
	        	   "BImageControl::BImageControl: error allocating "
		           "colormap\n"));
	exit(1);
      }

      int i = 0, ii, p, r, g, b,

#ifdef    ORDEREDPSEUDO
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

      basedisplay->grab();

      for (i = 0; i < ncolors; i++)
	if (! XAllocColor(basedisplay->getXDisplay(), colormap, &colors[i])) {
	  fprintf(stderr, i18n->getMessage(ImageSet, ImageColorAllocFail,
		                   "couldn't alloc color %i %i %i\n"),
		  colors[i].red, colors[i].green, colors[i].blue);
	  colors[i].flags = 0;
	} else
	  colors[i].flags = DoRed|DoGreen|DoBlue;

      basedisplay->ungrab();

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
  case StaticGray:
    {

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
	fprintf(stderr,	i18n->getMessage(ImageSet, ImageInvalidColormapSize,
                      "BImageControl::BImageControl: invalid colormap size %d "
		           "(%d/%d/%d) - reducing"),
		ncolors, colors_per_channel, colors_per_channel,
		colors_per_channel);

	colors_per_channel = (1 << screen_depth) / 3;
      }

      colors = new XColor[ncolors];
      if (! colors) {
	fprintf(stderr, i18n->getMessage(ImageSet,
	                                 ImageErrorAllocatingColormap,
			   "BImageControl::BImageControl: error allocating "
       	                   "colormap\n"));
	exit(1);
      }

      int i = 0, ii, p, bits = 255 / (colors_per_channel - 1);
      red_bits = green_bits = blue_bits = bits;

      for (i = 0; i < 256; i++)
	red_color_table[i] = green_color_table[i] = blue_color_table[i] =
	  i / bits;

      basedisplay->grab();
      for (i = 0; i < ncolors; i++) {
	colors[i].red = (i * 0xffff) / (colors_per_channel - 1);
	colors[i].green = (i * 0xffff) / (colors_per_channel - 1);
	colors[i].blue = (i * 0xffff) / (colors_per_channel - 1);;
	colors[i].flags = DoRed|DoGreen|DoBlue;

	if (! XAllocColor(basedisplay->getXDisplay(), colormap,
			  &colors[i])) {
	  fprintf(stderr, i18n->getMessage(ImageSet, ImageColorAllocFail,
		             "couldn't alloc color %i %i %i\n"),
		  colors[i].red, colors[i].green, colors[i].blue);
	  colors[i].flags = 0;
	} else
	  colors[i].flags = DoRed|DoGreen|DoBlue;
      }

      basedisplay->ungrab();

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
    fprintf(stderr, i18n->getMessage(ImageSet, ImageUnsupVisual,
               "BImageControl::BImageControl: unsupported visual %d\n"),
	    getVisual()->c_class);
    exit(1);
  }

  cache = new LinkedList<Cache>;
}


BImageControl::~BImageControl(void) {
  if (sqrt_table) {
    delete [] sqrt_table;
  }

  if (grad_xbuffer) {
    delete [] grad_xbuffer;
  }

  if (grad_ybuffer) {
    delete [] grad_ybuffer;
  }

  if (colors) {
    unsigned long *pixels = new unsigned long [ncolors];

    int i;
    for (i = 0; i < ncolors; i++)
      *(pixels + i) = (*(colors + i)).pixel;

    XFreeColors(basedisplay->getXDisplay(), colormap, pixels, ncolors, 0);

    delete [] colors;
  }

  if (cache->count()) {
    int i, n = cache->count();
    fprintf(stderr, i18n->getMessage(ImageSet, ImagePixmapRelease,
		       "BImageContol::~BImageControl: pixmap cache - "
	               "releasing %d pixmaps\n"), n);

    for (i = 0; i < n; i++) {
      Cache *tmp = cache->first();
      XFreePixmap(basedisplay->getXDisplay(), tmp->pixmap);
      cache->remove(tmp);
      delete tmp;
    }

#ifdef    TIMEDCACHE
    if (timer) {
      timer->stop();
      delete timer;
    }
#endif // TIMEDCACHE
  }

  delete cache;
}


Pixmap BImageControl::searchCache(unsigned int width, unsigned int height,
		  unsigned long texture,
		  BColor *c1, BColor *c2) {
  if (cache->count()) {
    LinkedListIterator<Cache> it(cache);

    for (Cache *tmp = it.current(); tmp; it++, tmp = it.current()) {
      if ((tmp->width == width) && (tmp->height == height) &&
          (tmp->texture == texture) && (tmp->pixel1 == c1->getPixel()))
          if (texture & BImage_Gradient) {
            if (tmp->pixel2 == c2->getPixel()) {
              tmp->count++;
              return tmp->pixmap;
            }
          } else {
            tmp->count++;
            return tmp->pixmap;
          }
        }
  }

  return None;
}


Pixmap BImageControl::renderImage(unsigned int width, unsigned int height,
      BTexture *texture) {
  if (texture->getTexture() & BImage_ParentRelative) return ParentRelative;

  Pixmap pixmap = searchCache(width, height, texture->getTexture(),
			      texture->getColor(), texture->getColorTo());
  if (pixmap) return pixmap;

  BImage image(this, width, height);
  pixmap = image.render(texture);

  if (pixmap) {
    Cache *tmp = new Cache;

    tmp->pixmap = pixmap;
    tmp->width = width;
    tmp->height = height;
    tmp->count = 1;
    tmp->texture = texture->getTexture();
    tmp->pixel1 = texture->getColor()->getPixel();

    if (texture->getTexture() & BImage_Gradient)
      tmp->pixel2 = texture->getColorTo()->getPixel();
    else
      tmp->pixel2 = 0l;

    cache->insert(tmp);

    if ((unsigned) cache->count() > cache_max) {
#ifdef    DEBUG
      fprintf(stderr, i18n->getMessage(ImageSet, ImagePixmapCacheLarge,
                         "BImageControl::renderImage: cache is large, "
                         "forcing cleanout\n"));
#endif // DEBUG

      timeout();
    }

    return pixmap;
  }

  return None;
}


void BImageControl::removeImage(Pixmap pixmap) {
  if (pixmap) {
    LinkedListIterator<Cache> it(cache);
    for (Cache *tmp = it.current(); tmp; it++, tmp = it.current()) {
      if (tmp->pixmap == pixmap) {
        if (tmp->count) {
	  tmp->count--;

#ifdef    TIMEDCACHE
	   if (! timer) timeout();
#else // !TIMEDCACHE
	   if (! tmp->count) timeout();
#endif // TIMEDCACHE
        }

	return;
      }
    }
  }
}


unsigned long BImageControl::getColor(const char *colorname,
				      unsigned char *r, unsigned char *g,
				      unsigned char *b)
{
  XColor color;
  color.pixel = 0;

  if (! XParseColor(basedisplay->getXDisplay(), colormap, colorname, &color))
    fprintf(stderr, "BImageControl::getColor: color parse error: \"%s\"\n",
	    colorname);
  else if (! XAllocColor(basedisplay->getXDisplay(), colormap, &color))
    fprintf(stderr, "BImageControl::getColor: color alloc error: \"%s\"\n",
	    colorname);

  if (color.red == 65535) *r = 0xff;
  else *r = (unsigned char) (color.red / 0xff);
  if (color.green == 65535) *g = 0xff;
  else *g = (unsigned char) (color.green / 0xff);
  if (color.blue == 65535) *b = 0xff;
  else *b = (unsigned char) (color.blue / 0xff);

  return color.pixel;
}


unsigned long BImageControl::getColor(const char *colorname) {
  XColor color;
  color.pixel = 0;

  if (! XParseColor(basedisplay->getXDisplay(), colormap, colorname, &color))
    fprintf(stderr, "BImageControl::getColor: color parse error: \"%s\"\n",
	    colorname);
  else if (! XAllocColor(basedisplay->getXDisplay(), colormap, &color))
    fprintf(stderr, "BImageControl::getColor: color alloc error: \"%s\"\n",
	    colorname);

  return color.pixel;
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
  basedisplay->grab();

  Bool install = True;
  int i = 0, ncmap = 0;
  Colormap *cmaps =
    XListInstalledColormaps(basedisplay->getXDisplay(), window, &ncmap);

  if (cmaps) {
    for (i = 0; i < ncmap; i++)
      if (*(cmaps + i) == colormap)
	install = False;

    if (install)
      XInstallColormap(basedisplay->getXDisplay(), colormap);

    XFree(cmaps);
  }

  basedisplay->ungrab();
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
    int i = 0;

    for (; i < (256 * 256 * 2); i++)
      *(sqrt_table + i) = bsqrt(i);
  }

  return (*(sqrt_table + x));
}


void BImageControl::parseTexture(BTexture *texture, const char *t) {
  if ((! texture) || (! t)) return;

  int t_len = strlen(t) + 1, i;
  char *ts = new char[t_len];
  if (! ts) return;

  // convert to lower case
  for (i = 0; i < t_len; i++)
    *(ts + i) = tolower(*(t + i));

  if (strstr(ts, "parentrelative")) {
    texture->setTexture(BImage_ParentRelative);
  } else {
    texture->setTexture(0);

    if (strstr(ts, "solid"))
      texture->addTexture(BImage_Solid);
    else if (strstr(ts, "gradient")) {
      texture->addTexture(BImage_Gradient);
      if (strstr(ts, "crossdiagonal"))
	texture->addTexture(BImage_CrossDiagonal);
      else if (strstr(ts, "rectangle"))
	texture->addTexture(BImage_Rectangle);
      else if (strstr(ts, "pyramid"))
	texture->addTexture(BImage_Pyramid);
      else if (strstr(ts, "pipecross"))
	texture->addTexture(BImage_PipeCross);
      else if (strstr(ts, "elliptic"))
	texture->addTexture(BImage_Elliptic);
      else if (strstr(ts, "diagonal"))
	texture->addTexture(BImage_Diagonal);
      else if (strstr(ts, "horizontal"))
	texture->addTexture(BImage_Horizontal);
      else if (strstr(ts, "vertical"))
	texture->addTexture(BImage_Vertical);
      else
	texture->addTexture(BImage_Diagonal);
    } else
      texture->addTexture(BImage_Solid);

    if (strstr(ts, "raised"))
      texture->addTexture(BImage_Raised);
    else if (strstr(ts, "sunken"))
      texture->addTexture(BImage_Sunken);
    else if (strstr(ts, "flat"))
      texture->addTexture(BImage_Flat);
    else
      texture->addTexture(BImage_Raised);

    if (! (texture->getTexture() & BImage_Flat))
      if (strstr(ts, "bevel2"))
	texture->addTexture(BImage_Bevel2);
      else
	texture->addTexture(BImage_Bevel1);

#ifdef    INTERLACE
    if (strstr(ts, "interlaced"))
      texture->addTexture(BImage_Interlaced);
#endif // INTERLACE
  }

  delete [] ts;
}


void BImageControl::parseColor(BColor *color, const char *c) {
  if (! color) return;

  if (color->isAllocated()) {
    unsigned long pixel = color->getPixel();

    XFreeColors(basedisplay->getXDisplay(), colormap, &pixel, 1, 0);

    color->setPixel(0l);
    color->setRGB(0, 0, 0);
    color->setAllocated(False);
  }

  if (c) {
    unsigned char r, g, b;

    color->setPixel(getColor(c, &r, &g, &b));
    color->setRGB(r, g, b);
    color->setAllocated(True);
  }
}


void BImageControl::timeout(void) {
  LinkedListIterator<Cache> it(cache);
  for (Cache *tmp = it.current(); tmp; it++, tmp = it.current()) {
    if (tmp->count <= 0) {
      XFreePixmap(basedisplay->getXDisplay(), tmp->pixmap);
      cache->remove(tmp);
      delete tmp;
    }
  }
}
