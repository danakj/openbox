// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <stdio.h>
}

#include <algorithm>

#include "gccache.hh"
#include "color.hh"
#include "assassin.hh"
#include "screeninfo.hh"

namespace otk {

BGCCacheContext::~BGCCacheContext(void) {
  if (gc)
    XFreeGC(OBDisplay::display, gc);
}


void BGCCacheContext::set(const BColor &_color,
                          const XFontStruct * const _font,
                          const int _function, const int _subwindow,
                          int _linewidth) {
  XGCValues gcv;
  pixel = gcv.foreground = _color.pixel();
  function = gcv.function = _function;
  subwindow = gcv.subwindow_mode = _subwindow;
  linewidth = gcv.line_width = _linewidth;
  gcv.cap_style = CapProjecting;

  unsigned long mask = GCForeground | GCFunction | GCSubwindowMode |
    GCLineWidth | GCCapStyle;

  if (_font) {
    fontid = gcv.font = _font->fid;
    mask |= GCFont;
  } else {
    fontid = 0;
  }

  XChangeGC(OBDisplay::display, gc, mask, &gcv);
}


void BGCCacheContext::set(const XFontStruct * const _font) {
  if (! _font) {
    fontid = 0;
    return;
  }

  XGCValues gcv;
  fontid = gcv.font = _font->fid;
  XChangeGC(OBDisplay::display, gc, GCFont, &gcv);
}


BGCCache::BGCCache(unsigned int screen_count)
  : context_count(128u), cache_size(16u), cache_buckets(8u * screen_count),
    cache_total_size(cache_size * cache_buckets) {

  contexts = new BGCCacheContext*[context_count];
  unsigned int i;
  for (i = 0; i < context_count; i++) {
    contexts[i] = new BGCCacheContext();
  }

  cache = new BGCCacheItem*[cache_total_size];
  for (i = 0; i < cache_total_size; ++i) {
    cache[i] = new BGCCacheItem;
  }
}


BGCCache::~BGCCache(void) {
  std::for_each(contexts, contexts + context_count, PointerAssassin());
  std::for_each(cache, cache + cache_total_size, PointerAssassin());
  delete [] cache;
  delete [] contexts;
}


BGCCacheContext *BGCCache::nextContext(unsigned int scr) {
  Window hd = OBDisplay::screenInfo(scr)->getRootWindow();

  BGCCacheContext *c;

  for (unsigned int i = 0; i < context_count; ++i) {
    c = contexts[i];

    if (! c->gc) {
      c->gc = XCreateGC(OBDisplay::display, hd, 0, 0);
      c->used = false;
      c->screen = scr;
    }
    if (! c->used && c->screen == scr)
      return c;
  }

  fprintf(stderr, "BGCCache: context fault!\n");
  abort();
  return (BGCCacheContext*) 0; // not reached
}


void BGCCache::release(BGCCacheContext *ctx) {
  ctx->used = false;
}


BGCCacheItem *BGCCache::find(const BColor &_color,
                             const XFontStruct * const _font,
                             int _function, int _subwindow, int _linewidth) {
  const unsigned long pixel = _color.pixel();
  const unsigned int screen = _color.screen();
  const int key = _color.red() ^ _color.green() ^ _color.blue();
  int k = (key % cache_size) * cache_buckets;
  unsigned int i = 0; // loop variable
  BGCCacheItem *c = cache[ k ], *prev = 0;

  /*
    this will either loop cache_buckets times then return/abort or
    it will stop matching
  */
  while (c->ctx &&
         (c->ctx->pixel != pixel || c->ctx->function != _function ||
          c->ctx->subwindow != _subwindow || c->ctx->screen != screen ||
          c->ctx->linewidth != _linewidth)) {
    if (i < (cache_buckets - 1)) {
      prev = c;
      c = cache[ ++k ];
      ++i;
      continue;
    }
    if (c->count == 0 && c->ctx->screen == screen) {
      // use this cache item
      c->ctx->set(_color, _font, _function, _subwindow, _linewidth);
      c->ctx->used = true;
      c->count = 1;
      c->hits = 1;
      return c;
    }
    // cache fault!
    fprintf(stderr, "BGCCache: cache fault, count: %d, screen: %d, item screen: %d\n", c->count, screen, c->ctx->screen);
    abort();
  }

  if (c->ctx) {
    // reuse existing context
    if (_font && _font->fid && _font->fid != c->ctx->fontid)
      c->ctx->set(_font);
    c->count++;
    c->hits++;
    if (prev && c->hits > prev->hits) {
      cache[ k     ] = prev;
      cache[ k - 1 ] = c;
    }
  } else {
    c->ctx = nextContext(screen);
    c->ctx->set(_color, _font, _function, _subwindow, _linewidth);
    c->ctx->used = true;
    c->count = 1;
    c->hits = 1;
  }

  return c;
}


void BGCCache::release(BGCCacheItem *_item) {
  _item->count--;
}


void BGCCache::purge(void) {
  for (unsigned int i = 0; i < cache_total_size; ++i) {
    BGCCacheItem *d = cache[ i ];

    if (d->ctx && d->count == 0) {
      release(d->ctx);
      d->ctx = 0;
    }
  }
}

}
