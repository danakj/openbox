#ifndef __dispatch_h
#define __dispatch_h

#include "client.h"
#include <X11/Xlib.h>

void dispatch_startup();
void dispatch_shutdown();

typedef enum {
    Event_X_EnterNotify   = 1 << 0,
    Event_X_LeaveNotify   = 1 << 1,
    Event_X_KeyPress      = 1 << 2,
    Event_X_KeyRelease    = 1 << 3,
    Event_X_ButtonPress   = 1 << 4,
    Event_X_ButtonRelease = 1 << 5,
    Event_X_MotionNotify  = 1 << 6,
    Event_X_Bell          = 1 << 7,

    Event_Client_New      = 1 << 8, /* new window, before mapping */
    Event_Client_Mapped   = 1 << 9, /* new window, after mapping */
    Event_Client_Destroy  = 1 << 10, /* unmanaged */
    Event_Client_Focus    = 1 << 11, /* focused */
    Event_Client_Unfocus  = 1 << 12, /* unfocused */
    Event_Client_Urgent   = 1 << 13, /* entered/left urgent state */
    Event_Client_Visible  = 1 << 14, /* shown/hidden (not on a workspace or
                                        show-the-desktop change though) */

    Event_Ob_Desktop      = 1 << 15, /* changed desktops */
    Event_Ob_NumDesktops  = 1 << 16, /* changed the number of desktops */
    Event_Ob_ShowDesktop  = 1 << 17, /* entered/left show-the-desktop mode */
    Event_Ob_Startup      = 1 << 18, /* startup complete */
    Event_Ob_Shutdown     = 1 << 19, /* shutdown about to start */

    Event_Signal          = 1 << 20,

    EVENT_RANGE           = 1 << 21
} EventType;

typedef struct {
    XEvent *e;
    Client *client;
} EventData_X;

typedef union {
    EventData_X x; /* for Event_X_* event types */
    Client *client; /* for Event_Client_* event types */
    int signal;
} EventData;

typedef struct {
    EventType type;
    EventData data;
} ObEvent;

typedef void (*EventHandler)(const ObEvent *e);

typedef unsigned int EventMask;

void dispatch_register(EventHandler h, EventMask mask);

void dispatch_x(XEvent *e, Client *c);
void dispatch_client(EventType e, Client *c);
void dispatch_ob(EventType e);
void dispatch_signal(int signal);

#endif
