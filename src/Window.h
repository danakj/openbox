// Window.h for Openbox
// Copyright (c) 2001 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef   __Window_hh
#define   __Window_hh

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef    SHAPE
#  include <X11/extensions/shape.h>
#endif // SHAPE

#include "BaseDisplay.h"
#include "Timer.h"
#include "Windowmenu.h"
#include "Geometry.h"

// forward declaration
class OpenboxWindow;

#define MwmHintsFunctions     (1l << 0)
#define MwmHintsDecorations   (1l << 1)

#define MwmFuncAll            (1l << 0)
#define MwmFuncResize         (1l << 1)
#define MwmFuncMove           (1l << 2)
#define MwmFuncIconify        (1l << 3)
#define MwmFuncMaximize       (1l << 4)
#define MwmFuncClose          (1l << 5)

#define MwmDecorAll           (1l << 0)
#define MwmDecorBorder        (1l << 1)
#define MwmDecorHandle        (1l << 2)
#define MwmDecorTitle         (1l << 3)
#define MwmDecorMenu          (1l << 4)
#define MwmDecorIconify       (1l << 5)
#define MwmDecorMaximize      (1l << 6)

// this structure only contains 3 elements... the Motif 2.0 structure contains
// 5... we only need the first 3... so that is all we will define
typedef struct MwmHints {
  unsigned long flags, functions, decorations;
} MwmHints;

#define PropMwmHintsElements  3


class OpenboxWindow : public TimeoutHandler {
private:
  BImageControl *image_ctrl;
  Openbox &openbox;
  BScreen *screen;
  Display *display;
  BTimer *timer;
  OpenboxAttributes openbox_attrib;

  Time lastButtonPressTime;  // used for double clicks, when were we clicked
  Windowmenu *windowmenu;

  int window_number, workspace_number;
  unsigned long current_state;

  enum FocusMode { F_NoInput = 0, F_Passive,
		   F_LocallyActive, F_GloballyActive };
  FocusMode focus_mode;

  enum ResizeZones {
    ZoneTop     = 1 << 0,
    ZoneBottom  = 1 << 1,
    ZoneLeft    = 1 << 2,
    ZoneRight   = 1 << 3
  };
  unsigned int resize_zone;     // bitmask of ResizeZones values

  struct _flags {
    Bool moving,             // is moving?
      resizing,              // is resizing?
      shaded,                // is shaded?
      visible,               // is visible?
      iconic,                // is iconified?
      transient,             // is a transient window?
      focused,               // has focus?
      stuck,                 // is omnipresent
      modal,                 // is modal? (must be dismissed to continue)
      send_focus_message,    // should we send focus messages to our client?
      shaped,                // does the frame use the shape extension?
      managed;               // under openbox's control?
                             // maximize is special, the number corresponds
                             // with a mouse button
                             // if 0, not maximized
    unsigned int maximized;  // 1 = HorizVert, 2 = Vertical, 3 = Horizontal
  } flags;

  struct _client {
    OpenboxWindow *transient_for,   // which window are we a transient for?
      *transient;                   // which window is our transient?

    Window window,                  // the client's window
      window_group;                 // the client's window group

    char *title, *icon_title;
    size_t title_len;               // strlen(title)

    int x, y,
      old_bw;                       // client's borderwidth

    unsigned int width, height,
      title_text_w,                 // width as rendered in the current font
      min_width, min_height,        // can not be resized smaller
      max_width, max_height,        // can not be resized larger
      width_inc, height_inc,        // increment step
      min_aspect_x, min_aspect_y,   // minimum aspect ratio
      max_aspect_x, max_aspect_y,   // maximum aspect ratio
      base_width, base_height,
      win_gravity;

    unsigned long initial_state, normal_hint_flags, wm_hint_flags;

    MwmHints *mwm_hint;
    OpenboxHints *openbox_hint;
  } client;

