// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __Window_hh
#define   __Window_hh

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef    SHAPE
#  include <X11/extensions/shape.h>
#endif // SHAPE
}

#include <string>

#include "otk/timer.hh"
#include "otk/property.hh"
#include "otk/rect.hh"
#include "otk/strut.hh"
#include "blackbox.hh"
#include "util.hh"

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
#define MwmDecorMenu          (1l << 4) // not used
#define MwmDecorIconify       (1l << 5)
#define MwmDecorMaximize      (1l << 6)

namespace ob {

// this structure only contains 3 elements... the Motif 2.0 structure contains
// 5... we only need the first 3... so that is all we will define
typedef struct MwmHints {
  unsigned long flags, functions, decorations;
} MwmHints;

#define PropMwmHintsElements  3

class BWindowGroup {
private:
  Blackbox *blackbox;
  Window group;
  BlackboxWindowList windowList;

public:
  BWindowGroup(Blackbox *b, Window _group);
  ~BWindowGroup(void);

  inline Window groupWindow(void) const { return group; }

  inline bool empty(void) const { return windowList.empty(); }

  void addWindow(BlackboxWindow *w) { windowList.push_back(w); }
  void removeWindow(BlackboxWindow *w) { windowList.remove(w); }

  /*
    find a window on the specified screen. the focused window (if any) is
    checked first, otherwise the first matching window found is returned.
    transients are returned only if allow_transients is True.
  */
  BlackboxWindow *find(BScreen *screen, bool allow_transients = False) const;
};


class BlackboxWindow {
public:
  enum Function { Func_Resize   = (1l << 0),
                  Func_Move     = (1l << 1),
                  Func_Iconify  = (1l << 2),
                  Func_Maximize = (1l << 3),
                  Func_Close    = (1l << 4) };
  typedef unsigned char FunctionFlags;

  enum Decoration { Decor_Titlebar = (1l << 0),
                    Decor_Handle   = (1l << 1),
                    Decor_Border   = (1l << 2),
                    Decor_Iconify  = (1l << 3),
                    Decor_Maximize = (1l << 4),
                    Decor_Close    = (1l << 5) };
  typedef unsigned char DecorationFlags;

  enum WindowType { Type_Desktop,
                    Type_Dock,
                    Type_Toolbar,
                    Type_Menu,
                    Type_Utility,
                    Type_Splash,
                    Type_Dialog,
                    Type_Normal };

  enum Corner { TopLeft,
                TopRight,
                BottomLeft,
                BottomRight };

private:
  Blackbox *blackbox;
  BScreen *screen;
  otk::OBProperty *xatom;
  otk::OBTimer *timer;
  BlackboxAttributes blackbox_attrib;

  Time lastButtonPressTime;  // used for double clicks, when were we clicked

  unsigned int window_number;
  unsigned long current_state;
  unsigned int mod_mask;    // the mod mask used to grab buttons

  enum FocusMode { F_NoInput = 0, F_Passive,
                   F_LocallyActive, F_GloballyActive };
  FocusMode focus_mode;

  struct _flags {
    bool moving,             // is moving?
      resizing,              // is resizing?
      shaded,                // is shaded?
      visible,               // is visible?
      iconic,                // is iconified?
      focused,               // has focus?
      stuck,                 // is omnipresent?
      modal,                 // is modal? (must be dismissed to continue)
      skip_taskbar,          // skipped by taskbars?
      skip_pager,            // skipped by pagers?
      fullscreen,            // a fullscreen window?
      send_focus_message,    // should we send focus messages to our client?
      shaped;                // does the frame use the shape extension?
    unsigned int maximized;  // maximize is special, the number corresponds
                             // with a mouse button
                             // if 0, not maximized
                             // 1 = HorizVert, 2 = Vertical, 3 = Horizontal
  } flags;

  struct _client {
    Window window,                  // the client's window
      window_group;
    BlackboxWindow *transient_for;  // which window are we a transient for?
    BlackboxWindowList transientList; // which windows are our transients?

    std::string title, icon_title;

    otk::Rect rect;
    otk::Strut strut;

    int old_bw;                       // client's borderwidth

    unsigned int
      min_width, min_height,        // can not be resized smaller
      max_width, max_height,        // can not be resized larger
      width_inc, height_inc,        // increment step
#if 0 // not supported at the moment
      min_aspect_x, min_aspect_y,   // minimum aspect ratio
      max_aspect_x, max_aspect_y,   // maximum aspect ratio
#endif
      base_width, base_height,
      win_gravity;

