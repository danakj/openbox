#ifndef __eventdata_h
#define __eventdata_h

#include "obexport.h"
#include <Python.h>
#include <glib.h>

struct Client;

typedef struct {
    int temp:1; /* just a placeholder to kill warnings for now.. */
} LogicalEvent;

typedef struct {
    /*! The button which generated the event */
    guint button;
    /*! The pointer's x position on the root window when the event occured */
    int xroot;
    /*! The pointer's y position on the root window when the event occured */
    int yroot;
    /*! The modifiers that were pressed when the event occured. A bitmask of:
      ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask, 
      Mod4Mask, Mod5Mask */
    guint modifiers;
    /*! The name of the button/modifier combination being pressed,
      eg "Mod1-1" */
    char *name;
} PointerEvent;

typedef struct {
    /*! The keycode of the key which generated the event */
    guint keycode;
    /*! The modifiers that were pressed when the event occured. A bitmask of:
      ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask, 
      Mod4Mask, Mod5Mask */
    guint modifiers;
    /* The list of strings which make up the chain that fired,
       eg ("Mod1-a", "a") */
    GList *keylist;
} KeyEvent;

/* EventData is a PyObject */
typedef struct EventData {
    PyObject_HEAD
    /* The type of event which occured */
    EventType type;
    /*! The context in which the event occured, the type of window it occured
      for. */
    const char *context;
    /* The Client on which the event occured, or NULL */
    struct Client *client;

    union EventDetails {
	LogicalEvent *logical;
	PointerEvent *pointer;
	KeyEvent *key;
    } details;
} EventData;

void eventdata_startup();
void eventdata_shutdown();

EventData *eventdata_new_logical(EventType type, GQuark context,
				 struct Client *client);
EventData *eventdata_new_pointer(EventType type, GQuark context,
				 struct Client *client, guint modifiers,
				 guint button, char *name,
				 int xroot, int yroot);
EventData *eventdata_new_key(EventType type, GQuark context,
			     struct Client *client, guint modifiers,
			     guint keycode, GList *keylist);
void eventdata_free(EventData *data);

#endif
