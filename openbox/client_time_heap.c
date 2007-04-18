/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-
   
   client_time_heap.c for the Openbox window manager
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

#include "client_time_heap.h"
#include "client.h"

#include <X11/Xlib.h>

/* Helper functions for the heap */

#define isroot(n) (n == 0)
#define parent(n) ((n-1)/2)
#define right(n) ((n+1)*2)
#define left(n) (right(n)-1)
#define exists(n) (n < h->nodes->len)
#define key(n) (((ObClient*)h->nodes->pdata[n])->user_time)

static inline void swap(ObClientTimeHeap *h, guint a, guint b)
{
    gpointer c;

    g_assert(a < h->nodes->len);
    g_assert(b < h->nodes->len);

    c = h->nodes->pdata[a];
    h->nodes->pdata[a] = h->nodes->pdata[b];
    h->nodes->pdata[b] = c;
}

static inline void heapify(ObClientTimeHeap *h, guint n)
{
    g_assert(exists(n));

    /* fix up the heap, move it down below keys it's smaller than */
    while ((exists(left(n)) && key(n) < key(left(n))) ||
           (exists(right(n)) && key(n) < key(right(n))))
    {
        if (exists(left(n)) && exists(right(n)))
            if (key(left(n)) > key(right(n))) {
                swap(h, n, left(n));
                n = left(n);
            } else {
                swap(h, n, right(n));
                n = right(n);
            }
        else {
            /* its impossible in this structure to have a right child but no
               left child */
            swap(h, n, left(n));
            n = left(n);
        }
    }
}

ObClientTimeHeap* client_time_heap_new()
{
    ObClientTimeHeap *h = g_new0(ObClientTimeHeap, 1);
    h->nodes = g_ptr_array_new();
    return h;
}

void client_time_heap_free(ObClientTimeHeap *h)
{
    if (h != NULL) {
        /* all the clients should be removed before the heap is destroyed. */
        g_assert(h->nodes->len == 0);
        g_ptr_array_free(h->nodes, TRUE);
        g_free(h);
    }
}

guint32 client_time_heap_maximum(ObClientTimeHeap *h)
{
    if (h->nodes->len == 0)
        return CurrentTime;
    else
        return key(0);
}


void client_time_heap_add(ObClientTimeHeap *h, ObClient *c)
{
    guint n;

    /* insert it as the last leaf */
    g_ptr_array_add(h->nodes, c);
    n = h->nodes->len - 1;

    /* move it up to its proper place */
    while (!isroot(n) && key(n) > key(parent(n))) {
        swap(h, n, parent(n));
        n = parent(n);
    }
}

void client_time_heap_remove(ObClientTimeHeap *h, ObClient *c)
{
    /* find the client */
    guint n;
    for (n = 0; h->nodes->pdata[n] != c && n < h->nodes->len; ++n);

    /* if the client is in the heap */
    if (n < h->nodes->len) {
        /* move it to a leaf and delete it from the heap */
        swap(h, n, h->nodes->len-1);
        g_ptr_array_remove_index(h->nodes, h->nodes->len-1);

        /* move the swapped leaf down to its proper place if it wasn't just
           deleted */
        if (exists(n))
            heapify(h, n);
    }
}

void client_time_heap_decrease_key(ObClientTimeHeap *h, ObClient *c)
{
    /* find the client */
    guint n;
    for (n = 0; h->nodes->pdata[n] != c && n < h->nodes->len; ++n);

    /* if the client is in the heap */
    if (n < h->nodes->len) {
        /* move it down to its proper place */
        heapify(h, n);
    }
}

void client_time_heap_increase_key(ObClientTimeHeap *h, ObClient *c)
{
    /* find the client */
    guint n;
    for (n = 0; h->nodes->pdata[n] != c && n < h->nodes->len; ++n);

    /* if the client is in the heap */
    if (n < h->nodes->len) {
        /* move it up to its proper place */
        while (!isroot(n) && key(n) > key(parent(n))) {
            swap(h, n, parent(n));
            n = parent(n);
        }
    }
}
