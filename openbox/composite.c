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
#include "geom.h"
#include "client.h"
#include "window.h"
#include "frame.h"
#include "geom.h"
#include "gettext.h"

#include <X11/Xlib.h>
#include <glib.h>

#ifdef USE_COMPOSITING
#include <GL/gl.h>
#include <GL/glx.h>
#endif

Window composite_overlay = None;

#ifndef USE_COMPOSITING
void   composite_startup (gboolean boiv)           {;(void)(biov);}
void   composite_shutdown(gboolean boiv)           {;(void)(biov);}
void   composite         (void)                    {}
void   composite_redir   (ObWindow *d, gboolean b) {;(void)(d);(void)(b);}
void   composite_resize  (void)                    {}
#else

#define MAX_DEPTH 32

typedef GLXPixmap    (*CreatePixmapT)     (Display *display,
                                           GLXFBConfig config,
                                           int attribute,
                                           int *value);
typedef void         (*BindTexImageT)     (Display *display,
                                           GLXDrawable drawable,
                                           int buffer,
                                           int *attriblist);
typedef void         (*ReleaseTexImageT)  (Display *display,
                                           GLXDrawable drawable,
                                           int buffer);
typedef GLXFBConfig* (*GetFBConfigsT)     (Display *display,
                                           int screen,
                                           int *nElements);
typedef int          (*GetFBConfigAttribT)(Display *display,
                                           GLXFBConfig config,
                                           int attribute,
                                           int *value);

typedef struct _ObCompositeFBConfig {
    GLXFBConfig fbc; /* the fbconfig */
    gint tf;         /* texture format */
} ObCompositeFBConfig;

static CreatePixmapT cglXCreatePixmap = NULL;
static BindTexImageT cglXBindTexImage = NULL;
static ReleaseTexImageT cglXReleaseTexImage = NULL;
static GetFBConfigsT cglXGetFBConfigs = NULL;
static GetFBConfigAttribT cglXGetFBConfigAttrib = NULL;

static GLXContext composite_ctx = NULL;
static ObCompositeFBConfig pixmap_config[MAX_DEPTH + 1]; /* depth is index */
static gboolean need_redraw = FALSE;

static gboolean composite(gpointer data);

static inline void time_fix(struct timeval *tv)
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

static void get_best_fbcon(GLXFBConfig *in, int count, int depth,
                           ObCompositeFBConfig *out)
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

        cglXGetFBConfigAttrib(obt_display, in[i], GLX_ALPHA_SIZE, &alpha);
        cglXGetFBConfigAttrib(obt_display, in[i], GLX_BUFFER_SIZE, &value);

        /* the buffer size should equal the depth or else the buffer size minus
           the alpha size should */
        if (value != depth && value - alpha != depth) continue;

        value = 0;
        if (depth == 32) {
            cglXGetFBConfigAttrib(obt_display, in[i],
                                  GLX_BIND_TO_TEXTURE_RGBA_EXT, &value);
            rgba = TRUE;
        }
        if (!value) {
            if (rgba) continue; /* a different one has rgba, prefer that */

            cglXGetFBConfigAttrib(obt_display, in[i],
                                  GLX_BIND_TO_TEXTURE_RGB_EXT, &value);
        }
        if (!value) // neither bind to texture?  no dice
            continue;

        /* get no doublebuffer if possible */
        cglXGetFBConfigAttrib(obt_display, in[i], GLX_DOUBLEBUFFER, &value);
        if (value && !db) continue;
        db = value;

        /* get the smallest stencil buffer */
        cglXGetFBConfigAttrib(obt_display, in[i], GLX_STENCIL_SIZE, &value);
        if (value > stencil) continue;
        stencil = value;

        /* get the smallest depth buffer */
        cglXGetFBConfigAttrib(obt_display, in[i], GLX_DEPTH_SIZE, &value);
        if (value > depthb) continue;
        depthb = value;

        best = in[i];
    }
    out->fbc = best;
    out->tf = rgba ? GLX_TEXTURE_FORMAT_RGBA_EXT : GLX_TEXTURE_FORMAT_RGB_EXT;
}

