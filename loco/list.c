/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   list.c for the Openbox window manager
   Copyright (c) 2008        Derek Foreman
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

#include "list.h"
#include "window.h"

LocoList* loco_list_prepend(LocoList **top, LocoList **bottom,
                            LocoWindow *window)
{
    LocoList *n = g_new(LocoList, 1);
    n->window = window;

    n->prev = NULL;
    n->next = *top;
    if (n->next) n->next->prev = n;

    *top = n;
    if (!*bottom) *bottom = n;
    return n;
}

void loco_list_delete_link(LocoList **top, LocoList **bottom,
                           LocoList *pos)
{
    LocoList *prev = pos->prev;
    LocoList *next = pos->next;

    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;
    if (!next)
        *bottom = prev;
    if (!prev)
        *top = next;

    g_free(pos);
}

void loco_list_move_before(LocoList **top, LocoList **bottom,
                           LocoList *move, LocoList *before)
{
    LocoList *prev, *next;

    /* these won't move it anywhere */
    if (move == before || move->next == before) return;

    prev = move->prev;
    next = move->next;

    /* remove it from the list */
    if (next) next->prev = prev;
    else      *bottom = prev;
    if (prev) prev->next = next;
    else      *top = next;

    /* reinsert it */
    if (before) {
        move->next = before;
        move->prev = before->prev;
        move->next->prev = move;
        if (move->prev) move->prev->next = move;
    }
    else {
        /* after the bottom */
        move->prev = *bottom;
        move->next = NULL;
        if (move->prev) move->prev->next = move;
        *bottom = move;
    }

    if (!move->prev) *top = move;
}

