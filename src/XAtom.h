// XAtom.h for Openbox
// Copyright (c) 2002 - 2002 Ben Janens (ben at orodu.net)
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

#ifndef   __XAtom_h
#define   __XAtom_h

#include "../config.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <vector>
#include <string>

class Openbox;
class ScreenInfo;

class XAtom {
  typedef std::vector<Window> SupportWindows;
  
  Display              *_display;
  // windows used to specify support for NETWM
  SupportWindows        _support_windows; 

  Atom 
#ifdef    HAVE_GETPID
    openbox_pid,
#endif // HAVE_GETPID

    // window hints
    wm_colormap_windows,
    wm_protocols,
    wm_state,
    wm_delete_window,
    wm_take_focus,
    wm_change_state,
    motif_wm_hints,
    openbox_attributes,
    openbox_change_attributes,
    openbox_hints,

    // blackbox-protocol atoms (wm -> client)
    openbox_structure_messages,
    openbox_notify_startup,
    openbox_notify_window_add,
    openbox_notify_window_del,
    openbox_notify_window_focus,
    openbox_notify_current_workspace,
    openbox_notify_workspace_count,
    openbox_notify_window_raise,
    openbox_notify_window_lower,
    // blackbox-protocol atoms (client -> wm)
    openbox_change_workspace,
    openbox_change_window_focus,
    openbox_cycle_window_focus,

    // NETWM atoms
    // root window properties
    net_supported,
    net_client_list,
    net_client_list_stacking,
    net_number_of_desktops,
    net_desktop_geometry,
    net_desktop_viewport,
    net_current_desktop,
    net_desktop_names,
    net_active_window,
    net_workarea,
    net_supporting_wm_check,
    net_virtual_roots,
    // root window messages
    net_close_window,
    net_wm_moveresize,
    // application window properties
    net_properties,
    net_wm_name,
    net_wm_desktop,
    net_wm_window_type,
    net_wm_state,
    net_wm_strut,
    net_wm_icon_geometry,
    net_wm_icon,
    net_wm_pid,
    net_wm_handled_icons,
    // application protocols
    net_wm_ping;

  Atom getAtom(const char *name) const;
  void setSupported(const ScreenInfo *screen);

  void setValue(Window win, Atom atom, Atom type, unsigned char *data,
                int size, int nelements, bool append) const;
  bool getValue(Window win, Atom atom, Atom type, unsigned long *nelements,
                unsigned char **value, int size) const;

  // no copying!!
  XAtom(const XAtom &);
  XAtom& operator=(const XAtom&);

public:
  XAtom(Openbox &ob);
  virtual ~XAtom();

  void setCardValue(Window win, Atom atom, long value) const; // 32-bit CARDINAL
  void setAtomValue(Window win, Atom atom, Atom value) const;
  void setWindowValue(Window win, Atom atom, Window value) const;
  void setPixmapValue(Window win, Atom atom, Pixmap value) const;
  void setStringValue(Window win, Atom atom, const std::string &value) const;
  
  void addCardValue(Window win, Atom atom, long value) const; // 32-bit CARDINAL
  void addAtomValue(Window win, Atom atom, Atom value) const;
  void addWindowValue(Window win, Atom atom, Window value) const;
  void addPixmapValue(Window win, Atom atom, Pixmap value) const;
  void addStringValue(Window win, Atom atom, const std::string &value) const;

  // the 'value' is allocated inside the function and
  // delete [] value needs to be called when you are done with it.
  // teh 'value' array returned is null terminated, and has 'nelements'
  // elements in it plus the null.
  bool getCardValue(Window win, Atom atom, unsigned long *nelements,
                long **value) const; // 32-bit CARDINAL
  bool getAtomValue(Window win, Atom atom, unsigned long *nelements,
                Atom **value) const;
  bool getWindowValue(Window win, Atom atom, unsigned long *nelements,
                Window **value) const;
  bool getPixmapValue(Window win, Atom atom, unsigned long *nelements,
                Pixmap **value) const;
  bool getStringValue(Window win, Atom atom, std::string &value) const;
  
  void eraseValue(Window win, Atom atom) const;
  
#ifdef    HAVE_GETPID
  inline Atom openboxPid() const { return openbox_pid; }
#endif // HAVE_GETPID

