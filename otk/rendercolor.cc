// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "rendercolor.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "rendercontrol.hh"

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
    _allocated(false),
    _created(false)
{
}

RenderColor::RenderColor(int screen, RGB rgb)
  : _screen(screen),
    _red(rgb.r),
    _green(rgb.g),
    _blue(rgb.b),
    _allocated(false),
    _created(false)
{
}

void RenderColor::create() const
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
    xcol.red = (_red << 8) | _red;
    xcol.green = (_green << 8) | _green;
    xcol.blue = (_blue << 8) | _blue;

    display->renderControl(_screen)->allocateColor(&xcol);
    _allocated = true;

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

  _created = true;
}

unsigned long RenderColor::pixel() const
{
  if (!_created) create();
  return _pixel;
}

GC RenderColor::gc() const
{
  if (!_created) create();
  return _gc;
}

RenderColor::~RenderColor()
{
  unsigned long color = _blue | _green << 8 | _red << 16;

  if (_created) {
    CacheItem *item = _cache[_screen][color];
    assert(item); // better be...

    if (--item->count <= 0) {
      // remove from the cache
      XFreeGC(**display, _gc);
      _cache[_screen][color] = 0;
      delete item;

      if (_allocated) {
        const ScreenInfo *info = display->screenInfo(_screen);
        XFreeColors(**display, info->colormap(), &_pixel, 1, 0);
      }
    }
  }
}

}
