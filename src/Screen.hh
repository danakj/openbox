// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Screen.hh for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
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

#ifndef   __Screen_hh
#define   __Screen_hh

extern "C" {
#include <X11/Xlib.h>

#ifdef    TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else // !TIME_WITH_SYS_TIME
#  ifdef    HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else // !HAVE_SYS_TIME_H
#    include <time.h>
#  endif // HAVE_SYS_TIME_H
#endif // TIME_WITH_SYS_TIME
}

#include <list>
#include <vector>

#include "Color.hh"
#include "Texture.hh"
#include "Image.hh"
#include "Configmenu.hh"
#include "Iconmenu.hh"
#include "Netizen.hh"
#include "Rootmenu.hh"
#include "Timer.hh"
#include "Workspace.hh"
#include "Workspacemenu.hh"
#include "blackbox.hh"

class Slit; // forward reference
class BFont;
class XAtom;
struct Strut;

enum TextJustify { LeftJustify = 1, RightJustify, CenterJustify };

struct WindowStyle {
  BColor f_focus, f_unfocus, l_text_focus, l_text_unfocus, b_pic_focus,
    b_pic_unfocus;
  BTexture t_focus, t_unfocus, l_focus, l_unfocus, h_focus, h_unfocus,
    b_focus, b_unfocus, b_pressed, g_focus, g_unfocus;

  BFont *font;

  TextJustify justify;

  void doJustify(const std::string &text, int &start_pos,
                 unsigned int max_length, unsigned int modifier) const;
};

struct ToolbarStyle {
  BColor l_text, w_text, c_text, b_pic;
  BTexture toolbar, label, window, button, pressed, clock;

  BFont *font;

  TextJustify justify;

  void doJustify(const std::string &text, int &start_pos,
                 unsigned int max_length, unsigned int modifier) const;
};

struct MenuStyle {
  BColor t_text, f_text, h_text, d_text;
  BTexture title, frame, hilite;

  BFont *t_font, *f_font;

  TextJustify t_justify, f_justify;
  int bullet, bullet_pos;
};

class BScreen : public ScreenInfo {
private:
  bool root_colormap_installed, managed, geom_visible;
  GC opGC;
  Pixmap geom_pixmap;
  Window geom_window;

  Blackbox *blackbox;
  BImageControl *image_control;
  Configmenu *configmenu;
  Iconmenu *iconmenu;
  Rootmenu *rootmenu;
  Configuration *config;
  XAtom *xatom;

  typedef std::list<Rootmenu*> RootmenuList;
  RootmenuList rootmenuList;

  typedef std::list<Netizen*> NetizenList;
  NetizenList netizenList;
  BlackboxWindowList iconList, windowList;

  typedef std::vector<Window> WindowList;
  WindowList desktopWindowList, systrayWindowList;

  Slit *slit;
  Toolbar *toolbar;
  Workspace *current_workspace;
  Workspacemenu *workspacemenu;

  unsigned int geom_w, geom_h;
  unsigned long event_mask;

  Rect usableArea;

  typedef std::list<Strut*> StrutList;
  StrutList strutList;
  typedef std::vector<Workspace*> WorkspaceList;
  WorkspaceList workspacesList;

  struct screen_resource {
    WindowStyle wstyle;
    ToolbarStyle tstyle;
    MenuStyle mstyle;

    bool sloppy_focus, auto_raise, auto_edge_balance, ordered_dither,
      opaque_move, full_max, focus_new, focus_last, click_raise,
      hide_toolbar, window_to_window_snap, window_corner_snap, aa_fonts,
      ignore_shaded, ignore_maximized;
    BColor border_color;

    unsigned int workspaces;
    int toolbar_placement, toolbar_width_percent, placement_policy,
      edge_snap_threshold, row_direction, col_direction;

    unsigned int handle_width, bevel_width, frame_width, border_width,
      resize_zones;

#ifdef    HAVE_STRFTIME
    std::string strftime_format;
#else // !HAVE_STRFTIME
    bool clock24hour;
    int date_format;
#endif // HAVE_STRFTIME

  } resource;
  std::string screenstr;

  BScreen(const BScreen&);
  BScreen& operator=(const BScreen&);

