/*#ifndef __cwmcc_root_props_h
  #define __cwmcc_root_props_h*/

#include <X11/Xlib.h>
#include <glib.h>

void cwmcc_root_get_supported(Window win, Atom **atoms);

void cwmcc_root_get_client_list(Window win, Window **windows);

void cwmcc_root_get_client_list_stacking(Window win, Window **windows);

void cwmcc_root_get_number_of_desktops(Window win, gulong *desktops);

void cwmcc_root_get_desktop_geometry(Window win, gulong *w, gulong *h);

void cwmcc_root_get_desktop_viewport(Window win, gulong *x, gulong *y);

void cwmcc_root_get_current_desktop(Window win, gulong *desktop);

void cwmcc_root_get_desktop_names(Window win, char ***names);

void cwmcc_root_get_active_window(Window win, Window *window);

void cwmcc_root_get_workarea(Window win, int **x, int **y, int **w, int **h);

void cwmcc_root_get_supporting_wm_check(Window win, Window *window);

/*! Orientation of the desktops */
enum Cwmcc_Orientation {
    Cwmcc_Orientation_Horz = 0,
    Cwmcc_Orientation_Vert = 1
};

enum Cwmcc_Corner {
    Cwmcc_Corner_TopLeft = 0,
    Cwmcc_Corner_TopRight = 1,
    Cwmcc_Corner_BottomRight = 2,
    Cwmcc_Corner_BottomLeft = 3
};

struct Cwmcc_DesktopLayout {
    enum Cwmcc_Orientation orientation;
    enum Cwmcc_Corner start_corner;
    guint rows;
    guint columns;
};

void cwmcc_root_get_desktop_layout(Window win,
                                   struct Cwmcc_DesktopLayout *layout);

void cwmcc_root_get_showing_desktop(Window win, gboolean *showing);

void cwmcc_root_get_openbox_pid(Window win, gulong *pid);

/*#endif*/
