#ifndef __frame_h
#define __frame_h

#include "geom.h"
#include "client.h"

typedef struct Frame {
    Client *client;

    Window window;
    Window plate;

    Strut  size;
    Rect   area;
    gboolean visible;
} Frame;

/*! Applies gravity to the client's position to find where the frame should
  be positioned.
  @return The proper coordinates for the frame, based on the client.
*/
void frame_client_gravity(Frame *self, int *x, int *y);

/*! Reversly applies gravity to the frame's position to find where the client
  should be positioned.
    @return The proper coordinates for the client, based on the frame.
*/
void frame_frame_gravity(Frame *self, int *x, int *y);


#endif
