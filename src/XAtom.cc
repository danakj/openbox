// XAtom.cc for Openbox
// Copyright (c) 2002 - 2002 Ben Jansens (xor at orodu.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "XAtom.h"
#include "XDisplay.h"
#include "XScreen.h"
#include "Util.h"

XAtom::XAtom(const XDisplay *display) {
  _display = display->_display;

#ifdef    HAVE_GETPID
  openbox_pid = getAtom("_BLACKBOX_PID");
#endif // HAVE_GETPID

  wm_colormap_windows = getAtom("WM_COLORMAP_WINDOWS");
  wm_protocols = getAtom("WM_PROTOCOLS");
  wm_state = getAtom("WM_STATE");
  wm_change_state = getAtom("WM_CHANGE_STATE");
  wm_delete_window = getAtom("WM_DELETE_WINDOW");
  wm_take_focus = getAtom("WM_TAKE_FOCUS");
  motif_wm_hints = getAtom("_MOTIF_WM_HINTS");
  openbox_hints = getAtom("_BLACKBOX_HINTS");
  openbox_attributes = getAtom("_BLACKBOX_ATTRIBUTES");
  openbox_change_attributes = getAtom("_BLACKBOX_CHANGE_ATTRIBUTES");

  openbox_structure_messages = getAtom("_BLACKBOX_STRUCTURE_MESSAGES");
  openbox_notify_startup = getAtom("_BLACKBOX_NOTIFY_STARTUP");
  openbox_notify_window_add = getAtom("_BLACKBOX_NOTIFY_WINDOW_ADD");
  openbox_notify_window_del = getAtom("_BLACKBOX_NOTIFY_WINDOW_DEL");
  openbox_notify_current_workspace =
    getAtom("_BLACKBOX_NOTIFY_CURRENT_WORKSPACE");
  openbox_notify_workspace_count = getAtom("_BLACKBOX_NOTIFY_WORKSPACE_COUNT");
  openbox_notify_window_focus = getAtom("_BLACKBOX_NOTIFY_WINDOW_FOCUS");
  openbox_notify_window_raise = getAtom("_BLACKBOX_NOTIFY_WINDOW_RAISE");
  openbox_notify_window_lower = getAtom("_BLACKBOX_NOTIFY_WINDOW_LOWER");
  
  openbox_change_workspace = getAtom("_BLACKBOX_CHANGE_WORKSPACE");
  openbox_change_window_focus = getAtom("_BLACKBOX_CHANGE_WINDOW_FOCUS");
  openbox_cycle_window_focus = getAtom("_BLACKBOX_CYCLE_WINDOW_FOCUS");

  net_supported = getAtom("_NET_SUPPORTED");
  net_client_list = getAtom("_NET_CLIENT_LIST");
  net_client_list_stacking = getAtom("_NET_CLIENT_LIST_STACKING");
  net_number_of_desktops = getAtom("_NET_NUMBER_OF_DESKTOPS");
  net_desktop_geometry = getAtom("_NET_DESKTOP_GEOMETRY");
  net_desktop_viewport = getAtom("_NET_DESKTOP_VIEWPORT");
  net_current_desktop = getAtom("_NET_CURRENT_DESKTOP");
  net_desktop_names = getAtom("_NET_DESKTOP_NAMES");
  net_active_window = getAtom("_NET_ACTIVE_WINDOW");
  net_workarea = getAtom("_NET_WORKAREA");
  net_supporting_wm_check = getAtom("_NET_SUPPORTING_WM_CHECK");
  net_virtual_roots = getAtom("_NET_VIRTUAL_ROOTS");

  net_close_window = getAtom("_NET_CLOSE_WINDOW");
  net_wm_moveresize = getAtom("_NET_WM_MOVERESIZE");

  net_properties = getAtom("_NET_PROPERTIES");
  net_wm_name = getAtom("_NET_WM_NAME");
  net_wm_desktop = getAtom("_NET_WM_DESKTOP");
  net_wm_window_type = getAtom("_NET_WM_WINDOW_TYPE");
  net_wm_state = getAtom("_NET_WM_STATE");
  net_wm_strut = getAtom("_NET_WM_STRUT");
  net_wm_icon_geometry = getAtom("_NET_WM_ICON_GEOMETRY");
  net_wm_icon = getAtom("_NET_WM_ICON");
  net_wm_pid = getAtom("_NET_WM_PID");
  net_wm_handled_icons = getAtom("_NET_WM_HANDLED_ICONS");

  net_wm_ping = getAtom("_NET_WM_PING");

  for (int s = 0, c = display->screenCount(); s < c; ++s)
    setSupported(display->screen(s));
}


