#include "paint.h"
#include "screen.h"
#include "list.h"
#include "window.h"
#include "glerror.h"

#include "obt/display.h"

#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glxtokens.h>

static int context_visual_config[] = {
    GLX_DEPTH_SIZE, 1,
    GLX_DOUBLEBUFFER,
    GLX_RGBA,
    None
};

gboolean paint_setup(LocoScreen *sc)
{
    XVisualInfo *vi;
    GLXContext context;
    gint w, h;
    
    w = WidthOfScreen(ScreenOfDisplay(obt_display, sc->number));
    h = HeightOfScreen(ScreenOfDisplay(obt_display, sc->number));

    vi = glXChooseVisual(obt_display, sc->number, context_visual_config);
    context = glXCreateContext(obt_display, vi, NULL, GL_TRUE);
    if (context == NULL) {
        g_print("context creation failed\n");
        return FALSE;
    }
    glXMakeCurrent(obt_display, sc->overlay, context);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    g_print("Setting up an orthographic projection of %dx%d\n", w, h);
    glOrtho(0, w, h, 0.0, -1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
glError();
    glXSwapBuffers(obt_display, sc->overlay);
    glClearColor(0.4, 0.4, 0.4, 1.0);
/*
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
    glEnable(GL_BLEND);
*/
    return TRUE;
}

void paint_everything(LocoScreen *sc)
{
    int ret;
    LocoList *it;

    XGrabServer(obt_display);
    XSync(obt_display, FALSE);

    /* XXX if (full_redraw_required) */
        glClear(GL_COLOR_BUFFER_BIT);

    for (it = sc->stacking_bottom; it != NULL; it = it->prev) {
        if (it->window->id == sc->overlay) continue;
        if (it->window->input_only) continue;
        if (!it->window->visible)   continue;
        /* XXX if (!full_redraw_required && !it->window->damaged) continue; */

        /* XXX if (!full_redraw_required) {
           /\* XXX if the window is transparent, then clear the background
           behind it *\/
        }*/

        /* get the window's updated contents
           XXX if animating the window, then don't do this, depending
        */
        loco_window_update_pixmap(it->window);

        glBindTexture(GL_TEXTURE_2D, it->window->texname);
        if (ret) {
            glBegin(GL_QUADS);
            glColor3f(1.0, 1.0, 1.0);
            glVertex2i(it->window->x, it->window->y);
            glTexCoord2f(1, 0);
            glVertex2i(it->window->x + it->window->w, it->window->y);
            glTexCoord2f(1, 1);
            glVertex2i(it->window->x + it->window->w,
                       it->window->y + it->window->h);
            glTexCoord2f(0, 1);
            glVertex2i(it->window->x, it->window->y + it->window->h);
            glTexCoord2f(0, 0);
            glEnd();
        }

        it->window->damaged = FALSE;
    }
    glXSwapBuffers(obt_display, sc->overlay);

    XUngrabServer(obt_display);

    loco_screen_redraw_done(sc);
}
