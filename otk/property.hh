// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __atom_hh
#define   __atom_hh

/*! @file property.hh
  @brief Provides access to window properties
*/

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <assert.h>
}

#include <vector>
#include <string>

#include "screeninfo.hh"

namespace otk {

//! Provides easy access to window properties.
class OBProperty {
public:
  //! The atoms on the X server which this class will cache
  enum Atoms {
    // types
    Atom_Cardinal, //!< The atom which represents the Cardinal data type
    Atom_Window,   //!< The atom which represents window ids
    Atom_Pixmap,   //!< The atom which represents pixmap ids
    Atom_Atom,     //!< The atom which represents atom values
    Atom_String,   //!< The atom which represents ascii strings
    Atom_Utf8,     //!< The atom which represents utf8-encoded strings
    
#ifdef    HAVE_GETPID
    blackbox_pid,
#endif // HAVE_GETPID

    // window hints
    wm_colormap_windows,
    wm_protocols,
    wm_state,
    wm_delete_window,
    wm_take_focus,
    wm_change_state,
    wm_name,
    wm_icon_name,
    wm_class,
    motif_wm_hints,
    blackbox_attributes,
    blackbox_change_attributes,
    blackbox_hints,

    // blackbox-protocol atoms (wm -> client)
    blackbox_structure_messages,
    blackbox_notify_startup,
    blackbox_notify_window_add,
    blackbox_notify_window_del,
    blackbox_notify_window_focus,
    blackbox_notify_current_workspace,
    blackbox_notify_workspace_count,
    blackbox_notify_window_raise,
    blackbox_notify_window_lower,
    // blackbox-protocol atoms (client -> wm)
    blackbox_change_workspace,
    blackbox_change_window_focus,
    blackbox_cycle_window_focus,

    openbox_show_root_menu,
    openbox_show_workspace_menu,

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
//    net_virtual_roots,
    // root window messages
    net_close_window,
    net_wm_moveresize,
    // application window properties
//    net_properties,
    net_wm_name,
    net_wm_visible_name,
    net_wm_icon_name,
    net_wm_visible_icon_name,
    net_wm_desktop,
    net_wm_window_type,
    net_wm_state,
    net_wm_strut,
//  net_wm_icon_geometry,
//  net_wm_icon,
//  net_wm_pid,
//  net_wm_handled_icons,
    net_wm_allowed_actions,
    // application protocols
//    net_wm_ping,

    net_wm_window_type_desktop,
    net_wm_window_type_dock,
    net_wm_window_type_toolbar,
    net_wm_window_type_menu,
    net_wm_window_type_utility,
    net_wm_window_type_splash,
    net_wm_window_type_dialog,
    net_wm_window_type_normal,

    net_wm_moveresize_size_topleft,
    net_wm_moveresize_size_topright,
    net_wm_moveresize_size_bottomleft,
    net_wm_moveresize_size_bottomright,
    net_wm_moveresize_move,

    net_wm_action_move,
    net_wm_action_resize,
    net_wm_action_shade,
    net_wm_action_maximize_horz,
    net_wm_action_maximize_vert,
    net_wm_action_change_desktop,
    net_wm_action_close,

    net_wm_state_modal,
    net_wm_state_maximized_vert,
    net_wm_state_maximized_horz,
    net_wm_state_shaded,
    net_wm_state_skip_taskbar,
    net_wm_state_skip_pager,
    net_wm_state_hidden,
    net_wm_state_fullscreen,

    kde_net_system_tray_windows,
    kde_net_wm_system_tray_window_for,
    kde_net_wm_window_type_override,
 
    // constant for how many atoms exist in the enumerator
    NUM_ATOMS
  };

  //! The possible types/encodings of strings
  enum StringType {
    ascii, //!< Standard 8-bit ascii string
    utf8,  //!< Utf8-encoded string
    NUM_STRING_TYPE
  };

private:
  //! The value of all atoms on the X server that exist in the
  //! OBProperty::Atoms enum
  Atom                  _atoms[NUM_ATOMS];

  //! Gets the value of an Atom from the X server, creating it if nessesary
  Atom create(const char *name) const;

  //! Sets a property on a window
  void set(Window win, Atom atom, Atom type, unsigned char *data,
           int size, int nelements, bool append) const;
  //! Gets a property's value from a window
  bool get(Window win, Atom atom, Atom type,
           unsigned long *nelements, unsigned char **value,
           int size) const;

public:
  //! A list of strings
  typedef std::vector<std::string> StringVect;

  //! Constructs a new OBAtom object
  /*!
    CAUTION: This constructor uses OBDisplay::display, so ensure that it is
    initialized before initializing this class!
  */
  OBProperty();
  //! Destroys the OBAtom object
  virtual ~OBProperty();