  inline Atom wmChangeState() const { return wm_change_state; }
  inline Atom wmState() const { return wm_state; }
  inline Atom wmDelete() const { return wm_delete_window; }
  inline Atom wmProtocols() const { return wm_protocols; }
  inline Atom wmTakeFocus() const { return wm_take_focus; }
  inline Atom wmColormap() const { return wm_colormap_windows; }
  inline Atom motifWMHints() const { return motif_wm_hints; }

  // this atom is for normal app->WM hints about decorations, stacking,
  // starting workspace etc...
  inline Atom openboxHints() const { return openbox_hints;}

  // these atoms are for normal app->WM interaction beyond the scope of the
  // ICCCM...
  inline Atom openboxAttributes() const { return openbox_attributes; }
  inline Atom openboxChangeAttributes() const
  { return openbox_change_attributes; }

  // these atoms are for window->WM interaction, with more control and
  // information on window "structure"... common examples are
  // notifying apps when windows are raised/lowered... when the user changes
  // workspaces... i.e. "pager talk"
  inline Atom openboxStructureMessages() const
  { return openbox_structure_messages; }

  inline Atom openboxNotifyStartup() const
  { return openbox_notify_startup; }
  inline Atom openboxNotifyWindowAdd() const
  { return openbox_notify_window_add; }
  inline Atom openboxNotifyWindowDel() const
  { return openbox_notify_window_del; }
  inline Atom openboxNotifyWindowFocus() const
  { return openbox_notify_window_focus; }
  inline Atom openboxNotifyCurrentWorkspace() const
  { return openbox_notify_current_workspace; }
  inline Atom openboxNotifyWorkspaceCount() const
  { return openbox_notify_workspace_count; }
  inline Atom openboxNotifyWindowRaise() const
  { return openbox_notify_window_raise; }
  inline Atom openboxNotifyWindowLower() const
  { return openbox_notify_window_lower; }

  // atoms to change that request changes to the desktop environment during
  // runtime... these messages can be sent by any client... as the sending
  // client window id is not included in the ClientMessage event...
  inline Atom openboxChangeWorkspace() const
  { return openbox_change_workspace; }
  inline Atom openboxChangeWindowFocus() const
  { return openbox_change_window_focus; }
  inline Atom openboxCycleWindowFocus() const
  { return openbox_cycle_window_focus; }

  // root window properties
  inline Atom netClientList() const { return net_client_list; }
  inline Atom netClientListStacking() const { return net_client_list_stacking; }
  inline Atom netNumberOfDesktops() const { return net_number_of_desktops; }
  inline Atom netDesktopGeometry() const { return net_desktop_geometry; }
  inline Atom netDesktopViewport() const { return net_desktop_viewport; }
  inline Atom netCurrentDesktop() const { return net_current_desktop; }
  inline Atom netDesktopNames() const { return net_desktop_names; }
  inline Atom netActiveWindow() const { return net_active_window; }
  inline Atom netWorkarea() const { return net_workarea; }
  inline Atom netSupportingWMCheck() const { return net_supporting_wm_check; }
  inline Atom netVirtualRoots() const { return net_virtual_roots; }

  // root window messages
  inline Atom netCloseWindow() const { return net_close_window; }
  inline Atom netWMMoveResize() const { return net_wm_moveresize; }

  // application window properties
  inline Atom netProperties() const { return net_properties; }
  inline Atom netWMName() const { return net_wm_name; }
  inline Atom netWMDesktop() const { return net_wm_desktop; }
  inline Atom netWMWindowType() const { return net_wm_window_type; }
  inline Atom netWMState() const { return net_wm_state; }
  inline Atom netWMStrut() const { return net_wm_strut; }
  inline Atom netWMIconGeometry() const { return net_wm_icon_geometry; }
  inline Atom netWMIcon() const { return net_wm_icon; }
  inline Atom netWMPid() const { return net_wm_pid; }
  inline Atom netWMHandledIcons() const { return net_wm_handled_icons; }

  // application protocols
  inline Atom netWMPing() const { return net_wm_ping; }
};

#endif // __XAtom_h
