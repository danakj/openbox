#include "cwmcc_internal.h"
#include "atom.h"
#include "prop.h"
#include "client_props.h"
#include "render/render.h"

#include <X11/Xutil.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

void cwmcc_root_get_supported(Window win, Atom **atoms)
{
    gulong num;

    if (!prop_get_array32(win, CWMCC_ATOM(root, net_supported),
                          CWMCC_ATOM(type, atom), atoms, &num)) {
        g_warning("Failed to read NET_SUPPORTED from 0x%lx", win);
        *atoms = NULL;
    }
}

void cwmcc_root_get_client_list(Window win, Window **windows)
{
    gulong num;

    if (!prop_get_array32(win, CWMCC_ATOM(root, net_client_list),
                          CWMCC_ATOM(type, window), windows, &num)) {
        g_warning("Failed to read NET_CLIENT_LIST from 0x%lx", win);
        *windows = NULL;
    }
}

void cwmcc_root_get_client_list_stacking(Window win, Window **windows)
{
    gulong num;

    if (!prop_get_array32(win, CWMCC_ATOM(root, net_client_list_stacking),
                          CWMCC_ATOM(type, window), windows, &num)) {
        g_warning("Failed to read NET_CLIENT_LIST_STACKING from 0x%lx", win);
        *windows = NULL;
    }
}

void cwmcc_root_get_number_of_desktops(Window win, gulong *desktops)
{
    if (!prop_get32(win, CWMCC_ATOM(root, net_number_of_desktops),
                    CWMCC_ATOM(type, cardinal), desktops)) {
        g_warning("Failed to read NET_NUMBER_OF_DESKTOPS from 0x%lx", win);
        *desktops = 1;
    }
}

void cwmcc_root_get_desktop_geometry(Window win, gulong *w, gulong *h)
{
    gulong *data = NULL, num;

    if (!prop_get_array32(win, CWMCC_ATOM(root, net_desktop_geometry),
                    CWMCC_ATOM(type, cardinal), &data, &num)) {
        g_warning("Failed to read NET_DESKTOP_GEOMETRY from 0x%lx", win);
        *w = *h = 0;
    } else if (num != 2) {
        g_warning("Read invalid NET_DESKTOP_GEOMETRY from 0x%lx", win);
        *w = *h = 0;
    } else {
	*w = data[0];
        *h = data[1];
    }
    g_free(data);
}

void cwmcc_root_get_desktop_viewport(Window win, gulong *x, gulong *y)
{
    gulong *data = NULL, num;

    if (!prop_get_array32(win, CWMCC_ATOM(root, net_desktop_viewport),
                    CWMCC_ATOM(type, cardinal), &data, &num)) {
        g_warning("Failed to read NET_DESKTOP_VIEWPORT from 0x%lx", win);
        *x = *y = 0;
    } else if (num != 2) {
        g_warning("Read invalid NET_DESKTOP_VIEWPORT from 0x%lx", win);
        *x = *y = 0;
    } else {
	*x = data[0];
        *y = data[1];
    }
    g_free(data);
}

void cwmcc_root_get_current_desktop(Window win, gulong *desktop)
{
    if (!prop_get32(win, CWMCC_ATOM(root, net_current_desktop),
                    CWMCC_ATOM(type, cardinal), desktop)) {
        g_warning("Failed to read NET_CURRENT_DESKTOP from 0x%lx", win);
        *desktop = 0;
    }
}

void cwmcc_root_get_desktop_names(Window win, char ***names)
{
    if (!prop_get_strings_utf8(win,
                               CWMCC_ATOM(root, net_desktop_names), names)) {
        g_warning("Failed to read NET_DESKTOP_NAMES from 0x%lx", win);
        *names = NULL;
    }
}

void cwmcc_root_get_active_window(Window win, Window *window)
{
    if (!prop_get32(win, CWMCC_ATOM(root, net_active_window),
                    CWMCC_ATOM(type, window), window)) {
        g_warning("Failed to read NET_ACTIVE_WINDOW from 0x%lx", win);
        *window = None;
    }
}

