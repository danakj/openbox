#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
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
#include <stdio.h>

int render_depth;
XVisualInfo render_visual_info;

Visual *render_visual;

int render_red_offset = 0, render_green_offset = 0, render_blue_offset = 0;
int render_red_shift, render_green_shift, render_blue_shift;
int render_red_mask, render_green_mask, render_blue_mask;


GLXContext render_glx_context;

int render_glx_rating(XVisualInfo *v)
{
    int rating = 0;
    int val;
    glXGetConfig(ob_display, v, GLX_BUFFER_SIZE, &val);
    printf("buffer size %d\n", val);

    switch (val) {
    case 32:
        rating += 300;
    break;
    case 24:
        rating += 200;
    break;
    case 16:
        rating += 100;
    break;
    }

    glXGetConfig(ob_display, v, GLX_LEVEL, &val);
    printf("level %d\n", val);
    if (val != 0)
        rating = -10000;

    glXGetConfig(ob_display, v, GLX_DEPTH_SIZE, &val);
    printf("depth size %d\n", val);
    switch (val) {
    case 32:
        rating += 30;
    break;
    case 24:
        rating += 20;
    break;
    case 16:
        rating += 10;
    break;
    case 0:
        rating -= 10000;
    }

    glXGetConfig(ob_display, v, GLX_DOUBLEBUFFER, &val);
    printf("double buffer %d\n", val);
    if (val)
        rating++;
    return rating;
}

void render_startup(void)
{
    int count, i = 0, val, best = 0, rate = 0, temp;
    XVisualInfo vimatch, *vilist;

    render_depth = DefaultDepth(ob_display, ob_screen);
    render_visual = DefaultVisual(ob_display, ob_screen);

    vimatch.screen = ob_screen;
    vimatch.class = TrueColor;
    vilist = XGetVisualInfo(ob_display, VisualScreenMask | VisualClassMask,
                            &vimatch, &count);

    if (vilist) {
        printf("looking for a GL visualin %d visuals\n", count);
        for (i = 0; i < count; i++) {
            glXGetConfig(ob_display, &vilist[i], GLX_USE_GL, &val);
            if (val) {
                temp = render_glx_rating(&vilist[i]);
                if (temp > rate) {
                    best = i;
                    rate = temp;
                }
            }
        }
    }
    if (rate > 0) {
        printf("picked visual %d with rating %d\n", best, rate);
        render_depth = vilist[best].depth;
        render_visual = vilist[best].visual;
        render_visual_info = vilist[best];
        render_glx_context = glXCreateContext(ob_display, &render_visual_info,
                                              NULL, True);
        if (render_glx_context == NULL)
            printf("sadness\n");
        else {
            paint = gl_paint;
        }
    }


  switch (render_visual->class) {
  case TrueColor:
    truecolor_startup();
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

void render_shutdown(void)
{
}

Appearance *appearance_new(SurfaceType type, int numtex)
{
  Appearance *out;

  out = g_new(Appearance, 1);
  out->surface.type = type;
  out->textures = numtex;
  if (numtex) out->texture = g_new0(Texture, numtex);
  else out->texture = NULL;

  return out;
}

Appearance *appearance_copy(Appearance *orig)
{
    Appearance *copy = g_new(Appearance, 1);
    copy->surface.type = orig->surface.type;
    switch (orig->surface.type) {
    case Surface_Planar:
        copy->surface.data.planar = orig->surface.data.planar;
    break;
    }
    copy->textures = orig->textures;
    copy->texture = g_memdup(orig->texture, orig->textures * sizeof(Texture));
    return copy;
}

void appearance_free(Appearance *a)
{
    if (a) {
        PlanarSurface *p;
        if (a->textures)
            g_free(a->texture);
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

void gl_paint(Window win, Appearance *l)
{
    int err;
    Window root, child;
    int i, transferred = 0, sw, b, d;
    pixel32 *source, *dest;
    Pixmap oldp;
    int tempx, tempy, absx, absy, absw, absh;
    int x = l->area.x;
    int y = l->area.y;
    int w = l->area.width;
    int h = l->area.height;
    Rect tarea; /* area in which to draw textures */
    if (w <= 0 || h <= 0 || x+w <= 0 || y+h <= 0) return;

    g_assert(l->surface.type == Surface_Planar);

printf("making %p, %p, %p current\n", ob_display, win, render_glx_context);
    err = glXMakeCurrent(ob_display, win, render_glx_context);
g_assert(err != 0);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, 1376, 1032, 0, 0, 10);
    if (XGetGeometry(ob_display, win, &root, &tempx, &tempy,
                     &absw, &absh,  &b, &d) &&
        XTranslateCoordinates(ob_display, win, root, tempx, tempy, 
        &absx, &absy, &child))
        printf("window at %d, %d (%d,%d)\n", absx, absy, absw, absh);
    else
        return;

    glViewport(0, 0, 1376, 1032);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(-absx, 1032-absh-absy, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    if (l->surface.data.planar.grad == Background_ParentRelative) {
        printf("crap\n");
    } else
        render_gl_gradient(&l->surface, absx+x, absy+y, absw, absh);

    glXSwapBuffers(ob_display, win);
}
