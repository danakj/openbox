// -*- mode: C; indent-tabs-mode: nil; -*-
#ifndef __gccache_h
#define __gccache_h

#include <X11/Xlib.h>

#include "display.h"
#include "color.h"

struct OtkGCCacheItem;

typedef struct OtkGCCacheContext {
  GC gc;
  unsigned long pixel;
  unsigned long fontid;
  int function;
  int subwindow;
  Bool used;
  int screen;
  int linewidth;
} OtkGCCacheContext;

OtkGCCacheContext *OtkGCCacheContext_New();
void OtkGCCacheContext_Destroy(OtkGCCacheContext *self);

void OtkGCCacheContext_Set(OtkGCCacheContext *self,
			   OtkColor *color, XFontStruct *font,
			   int function, int subwindow, int linewidth);
void OtkGCCacheContext_SetFont(OtkGCCacheContext *self,
			       XFontStruct *font);


typedef struct OtkGCCacheItem {
  OtkGCCacheContext *ctx;
  unsigned int count;
  unsigned int hits;
  Bool fault;
} OtkGCCacheItem;

OtkGCCacheItem *OtkGCCacheItem_New();


typedef struct OtkGCCache {
  // this is closely modelled after the Qt GC cache, but with some of the
  // complexity stripped out
  unsigned int context_count;
  unsigned int cache_size;
  unsigned int cache_buckets;
  unsigned int cache_total_size;
  OtkGCCacheContext **contexts;
  OtkGCCacheItem **cache;
} OtkGCCache;

void OtkGCCache_Initialize(int screen_count);
void OtkGCCache_Destroy();

// cleans up the cache
void OtkGCCache_Purge();

OtkGCCacheItem *OtkGCCache_Find(OtkColor *color,
				XFontStruct *font, int function,
				int subwindow, int linewidth);
void OtkGCCache_Release(OtkGCCacheItem *item);


/*


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

}*/

#endif // __gccache_h
