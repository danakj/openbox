#include "cwmcc_internal.h"
#include "atom.h"
#include "prop.h"
#include "client_props.h"
#include "render/render.h"

#include <X11/Xutil.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

void cwmcc_client_get_protocols(Window win, Atom **protocols)
{
    gulong num;

    if (!prop_get_array32(win, CWMCC_ATOM(client, wm_protocols),
                          CWMCC_ATOM(type, atom), protocols, &num)) {
        g_warning("Failed to read WM_PROTOCOLS from 0x%lx", win);
        *protocols = NULL;
    }
}

void cwmcc_client_set_protocols(Window win, Atom *protocols)
{
    gulong n;
    Atom *a;

    for (a = protocols, n = 0; *a; ++a, ++n);
    XChangeProperty(cwmcc_display, win, CWMCC_ATOM(client, wm_state),
                    CWMCC_ATOM(type, atom), 32, PropModeReplace,
                    (guchar*)protocols, n);
}

void cwmcc_client_get_wm_state(Window win, gulong *state)
{
    if (!prop_get32(win, CWMCC_ATOM(client, wm_state),
                    CWMCC_ATOM(client, wm_state), state)) {
        g_warning("Failed to read WM_STATE from 0x%lx", win);
        *state = NormalState;
    }
}

void cwmcc_client_set_wm_state(Window win, gulong state)
{
    XChangeProperty(cwmcc_display, win, CWMCC_ATOM(client, wm_state),
                    CWMCC_ATOM(client, wm_state), 32, PropModeReplace,
                    (guchar*)&state, 1);
}

void cwmcc_client_get_name(Window win, char **name)
{
    if (!prop_get_string_utf8(win, CWMCC_ATOM(client, net_wm_name), name))
        if (!prop_get_string_locale(win, CWMCC_ATOM(client, wm_name), name)) {
            g_warning("Failed to read a name from 0x%lx", win);
            *name = g_strdup("Unnamed Window");
        }
}

void cwmcc_client_set_name(Window win, char *name)
{
    XChangeProperty(cwmcc_display, win, CWMCC_ATOM(client, net_wm_name),
                    CWMCC_ATOM(type, utf8), 32, PropModeReplace,
                    (guchar*)name, strlen(name));
}

void cwmcc_client_get_icon_name(Window win, char **name)
{
    if (!prop_get_string_utf8(win, CWMCC_ATOM(client, net_wm_icon_name), name))
        if (!prop_get_string_locale(win,
                                    CWMCC_ATOM(client, wm_icon_name), name)) {
            g_warning("Failed to read an icon name from 0x%lx", win);
            *name = g_strdup("Unnamed Window");
        }
}

void cwmcc_client_set_icon_name(Window win, char *name)
{
    XChangeProperty(cwmcc_display, win, CWMCC_ATOM(client, net_wm_icon_name),
                    CWMCC_ATOM(type, utf8), 32, PropModeReplace,
                    (guchar*)name, strlen(name));
}

void cwmcc_client_get_class(Window win, char **class, char **name)
{
    char **s;

    if (!prop_get_strings_locale(win, CWMCC_ATOM(client, wm_class), &s)) {
        g_warning("Failed to read WM_CLASS from 0x%lx", win);
        *class = g_strdup("");
        *name = g_strdup("");
    } else {
        if (!s[0]) {
            g_warning("Failed to read class element of WM_CLASS from 0x%lx",
                      win);
            *class = g_strdup("");
        } else
            *class = g_strdup(s[0]);
        if (!s[0] || !s[1]) {
            g_warning("Failed to read name element of WM_CLASS from 0x%lx",
                      win);
            *name = g_strdup("");
        } else
            *name = g_strdup(s[1]);
    }
    g_strfreev(s);
}

void cwmcc_client_get_role(Window win, char **role)
{
    if (!prop_get_string_locale(win,
                                CWMCC_ATOM(client, wm_window_role), role)) {
        g_warning("Failed to read WM_WINDOW_ROLE from 0x%lx", win);
        *role = g_strdup("");
    }
}

