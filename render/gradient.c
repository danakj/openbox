#ifdef USE_GL
#include <GL/gl.h>
#endif /* USE_GL */
#include <glib.h>
#include "render.h"
#include "gradient.h"
#include "../kernel/openbox.h"
#include "color.h"

void gradient_render(Surface *sf, int w, int h)
{
    pixel32 *data = sf->pixel_data;
    pixel32 current;
    unsigned int r,g,b;
    int off, x;

    switch (sf->grad) {
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
    case Background_Pyramid:
        gradient_pyramid(sf, w, h);
        break;
    case Background_PipeCross:
        gradient_pipecross(sf, w, h);
        break;
    case Background_Rectangle:
        gradient_rectangle(sf, w, h);
        break;
    default:
        g_message("unhandled gradient");
        return;
    }
  
    if (sf->relief == Flat && sf->border) {
        r = sf->border_color->r;
        g = sf->border_color->g;
        b = sf->border_color->b;
        current = (r << default_red_offset)
            + (g << default_green_offset)
            + (b << default_blue_offset);
        for (off = 0, x = 0; x < w; ++x, off++) {
            *(data + off) = current;
            *(data + off + ((h-1) * w)) = current;
        }
        for (off = 0, x = 0; x < h; ++x, off++) {
            *(data + (off * w)) = current;
            *(data + (off * w) + w - 1) = current;
        }
    }

    if (sf->relief != Flat) {
        if (sf->bevel == Bevel1) {
            for (off = 1, x = 1; x < w - 1; ++x, off++)
                highlight(data + off,
                          data + off + (h-1) * w,
                          sf->relief==Raised);
            for (off = 0, x = 0; x < h; ++x, off++)
                highlight(data + off * w,
                          data + off * w + w - 1,
                          sf->relief==Raised);
        }

        if (sf->bevel == Bevel2) {
            for (off = 2, x = 2; x < w - 2; ++x, off++)
                highlight(data + off + w,
                          data + off + (h-2) * w,
                          sf->relief==Raised);
            for (off = 1, x = 1; x < h-1; ++x, off++)
                highlight(data + off * w + 1,
                          data + off * w + w - 2,
                          sf->relief==Raised);
        }
    }
}



void gradient_vertical(Surface *sf, int w, int h)
{
    pixel32 *data = sf->pixel_data;
    pixel32 current;
    float dr, dg, db;
    unsigned int r,g,b;
    int x, y;

    dr = (float)(sf->secondary->r - sf->primary->r);
    dr/= (float)h;

    dg = (float)(sf->secondary->g - sf->primary->g);
    dg/= (float)h;

    db = (float)(sf->secondary->b - sf->primary->b);
    db/= (float)h;

    for (y = 0; y < h; ++y) {
        r = sf->primary->r + (int)(dr * y);
        g = sf->primary->g + (int)(dg * y);
        b = sf->primary->b + (int)(db * y);
        current = (r << default_red_offset)
            + (g << default_green_offset)
            + (b << default_blue_offset);
        for (x = 0; x < w; ++x, ++data)
            *data = current;
    }
}

void gradient_horizontal(Surface *sf, int w, int h)
{
    pixel32 *data = sf->pixel_data;
    pixel32 current;
    float dr, dg, db;
    unsigned int r,g,b;
    int x, y;

    dr = (float)(sf->secondary->r - sf->primary->r);
    dr/= (float)w;

    dg = (float)(sf->secondary->g - sf->primary->g);
    dg/= (float)w;

    db = (float)(sf->secondary->b - sf->primary->b);
    db/= (float)w;

    for (x = 0; x < w; ++x, ++data) {
        r = sf->primary->r + (int)(dr * x);
        g = sf->primary->g + (int)(dg * x);
        b = sf->primary->b + (int)(db * x);
        current = (r << default_red_offset)
            + (g << default_green_offset)
            + (b << default_blue_offset);
        for (y = 0; y < h; ++y)
            *(data + y*w) = current;
    }
}

