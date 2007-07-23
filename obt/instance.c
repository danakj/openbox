#include "obt/obt.h"

#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

struct _ObtInstance
{
    gint ref;
    Display *d;
};

ObtInstance* obt_instance_new(const char *display_name)
{
    gchar *n;
    Display *d;
    ObtInstance *inst = NULL;

    n = display_name ? g_strdup(display_name) : NULL;
    d = XOpenDisplay(n);
    if (d) {
        if (fcntl(ConnectionNumber(d), F_SETFD, 1) == -1)
            g_message("Failed to set display as close-on-exec");

        inst = g_new(ObtInstance, 1);
        inst->ref = 1;
        inst->d = d;
    }
    g_free(n);

    return inst;
}

void obt_instance_ref(ObtInstance *inst)
{
    ++inst->ref;
}

void obt_instance_unref(ObtInstance *inst)
{
    if (inst && --inst->ref == 0) {
        XCloseDisplay(inst->d);
        obt_free0(inst, ObtInstance, 1);
    }
}

Display* obt_display(const ObtInstance *inst)
{
    return inst->d;
}
