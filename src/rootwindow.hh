// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __rootwindow_hh
#define   __rootwindow_hh

/*! @file rootwindow.hh
  @brief The OBClient class maintains the state of a client window by handling
  property changes on the window and some client messages
*/

extern "C" {
#include <X11/Xlib.h>

#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE
}

#include <string>
#include <vector>

#include "otk/screeninfo.hh"
#include "otk/eventhandler.hh"
#include "otk/property.hh"

namespace ob {

//! Maintains the state of a root window's properties.
/*!
  OBRootWindow maintains the state of a root window. The state consists of the
  hints that the wm sets on the window, such as the number of desktops,
  gravity.
  <p>
  OBRootWindow also manages client messages for the root window.
*/
class OBRootWindow : public otk::OtkEventHandler {
private:
  //! Information about this screen
  const otk::ScreenInfo *_info;

  //! The names of all desktops
  otk::OBProperty::StringVect _names;

  //! Get desktop names from the 
  void updateDesktopNames();

public:
  //! Constructs a new OBRootWindow for a screen
  /*!
    @param screen The screen whose root window to wrap
  */
  OBRootWindow(int screen);
  //! Destroys the OBRootWindow object
  virtual ~OBRootWindow();

  //! Sets the name of a desktop
  /*!
    @param i The index of the desktop to set the name for (base 0)
    @param name The name to set for the desktop
  */
  void setDesktopName(int i, const std::string &name);

  virtual void propertyHandler(const XPropertyEvent &e);
  virtual void clientMessageHandler(const XClientMessageEvent &e);
  virtual void mapRequestHandler(const XMapRequestEvent &);
};

}

#endif // __client_hh
