#include <GL/gl.h>
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
  
    if (sf->data.planar.relief == Flat && sf->data.planar.border) {
        r = sf->data.planar.border_color->r;
        g = sf->data.planar.border_color->g;
        b = sf->data.planar.border_color->b;
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
        current = (r << default_red_offset)
            + (g << default_green_offset)
            + (b << default_blue_offset);
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
        current = (r << default_red_offset)
            + (g << default_green_offset)
            + (b << default_blue_offset);
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
        drx = (float)(sf->data.planar.secondary->r -
                      sf->data.planar.primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->data.planar.secondary->g -
                      sf->data.planar.primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->data.planar.secondary->b -
                      sf->data.planar.primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = 0; x < w; ++x, ++data) {
            r = sf->data.planar.primary->r +
                ((int)(drx * x) + (int)(dry * y))/2;
            g = sf->data.planar.primary->g +
                ((int)(dgx * x) + (int)(dgy * y))/2;
            b = sf->data.planar.primary->b +
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
    pixel32 *data = sf->data.planar.pixel_data;
    pixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y;

    for (y = 0; y < h; ++y) {
        drx = (float)(sf->data.planar.secondary->r -
                      sf->data.planar.primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->data.planar.secondary->g -
                      sf->data.planar.primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->data.planar.secondary->b -
                      sf->data.planar.primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = w; x > 0; --x, ++data) {
            r = sf->data.planar.primary->r +
                ((int)(drx * (x-1)) + (int)(dry * y))/2;
            g = sf->data.planar.primary->g +
                ((int)(dgx * (x-1)) + (int)(dgy * y))/2;
            b = sf->data.planar.primary->b +
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
    r = l->surface.data.planar.primary->r;
    r += r >> 1;
    g = l->surface.data.planar.primary->g;
    g += g >> 1;
    b = l->surface.data.planar.primary->b;
    b += b >> 1;
    if (r > 0xFF) r = 0xFF;
    if (g > 0xFF) g = 0xFF;
    if (b > 0xFF) b = 0xFF;
    g_assert(!l->surface.data.planar.bevel_light);
    l->surface.data.planar.bevel_light = color_new(r, g, b);
    color_allocate_gc(l->surface.data.planar.bevel_light);

    /* dark color */
    r = l->surface.data.planar.primary->r;
    r = (r >> 1) + (r >> 2);
    g = l->surface.data.planar.primary->g;
    g = (g >> 1) + (g >> 2);
    b = l->surface.data.planar.primary->b;
    b = (b >> 1) + (b >> 2);
    g_assert(!l->surface.data.planar.bevel_dark);
    l->surface.data.planar.bevel_dark = color_new(r, g, b);
    color_allocate_gc(l->surface.data.planar.bevel_dark);
}

void gradient_solid(Appearance *l, int x, int y, int w, int h) 
{
    pixel32 pix;
    int i, a, b;
    PlanarSurface *sp = &l->surface.data.planar;
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
    pixel32 *data = sf->data.planar.pixel_data;
    pixel32 *end = data + inw*inh - 1;
    pixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y, h=(inh/2) + 1, w=(inw/2) + 1;
    for (y = 0; y < h; ++y) {
        drx = (float)(sf->data.planar.secondary->r -
                      sf->data.planar.primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->data.planar.secondary->g -
                      sf->data.planar.primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->data.planar.secondary->b -
                      sf->data.planar.primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = 0; x < w; ++x, data) {
            r = sf->data.planar.primary->r +
                ((int)(drx * x) + (int)(dry * y))/2;
            g = sf->data.planar.primary->g +
                ((int)(dgx * x) + (int)(dgy * y))/2;
            b = sf->data.planar.primary->b +
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
    pixel32 *data = sf->data.planar.pixel_data;
    pixel32 *end = data + inw*inh - 1;
    pixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y, h=(inh/2) + 1, w=(inw/2) + 1;
    int val;

    for (y = 0; y < h; ++y) {
        drx = (float)(sf->data.planar.primary->r -
                      sf->data.planar.secondary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->data.planar.primary->g -
                      sf->data.planar.secondary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->data.planar.primary->b -
                      sf->data.planar.secondary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = 0; x < w; ++x, data) {
            if ((float)x/(float)w < (float)y/(float)h) val = (int)(drx * x);
            else val = (int)(dry * y);

            r = sf->data.planar.secondary->r + val;
            g = sf->data.planar.secondary->g + val;
            b = sf->data.planar.secondary->b + val;
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
    pixel32 *data = sf->data.planar.pixel_data;
    pixel32 *end = data + inw*inh - 1;
    pixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y, h=(inh/2) + 1, w=(inw/2) + 1;
    int val;

    for (y = 0; y < h; ++y) {
        drx = (float)(sf->data.planar.secondary->r -
                      sf->data.planar.primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->data.planar.secondary->g -
                      sf->data.planar.primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->data.planar.secondary->b -
                      sf->data.planar.primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = 0; x < w; ++x, data) {
            if ((float)x/(float)w > (float)y/(float)h) val = (int)(drx * x);
            else val = (int)(dry * y);

            r = sf->data.planar.primary->r + val;
            g = sf->data.planar.primary->g + val;
            b = sf->data.planar.primary->b + val;
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

    pr = (float)sf->data.planar.primary->r/255.0;
    pg = (float)sf->data.planar.primary->g/255.0;
    pb = (float)sf->data.planar.primary->b/255.0;
    if (sf->data.planar.secondary) {
        sr = (float)sf->data.planar.secondary->r/255.0;
        sg = (float)sf->data.planar.secondary->g/255.0;
        sb = (float)sf->data.planar.secondary->b/255.0;
    }
    switch (sf->data.planar.grad) {
    case Background_Solid: /* already handled */
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex3i(x, y, 0);
        glVertex3i(x+w, y, 0);
        glVertex3i(x+w, y+h, 0);

        glVertex3i(x+w, y+h, 0);
        glVertex3i(x, y+h, 0);
        glVertex3i(x, y, 0);
        glEnd();
        return;
    case Background_Vertical:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex3i(x, y, 0);
        glColor3f(sr, sg, sb);
        glVertex3i(x+w, y, 0);
        glVertex3i(x+w, y+h, 0);

        glVertex3i(x+w, y+h, 0);
        glColor3f(pr, pg, pb);
        glVertex3i(x, y+h, 0);
        glVertex3i(x, y, 0);
        glEnd();
        break;
    case Background_Horizontal:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex3i(x, y, 0);
        glVertex3i(x+w, y, 0);
        glColor3f(sr, sg, sb);
        glVertex3i(x+w, y+h, 0);

        glVertex3i(x+w, y+h, 0);
        glVertex3i(x, y+h, 0);
        glColor3f(pr, pg, pb);
        glVertex3i(x, y, 0);
        glEnd();
        break;
    case Background_Diagonal:
printf("diagonal\n");
        break;
    case Background_CrossDiagonal:
printf("crossdiagonal\n");
        break;
    case Background_Pyramid:
printf("pyramid\n");
        break;
    case Background_PipeCross:
printf("pipecross\n");
        break;
    case Background_Rectangle:
printf("rect\n");
        break;
    default:
        g_message("unhandled gradient");
        return;
    }
}
#endif /* USE_GL */
