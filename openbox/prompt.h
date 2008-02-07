/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   prompt.h for the Openbox window manager
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

#ifndef ob__prompt_h
#define ob__prompt_h

typedef struct _ObPrompt       ObPrompt;
typedef struct _ObPromptElement ObPromptElement;

#include "window.h"
#include "geom.h"
#include "render/render.h"
#include <glib.h>

struct _ObPromptElement {
    gchar *text;
    Window window;

    gint x, y, width, height;
};

struct _ObPrompt
{
    ObInternalWindow super;
    gint ref;

    /* keep a copy of this because we re-render things that may need it
       (i.e. the buttons) */
    RrAppearance *a_bg;

    gboolean mapped;
    gint x, y, width, height;
    gint msg_wbound;

    ObPromptElement msg;

    /* one for each answer */
    ObPromptElement *button;
    guint n_buttons;
};

void prompt_startup(gboolean reconfig);
void prompt_shutdown(gboolean reconfig);

ObPrompt* prompt_new(const gchar *msg, const gchar *const *answers);
void prompt_ref(ObPrompt *self);
void prompt_unref(ObPrompt *self);

/*! Show the prompt.  It will be centered within the given area rectangle */
void prompt_show(ObPrompt *self, const Rect *area);
void prompt_hide(ObPrompt *self);

#endif
