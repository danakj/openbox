/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-
 
   obt/bsearch.h for the Openbox window manager
   Copyright (c) 2010        Dana Jansens
 
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

#ifndef __obt_bsearch_h
#define __obt_bsearch_h

#include <glib.h>

G_BEGIN_DECLS

/*! Setup to do a binary search on an array holding elements of type @t */
#define BSEARCH_SETUP(t) \
    register t l_BSEARCH, r_BSEARCH, out_BSEARCH;

/*! Search an array @ar holding elements of type @t, starting at index @start,
  with @size elements, looking for value @val. */
#define BSEARCH(t, ar, start, size, val)         \
{ \
    l_BSEARCH = (start);              \
    r_BSEARCH = (start)+(size)-1;     \
    while (l_BSEARCH <= r_BSEARCH) { \
        /* m is in the middle, but to the left if there's an even number \
           of elements */ \
        out_BSEARCH = l_BSEARCH + (r_BSEARCH - l_BSEARCH)/2;      \
        if ((val) == (ar)[out_BSEARCH]) {                           \
            break; \
        } \
        else if ((val) < (ar)[out_BSEARCH])                       \
            r_BSEARCH = out_BSEARCH-1; /* search to the left side */ \
        else \
            l_BSEARCH = out_BSEARCH+1; /* search to the left side */ \
    } \
}

/*! Returns true if the element last searched for was found in the array */
#define BSEARCH_FOUND() (l_BSEARCH <= r_BSEARCH)
/*! Returns the position in the array at which the element last searched for
  was found. */
#define BSEARCH_AT() (out_BSEARCH)



G_END_DECLS

#endif
