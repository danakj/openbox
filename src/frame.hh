// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __frame_hh
#define   __frame_hh

/*! @file frame.hh
*/

extern "C" {
#include <X11/Xlib.h>
}

#include "client.hh"
#include "python.hh"
#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/renderstyle.hh"
#include "otk/ustring.hh"
#include "otk/surface.hh"
#include "otk/eventhandler.hh"

#include <string>
#include <vector>

namespace ob {

//! Varius geometry settings in the frame decorations
struct FrameGeometry {
  int width; // title and handle
  int font_height;
  int title_height() { return font_height + bevel*2; }
  int label_width;
  int label_height() { return font_height; }
  int handle_height; // static, from the style
  int icon_x;        // x-position of the window icon button
  int handle_y;
  int button_size;   // static, from the style
  int grip_width() { return button_size * 2; }
  int bevel;         // static, from the style
  int bwidth;  // frame elements' border width
  int cbwidth; // client border width
};

//! Holds and decorates a frame around an Client (client window)
/*!
  The frame is responsible for calling XSelectInput on the client window's new
  parent with the SubstructureRedirectMask so that structure events for the
  client are sent to the window manager.
*/
class Frame : public otk::StyleNotify, public otk::EventHandler {
public:

  //! The event mask to grab on frame windows
  static const long event_mask = EnterWindowMask | LeaveWindowMask;
   
private:
  Client *_client;

  //! The size of the frame on each side of the client window
  otk::Strut _size;

  //! The size of the frame on each side of the client window inside the border
  otk::Strut _innersize;

  //! The position and size of the entire frame (including borders)
  otk::Rect _area;

  bool _visible;

  //! The decorations that are being displayed in the frame.
  Client::DecorationFlags _decorations;
  
  // decoration windows
  Window  _frame;   // sits under everything
  Window  _plate;   // sits entirely under the client window
  Window  _title;   // the titlebar
  Window  _label;   // the section of the titlebar which shows the window name
  Window  _handle;  // bottom bar
  Window  _lgrip;   // lefthand resize grab on the handle
  Window  _rgrip;   // righthand resize grab on the handle
  Window  _max;     // maximize button
  Window  _desk;    // all-desktops button
  Window  _iconify; // iconify button
  Window  _icon;    // window icon button
  Window  _close;   // close button

  // surfaces for each 
  otk::Surface  *_frame_sur;
  otk::Surface  *_title_sur;
  otk::Surface  *_label_sur;
  otk::Surface  *_handle_sur;
  otk::Surface  *_grip_sur;
  otk::Surface  *_max_sur;
  otk::Surface  *_desk_sur;
  otk::Surface  *_iconify_sur;
  otk::Surface  *_icon_sur;
  otk::Surface  *_close_sur;

  std::string _layout; // layout of the titlebar

  bool _max_press;
  bool _desk_press;
  bool _iconify_press;
  bool _icon_press;
  bool _close_press;
  unsigned int _press_button; // mouse button that started the press

  FrameGeometry geom;
  
  void applyStyle(const otk::RenderStyle &style);
  void layoutTitle();
  void renderLabel();
  void renderMax();
  void renderDesk();
  void renderIconify();
  void renderClose();
  void renderIcon();

public:
  //! Constructs an Frame object for a client
  /*!
    @param client The client which will be decorated by the new Frame
  */
  Frame(Client *client);
  //! Destroys the Frame object
  virtual ~Frame();

  //! Returns the size of the frame on each side of the client
  const otk::Strut& size() const { return _size; }
  
  //! Set the style to decorate the frame with
  virtual void styleChanged(const otk::RenderStyle &style);

  //! Reparents the client window from the root window onto the frame
  void grabClient();
  //! Reparents the client window back to the root window
  void releaseClient();

  //! Update the frame's size to match the client
  void adjustSize();
  //! Update the frame's position to match the client
  void adjustPosition();
  //! Shape the frame window to the client window
  void adjustShape();
  //! Update the frame to match the client's new state (for things like toggle
  //! buttons, focus, and the title) XXX break this up
  void adjustState();
  void adjustFocus();
  void adjustTitle();
  void adjustIcon();

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

  //! The position and size of the frame window
  inline const otk::Rect& area() const { return _area; }

  //! Returns if the frame is visible
  inline bool visible() const { return _visible; }

  //! Shows the frame
  void show();
  //! Hides the frame
  void hide();

  void buttonPressHandler(const XButtonEvent &e);
  void buttonReleaseHandler(const XButtonEvent &e);

  //! Returns the MouseContext for the given window id
  /*!
    Returns '-1' if no valid mouse context exists in the frame for the given
    id.
  */
  ob::MouseContext::MC mouseContext(Window win) const;

  //! Gets the window id of the frame's base top-level parent
  inline Window window() const { return _frame; }
  //! Gets the window id of the client's parent window
  inline Window plate() const { return _plate; }

  //! Returns a null terminated array of the window ids that make up the
  //! frame's decorations.
  Window *allWindows() const;
};

}

#endif // __frame_hh
