#include <glib.h>
#include "render.h"
#include "gradient.h"
#include "../kernel/openbox.h"
#include "color.h"

void gradient_render(Surface *sf, int w, int h)
{
  pixel32 *data = sf->data.planar.pixel_data;
  pixel32 current;
  unsigned int r,g,b;
  int off, x;

  switch (sf->data.planar.grad) {
  case Background_Solid: /* already handled */
    return;
  case Background_Vertical:
    gradient_vertical(sf, w, h);
    break;
  case Background_Horizontal:
    gradient_horizontal(sf, w, h);
    break;
  case Background_Diagonal:
    gradient_diagonal(sf, w, h);
    break;
  case Background_CrossDiagonal:
    gradient_crossdiagonal(sf, w, h);
    break;
  default:
    g_message("unhandled gradient");
    return;
  }
  
  if (sf->data.planar.relief == Flat && sf->data.planar.border) {
    r = sf->data.planar.border_color->r;
    g = sf->data.planar.border_color->g;
    b = sf->data.planar.border_color->b;
    current = (r << default_red_shift)
            + (g << default_green_shift)
            + (b << default_blue_shift);
    for (off = 0, x = 0; x < w; ++x, off++) {
      *(data + off) = current;
      *(data + off + ((h-1) * w)) = current;
    }
    for (off = 0, x = 0; x < h; ++x, off++) {
      *(data + (off * w)) = current;
      *(data + (off * w) + w - 1) = current;
    }
  }

  if (sf->data.planar.relief != Flat) {
    if (sf->data.planar.bevel == Bevel1) {
      for (off = 1, x = 1; x < w - 1; ++x, off++)
        highlight(data + off,
                  data + off + (h-1) * w,
                  sf->data.planar.relief==Raised);
      for (off = 0, x = 0; x < h; ++x, off++)
        highlight(data + off * w,
                  data + off * w + w - 1,
                  sf->data.planar.relief==Raised);
    }

    if (sf->data.planar.bevel == Bevel2) {
      for (off = 2, x = 2; x < w - 2; ++x, off++)
        highlight(data + off + w,
                  data + off + (h-2) * w,
                  sf->data.planar.relief==Raised);
      for (off = 1, x = 1; x < h-1; ++x, off++)
        highlight(data + off * w + 1,
                  data + off * w + w - 2,
                  sf->data.planar.relief==Raised);
    }
  }
}



void gradient_vertical(Surface *sf, int w, int h)
{
  pixel32 *data = sf->data.planar.pixel_data;
  pixel32 current;
  float dr, dg, db;
  unsigned int r,g,b;
  int x, y;

  dr = (float)(sf->data.planar.secondary->r - sf->data.planar.primary->r);
  dr/= (float)h;

  dg = (float)(sf->data.planar.secondary->g - sf->data.planar.primary->g);
  dg/= (float)h;

  db = (float)(sf->data.planar.secondary->b - sf->data.planar.primary->b);
  db/= (float)h;

  for (y = 0; y < h; ++y) {
    r = sf->data.planar.primary->r + (int)(dr * y);
    g = sf->data.planar.primary->g + (int)(dg * y);
    b = sf->data.planar.primary->b + (int)(db * y);
    current = (r << default_red_shift)
            + (g << default_green_shift)
            + (b << default_blue_shift);
    for (x = 0; x < w; ++x, ++data)
      *data = current;
  }
}

void gradient_horizontal(Surface *sf, int w, int h)
{
  pixel32 *data = sf->data.planar.pixel_data;
  pixel32 current;
  float dr, dg, db;
  unsigned int r,g,b;
  int x, y;

  dr = (float)(sf->data.planar.secondary->r - sf->data.planar.primary->r);
  dr/= (float)w;

  dg = (float)(sf->data.planar.secondary->g - sf->data.planar.primary->g);
  dg/= (float)w;

  db = (float)(sf->data.planar.secondary->b - sf->data.planar.primary->b);
  db/= (float)w;

  for (x = 0; x < w; ++x, ++data) {
    r = sf->data.planar.primary->r + (int)(dr * x);
    g = sf->data.planar.primary->g + (int)(dg * x);
    b = sf->data.planar.primary->b + (int)(db * x);
    current = (r << default_red_shift)
            + (g << default_green_shift)
            + (b << default_blue_shift);
    for (y = 0; y < h; ++y)
      *(data + y*w) = current;
  }
}

void gradient_diagonal(Surface *sf, int w, int h)
{
  pixel32 *data = sf->data.planar.pixel_data;
  pixel32 current;
  float drx, dgx, dbx, dry, dgy, dby;
  unsigned int r,g,b;
  int x, y;

  for (y = 0; y < h; ++y) {
    drx = (float)(sf->data.planar.secondary->r - sf->data.planar.primary->r);
    dry = drx/(float)h;
    drx/= (float)w;

    dgx = (float)(sf->data.planar.secondary->g - sf->data.planar.primary->g);
    dgy = dgx/(float)h;
    dgx/= (float)w;

    dbx = (float)(sf->data.planar.secondary->b - sf->data.planar.primary->b);
    dby = dbx/(float)h;
    dbx/= (float)w;
    for (x = 0; x < w; ++x, ++data) {
      r = sf->data.planar.primary->r + ((int)(drx * x) + (int)(dry * y))/2;
      g = sf->data.planar.primary->g + ((int)(dgx * x) + (int)(dgy * y))/2;
      b = sf->data.planar.primary->b + ((int)(dbx * x) + (int)(dby * y))/2;
      current = (r << default_red_shift)
              + (g << default_green_shift)
              + (b << default_blue_shift);
      *data = current;
    }
  }
}

