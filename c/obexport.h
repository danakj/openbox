#ifndef __obexport_h
#define __obexport_h

#include <X11/Xlib.h>

/* Define values which will be exported in the 'ob' module. */

typedef enum {
    /*! Occurs when the mouse enters a window */
    Logical_EnterWindow,
    /*! Occurs when the mouse enters a window */
    Logical_LeaveWindow,
    /*! Occurs when a window is finished being managed, just before it is
      (possibly) displayed.
      The python scripts are reponsible for showing the window when this is
      called if they want it to be shown.
    */
    Logical_NewWindow,
    /*! Occurs when a window is being unmanaged */
    Logical_CloseWindow,
    /*! Occurs when the window manager starts up */
    Logical_Startup,
    /*! Occurs when the window manager is shutting down */
    Logical_Shutdown,
    /*! Occurs when a client is requesting/requested to be activated (i.e.
      focused, raised, unshaded) */
    Logical_RequestActivate,
    /*! Occurs when the input focus target changes
      The data.client will be NULL of no client is focused. */
    Logical_Focus,
    /*! Occurs when the system is fired through X.
      The data.client will hold the client associated with the bell if
      one has been specified, or NULL. */
    Logical_Bell,
    /*! Occurs when a client toggles its urgent status.
      The client.urgent member can be used to get the status. */
    Logical_UrgentWindow,
    /*! Occurs when a client becomes visible */
    Logical_WindowShow,
    /*! Occurs when a client becomes non-visible */
    Logical_WindowHide,
    /*! Occurs when a pointer button is pressed on a client or its
      decorations.
      Note: to get the event for the client's window or for the entire
      window+decorations, you need to do an mgrab for the window. */
    Pointer_Press,
    /*! Occurs when a pointer button is released on a client or its
      decorations.
      Note: to get the event for the client's window or for the entire
      window+decorations, you need to do an mgrab for the window. */
    Pointer_Release,
    /*! Occurs when a pointer button is held and the pointer is dragged on a
      client or its decorations.
      Note: to get the event for the client's window or for the entire
      window+decorations, you need to do an mgrab for the window, or an
      mgrab_pointer (in which case it may not be a drag). */
    Pointer_Motion,
    /*! Occurs when a key is pressed.
      Note: in order to recieve a key event, a kgrab must be done for the
      key combination, or a kgrab_keyboard.
    */
    Key_Press,
    /*! Occurs when a key is released.
      Note: in order to recieve a key event, a kgrab must be done for the
      key combination, or a kgrab_keyboard.
    */
    Key_Release
} EventType;

/* create the 'ob' module */
void obexport_startup();
void obexport_shutdown();

#endif
