// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __screen_hh
#define   __screen_hh

/*! @file screen.hh
  @brief Screen manages a single screen
*/

extern "C" {
#include <X11/Xlib.h>
}

#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/screeninfo.hh"
#include "otk/eventhandler.hh"
#include "otk/property.hh"
#include "otk/ustring.hh"

#include <string>
#include <list>

namespace ob {

class Client;

struct DesktopLayout {
  enum Corner { TopLeft, TopRight, BottomRight, BottomLeft };
  enum Direction { Horizontal, Vertical };

  Direction orientation;
  Corner start_corner;
  unsigned int rows;
  unsigned int columns;
};

//! Manages a single screen
/*!
*/
class Screen : public otk::EventHandler {
public:
  //! Holds a list of otk::Strut objects
  typedef std::vector<otk::Strut> StrutList;
  //! Holds a list of otk::Rect objects
  typedef std::vector<otk::Rect> RectList;

  static const unsigned long event_mask = ColormapChangeMask |
                                          EnterWindowMask |
                                          LeaveWindowMask |
                                          PropertyChangeMask |
                                          SubstructureNotifyMask |
                                          SubstructureRedirectMask |
                                          ButtonPressMask |
                                          ButtonReleaseMask;

  //! Holds a list of Clients
  typedef std::list<Client*> ClientList;
  //! All managed clients on the screen (in order of being mapped)
  ClientList clients;
  
private:
  //! Was %Openbox able to manage the screen?
  bool _managed;

  //! The number of the screen on the X server
  int _number;

  //! Information about this screen
  const otk::ScreenInfo *_info;
  
  //! Area usable for placement etc (total - struts), one per desktop,
  //! plus one extra for windows on all desktops
  RectList _area;

  //! Combined strut from all of the clients' struts, one per desktop,
  //! plus one extra for windows on all desktops
  StrutList _struts;

  //!  An offscreen window which gets focus when nothing else has it
  Window _focuswindow;

  //! An offscreen window which shows that a NETWM compliant window manager is
  //! running
  Window _supportwindow;

  //! A list of all managed clients on the screen, in their stacking order
  ClientList _stacking;

  //! The desktop currently being displayed
  unsigned int _desktop;

  //! The number of desktops
  unsigned int _num_desktops;

  //! The names of all desktops
  otk::Property::StringVect _desktop_names;

  DesktopLayout _layout;

  //! Calculate the Screen::_area member
  void calcArea();
  //! Set the list of supported NETWM atoms on the root window
  void changeSupportedAtoms();
  //! Set the client list on the root window
  /*!
    Sets the _NET_CLIENT_LIST root window property.<br>
    Also calls Screen::updateStackingList.
  */
  void changeClientList();
  //! Set the client stacking list on the root window
  /*!
    Set the _NET_CLIENT_LIST_STACKING root window property.
  */
  void changeStackingList();
  //! Set the work area hint on the root window
  /*!
    Set the _NET_WORKAREA root window property.
  */
  void changeWorkArea();

  //! Get desktop names from the root window property
  void updateDesktopNames();

  //! Gets the layout of the desktops from the root window property
  void updateDesktopLayout();

  //! Changes to the specified desktop, displaying windows on it and hiding
  //! windows on the others.
  /*!
    @param desktop The number of the desktop to switch to (starts from 0).
    If the desktop is out of valid range, it is ignored.
  */
  void changeDesktop(unsigned int desktop);

  //! Changes the number of desktops.
  /*!
    @param num The number of desktops that should exist. This value must be
               greater than 0 or it will be ignored.
  */
  void changeNumDesktops(unsigned int num);

public:
#ifndef SWIG
  //! Constructs a new Screen object
  Screen(int screen);
  //! Destroys the Screen object
  virtual ~Screen();
#endif

  inline int number() const { return _number; }
  
  //! Returns if the screen was successfully managed
  /*!
    If this is false, then the screen should be deleted and should NOT be
    used.
  */
  inline bool managed() const { return _managed; }
  //!  An offscreen window which gets focus when nothing else has it
  inline Window focuswindow() const { return _focuswindow; }
  //! Returns the desktop being displayed
  inline unsigned int desktop() const { return _desktop; }
  //! Returns the number of desktops
  inline unsigned int numDesktops() const { return _num_desktops; }

  //! Returns the area of the screen not reserved by applications' Struts
  /*!
    @param desktop The desktop number of the area to retrieve for. A value of
                   0xffffffff will return an area that combines all struts
                   on all desktops.
  */
  const otk::Rect& area(unsigned int desktop) const;

  const DesktopLayout& desktopLayout() const { return _layout; }

  //! Update's the screen's combined strut of all the clients.
  /*!
    Clients should call this whenever they change their strut.
  */
  void updateStruts();

  //! Manage any pre-existing windows on the screen
  void manageExisting();
  //! Manage a client window
  /*!
    This gives the window a frame, reparents it, selects events on it, etc.
  */
  void manageWindow(Window window);
  //! Unmanage a client
  /*!
    This removes the window's frame, reparents it to root, unselects events on
    it, etc.
    @param client The client to unmanage
  */
  void unmanageWindow(Client *client);

  //! Raises a client window above all others in its stacking layer
  /*!
    raiseWindow has a couple of constraints that lowerWindow does not.<br>
    1) raiseWindow can be called after changing a Client's stack layer, and
       the list will be reorganized properly.<br>
    2) raiseWindow guarantees that XRestackWindows() will <i>always</i> be
       called for the specified client.
  */
  void raiseWindow(Client *client);

  //! Lowers a client window below all others in its stacking layer
  void lowerWindow(Client *client);

  //! Sets the name of a desktop by changing the root window property
  /*!
    @param i The index of the desktop to set the name for (starts at 0)
    @param name The name to set for the desktop
    If the index is too large, it is simply ignored.
  */
  void setDesktopName(unsigned int i, const otk::ustring &name);

  void installColormap(bool install) const;

  virtual void propertyHandler(const XPropertyEvent &e);
  virtual void clientMessageHandler(const XClientMessageEvent &e);
  virtual void mapRequestHandler(const XMapRequestEvent &e);
};

}

#endif// __screen_hh