void cwmcc_client_get_mwmhints(Window win, struct Cwmcc_MwmHints *hints)
{
    gulong *l = NULL, num;

    if (!prop_get_array32(win, CWMCC_ATOM(client, motif_wm_hints),
                          CWMCC_ATOM(client, motif_wm_hints), &l, &num)) {
        g_warning("Failed to read Motif WM Hints from 0x%lx", win);
        hints->flags = 0;
    } else if (num < 3) {
        g_warning("Read incomplete Motif WM Hints from 0x%lx", win);
        hints->flags = 0;
    } else {
        hints->flags = l[0];
        hints->functions = l[1];
        hints->decorations = l[2];
    }
    g_free(l);
}

void cwmcc_client_get_desktop(Window win, gulong *desk)
{
    if (!prop_get32(win, CWMCC_ATOM(client, net_wm_desktop),
                    CWMCC_ATOM(type, cardinal), desk)) {
        g_warning("Failed to read NET_WM_DESKTOP from 0x%lx", win);
        *desk = 0;
    }
}

void cwmcc_client_set_desktop(Window win, gulong desk)
{
    XChangeProperty(cwmcc_display, win, CWMCC_ATOM(client, net_wm_desktop),
                    CWMCC_ATOM(type, cardinal), 32, PropModeReplace,
                    (guchar*)&desk, 1);
}

void cwmcc_client_get_type(Window win, gulong **types)
{
    gulong num;

    if (!prop_get_array32(win, CWMCC_ATOM(client, net_wm_window_type),
                    CWMCC_ATOM(type, atom), types, &num)) {
        g_warning("Failed to read NET_WM_WINDOW_TYPE from 0x%lx", win);
        *types = g_new(Atom, 2);
        (*types)[0] = CWMCC_ATOM(data, net_wm_window_type_normal);
        (*types)[1] = 0;
    }
}

void cwmcc_client_set_type(Window win, gulong *types)
{
    gulong n;
    gulong *t;

    for (t = types, n = 0; *t; ++t, ++n);
    XChangeProperty(cwmcc_display, win, CWMCC_ATOM(client, wm_state),
                    CWMCC_ATOM(type, atom), 32, PropModeReplace,
                    (guchar*)types, n);
}

void cwmcc_client_get_state(Window win, gulong **states)
{
    gulong num;

    if (!prop_get_array32(win, CWMCC_ATOM(client, net_wm_state),
                    CWMCC_ATOM(type, atom), states, &num)) {
        g_warning("Failed to read NET_WM_STATE from 0x%lx", win);
        *states = g_new(Atom, 1);
        (*states)[0] = 0;
    }
}

void cwmcc_client_set_state(Window win, gulong *states)
{
    gulong n;
    gulong *s;

    for (s = states, n = 0; *s; ++s, ++n);
    XChangeProperty(cwmcc_display, win, CWMCC_ATOM(client, wm_state),
                    CWMCC_ATOM(type, atom), 32, PropModeReplace,
                    (guchar*)states, n);
}

void cwmcc_client_get_strut(Window win, int *l, int *t, int *r, int *b)
{
    gulong *data = NULL, num;

    if (!prop_get_array32(win, CWMCC_ATOM(client, net_wm_strut),
                    CWMCC_ATOM(type, cardinal), &data, &num)) {
        g_warning("Failed to read NET_WM_STRUT from 0x%lx", win);
        *l = *t = *r = *b = 0;
    } else if (num != 4) {
        g_warning("Read invalid NET_WM_STRUT from 0x%lx", win);
        *l = *t = *r = *b = 0;
    } else {
	*l = data[0];
        *r = data[1];
        *t = data[2];
        *b = data[3];
    }
    g_free(data);
}

