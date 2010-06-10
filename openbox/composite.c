/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   composite.c for the Openbox window manager
   Copyright (c) 2010        Dana Jansens
   Copyright (c) 2010        Derek Foreman

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

#include "composite.h"
#include "config.h"
#include "obt/display.h"
#include "openbox.h"
#include "screen.h"
#include "event.h"
#include "client.h"
#include "frame.h"
#include "geom.h"
#include "gettext.h"

#include <X11/Xlib.h>
#include <glib.h>

#ifdef USE_COMPOSITING

static gboolean composite(gpointer data);

static struct ObCompositor obcomp;

static inline void
time_fix(struct timeval *tv)
{
    while (tv->tv_usec >= 1000000) {
        tv->tv_usec -= 1000000;
        ++tv->tv_sec;
    }
    while (tv->tv_usec < 0) {
        tv->tv_usec += 1000000;
        --tv->tv_sec;
    }
}

static gboolean composite_need_redraw(void)
{
    return TRUE;
}

static void get_best_fbcon(GLXFBConfig *in, int count, int depth,
                           struct ObCompositeFBConfig *out)
{
    GLXFBConfig best = 0;
    XVisualInfo *vi;
    int i, value, alpha, stencil, depthb;
    gboolean rgba, db;

    rgba = FALSE;
    db = TRUE;
    stencil = G_MAXSHORT;
    depthb = G_MAXSHORT;

    for (i = 0; i < count; i++) {
        vi = glXGetVisualFromFBConfig(obt_display, in[i]);
        if (vi == NULL)
            continue;

        value = vi->depth;
        XFree(vi);

        if (value != depth)
            continue;

        obcomp.GetFBConfigAttrib(obt_display, in[i], GLX_ALPHA_SIZE, &alpha);
        obcomp.GetFBConfigAttrib(obt_display, in[i], GLX_BUFFER_SIZE, &value);

        /* the buffer size should equal the depth or else the buffer size minus
           the alpha size should */
        if (value != depth && value - alpha != depth) continue;

        value = 0;
        if (depth == 32) {
            obcomp.GetFBConfigAttrib(obt_display, in[i],
                                     GLX_BIND_TO_TEXTURE_RGBA_EXT, &value);
            rgba = TRUE;
        }
        if (!value) {
            if (rgba) continue; /* a different one has rgba, prefer that */

            obcomp.GetFBConfigAttrib(obt_display, in[i],
                                     GLX_BIND_TO_TEXTURE_RGB_EXT, &value);
        }
        if (!value) // neither bind to texture?  no dice
            continue;

        /* get no doublebuffer if possible */
        obcomp.GetFBConfigAttrib(obt_display, in[i], GLX_DOUBLEBUFFER, &value);
        if (value && !db) continue;
        db = value;

        /* get the smallest stencil buffer */
        obcomp.GetFBConfigAttrib(obt_display, in[i], GLX_STENCIL_SIZE, &value);
        if (value > stencil) continue;
        stencil = value;

        /* get the smallest depth buffer */
        obcomp.GetFBConfigAttrib(obt_display, in[i], GLX_DEPTH_SIZE, &value);
        if (value > depthb) continue;
        depthb = value;

        best = in[i];
    }
    out->fbc = best;
    out->tf = rgba ? GLX_TEXTURE_FORMAT_RGBA_EXT : GLX_TEXTURE_FORMAT_RGB_EXT;
}
#endif

