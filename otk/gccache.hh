// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef GCCACHE_HH
#define GCCACHE_HH

extern "C" {
#include <X11/Xlib.h>
}

#include "display.hh"
#include "color.hh"

namespace otk {

class BGCCacheItem;

class BGCCacheContext {
public:
  void set(const BColor &_color, const XFontStruct * const _font,
           const int _function, const int _subwindow, const int _linewidth);
  void set(const XFontStruct * const _font);

  ~BGCCacheContext(void);

private:
  BGCCacheContext()
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

  BGCCacheContext(const BGCCacheContext &_nocopy);
  BGCCacheContext &operator=(const BGCCacheContext &_nocopy);

  friend class BGCCache;
  friend class BGCCacheItem;
};

class BGCCacheItem {
public:
  inline const GC &gc(void) const { return ctx->gc; }

private:
  BGCCacheItem(void) : ctx(0), count(0), hits(0), fault(false) { }

  BGCCacheContext *ctx;
  unsigned int count;
  unsigned int hits;
  bool fault;

  BGCCacheItem(const BGCCacheItem &_nocopy);
  BGCCacheItem &operator=(const BGCCacheItem &_nocopy);

  friend class BGCCache;
};

class BGCCache {
public:
  BGCCache(unsigned int screen_count);
  ~BGCCache(void);

  // cleans up the cache
  void purge(void);

  BGCCacheItem *find(const BColor &_color, const XFontStruct * const _font = 0,
                     int _function = GXcopy, int _subwindow = ClipByChildren,
                     int _linewidth = 0);
  void release(BGCCacheItem *_item);

private:
  BGCCacheContext *nextContext(unsigned int _screen);
  void release(BGCCacheContext *ctx);

  // this is closely modelled after the Qt GC cache, but with some of the
  // complexity stripped out
  const unsigned int context_count;
  const unsigned int cache_size;
  const unsigned int cache_buckets;
  const unsigned int cache_total_size;
  BGCCacheContext **contexts;
  BGCCacheItem **cache;
};

class BPen {
public:
  inline BPen(const BColor &_color,  const XFontStruct * const _font = 0,
              int _linewidth = 0, int _function = GXcopy,
              int _subwindow = ClipByChildren)
    : color(_color), font(_font), linewidth(_linewidth), function(_function),
      subwindow(_subwindow), cache(OBDisplay::gcCache()), item(0) { }

  inline ~BPen(void) { if (item) cache->release(item); }

  inline const GC &gc(void) const {
    if (! item) item = cache->find(color, font, function, subwindow,
                                   linewidth);
    return item->gc();
  }

private:
  const BColor &color;
  const XFontStruct *font;
  int linewidth;
  int function;
  int subwindow;

  mutable BGCCache *cache;
  mutable BGCCacheItem *item;
};

}

#endif // GCCACHE_HH
