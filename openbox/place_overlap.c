/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   overlap.c for the Openbox window manager
   Copyright (c) 2011        Ian Zimmerman

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

#include "config.h"
#include "geom.h"
#include "place_overlap.h"

#include <stdlib.h>

static void make_grid(const Rect* client_rects, int n_client_rects,
                      const Rect* bound, int* x_edges, int* y_edges,
                      int max_edges);

static int best_direction(const Point* grid_point,
                          const Rect* client_rects, int n_client_rects,
                          const Rect* bound, const Size* req_size,
                          Point* best_top_left);

/* Choose the placement on a grid with least overlap */

void place_overlap_find_least_placement(const Rect* client_rects,
                                        int n_client_rects,
                                        Rect *const bound,
                                        const Size* req_size,
                                        Point* result)
{
    POINT_SET(*result, 0, 0);
    int overlap = G_MAXINT;
    int max_edges = 2 * (n_client_rects + 1);

    int x_edges[max_edges];
    int y_edges[max_edges];
    make_grid(client_rects, n_client_rects, bound,
            x_edges, y_edges, max_edges);
    int i;
    for (i = 0; i < max_edges; ++i) {
        if (x_edges[i] == G_MAXINT)
            break;
        int j;
        for (j = 0; j < max_edges; ++j) {
            if (y_edges[j] == G_MAXINT)
                break;
            Point grid_point = {.x = x_edges[i], .y = y_edges[j]};
            Point best_top_left;
            int this_overlap =
                best_direction(&grid_point, client_rects, n_client_rects,
                        bound, req_size, &best_top_left);
            if (this_overlap < overlap) {
                overlap = this_overlap;
                *result = best_top_left;
            }
            if (overlap == 0)
                break;
        }
        if (overlap == 0)
            break;
    }
}

static int compare_ints(const void* a, const void* b)
{
    const int* ia = (const int*)a;
    const int* ib = (const int*)b;
    return *ia - *ib;
}

static void uniquify(int* edges, int n_edges)
{
    int i = 0;
    int j = 0;

    while (j < n_edges) {
        int last = edges[j++];
        edges[i++] = last;
        while (j < n_edges && edges[j] == last)
            ++j;
    }
    /* fill the rest with nonsense */
    for (; i < n_edges ; ++i)
        edges[i] = G_MAXINT;
}

static void make_grid(const Rect* client_rects, int n_client_rects,
                      const Rect* bound, int* x_edges, int* y_edges,
                      int max_edges)
{
    int i;
    int n_edges = 0;
    for (i = 0; i < n_client_rects; ++i) {
        if (!RECT_INTERSECTS_RECT(client_rects[i], *bound))
            continue;
        x_edges[n_edges] = client_rects[i].x;
        y_edges[n_edges++] = client_rects[i].y;
        x_edges[n_edges] = client_rects[i].x + client_rects[i].width;
        y_edges[n_edges++] = client_rects[i].y + client_rects[i].height;
    }
    x_edges[n_edges] = bound->x;
    y_edges[n_edges++] = bound->y;
    x_edges[n_edges] = bound->x + bound->width;
    y_edges[n_edges++] = bound->y + bound->height;
    for (i = n_edges; i < max_edges; ++i)
        x_edges[i] = y_edges[i] = G_MAXINT;
    qsort(x_edges, n_edges, sizeof(int), compare_ints);
    uniquify(x_edges, n_edges);
    qsort(y_edges, n_edges, sizeof(int), compare_ints);
    uniquify(y_edges, n_edges);
}

static int total_overlap(const Rect* client_rects, int n_client_rects,
                         const Rect* proposed_rect)
{
    int overlap = 0;
    int i;
    for (i = 0; i < n_client_rects; ++i) {
        if (!RECT_INTERSECTS_RECT(*proposed_rect, client_rects[i]))
            continue;
        Rect rtemp;
        RECT_SET_INTERSECTION(rtemp, *proposed_rect, client_rects[i]);
        overlap += RECT_AREA(rtemp);
    }
    return overlap;
}

/* Given a list of Rect RECTS, a Point PT and a Size size, determine the
   direction from PT which results in the least total overlap with RECTS
   if a rectangle is placed in that direction.  Return the top/left
   Point of such rectangle and the resulting overlap amount.  Only
   consider placements within BOUNDS. */

#define NUM_DIRECTIONS 4

static int best_direction(const Point* grid_point,
                          const Rect* client_rects, int n_client_rects,
                          const Rect* bound, const Size* req_size,
                          Point* best_top_left)
{
    static const Size directions[NUM_DIRECTIONS] = {
        {0, 0}, {0, -1}, {-1, 0}, {-1, -1}
    };
    int overlap = G_MAXINT;
    int i;
    for (i = 0; i < NUM_DIRECTIONS; ++i) {
        Point pt = {
            .x = grid_point->x + (req_size->width * directions[i].width),
            .y = grid_point->y + (req_size->height * directions[i].height)
        };
        Rect r;
        RECT_SET(r, pt.x, pt.y, req_size->width, req_size->height);
        if (!RECT_CONTAINS_RECT(*bound, r))
            continue;
        int this_overlap = total_overlap(client_rects, n_client_rects, &r);
        if (this_overlap < overlap) {
            overlap = this_overlap;
            *best_top_left = pt;
        }
        if (overlap == 0)
            break;
    }
    return overlap;
}
