#include "composite.h"
#include "openbox.h"
#include "extensions.h"

#ifndef USE_XCOMPOSITE
void composite_startup(gboolean reconfig) {}
void composite_shutdown(gboolean reconfig) {}
gboolean composite_window_has_alpha(Visual *vis) { return FALSE; }
XID composite_get_window_picture(Window win, Visual *vis) { return None; }
Pixmap composite_get_window_pixmap(Window win) { return None; }
void composite_setup_root_window() {}
void composite_enable_for_window(Window win) {}
#else

static Picture root_picture = None;

void composite_startup(gboolean reconfig)
{
    if (reconfig) return;
    if (!extensions_comp) return;
}

void composite_shutdown(gboolean reconfig)
{
    if (reconfig) return;
    if (!extensions_comp) return;
}

void composite_setup_root_window()
{
    if (root_picture)
        XRenderFreePicture(ob_display, root_picture);

    root_picture =
        composite_get_window_picture(RootWindow(ob_display, ob_screen),
                                     RrVisual(ob_rr_inst));
}

gboolean composite_window_has_alpha(Visual *vis)
{
    XRenderPictFormat *format;

    if (!extensions_comp) return FALSE;

    format = XRenderFindVisualFormat(ob_display, vis);
    return format->type == PictTypeDirect && format->direct.alphaMask;
}

XID composite_get_window_picture(Window win, Visual *vis)
{
    XRenderPictureAttributes pa;
    XRenderPictFormat *format;

    if (!extensions_comp) return None;

    format = XRenderFindVisualFormat(ob_display, vis);

    pa.subwindow_mode = IncludeInferiors;
    return XRenderCreatePicture(ob_display, win, format, CPSubwindowMode, &pa);
}

Pixmap composite_get_window_pixmap(Window win)
{
    if (!extensions_comp) return None;

    return XCompositeNameWindowPixmap(ob_display, win);
}

void composite_enable_for_window(Window win)
{
    /* Redirect window contents to offscreen pixmaps */
    XCompositeRedirectWindow(ob_display, win, CompositeRedirectAutomatic);
}

#endif
