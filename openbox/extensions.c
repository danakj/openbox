#include "openbox.h"
#include "extensions.h"

gboolean extensions_xkb       = FALSE;
int      extensions_xkb_event_basep;
gboolean extensions_shape     = FALSE;
int      extensions_shape_event_basep;
gboolean extensions_xinerama  = FALSE;
int      extensions_xinerama_event_basep;
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
