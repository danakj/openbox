#include "openbox.h"
#include "geom.h"
#include "extensions.h"
#include "screen.h"

gboolean extensions_xkb       = FALSE;
int      extensions_xkb_event_basep;
gboolean extensions_shape     = FALSE;
int      extensions_shape_event_basep;
gboolean extensions_xinerama  = FALSE;
int      extensions_xinerama_event_basep;
gboolean extensions_xinerama_active = FALSE;
gboolean extensions_randr     = FALSE;
int      extensions_randr_event_basep;
gboolean extensions_vidmode   = FALSE;
int      extensions_vidmode_event_basep;

void extensions_query_all()
{
    int junk;
    (void)junk;
     
#ifdef XKB
    extensions_xkb =
	XkbQueryExtension(ob_display, &junk, &extensions_xkb_event_basep,
			  &junk, NULL, NULL);
#endif

#ifdef SHAPE
    extensions_shape =
	XShapeQueryExtension(ob_display, &extensions_shape_event_basep,
			     &junk);
#endif

#ifdef XINERAMA
    extensions_xinerama =
	XineramaQueryExtension(ob_display, &extensions_xinerama_event_basep,
			       &junk);
    extensions_xinerama_active = XineramaIsActive(ob_display);
#endif

#ifdef XRANDR
    extensions_randr =
	XRRQueryExtension(ob_display, &extensions_randr_event_basep,
                          &junk);
#endif

#ifdef VIDMODE
    extensions_vidmode =
	XF86VidModeQueryExtension(ob_display, &extensions_vidmode_event_basep,
                                  &junk);
#endif
}

void extensions_xinerama_screens(Rect **xin_areas, guint *nxin)
{
    guint i;
    gint l, r, t, b;
#ifdef XINERAMA
    if (extensions_xinerama_active) {
        guint i;
        gint n;
        XineramaScreenInfo *info = XineramaQueryScreens(ob_display, &n);
        *nxin = n;
        *xin_areas = g_new(Rect, *nxin + 1);
        for (i = 0; i < *nxin; ++i)
            RECT_SET((*xin_areas)[i], info[i].x_org, info[i].y_org,
                     info[i].width, info[i].height);
    } else
#endif
    {
        *nxin = 1;
        *xin_areas = g_new(Rect, *nxin + 1);
        RECT_SET((*xin_areas)[0], 0, 0,
                 screen_physical_size.width, screen_physical_size.height);
    }

    /* returns one extra with the total area in it */
    l = (*xin_areas)[0].x;
    t = (*xin_areas)[0].y;
    r = (*xin_areas)[0].x + (*xin_areas)[0].width - 1;
    b = (*xin_areas)[0].y + (*xin_areas)[0].height - 1;
    for (i = 1; i < *nxin; ++i) {
        l = MIN(l, (*xin_areas)[i].x);
        t = MIN(l, (*xin_areas)[i].y);
        r = MAX(r, (*xin_areas)[0].x + (*xin_areas)[0].width - 1);
        b = MAX(b, (*xin_areas)[0].y + (*xin_areas)[0].height - 1);
    }
    RECT_SET((*xin_areas)[*nxin], l, t, r - l + 1, b - t + 1);
}
