// -*- mode: C; indent-tabs-mode: nil; -*-

#include "../config.h"
#include "gccache.h"
#include "screeninfo.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

static OtkGCCache *gccache;

OtkGCCacheContext *OtkGCCacheContext_New()
{
  OtkGCCacheContext *self = malloc(sizeof(OtkGCCacheContext));

  self->gc = 0;
  self->pixel = 0ul;
  self->fontid = 0ul;
  self->function = 0;
  self->subwindow = 0;
  self->used = False;
  self->screen = ~0;
  self->linewidth = 0;

  return self;
}

void OtkGCCacheContext_Destroy(OtkGCCacheContext *self)
{
  if (self->gc)
    XFreeGC(OBDisplay->display, self->gc);
  free(self);
}

void OtkGCCacheContext_Set(OtkGCCacheContext *self,
			   OtkColor *color, XFontStruct *font,
			   int function, int subwindow, int linewidth)
{
  XGCValues gcv;
  unsigned long mask;
  
  self->pixel = gcv.foreground = OtkColor_Pixel(color);
  self->function = gcv.function = function;
  self->subwindow = gcv.subwindow_mode = subwindow;
  self->linewidth = gcv.line_width = linewidth;
  gcv.cap_style = CapProjecting;

  mask = GCForeground | GCFunction | GCSubwindowMode | GCLineWidth |
         GCCapStyle;

  if (font) {
    self->fontid = gcv.font = font->fid;
    mask |= GCFont;
  } else {
    self->fontid = 0;
  }

  XChangeGC(OBDisplay->display, self->gc, mask, &gcv);
}

void OtkGCCacheContext_SetFont(OtkGCCacheContext *self,
			       XFontStruct *font)
{
  if (!font) {
    self->fontid = 0;
    return;
  }

  XGCValues gcv;
  self->fontid = gcv.font = font->fid;
  XChangeGC(OBDisplay->display, self->gc, GCFont, &gcv);
}


OtkGCCacheItem *OtkGCCacheItem_New()
{
  OtkGCCacheItem *self = malloc(sizeof(OtkGCCacheItem));

  self->ctx = 0;
  self->count = 0;
  self->hits = 0;
  self->fault = False;
}


void OtkGCCache_Initialize(int screen_count)
{
  int i;

  gccache = malloc(sizeof(OtkGCCache));

  gccache->context_count = 128;
  gccache->cache_size = 16;
  gccache->cache_buckets = 8 * screen_count;
  gccache->cache_total_size = gccache->cache_size * gccache->cache_buckets;

  gccache->contexts = malloc(sizeof(OtkGCCacheContext*) *
                             gccache->context_count);
  for (i = 0; i < gccache->context_count; ++i)
    gccache->contexts[i] = OtkGCCacheContext_New();

  gccache->cache = malloc(sizeof(OtkGCCacheItem*) * gccache->cache_total_size);
  for (i = 0; i < gccache->cache_total_size; ++i)
    gccache->cache[i] = OtkGCCacheItem_New();
}


void OtkGCCache_Destroy()
{
  int i;

  for (i = 0; i < gccache->context_count; ++i)
    OtkGCCacheContext_Destroy(gccache->contexts[i]);

  for (i = 0; i < gccache->cache_total_size; ++i)
    free(gccache->cache[i]);

  free(gccache->contexts);
  free(gccache->cache);
  free(gccache);
  gccache = NULL;
}

OtkGCCacheContext *OtkGCCache_NextContext(int screen)
{
  Window hd = OtkDisplay_ScreenInfo(OBDisplay, screen)->root_window;
  OtkGCCacheContext *c;
  int i;

  for (i = 0; i < gccache->context_count; ++i) {
    c = gccache->contexts[i];

    if (! c->gc) {
      c->gc = XCreateGC(OBDisplay->display, hd, 0, 0);
      c->used = False;
      c->screen = screen;
    }
    if (! c->used && c->screen == screen)
      return c;
  }

  fprintf(stderr, "OtkGCCache: context fault!\n");
  abort();
  return NULL; // shut gcc up
}


static void OtkGCCache_InternalRelease(OtkGCCacheContext *ctx)
{
  ctx->used = False;
}

OtkGCCacheItem *OtkGCCache_Find(OtkColor *color, XFontStruct *font,
				int function, int subwindow, int linewidth)
{
  const unsigned long pixel = OtkColor_Pixel(color);
  const unsigned int screen = color->screen;
  const int key = color->red ^ color->green ^ color->blue;
  int k = (key % gccache->cache_size) * gccache->cache_buckets;
  int i = 0; // loop variable
  OtkGCCacheItem *c = gccache->cache[k], *prev = 0;

  /*
    this will either loop cache_buckets times then return/abort or
    it will stop matching
  */
  while (c->ctx &&
         (c->ctx->pixel != pixel || c->ctx->function != function ||
          c->ctx->subwindow != subwindow || c->ctx->screen != screen ||
          c->ctx->linewidth != linewidth)) {
    if (i < (gccache->cache_buckets - 1)) {
      prev = c;
      c = gccache->cache[++k];
      ++i;
      continue;
    }
    if (c->count == 0 && c->ctx->screen == screen) {
      // use this cache item
      OtkGCCacheContext_Set(c->ctx, color, font, function, subwindow,
                            linewidth);
      c->ctx->used = True;
      c->count = 1;
      c->hits = 1;
      return c;
    }
    // cache fault!
    fprintf(stderr, "OtkGCCache: cache fault, count: %d, screen: %d, item screen: %d\n", c->count, screen, c->ctx->screen);
    abort();
  }

  if (c->ctx) {
    // reuse existing context
    if (font && font->fid && font->fid != c->ctx->fontid)
      OtkGCCacheContext_SetFont(c->ctx, font);
    c->count++;
    c->hits++;
    if (prev && c->hits > prev->hits) {
      gccache->cache[k] = prev;
      gccache->cache[k-1] = c;
    }
  } else {
    c->ctx = OtkGCCache_NextContext(screen);
    OtkGCCacheContext_Set(c->ctx, color, font, function, subwindow, linewidth);
    c->ctx->used = True;
    c->count = 1;
    c->hits = 1;
  }

  return c;
}


void OtkGCCache_Release(OtkGCCacheItem *item)
{
  item->count--;
}


void OtkGCCache_Purge()
{
  int i;
  
  for (i = 0; i < gccache->cache_total_size; ++i) {
    OtkGCCacheItem *d = gccache->cache[i];

    if (d->ctx && d->count == 0) {
      release(d->ctx);
      d->ctx = 0;
    }
  }
}
