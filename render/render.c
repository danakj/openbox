#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glib.h>
#include "render.h"
#include "gradient.h"
#include "../kernel/openbox.h"

int render_depth;
Visual *render_visual;
Colormap render_colormap;

void render_startup(void)
{
    paint = x_paint;

    render_depth = DefaultDepth(ob_display, ob_screen);
    render_visual = DefaultVisual(ob_display, ob_screen);
    render_colormap = DefaultColormap(ob_display, ob_screen);

    if (render_depth < 8) {
      XVisualInfo vinfo_template, *vinfo_return;
      // search for a TrueColor Visual... if we can't find one...
      // we will use the default visual for the screen
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
              break;          // prefer 24 bit over 32
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
}

void x_paint(Window win, Appearance *l, int w, int h)
{
    int i;
    XImage *im;

    if (w <= 0 || h <= 0) return;

    g_assert(l->surface.type == Surface_Planar);
//    printf("painting window %ld\n", win);

    if (l->pixmap != None) XFreePixmap(ob_display, l->pixmap);
    l->pixmap = XCreatePixmap(ob_display, ob_root, w, h, render_depth);
    g_assert(l->pixmap != None);

    if (l->xftdraw != NULL)
        XftDrawDestroy(l->xftdraw);
    l->xftdraw = XftDrawCreate(ob_display, l->pixmap, render_visual, 
                               render_colormap);
    g_assert(l->xftdraw != NULL);

    if (l->surface.data.planar.pixel_data != NULL)
        g_free(l->surface.data.planar.pixel_data);
    l->surface.data.planar.pixel_data = g_new(pixel32, w * h);

    if (l->surface.data.planar.grad == Background_Solid)
        gradient_solid(l, w, h);
    else gradient_render(&l->surface, w, h);
    for (i = 0; i < l->textures; i++) {
        switch (l->texture[i].type) {
        case Text:
            if (l->xftdraw == NULL) {
                l->xftdraw = XftDrawCreate(ob_display, l->pixmap, 
                                        render_visual, render_colormap);
            }
            font_draw(l->xftdraw, l->texture[i].data.text);
        break;
        }
    }
//reduce depth
    if (l->surface.data.planar.grad != Background_Solid) {
        im = XCreateImage(ob_display, render_visual, render_depth,
                          ZPixmap, 0, NULL, w, h, 32, 0);
        g_assert(im != None);
        im->byte_order = endian;
        im->data = l->surface.data.planar.pixel_data;
        XPutImage(ob_display, l->pixmap, DefaultGC(ob_display, ob_screen),
                  im, 0, 0, 0, 0, w, h);
        im->data = NULL;
        XDestroyImage(im);
    }
    XSetWindowBackgroundPixmap(ob_display, win, l->pixmap);
    XClearWindow(ob_display, win);
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
  if (numtex) out->texture = g_new(Texture, numtex);
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
    copy->texture = NULL; /* XXX FIX ME */
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
    }
    g_free(a);
}