static gboolean composite_annex(void)
{
    gchar *astr;
    Atom cm_atom;
    Window cm_owner;

    astr = g_strdup_printf("_NET_WM_CM_S%d", ob_screen);
    cm_atom = XInternAtom(obt_display, astr, FALSE);
    g_free(astr);

    cm_owner = XGetSelectionOwner(obt_display, cm_atom);
    if (cm_owner != None) return FALSE;

    XSetSelectionOwner(obt_display, cm_atom, screen_support_win, event_time());

    if (XGetSelectionOwner(obt_display, cm_atom) != screen_support_win)
        return FALSE;

    return TRUE;
}

/*! This function will try enable composite if config_comp is TRUE.  At the
  end of this process, config_comp will be set to TRUE only if composite
  is enabled, and FALSE otherwise. */
void composite_startup(gboolean reconfig)
{
    int count, val, i;
    XWindowAttributes xa;
    XserverRegion xr;
    XVisualInfo tmp, *vi;
    GLXFBConfig *fbcs;
    const char *glstring;

    if (reconfig) return;
    if (!config_comp) return;

    if (ob_comp_indirect)
        setenv("LIBGL_ALWAYS_INDIRECT", "1", TRUE);

    config_comp = FALSE;

    /* Check for the required extensions in the server */
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

    /* Check for the required glX functions */

    cglXCreatePixmap = (CreatePixmapT)
        glXGetProcAddress((const unsigned char*)"glXCreatePixmap");
    if (!cglXCreatePixmap) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXCreatePixmap");
        return;
    }
    cglXBindTexImage = (BindTexImageT)
        glXGetProcAddress((const unsigned char*)"glXBindTexImageEXT");
    if (!cglXBindTexImage) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXBindTexImage");
        return;
    }
    cglXReleaseTexImage = (ReleaseTexImageT)
        glXGetProcAddress((const unsigned char*)"glXReleaseTexImageEXT");
    if (!cglXReleaseTexImage) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXReleaseTexImage");
        return;
    }
    cglXGetFBConfigs = (GetFBConfigsT)glXGetProcAddress(
        (const unsigned char*)"glXGetFBConfigs");
    if (!cglXGetFBConfigs) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXGetFBConfigs");
        return;
    }
    cglXGetFBConfigAttrib = (GetFBConfigAttribT)glXGetProcAddress(
        (const unsigned char*)"glXGetFBConfigAttrib");
    if (!cglXGetFBConfigAttrib) {
        g_message(_("Failed to enable composite. %s unavailable."),
                  "glXGetFBConfigAttrib");
        return;
    }

    /* Check for required GLX extensions */

    glstring = glXQueryExtensionsString(obt_display, ob_screen);
    if (!strstr(glstring, "GLX_EXT_texture_from_pixmap")) {
        g_message(_("Failed to enable composite. %s is not present."),
                  "GLX_EXT_texture_from_pixmap");
        return;
    }

    /* Check for FBconfigs */

    fbcs = cglXGetFBConfigs(obt_display, ob_screen, &count);
    if (!count) {
        g_message(_("Failed to enable composite. No valid FBConfigs."));
        return;
    }
    memset(&pixmap_config, 0, sizeof(pixmap_config));
    for (i = 1; i < MAX_DEPTH + 1; i++)
        get_best_fbcon(fbcs, count, i, &pixmap_config[i]);
    if (count) XFree(fbcs);

    /* Attempt to take over as composite manager.  There can only be one. */

    if (!composite_annex()) {
        g_message(_("Failed to enable composite. Another composite manager is running."));
        return;
    }

    /* Set up the overlay window */

    composite_overlay = XCompositeGetOverlayWindow(obt_display,
                                                   obt_root(ob_screen));
    if (!composite_overlay) {
        g_message(_("Failed to enable composite. Unable to get overlay window from X server"));
        return;
    }
    xr = XFixesCreateRegion(obt_display, NULL, 0);
    XFixesSetWindowShapeRegion(obt_display, composite_overlay, ShapeBounding,
                               0, 0, 0);
    XFixesSetWindowShapeRegion(obt_display, composite_overlay, ShapeInput,
                               0, 0, xr);
    XFixesDestroyRegion(obt_display, xr);

    /* From here on, if initializing composite fails, make sure you call
       composite_shutdown() ! */


    /* Make sure the root window's visual is acceptable for our GLX needs
       and create a GLX context with it */

    if (!XGetWindowAttributes(obt_display, obt_root(ob_screen), &xa)) {
        g_message(_("Failed to enable composite. %s failed."),
                  "XGetWindowAttributes");
        composite_shutdown(reconfig);
        return;
    }
    tmp.visualid = XVisualIDFromVisual(xa.visual);
    vi = XGetVisualInfo(obt_display, VisualIDMask, &tmp, &count);
    if (!count) {
        g_message(
            _("Failed to enable composite. Failed to get visual info."));
        composite_shutdown(reconfig);
        return;
    }
    glXGetConfig(obt_display, vi, GLX_USE_GL, &val);
    if (!val) {
        g_message(_("Failed to enable composite. Visual is not GL capable"));
        XFree(vi);
        composite_shutdown(reconfig);
        return;
    }
    glXGetConfig(obt_display, vi, GLX_DOUBLEBUFFER, &val);
    if (!val) {
        g_message(
            _("Failed to enable composite. Visual is not double buffered"));
        XFree(vi);
        composite_shutdown(reconfig);
        return;
    }
    composite_ctx = glXCreateContext(obt_display, vi, NULL, True);
    XFree(vi);

    printf("Best visual for 24bpp was 0x%lx\n",
           (gulong)pixmap_config[24].fbc);
    printf("Best visual for 32bpp was 0x%lx\n",
           (gulong)pixmap_config[32].fbc);

    /* We're good to go for composite ! */
    config_comp = TRUE;

    g_idle_add(composite, NULL);

    glXMakeCurrent(obt_display, composite_overlay, composite_ctx);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glXSwapBuffers(obt_display, composite_overlay);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    composite_resize();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void composite_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    obt_display_ignore_errors(TRUE);
    glXMakeCurrent(obt_display, None, NULL);
    obt_display_ignore_errors(FALSE);

    if (composite_ctx) {
        glXDestroyContext(obt_display, composite_ctx);
        composite_ctx = NULL;
    }

    if (composite_overlay) {
        XCompositeReleaseOverlayWindow(obt_display, composite_overlay);
        composite_overlay = None;
    }
}

