// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __rendercolor_hh
#define __rendercolor_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <map>

namespace otk {

class RenderColor {
  struct CacheItem {
    GC gc;
    int count;
    CacheItem(GC g) : gc(g), count(0) {}
  };
  static std::map<unsigned long, CacheItem*> *_cache;

  int _screen;
  unsigned char _red;
  unsigned char _green;
  unsigned char _blue;

  GC _gc;

public:
  static void initialize();
  static void destroy();
  
  RenderColor(int screen, unsigned char red,
	      unsigned char green, unsigned char blue);
  virtual ~RenderColor();

  inline int screen() const { return _screen; }
  inline unsigned char red() const { return _red; }
  inline unsigned char green() const { return _green; }
  inline unsigned char blue() const { return _blue; }
  inline GC gc() const { return _gc; }
};

}

#endif // __rendercolor_hh
