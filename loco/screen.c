#include "screen.h"
#include "window.h"
#include "list.h"
#include "obt/display.h"

static void find_visuals(LocoScreen *sc);

static guint window_hash(Window *w) { return *w; }
static gboolean window_comp(Window *w1, Window *w2) { return *w1 == *w2; }

static const int drawable_tfp_attrs[] = {
    GLX_CONFIG_CAVEAT, GLX_NONE,
    GLX_DOUBLEBUFFER, FALSE,
    GLX_DEPTH_SIZE, 0,
    GLX_RED_SIZE, 1,
    GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1,
    GLX_ALPHA_SIZE, 1,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_BIND_TO_TEXTURE_RGBA_EXT, TRUE, /* For TextureFromPixmap */
    None
};

LocoScreen* loco_screen_new(gint number)
{
    LocoScreen *sc;

    /* try get the root redirect */
    obt_display_ignore_errors(TRUE);
    XCompositeRedirectSubwindows(obt_display, obt_root(number),
                                 CompositeRedirectManual);
    obt_display_ignore_errors(FALSE);
    if (obt_display_error_occured) {
        g_message("Another composite manager is running on screen %d", number);
        return NULL;
    }

    sc = g_new0(LocoScreen, 1);
    sc->ref = 1;
    sc->number = number;
    sc->root = obt_root(number);
    //sc->root = loco_window_new(obt_root(number));
    sc->stacking_map = g_hash_table_new((GHashFunc)window_hash,
                                        (GEqualFunc)window_comp);
    sc->stacking_map_ptr = g_hash_table_new((GHashFunc)g_direct_hash,
                                            (GEqualFunc)g_direct_equal);
    sc->bindTexImageEXT = (BindEXTFunc)
        glXGetProcAddress((const guchar*)"glXBindTexImageEXT");
    sc->releaseTexImageEXT = (ReleaseEXTFunc)
        glXGetProcAddress((const guchar*)"glXReleaseTexImageEXT");

    find_visuals(sc);

    sc->overlay = XCompositeGetOverlayWindow(obt_display, sc->root);

    if (sc->overlay) {
        XserverRegion region;

        region = XFixesCreateRegion(obt_display, NULL, 0);
        XFixesSetWindowShapeRegion(obt_display, sc->overlay,
                                   ShapeBounding, 0, 0, 0);
        XFixesSetWindowShapeRegion(obt_display, sc->overlay,
                                   ShapeInput, 0, 0, region);
        XFixesDestroyRegion(obt_display, region);
    }

    loco_screen_redraw(sc);

    return sc;
}

void loco_screen_ref(LocoScreen *sc)
{
    ++sc->ref;
}

void loco_screen_unref(LocoScreen *sc)
{
    if (sc && --sc->ref == 0) {
        /*XXX loco_window_unref(sc->root);*/

        if (sc->overlay)
            XCompositeReleaseOverlayWindow(obt_display, sc->overlay);

        g_hash_table_destroy(sc->stacking_map);
        g_hash_table_destroy(sc->stacking_map_ptr);

        g_free(sc);
    }
}

static void find_visuals(LocoScreen *sc)
{
    gint db, stencil, depth;
    gint i, j, value, count;
    XVisualInfo tvis, *visinfo;
    GLXFBConfig *fbcons;
    gint numfb;

    fbcons = glXChooseFBConfig(obt_display, sc->number,
                               drawable_tfp_attrs, &numfb);

    db = 32767;
    stencil = 32767;
    depth = 32767;
    for (i = 0; i <= LOCO_SCREEN_MAX_DEPTH; i++) {
        VisualID vid;

        vid = 0;
        sc->glxFBConfig[i] = 0;

        tvis.depth = i;
        visinfo = XGetVisualInfo(obt_display, VisualDepthMask, &tvis, &count);
        /* pick the nicest visual for the depth */
        for (j = 0; j < count; j++) {
            glXGetConfig(obt_display, &visinfo[j], GLX_USE_GL, &value);
            if (!value)
                continue;

            glXGetConfig(obt_display, &visinfo[j], GLX_DOUBLEBUFFER, &value);
            if (value > db)
                continue;
            db = value;

            glXGetConfig(obt_display, &visinfo[j], GLX_STENCIL_SIZE, &value);
            if (value > stencil)
                continue;
            stencil = value;

            glXGetConfig(obt_display, &visinfo[j], GLX_DEPTH_SIZE, &value);
            if (value > depth)
                continue;
            depth = value;

            /* use this visual */
            vid = visinfo[j].visualid;
        }

        if (!vid)
            continue;

        g_print("found visual %d\n", i);

        /* find the fbconfig for this depth/visual */
        for(j = 0; j < numfb; ++j) {
            glXGetFBConfigAttrib(obt_display, fbcons[j],
                                 GLX_VISUAL_ID, &value);
            if (value == (int)vid) {
                sc->glxFBConfig[i] = fbcons[j]; /* save it */
                g_print("supporting depth %d\n", i);
                break; /* next depth */
            }
        }
    }

    XFree(fbcons);
}

void loco_screen_add_window(LocoScreen *sc, LocoWindow *lw)
{
    LocoList *it;

    /*g_print("add window 0x%lx\n", lw->id);*/

    /* new windows are at the top */
    it = loco_list_prepend(&sc->stacking_top, &sc->stacking_bottom, lw);
    g_hash_table_insert(sc->stacking_map, &lw->id, it);
    g_hash_table_insert(sc->stacking_map_ptr, &lw, it);

    loco_window_ref(lw);
}

void loco_screen_zombie_window(LocoScreen *sc, LocoWindow *lw)
{
    /*g_print("zombie window 0x%lx\n", lw->id);*/

    /* the id will no longer be useful, so remove it from the hash */
    g_hash_table_remove(sc->stacking_map, &lw->id);
}

void loco_screen_remove_window(LocoScreen *sc, LocoWindow *lw)
{
    LocoList *pos;

    /*g_print("remove window 0x%lx\n", lw->id);*/

    pos = loco_screen_find_stacking_ptr(sc, lw);
    g_assert(pos);
    loco_list_delete_link(&sc->stacking_top, &sc->stacking_bottom, pos);
    g_hash_table_remove(sc->stacking_map, &lw->id);
    g_hash_table_remove(sc->stacking_map_ptr, &lw);

    loco_window_unref(lw);
}

struct _LocoWindow* loco_screen_find_window(LocoScreen *sc, Window xwin)
{
    LocoList *it;

    it = g_hash_table_lookup(sc->stacking_map, &xwin);
    return (it ? it->window : NULL);
}

struct _LocoList* loco_screen_find_stacking(LocoScreen *sc, Window xwin)
{
    return g_hash_table_lookup(sc->stacking_map, &xwin);
}

struct _LocoList* loco_screen_find_stacking_ptr(LocoScreen *sc, LocoWindow *lw)
{
    return g_hash_table_lookup(sc->stacking_map_ptr, &lw);
}

void loco_screen_redraw(LocoScreen *sc)
{
    sc->redraw = TRUE;
}

void loco_screen_redraw_done(LocoScreen *sc)
{
    sc->redraw = TRUE;
}