void composite_resize(void)
{
    const Rect *a;
    a = screen_physical_area_all_monitors();
    glOrtho(a->x, a->x + a->width, a->y + a->height, a->y, -100, 100);
}

static gboolean composite(gpointer data)
{
    struct timeval start, end, dif;
    GList *it;
    ObWindow *win;
    ObClient *client;

    if (!config_comp) return FALSE;

/*    if (!need_redraw) return FALSE; */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/* XXX for (it = stacking_list_last; it; it = g_list_previous(it)) { */
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

        if (win->pixmap == None)
            win->pixmap = XCompositeNameWindowPixmap(obt_display,
                                                     window_top(win));
        if (win->pixmap == None)
            continue;

        if (win->gpixmap == None) {
            int attribs[] = {
                GLX_TEXTURE_FORMAT_EXT,
                pixmap_config[d].tf,
                GLX_TEXTURE_TARGET_EXT,
                GLX_TEXTURE_2D_EXT,
                None
            };
            win->gpixmap = cglXCreatePixmap(obt_display,
                                            pixmap_config[d].fbc,
                                            win->pixmap, attribs);
        }
        if (win->gpixmap == None)
            continue;

        glBindTexture(GL_TEXTURE_2D, win->texture);
gettimeofday(&start, NULL);
        cglXBindTexImage(obt_display, win->gpixmap, GLX_FRONT_LEFT_EXT, NULL);
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
        cglXReleaseTexImage(obt_display, win->gpixmap, GLX_FRONT_LEFT_EXT);
    }

    glXSwapBuffers(obt_display, composite_overlay);
    glFinish();

#ifdef DEBUG
    {
        GLenum gler;
        while ((gler = glGetError()) != GL_NO_ERROR) {
            printf("gl error %d\n", gler);
        }
    }
#endif

    need_redraw = 0;
    return TRUE;
}

void composite_redir(ObWindow *w, gboolean on)
{
    if (!config_comp) return;

    if (on) {
        if (w->redir) return;
        XCompositeRedirectWindow(obt_display, window_top(w),
                                 CompositeRedirectManual);
        w->redir = TRUE;
    }
    else {
        if (!w->redir) return;
        XCompositeUnredirectWindow(obt_display, window_top(w),
                                   CompositeRedirectManual);
        w->redir = FALSE;
    }
}
        
#endif