void composite_startup(gboolean reconfig)
{
    /* This function will try enable composite if config_comp is TRUE.  At the
       end of this process, config_comp will be set to TRUE only if composite
       is enabled, and FALSE otherwise. */
#ifdef USE_COMPOSITING
    int count, val;
    Time timestamp;
    XWindowAttributes xa;
    XserverRegion xr;
    XVisualInfo tmp;
    XVisualInfo *vi;
    GLXFBConfig *fbcs;
    const char *glstring;
    gchar *astr;
    Atom cm_atom;
    Window cm_owner;
    int i;

    if (reconfig) return;
    if (!config_comp) return;

    if (ob_comp_indirect)
        setenv("LIBGL_ALWAYS_INDIRECT", "1", TRUE);

    config_comp = FALSE;

    astr = g_strdup_printf("_NET_WM_CM_S%d", ob_screen);
    cm_atom = XInternAtom(obt_display, astr, FALSE);
    g_free(astr);

    cm_owner = XGetSelectionOwner(obt_display, cm_atom);
    if (cm_owner != None) {
        g_message(_("Failed to enable composite. There is already a compositor running."));
        return;
    }

    timestamp = event_time();
    XSetSelectionOwner(obt_display, cm_atom, screen_support_win, timestamp);

    if (XGetSelectionOwner(obt_display, cm_atom) != screen_support_win) {
        g_message(_("Failed to enable composite. Could not acquire the composite manager selection"));
        return;
    }

    if (!obt_display_extension_composite) {
        g_message(
            _("Failed to enable composite. The %s extension is missing."),
            "XComposite");
        return;
    }

    if (!obt_display_extension_damage) {
        g_message(
            _("Failed to enable composite. The %s extension is missing."),
            "XDamage");
        return;
    }

    if (!obt_display_extension_fixes) {
        g_message(
            _("Failed to enable composite. The %s extension is missing."),
            "XFixes");
        return;
    }

    glstring = glXQueryExtensionsString(obt_display, ob_screen);
    if (!strstr(glstring, "GLX_EXT_texture_from_pixmap")) {
        g_message(_("Failed to enable composite. %s is not present."),
                  "GLX_EXT_texture_from_pixmap");
        return;
    }

    obcomp.CreatePixmap = (CreatePixmapT)
        glXGetProcAddress((const unsigned char*)"glXCreatePixmap");
    if (!obcomp.CreatePixmap) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXCreatePixmap");
        return;
    }

    obcomp.BindTexImage = (BindTexImageT)
        glXGetProcAddress((const unsigned char*)"glXBindTexImageEXT");
    if (!obcomp.BindTexImage) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXBindTexImage");
        return;
    }

    obcomp.ReleaseTexImage = (ReleaseTexImageT)
        glXGetProcAddress((const unsigned char*)"glXReleaseTexImageEXT");
    if (!obcomp.ReleaseTexImage) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXReleaseTexImage");
        return;
    }

    obcomp.GetFBConfigs = (GetFBConfigsT)glXGetProcAddress(
        (const unsigned char*)"glXGetFBConfigs");
    if (!obcomp.GetFBConfigs) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXGetFBConfigs");
        return;
    }

    obcomp.GetFBConfigAttrib = (GetFBConfigAttribT)glXGetProcAddress(
        (const unsigned char*)"glXGetFBConfigAttrib");
    if (!obcomp.GetFBConfigAttrib) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXGetFBConfigAttrib");
        return;
    }

    obcomp.overlay = XCompositeGetOverlayWindow(obt_display,
                                                obt_root(ob_screen));
//now you've done it.  better release this if we fail later!
//or move this get to the end?

    xr = XFixesCreateRegion(obt_display, NULL, 0);
    XFixesSetWindowShapeRegion(obt_display, obcomp.overlay, ShapeBounding, 0, 0, 0);
    XFixesSetWindowShapeRegion(obt_display, obcomp.overlay, ShapeInput, 0, 0, xr);
    XFixesDestroyRegion(obt_display, xr);

    if (!XGetWindowAttributes(obt_display, obt_root(ob_screen), &xa)) {
        g_message(_("Failed to enable composite. %s failed."),
                  "XGetWindowAttributes");
        return;
    }

    tmp.visualid = XVisualIDFromVisual(xa.visual);
    vi = XGetVisualInfo(obt_display, VisualIDMask, &tmp, &count);

    if (!count) {
        g_message(
            _("Failed to enable composite. Failed to get visual info."));
        return;
    }


    glXGetConfig(obt_display, vi, GLX_USE_GL, &val);
    if (!val) {
        g_message(_("Failed to enable composite. Visual is not GL capable"));
        return;
    }

    glXGetConfig(obt_display, vi, GLX_DOUBLEBUFFER, &val);
    if (!val) {
        g_message(
            _("Failed to enable composite. Visual is not double buffered"));
        return;
    }

    obcomp.ctx = glXCreateContext(obt_display, vi, NULL, !ob_comp_indirect);
    XFree(vi);

    fbcs = obcomp.GetFBConfigs(obt_display, ob_screen, &count);

    if (!count) {
        g_message(_("Failed to enable composite. No valid FBConfigs."));
        return;
    }

    memset(&obcomp.PixmapConfig, 0, sizeof(obcomp.PixmapConfig));

    for (i = 1; i < MAX_DEPTH + 1; i++) {
        get_best_fbcon(fbcs, count, i, &obcomp.PixmapConfig[i]);
    }

    if (count)
        XFree(fbcs);

    printf("Best visual for 24bpp was 0x%lx\n",
           (gulong)obcomp.PixmapConfig[24].fbc);
    printf("Best visual for 32bpp was 0x%lx\n",
           (gulong)obcomp.PixmapConfig[32].fbc);

    g_idle_add(composite, NULL);

    glXMakeCurrent(obt_display, obcomp.overlay, obcomp.ctx);
    config_comp = TRUE;
    obcomp.screendims = screen_physical_area_all_monitors();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glXSwapBuffers(obt_display, obcomp.overlay);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(obcomp.screendims->x,
            obcomp.screendims->x + obcomp.screendims->width,
            obcomp.screendims->y + obcomp.screendims->height,
            obcomp.screendims->y,
            -100, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
}

