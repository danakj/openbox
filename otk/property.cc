// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "property.hh"
#include "display.hh"

extern "C" {
#include <assert.h>
}

#include <algorithm>

namespace otk {

Property::Property()
{
  assert(Display::display);

  // make sure asserts fire if there is a problem
  memset(_atoms, 0, sizeof(_atoms));

  _atoms[Atom_Cardinal] = XA_CARDINAL;
  _atoms[Atom_Window] = XA_WINDOW;
  _atoms[Atom_Pixmap] = XA_PIXMAP;
  _atoms[Atom_Atom] = XA_ATOM;
  _atoms[Atom_String] = XA_STRING;
  _atoms[Atom_Utf8] = create("UTF8_STRING");
  
  _atoms[openbox_pid] = create("_OPENBOX_PID");

  _atoms[wm_colormap_windows] = create("WM_COLORMAP_WINDOWS");
  _atoms[wm_protocols] = create("WM_PROTOCOLS");
  _atoms[wm_state] = create("WM_STATE");
  _atoms[wm_change_state] = create("WM_CHANGE_STATE");
  _atoms[wm_delete_window] = create("WM_DELETE_WINDOW");
  _atoms[wm_take_focus] = create("WM_TAKE_FOCUS");
  _atoms[wm_name] = create("WM_NAME");
  _atoms[wm_icon_name] = create("WM_ICON_NAME");
  _atoms[wm_class] = create("WM_CLASS");
  _atoms[wm_window_role] = create("WM_WINDOW_ROLE");
  _atoms[motif_wm_hints] = create("_MOTIF_WM_HINTS");
  _atoms[blackbox_hints] = create("_BLACKBOX_HINTS");
  _atoms[blackbox_attributes] = create("_BLACKBOX_ATTRIBUTES");
  _atoms[blackbox_change_attributes] = create("_BLACKBOX_CHANGE_ATTRIBUTES");
  _atoms[blackbox_structure_messages] = create("_BLACKBOX_STRUCTURE_MESSAGES");
  _atoms[blackbox_notify_startup] = create("_BLACKBOX_NOTIFY_STARTUP");
  _atoms[blackbox_notify_window_add] = create("_BLACKBOX_NOTIFY_WINDOW_ADD");
  _atoms[blackbox_notify_window_del] = create("_BLACKBOX_NOTIFY_WINDOW_DEL");
  _atoms[blackbox_notify_current_workspace] = 
    create("_BLACKBOX_NOTIFY_CURRENT_WORKSPACE");
  _atoms[blackbox_notify_workspace_count] =
    create("_BLACKBOX_NOTIFY_WORKSPACE_COUNT");
  _atoms[blackbox_notify_window_focus] =
    create("_BLACKBOX_NOTIFY_WINDOW_FOCUS");
  _atoms[blackbox_notify_window_raise] =
    create("_BLACKBOX_NOTIFY_WINDOW_RAISE");
  _atoms[blackbox_notify_window_lower] =
    create("_BLACKBOX_NOTIFY_WINDOW_LOWER");
  
  _atoms[blackbox_change_workspace] = create("_BLACKBOX_CHANGE_WORKSPACE");
  _atoms[blackbox_change_window_focus] =
    create("_BLACKBOX_CHANGE_WINDOW_FOCUS");
  _atoms[blackbox_cycle_window_focus] = create("_BLACKBOX_CYCLE_WINDOW_FOCUS");

  _atoms[openbox_show_root_menu] = create("_OPENBOX_SHOW_ROOT_MENU");
  _atoms[openbox_show_workspace_menu] = create("_OPENBOX_SHOW_WORKSPACE_MENU");

  _atoms[net_supported] = create("_NET_SUPPORTED");
  _atoms[net_client_list] = create("_NET_CLIENT_LIST");
  _atoms[net_client_list_stacking] = create("_NET_CLIENT_LIST_STACKING");
  _atoms[net_number_of_desktops] = create("_NET_NUMBER_OF_DESKTOPS");
  _atoms[net_desktop_geometry] = create("_NET_DESKTOP_GEOMETRY");
  _atoms[net_desktop_viewport] = create("_NET_DESKTOP_VIEWPORT");
  _atoms[net_current_desktop] = create("_NET_CURRENT_DESKTOP");
  _atoms[net_desktop_names] = create("_NET_DESKTOP_NAMES");
  _atoms[net_active_window] = create("_NET_ACTIVE_WINDOW");
  _atoms[net_workarea] = create("_NET_WORKAREA");
  _atoms[net_supporting_wm_check] = create("_NET_SUPPORTING_WM_CHECK");
//  _atoms[net_virtual_roots] = create("_NET_VIRTUAL_ROOTS");

  _atoms[net_close_window] = create("_NET_CLOSE_WINDOW");
  _atoms[net_wm_moveresize] = create("_NET_WM_MOVERESIZE");

//  _atoms[net_properties] = create("_NET_PROPERTIES");
  _atoms[net_wm_name] = create("_NET_WM_NAME");
  _atoms[net_wm_visible_name] = create("_NET_WM_VISIBLE_NAME");
  _atoms[net_wm_icon_name] = create("_NET_WM_ICON_NAME");
  _atoms[net_wm_visible_icon_name] = create("_NET_WM_VISIBLE_ICON_NAME");
  _atoms[net_wm_desktop] = create("_NET_WM_DESKTOP");
  _atoms[net_wm_window_type] = create("_NET_WM_WINDOW_TYPE");
  _atoms[net_wm_state] = create("_NET_WM_STATE");
  _atoms[net_wm_strut] = create("_NET_WM_STRUT");
//  _atoms[net_wm_icon_geometry] = create("_NET_WM_ICON_GEOMETRY");
//  _atoms[net_wm_icon] = create("_NET_WM_ICON");
//  _atoms[net_wm_pid] = create("_NET_WM_PID");
//  _atoms[net_wm_handled_icons] = create("_NET_WM_HANDLED_ICONS");
  _atoms[net_wm_allowed_actions] = create("_NET_WM_ALLOWED_ACTIONS");

//  _atoms[net_wm_ping] = create("_NET_WM_PING");
  
  _atoms[net_wm_window_type_desktop] = create("_NET_WM_WINDOW_TYPE_DESKTOP");
  _atoms[net_wm_window_type_dock] = create("_NET_WM_WINDOW_TYPE_DOCK");
  _atoms[net_wm_window_type_toolbar] = create("_NET_WM_WINDOW_TYPE_TOOLBAR");
  _atoms[net_wm_window_type_menu] = create("_NET_WM_WINDOW_TYPE_MENU");
  _atoms[net_wm_window_type_utility] = create("_NET_WM_WINDOW_TYPE_UTILITY");
  _atoms[net_wm_window_type_splash] = create("_NET_WM_WINDOW_TYPE_SPLASH");
  _atoms[net_wm_window_type_dialog] = create("_NET_WM_WINDOW_TYPE_DIALOG");
  _atoms[net_wm_window_type_normal] = create("_NET_WM_WINDOW_TYPE_NORMAL");

  _atoms[net_wm_moveresize_size_topleft] =
    create("_NET_WM_MOVERESIZE_SIZE_TOPLEFT");
  _atoms[net_wm_moveresize_size_topright] =
    create("_NET_WM_MOVERESIZE_SIZE_TOPRIGHT");
  _atoms[net_wm_moveresize_size_bottomleft] =
    create("_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT");
  _atoms[net_wm_moveresize_size_bottomright] =
    create("_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT");
  _atoms[net_wm_moveresize_move] =
    create("_NET_WM_MOVERESIZE_MOVE");
 
  _atoms[net_wm_action_move] = create("_NET_WM_ACTION_MOVE");
  _atoms[net_wm_action_resize] = create("_NET_WM_ACTION_RESIZE");
  _atoms[net_wm_action_shade] = create("_NET_WM_ACTION_SHADE");
  _atoms[net_wm_action_maximize_horz] = create("_NET_WM_ACTION_MAXIMIZE_HORZ");
  _atoms[net_wm_action_maximize_vert] = create("_NET_WM_ACTION_MAXIMIZE_VERT");
  _atoms[net_wm_action_change_desktop] =
    create("_NET_WM_ACTION_CHANGE_DESKTOP");
  _atoms[net_wm_action_close] = create("_NET_WM_ACTION_CLOSE");
    
  _atoms[net_wm_state_modal] = create("_NET_WM_STATE_MODAL");
  _atoms[net_wm_state_sticky] = create("_NET_WM_STATE_STICKY");
  _atoms[net_wm_state_maximized_vert] = create("_NET_WM_STATE_MAXIMIZED_VERT");
  _atoms[net_wm_state_maximized_horz] = create("_NET_WM_STATE_MAXIMIZED_HORZ");
  _atoms[net_wm_state_shaded] = create("_NET_WM_STATE_SHADED");
  _atoms[net_wm_state_skip_taskbar] = create("_NET_WM_STATE_SKIP_TASKBAR");
  _atoms[net_wm_state_skip_pager] = create("_NET_WM_STATE_SKIP_PAGER");
  _atoms[net_wm_state_hidden] = create("_NET_WM_STATE_HIDDEN");
  _atoms[net_wm_state_fullscreen] = create("_NET_WM_STATE_FULLSCREEN");
  _atoms[net_wm_state_above] = create("_NET_WM_STATE_ABOVE");
  _atoms[net_wm_state_below] = create("_NET_WM_STATE_BELOW");
  
  _atoms[kde_net_system_tray_windows] = create("_KDE_NET_SYSTEM_TRAY_WINDOWS");
  _atoms[kde_net_wm_system_tray_window_for] =
    create("_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR");
  _atoms[kde_net_wm_window_type_override] =
    create("_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");
}


/*
 * clean up the class' members
 */
Property::~Property()
{
}


/*
 * Returns an atom from the Xserver, creating it if necessary.
 */
Atom Property::create(const char *name) const
{
  Atom a = XInternAtom(Display::display, name, False);
  assert(a);
  return a;
}


/*
 * Internal set.
 * Sets a window property on a window, optionally appending to the existing
 * value.
 */
void Property::set(Window win, Atom atom, Atom type,
                          unsigned char* data, int size, int nelements,
                          bool append) const
{
  assert(win != None); assert(atom != None); assert(type != None);
  assert(nelements == 0 || (nelements > 0 && data != (unsigned char *) 0));
  assert(size == 8 || size == 16 || size == 32);
  XChangeProperty(Display::display, win, atom, type, size,
                  (append ? PropModeAppend : PropModeReplace),
                  data, nelements);
}


/*
 * Set a 32-bit property value on a window.
 */
void Property::set(Window win, Atoms atom, Atoms type,
                          unsigned long value) const
{
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_ATOMS);
  set(win, _atoms[atom], _atoms[type],
           reinterpret_cast<unsigned char*>(&value), 32, 1, False);
}


