#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glib.h>
#include "render.h"
#include "gradient.h"
#include "font.h"
#include "mask.h"
#include "color.h"
#include "image.h"
#include "../kernel/openbox.h"

int render_depth;
Visual *render_visual;
Colormap render_colormap;
int render_red_offset = 0, render_green_offset = 0, render_blue_offset = 0;
int render_red_shift, render_green_shift, render_blue_shift;

void render_startup(void)
{
    paint = x_paint;

    render_depth = DefaultDepth(ob_display, ob_screen);
    render_visual = DefaultVisual(ob_display, ob_screen);
    render_colormap = DefaultColormap(ob_display, ob_screen);

    if (render_depth < 8) {
      XVisualInfo vinfo_template, *vinfo_return;
      /* search for a TrueColor Visual... if we can't find one...
         we will use the default visual for the screen */
      int vinfo_nitems;
      int best = -1;

      vinfo_template.screen = ob_screen;
      vinfo_template.class = TrueColor;
      vinfo_return = XGetVisualInfo(ob_display,
                                    VisualScreenMask | VisualClassMask,
                                    &vinfo_template, &vinfo_nitems);
      if (vinfo_return) {
        int i;
        int max_depth = 1;
        for (i = 0; i < vinfo_nitems; ++i) {
          if (vinfo_return[i].depth > max_depth) {
            if (max_depth == 24 && vinfo_return[i].depth > 24)
                break;          /* prefer 24 bit over 32 */
            max_depth = vinfo_return[i].depth;
            best = i;
          }
        }
        if (max_depth < render_depth) best = -1;
      }
      if (best != -1) {
        render_depth = vinfo_return[best].depth;
        render_visual = vinfo_return[best].visual;
        render_colormap = XCreateColormap(ob_display, ob_root, render_visual,
                                          AllocNone);
      }
      XFree(vinfo_return);
    }  
  truecolor_startup();
}

void truecolor_startup(void)
{
  unsigned long red_mask, green_mask, blue_mask;
  XImage *timage = NULL;

  timage = XCreateImage(ob_display, render_visual, render_depth,
                        ZPixmap, 0, NULL, 1, 1, 32, 0);
  g_assert(timage != NULL);
  /* find the offsets for each color in the visual's masks */
  red_mask = timage->red_mask;
  green_mask = timage->green_mask;
  blue_mask = timage->blue_mask;

  render_red_offset = 0;
  render_green_offset = 0;
  render_blue_offset = 0;

  while (! (red_mask & 1))   { render_red_offset++;   red_mask   >>= 1; }
  while (! (green_mask & 1)) { render_green_offset++; green_mask >>= 1; }
  while (! (blue_mask & 1))  { render_blue_offset++;  blue_mask  >>= 1; }

  render_red_shift = render_green_shift = render_blue_shift = 8;
  while (red_mask)   { red_mask   >>= 1; render_red_shift--;   }
  while (green_mask) { green_mask >>= 1; render_green_shift--; }
  while (blue_mask)  { blue_mask  >>= 1; render_blue_shift--;  }
  XFree(timage);
}

void x_paint(Window win, Appearance *l)
{
    int i, transferred = 0, sw, sh;
    pixel32 *source;
    Pixmap oldp;
    int x = l->area.x;
    int y = l->area.y;
    int w = l->area.width;
    int h = l->area.height;

    if (w <= 0 || h <= 0 || x+w <= 0 || y+h <= 0) return;

    g_assert(l->surface.type == Surface_Planar);

    oldp = l->pixmap; /* save to free after changing the visible pixmap */
    l->pixmap = XCreatePixmap(ob_display, ob_root, x+w, y+h, render_depth);
    g_assert(l->pixmap != None);

    if (l->xftdraw != NULL)
        XftDrawDestroy(l->xftdraw);
    l->xftdraw = XftDrawCreate(ob_display, l->pixmap, render_visual, 
                               render_colormap);
    g_assert(l->xftdraw != NULL);

    g_free(l->surface.data.planar.pixel_data);
    l->surface.data.planar.pixel_data = g_new(pixel32, w * h);


    if (l->surface.data.planar.grad == Background_ParentRelative) {
        sw = l->surface.data.planar.parent->area.width;
        source = l->surface.data.planar.pixel_data;
/*        for (i = */
    }
    else if (l->surface.data.planar.grad == Background_Solid)
        gradient_solid(l, x, y, w, h);
    else gradient_render(&l->surface, w, h);

    for (i = 0; i < l->textures; i++) {
        switch (l->texture[i].type) {
        case Text:
            if (!transferred) {
                transferred = 1;
                if (l->surface.data.planar.grad != Background_Solid)
                    pixel32_to_pixmap(l->surface.data.planar.pixel_data, 
                                      l->pixmap,x,y,w,h);
            }
            if (l->xftdraw == NULL) {
                l->xftdraw = XftDrawCreate(ob_display, l->pixmap, 
                                        render_visual, render_colormap);
            }
            font_draw(l->xftdraw, &l->texture[i].data.text, 
                      &l->texture[i].position);
        break;
        case Bitmask:
            if (!transferred) {
                transferred = 1;
                if (l->surface.data.planar.grad != Background_Solid)
                    pixel32_to_pixmap(l->surface.data.planar.pixel_data, 
                                      l->pixmap,x,y,w,h);
            }
            if (l->texture[i].data.mask.color->gc == None)
                color_allocate_gc(l->texture[i].data.mask.color);
            mask_draw(l->pixmap, &l->texture[i].data.mask,
                      &l->texture[i].position);
        break;
        case RGBA:
            image_draw(l->surface.data.planar.pixel_data, 
                       &l->texture[i].data.rgba,
                       &l->texture[i].position);
        break;
        }
    }

    if (!transferred) {
        transferred = 1;
        if (l->surface.data.planar.grad != Background_Solid)
            pixel32_to_pixmap(l->surface.data.planar.pixel_data, l->pixmap
                              ,x,y,w,h);
    }


    XSetWindowBackgroundPixmap(ob_display, win, l->pixmap);
    XClearWindow(ob_display, win);
    if (oldp != None) XFreePixmap(ob_display, oldp);
}