static void convert_pixmap_to_icon(Pixmap pix, Pixmap mask,
                                   struct Cwmcc_Icon *icon)
{
/*
    guint pw, ph, mw, mh, depth;
    Window wjunk;
    int ijunk;
    guint uijunk;
    guint x, y;

    if (!XGetGeometry(cwmcc_display, pix, &wjunk, &ijunk, &ijunk, &pw, &ph,
                      &uijunk, &depth)) {
        g_message("Unable to read pixmap icon's geometry");
        icon->width = icon->height = 0;
        icon->data = NULL;
        return;
    }
    if (!XGetGeometry(cwmcc_display, mask, &wjunk, &ijunk, &ijunk, &mw, &mh,
                      &uijunk, &ujunk)) {
        g_message("Unable to read pixmap icon's mask's geometry");
        icon->width = icon->height = 0;
        icon->data = NULL;
        return;
    }
    if (pw != mw || ph !_ mh) {
        g_warning("Pixmap icon's mask does not match icon's dimensions");
        icon->width = icon->height = 0;
        icon->data = NULL;
        return;
    }

    for (y = 0; y < ph; ++y)
        for (x = 0; x < pw; ++x) {
        }
*/
    icon->width = icon->height = 0;
    icon->data = NULL;
}

void cwmcc_client_get_icon(Window win, struct Cwmcc_Icon **icons)
{
    gulong *data = NULL, num;
    gulong w, h, i;
    int j;
    int nicons;

    if (!prop_get_array32(win, CWMCC_ATOM(client, net_wm_icon),
                          CWMCC_ATOM(type, cardinal), &data, &num)) {
        g_warning("Failed to read NET_WM_ICON from 0x%lx", win);
        *icons = NULL;
        nicons = 0;
    } else { 
	/* figure out how many valid icons are in here */
	i = 0;
        nicons = 0;
	while (num - i > 2) {
	    w = data[i++];
	    h = data[i++];
	    i += w * h;
	    if (i > num) break;
	    ++nicons;
	}

	*icons = g_new(struct Cwmcc_Icon, nicons + 1);
        (*icons)[nicons].data = NULL;
    
	/* store the icons */
	i = 0;
	for (j = 0; j < nicons; ++j) {
	    w = (*icons)[j].width = data[i++];
	    h = (*icons)[j].height = data[i++];
            (*icons)[j].data =
                g_memdup(&data[i], w * h * sizeof(gulong));
	    i += w * h;
	    g_assert(i <= num);
	}
    }
    g_free(data);

    data = NULL;
    if (!prop_get_array32(win, CWMCC_ATOM(client, kwm_win_icon),
                          CWMCC_ATOM(client, kwm_win_icon), &data, &num)) {
        g_warning("Failed to read KWM_WIN_ICON from 0x%lx", win);
    } else if (num != 2) {
        g_warning("Read invalid KWM_WIN_ICON from 0x%lx", win);
    } else {
        Pixmap p, m;
        struct Cwmcc_Icon icon;

        p = data[0];
        m = data[1];

        convert_pixmap_to_icon(p, m, &icon);

        if (icon.data) {
            *icons = g_renew(struct Cwmcc_Icon, *icons, nicons + 2);
            (*icons[nicons + 1]).data = NULL;
            g_memmove(&(*icons)[nicons], &icon, sizeof(struct Cwmcc_Icon));
        }
    }
    g_free(data);

}

void cwmcc_client_get_premax(Window win, int *x, int *y, int *w, int *h)
{
    gulong *l = NULL, num;

    if (!prop_get_array32(win, CWMCC_ATOM(client, openbox_premax),
                    CWMCC_ATOM(type, cardinal), &l, &num)) {
        g_warning("Failed to read OPENBOX_PREMAX from 0x%lx", win);
        *x = *y = *w = *h = 0;
    } else if (num != 4) {
        g_warning("Read invalid OPENBOX_PREMAX from 0x%lx", win);
        *x = *y = *w = *h = 0;
    } else {
	*x = l[0];
        *y = l[1];
        *w = l[2];
        *h = l[3];
    }
    g_free(l);
}

void cwmcc_client_set_premax(Window win, int x, int y, int w, int h)
{
    gulong l[4];

    l[0] = x;
    l[1] = y;
    l[2] = w;
    l[3] = h;
    XChangeProperty(cwmcc_display, win, CWMCC_ATOM(client, openbox_premax),
                    CWMCC_ATOM(type, cardinal), 32, PropModeReplace,
                    (guchar*)l, 4);
}