void gradient_diagonal(Surface *sf, int w, int h)
{
    pixel32 *data = sf->pixel_data;
    pixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y;

    for (y = 0; y < h; ++y) {
        drx = (float)(sf->secondary->r -
                      sf->primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->secondary->g -
                      sf->primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->secondary->b -
                      sf->primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = 0; x < w; ++x, ++data) {
            r = sf->primary->r +
                ((int)(drx * x) + (int)(dry * y))/2;
            g = sf->primary->g +
                ((int)(dgx * x) + (int)(dgy * y))/2;
            b = sf->primary->b +
                ((int)(dbx * x) + (int)(dby * y))/2;
            current = (r << default_red_offset)
                + (g << default_green_offset)
                + (b << default_blue_offset);
            *data = current;
        }
    }
}

void gradient_crossdiagonal(Surface *sf, int w, int h)
{
    pixel32 *data = sf->pixel_data;
    pixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y;

    for (y = 0; y < h; ++y) {
        drx = (float)(sf->secondary->r -
                      sf->primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->secondary->g -
                      sf->primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->secondary->b -
                      sf->primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = w; x > 0; --x, ++data) {
            r = sf->primary->r +
                ((int)(drx * (x-1)) + (int)(dry * y))/2;
            g = sf->primary->g +
                ((int)(dgx * (x-1)) + (int)(dgy * y))/2;
            b = sf->primary->b +
                ((int)(dbx * (x-1)) + (int)(dby * y))/2;
            current = (r << default_red_offset)
                + (g << default_green_offset)
                + (b << default_blue_offset);
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
    r = (*up >> default_red_offset) & 0xFF;
    r += r >> 1;
    g = (*up >> default_green_offset) & 0xFF;
    g += g >> 1;
    b = (*up >> default_blue_offset) & 0xFF;
    b += b >> 1;
    if (r > 0xFF) r = 0xFF;
    if (g > 0xFF) g = 0xFF;
    if (b > 0xFF) b = 0xFF;
    *up = (r << default_red_offset) + (g << default_green_offset)
        + (b << default_blue_offset);
  
    r = (*down >> default_red_offset) & 0xFF;
    r = (r >> 1) + (r >> 2);
    g = (*down >> default_green_offset) & 0xFF;
    g = (g >> 1) + (g >> 2);
    b = (*down >> default_blue_offset) & 0xFF;
    b = (b >> 1) + (b >> 2);
    *down = (r << default_red_offset) + (g << default_green_offset)
        + (b << default_blue_offset);
}

static void create_bevel_colors(Appearance *l)
{
    int r, g, b;

    /* light color */
    r = l->surface.primary->r;
    r += r >> 1;
    g = l->surface.primary->g;
    g += g >> 1;
    b = l->surface.primary->b;
    b += b >> 1;
    if (r > 0xFF) r = 0xFF;
    if (g > 0xFF) g = 0xFF;
    if (b > 0xFF) b = 0xFF;
    g_assert(!l->surface.bevel_light);
    l->surface.bevel_light = color_new(r, g, b);
    color_allocate_gc(l->surface.bevel_light);

    /* dark color */
    r = l->surface.primary->r;
    r = (r >> 1) + (r >> 2);
    g = l->surface.primary->g;
    g = (g >> 1) + (g >> 2);
    b = l->surface.primary->b;
    b = (b >> 1) + (b >> 2);
    g_assert(!l->surface.bevel_dark);
    l->surface.bevel_dark = color_new(r, g, b);
    color_allocate_gc(l->surface.bevel_dark);
}

void gradient_solid(Appearance *l, int x, int y, int w, int h) 
{
    pixel32 pix;
    int i, a, b;
    Surface *sp = &l->surface;
    int left = x, top = y, right = x + w - 1, bottom = y + h - 1;

    if (sp->primary->gc == None)
        color_allocate_gc(sp->primary);
    pix = (sp->primary->r << default_red_offset)
        + (sp->primary->g << default_green_offset)
        + (sp->primary->b << default_blue_offset);

    for (a = 0; a < l->area.width; a++)
        for (b = 0; b < l->area.height; b++)
            sp->pixel_data[a + b*l->area.width] = pix;

    XFillRectangle(ob_display, l->pixmap, sp->primary->gc,
                   x, y, w, h);

    if (sp->interlaced) {
        if (sp->secondary->gc == None)
            color_allocate_gc(sp->secondary);
        for (i = y; i < h; i += 2)
            XDrawLine(ob_display, l->pixmap, sp->secondary->gc,
                      x, i, w, i);
    }

    switch (sp->relief) {
    case Raised:
        if (!sp->bevel_dark)
            create_bevel_colors(l);

        switch (sp->bevel) {
        case Bevel1:
            XDrawLine(ob_display, l->pixmap, sp->bevel_dark->gc,
                      left, bottom, right, bottom);
            XDrawLine(ob_display, l->pixmap, sp->bevel_dark->gc,
                      right, bottom, right, top);
                
            XDrawLine(ob_display, l->pixmap, sp->bevel_light->gc,
                      left, top, right, top);
            XDrawLine(ob_display, l->pixmap, sp->bevel_light->gc,
                      left, bottom, left, top);
            break;
        case Bevel2:
            XDrawLine(ob_display, l->pixmap,
                      sp->bevel_dark->gc,
                      left + 1, bottom - 2, right - 2, bottom - 2);
            XDrawLine(ob_display, l->pixmap,
                      sp->bevel_dark->gc,
                      right - 2, bottom - 2, right - 2, top + 1);

            XDrawLine(ob_display, l->pixmap,
                      sp->bevel_light->gc,
                      left + 1, top + 1, right - 2, top + 1);
            XDrawLine(ob_display, l->pixmap,
                      sp->bevel_light->gc,
                      left + 1, bottom - 2, left + 1, top + 1);
            break;
        default:
            g_assert_not_reached(); /* unhandled BevelType */
        }
        break;
    case Sunken:
        if (!sp->bevel_dark)
            create_bevel_colors(l);

        switch (sp->bevel) {
        case Bevel1:
            XDrawLine(ob_display, l->pixmap, sp->bevel_light->gc,
                      left, bottom, right, bottom);
            XDrawLine(ob_display, l->pixmap, sp->bevel_light->gc,
                      right, bottom, right, top);
      
            XDrawLine(ob_display, l->pixmap, sp->bevel_dark->gc,
                      left, top, right, top);
            XDrawLine(ob_display, l->pixmap, sp->bevel_dark->gc,
                      left, bottom, left, top);
            break;
        case Bevel2:
            XDrawLine(ob_display, l->pixmap, sp->bevel_light->gc,
                      left + 1, bottom - 2, right - 2, bottom - 2);
            XDrawLine(ob_display, l->pixmap, sp->bevel_light->gc,
                      right - 2, bottom - 2, right - 2, top + 1);
      
            XDrawLine(ob_display, l->pixmap, sp->bevel_dark->gc,
                      left + 1, top + 1, right - 2, top + 1);
            XDrawLine(ob_display, l->pixmap, sp->bevel_dark->gc,
                      left + 1, bottom - 2, left + 1, top + 1);

            break;
        default:
            g_assert_not_reached(); /* unhandled BevelType */
        }
        break;
    case Flat:
        if (sp->border) {
            if (sp->border_color->gc == None)
                color_allocate_gc(sp->border_color);
            XDrawRectangle(ob_display, l->pixmap, sp->border_color->gc,
                           left, top, right, bottom);
        }
        break;
    default:  
        g_assert_not_reached(); /* unhandled ReliefType */
    }
}

void gradient_pyramid(Surface *sf, int inw, int inh)
{
    pixel32 *data = sf->pixel_data;
    pixel32 *end = data + inw*inh - 1;
    pixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y, h=(inh/2) + 1, w=(inw/2) + 1;
    for (y = 0; y < h; ++y) {
        drx = (float)(sf->secondary->r -
                      sf->primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->secondary->g -
                      sf->primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->secondary->b -
                      sf->primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = 0; x < w; ++x, data) {
            r = sf->primary->r +
                ((int)(drx * x) + (int)(dry * y))/2;
            g = sf->primary->g +
                ((int)(dgx * x) + (int)(dgy * y))/2;
            b = sf->primary->b +
                ((int)(dbx * x) + (int)(dby * y))/2;
            current = (r << default_red_offset)
                + (g << default_green_offset)
                + (b << default_blue_offset);
            *(data+x) = current;
            *(data+inw-x) = current;
            *(end-x) = current;
            *(end-(inw-x)) = current;
        }
        data+=inw;
        end-=inw;
    }
}

void gradient_rectangle(Surface *sf, int inw, int inh)
{
    pixel32 *data = sf->pixel_data;
    pixel32 *end = data + inw*inh - 1;
    pixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y, h=(inh/2) + 1, w=(inw/2) + 1;
    int val;

    for (y = 0; y < h; ++y) {
        drx = (float)(sf->primary->r -
                      sf->secondary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->primary->g -
                      sf->secondary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->primary->b -
                      sf->secondary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = 0; x < w; ++x, data) {
            if ((float)x/(float)w < (float)y/(float)h) val = (int)(drx * x);
            else val = (int)(dry * y);

            r = sf->secondary->r + val;
            g = sf->secondary->g + val;
            b = sf->secondary->b + val;
            current = (r << default_red_offset)
                + (g << default_green_offset)
                + (b << default_blue_offset);
            *(data+x) = current;
            *(data+inw-x) = current;
            *(end-x) = current;
            *(end-(inw-x)) = current;
        }
        data+=inw;
        end-=inw;
    }
}

void gradient_pipecross(Surface *sf, int inw, int inh)
{
    pixel32 *data = sf->pixel_data;
    pixel32 *end = data + inw*inh - 1;
    pixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y, h=(inh/2) + 1, w=(inw/2) + 1;
    int val;

    for (y = 0; y < h; ++y) {
        drx = (float)(sf->secondary->r -
                      sf->primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->secondary->g -
                      sf->primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->secondary->b -
                      sf->primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = 0; x < w; ++x, data) {
            if ((float)x/(float)w > (float)y/(float)h) val = (int)(drx * x);
            else val = (int)(dry * y);

            r = sf->primary->r + val;
            g = sf->primary->g + val;
            b = sf->primary->b + val;
            current = (r << default_red_offset)
                + (g << default_green_offset)
                + (b << default_blue_offset);
            *(data+x) = current;
            *(data+inw-x) = current;
            *(end-x) = current;
            *(end-(inw-x)) = current;
        }
        data+=inw;
        end-=inw;
    }
}
#ifdef USE_GL
void render_gl_gradient(Surface *sf, int x, int y, int w, int h)
{
    float pr,pg,pb;
    float sr, sg, sb;
    float ar, ag, ab;

    pr = (float)sf->primary->r/255.0;
    pg = (float)sf->primary->g/255.0;
    pb = (float)sf->primary->b/255.0;
    if (sf->secondary) {
        sr = (float)sf->secondary->r/255.0;
        sg = (float)sf->secondary->g/255.0;
        sb = (float)sf->secondary->b/255.0;
    }
    switch (sf->grad) {
    case Background_Solid: /* already handled */
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glVertex2i(x+w, y);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glVertex2i(x, y+h);
        glVertex2i(x, y);
        glEnd();
        return;
    case Background_Horizontal:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_Vertical:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glVertex2i(x, y+h);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_Diagonal:
	ar = (pr + sr) / 2.0;
	ag = (pg + sg) / 2.0;
	ab = (pb + sb) / 2.0;
        glBegin(GL_TRIANGLES);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y+h);

        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x, y+h);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_CrossDiagonal:
	ar = (pr + sr) / 2.0;
	ag = (pg + sg) / 2.0;
	ab = (pb + sb) / 2.0;
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y+h);

        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y+h);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y+h);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_Pyramid:
	ar = (pr + sr) / 2.0;
	ag = (pg + sg) / 2.0;
	ab = (pb + sb) / 2.0;
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y+h/2);

        glVertex2i(x, y+h/2);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w/2, y+h);

        glVertex2i(x+w/2, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y+h/2);

        glVertex2i(x+w, y+h/2);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w/2, y);

        glVertex2i(x+w/2, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_PipeCross:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x, y+h/2);

        glVertex2i(x, y+h/2);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w/2, y+h);

        glVertex2i(x+w/2, y+h);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w, y+h/2);

        glVertex2i(x+w, y+h/2);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w/2, y);

        glVertex2i(x+w/2, y);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_Rectangle:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);

        glEnd();
        break;
    default:
        g_message("unhandled gradient");
        return;
    }
}
#endif /* USE_GL */
