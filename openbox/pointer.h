#ifndef __pointer_h
#define __pointer_h

#include "client.h"
#include <glib.h>

void pointer_startup();
void pointer_shutdown();

void pointer_grab_all(Client *client, gboolean grab);

void pointer_event(XEvent *e, Client *c);

#endif
