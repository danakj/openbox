// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __gccache_hh
#define __gccache_hh

extern "C" {
#include <X11/Xlib.h>
}

#include "display.hh"
#include "color.hh"

namespace otk {

class GCCacheItem;

class GCCacheContext {
public:
  void set(const Color &_color, const XFontStruct * const _font,
           const int _function, const int _subwindow, const int _linewidth);
  void set(const XFontStruct * const _font);

  ~GCCacheContext(void);

private:
  GCCacheContext()
    : gc(0), pixel(0ul), fontid(0ul),
      function(0), subwindow(0), used(false), screen(~(0u)), linewidth(0) {}

  GC gc;
  unsigned long pixel;
  unsigned long fontid;
  int function;
  int subwindow;
  bool used;
  unsigned int screen;
  int linewidth;

  GCCacheContext(const GCCacheContext &_nocopy);
  GCCacheContext &operator=(const GCCacheContext &_nocopy);

  friend class GCCache;
  friend class GCCacheItem;
};

class GCCacheItem {
public:
  inline const GC &gc(void) const { return ctx->gc; }

private:
  GCCacheItem(void) : ctx(0), count(0), hits(0), fault(false) { }

  GCCacheContext *ctx;
  unsigned int count;
  unsigned int hits;
  bool fault;

  GCCacheItem(const GCCacheItem &_nocopy);
  GCCacheItem &operator=(const GCCacheItem &_nocopy);

  friend class GCCache;
};

class GCCache {
public:
  GCCache(unsigned int screen_count);
  ~GCCache(void);

  // cleans up the cache
  void purge(void);

  GCCacheItem *find(const Color &_color, const XFontStruct * const _font = 0,
                     int _function = GXcopy, int _subwindow = ClipByChildren,
                     int _linewidth = 0);
  void release(GCCacheItem *_item);

private:
  GCCacheContext *nextContext(unsigned int _screen);
  void release(GCCacheContext *ctx);

  // this is closely modelled after the Qt GC cache, but with some of the
  // complexity stripped out
  const unsigned int context_count;
  const unsigned int cache_size;
  const unsigned int cache_buckets;
  const unsigned int cache_total_size;
  GCCacheContext **contexts;
  GCCacheItem **cache;
};

class Pen {
public:
  inline Pen(const Color &_color,  const XFontStruct * const _font = 0,
              int _linewidth = 0, int _function = GXcopy,
              int _subwindow = ClipByChildren)
    : color(_color), font(_font), linewidth(_linewidth), function(_function),
      subwindow(_subwindow), cache(display->gcCache()), item(0) { }

  inline ~Pen(void) { if (item) cache->release(item); }

  inline const GC &gc(void) const {
    if (! item) item = cache->find(color, font, function, subwindow,
                                   linewidth);
    return item->gc();
  }

private:
  const Color &color;
  const XFontStruct *font;
  int linewidth;
  int function;
  int subwindow;

  mutable GCCache *cache;
  mutable GCCacheItem *item;
};

}

#endif // __gccache_hh
