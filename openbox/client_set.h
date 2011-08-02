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

typedef struct _ObClientSet ObClientSet;

typedef gboolean (*ObClientSetReduceFunc)(struct _ObClient *c, gpointer data);
typedef gboolean (*ObClientSetExpandFunc)(struct _ObClient *c, gpointer data);

/*! Returns a new set of clients without anything in it. */
ObClientSet* client_set_empty(void);
/*! Returns a new set of clients with a single client in it. */
ObClientSet* client_set_single(struct _ObClient *c);
/*! Returns a new set of clients with all possible clients in it. */
ObClientSet* client_set_all(void);

/*! Returns an identical set to @a. */
ObClientSet* client_set_clone(ObClientSet *a);

void client_set_destroy(ObClientSet *set);

/*! Returns a new set which contains all clients in either @a or @b.  The sets
  @a and @b are considered freed once passed to this function.
*/
ObClientSet* client_set_union(ObClientSet *a, ObClientSet *b);

/*! Returns a new set which contains all clients in both @a and @b.  The sets
  @a and @b are considered freed once passed to this function.
*/
ObClientSet* client_set_intersection(ObClientSet *a, ObClientSet *b);

/*! Returns a new set which contains all clients in @a but not in @b.  The sets
  @a and @b are considered freed once passed to this function.
*/
ObClientSet* client_set_minus(ObClientSet *a, ObClientSet *b);

/*! Reduce a set of clients.  The function @f is called for each client
  currently in the set. For each client that it returns TRUE, the client will
  be removed from the set. */
ObClientSet* client_set_reduce(ObClientSet *set, ObClientSetReduceFunc f,
                               gpointer data);

/*! Expand a set of clients.  The function @f is called for each client
  not currently in the set. For each client that it returns TRUE, the client
  will be added to the set. */
ObClientSet* client_set_expand(ObClientSet *set, ObClientSetExpandFunc f,
                               gpointer data);

/*! Returns TRUE if there is nothing in the set. */
gboolean client_set_is_empty(ObClientSet *set);

/*! Returns TRUE if there is someting in the set, or if it is the special
  "ALL" set, which contains all clients.  Even when there are no clients
  present, this set returns TRUE. */
gboolean client_set_test_boolean(ObClientSet *set);

/*! Returns TRUE if @set contains @c. */
gboolean client_set_contains(ObClientSet *set, struct _ObClient *c);