/*
void gl_paint(Window win, Appearance *l)
{
    glXMakeCurrent(ob_display, win, gl_context);
}
*/

void render_shutdown(void)
{
}

Appearance *appearance_new(SurfaceType type, int numtex)
{
  PlanarSurface *p;
  Appearance *out;

  out = g_new(Appearance, 1);
  out->surface.type = type;
  out->textures = numtex;
  out->xftdraw = NULL;
  if (numtex) out->texture = g_new0(Texture, numtex);
  else out->texture = NULL;
  out->pixmap = None;

  switch (type) {
  case Surface_Planar:
    p = &out->surface.data.planar;
    p->primary = NULL;
    p->secondary = NULL;
    p->border_color = NULL;
    p->pixel_data = NULL;
    break;
  }
  return out;
}

Appearance *appearance_copy(Appearance *orig)
{
    PlanarSurface *spo, *spc;
    Appearance *copy = g_new(Appearance, 1);
    copy->surface.type = orig->surface.type;
    switch (orig->surface.type) {
    case Surface_Planar:
        spo = &(orig->surface.data.planar);
        spc = &(copy->surface.data.planar);
        spc->grad = spo->grad;
        spc->relief = spo->relief;
        spc->bevel = spo->bevel;
        if (spo->primary != NULL)
            spc->primary = color_new(spo->primary->r,
                                     spo->primary->g, 
                                     spo->primary->b);
        else spc->primary = NULL;

        if (spo->secondary != NULL)
            spc->secondary = color_new(spo->secondary->r,
                                       spo->secondary->g,
                                       spo->secondary->b);
        else spc->secondary = NULL;

        if (spo->border_color != NULL)
            spc->border_color = color_new(spo->border_color->r,
                                          spo->border_color->g,
                                          spo->border_color->b);
        else spc->border_color = NULL;

        spc->interlaced = spo->interlaced;
        spc->border = spo->border;
        spc->pixel_data = NULL;
    break;
    }
    copy->textures = orig->textures;
    copy->texture = g_memdup(orig->texture, orig->textures * sizeof(Texture));
    copy->pixmap = None;
    copy->xftdraw = NULL;
    return copy;
}

void appearance_free(Appearance *a)
{
    PlanarSurface *p;
    if (a->pixmap != None) XFreePixmap(ob_display, a->pixmap);
    if (a->xftdraw != NULL) XftDrawDestroy(a->xftdraw);
    if (a->textures)
        g_free(a->texture);
    if (a->surface.type == Surface_Planar) {
        p = &a->surface.data.planar;
        if (p->primary != NULL) color_free(p->primary);
        if (p->secondary != NULL) color_free(p->secondary);
        if (p->border_color != NULL) color_free(p->border_color);
        if (p->pixel_data != NULL) g_free(p->pixel_data);
    }
    g_free(a);
}


void pixel32_to_pixmap(pixel32 *in, Pixmap out, int x, int y, int w, int h)
{
    XImage *im = NULL;
    im = XCreateImage(ob_display, render_visual, render_depth,
                      ZPixmap, 0, NULL, w, h, 32, 0);
    g_assert(im != NULL);
    im->byte_order = endian;
    im->data = (char *)in;
    reduce_depth((pixel32*)im->data, im);
    XPutImage(ob_display, out, DefaultGC(ob_display, ob_screen),
              im, 0, 0, x, y, w, h);
    im->data = NULL;
    XDestroyImage(im);
}
