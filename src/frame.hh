// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __frame_hh
#define   __frame_hh

/*! @file frame.hh
*/

extern "C" {
#include <X11/Xlib.h>
}

#include "client.hh"
#include "backgroundwidget.hh"
#include "labelwidget.hh"
#include "buttonwidget.hh"
#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/screeninfo.hh"
#include "otk/style.hh"
#include "otk/widget.hh"

#include <string>

namespace ob {

//! Holds and decorates a frame around an OBClient (client window)
/*!
  The frame is responsible for calling XSelectInput on the client window's new
  parent with the SubstructureRedirectMask so that structure events for the
  client are sent to the window manager.
*/
class OBFrame : public otk::OtkWidget, public OBWidget {
public:

  //! The event mask to grab on frame windows
  static const long event_mask = EnterWindowMask | LeaveWindowMask;
   
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
  OBBackgroundWidget  _plate;   // sits entirely under the client window
  OBBackgroundWidget  _titlebar;
  OBButtonWidget      _button_close;
  OBButtonWidget      _button_iconify;
  OBButtonWidget      _button_max;
  OBButtonWidget      _button_stick;
  OBLabelWidget       _label;
  OBBackgroundWidget  _handle;
  OBButtonWidget      _grip_left;
  OBButtonWidget      _grip_right;

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

  //! Shape the frame window to the client window
  void adjustShape();

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

  //! Empty overridden method to prevent automatic alignment of children
  virtual void adjust();
  
  //! Displays focused decorations
  virtual void focus();
  //! Displays unfocused decorations
  virtual void unfocus();

  void setTitle(const std::string &text);
 
  //! Update the frame's size to match the client
  void adjustSize();
  //! Update the frame's position to match the client
  void adjustPosition();

  //! Applies gravity to the client's position to find where the frame should
  //! be positioned.
  /*!
    @return The proper coordinates for the frame, based on the client.
  */
  void clientGravity(int &x, int &y);

  //! Reversly applies gravity to the frame's position to find where the client
  //! should be positioned.
  /*!
    @return The proper coordinates for the client, based on the frame.
  */
  void frameGravity(int &x, int &y);

  //! Gets the window id of the frame's "plate" subelement
  inline Window plate() const { return _plate.window(); }
  //! Gets the window id of the frame's "titlebar" subelement
  inline Window titlebar() const { return _titlebar.window(); }
  //! Gets the window id of the frame's "label" subelement
  inline Window label() const { return _label.window(); }
  //! Gets the window id of the frame's "close button" subelement
  inline Window button_close() const { return _button_close.window(); }
  //! Gets the window id of the frame's "iconify button" subelement
  inline Window button_iconify() const { return _button_iconify.window(); }
  //! Gets the window id of the frame's "maximize button" subelement
  inline Window button_max() const { return _button_max.window(); }
  //! Gets the window id of the frame's "sticky button" subelement
  inline Window button_stick() const { return _button_stick.window(); }
  //! Gets the window id of the frame's "handle" subelement
  inline Window handle() const { return _handle.window(); }
  //! Gets the window id of the frame's "left grip" subelement
  inline Window grip_left() const { return _grip_left.window(); }
  //! Gets the window id of the frame's "right grip" subelement
  inline Window grip_right() const { return _grip_right.window(); }

};

}

#endif // __frame_hh
