#ifndef ob__place_h
#define ob__place_h

#include <glib.h>

struct _ObClient;

void place_client(ObClient *client, gint *x, gint *y);

#endif
