/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions_list.h for the Openbox window manager
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

struct _ObActionsAct;
struct _ObActionsValue;

typedef struct _ObActionsList      ObActionsList;
typedef struct _ObActionsListTest  ObActionsListTest;

/*! Each node of the Actions list is an action itself (or a filter bound to
  an action). */
struct _ObActionsList {
    gint ref;
    gboolean isfilter;
    union {
        struct _ObActionsListFilter {
            ObActionsListTest *test; /* can be null */
            ObActionsList *thendo; /* can be null */
            ObActionsList *elsedo; /* can be null */
        } f;
        struct _ObActionsAct *action;
    } u;
    ObActionsList *next;
};

struct _ObActionsListTest {
    gchar *key;
    struct _ObActionsValue *value; /* can be null */
    gboolean and;
    ObActionsListTest *next;
};

void actions_list_ref(ObActionsList *l);
void actions_list_unref(ObActionsList *l);

void actions_list_test_destroy(ObActionsListTest *t);

ObActionsList* actions_list_concat(ObActionsList *a, ObActionsList *b);