    unsigned long initial_state, normal_hint_flags;
  } client;

  FunctionFlags functions;
  /*
   * what decorations do we have?
   * this is based on the type of the client window as well as user input
   */
  DecorationFlags decorations;
  DecorationFlags mwm_decorations;
  Corner resize_dir;
  WindowType window_type;

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

  struct _frame {
    // u -> unfocused, f -> has focus, p -> pressed
    unsigned long ulabel_pixel, flabel_pixel, utitle_pixel,
      ftitle_pixel, uhandle_pixel, fhandle_pixel, ubutton_pixel,
      fbutton_pixel, pfbutton_pixel, pubutton_pixel,
      uborder_pixel, fborder_pixel, ugrip_pixel, fgrip_pixel;
    Pixmap ulabel, flabel, utitle, ftitle, uhandle, fhandle,
      ubutton, fbutton, pfbutton, pubutton, ugrip, fgrip;

    Window window,       // the frame
      plate,             // holds the client
      title,
      label,
      handle,
      close_button, iconify_button, maximize_button, stick_button,
      right_grip, left_grip;

    /*
     * size and location of the box drawn while the window dimensions or
     * location is being changed, ie. resized or moved
     */
    otk::Rect changing;

    otk::Rect rect;                  // frame geometry
    otk::Strut margin;               // margins between the frame and client

    int grab_x, grab_y;         // where was the window when it was grabbed?

    unsigned int inside_w, inside_h, // window w/h without border_w
      title_h, label_w, label_h, handle_h,
      button_w, grip_w, mwm_border_w, border_w,
      bevel_w;
  } frame;

  BlackboxWindow(const BlackboxWindow&);
  BlackboxWindow& operator=(const BlackboxWindow&);

  bool getState(void);
  Window createToplevelWindow();
  Window createChildWindow(Window parent, unsigned long event_mask,
                           Cursor = None);

  bool getWindowType(void);
  void updateStrut(void);
  void getWMName(void);
  void getWMIconName(void);
  void getWMNormalHints(void);
  void getWMProtocols(void);
  void getWMHints(void);
  void getNetWMHints(void);
  void getMWMHints(void);
  bool getBlackboxHints(void);
  void getTransientInfo(void);
  void setNetWMAttributes(void);
  void associateClientWindow(void);
  void decorate(void);
  void decorateLabel(void);
  void positionButtons(bool redecorate_label = False);
  void positionWindows(void);
  void createHandle(void);
  void destroyHandle(void);
  void createTitlebar(void);
  void destroyTitlebar(void);
  void createCloseButton(void);
  void destroyCloseButton(void);
  void createIconifyButton(void);
  void destroyIconifyButton(void);
  void createMaximizeButton(void);
  void destroyMaximizeButton(void);
  void createStickyButton(void);
  void destroyStickyButton(void);
  void redrawWindowFrame(void) const;
  void redrawLabel(void) const;
  void redrawAllButtons(void) const;
  void redrawButton(bool pressed, Window win,
                    Pixmap fppix, unsigned long fppixel,
                    Pixmap uppix, unsigned long uppixel,
                    Pixmap fpix, unsigned long fpixel,
                    Pixmap upix, unsigned long upixel) const;
  void redrawCloseButton(bool pressed) const;
  void redrawIconifyButton(bool pressed) const;
  void redrawMaximizeButton(bool pressed) const;
  void redrawStickyButton(bool pressed) const;
  void applyGravity(otk::Rect &r);
  void restoreGravity(otk::Rect &r);
  void setAllowedActions(void);
  void setState(unsigned long new_state);
  void upsize(void);
  void doMove(int x_root, int y_root);
  void doWorkspaceWarping(int x_root, int y_root, int &dx);
  void doWindowSnapping(int &dx, int &dy);
  void endMove(void);
  void doResize(int x_root, int y_root);
  void endResize(void);

  void constrain(Corner anchor, unsigned int *pw = 0, unsigned int *ph = 0);

public:
  BlackboxWindow(Blackbox *b, Window w, BScreen *s);
  virtual ~BlackboxWindow(void);

  inline bool isTransient(void) const { return client.transient_for != 0; }
  inline bool isFocused(void) const { return flags.focused; }
  inline bool isVisible(void) const { return flags.visible; }
  inline bool isIconic(void) const { return flags.iconic; }
  inline bool isShaded(void) const { return flags.shaded; }
  inline bool isMaximized(void) const { return flags.maximized; }
  inline bool isMaximizedHoriz(void) const { return flags.maximized == 3; }
  inline bool isMaximizedVert(void) const { return flags.maximized == 2; }
  inline bool isMaximizedFull(void) const { return flags.maximized == 1; }
  inline bool isStuck(void) const { return flags.stuck; }
  inline bool isModal(void) const { return flags.modal; }
  inline bool isIconifiable(void) const { return functions & Func_Iconify; }
  inline bool isMaximizable(void) const { return functions & Func_Maximize; }
  inline bool isResizable(void) const { return functions & Func_Resize; }
  inline bool isClosable(void) const { return functions & Func_Close; }

