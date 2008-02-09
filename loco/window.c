/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   window.c for the Openbox window manager
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

#include "window.h"
#include "screen.h"
#include "list.h"
#include "obt/prop.h"

static void pixmap_create(LocoWindow *lw);
static void texture_create(LocoWindow *lw);
static void texture_destroy(LocoWindow *lw);
static void pixmap_destroy(LocoWindow *lw);
static Bool look_for_destroy(Display *d, XEvent *e, XPointer arg);

LocoWindow* loco_window_new(Window xwin, LocoScreen *screen)
{
    LocoWindow *lw;
    XWindowAttributes attrib;

    if (!XGetWindowAttributes(obt_display, xwin, &attrib))
        return NULL;

    lw = g_new0(LocoWindow, 1);
    lw->ref = 1;
    lw->id = xwin;
    lw->screen = screen;
    lw->input_only = attrib.class == InputOnly;
    lw->x = attrib.x;
    lw->y = attrib.y;
    lw->w = attrib.width;
    lw->h = attrib.height;
    lw->depth = attrib.depth;

    if (!lw->input_only) {
        glGenTextures(1, &lw->texname);
        /*glTexImage2D(TARGET, 0, GL_RGB, lw->w, lw->h,
                       0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);*/
        lw->damage = XDamageCreate(obt_display, lw->id, XDamageReportNonEmpty);
    }

    loco_screen_add_window(lw->screen, lw);

    if (attrib.map_state != IsUnmapped)
        loco_window_show(lw);

    return lw;
}

void loco_window_ref(LocoWindow *lw)
{
    ++lw->ref;
}

void loco_window_unref(LocoWindow *lw)
{
    if (lw && --lw->ref == 0) {
        if (!lw->input_only) {
            texture_destroy(lw);
            pixmap_destroy(lw);

            glDeleteTextures(1, &lw->texname);

            obt_display_ignore_errors(TRUE);
            XDamageDestroy(obt_display, lw->damage);
            obt_display_ignore_errors(FALSE);
        }

        loco_screen_remove_window(lw->screen, lw);

        g_free(lw);
    }
}

void loco_window_show(LocoWindow *lw) {
    guint32 *type;
    guint i, ntype;

    lw->visible = TRUE;

    /* get the window's semantic type (for different effects!) */
    lw->type = 0; /* XXX set this to the default type */
    if (OBT_PROP_GETA32(lw->id, NET_WM_WINDOW_TYPE, ATOM, &type, &ntype)) {
        /* use the first value that we know about in the array */
        for (i = 0; i < ntype; ++i) {
            /* XXX SET THESE TO AN ENUM'S VALUES */
            if (type[i] == OBT_PROP_ATOM(NET_WM_WINDOW_TYPE_DESKTOP))
                lw->type = 1;
            if (type[i] == OBT_PROP_ATOM(NET_WM_WINDOW_TYPE_MENU))
                lw->type = 2;
            /* XXX there are more TYPES that need to be added to prop.h */
        }
        g_free(type);
    }

    loco_screen_redraw(lw->screen);
}

void loco_window_hide(LocoWindow *lw, gboolean gone)
{
    /* if gone = TRUE, then the window is no longer available and we
       become a zombie */

    lw->visible = FALSE;

    /* leave the glx texture alone though.. */
    lw->stale = TRUE;
    pixmap_destroy(lw);

    loco_screen_redraw(lw->screen);

    if (gone) {
        loco_screen_zombie_window(lw->screen, lw);
        lw->id = None;
    }
}

