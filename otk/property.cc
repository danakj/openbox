// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "property.hh"
#include "display.hh"

extern "C" {
#include <X11/Xatom.h>
}

#include <algorithm>
#include <cassert>

namespace otk {

Atoms Property::atoms;

static Atom create(char *name) { return XInternAtom(**display, name, false); }

void Property::initialize()
{
  assert(display);

  // make sure asserts fire if there is a problem
  memset(&atoms, 0, sizeof(Atoms));

  atoms.cardinal = XA_CARDINAL;
  atoms.window = XA_WINDOW;
  atoms.pixmap = XA_PIXMAP;
  atoms.atom = XA_ATOM;
  atoms.string = XA_STRING;
  atoms.utf8 = create("UTF8_STRING");
  
  atoms.openbox_pid = create("_OPENBOX_PID");

  atoms.wm_colormap_windows = create("WM_COLORMAP_WINDOWS");
  atoms.wm_protocols = create("WM_PROTOCOLS");
  atoms.wm_state = create("WM_STATE");
  atoms.wm_change_state = create("WM_CHANGE_STATE");
  atoms.wm_delete_window = create("WM_DELETE_WINDOW");
  atoms.wm_take_focus = create("WM_TAKE_FOCUS");
  atoms.wm_name = create("WM_NAME");
  atoms.wm_icon_name = create("WM_ICON_NAME");
  atoms.wm_class = create("WM_CLASS");
  atoms.wm_window_role = create("WM_WINDOW_ROLE");
  atoms.motif_wm_hints = create("_MOTIF_WM_HINTS");

  atoms.openbox_show_root_menu = create("_OPENBOX_SHOW_ROOT_MENU");
  atoms.openbox_show_workspace_menu = create("_OPENBOX_SHOW_WORKSPACE_MENU");

  atoms.net_supported = create("_NET_SUPPORTED");
  atoms.net_client_list = create("_NET_CLIENT_LIST");
  atoms.net_client_list_stacking = create("_NET_CLIENT_LIST_STACKING");
  atoms.net_number_of_desktops = create("_NET_NUMBER_OF_DESKTOPS");
  atoms.net_desktop_geometry = create("_NET_DESKTOP_GEOMETRY");
  atoms.net_desktop_viewport = create("_NET_DESKTOP_VIEWPORT");
  atoms.net_current_desktop = create("_NET_CURRENT_DESKTOP");
  atoms.net_desktop_names = create("_NET_DESKTOP_NAMES");
  atoms.net_active_window = create("_NET_ACTIVE_WINDOW");
  atoms.net_workarea = create("_NET_WORKAREA");
  atoms.net_supporting_wm_check = create("_NET_SUPPORTING_WM_CHECK");
//  atoms.net_virtual_roots = create("_NET_VIRTUAL_ROOTS");
  atoms.net_desktop_layout = create("_NET_DESKTOP_LAYOUT");
  atoms.net_showing_desktop = create("_NET_SHOWING_DESKTOP");

  atoms.net_close_window = create("_NET_CLOSE_WINDOW");
  atoms.net_wm_moveresize = create("_NET_WM_MOVERESIZE");

//  atoms.net_properties = create("_NET_PROPERTIES");
  atoms.net_wm_name = create("_NET_WM_NAME");
  atoms.net_wm_visible_name = create("_NET_WM_VISIBLE_NAME");
  atoms.net_wm_icon_name = create("_NET_WM_ICON_NAME");
  atoms.net_wm_visible_icon_name = create("_NET_WM_VISIBLE_ICON_NAME");
  atoms.net_wm_desktop = create("_NET_WM_DESKTOP");
  atoms.net_wm_window_type = create("_NET_WM_WINDOW_TYPE");
  atoms.net_wm_state = create("_NET_WM_STATE");
  atoms.net_wm_strut = create("_NET_WM_STRUT");
//  atoms.net_wm_icon_geometry = create("_NET_WM_ICON_GEOMETRY");
//  atoms.net_wm_icon = create("_NET_WM_ICON");
//  atoms.net_wm_pid = create("_NET_WM_PID");
//  atoms.net_wm_handled_icons = create("_NET_WM_HANDLED_ICONS");
  atoms.net_wm_allowed_actions = create("_NET_WM_ALLOWED_ACTIONS");

//  atoms.net_wm_ping = create("_NET_WM_PING");
  
  atoms.net_wm_window_type_desktop = create("_NET_WM_WINDOW_TYPE_DESKTOP");
  atoms.net_wm_window_type_dock = create("_NET_WM_WINDOW_TYPE_DOCK");
  atoms.net_wm_window_type_toolbar = create("_NET_WM_WINDOW_TYPE_TOOLBAR");
  atoms.net_wm_window_type_menu = create("_NET_WM_WINDOW_TYPE_MENU");
  atoms.net_wm_window_type_utility = create("_NET_WM_WINDOW_TYPE_UTILITY");
  atoms.net_wm_window_type_splash = create("_NET_WM_WINDOW_TYPE_SPLASH");
  atoms.net_wm_window_type_dialog = create("_NET_WM_WINDOW_TYPE_DIALOG");
  atoms.net_wm_window_type_normal = create("_NET_WM_WINDOW_TYPE_NORMAL");

  atoms.net_wm_moveresize_size_topleft =
    create("_NET_WM_MOVERESIZE_SIZE_TOPLEFT");
  atoms.net_wm_moveresize_size_topright =
    create("_NET_WM_MOVERESIZE_SIZE_TOPRIGHT");
  atoms.net_wm_moveresize_size_bottomleft =
    create("_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT");
  atoms.net_wm_moveresize_size_bottomright =
    create("_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT");
  atoms.net_wm_moveresize_move =
    create("_NET_WM_MOVERESIZE_MOVE");
 
  atoms.net_wm_action_move = create("_NET_WM_ACTION_MOVE");
  atoms.net_wm_action_resize = create("_NET_WM_ACTION_RESIZE");
  atoms.net_wm_action_minimize = create("_NET_WM_ACTION_MINIMIZE");
  atoms.net_wm_action_shade = create("_NET_WM_ACTION_SHADE");
  atoms.net_wm_action_stick = create("_NET_WM_ACTION_STICK");
  atoms.net_wm_action_maximize_horz = create("_NET_WM_ACTION_MAXIMIZE_HORZ");
  atoms.net_wm_action_maximize_vert = create("_NET_WM_ACTION_MAXIMIZE_VERT");
  atoms.net_wm_action_fullscreen = create("_NET_WM_ACTION_FULLSCREEN");
  atoms.net_wm_action_change_desktop =
    create("_NET_WM_ACTION_CHANGE_DESKTOP");
  atoms.net_wm_action_close = create("_NET_WM_ACTION_CLOSE");
    
  atoms.net_wm_state_modal = create("_NET_WM_STATE_MODAL");
  atoms.net_wm_state_sticky = create("_NET_WM_STATE_STICKY");
  atoms.net_wm_state_maximized_vert = create("_NET_WM_STATE_MAXIMIZED_VERT");
  atoms.net_wm_state_maximized_horz = create("_NET_WM_STATE_MAXIMIZED_HORZ");
  atoms.net_wm_state_shaded = create("_NET_WM_STATE_SHADED");
  atoms.net_wm_state_skip_taskbar = create("_NET_WM_STATE_SKIP_TASKBAR");
  atoms.net_wm_state_skip_pager = create("_NET_WM_STATE_SKIP_PAGER");
  atoms.net_wm_state_hidden = create("_NET_WM_STATE_HIDDEN");
  atoms.net_wm_state_fullscreen = create("_NET_WM_STATE_FULLSCREEN");
  atoms.net_wm_state_above = create("_NET_WM_STATE_ABOVE");
  atoms.net_wm_state_below = create("_NET_WM_STATE_BELOW");
  
  atoms.kde_net_system_tray_windows = create("_KDE_NET_SYSTEM_TRAY_WINDOWS");
  atoms.kde_net_wm_system_tray_window_for =
    create("_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR");
  atoms.kde_net_wm_window_type_override =
    create("_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");

  atoms.openbox_premax = create("_OPENBOX_PREMAX");
  atoms.openbox_active_window = create("_OPENBOX_ACTIVE_WINDOW");
}

void Property::set(Window win, Atom atom, Atom type, unsigned char* data,
                   int size, int nelements, bool append)
{
  assert(win != None); assert(atom != None); assert(type != None);
  assert(nelements == 0 || (nelements > 0 && data != (unsigned char *) 0));
  assert(size == 8 || size == 16 || size == 32);
  XChangeProperty(**display, win, atom, type, size,
                  (append ? PropModeAppend : PropModeReplace),
                  data, nelements);
}

void Property::set(Window win, Atom atom, Atom type, unsigned long value)
{
  set(win, atom, type, (unsigned char*) &value, 32, 1, false);
}

void Property::set(Window win, Atom atom, Atom type, unsigned long value[],
                   int elements)
{
  set(win, atom, type, (unsigned char*) value, 32, elements, false);
}

void Property::set(Window win, Atom atom, StringType type,
                   const ustring &value)
{
  Atom t;
  switch (type) {
  case ascii: t = atoms.string; assert(!value.utf8()); break;
  case utf8:  t = atoms.utf8;   assert(value.utf8());  break;
  default: assert(false); return; // unhandled StringType
  }

  // add 1 to the size to include the trailing null
  set(win, atom, t, (unsigned char*) value.c_str(), 8, value.bytes() + 1,
      false);
}

void Property::set(Window win, Atom atom, StringType type,
                     const StringVect &strings)
{
  Atom t;
  bool u; // utf8 encoded?
  switch (type) {
  case ascii: t = atoms.string; u = false; break;
  case utf8:  t = atoms.utf8;   u = true;  break;
  default: assert(false); return; // unhandled StringType
  }

  ustring value(u);

  StringVect::const_iterator it = strings.begin();
  const StringVect::const_iterator end = strings.end();
  for (; it != end; ++it) {
    assert(it->utf8() == u); // the ustring is encoded correctly?
    value += *it;
    value += '\0';
  }

  // add 1 to the size to include the trailing null
  set(win, atom, t, (unsigned char*)value.c_str(), 8,
      value.bytes() + 1, false);
}

bool Property::get(Window win, Atom atom, Atom type, unsigned long *nelements,
                   unsigned char **value, int size)
{
  assert(win != None); assert(atom != None); assert(type != None);
  assert(size == 8 || size == 16 || size == 32);
  assert(*nelements > 0);
  unsigned char *c_val = 0;        // value alloc'd in Xlib, must be XFree()d
  Atom ret_type;
  int ret_size;
  unsigned long ret_bytes;
  int result;
  unsigned long maxread = *nelements;
  bool ret = false;

  // try get the first element
  result = XGetWindowProperty(**display, win, atom, 0l, 1l,
                              false, AnyPropertyType, &ret_type, &ret_size,
                              nelements, &ret_bytes, &c_val);
  ret = (result == Success && ret_type == type && ret_size == size &&
         *nelements > 0);
  if (ret) {
    if (ret_bytes == 0 || maxread <= *nelements) {
      // we got the whole property's value
      *value = new unsigned char[*nelements * size/8 + 1];
      memcpy(*value, c_val, *nelements * size/8 + 1);
    } else {
      // get the entire property since it is larger than one long
      XFree(c_val);
      // the number of longs that need to be retreived to get the property's
      // entire value. The last + 1 is the first long that we retrieved above.
      int remain = (ret_bytes - 1)/sizeof(long) + 1 + 1;
      if (remain > size/8 * (signed)maxread) // dont get more than the max
        remain = size/8 * (signed)maxread;
      result = XGetWindowProperty(**display, win, atom, 0l,
                                  remain, false, type, &ret_type, &ret_size,
                                  nelements, &ret_bytes, &c_val);
      ret = (result == Success && ret_type == type && ret_size == size &&
             ret_bytes == 0);
      /*
        If the property has changed type/size, or has grown since our first
        read of it, then stop here and try again. If it shrank, then this will
        still work.
      */
      if (! ret)
        return get(win, atom, type, &maxread, value, size);
  
      *value = new unsigned char[*nelements * size/8 + 1];
      memcpy(*value, c_val, *nelements * size/8 + 1);
    }    
  }
  if (c_val) XFree(c_val);
  return ret;
}

bool Property::get(Window win, Atom atom, Atom type, unsigned long *nelements,
                   unsigned long **value)
{
  return get(win, atom, type, nelements, (unsigned char**) value, 32);
}

bool Property::get(Window win, Atom atom, Atom type, unsigned long *value)
{
  unsigned long *temp;
  unsigned long num = 1;
  if (! get(win, atom, type, &num, (unsigned char **) &temp, 32))
    return false;
  *value = temp[0];
  delete [] temp;
  return true;
}

bool Property::get(Window win, Atom atom, StringType type, ustring *value)
{
  unsigned long n = 1;
  StringVect s;

  if (get(win, atom, type, &n, &s)) {
    *value = s[0];
    return true;
  }
  return false;
}

bool Property::get(Window win, Atom atom, StringType type,
                   unsigned long *nelements, StringVect *strings)
{
  assert(*nelements > 0);

  Atom t;
  bool u; // utf8 encoded?
  switch (type) {
  case ascii: t = atoms.string; u = false; break;
  case utf8:  t = atoms.utf8;   u = true;  break;
  default: assert(false); return false; // unhandled StringType
  }
  
  unsigned char *value;
  unsigned long elements = (unsigned) -1;
  if (!get(win, atom, t, &elements, &value, 8) || elements < 1)
    return false;

  std::string s((char*)value, elements);
  delete [] value;

  std::string::const_iterator it = s.begin(), end = s.end();
  unsigned long num = 0;
  while(num < *nelements) {
    std::string::const_iterator tmp = it; // current string.begin()
    it = std::find(tmp, end, '\0');       // look for null between tmp and end
    strings->push_back(std::string(tmp, it));   // s[tmp:it)
    strings->back().setUtf8(u);
    ++num;
    if (it == end) break;
    ++it;
    if (it == end) break;
  }

  *nelements = num;

  return true;
}


/*
 * Removes a property entirely from a window.
 */
void Property::erase(Window win, Atom atom)
{
  XDeleteProperty(**display, win, atom);
}

}