void gradient_crossdiagonal(Surface *sf, int w, int h)
{
  pixel32 *data = sf->data.planar.pixel_data;
  pixel32 current;
  float drx, dgx, dbx, dry, dgy, dby;
  unsigned int r,g,b;
  int x, y;

  for (y = 0; y < h; ++y) {
    drx = (float)(sf->data.planar.secondary->r - sf->data.planar.primary->r);
    dry = drx/(float)h;
    drx/= (float)w;

    dgx = (float)(sf->data.planar.secondary->g - sf->data.planar.primary->g);
    dgy = dgx/(float)h;
    dgx/= (float)w;

    dbx = (float)(sf->data.planar.secondary->b - sf->data.planar.primary->b);
    dby = dbx/(float)h;
    dbx/= (float)w;
    for (x = w; x > 0; --x, ++data) {
      r = sf->data.planar.primary->r + ((int)(drx * (x-1)) + (int)(dry * y))/2;
      g = sf->data.planar.primary->g + ((int)(dgx * (x-1)) + (int)(dgy * y))/2;
      b = sf->data.planar.primary->b + ((int)(dbx * (x-1)) + (int)(dby * y))/2;
      current = (r << default_red_shift)
              + (g << default_green_shift)
              + (b << default_blue_shift);
      *data = current;
    }
  }
}

void highlight(pixel32 *x, pixel32 *y, gboolean raised)
{
  int r, g, b;

  pixel32 *up, *down;
  if (raised) {
    up = x;
    down = y;
  } else {
    up = y;
    down = x;
  }
  r = (*up >> default_red_shift) & 0xFF;
  r += r >> 1;
  g = (*up >> default_green_shift) & 0xFF;
  g += g >> 1;
  b = (*up >> default_blue_shift) & 0xFF;
  b += b >> 1;
  if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (b > 255) b = 255;
  *up = (r << default_red_shift) + (g << default_green_shift)
      + (b << default_blue_shift);
  
  r = (*down >> default_red_shift) & 0xFF;
  r = (r >> 1) + (r >> 2);
  g = (*down >> default_green_shift) & 0xFF;
  g = (g >> 1) + (g >> 2);
  b = (*down >> default_blue_shift) & 0xFF;
  b = (b >> 1) + (b >> 2);
  *down = (r << default_red_shift) + (g << default_green_shift)
        + (b << default_blue_shift);
}

void gradient_solid(Appearance *l, int x, int y, int w, int h) 
{
  int i;
  PlanarSurface *sp = &l->surface.data.planar;
  int left = x, top = y, right = w - 1, bottom = h - 1;

  if (sp->primary->gc == None)
    color_allocate_gc(sp->primary);
  XFillRectangle(ob_display, l->pixmap, sp->primary->gc
                 , x, y, w, h);

  if (l->surface.data.planar.interlaced) {
    if (sp->secondary->gc == None)
      color_allocate_gc(sp->secondary);
    for (i = y; i < h; i += 2)
      XDrawLine(ob_display, l->pixmap, sp->secondary->gc,
                x, i, w, i);
  }
/*
  switch (texture.relief()) {
  case RenderTexture::Raised:
    switch (texture.bevel()) {
    case RenderTexture::Bevel1:
      XDrawLine(ob_display, l->pixmap, texture.bevelDarkColor().gc(),
                left, bottom, right, bottom);
      XDrawLine(ob_display, l->pixmap, texture.bevelDarkColor().gc(),
                right, bottom, right, top);
                
      XDrawLine(ob_display, l->pixmap, texture.bevelLightColor().gc(),
                left, top, right, top);
      XDrawLine(ob_display, l->pixmap, texture.bevelLightColor().gc(),
                left, bottom, left, top);
      break;
    case RenderTexture::Bevel2:
      XDrawLine(ob_display, l->pixmap, texture.bevelDarkColor().gc(),
                left + 1, bottom - 2, right - 2, bottom - 2);
      XDrawLine(ob_display, l->pixmap, texture.bevelDarkColor().gc(),
                right - 2, bottom - 2, right - 2, top + 1);

      XDrawLine(ob_display, l->pixmap, texture.bevelLightColor().gc(),
                left + 1, top + 1, right - 2, top + 1);
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                left + 1, bottom - 2, left + 1, top + 1);
      break;
    default:
      assert(false); // unhandled RenderTexture::BevelType   
    }
    break;
  case RenderTexture::Sunken:
    switch (texture.bevel()) {
    case RenderTexture::Bevel1:
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                left, bottom, right, bottom);
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                right, bottom, right, top);
      
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left, top, right, top);
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left, bottom, left, top);
      break;
    case RenderTexture::Bevel2:
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                left + 1, bottom - 2, right - 2, bottom - 2);
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                right - 2, bottom - 2, right - 2, top + 1);
      
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left + 1, top + 1, right - 2, top + 1);
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left + 1, bottom - 2, left + 1, top + 1);

      break;
    default:
      assert(false); // unhandled RenderTexture::BevelType   
    }
    break;
  case RenderTexture::Flat:
    if (texture.border())
      XDrawRectangle(**display, sf.pixmap(), texture.borderColor().gc(),
                     left, top, right, bottom);
    break;
  default:  
    assert(false); // unhandled RenderTexture::ReliefType
  }
*/
}