/*
 * Set an array of 32-bit properties value on a window.
 */
void Property::set(Window win, Atoms atom, Atoms type,
                          unsigned long value[], int elements) const
{
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_ATOMS);
  set(win, _atoms[atom], _atoms[type],
      reinterpret_cast<unsigned char*>(value), 32, elements, False);
}


/*
 * Set an string property value on a window.
 */
void Property::set(Window win, Atoms atom, StringType type,
                          const ustring &value) const
{
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_STRING_TYPE);
  
  Atom t;
  switch (type) {
  case ascii: t = _atoms[Atom_String]; assert(!value.utf8()); break;
  case utf8:  t = _atoms[Atom_Utf8]; assert(value.utf8()); break;
  default: assert(False); return; // unhandled StringType
  }
  
  set(win, _atoms[atom], t,
      reinterpret_cast<unsigned char *>(const_cast<char *>(value.c_str())),
      8, value.size() + 1, False); // add 1 to the size to include the null
}


/*
 * Set an array of string property values on a window.
 */
void Property::set(Window win, Atoms atom, StringType type,
                     const StringVect &strings) const
{
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_STRING_TYPE);

  Atom t;
  bool u; // utf8 encoded?
  switch (type) {
  case ascii: t = _atoms[Atom_String]; u = false; break;
  case utf8:  t = _atoms[Atom_Utf8];   u = true;  break;
  default: assert(False); return; // unhandled StringType
  }

  ustring value;

  StringVect::const_iterator it = strings.begin();
  const StringVect::const_iterator end = strings.end();
  for (; it != end; ++it) {
    assert(it->utf8() == u); // the ustring is encoded correctly?
    value += *it;
    value += '\0';
  }

  set(win, _atoms[atom], t,
      reinterpret_cast<unsigned char *>(const_cast<char *>(value.c_str())),
      8, value.size(), False);
}


