#include "cwmcc_internal.h"
#include "atom.h"
#include "prop.h"
#include "root_props.h"

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

/*void cwmcc_root_get_workarea(Window win, Rect a)
{
}*/

void cwmcc_root_get_supporting_wm_check(Window win, Window *window)
{
    if (!prop_get32(win, CWMCC_ATOM(root, net_supporting_wm_check),
                    CWMCC_ATOM(type, window), window)) {
        g_warning("Failed to read NET_SUPPORTING_WM_CHECK from 0x%lx", win);
        *window = None;
    }
}

void cwmcc_root_get_desktop_layout(Window win,
                                   struct Cwmcc_DesktopLayout *layout)
{
    gulong *data = NULL, num;
    gulong desks;

    /* need the number of desktops */
    cwmcc_root_get_number_of_desktops(win, &desks);

    layout->orientation = Cwmcc_Orientation_Horz;
    layout->start_corner = Cwmcc_Corner_TopLeft;
    layout->rows = 1;
    layout->columns = desks;

    if (!prop_get_array32(win, CWMCC_ATOM(root, net_desktop_layout),
                    CWMCC_ATOM(type, cardinal), &data, &num)) {
        g_warning("Failed to read NET_DESKTOP_LAYOUT from 0x%lx", win);
    } else if (num != 4) {
        g_warning("Read invalid NET_DESKTOP_LAYOUT from 0x%lx", win);
    } else {
        if (data[0] == Cwmcc_Orientation_Horz ||
            data[0] == Cwmcc_Orientation_Vert)
            layout->orientation = data[0];
        if (data[3] == Cwmcc_Corner_TopLeft ||
            data[3] == Cwmcc_Corner_TopRight ||
            data[3] == Cwmcc_Corner_BottomLeft ||
            data[3] == Cwmcc_Corner_BottomRight)
            layout->start_corner = data[3];
        layout->rows = data[2];
        layout->columns = data[1];

	/* bounds checking */
	if (layout->orientation == Cwmcc_Orientation_Horz) {
	    if (layout->rows > desks)
		layout->rows = desks;
	    if (layout->columns > ((desks + desks % layout->rows) /
                                   layout->rows))
		layout->columns = ((desks + desks % layout->rows) /
                                   layout->rows);
	} else {
	    if (layout->columns > desks)
		layout->columns = desks;
	    if (layout->rows > ((desks + desks % layout->columns) /
                                layout->columns))
		layout->rows = ((desks + desks % layout->columns) /
                                layout->columns);
	}
    }
    g_free(data);
}

void cwmcc_root_get_showing_desktop(Window win, gboolean *showing)
{
    gulong a;

    if (!prop_get32(win, CWMCC_ATOM(root, net_showing_desktop),
                    CWMCC_ATOM(type, cardinal), &a)) {
        g_warning("Failed to read NET_SHOWING_DESKTOP from 0x%lx", win);
        a = FALSE;
    }
    *showing = !!a;
}

void cwmcc_root_get_openbox_pid(Window win, gulong *pid)
{
    if (!prop_get32(win, CWMCC_ATOM(root, openbox_pid),
                    CWMCC_ATOM(type, cardinal), pid)) {
        g_warning("Failed to read OPENBOX_PID from 0x%lx", win);
        *pid = 0;
    }
}

