/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   image.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "geom.h"
#include "image.h"
#include "color.h"
#include "imagecache.h"

#include <glib.h>

#define FRACTION        12
#define FLOOR(i)        ((i) & (~0UL << FRACTION))
#define AVERAGE(a, b)   (((((a) ^ (b)) & 0xfefefefeL) >> 1) + ((a) & (b)))

static void AddPicture(RrImage *self, RrImagePic ***list, gint *len,
                       RrImagePic *pic)
{
    gint i;

    g_assert(pic->width > 0 && pic->height > 0);

    g_assert(g_hash_table_lookup(self->cache->table, pic) == NULL);

    /* grow the list */
    *list = g_renew(RrImagePic*, *list, ++*len);

    /* move everything else down one */
    for (i = *len-1; i > 0; --i)
        (*list)[i] = (*list)[i-1];

    /* set the new picture up at the front of the list */
    (*list)[0] = pic;

    /* add the picture as a key to point to this image in the cache */
    g_hash_table_insert(self->cache->table, (*list)[0], self);

#ifdef DEBUG
    g_print("Adding %s picture to the cache: "
            "Image 0x%x, w %d h %d Hash %u\n",
            (*list == self->original ? "ORIGINAL" : "RESIZED"),
            (guint)self, pic->width, pic->height, RrImagePicHash(pic));
#endif
}

static void RemovePicture(RrImage *self, RrImagePic ***list,
                          gint i, gint *len)
{
    gint j;

#ifdef DEBUG
    g_print("Removing %s picture from the cache: "
            "Image 0x%x, w %d h %d Hash %u\n",
            (*list == self->original ? "ORIGINAL" : "RESIZED"),
            (guint)self, (*list)[i]->width, (*list)[i]->height,
            RrImagePicHash((*list)[i]));
#endif

    /* remove the picture as a key in the cache */
    g_hash_table_remove(self->cache->table, (*list)[i]);

    /* free the picture (and its rgba data) */
    g_free((*list)[i]);
    g_free((*list)[i]->data);
    /* shift everything down one */
    for (j = i; j < *len-1; ++j)
        (*list)[j] = (*list)[j+1];
    /* shrink the list */
    *list = g_renew(RrImagePic*, *list, --*len);
}

static RrImagePic* ResizeImage(RrPixel32 *src,
                                       gulong srcW, gulong srcH,
                                       gulong dstW, gulong dstH)
{
    RrPixel32 *dst;
    RrImagePic *pic;
    gulong dstX, dstY, srcX, srcY;
    gulong srcX1, srcX2, srcY1, srcY2;
    gulong ratioX, ratioY;
    gulong aspectW, aspectH;

    /* keep the aspect ratio */
    aspectW = dstW;
    aspectH = (gint)(dstW * ((gdouble)srcH / srcW));
    if (aspectH > dstH) {
        aspectH = dstH;
        aspectW = (gint)(dstH * ((gdouble)srcW / srcH));
    }
    dstW = aspectW;
    dstH = aspectH;

    if (srcW == dstW && srcH == dstH)
        return NULL; /* no scaling needed ! */

    pic = g_new(RrImagePic, 1);
    dst = g_new(RrPixel32, dstW * dstH);
    pic->width = dstW;
    pic->height = dstH;
    pic->data = dst;

    ratioX = (srcW << FRACTION) / dstW;
    ratioY = (srcH << FRACTION) / dstH;

    srcY2 = 0;
    for (dstY = 0; dstY < dstH; dstY++) {
        srcY1 = srcY2;
        srcY2 += ratioY;

        srcX2 = 0;
        for (dstX = 0; dstX < dstW; dstX++) {
            gulong red = 0, green = 0, blue = 0, alpha = 0;
            gulong portionX, portionY, portionXY, sumXY = 0;
            RrPixel32 pixel;

            srcX1 = srcX2;
            srcX2 += ratioX;

            for (srcY = srcY1; srcY < srcY2; srcY += (1UL << FRACTION)) {
                if (srcY == srcY1) {
                    srcY = FLOOR(srcY);
                    portionY = (1UL << FRACTION) - (srcY1 - srcY);
                    if (portionY > srcY2 - srcY1)
                        portionY = srcY2 - srcY1;
                }
                else if (srcY == FLOOR(srcY2))
                    portionY = srcY2 - srcY;
                else
                    portionY = (1UL << FRACTION);

                for (srcX = srcX1; srcX < srcX2; srcX += (1UL << FRACTION)) {
                    if (srcX == srcX1) {
                        srcX = FLOOR(srcX);
                        portionX = (1UL << FRACTION) - (srcX1 - srcX);
                        if (portionX > srcX2 - srcX1)
                            portionX = srcX2 - srcX1;
                    }
                    else if (srcX == FLOOR(srcX2))
                        portionX = srcX2 - srcX;
                    else
                        portionX = (1UL << FRACTION);

                    portionXY = (portionX * portionY) >> FRACTION;
                    sumXY += portionXY;

                    pixel = *(src + (srcY >> FRACTION) * srcW
                            + (srcX >> FRACTION));
                    red   += ((pixel >> RrDefaultRedOffset)   & 0xFF)
                             * portionXY;
                    green += ((pixel >> RrDefaultGreenOffset) & 0xFF)
                             * portionXY;
                    blue  += ((pixel >> RrDefaultBlueOffset)  & 0xFF)
                             * portionXY;
                    alpha += ((pixel >> RrDefaultAlphaOffset) & 0xFF)
                             * portionXY;
                }
            }

            g_assert(sumXY != 0);
            red   /= sumXY;
            green /= sumXY;
            blue  /= sumXY;
            alpha /= sumXY;

            *dst++ = (red   << RrDefaultRedOffset)   |
                     (green << RrDefaultGreenOffset) |
                     (blue  << RrDefaultBlueOffset)  |
                     (alpha << RrDefaultAlphaOffset);
        }
    }

    return pic;
}

