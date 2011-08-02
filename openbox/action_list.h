/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action_list.h for the Openbox window manager
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

struct _ObActionAct;
struct _ObActionFilter;
struct _ObActionValue;

typedef struct _ObActionList      ObActionList;
typedef struct _ObActionListTest  ObActionListTest;

/*! Each node of the Action list is an action itself (or a filter bound to
  an action). */
struct _ObActionList {
    gint ref;
    gboolean isfilterset;
    union {
        struct _ObActionListFilterSet {
            ObActionListTest *test; /* can be null */
            ObActionList *thendo; /* can be null */
            ObActionList *elsedo; /* can be null */
        } f;
        struct _ObAction *action;
    } u;
    ObActionList *next;
};

struct _ObActionListTest {
    struct _ObActionFilter *filter;
    gboolean and;
    ObActionListTest *next;
};

void action_list_ref(ObActionList *l);
void action_list_unref(ObActionList *l);

void action_list_test_destroy(ObActionListTest *t);

ObActionList* action_list_concat(ObActionList *a, ObActionList *b);
