#ifndef __mbind_h
#define __mbind_h

#include "obexport.h"
#include "client.h"
#include <glib.h>

void mbind_startup();
void mbind_shutdown();

/*! Adds a new pointer binding */
gboolean mbind_add(char *name, GQuark context);
void mbind_clearall();

void mbind_fire(guint state, guint button, GQuark context, EventType type,
		Client *client, int xroot, int yroot);

void mbind_grab_all(Client *client, gboolean grab);
gboolean mbind_grab_pointer(gboolean grab);

#endif
