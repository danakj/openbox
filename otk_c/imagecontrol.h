// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __imagecontrol_h
#define   __imagecontrol_h

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Python.h>

struct OtkScreenInfo;
struct OtkTimer;

extern PyTypeObject OtkImageControl_Type;

typedef struct OtkImageControl {
  struct OtkScreenInfo *screen;

  // for the pixmap cache
  struct OtkTimer *timer;
  unsigned long cache_max;

  Bool dither;

  int cpc; // colors-per-channel: must be a value between [2,6]
  int bpp; // bits-per-pixel

  unsigned int *grad_xbuffer;
  unsigned int *grad_ybuffer;
  unsigned int grad_buffer_width;
  unsigned int grad_buffer_height;

  unsigned long *sqrt_table;

  // These values are all determined based on a visual
  
  int red_bits;    // the number of bits (1-255) that each shade of color
  int green_bits;  // spans across. best case is 1, which gives 255 shades.
  int blue_bits;
  unsigned char red_color_table[256];
  unsigned char green_color_table[256];
  unsigned char blue_color_table[256];

  // These are only used for TrueColor visuals
  int red_offset;  // the offset of each color in a color mask
  int green_offset;
  int blue_offset;

  // These are only used for !TrueColor visuals
  XColor *colors;
  int ncolors;

} OtkImageControl;

PyObject *OtkImageControl_New(int screen);


/*
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
*/

#endif // __imagecontrol_h
