// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __screen_hh
#define   __screen_hh

/*! @file screen.hh
  @brief OBScreen manages a single screen
*/

extern "C" {
#include <X11/Xlib.h>
}

#include "rootwindow.hh"
#include "otk/image.hh"
#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/style.hh"
#include "otk/configuration.hh" // TEMPORARY

#include <string>

namespace ob {

class OBClient;
class OBRootWindow;

//! Manages a single screen
/*!
*/
class OBScreen {
public:
  //! Holds a list of OBClient objects
  typedef std::list<OBClient*> ClientList;
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

  //! All managed clients on the screen
  ClientList clients;
  
private:
  //! Was %Openbox able to manage the screen?
  bool _managed;

  //! The number of the screen on the X server
  int _number;

  //! Information about this screen
  const otk::ScreenInfo *_info;
  
  //! The Image Control used for rendering on the screen
  otk::BImageControl *_image_control;

  //! The style with which to render on the screen
  otk::Style _style;

  //! The screen's root window
  OBRootWindow _root;
  
  //! Is the root colormap currently installed?
  bool _root_cmap_installed;

  //! Area usable for placement etc (total - struts)
  otk::Rect _area;

  //! Areas of the screen reserved by applications
  StrutList _struts;

  //!  An offscreen window which gets focus when nothing else has it
  Window _focuswindow;
  

  //! Calculate the OBScreen::_area member
  void calcArea();
  //! Set the client list on the root window
  /*!
    Sets the _NET_CLIENT_LIST root window property.<br>
    Also calls OBScreen::updateStackingList.
  */
  void setClientList();
  //! Set the client stacking list on the root window
  /*!
    Set the _NET_CLIENT_LIST_STACKING root window property.
  */
  void setStackingList();
  //! Set the work area hint on the root window
  /*!
    Set the _NET_WORKAREA root window property.
  */
  void setWorkArea();
  
public:
#ifndef SWIG
  //! Constructs a new OBScreen object
  OBScreen(int screen);
  //! Destroys the OBScreen object
  virtual ~OBScreen();
#endif

  //! Returns if the screen was successfully managed
  /*!
    If this is false, then the screen should be deleted and should NOT be
    used.
  */
  inline bool managed() const { return _managed; }
  //! Returns the Image Control used for rendering on the screen
  inline otk::BImageControl *imageControl() { return _image_control; }
  //! Returns the area of the screen not reserved by applications' Struts
  inline const otk::Rect &area() const { return _area; }
  //! Returns the style in use on the screen
  inline const otk::Style *style() const { return &_style; }
  //!  An offscreen window which gets focus when nothing else has it
  inline Window focuswindow() const { return _focuswindow; }

  //! Adds a window's strut to the screen's list of reserved spaces
  void addStrut(otk::Strut *strut);
  //! Removes a window's strut from the screen's list of reserved spaces
  void removeStrut(otk::Strut *strut);

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
  */
  void unmanageWindow(OBClient *client);
};

}

#endif// __screen_hh
