#ifndef __dispatch_h
#define __dispatch_h

#include "client.h"
#include <X11/Xlib.h>

void dispatch_startup();
void dispatch_shutdown();

typedef enum {
    Event_X_EnterNotify   = 1 << 0, /* pointer entered a window */
    Event_X_LeaveNotify   = 1 << 1, /* pointer left a window */
    Event_X_KeyPress      = 1 << 2, /* key pressed */
    Event_X_KeyRelease    = 1 << 3, /* key released */
    Event_X_ButtonPress   = 1 << 4, /* mouse button pressed */
    Event_X_ButtonRelease = 1 << 5, /* mouse button released */
    Event_X_MotionNotify  = 1 << 6, /* mouse motion */
    Event_X_Bell          = 1 << 7, /* an XKB bell event */

    Event_Client_New      = 1 << 8, /* new window, before mapping */
    Event_Client_Mapped   = 1 << 9, /* new window, after mapping
                                       or uniconified */
    Event_Client_Destroy  = 1 << 10, /* unmanaged */
    Event_Client_Unmapped = 1 << 11, /* unmanaged, after unmapping
                                        or iconified */
    Event_Client_Focus    = 1 << 12, /* focused */
    Event_Client_Unfocus  = 1 << 13, /* unfocused */
    Event_Client_Urgent   = 1 << 14, /* entered/left urgent state */
    Event_Client_Desktop  = 1 << 15, /* moved to a new desktop */
    Event_Client_Moving   = 1 << 16, /* being interactively moved */

    Event_Ob_Desktop      = 1 << 17, /* changed desktops */
    Event_Ob_NumDesktops  = 1 << 18, /* changed the number of desktops */
    Event_Ob_ShowDesktop  = 1 << 19, /* entered/left show-the-desktop mode */

    Event_Signal          = 1 << 20, /* a signal from the OS */

    EVENT_RANGE           = 1 << 21
} EventType;

typedef struct {
    XEvent *e;
    Client *client;
} EventData_X;

typedef struct {
    Client *client;
    int num[2];
    /* Event_Client_Desktop: num[0] = new number, num[1] = old number
       Event_Client_Urgent: num[0] = urgent state
       Event_Client_Moving: num[0] = dest x coord, num[1] = dest y coord --
                            change these in the handler to adjust where the
                            window will be placed
     */
} EventData_Client;

typedef struct {
    int num[2];
    /* Event_Ob_Desktop: num[0] = new number, num[1] = old number
       Event_Ob_NumDesktops: num[0] = new number, num[1] = old number
       Event_Ob_ShowDesktop: num[0] = new show-desktop mode
     */
} EventData_Ob;

typedef struct {
    int signal;
} EventData_Signal;

typedef struct {
    EventData_X x;      /* for Event_X_* event types */
    EventData_Client c; /* for Event_Client_* event types */
    EventData_Ob o;     /* for Event_Ob_* event types */
    EventData_Signal s; /* for Event_Signal */
} EventData;

typedef struct {
    EventType type;
    EventData data;
} ObEvent;

typedef void (*EventHandler)(const ObEvent *e, void *data);

typedef unsigned int EventMask;

void dispatch_register(EventMask mask, EventHandler h, void *data);

void dispatch_x(XEvent *e, Client *c);
void dispatch_client(EventType e, Client *c, int num0, int num1);
void dispatch_ob(EventType e, int num0, int num1);
void dispatch_signal(int signal);
/* *x and *y should be set with the destination of the window, they may be
   changed by the event handlers */
void dispatch_move(Client *c, int *x, int *y);

#endif
