// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __screen_hh
#define   __screen_hh

/*! @file screen.hh
  @brief OBScreen manages a single screen
*/

extern "C" {
#include <X11/Xlib.h>
}

#include "otk/image.hh"
#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/point.hh"

#include <string>

namespace ob {

class OBClient;

//! Manages a single screen
/*!
*/
class OBScreen {
public:
  //! Holds a list of OBClient objects
  typedef std::vector<OBClient*> ClientList;
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

private:
  //! Was %Openbox able to manage the screen?
  bool _managed;

  //! The number of the screen on the X server
  int _number;

  //! Information about this screen
  const otk::ScreenInfo *_info;
  
  //! The Image Control used for rendering on the screen
  otk::BImageControl *_image_control;

  //! Is the root colormap currently installed?
  bool _root_cmap_installed;

  //! The dimentions of the screen
  otk::Point _size;

  //! All managed clients on the screen
  ClientList _clients;

  //! Area usable for placement etc (total - struts)
  otk::Rect _area;

  //! Areas of the screen reserved by applications
  StrutList _struts;


  //! Manage any pre-existing windows on the screen
  void manageExisting();
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
  //! Constructs a new OBScreen object
  OBScreen(int screen);
  //! Destroys the OBScreen object
  virtual ~OBScreen();

  //! Returns the Image Control used for rendering on the screen
  inline otk::BImageControl *imageControl() { return _image_control; }
  //! Returns the dimentions of the screen
  inline const otk::Point &size() const { return _size; }
  //! Returns the area of the screen not reserved by applications' Struts
  inline const otk::Rect &area() const { return _area; }

  //! Adds a window's strut to the screen's list of reserved spaces
  void addStrut(otk::Strut *strut);
  //! Removes a window's strut from the screen's list of reserved spaces
  void removeStrut(otk::Strut *strut);
  
};

}

#endif// __screen_hh