/*! This drawns an RGBA picture into the target, within the rectangle specified
  by the area parameter.  If the area's size differs from the source's then it
  will be centered within the rectangle */
void DrawRGBA(RrPixel32 *target, gint target_w, gint target_h,
              RrPixel32 *source, gint source_w, gint source_h,
              gint alpha, RrRect *area)
{
    RrPixel32 *dest;
    gint col, num_pixels;
    gint dw, dh;

    g_assert(source_w <= area->width && source_h <= area->height);

    /* keep the aspect ratio */
    dw = area->width;
    dh = (gint)(dw * ((gdouble)source_h / source_w));
    if (dh > area->height) {
        dh = area->height;
        dw = (gint)(dh * ((gdouble)source_w / source_h));
    }

    /* copy source -> dest, and apply the alpha channel.
       center the image if it is smaller than the area */
    col = 0;
    num_pixels = dw * dh;
    dest = target + area->x + (area->width - dw) / 2 +
        (target_w * (area->y + (area->height - dh) / 2));
    while (num_pixels-- > 0) {
        guchar a, r, g, b, bgr, bgg, bgb;

        /* apply the rgba's opacity as well */
        a = ((*source >> RrDefaultAlphaOffset) * alpha) >> 8;
        r = *source >> RrDefaultRedOffset;
        g = *source >> RrDefaultGreenOffset;
        b = *source >> RrDefaultBlueOffset;

        /* background color */
        bgr = *dest >> RrDefaultRedOffset;
        bgg = *dest >> RrDefaultGreenOffset;
        bgb = *dest >> RrDefaultBlueOffset;

        r = bgr + (((r - bgr) * a) >> 8);
        g = bgg + (((g - bgg) * a) >> 8);
        b = bgb + (((b - bgb) * a) >> 8);

        *dest = ((r << RrDefaultRedOffset) |
                 (g << RrDefaultGreenOffset) |
                 (b << RrDefaultBlueOffset));

        dest++;
        source++;

        if (++col >= dw) {
            col = 0;
            dest += target_w - dw;
        }
    }
}

void RrImageDrawRGBA(RrPixel32 *target, RrTextureRGBA *rgba,
                     gint target_w, gint target_h,
                     RrRect *area)
{
    RrImagePic *scaled;

    scaled = ResizeImage(rgba->data, rgba->width, rgba->height,
                         area->width, area->height);

    if (scaled) {
#ifdef DEBUG
            g_warning("Scaling an RGBA! You should avoid this and just make "
                      "it the right size yourself!");
#endif
            DrawRGBA(target, target_w, target_h,
                     scaled->data, scaled->width, scaled->height,
                     rgba->alpha, area);
    }
    else
        DrawRGBA(target, target_w, target_h,
                 rgba->data, rgba->width, rgba->height,
                 rgba->alpha, area);
}

RrImage* RrImageNew(RrImageCache *cache)
{
    RrImage *self;

    self = g_new0(RrImage, 1);
    self->ref = 1;
    self->cache = cache;
    return self;
}

void RrImageRef(RrImage *self)
{
    ++self->ref;
}

void RrImageUnref(RrImage *self)
{
    if (self && --self->ref == 0) {
#ifdef DEBUG
        g_print("Refcount to 0, removing ALL pictures from the cache: "
                  "Image 0x%x\n", (guint)self);
#endif
        while (self->n_original > 0)
            RemovePicture(self, &self->original, 0, &self->n_original);
        while (self->n_resized > 0)
            RemovePicture(self, &self->resized, 0, &self->n_resized);
        g_free(self);
    }
}