  //! Sets a property on a window to a new value
  /*!
    @param win The window id of the window on which to set the property's value
    @param atom A member of the OBProperty::Atoms enum that specifies which
                property to set
    @param type A member of the OBProperty::Atoms enum that specifies the type
                of the property to set
    @param value The value to set the property to
  */
  void set(Window win, Atoms atom, Atoms type, unsigned long value) const;
  //! Sets a property on a window to a new value
  /*!
    @param win The window id of the window on which to set the property's value
    @param atom A member of the OBProperty::Atoms enum that specifies which
                property to set
    @param type A member of the OBProperty::Atoms enum that specifies the type
                of the property to set
    @param value Any array of values to set the property to. The array must
                 contain <i>elements</i> number of elements
    @param elements The number of elements in the <i>value</i> array
  */
  void set(Window win, Atoms atom, Atoms type,
           unsigned long value[], int elements) const;
  //! Sets a property on a window to a new value
  /*!
    @param win The window id of the window on which to set the property's value
    @param atom A member of the OBProperty::Atoms enum that specifies which
                property to set
    @param type A member of the OBProperty::StringType enum that specifies the
                type of the string the property is being set to
    @param value The string to set the property to
  */
  void set(Window win, Atoms atom, StringType type,
           const std::string &value) const;
  //! Sets a property on a window to a new value
  /*!
    @param win The window id of the window on which to set the property's value
    @param atom A member of the OBProperty::Atoms enum that specifies which
                property to set
    @param type A member of the OBProperty::StringType enum that specifies the
                type of the string the property is being set to
    @param strings A list of strings to set the property to
  */
  void set(Window win, Atoms atom, StringType type,
           const StringVect &strings) const;

  //! Gets the value of a property on a window
  /*!
    @param win The window id of the window to get the property value from
    @param atom A member of the OBProperty::Atoms enum that specifies which
                property to retrieve
    @param type A member of the OBProperty::Atoms enum that specifies the type
                of the property to retrieve
    @param nelements The maximum number of elements to retrieve from the
                     property (assuming it has more than 1 value in it). To
                     retrieve all possible elements, use "(unsigned) -1".<br>
                     When the function returns, if it returns true, this will
                     contain the actual number of elements retrieved.<br>
    @param value If the function returns true, then this contains an array of
                 retrieved values for the property.<br>
                 The <i>value</i> is allocated inside the function and
                 <b>delete[]</b> value needs to be called when you are done
                 with it.<br>
                 The <i>value</i> array returned is null terminated, and has
                 <i>nelements</i> elements in it plus the terminating null.
    @return true if retrieval of the specified property with the specified
            type was successful; otherwise, false
  */
  bool get(Window win, Atoms atom, Atoms type,
           unsigned long *nelements, unsigned long **value) const;
  //! Gets a single element from the value of a property on a window
  /*!
    @param win The window id of the window to get the property value from
    @param atom A member of the OBProperty::Atoms enum that specifies which
                property to retrieve
    @param type A member of the OBProperty::Atoms enum that specifies the type
                of the property to retrieve
    @param value If the function returns true, then this contains the first
                 (and possibly only) element in the value of the specified
                 property.
    @return true if retrieval of the specified property with the specified
            type was successful; otherwise, false
  */
  bool get(Window win, Atoms atom, Atoms type, unsigned long *value) const;
  //! Gets a single string from the value of a property on a window
  /*!
    @param win The window id of the window to get the property value from
    @param atom A member of the OBProperty::Atoms enum that specifies which
                property to retrieve
    @param type A member of the OBProperty::StringType enum that specifies the
                type of the string property to retrieve
    @param value If the function returns true, then this contains the first
                 (and possibly only) string in the value of the specified
                 property.
    @return true if retrieval of the specified property with the specified
            type was successful; otherwise, false
  */
  bool get(Window win, Atoms atom, StringType type, std::string *value) const;
  //! Gets strings from the value of a property on a window
  /*!
    @param win The window id of the window to get the property value from
    @param atom A member of the OBProperty::Atoms enum that specifies which
                property to retrieve
    @param type A member of the OBProperty::StringType enum that specifies the
                type of the string property to retrieve
    @param nelements The maximum number of strings to retrieve from the
                     property (assuming it has more than 1 string in it). To
                     retrieve all possible strings, use "(unsigned) -1".<br>
                     When the function returns, if it returns true, this will
                     contain the actual number of strings retrieved.<br>
    @param value If the function returns true, then this contains all of the
                 strings retrieved from the property's value.
    @return true if retrieval of the specified property with the specified
            type was successful; otherwise, false
  */
  bool get(Window win, Atoms atom, StringType type,
           unsigned long *nelements, StringVect *strings) const;

  //! Removes a property from a window
  /*!
    @param The window id of the window to remove the property from
    @param A member of the OBProperty::Atoms enum that specifies which property
           to remove from the window
  */
  void erase(Window win, Atoms atom) const;

  //! Gets the value of an atom on the X server
  /*!
    @param a A member of the OBProperty::Atoms enum that specifies which Atom's
             value to return
    @return The value of the specified Atom
  */
  inline Atom atom(Atoms a) {
    assert(a >= 0 && a < NUM_ATOMS); Atom ret = _atoms[a]; assert(ret != 0);
    return ret;
  }
};

}

#endif // __atom_hh
