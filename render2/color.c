#include "render.h"
#include "instance.h"
#include "debug.h"
#include <X11/Xlib.h>

int RrColorParse(struct RrInstance *inst, const char *colorname,
                 struct RrColor *ret)
{
    XColor xcol;

    if (!XParseColor(RrDisplay(inst), RrColormap(inst), colorname, &xcol)) {
        RrDebug("unable to parse color '%s'", colorname);
        ret->r = 0.0;
        ret->g = 0.0;
        ret->b = 0.0;
        return 0;
    }
    ret->r = (xcol.red >> 8) / 255.0;
    ret->g = (xcol.green >> 8) / 255.0;
    ret->b = (xcol.blue >> 8) / 255.0;
    return 1;
}
