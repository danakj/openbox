#ifndef __geom_h
#define __geom_h

typedef struct {
    int width;
    int height;
} RrSize;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} RrRect;

#define RECT_SET(r, nx, ny, w, h) \
    (r).x = (nx), (r).y = (ny), (r).width = (w), (r).height = (h)

#endif
