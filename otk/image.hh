// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __Image_hh
#define   __Image_hh

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include <list>

#include "color.hh"
#include "screeninfo.hh"
#include "timer.hh"

namespace otk {

class BImageControl;
class BTexture;
class ScreenInfo;

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
  Pixmap render_solid(const BTexture &texture);
  Pixmap render_gradient(const BTexture &texture);

  XImage *renderXImage(void);

  void invert(void);
  void bevel1(void);
  void bevel2(void);
  void border(const BTexture &texture);
  void dgradient(void);
  void egradient(void);
  void hgradient(void);
  void pgradient(void);
  void rgradient(void);
  void vgradient(void);
  void cdgradient(void);
  void pcgradient(void);


public:
  BImage(BImageControl *c, int w, int h);
  ~BImage(void);

  Pixmap render(const BTexture &texture);
};


class BImageControl {
public:
  struct CachedImage {
    Pixmap pixmap;

    unsigned int count, width, height;
    unsigned long pixel1, pixel2, texture;
  };

  BImageControl(OBTimerQueueManager *timermanager,
                const ScreenInfo *scrn,
                bool _dither= False, int _cpc = 4,
                unsigned long cache_timeout = 300000l,
                unsigned long cmax = 200l);
  virtual ~BImageControl(void);

  inline bool doDither(void) { return dither; }

  inline const ScreenInfo* getScreenInfo() const { return screeninfo; }

  inline Window getDrawable(void) const { return window; }

  inline Visual *getVisual(void) { return screeninfo->visual(); }

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

  static void timeout(BImageControl *t);

private:
  bool dither;
  const ScreenInfo *screeninfo;
  OBTimer *timer;

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

}

#endif // __Image_hh