void composite_shutdown(gboolean reconfig)
{
#ifdef USE_COMPOSITING
    if (reconfig) return;
#endif
}

static gboolean composite(gpointer data)
{
#ifdef USE_COMPOSITING
    int attribs[] = {
        GLX_TEXTURE_FORMAT_EXT,
        None,
        GLX_TEXTURE_TARGET_EXT,
        GLX_TEXTURE_2D_EXT,
        None
    };
    struct timeval start, end, dif;
    GList *it;
    ObWindow *win;
    ObClient *client;

//    if (!obcomp.need_redraw)
//        return;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    for (it = stacking_list; it; it = g_list_next(it)) {
    for (it = g_list_last(stacking_list); it; it = g_list_previous(it)) {
        gint d, x, y, w, h;

        win = it->data;
        if (win->type == OB_WINDOW_CLASS_PROMPT)
            continue;

        if (!win->mapped)
            continue;

        if (win->type == OB_WINDOW_CLASS_CLIENT) {
            client = WINDOW_AS_CLIENT(win);
            if (!client->frame->visible)
                continue;
        }

        d = window_depth(win);

        attribs[1] = obcomp.PixmapConfig[d].tf;

        if (win->pixmap == None)
            win->pixmap = XCompositeNameWindowPixmap(obt_display,
                                                     window_top(win));
        if (win->pixmap == None)
            continue;

        if (win->gpixmap == None)
            win->gpixmap = obcomp.CreatePixmap(obt_display,
                                               obcomp.PixmapConfig[d].fbc,
                                               win->pixmap, attribs);
        if (win->gpixmap == None)
            continue;

        glBindTexture(GL_TEXTURE_2D, win->texture);
gettimeofday(&start, NULL);
        obcomp.BindTexImage(obt_display, win->gpixmap,
                            GLX_FRONT_LEFT_EXT, NULL);
gettimeofday(&end, NULL);
dif.tv_sec = end.tv_sec - start.tv_sec;
dif.tv_usec = end.tv_usec - start.tv_usec;
time_fix(&dif);
//printf("took %f ms\n", dif.tv_sec * 1000.0 + dif.tv_usec / 1000.0);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        x = win->area.x;
        y = win->area.y;
        w = win->area.width + win->border * 2;
        h = win->area.height + win->border * 2;

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(x, y, 0.0);
        glTexCoord2f(0, 1);
        glVertex3f(x, y + h, 0.0);
        glTexCoord2f(1, 1);
        glVertex3f(x + w, y + h, 0.0);
        glTexCoord2f(1, 0);
        glVertex3f(x + w, y, 0.0);
        glEnd();
    }
    glXSwapBuffers(obt_display, obcomp.overlay);
    glFinish();
    GLenum gler;
    while ((gler = glGetError()) != GL_NO_ERROR) {
        printf("gl error %d\n", gler);
    }
    
    obcomp.need_redraw = 0;
#endif
    return TRUE;
}