  struct _functions {
    Bool resize, move, iconify, maximize, close;
  } functions;

  /*
   * client window = the application's window
   * frame window = the window drawn around the outside of the client window
   *                by the window manager which contains items like the
   *                titlebar and close button
   * title = the titlebar drawn above the client window, it displays the
   *         window's name and any buttons for interacting with the window,
   *         such as iconify, maximize, and close
   * label = the window in the titlebar where the title is drawn
   * buttons = maximize, iconify, close
   * handle = the bar drawn at the bottom of the window, which contains the
   *          left and right grips used for resizing the window
   * grips = the smaller reactangles in the handle, one of each side of it.
   *         When clicked and dragged, these resize the window interactively
   * border = the line drawn around the outside edge of the frame window,
   *          between the title, the bordered client window, and the handle.
   *          Also drawn between the grips and the handle
   */

  /*
   * what decorations do we have?
   * this is based on the type of the client window as well as user input
   * the menu is not really decor, but it goes hand in hand with the decor
   */
  struct _decorations {
    Bool titlebar, handle, border, iconify, maximize, close, menu;
  } decorations;

  struct _frame {
    // u -> unfocused, f -> has focus
    unsigned long ulabel_pixel, flabel_pixel, utitle_pixel,
      ftitle_pixel, uhandle_pixel, fhandle_pixel, ubutton_pixel,
      fbutton_pixel, pbutton_pixel, uborder_pixel, fborder_pixel,
      ugrip_pixel, fgrip_pixel;
    Pixmap ulabel, flabel, utitle, ftitle, uhandle, fhandle,
      ubutton, fbutton, pbutton, ugrip, fgrip;

    Window window,       // the frame
      plate,             // holds the client
      title,
      label,
      handle,
      close_button, iconify_button, maximize_button,
      right_grip, left_grip;


    unsigned int resize_w, resize_h;
    int resize_x, resize_y,    // size and location of box drawn while resizing
      move_x, move_y;          // location of box drawn while moving

    int x, y,
      grab_x, grab_y,          // where was the window when it was grabbed?
      y_border, y_handle;      // where within frame is the border and handle

    unsigned int width, height, title_h, label_w, label_h, handle_h,
      button_w, button_h, grip_w, grip_h, mwm_border_w, border_h, border_w,
      bevel_w;
  } frame;

protected:
  Bool getState();
  Window createToplevelWindow(int x, int y, unsigned int width,
			      unsigned int height, unsigned int borderwidth);
  Window createChildWindow(Window parent, Cursor = None);

  void getWMName();
  void getWMIconName();
  void getWMNormalHints();
  void getWMProtocols();
  void getWMHints();
  void getMWMHints();
  void getOpenboxHints();
  void setNetWMAttributes();
  void associateClientWindow();
  void decorate();
  void decorateLabel();
  void positionButtons(Bool redecorate_label = False);
  void positionWindows();
  void createCloseButton();
  void createIconifyButton();
  void createMaximizeButton();
  void redrawLabel();
  void redrawAllButtons();
  void redrawCloseButton(Bool);
  void redrawIconifyButton(Bool);
  void redrawMaximizeButton(Bool);
  void restoreGravity();
  void setGravityOffsets();
  void setState(unsigned long);
  void upsize();
  void downsize();
  void right_fixsize(int *gx = 0, int *gy = 0);
  void left_fixsize(int *gx = 0, int *gy = 0);
  void doMove(int x, int y);


public:
  OpenboxWindow(Openbox &b, Window w, BScreen *s = (BScreen *) 0);
  virtual ~OpenboxWindow();

