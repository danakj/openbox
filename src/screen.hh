// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __screen_hh
#define   __screen_hh

/*! @file screen.hh
  @brief Screen manages a single screen
*/

extern "C" {
#include <X11/Xlib.h>
}

#include "client.hh"
#include "widgetbase.hh"
#include "otk/image.hh"
#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/style.hh"
#include "otk/screeninfo.hh"
#include "otk/eventhandler.hh"
#include "otk/property.hh"
#include "otk/ustring.hh"

#include <string>
#include <list>

namespace ob {

class Client;
class RootWindow;

//! Manages a single screen
/*!
*/
class Screen : public otk::EventHandler, public WidgetBase {
public:
  //! Holds a list of otk::Strut objects
  typedef std::list<otk::Strut*> StrutList;

  static const unsigned long event_mask = ColormapChangeMask |
                                          EnterWindowMask |
                                          LeaveWindowMask |
                                          PropertyChangeMask |
                                          SubstructureNotifyMask |
                                          SubstructureRedirectMask |
                                          ButtonPressMask |
                                          ButtonReleaseMask;

  //! All managed clients on the screen (in order of being mapped)
  Client::List clients;
  
private:
  //! Was %Openbox able to manage the screen?
  bool _managed;

  //! The number of the screen on the X server
  int _number;

  //! Information about this screen
  const otk::ScreenInfo *_info;
  
  //! The Image Control used for rendering on the screen
  otk::ImageControl *_image_control;

  //! The style with which to render on the screen
  otk::Style _style;

  //! Is the root colormap currently installed?
  bool _root_cmap_installed;

  //! Area usable for placement etc (total - struts)
  otk::Rect _area;

  //! Combined strut from all of the clients' struts
  otk::Strut _strut;

  //!  An offscreen window which gets focus when nothing else has it
  Window _focuswindow;

  //! An offscreen window which shows that a NETWM compliant window manager is
  //! running
  Window _supportwindow;

  //! A list of all managed clients on the screen, in their stacking order
  Client::List _stacking;

  //! The desktop currently being displayed
  long _desktop;

  //! The number of desktops
  long _num_desktops;

  //! The names of all desktops
  otk::Property::StringVect _desktop_names;

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

  //! Changes to the specified desktop, displaying windows on it and hiding
  //! windows on the others.
  /*!
    @param desktop The number of the desktop to switch to (starts from 0).
    If the desktop is out of valid range, it is ignored.
  */
  void changeDesktop(long desktop);

  //! Changes the number of desktops.
  /*!
    @param num The number of desktops that should exist. This value must be
               greater than 0 or it will be ignored.
  */
  void changeNumDesktops(long num);

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
  //! Returns the Image Control used for rendering on the screen
  inline otk::ImageControl *imageControl() { return _image_control; }
  //! Returns the area of the screen not reserved by applications' Struts
  inline const otk::Rect &area() const { return _area; }
  //! Returns the style in use on the screen
  inline const otk::Style *style() const { return &_style; }
  //!  An offscreen window which gets focus when nothing else has it
  inline Window focuswindow() const { return _focuswindow; }
  //! Returns the desktop being displayed
  inline long desktop() const { return _desktop; }
  //! Returns the number of desktops
  inline long numDesktops() const { return _num_desktops; }

  //! Update's the screen's combined strut of all the clients.
  /*!
    Clients should call this whenever they change their strut.
  */
  void updateStrut();

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

  //! Raises/Lowers a client window above/below all others in its stacking
  //! layer
  void restack(bool raise, Client *client);

  //! Sets the name of a desktop by changing the root window property
  /*!
    @param i The index of the desktop to set the name for (starts at 0)
    @param name The name to set for the desktop
    If the index is too large, it is simply ignored.
  */
  void setDesktopName(long i, const otk::ustring &name);

  virtual void propertyHandler(const XPropertyEvent &e);
  virtual void clientMessageHandler(const XClientMessageEvent &e);
  virtual void mapRequestHandler(const XMapRequestEvent &e);
};

}

#endif// __screen_hh
