// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __rendercolor_hh
#define __rendercolor_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <map>

namespace otk {

struct RGB {
  int r;
  int g;
  int b;
  RGB(int red, int green, int blue) : r(red), g(green), b(blue) {}
  // color is in ARGB format
  RGB(unsigned long color)
    : r((color >> 16) & 0xff),
      g((color >> 8) & 0xff),
      b((color) & 0xff) {}
};

class RenderColor {
private:
  struct CacheItem {
    GC gc;
    unsigned long pixel;
    int count;
    CacheItem(GC g, unsigned long p) : gc(g), pixel(p), count(0) {}
  };
  static std::map<unsigned long, CacheItem*> *_cache;

  int _screen;
  unsigned char _red;
  unsigned char _green;
  unsigned char _blue;

  mutable unsigned long _pixel;
  mutable GC _gc;

  mutable bool _allocated;

  void create() const;
  
public:
  static void initialize();
  static void destroy();
  
  RenderColor(int screen, unsigned char red,
	      unsigned char green, unsigned char blue);
  RenderColor(int screen, RGB rgb);
  virtual ~RenderColor();

  inline int screen() const { return _screen; }
  inline unsigned char red() const { return _red; }
  inline unsigned char green() const { return _green; }
  inline unsigned char blue() const { return _blue; }
  unsigned long pixel() const;
  GC gc() const;
};

}

#endif // __rendercolor_hh
