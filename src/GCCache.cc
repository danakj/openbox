// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// GCCache.cc for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh at debian.org>
// Copyright (c) 1997 - 2000, 2002 Bradley T Hughes <bhughes at trolltech.com>
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

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <stdio.h>
}

#include "GCCache.hh"
#include "BaseDisplay.hh"
#include "Color.hh"
#include "Util.hh"


BGCCacheContext::~BGCCacheContext(void) {
  if (gc)
    XFreeGC(display->getXDisplay(), gc);
}


void BGCCacheContext::set(const BColor &_color,
                          const XFontStruct * const _font,
                          const int _function, const int _subwindow) {
  XGCValues gcv;
  pixel = gcv.foreground = _color.pixel();
  function = gcv.function = _function;
  subwindow = gcv.subwindow_mode = _subwindow;
  unsigned long mask = GCForeground | GCFunction | GCSubwindowMode;

  if (_font) {
    fontid = gcv.font = _font->fid;
    mask |= GCFont;
  } else {
    fontid = 0;
  }

  XChangeGC(display->getXDisplay(), gc, mask, &gcv);
}


void BGCCacheContext::set(const XFontStruct * const _font) {
  if (! _font) {
    fontid = 0;
    return;
  }

  XGCValues gcv;
  fontid = gcv.font = _font->fid;
  XChangeGC(display->getXDisplay(), gc, GCFont, &gcv);
}


BGCCache::BGCCache(const BaseDisplay * const _display)
  : display(_display),  context_count(128u),
    cache_size(16u), cache_buckets(8u),
    cache_total_size(cache_size * cache_buckets) {

  contexts = new BGCCacheContext*[context_count];
  unsigned int i;
  for (i = 0; i < context_count; i++) {
    contexts[i] = new BGCCacheContext(display);
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
  Window hd = display->getScreenInfo(scr)->getRootWindow();

  BGCCacheContext *c;

  for (unsigned int i = 0; i < context_count; ++i) {
    c = contexts[i];

    if (! c->gc) {
      c->gc = XCreateGC(display->getXDisplay(), hd, 0, 0);
      c->used = false;
      c->screen = scr;
    }
    if (! c->used && c->screen == scr) {
      c->used = true;
      return c;
    }
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
                             int _function, int _subwindow) {
  const unsigned long pixel = _color.pixel();
  const unsigned int screen = _color.screen();
  const int key = _color.red() ^ _color.green() ^ _color.blue();
  int k = (key % cache_size) * cache_buckets;
  int i = 0; // loop variable
  BGCCacheItem *c = cache[ k ], *prev = 0;

  // this will either loop 8 times then return/abort or it will stop matching
  while (c->ctx &&
         (c->ctx->pixel != pixel || c->ctx->function != _function ||
          c->ctx->subwindow != _subwindow || c->ctx->screen != screen)) {
    if (i < 7) {
      prev = c;
      c = cache[ ++k ];
      ++i;
      continue;
    }
    if (c->count == 0 && c->ctx->screen == screen) {
      // use this cache item
      c->ctx->set(_color, _font, _function, _subwindow);
      c->ctx->used = true;
      c->count = 1;
      c->hits = 1;
      return c;
    }
    // cache fault!
    fprintf(stderr, "BGCCache: cache fault\n");
    abort();
  }

  const unsigned long fontid = _font ? _font->fid : 0;
  if (c->ctx) {
    // reuse existing context
    if (fontid && fontid != c->ctx->fontid)
      c->ctx->set(_font);
    c->count++;
    c->hits++;
    if (prev && c->hits > prev->hits) {
      cache[ k     ] = prev;
      cache[ k - 1 ] = c;
    }
  } else {
    c->ctx = nextContext(screen);
    c->ctx->set(_color, _font, _function, _subwindow);
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
