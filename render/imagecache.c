/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   imagecache.c for the Openbox window manager
   Copyright (c) 2008        Dana Jansens

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

#include "render.h"
#include "imagecache.h"

static gboolean RrImagePicEqual(const RrImagePic *p1,
                                const RrImagePic *p2);

RrImageCache* RrImageCacheNew()
{
    RrImageCache *self;

    self = g_new(RrImageCache, 1);
    self->ref = 1;
    self->table = g_hash_table_new((GHashFunc)RrImagePicHash,
                                   (GEqualFunc)RrImagePicEqual);
    return self;
}

void RrImageCacheRef(RrImageCache *self)
{
    ++self->ref;
}

void RrImageCacheUnref(RrImageCache *self)
{
    if (self && --self->ref == 0) {
        g_assert(g_hash_table_size(self->table) == 0);
        g_hash_table_unref(self->table);

        g_free(self);
    }
}

/*! Finds an image in the cache, if it is already in there */
RrImage* RrImageCacheFind(RrImageCache *self,
                          RrPixel32 *data, gint w, gint h)
{
    RrImagePic pic;
    pic.width = w;
    pic.height = h;
    pic.data = data;
    return g_hash_table_lookup(self->table, &pic);
}

/* This is a fast, reversable hash function called "lookup3", found here:
   http://burtleburtle.net/bob/c/lookup3.c
*/
#define hashsize(n) ((RrPixel32)1<<(n))
#define hashmask(n) (hashsize(n)-1)
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
/* mix -- mix 3 32-bit values reversibly. */
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}
/* final -- final mixing of 3 32-bit values (a,b,c) into c */
#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

guint32 hashword(const guint32 *key, gint length, guint32 initval)
{
    guint32 a,b,c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (((guint32)length)<<2) + initval;

    /* handle most of the key */
    while (length > 3)
    {
        a += key[0];
        b += key[1];
        c += key[2];
        mix(a,b,c);
        length -= 3;
        key += 3;
    }

    /* handle the last 3 guint32's */
    switch(length)      /* all the case statements fall through */
    { 
    case 3: c+=key[2];
    case 2: b+=key[1];
    case 1: a+=key[0];
        final(a,b,c);
    case 0:             /* case 0: nothing left to add */
        break;
    }
    /* report the result */
    return c;
}

#define HASH_INITVAL 0xf00d

guint RrImagePicHash(const RrImagePic *p)
{
    return hashword(p->data, p->width * p->height, HASH_INITVAL);
}

static gboolean RrImagePicEqual(const RrImagePic *p1,
                                const RrImagePic *p2)
{
    guint s1, s2;
    RrPixel32 *data1, *data2;
    gint i;

    if (p1->width != p2->width || p1->height != p2->height) return FALSE;

    /* strcmp() would probably suck on 4k of data.. sum all their values and
       see if they get the same thing.  they already matched on their hashes
       at this point. */
    s1 = s2 = 0;
    data1 = p1->data;
    data2 = p2->data;
    for (i = 0; i < p1->width * p1->height; ++i, ++data1)
        s1 += *data1;
    for (i = 0; i < p2->width * p2->height; ++i, ++data2)
        s2 += *data2;
    return s1 == s2;
}