  // is a 'normal' window? meaning, a standard client application
  inline bool isNormal(void) const
  { return window_type == Type_Dialog || window_type == Type_Normal ||
           window_type == Type_Toolbar || window_type == Type_Utility; }
  inline bool isTopmost(void) const
  { return window_type == Type_Toolbar || window_type == Type_Utility; }
  inline bool isDesktop(void) const { return window_type == Type_Desktop; }
  
  inline bool hasTitlebar(void) const { return decorations & Decor_Titlebar; }

  inline const BlackboxWindowList &getTransients(void) const
  { return client.transientList; }
  BlackboxWindow *getTransientFor(void) const;

  inline BScreen *getScreen(void) const { return screen; }

  inline Window getFrameWindow(void) const { return frame.window; }
  inline Window getClientWindow(void) const { return client.window; }
  inline Window getGroupWindow(void) const { return client.window_group; }

  inline const char *getTitle(void) const
  { return client.title.c_str(); }
  inline const char *getIconTitle(void) const
  { return client.icon_title.c_str(); }

  inline unsigned int getWorkspaceNumber(void) const
  { return blackbox_attrib.workspace; }
  inline unsigned int getWindowNumber(void) const { return window_number; }

  inline const otk::Rect &frameRect(void) const { return frame.rect; }
  inline const otk::Rect &clientRect(void) const { return client.rect; }

  inline unsigned int getTitleHeight(void) const
  { return frame.title_h; }

  inline void setWindowNumber(int n) { window_number = n; }

  bool validateClient(void) const;
  bool setInputFocus(void);

  // none of these are used by the window manager, they are here to persist
  // them properly in the window's netwm state property.
  inline bool skipTaskbar(void) const { return flags.skip_taskbar; }
  inline void setSkipTaskbar(const bool s) { flags.skip_taskbar = s; }
  inline bool skipPager(void) const { return flags.skip_pager; }
  inline void setSkipPager(const bool s) { flags.skip_pager = s; }
  inline bool isFullscreen(void) const { return flags.fullscreen; }
  inline void setFullscreen(const bool f) { flags.fullscreen = f; }

  inline void setModal(const bool m) { flags.modal = m; }

  void beginMove(int x_root, int y_root);
  void beginResize(int x_root, int y_root, Corner dir);
  void enableDecor(bool enable);
  void setupDecor();
  void setFocusFlag(bool focus);
  void iconify(void);
  void deiconify(bool reassoc = True, bool raise = True);
  void show(void);
  void close(void);
  void withdraw(void);
  void maximize(unsigned int button);
  void remaximize(void);
  void shade(void);
  void stick(void);
  void reconfigure(void);
  void grabButtons(void);
  void ungrabButtons(void);
  void installColormap(bool install);
  void restore(bool remap);
  void configure(int dx, int dy, unsigned int dw, unsigned int dh);
  void setWorkspace(unsigned int n);
  void changeBlackboxHints(const BlackboxHints *net);
  void restoreAttributes(void);

  void buttonPressEvent(const XButtonEvent *be);
  void buttonReleaseEvent(const XButtonEvent *re);
  void motionNotifyEvent(const XMotionEvent *me);
  void destroyNotifyEvent(const XDestroyWindowEvent* /*unused*/);
  void mapRequestEvent(const XMapRequestEvent *mre);
  void unmapNotifyEvent(const XUnmapEvent* /*unused*/);
  void reparentNotifyEvent(const XReparentEvent* /*unused*/);
  void propertyNotifyEvent(const XPropertyEvent *pe);
  void exposeEvent(const XExposeEvent *ee);
  void configureRequestEvent(const XConfigureRequestEvent *cr);
  void enterNotifyEvent(const XCrossingEvent *ce);
  void leaveNotifyEvent(const XCrossingEvent* /*unused*/);

#ifdef    SHAPE
  void configureShape(void);
  void clearShape(void);
  void shapeEvent(XShapeEvent * /*unused*/);
#endif // SHAPE

  static void timeout(BlackboxWindow *t);
};

}

#endif // __Window_hh
