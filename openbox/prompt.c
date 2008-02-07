/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   prompt.c for the Openbox window manager
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

#include "prompt.h"
#include "openbox.h"
#include "screen.h"
#include "obt/display.h"
#include "gettext.h"

static GList *prompt_list = NULL;

/* we construct these */
static RrAppearance *prompt_a_button;
static RrAppearance *prompt_a_hover;
static RrAppearance *prompt_a_press;
/* we change the max width which would screw with others */
static RrAppearance *prompt_a_msg;

void prompt_startup(gboolean reconfig)
{
    RrColor *c_button, *c_hover, *c_press;

    prompt_a_button = RrAppearanceCopy(ob_rr_theme->a_focused_unpressed_close);
    prompt_a_hover = RrAppearanceCopy(ob_rr_theme->a_hover_focused_close);
    prompt_a_press = RrAppearanceCopy(ob_rr_theme->a_focused_pressed_close);

    c_button = prompt_a_button->texture[0].data.mask.color;
    c_hover = prompt_a_button->texture[0].data.mask.color;
    c_press = prompt_a_button->texture[0].data.mask.color;

    RrAppearanceRemoveTextures(prompt_a_button);
    RrAppearanceRemoveTextures(prompt_a_hover);
    RrAppearanceRemoveTextures(prompt_a_press);

    RrAppearanceAddTextures(prompt_a_button, 1);
    RrAppearanceAddTextures(prompt_a_hover, 1);
    RrAppearanceAddTextures(prompt_a_press, 1);

    /* totally cheating here.. */
    prompt_a_button->texture[0] = ob_rr_theme->osd_hilite_label->texture[0];
    prompt_a_hover->texture[0] = ob_rr_theme->osd_hilite_label->texture[0];
    prompt_a_press->texture[0] = ob_rr_theme->osd_hilite_label->texture[0];

    prompt_a_button->texture[0].data.text.color = c_button;
    prompt_a_hover->texture[0].data.text.color = c_hover;
    prompt_a_press->texture[0].data.text.color = c_press;

    prompt_a_msg = RrAppearanceCopy(ob_rr_theme->osd_hilite_label);
    prompt_a_msg->texture[0].data.text.flow = TRUE;
}

void prompt_shutdown(gboolean reconfig)
{
    RrAppearanceFree(prompt_a_button);
    RrAppearanceFree(prompt_a_hover);
    RrAppearanceFree(prompt_a_press);
    RrAppearanceFree(prompt_a_msg);
}

ObPrompt* prompt_new(const gchar *msg, const gchar *const *answers)
{
    ObPrompt *self;
    XSetWindowAttributes attrib;
    guint i;
    const gchar *const *c;

    attrib.override_redirect = TRUE;
    attrib.border_pixel = RrColorPixel(ob_rr_theme->osd_border_color);

    self = g_new0(ObPrompt, 1);
    self->ref = 1;
    self->super.type = OB_WINDOW_CLASS_PROMPT;
    self->super.window = XCreateWindow(obt_display, obt_root(ob_screen),
                                       0, 0, 1, 1, ob_rr_theme->obwidth,
                                       CopyFromParent, InputOutput,
                                       CopyFromParent,
                                       CWOverrideRedirect | CWBorderPixel,
                                       &attrib);
    window_add(&self->super.window, PROMPT_AS_WINDOW(self));

    self->a_bg = RrAppearanceCopy(ob_rr_theme->osd_hilite_bg);

    self->msg.text = g_strdup(msg);
    self->msg.window = XCreateWindow(obt_display, self->super.window,
                                     0, 0, 1, 1, 0,
                                     CopyFromParent, InputOutput,
                                     CopyFromParent, 0, NULL);
    XMapWindow(obt_display, self->msg.window);

    self->n_buttons = 0;
    for (c = answers; *c != NULL; ++c)
        ++self->n_buttons;

    if (!self->n_buttons)
        self->n_buttons = 1;

    self->button = g_new(ObPromptElement, self->n_buttons);

    if (!answers) {
        g_assert(self->n_buttons == 1); /* should be set to this above.. */
        self->button[0].text = g_strdup(_("OK"));
    }
    else {
        g_assert(self->n_buttons > 0);
        for (i = 0; i < self->n_buttons; ++i)
            self->button[i].text = g_strdup(answers[i]);
    }

    for (i = 0; i < self->n_buttons; ++i) {
        self->button[i].window = XCreateWindow(obt_display, self->super.window,
                                               0, 0, 1, 1, 0,
                                               CopyFromParent, InputOutput,
                                               CopyFromParent, 0, NULL);
        XMapWindow(obt_display, self->button[i].window);
        window_add(&self->button[i].window, PROMPT_AS_WINDOW(self));
    }

    return self;
}

void prompt_ref(ObPrompt *self)
{
    ++self->ref;
}

void prompt_unref(ObPrompt *self)
{
    if (self && --self->ref == 0) {
        guint i;

        for (i = 0; i < self->n_buttons; ++i) {
            window_remove(self->button[i].window);
            XDestroyWindow(obt_display, self->button[i].window);
        }

        XDestroyWindow(obt_display, self->msg.window);

        RrAppearanceFree(self->a_bg);

        window_remove(self->super.window);
        XDestroyWindow(obt_display, self->super.window);
        g_free(self);
    }
}

