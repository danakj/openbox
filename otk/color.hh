// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __color_hh
#define __color_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <map>
#include <string>

namespace otk {

class Color {
public:
  Color(unsigned int _screen = ~(0u));
  Color(int _r, int _g, int _b, unsigned int _screen = ~(0u));
  Color(const std::string &_name, unsigned int _screen = ~(0u));
  ~Color(void);

  inline const std::string &name(void) const { return colorname; }

  inline int   red(void) const { return r; }
  inline int green(void) const { return g; }
  inline int  blue(void) const { return b; }
  void setRGB(int _r, int _g, int _b) {
    deallocate();
    r = _r;
    g = _g;
    b = _b;
  }

  inline unsigned int screen(void) const { return scrn; }
  void setScreen(unsigned int _screen = ~(0u));

  inline bool isAllocated(void) const { return allocated; }

  inline bool isValid(void) const { return r != -1 && g != -1 && b != -1; }

  unsigned long pixel(void) const;

  // operators
#ifndef SWIG
  Color &operator=(const Color &c);
#endif
  inline bool operator==(const Color &c) const
  { return (r == c.r && b == c.b && b == c.b); }
  inline bool operator!=(const Color &c) const
  { return (! operator==(c)); }

  static void cleanupColorCache(void);

private:
  void parseColorName(void);
  void allocate(void);
  void deallocate(void);

  bool allocated;
  int r, g, b;
  unsigned long p;
  unsigned int scrn;
  std::string colorname;

  // global color allocator/deallocator
  struct RGB {
    const int screen;
    const int r, g, b;

    RGB(void) : screen(~(0u)), r(-1), g(-1), b(-1) { }
    RGB(const int b, const int x, const int y, const int z)
      : screen(b), r(x), g(y), b(z) {}
    RGB(const RGB &x)
      : screen(x.screen), r(x.r), g(x.g), b(x.b) {}

    inline bool operator==(const RGB &x) const {
      return screen == x.screen &&
             r == x.r && g == x.g && b == x.b;
    }

    inline bool operator<(const RGB &x) const {
      unsigned long p1, p2;
      p1 = (screen << 24 | r << 16 | g << 8 | b) & 0x00ffffff;
      p2 = (x.screen << 24 | x.r << 16 | x.g << 8 | x.b) & 0x00ffffff;
      return p1 < p2;
    }
  };
  struct PixelRef {
    const unsigned long p;
    unsigned int count;
    inline PixelRef(void) : p(0), count(0) { }
    inline PixelRef(const unsigned long x) : p(x), count(1) { }
  };
  typedef std::map<RGB,PixelRef> ColorCache;
  typedef ColorCache::value_type ColorCacheItem;
  static ColorCache colorcache;
  static bool cleancache;
  static void doCacheCleanup(void);
};

}

#endif // __color_hh