void RrImageAddPicture(RrImage *self, RrPixel32 *data, gint w, gint h)
{
    gint i;
    RrImagePic *pic;

    /* make sure we don't already have this size.. */
    for (i = 0; i < self->n_original; ++i)
        if (self->original[i]->width == w && self->original[i]->height == h) {
#ifdef DEBUG
            g_print("Found duplicate ORIGINAL image: "
                    "Image 0x%x, w %d h %d\n", (guint)self, w, h);
#endif
            return;
        }

    /* remove any resized pictures of this same size */
    for (i = 0; i < self->n_resized; ++i)
        if (self->resized[i]->width == w || self->resized[i]->height == h) {
            RemovePicture(self, &self->resized, i, &self->n_resized);
            break;
        }

    /* add the new picture */
    pic = g_new(RrImagePic, 1);
    pic->width = w;
    pic->height = h;
    pic->data = g_memdup(data, w*h*sizeof(RrPixel32));
    AddPicture(self, &self->original, &self->n_original, pic);
}

void RrImageRemovePicture(RrImage *self, gint w, gint h)
{
    gint i;

    /* remove any resized pictures of this same size */
    for (i = 0; i < self->n_original; ++i)
        if (self->original[i]->width == w && self->original[i]->height == h) {
            RemovePicture(self, &self->original, i, &self->n_original);
            break;
        }
}

void RrImageDrawImage(RrPixel32 *target, RrTextureImage *img,
                      gint target_w, gint target_h,
                      RrRect *area)
{
    gint i, min_diff, min_i, min_aspect_diff, min_aspect_i;
    RrImage *self;
    RrImagePic *pic;

    self = img->image;
    pic = NULL;

    /* is there an original of this size? (only w or h has to be right cuz
       we maintain aspect ratios) */
    for (i = 0; i < self->n_original; ++i)
        if (self->original[i]->width == area->width ||
            self->original[i]->height == area->height)
        {
            pic = self->original[i];
            break;
        }

    /* is there a resize of this size? */
    for (i = 0; i < self->n_resized; ++i)
        if (self->resized[i]->width == area->width ||
            self->resized[i]->height == area->height)
        {
            gint j;
            RrImagePic *saved;

            /* save the selected one */
            saved = self->resized[i];

            /* shift all the others down */
            for (j = i; j > 0; --j)
                self->resized[j] = self->resized[j-1];

            /* and move the selected one to the top of the list */
            self->resized[0] = saved;

            pic = self->resized[0];
            break;
        }

    if (!pic) {
        gdouble aspect;

        /* find an original with a close size */
        min_diff = min_aspect_diff = -1;
        min_i = min_aspect_i = 0;
        aspect = ((gdouble)area->width) / area->height;
        for (i = 0; i < self->n_original; ++i) {
            gint diff;
            gint wdiff, hdiff;
            gdouble myasp;

            /* our size difference metric.. */
            wdiff = self->original[i]->width - area->width;
            hdiff = self->original[i]->height - area->height;
            diff = (wdiff * wdiff) + (hdiff * hdiff);

            /* find the smallest difference */
            if (min_diff < 0 || diff < min_diff) {
                min_diff = diff;
                min_i = i;
            }
            /* and also find the smallest difference with the same aspect
               ratio (and prefer this one) */
            myasp = ((gdouble)self->original[i]->width) /
                self->original[i]->height;
            if (ABS(aspect - myasp) < 0.0000001 &&
                (min_aspect_diff < 0 || diff < min_aspect_diff))
            {
                min_aspect_diff = diff;
                min_aspect_i = i;
            }
        }

        /* use the aspect ratio correct source if there is one */
        if (min_aspect_i >= 0)
            min_i = min_aspect_i;

        /* resize the original to the given area */
        pic = ResizeImage(self->original[min_i]->data,
                          self->original[min_i]->width,
                          self->original[min_i]->height,
                          area->width, area->height);

        /* add the resized image to the image, as the first in the resized
           list */
        if (self->n_resized >= MAX_CACHE_RESIZED) {
            /* remove the last one (last used one) */
            RemovePicture(self, &self->resized, self->n_resized - 1,
                          &self->n_resized);
        }
        /* add it to the top of the resized list */
        AddPicture(self, &self->resized, &self->n_resized, pic);
    }

    g_assert(pic != NULL);

    DrawRGBA(target, target_w, target_h,
             pic->data, pic->width, pic->height,
             img->alpha, area);
}
