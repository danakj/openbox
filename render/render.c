#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glib.h>
#include "render.h"
#include "gradient.h"
#include "font.h"
#include "mask.h"
#include "color.h"
#include "image.h"
#include "theme.h"
#include "kernel/openbox.h"

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

int render_depth;
XVisualInfo render_visual_info;

Visual *render_visual;
Colormap render_colormap;
int render_red_offset = 0, render_green_offset = 0, render_blue_offset = 0;
int render_red_shift, render_green_shift, render_blue_shift;
int render_red_mask, render_green_mask, render_blue_mask;

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
        render_visual_info = vinfo_return[best];
      }
      XFree(vinfo_return);
    }  
  switch (render_visual->class) {
  case TrueColor:
    truecolor_startup();
    break;
  case PseudoColor:
  case StaticColor:
  case GrayScale:
  case StaticGray:
    pseudocolor_startup();
    break;
  default:
    g_critical("unsupported visual class.\n");
    exit(EXIT_FAILURE);

  }
}

void truecolor_startup(void)
{
  unsigned long red_mask, green_mask, blue_mask;
  XImage *timage = NULL;

  timage = XCreateImage(ob_display, render_visual, render_depth,
                        ZPixmap, 0, NULL, 1, 1, 32, 0);
  g_assert(timage != NULL);
  /* find the offsets for each color in the visual's masks */
  render_red_mask = red_mask = timage->red_mask;
  render_green_mask = green_mask = timage->green_mask;
  render_blue_mask = blue_mask = timage->blue_mask;

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

void pseudocolor_startup(void)
{
  XColor icolors[256];
  int tr, tg, tb, n, r, g, b, i, incolors, ii;
  unsigned long dev;
  int cpc, _ncolors;
  g_message("Initializing PseudoColor RenderControl\n");

  /* determine the number of colors and the bits-per-color */
  pseudo_bpc = 2; /* XXX THIS SHOULD BE A USER OPTION */
  g_assert(pseudo_bpc >= 1);
  _ncolors = pseudo_ncolors();

  if (_ncolors > 1 << render_depth) {
    g_warning("PseudoRenderControl: Invalid colormap size. Resizing.\n");
    pseudo_bpc = 1 << (render_depth/3) >> 3;
    _ncolors = 1 << (pseudo_bpc * 3);
  }

  /* build a color cube */
  pseudo_colors = malloc(_ncolors * sizeof(XColor));
  cpc = 1 << pseudo_bpc; /* colors per channel */

  for (n = 0, r = 0; r < cpc; r++)
    for (g = 0; g < cpc; g++)
      for (b = 0; b < cpc; b++, n++) {
        tr = (int)(((float)(r)/(float)(cpc-1)) * 0xFF);
        tg = (int)(((float)(g)/(float)(cpc-1)) * 0xFF);
        tb = (int)(((float)(b)/(float)(cpc-1)) * 0xFF);
        pseudo_colors[n].red = tr | tr << 8;
        pseudo_colors[n].green = tg | tg << 8;
        pseudo_colors[n].blue = tb | tb << 8;
        pseudo_colors[n].flags = DoRed|DoGreen|DoBlue; /* used to track 
                                                          allocation */
      }

  /* allocate the colors */
  for (i = 0; i < _ncolors; i++)
    if (!XAllocColor(ob_display, render_colormap, &pseudo_colors[i]))
      pseudo_colors[i].flags = 0; /* mark it as unallocated */

  /* try allocate any colors that failed allocation above */

  /* get the allocated values from the X server (only the first 256 XXX why!?)
   */
  incolors = (((1 << render_depth) > 256) ? 256 : (1 << render_depth));
  for (i = 0; i < incolors; i++)
    icolors[i].pixel = i;
  XQueryColors(ob_display, render_colormap, icolors, incolors);

  /* try match unallocated ones */
  for (i = 0; i < _ncolors; i++) {
    if (!pseudo_colors[i].flags) { /* if it wasn't allocated... */
      unsigned long closest = 0xffffffff, close = 0;
      for (ii = 0; ii < incolors; ii++) {
        /* find deviations */
        r = (pseudo_colors[i].red - icolors[ii].red) & 0xff;
        g = (pseudo_colors[i].green - icolors[ii].green) & 0xff;
        b = (pseudo_colors[i].blue - icolors[ii].blue) & 0xff;
        /* find a weighted absolute deviation */
        dev = (r * r) + (g * g) + (b * b);

        if (dev < closest) {
          closest = dev;
          close = ii;
        }
      }

      pseudo_colors[i].red = icolors[close].red;
      pseudo_colors[i].green = icolors[close].green;
      pseudo_colors[i].blue = icolors[close].blue;
      pseudo_colors[i].pixel = icolors[close].pixel;

      /* try alloc this closest color, it had better succeed! */
      if (XAllocColor(ob_display, render_colormap, &pseudo_colors[i]))
        pseudo_colors[i].flags = DoRed|DoGreen|DoBlue; /* mark as alloced */
      else
        g_assert(FALSE); /* wtf has gone wrong, its already alloced for
                            chissake! */
    }
  }
}

void x_paint(Window win, Appearance *l)
{
    int i, transferred = 0, sw;
    pixel32 *source, *dest;
    Pixmap oldp;
    int x = l->area.x;
    int y = l->area.y;
    int w = l->area.width;
    int h = l->area.height;
    Rect tarea; /* area in which to draw textures */

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
        source = l->surface.data.planar.parent->surface.data.planar.pixel_data
            + l->surface.data.planar.parentx
            + sw * l->surface.data.planar.parenty;
        dest = l->surface.data.planar.pixel_data;
        for (i = 0; i < h; i++, source += sw, dest += w) {
            memcpy(dest, source, w * sizeof(pixel32));
        }
    }
    else if (l->surface.data.planar.grad == Background_Solid)
        gradient_solid(l, x, y, w, h);
    else gradient_render(&l->surface, w, h);

    for (i = 0; i < l->textures; i++) {
        tarea = l->texture[i].position;
        if (l->surface.data.planar.grad != Background_ParentRelative) {
            if (l->surface.data.planar.relief != Flat) {
                switch (l->surface.data.planar.bevel) {
                case Bevel1:
                    tarea.x += 1; tarea.y += 1;
                    tarea.width -= 2; tarea.height -= 2;
                    break;
                case Bevel2:
                    tarea.x += 2; tarea.y += 2;
                    tarea.width -= 4; tarea.height -= 4;
                    break;
                }
            } else if (l->surface.data.planar.border) {
                tarea.x += 1; tarea.y += 1;
                tarea.width -= 2; tarea.height -= 2;
            }
        }

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
                      &tarea);
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
                      &tarea);
        break;
        case RGBA:
            image_draw(l->surface.data.planar.pixel_data, 
                       &l->texture[i].data.rgba,
                       &tarea, &l->area);
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
    p->bevel_dark = NULL;
    p->bevel_light = NULL;
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

        if (spo->bevel_dark != NULL)
            spc->bevel_dark = color_new(spo->bevel_dark->r,
                                        spo->bevel_dark->g,
                                        spo->bevel_dark->b);
        else spc->bevel_dark = NULL;

        if (spo->bevel_light != NULL)
            spc->bevel_light = color_new(spo->bevel_light->r,
                                         spo->bevel_light->g,
                                         spo->bevel_light->b);
        else spc->bevel_light = NULL;

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
    if (a) {
        PlanarSurface *p;
        if (a->pixmap != None) XFreePixmap(ob_display, a->pixmap);
        if (a->xftdraw != NULL) XftDrawDestroy(a->xftdraw);
        if (a->textures)
            g_free(a->texture);
        if (a->surface.type == Surface_Planar) {
            p = &a->surface.data.planar;
            color_free(p->primary);
            color_free(p->secondary);
            color_free(p->border_color);
            color_free(p->bevel_dark);
            color_free(p->bevel_light);
            g_free(p->pixel_data);
        }
        g_free(a);
    }
}


