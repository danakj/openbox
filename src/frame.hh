// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __frame_hh
#define   __frame_hh

/*! @file frame.hh
*/

extern "C" {
#include <X11/Xlib.h>
}

#include "client.hh"
#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/screeninfo.hh"
#include "otk/style.hh"
#include "otk/widget.hh"
#include "otk/button.hh"
#include "otk/focuswidget.hh"
#include "otk/focuslabel.hh"

#include <string>

namespace ob {

//! Holds and decorates a frame around an OBClient (client window)
/*!
*/
class OBFrame : public otk::OtkWidget {
private:
  OBClient *_client;
  const otk::ScreenInfo *_screen;

  //! The style to use for size and display the decorations
  otk::Style *_style;

  //! The size of the frame on each side of the client window
  otk::Strut _size;

  //! The size of the frame on each side of the client window inside the border
  otk::Strut _innersize;

  // decoration windows
  otk::OtkFocusWidget _plate;   // sits entirely under the client window
  otk::OtkFocusWidget _titlebar;
  otk::OtkButton      _button_close;
  otk::OtkButton      _button_iconify;
  otk::OtkButton      _button_max;
  otk::OtkButton      _button_stick;
  otk::OtkFocusLabel  _label;
  otk::OtkFocusWidget _handle;
  otk::OtkButton      _grip_left;
  otk::OtkButton      _grip_right;

  //! The decorations to display on the window.
  /*!
    This is by default the same value as in the OBClient::decorations, but it
    is duplicated here so that it can be overridden per-window by the user.
  */
  OBClient::DecorationFlags _decorations;

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
  OBFrame(OBClient *client, otk::Style *style);
  //! Destroys the OBFrame object
  virtual ~OBFrame();

  //! Set the style to decorate the frame with
  virtual void setStyle(otk::Style *style);

  //! Update the frame to match the client
  void adjust();
  //! Shape the frame window to the client window
  void adjustShape();

  //! Applies gravity for the client's gravity, moving the frame to the
  //! appropriate place
  void applyGravity();

  //! Reversely applies gravity for the client's gravity, moving the frame so
  //! that the client is in its pre-gravity position
  void restoreGravity();
};

}

#endif // __frame_hh