static void prompt_layout(ObPrompt *self, const Rect *area)
{
    gint l, r, t, b;
    guint i;
    gint allbuttonsw, allbuttonsh, buttonx;
    gint w, h;

    const gint OUTSIDE_MARGIN = 4;
    const gint MSG_BUTTON_SEPARATION = 4;
    const gint BUTTON_SEPARATION = 4;

    RrMargins(self->a_bg, &l, &t, &r, &b);
    l += OUTSIDE_MARGIN;
    t += OUTSIDE_MARGIN;
    r += OUTSIDE_MARGIN;
    b += OUTSIDE_MARGIN;

    /* find the button sizes and how much space we need for them */
    allbuttonsw = allbuttonsh = 0;
    for (i = 0; i < self->n_buttons; ++i) {
        gint bw, bh;

        prompt_a_button->texture[0].data.text.string = self->button[i].text;
        prompt_a_hover->texture[0].data.text.string = self->button[i].text;
        prompt_a_press->texture[0].data.text.string = self->button[i].text;
        RrMinSize(prompt_a_button, &bw, &bh);
        self->button[i].width = bw;
        self->button[i].height = bh;
        RrMinSize(prompt_a_hover, &bw, &bh);
        self->button[i].width = MAX(self->button[i].width, bw);
        self->button[i].height = MAX(self->button[i].height, bh);
        RrMinSize(prompt_a_press, &bw, &bh);
        self->button[i].width = MAX(self->button[i].width, bw);
        self->button[i].height = MAX(self->button[i].height, bh);

        allbuttonsw += self->button[i].width + (i > 0 ? BUTTON_SEPARATION : 0);
        allbuttonsh = MAX(allbuttonsh, self->button[i].height);
    }

    self->msg_wbound = MAX(allbuttonsw, area->width*3/5);

    /* measure the text message area */
    prompt_a_msg->texture[0].data.text.string = self->msg.text;
    prompt_a_msg->texture[0].data.text.maxwidth = self->msg_wbound;
    RrMinSize(prompt_a_msg, &self->msg.width, &self->msg.height);

    g_print("height %d\n", self->msg.height);

    /* width and height inside the outer margins */
    w = MAX(self->msg.width, allbuttonsw);
    h = self->msg.height + MSG_BUTTON_SEPARATION + allbuttonsh;

    /* position the text message */
    self->msg.x = l + (w - self->msg.width) / 2;
    self->msg.y = t;

    /* position the button buttons */
    buttonx = l + (w - allbuttonsw) / 2;
    for (i = 0; i < self->n_buttons; ++i) {
        self->button[i].x = buttonx;
        buttonx += self->button[i].width + BUTTON_SEPARATION;
        self->button[i].y = t + h - allbuttonsh;
        self->button[i].y += (allbuttonsh - self->button[i].height) / 2;
    }

    /* size and position the toplevel window */
    self->width = w + l + r;
    self->height = h + t + b;
    self->x = (area->width - self->width) / 2;
    self->y = (area->height - self->height) / 2;

    /* move and resize the actual windows */
    XMoveResizeWindow(obt_display, self->super.window,
                      self->x, self->y, self->width, self->height);
    XMoveResizeWindow(obt_display, self->msg.window,
                      self->msg.x, self->msg.y,
                      self->msg.width, self->msg.height);
    for (i = 0; i < self->n_buttons; ++i)
        XMoveResizeWindow(obt_display, self->button[i].window,
                          self->button[i].x, self->button[i].y,
                          self->button[i].width, self->button[i].height);
}

static void render_button(ObPrompt *self, ObPromptElement *e)
{
    prompt_a_button->surface.parent = self->a_bg;
    prompt_a_button->surface.parentx = e->x;
    prompt_a_button->surface.parentx = e->y;

    prompt_a_button->texture[0].data.text.string = e->text;
    RrPaint(prompt_a_button, e->window, e->width, e->height);
}

static void render_all(ObPrompt *self)
{
    guint i;

    RrPaint(self->a_bg, self->super.window, self->width, self->height);

    prompt_a_msg->surface.parent = self->a_bg;
    prompt_a_msg->surface.parentx = self->msg.x;
    prompt_a_msg->surface.parentx = self->msg.y;

    prompt_a_msg->texture[0].data.text.string = self->msg.text;
    prompt_a_msg->texture[0].data.text.maxwidth = self->msg_wbound;
    RrPaint(prompt_a_msg, self->msg.window, self->msg.width, self->msg.height);

    for (i = 0; i < self->n_buttons; ++i)
        render_button(self, &self->button[i]);
}

void prompt_show(ObPrompt *self, const Rect *area)
{
    if (self->mapped) return;

    prompt_layout(self, area);
    render_all(self);
    XMapWindow(obt_display, self->super.window);

    self->mapped = TRUE;
}

void prompt_hide(ObPrompt *self)
{
    XUnmapWindow(obt_display, self->super.window);
    self->mapped = FALSE;
}