/*
 * Internal get function used by all of the typed get functions.
 * Gets an property's value from a window.
 * Returns True if the property was successfully retrieved; False if the
 * property did not exist on the window, or has a different type/size format
 * than the user tried to retrieve.
 */
bool Property::get(Window win, Atom atom, Atom type,
                          unsigned long *nelements, unsigned char **value,
                          int size) const
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
  bool ret = False;

  // try get the first element
  result = XGetWindowProperty(Display::display, win, atom, 0l, 1l,
                              False, AnyPropertyType, &ret_type, &ret_size,
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
      result = XGetWindowProperty(Display::display, win, atom, 0l,
                                  remain, False, type, &ret_type, &ret_size,
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


/*
 * Gets a 32-bit property's value from a window.
 */
bool Property::get(Window win, Atoms atom, Atoms type,
                          unsigned long *nelements,
                          unsigned long **value) const
{
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_ATOMS);
  return get(win, _atoms[atom], _atoms[type], nelements,
                  reinterpret_cast<unsigned char **>(value), 32);
}


/*
 * Gets a single 32-bit property's value from a window.
 */
bool Property::get(Window win, Atoms atom, Atoms type,
                     unsigned long *value) const
{
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_ATOMS);
  unsigned long *temp;
  unsigned long num = 1;
  if (! get(win, _atoms[atom], _atoms[type], &num,
                 reinterpret_cast<unsigned char **>(&temp), 32))
    return False;
  *value = temp[0];
  delete [] temp;
  return True;
}


