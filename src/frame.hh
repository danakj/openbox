// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __frame_hh
#define   __frame_hh

/*! @file frame.hh
*/

extern "C" {
#include <X11/Xlib.h>
}

#include <string>

#include "client.hh"
#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/screeninfo.hh"
#include "otk/style.hh"

namespace ob {

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

  // decoration windows
  Window _titlebar;
  otk::Rect _titlebar_area;
  
  Window _button_close;
  otk::Rect _button_close_area;
  
  Window _button_iconify;
  otk::Rect _button_iconify_area;

  Window _button_max;
  otk::Rect _button_max_area;

  Window _button_stick;
  otk::Rect _button_stick_area;

  Window _label;
  otk::Rect _label_area;

  Window _handle;
  otk::Rect _handle_area;

  Window _grip_left;
  otk::Rect _grip_left_area;

  Window _grip_right;
  otk::Rect _grip_right_area;

  //! The decorations to display on the window.
  /*!
    This is by default the same value as in the OBClient::decorations, but it
    is duplicated here so that it can be overridden per-window by the user.
  */
  OBClient::DecorationFlags _decorations;

  //! Creates the base frame window
  Window createFrame();
  //! Creates a child frame decoration element window
  Window createChild(Window parent, Cursor cursor);

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

  //! Update the frame to match the client
  void update();
  //! Shape the frame window to the client window
  void updateShape(); 
  
  //! Returns the frame's most-parent window, which is a child of the root
  //! window
  inline Window window() const { return _window; }

  inline Window titlebar() const { return _titlebar; }
  inline Window label() const { return _label; }
  inline Window buttonIconify() const { return _button_iconify; }
  inline Window buttonMax() const { return _button_max; }
  inline Window buttonStick() const { return _button_stick; }
  inline Window buttonClose() const { return _button_close; }
  inline Window handle() const { return _handle; }
  inline Window gripLeft() const { return _grip_left; }
  inline Window gripRight() const { return _grip_right; }
};

}

#endif // __frame_hh
