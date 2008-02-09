/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   list.h for the Openbox compositor
   Copyright (c) 2008        Dana Jansens

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

#ifndef loco__list_h
#define loco__list_h

struct _LocoWindow;

/* To make a new LocoList, create 2 pointers: one for the head, one for the
   tail.  Initialize them both to NULL.  Then pass them to loco_list_prepend.
*/

typedef struct _LocoList {
    struct _LocoList   *next;
    struct _LocoList   *prev;
    struct _LocoWindow *window;
} LocoList;

LocoList* loco_list_prepend(LocoList **top, LocoList **bottom,
                            struct _LocoWindow *window);

void loco_list_delete_link(LocoList **top, LocoList **bottom,
                           LocoList *pos);

void loco_list_move_before(LocoList **top, LocoList **bottom,
                           LocoList *move, LocoList *before);

#endif