  bool parseMenuFile(FILE *file, Rootmenu *menu);

  BTexture readDatabaseTexture(const std::string &rname,
                               const std::string &default_color,
                               const Configuration &style);
  BColor readDatabaseColor(const std::string &rname,
                           const std::string &default_color,
                           const Configuration &style);
  BFont *readDatabaseFont(const std::string &rbasename,
                          const Configuration &style);

  void InitMenu(void);
  void LoadStyle(void);

  void updateWorkArea(void);
public:
  enum { RowSmartPlacement = 1, ColSmartPlacement, CascadePlacement,
         UnderMousePlacement, ClickMousePlacement, LeftRight, RightLeft,
         TopBottom, BottomTop, IgnoreShaded, IgnoreMaximized };
  enum { RoundBullet = 1, TriangleBullet, SquareBullet, NoBullet };
  enum { Restart = 1, RestartOther, Exit, Shutdown, Execute, Reconfigure,
         WindowShade, WindowIconify, WindowMaximize, WindowClose, WindowRaise,
         WindowLower, WindowStick, WindowKill, SetStyle };
  enum FocusModel { SloppyFocus, ClickToFocus };

  BScreen(Blackbox *bb, unsigned int scrn);
  ~BScreen(void);

  inline bool isSloppyFocus(void) const { return resource.sloppy_focus; }
  inline bool isRootColormapInstalled(void) const
  { return root_colormap_installed; }
  inline bool doAutoRaise(void) const { return resource.auto_raise; }
  inline bool doClickRaise(void) const { return resource.click_raise; }
  inline bool isScreenManaged(void) const { return managed; }
  inline bool doAAFonts(void) const { return resource.aa_fonts; }
  inline bool doImageDither(void) const { return image_control->doDither(); }
  inline bool doOrderedDither(void) const { return resource.ordered_dither; }
  inline bool doOpaqueMove(void) const { return resource.opaque_move; }
  inline bool doFullMax(void) const { return resource.full_max; }
  inline bool doFocusNew(void) const { return resource.focus_new; }
  inline bool doFocusLast(void) const { return resource.focus_last; }
  inline bool doHideToolbar(void) const { return resource.hide_toolbar; }
  inline bool getWindowToWindowSnap(void) const
    { return resource.window_to_window_snap; }
  inline bool getWindowCornerSnap(void) const
    { return resource.window_corner_snap; }

  inline const GC &getOpGC(void) const { return opGC; }

  inline Blackbox *getBlackbox(void) { return blackbox; }
  inline BColor *getBorderColor(void) { return &resource.border_color; }
  inline BImageControl *getImageControl(void) { return image_control; }
  inline Rootmenu *getRootmenu(void) { return rootmenu; }

  inline Slit *getSlit(void) { return slit; }
  inline Toolbar *getToolbar(void) { return toolbar; }

  Workspace *getWorkspace(unsigned int index);

  inline Workspace *getCurrentWorkspace(void) { return current_workspace; }

  inline Workspacemenu *getWorkspacemenu(void) { return workspacemenu; }

  inline unsigned int getHandleWidth(void) const
  { return resource.handle_width; }
  inline unsigned int getBevelWidth(void) const
  { return resource.bevel_width; }
  inline unsigned int getFrameWidth(void) const
  { return resource.frame_width; }
  inline unsigned int getBorderWidth(void) const
  { return resource.border_width; }
  inline unsigned int getResizeZones(void) const
  { return resource.resize_zones; }
  inline bool getPlaceIgnoreShaded(void) const
  { return resource.ignore_shaded; }
  inline bool getPlaceIgnoreMaximized(void) const
  { return resource.ignore_maximized; }

  inline unsigned int getCurrentWorkspaceID(void) const
  { return current_workspace->getID(); }
  inline unsigned int getWorkspaceCount(void) const
  { return workspacesList.size(); }
  inline unsigned int getIconCount(void) const { return iconList.size(); }
  inline unsigned int getNumberOfWorkspaces(void) const
  { return resource.workspaces; }
  inline int getPlacementPolicy(void) const
  { return resource.placement_policy; }
  inline int getEdgeSnapThreshold(void) const
  { return resource.edge_snap_threshold; }
  inline int getRowPlacementDirection(void) const
  { return resource.row_direction; }
  inline int getColPlacementDirection(void) const
  { return resource.col_direction; }