void loco_window_configure(LocoWindow *lw, const XConfigureEvent *e)
{
    LocoList *above, *pos;

    pos = loco_screen_find_stacking(lw->screen, e->window);
    above = loco_screen_find_stacking(lw->screen, e->above);

    g_assert(pos != NULL && pos->window != NULL);

    if (e->above && !above)
        g_print("missing windows from the stacking list!!\n");

    if ((lw->x != e->x) || (lw->y != e->y)) {
        lw->x = e->x;
        lw->y = e->y;

        loco_screen_redraw(lw->screen);
    }
        
    if ((lw->w != e->width) || (lw->h != e->height)) {
        lw->w = e->width;
        lw->h = e->height;

        /* leave the glx texture alone though.. */
        lw->stale = TRUE;
        pixmap_destroy(lw);

        loco_screen_redraw(lw->screen);
    }

    if (pos->next != above) {
        //printf("Window 0x%lx above 0x%lx\n", pos->window->id,
        //       above ? above->window->id : 0);
        loco_list_move_before(&lw->screen->stacking_top,
                              &lw->screen->stacking_bottom,
                              pos, above);

        loco_screen_redraw(lw->screen);
    }
}

static Bool look_for_destroy(Display *d, XEvent *e, XPointer arg)
{
    const Window w = (Window)*arg;
    return e->type == DestroyNotify && e->xdestroywindow.window == w;
}

static void pixmap_create(LocoWindow *lw)
{
    XEvent ce;

    if (lw->pixmap) return;

    /* make sure the window exists */
    XGrabServer(obt_display);
    XSync(obt_display, FALSE);

    if (!XCheckIfEvent(obt_display, &ce, look_for_destroy, (XPointer)&lw->id))
        lw->pixmap = XCompositeNameWindowPixmap(obt_display, lw->id);
    XUngrabServer(obt_display);
}

static void texture_create(LocoWindow *lw)
{
    static const int attrs[] = {
        GLX_TEXTURE_FORMAT_EXT,
        GLX_TEXTURE_FORMAT_RGBA_EXT,
        None
    };

    if (lw->glpixmap) return;

    g_assert(lw->pixmap);

    if (!lw->screen->glxFBConfig[lw->depth]) {
        g_print("no glxFBConfig for depth %d for window 0x%lx\n",
                lw->depth, lw->id);
        return;
    }
    
    lw->glpixmap = glXCreatePixmap(obt_display,
                                   lw->screen->glxFBConfig[lw->depth],
                                   lw->pixmap, attrs);
    if (!lw->glpixmap) return;

#if 0
    if (screen->queryDrawable (screen->display->display,
			       texture->pixmap,
			       GLX_TEXTURE_TARGET_EXT,
			       &target))
    {
	fprintf (stderr, "%s: glXQueryDrawable failed\n", programName);

	glXDestroyGLXPixmap (screen->display->display, texture->pixmap);
	texture->pixmap = None;

	return FALSE;
    }
#endif

    glBindTexture(GL_TEXTURE_2D, lw->texname);

    lw->screen->bindTexImageEXT(obt_display, lw->glpixmap,
                                GLX_FRONT_LEFT_EXT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

static void texture_destroy(LocoWindow *lw)
{
    if (!lw->glpixmap) return;

    glBindTexture(GL_TEXTURE_2D, lw->texname);

    lw->screen->releaseTexImageEXT(obt_display, lw->glpixmap,
                                   GLX_FRONT_LEFT_EXT);

    glBindTexture(GL_TEXTURE_2D, 0);

    obt_display_ignore_errors(TRUE);
    glXDestroyGLXPixmap(obt_display, lw->glpixmap);
    obt_display_ignore_errors(FALSE);

    lw->glpixmap = None;
}

static void pixmap_destroy(LocoWindow *lw)
{
    if (!lw->pixmap) return;

    XFreePixmap(obt_display, lw->pixmap);
    lw->pixmap = None;
}

void loco_window_update_pixmap(LocoWindow *lw)
{
    if (loco_window_is_zombie(lw)) return;

    if (lw->stale || lw->glpixmap == None) {
        g_assert(lw->pixmap == None);

        texture_destroy(lw);
        pixmap_create(lw);
        texture_create(lw);
        lw->stale = FALSE;
    }
}

gboolean loco_window_is_zombie(LocoWindow *lw)
{
    return lw->id == None;
}
