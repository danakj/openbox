/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   loco.c for the Openbox window manager
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

#include <stdio.h>
#include "obt/mainloop.h"
#include "obt/display.h"
#include "obt/mainloop.h"
#include <glib.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glxtokens.h>
/*
<dana> you want CreateNotify, DestroyNotify, MapNotify, UnmapNotify, and
          ConfigureNotify
          <dana> and then xdamage or whatever

*/

void composite_setup_window(Window win)
{
    XCompositeRedirectWindow(obt_display, win, CompositeRedirectAutomatic);
    /*something useful = */XDamageCreate(obt_display, win, XDamageReportRawRectangles);
}

void COMPOSTER_RAWR(const XEvent *e, gpointer data)
{
    Window window;
    if (e->type == CreateNotify) {
        window = e->xmap.window;
        printf("Do first time stuff\n");
        composite_setup_window(window);
    }
    if (e->type == obt_display_extension_damage_basep + XDamageNotify) {
    }
    if (e->type == ConfigureNotify) {
        printf("Window moved or something\n");
    }
}

void loco_set_mainloop(gint ob_screen, ObtMainLoop *loop)
{
    int w, h;
    XVisualInfo *vi;
    GLXContext cont;
    int config[] =
        { GLX_DEPTH_SIZE, 1, GLX_DOUBLEBUFFER, GLX_RGBA, None };

    vi = glXChooseVisual(obt_display, DefaultScreen(obt_display), config);
    cont = glXCreateContext(obt_display, vi, NULL, GL_TRUE);
    if (cont == NULL)
        printf("context creation failed\n");
    glXMakeCurrent(obt_display, RootWindow(obt_display, ob_screen), cont);

    w = WidthOfScreen(ScreenOfDisplay(obt_display, ob_screen));
    h = HeightOfScreen(ScreenOfDisplay(obt_display, ob_screen));
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
printf("Setting up an orthographic projection of %dx%d\n", w, h);
    glOrtho(0, w, h, 0.0, -1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glXSwapBuffers(obt_display, RootWindow(obt_display, ob_screen));
    obt_main_loop_x_add(loop, COMPOSTER_RAWR, NULL, NULL);
}

void loco_shutdown(void)
{
}
