// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __frame_hh
#define   __frame_hh

/*! @file frame.hh
*/

extern "C" {
#include <X11/Xlib.h>
}

#include <string>

#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/screeninfo.hh"
#include "otk/style.hh"

namespace ob {

class OBClient;

//! Holds and decorates a frame around an OBClient (client window)
/*!
*/
class OBFrame {
private:
  const OBClient *_client;
  const otk::ScreenInfo *_screen;

  //! The style to use for size and display the decorations
  const otk::Style *_style;

  //! The window id of the base frame window
  Window _window;
  //! The size of the frame on each side of the client window
  otk::Strut _size;

  //! Creates the base frame window
  Window createFrame();

  //! Reparents the client window from the root window onto the frame
  void grabClient();
  //! Reparents the client window back to the root window
  /*!
    @param remap Re-map the client window when we're done reparenting?
  */
  void releaseClient(bool remap);

public:
  //! Constructs an OBFrame object, and reparents the client to itself
  /*!
    @param client The client window which will be decorated by the new OBFrame
    @param style The style to use to decorate the frame
  */
  OBFrame(const OBClient *client, const otk::Style *style);
  //! Destroys the OBFrame object
  virtual ~OBFrame();

  //! Load a style to decorate the frame with
  void loadStyle(const otk::Style *style);

  //! Size the frame to the client
  void resize();
  //! Shape the frame window to the client window
  void shape(); 
  
  //! Returns the frame's most-parent window, which is a child of the root
  //! window
  inline Window window() const { return _window; }
};

}

#endif // __frame_hh