  void changeWorkspaceCount(unsigned int new_count);
  
  inline void setRootColormapInstalled(bool r) { root_colormap_installed = r; }
  void saveSloppyFocus(bool s);
  void saveAutoRaise(bool a);
  void saveClickRaise(bool c);
  void saveWorkspaces(unsigned int w);
  void savePlacementPolicy(int p);
  void saveRowPlacementDirection(int d);
  void saveColPlacementDirection(int d);
  void saveEdgeSnapThreshold(int t);
  void saveImageDither(bool d);
  void saveAAFonts(bool f);
  void saveOpaqueMove(bool o);
  void saveFullMax(bool f);
  void saveFocusNew(bool f);
  void saveFocusLast(bool f);
  void saveHideToolbar(bool h);
  void saveWindowToWindowSnap(bool s);
  void saveWindowCornerSnap(bool s);
  void saveResizeZones(unsigned int z);
  void savePlaceIgnoreShaded(bool i);
  void savePlaceIgnoreMaximized(bool i);
  inline void iconUpdate(void) { iconmenu->update(); }

#ifdef    HAVE_STRFTIME
  inline const char *getStrftimeFormat(void)
  { return resource.strftime_format.c_str(); }
  void saveStrftimeFormat(const std::string& format);
#else // !HAVE_STRFTIME
  inline int getDateFormat(void) { return resource.date_format; }
  inline void saveDateFormat(int f);
  inline bool isClock24Hour(void) { return resource.clock24hour; }
  inline void saveClock24Hour(bool c);
#endif // HAVE_STRFTIME

  inline WindowStyle *getWindowStyle(void) { return &resource.wstyle; }
  inline MenuStyle *getMenuStyle(void) { return &resource.mstyle; }
  inline ToolbarStyle *getToolbarStyle(void) { return &resource.tstyle; }

  BlackboxWindow *getIcon(unsigned int index);

  const Rect& availableArea(void) const;
  void updateAvailableArea(void);
  void addStrut(Strut *strut);
  void removeStrut(Strut *strut);

  unsigned int addWorkspace(void);
  unsigned int removeLastWorkspace(void);
  void changeWorkspaceID(unsigned int id);
  void saveWorkspaceNames(void);

  void addNetizen(Netizen *n);
  void removeNetizen(Window w);

  void addSystrayWindow(Window window);
  void removeSystrayWindow(Window window);

  void addIcon(BlackboxWindow *w);
  void removeIcon(BlackboxWindow *w);

  void updateClientList(void);
  void updateStackingList(void);
  void manageWindow(Window w);
  void unmanageWindow(BlackboxWindow *w, bool remap);
  void raiseWindows(Window *workspace_stack, unsigned int num);
  void lowerWindows(Window *workspace_stack, unsigned int num);
  void reassociateWindow(BlackboxWindow *w, unsigned int wkspc_id,
                         bool ignore_sticky);
  void propagateWindowName(const BlackboxWindow *bw);
  void prevFocus(void);
  void nextFocus(void);
  void raiseFocus(void);
  void load_rc(void);
  void save_rc(void);
  void reconfigure(void);
  void toggleFocusModel(FocusModel model);
  void rereadMenu(void);
  void shutdown(void);
  void showPosition(int x, int y);
  void showGeometry(unsigned int gx, unsigned int gy);
  void hideGeometry(void);

  void buttonPressEvent(const XButtonEvent *xbutton);
  void propertyNotifyEvent(const XPropertyEvent *pe);

  void updateNetizenCurrentWorkspace(void);
  void updateNetizenWorkspaceCount(void);
  void updateNetizenWindowFocus(void);
  void updateNetizenWindowAdd(Window w, unsigned long p);
  void updateNetizenWindowDel(Window w);
  void updateNetizenConfigNotify(XEvent *e);
  void updateNetizenWindowRaise(Window w);
  void updateNetizenWindowLower(Window w);
};


#endif // __Screen_hh
