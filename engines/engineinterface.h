#ifndef __engineinterface_h
#define __engineinterface_h

#include "../kernel/frame.h"
#include <glib.h>

/* startup */
typedef gboolean EngineStartup();

/* shutdown */
typedef void EngineShutdown();

/* frame_new */
typedef Frame *EngineFrameNew();

/* frame_grab_client */
typedef void EngineFrameGrabClient(Frame *self, Client *client);
/* frame_release_client */
typedef void EngineFrameReleaseClient(Frame *self, Client *client);

/* frame_adjust_area */
/*! Update the frame's size/position to match the client */
typedef void EngineFrameAdjustArea(Frame *self, gboolean moved,
                                   gboolean resized);
/* frame_adjust_shape */
/*! Shape the frame window to the client window */
typedef void EngineFrameAdjustShape(Frame *self);
/* frame_adjust_state */
/*! Update the frame to match the client's new state (for things like toggle
  buttons, focus, and the title) XXX break this up */
typedef void EngineFrameAdjustState(Frame *self);
/* frame_adjust_focus */
/*! Update the frame to match the client's focused state */
typedef void EngineFrameAdjustFocus(Frame *self);
/* frame_adjust_title */
/*! Update the frame to display the client's current title */
typedef void EngineFrameAdjustTitle(Frame *self);
/* frame_adjust_icon */
/*! Update the frame to display the client's current icon */
typedef void EngineFrameAdjustIcon(Frame *self);

/* frame_show */
/*! Shows the frame */
typedef void EngineFrameShow(Frame *self);
/*! Hides the frame */
typedef void EngineFrameHide(Frame *self);

/* get_context */
typedef Context EngineGetContext(Client *client, Window win);

#endif