/*
 * Gets an string property's value from a window.
 */
bool Property::get(Window win, Atoms atom, StringType type,
                   ustring *value) const
{
  unsigned long n = 1;
  StringVect s;

  if (get(win, atom, type, &n, &s)) {
    *value = s[0];
    return True;
  }
  return False;
}


bool Property::get(Window win, Atoms atom, StringType type,
                   unsigned long *nelements, StringVect *strings) const
{
  assert(atom >= 0 && atom < NUM_ATOMS);
  assert(type >= 0 && type < NUM_STRING_TYPE);
  assert(win != None); assert(_atoms[atom] != None);
  assert(*nelements > 0);

  Atom t;
  bool u; // utf8 encoded?
  switch (type) {
  case ascii: t = _atoms[Atom_String]; u = false; break;
  case utf8:  t = _atoms[Atom_Utf8];   u = true;  break;
  default: assert(False); return False; // unhandled StringType
  }
  
  unsigned char *value;
  unsigned long elements = (unsigned) -1;
  if (!get(win, _atoms[atom], t, &elements, &value, 8) || elements < 1)
    return False;

  std::string s(reinterpret_cast<char *>(value), elements);
  delete [] value;

  std::string::const_iterator it = s.begin(), end = s.end();
  unsigned long num = 0;
  while(num < *nelements) {
    std::string::const_iterator tmp = it; // current string.begin()
    it = std::find(tmp, end, '\0');       // look for null between tmp and end
    strings->push_back(std::string(tmp, it));   // s[tmp:it)
    if (!u) strings->back().setUtf8(false);
    ++num;
    if (it == end) break;
    ++it;
    if (it == end) break;
  }

  *nelements = num;

  return True;
}


/*
 * Removes a property entirely from a window.
 */
void Property::erase(Window win, Atoms atom) const
{
  assert(atom >= 0 && atom < NUM_ATOMS);
  XDeleteProperty(Display::display, win, _atoms[atom]);
}

}