/*
 * clean up the class' members
 */
XAtom::~XAtom() {
  while (!_support_windows.empty()) {
    // make sure we aren't fucking with this somewhere
    ASSERT(_support_windows.back() != None);
    XDestroyWindow(_display, _support_windows.back());
    _support_windows.pop_back();
  }
}


/*
 * Returns an atom from the Xserver, creating it if necessary.
 */
Atom XAtom::getAtom(const char *name) const {
  return XInternAtom(_display, name, False);
}


/*
 * Sets which atoms are supported for NETWM, by Openbox, on the root window.
 */
void XAtom::setSupported(const XScreen *screen) {
  // create the netwm support window
  Window w = XCreateSimpleWindow(_display, screen->rootWindow(),
                                 0, 0, 1, 1, 0, 0, 0);
  ASSERT(w != None);
  _support_windows.push_back(w);
  
  // we don't support any yet..
}
  

/*
 * Internal setValue used by all typed setValue functions.
 * Sets a window property on a window, optionally appending to the existing
 * value.
 */
void XAtom::setValue(Window win, Atom atom, Atom type, unsigned char* data,
                     int size, int nelements, bool append) const {
  ASSERT(win != None); ASSERT(atom != None); ASSERT(type != None);
  ASSERT(data != (unsigned char *) 0);
  ASSERT(size == 8 || size == 16 || size == 32);
  ASSERT(nelements > 0);
  XChangeProperty(_display, win, atom, type, size,
                  (append ? PropModeAppend : PropModeReplace),
                  data, nelements);                  
}


/*
 * Set a 32-bit CARDINAL property value on a window.
 */
void XAtom::setCardValue(Window win, Atom atom, long value) const {
  setValue(win, atom, XA_CARDINAL, reinterpret_cast<unsigned char*>(&value),
           32, 1, false);
}


/*
 * Set an Atom property value on a window.
 */
void XAtom::setAtomValue(Window win, Atom atom, Atom value) const {
  setValue(win, atom, XA_ATOM, reinterpret_cast<unsigned char*>(&value),
           32, 1, false);
}


/*
 * Set a Window property value on a window.
 */
void XAtom::setWindowValue(Window win, Atom atom, Window value) const {
  setValue(win, atom, XA_WINDOW, reinterpret_cast<unsigned char*>(&value),
           32, 1, false);
}


/*
 * Set a Pixmap property value on a window.
 */
void XAtom::setPixmapValue(Window win, Atom atom, Pixmap value) const {
  setValue(win, atom, XA_PIXMAP, reinterpret_cast<unsigned char*>(&value),
           32, 1, false);
}


/*
 * Set a string property value on a window.
 */
void XAtom::setStringValue(Window win, Atom atom, std::string &value) const {
  setValue(win, atom, XA_STRING,
           const_cast<unsigned char*>
           (reinterpret_cast<const unsigned char*>(value.c_str())),
           8, value.size(), false);
}


/*
 * Add elements to a 32-bit CARDINAL property value on a window.
 */
void XAtom::addCardValue(Window win, Atom atom, long value) const {
  setValue(win, atom, XA_CARDINAL, reinterpret_cast<unsigned char*>(&value),
           32, 1, true);
}


/*
 * Add elements to an Atom property value on a window.
 */
void XAtom::addAtomValue(Window win, Atom atom, Atom value) const {
  setValue(win, atom, XA_ATOM, reinterpret_cast<unsigned char*>(&value),
           32, 1, true);
}


/*
 * Add elements to a Window property value on a window.
 */
void XAtom::addWindowValue(Window win, Atom atom, Window value) const {
  setValue(win, atom, XA_WINDOW, reinterpret_cast<unsigned char*>(&value),
           32, 1, true);
}


/*
 * Add elements to a Pixmap property value on a window.
 */
