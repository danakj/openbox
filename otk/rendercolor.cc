// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "rendercolor.hh"
#include "display.hh"
#include "screeninfo.hh"

#include <cstdio>

namespace otk {

std::map<unsigned long, RenderColor::CacheItem*> *RenderColor::_cache = 0;

void RenderColor::initialize()
{
  _cache = new std::map<unsigned long, CacheItem*>[ScreenCount(**display)];
}

void RenderColor::destroy()
{
  delete [] _cache;
}
  
RenderColor::RenderColor(int screen, unsigned char red,
			 unsigned char green, unsigned char blue)
  : _screen(screen),
    _red(red),
    _green(green),
    _blue(blue),
    _gc(0)
{
  create();
}

RenderColor::RenderColor(int screen, RGB rgb)
  : _screen(screen),
    _red(rgb.r),
    _green(rgb.g),
    _blue(rgb.b),
    _gc(0)
{
  create();
}

void RenderColor::create()
{
  unsigned long color = _blue | _green << 8 | _red << 16;
  
  // try get a gc from the cache
  CacheItem *item = _cache[_screen][color];

  if (item) {
    _gc = item->gc;
    _pixel = item->pixel;
    ++item->count;
  } else {
    XGCValues gcv;

    // allocate a color and GC from the server
    const ScreenInfo *info = display->screenInfo(_screen);

    XColor xcol;    // convert from 0-0xff to 0-0xffff
    xcol.red = _red; xcol.red |= xcol.red << 8;
    xcol.green = _green; xcol.green |= xcol.green << 8;
    xcol.blue = _blue; xcol.blue |= xcol.blue << 8;
    xcol.pixel = 0;

    if (! XAllocColor(**display, info->colormap(), &xcol)) {
      fprintf(stderr, "RenderColor: color alloc error: rgb:%x/%x/%x\n",
	      _red, _green, _blue);
      xcol.pixel = 0;
    }

    _pixel = xcol.pixel;
    gcv.foreground = _pixel;
    gcv.cap_style = CapProjecting;
    _gc = XCreateGC(**display, info->rootWindow(),
		    GCForeground | GCCapStyle, &gcv);
    assert(_gc);

    // insert into the cache
    item = new CacheItem(_gc, _pixel);
    _cache[_screen][color] = item;
    ++item->count;
  }
}

RenderColor::~RenderColor()
{
  unsigned long color = _blue | _green << 8 | _red << 16;

  CacheItem *item = _cache[_screen][color];
  assert(item); // it better be in the cache ...

  if (--item->count <= 0) {
    // remove from the cache
    XFreeGC(**display, _gc);
    _cache[_screen][color] = 0;
    delete item;
  }
}

}