  inline Bool isTransient() const { return flags.transient; }
  inline Bool isFocused() const { return flags.focused; }
  inline Bool isVisible() const { return flags.visible; }
  inline Bool isIconic() const { return flags.iconic; }
  inline Bool isShaded() const { return flags.shaded; }
  inline Bool isMaximized() const { return flags.maximized; }
  inline Bool isMaximizedFull() const { return flags.maximized == 1; }
  inline Bool isStuck() const { return flags.stuck; }
  inline Bool isIconifiable() const { return functions.iconify; }
  inline Bool isMaximizable() const { return functions.maximize; }
  inline Bool isResizable() const { return functions.resize; }
  inline Bool isClosable() const { return functions.close; }

  inline Bool hasTitlebar() const { return decorations.titlebar; }
  inline Bool hasTransient() const
  { return ((client.transient) ? True : False); }

  inline OpenboxWindow *getTransient() { return client.transient; }
  inline OpenboxWindow *getTransientFor() { return client.transient_for; }

  inline BScreen *getScreen() { return screen; }

  inline const Window &getFrameWindow() const { return frame.window; }
  inline const Window &getClientWindow() const { return client.window; }

  inline Windowmenu * getWindowmenu() { return windowmenu; }

  inline char **getTitle() { return &client.title; }
  inline char **getIconTitle() { return &client.icon_title; }
  //inline const int &getXFrame() const { return frame.x; }
  //inline const int &getYFrame() const { return frame.y; }
  //inline const int &getXClient() const { return client.x; }
  //inline const int &getYClient() const { return client.y; }
  inline const int &getWorkspaceNumber() const { return workspace_number; }
  inline const int &getWindowNumber() const { return window_number; }

  //inline const unsigned int &getWidth() const { return frame.width; }
  //inline const unsigned int &getHeight() const {
  //  if (!flags.shaded)
  //    return frame.height;
  //  else
  //    return frame.title_h;
  //}
  //inline const unsigned int &getClientHeight() const
  //{ return client.height; }
  //inline const unsigned int &getClientWidth() const
  //{ return client.width; }
  inline const unsigned int &getTitleHeight() const
  { return frame.title_h; }

  //inline const Point origin() const {
  //  return Point(frame.x, frame.y);
  //}
  //inline const Point clientOrigin() const {
  //  return Point(client.x, client.y);
  //}
  //inline const Size size() const {
  //  return Size(frame.width, flags.shaded ? frame.title_h : frame.height);
  //}
  //inline const Size clientSize() const {
  //  return Size(client.width, client.height);
  //}
  inline const Rect area() const {
    return Rect(frame.x, frame.y, frame.width,
                flags.shaded ? frame.title_h : frame.height);
  }
  inline const Rect clientArea() const {
    return Rect(client.x, client.y, client.width, client.height);
  }
  
  inline void setWindowNumber(int n) { window_number = n; }
  
  Bool validateClient();
  Bool setInputFocus();

  void setFocusFlag(Bool);
  void iconify();
  void deiconify(Bool reassoc = True, Bool raise = True);
  void close();
  void withdraw();
  void maximize(unsigned int button);
  void shade();
  void stick();
  void unstick();
  void reconfigure();
  void installColormap(Bool);
  void restore();
  void configure(int dx, int dy, unsigned int dw, unsigned int dh);
  void setWorkspace(int n);
  void changeOpenboxHints(OpenboxHints *);
  void restoreAttributes();

  void startMove(int x, int y);
  void endMove();
  
  void buttonPressEvent(XButtonEvent *);
  void buttonReleaseEvent(XButtonEvent *);
  void motionNotifyEvent(XMotionEvent *);
  void destroyNotifyEvent(XDestroyWindowEvent *);
  void mapRequestEvent(XMapRequestEvent *);
  void mapNotifyEvent(XMapEvent *);
  void unmapNotifyEvent(XUnmapEvent *);
  void propertyNotifyEvent(Atom);
  void exposeEvent(XExposeEvent *);
  void configureRequestEvent(XConfigureRequestEvent *);

#ifdef    SHAPE
  void shapeEvent(XShapeEvent *);
#endif // SHAPE

  virtual void timeout();
};


#endif // __Window_hh