void XAtom::addPixmapValue(Window win, Atom atom, Pixmap value) const {
  setValue(win, atom, XA_PIXMAP, reinterpret_cast<unsigned char*>(&value),
           32, 1, true);
}


/*
 * Add characters to a string property value on a window.
 */
void XAtom::addStringValue(Window win, Atom atom, std::string &value) const {
  setValue(win, atom, XA_STRING,
           const_cast<unsigned char*>
           (reinterpret_cast<const unsigned char *>
            (value.c_str())),
           8, value.size(), true);
} 


/*
 * Internal getValue function used by all of the typed getValue functions.
 * Gets an property's value from a window.
 * Returns true if the property was successfully retrieved; false if the
 * property did not exist on the window, or has a different type/size format
 * than the user tried to retrieve.
 */
bool XAtom::getValue(Window win, Atom atom, Atom type, unsigned long *nelements,
                     unsigned char **value, int size) const {
  unsigned char *c_val;        // value alloc'd with c malloc
  Atom ret_type;
  int ret_size;
  unsigned long ret_bytes;
  XGetWindowProperty(_display, win, atom, 0l, 1l, False, AnyPropertyType,
                     &ret_type, &ret_size, nelements, &ret_bytes,
                     &c_val); // try get the first element
  if (ret_type == None)
    // the property does not exist on the window
    return false;
  if (ret_type != type || ret_size != size) {
    // wrong data in property
    XFree(c_val);
    return false;
  }
  // the data is correct, now, is there more than 1 element?
  if (ret_bytes == 0) {
    // we got the whole property's value
    *value = new unsigned char[*nelements * size/8 + 1];
    memcpy(*value, c_val, *nelements * size/8 + 1);
    XFree(c_val);
    return true;    
  }
  // get the entire property since it is larger than one long
  free(c_val);
  // the number of longs that need to be retreived to get the property's entire
  // value. The last + 1 is the first long that we retrieved above.
  const int remain = (ret_bytes - 1)/sizeof(long) + 1 + 1;
  XGetWindowProperty(_display, win, atom, 0l, remain, False, type, &ret_type,
                     &ret_size, nelements, &ret_bytes, &c_val);
  ASSERT(ret_bytes == 0);
  *value = new unsigned char[*nelements * size/8 + 1];
  memcpy(*value, c_val, *nelements * size/8 + 1);
  XFree(c_val);
  return true;    
}


/*
 * Gets a 32-bit Cardinal property's value from a window.
 */
bool XAtom::getCardValue(Window win, Atom atom, unsigned long *nelements,
                     long **value) const {
  return XAtom::getValue(win, atom, XA_CARDINAL, nelements,
                  reinterpret_cast<unsigned char **>(value), 32);
}


/*
 * Gets an Atom property's value from a window.
 */
bool XAtom::getAtomValue(Window win, Atom atom, unsigned long *nelements,
                     Atom **value) const {
  return XAtom::getValue(win, atom, XA_ATOM, nelements,
                  reinterpret_cast<unsigned char **>(value), 32);
}


/*
 * Gets an Window property's value from a window.
 */
bool XAtom::getWindowValue(Window win, Atom atom, unsigned long *nelements,
                     Window **value) const {
  return XAtom::getValue(win, atom, XA_WINDOW, nelements,
                  reinterpret_cast<unsigned char **>(value), 32);
}


/*
 * Gets an Pixmap property's value from a window.
 */
bool XAtom::getPixmapValue(Window win, Atom atom, unsigned long *nelements,
                     Pixmap **value) const {
  return XAtom::getValue(win, atom, XA_PIXMAP, nelements,
                  reinterpret_cast<unsigned char **>(value), 32);
}


/*
 * Gets an string property's value from a window.
 */
bool XAtom::getStringValue(Window win, Atom atom, unsigned long *nelements,
                     std::string &value) const {
  unsigned char *data;
  bool ret = XAtom::getValue(win, atom, XA_STRING, nelements, &data, 8);
  if (ret)
    value = reinterpret_cast<char*>(data);
  return ret;
}


/*
 * Removes a property entirely from a window.
 */
void XAtom::eraseValue(Window win, Atom atom) const {
  XDeleteProperty(_display, win, atom);
}