void pixel32_to_pixmap(pixel32 *in, Pixmap out, int x, int y, int w, int h)
{
    pixel32 *scratch;
    XImage *im = NULL;
    im = XCreateImage(ob_display, render_visual, render_depth,
                      ZPixmap, 0, NULL, w, h, 32, 0);
    g_assert(im != NULL);
    im->byte_order = render_endian;
/* this malloc is a complete waste of time on normal 32bpp
   as reduce_depth just sets im->data = data and returns
*/
    scratch = g_new(pixel32, im->width * im->height);
    im->data = (char*) scratch;
    reduce_depth(in, im);
    XPutImage(ob_display, out, DefaultGC(ob_display, ob_screen),
              im, 0, 0, x, y, w, h);
    im->data = NULL;
    XDestroyImage(im);
    g_free(scratch);
}

void appearance_minsize(Appearance *l, int *w, int *h)
{
    int i;
    int m;
    *w = *h = 1;

    switch (l->surface.type) {
    case Surface_Planar:
        if (l->surface.data.planar.relief != Flat) {
            switch (l->surface.data.planar.bevel) {
            case Bevel1:
                *w = *h = 2;
                break;
            case Bevel2:
                *w = *h = 4;
                break;
            }
        } else if (l->surface.data.planar.border)
            *w = *h = 2;

        for (i = 0; i < l->textures; ++i) {
            switch (l->texture[i].type) {
            case Bitmask:
                *w += l->texture[i].data.mask.mask->w;
                *h += l->texture[i].data.mask.mask->h;
                break;
            case Text:
                m = font_measure_string(l->texture[i].data.text.font,
                                        l->texture[i].data.text.string,
                                        l->texture[i].data.text.shadow,
                                        l->texture[i].data.text.offset);
                *w += m;
                m = font_height(l->texture[i].data.text.font,
                                l->texture[i].data.text.shadow,
                                l->texture[i].data.text.offset);
                *h += m;
                break;
            case RGBA:
                *w += l->texture[i].data.rgba.width;
                *h += l->texture[i].data.rgba.height;
                break;
            case NoTexture:
                break;
            }
        }
        break;
    }
}

