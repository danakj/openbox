#ifndef __geom_h
#define __geom_h

#ifdef HAVE_ASSERT_H
#  include <assert.h>
#endif

typedef struct Point {
    int x;
    int y;
} Point;

#define POINT_SET(pt, nx, ny) {pt.x = nx; pt.y = ny;}

typedef struct Size {
    int width;
    int height;
} Size;

#define SIZE_SET(sz, w, h) {sz.width = w; sz.height = h;}

typedef struct Rect {
    int x;
    int y;
    int width;
    int height;
} Rect;

#define RECT_SET_POINT(r, nx, ny) \
  {r.x = ny; r.y = ny;}
#define RECT_SET_SIZE(r, w, h) \
  {r.width = w; r.height = h;}
#define RECT_SET(r, nx, ny, w, h) \
  {r.x = nx; r.y = ny; r.width = w; r.height = h;}

#define RECT_EQUAL(r1, r2) (r1.x == r2.x && r1.y == r2.y && \
			    r1.width == r2.width && r1.height == r2.height)

typedef struct Strut {
    int left;
    int top;
    int right;
    int bottom;
} Strut;

#define STRUT_SET(s, l, t, r, b) \
  {s.left = l; s.top = t; s.right = r; s.bottom = b; }

#define STRUT_ADD(s1, s2) \
  {s1.left = MAX(s1.left, s2.left); s1.right = MAX(s1.right, s2.right); \
   s1.top = MAX(s1.top, s2.top); s1.bottom = MAX(s1.bottom, s2.bottom); }

#endif
