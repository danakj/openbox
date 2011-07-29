/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   client_set.h for the Openbox window manager
   Copyright (c) 2011        Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include <glib.h>

struct _ObClient;

/*! A set of ObClients. An empty set is always a NULL pointer. */
typedef struct _ObClientSet ObClientSet;

typedef gboolean (*ObClientSetReduceFunc)(struct _ObClient *c);
typedef gboolean (*ObClientSetExpandFunc)(struct _ObClient *c);

/*! Returns a new set of clients with a single client in it.
  The client is the currently targeted window. */
ObClientSet* client_set_single(void);
/*! Returns a new set of clients with all possible client in it.*/
ObClientSet* client_set_all(void);

void client_set_destroy(ObClientSet *set);

/* Returns a new set which contains all clients in either @a or @b.  The sets
   @a and @b are considered freed once passed to this function.
*/
ObClientSet* client_set_union(ObClientSet *a, ObClientSet *b);

/* Returns a new set which contains all clients in both @a and @b.  The sets
   @a and @b are considered freed once passed to this function.
*/
ObClientSet* client_set_intersection(ObClientSet *a, ObClientSet *b);

/*! Reduce a set of clients.  The function @f is called for each client
  currently in the set. For each client that it returns TRUE, the client will
  be removed from the set. */
ObClientSet* client_set_reduce(ObClientSet *set, ObClientSetReduceFunc f);

/*! Expand a set of clients.  The function @f is called for each client
  not currently in the set. For each client that it returns TRUE, the client
  will be added to the set. */
ObClientSet* client_set_expand(ObClientSet *set, ObClientSetExpandFunc f);