gboolean render_pixmap_to_rgba(Pixmap pmap, Pixmap mask,
                               int *w, int *h, pixel32 **data)
{
    Window xr;
    int xx, xy;
    guint pw, ph, mw, mh, xb, xd, i, x, y, di;
    XImage *xi, *xm = NULL;

    if (!XGetGeometry(ob_display, pmap, &xr, &xx, &xy, &pw, &ph, &xb, &xd))
        return FALSE;
    if (mask) {
        if (!XGetGeometry(ob_display, mask, &xr, &xx, &xy, &mw, &mh, &xb, &xd))
            return FALSE;
        if (pw != mw || ph != mh || xd != 1)
            return FALSE;
    }

    xi = XGetImage(ob_display, pmap, 0, 0, pw, ph, 0xffffffff, ZPixmap);
    if (!xi)
        return FALSE;

    if (mask) {
        xm = XGetImage(ob_display, mask, 0, 0, mw, mh, 0xffffffff, ZPixmap);
        if (!xm)
            return FALSE;
    }

    *data = g_new(pixel32, pw * ph);
    increase_depth(*data, xi);

    if (mask) {
        /* apply transparency from the mask */
        di = 0;
        for (i = 0, y = 0; y < ph; ++y) {
            for (x = 0; x < pw; ++x, ++i) {
                if (!((((unsigned)xm->data[di + x / 8]) >> (x % 8)) & 0x1))
                    (*data)[i] &= ~(0xff << default_alpha_offset);
            }
            di += xm->bytes_per_line;
        }
    }

    *w = pw;
    *h = ph;

    return TRUE;
}
