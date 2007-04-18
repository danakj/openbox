/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   client_time_heap.h for the Openbox window manager
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

#ifndef __client_time_heap_h
#define __client_time_heap_h

#include <glib.h>

struct _ObClient *client;

typedef struct _ObClientTimeHeap     ObClientTimeHeap;
typedef struct _ObClientTimeHeapNode ObClientTimeHeapNode;

/*! A min-heap of the clients based on their user_time as the key */
struct _ObClientTimeHeap
{
    /* The nodes in the heap */
    GPtrArray *nodes;
};

ObClientTimeHeap* client_time_heap_new          ();
void              client_time_heap_free         (ObClientTimeHeap *h);

guint32           client_time_heap_maximum      (ObClientTimeHeap *h);

void              client_time_heap_add          (ObClientTimeHeap *h,
                                                 struct _ObClient *c);
void              client_time_heap_remove       (ObClientTimeHeap *h,
                                                 struct _ObClient *c);
void              client_time_heap_decrease_key (ObClientTimeHeap *h,
                                                 struct _ObClient *c);
void              client_time_heap_increase_key (ObClientTimeHeap *h,
                                                 struct _ObClient *c);

#endif
