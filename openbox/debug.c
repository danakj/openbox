#include <glib.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

static gboolean show;

void ob_debug_show_output(gboolean enable)
{
    show = enable;
}

void ob_debug(char *a, ...)
{
    va_list vl;

    if (show) {
        va_start(vl, a);
        vfprintf(stderr, a, vl);
    }
}
