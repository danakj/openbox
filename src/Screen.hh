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
#include <X11/Xresource.h>

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
#include "Configmenu.hh"
#include "Iconmenu.hh"
#include "Netizen.hh"
#include "Rootmenu.hh"
#include "Timer.hh"
#include "Workspace.hh"
#include "Workspacemenu.hh"
#include "blackbox.hh"
class Slit; // forward reference

enum TextJustify { LeftJustify = 1, RightJustify, CenterJustify };

struct WindowStyle {
  BColor f_focus, f_unfocus, l_text_focus, l_text_unfocus, b_pic_focus,
    b_pic_unfocus;
  BTexture t_focus, t_unfocus, l_focus, l_unfocus, h_focus, h_unfocus,
    b_focus, b_unfocus, b_pressed, g_focus, g_unfocus;

  XFontSet fontset;
  XFontSetExtents *fontset_extents;
  XFontStruct *font;

  TextJustify justify;

  int doJustify(const char *text, int &start_pos, unsigned int max_length,
                unsigned int modifier, bool multibyte) const;
};

struct ToolbarStyle {
  BColor l_text, w_text, c_text, b_pic;
  BTexture toolbar, label, window, button, pressed, clock;

  XFontSet fontset;
  XFontSetExtents *fontset_extents;
  XFontStruct *font;

  TextJustify justify;

  int doJustify(const char *text, int &start_pos, unsigned int max_length,
                unsigned int modifier, bool multibyte) const;
};

struct MenuStyle {
  BColor t_text, f_text, h_text, d_text;
  BTexture title, frame, hilite;

  XFontSet t_fontset, f_fontset;
  XFontSetExtents *t_fontset_extents, *f_fontset_extents;
  XFontStruct *t_font, *f_font;

  TextJustify t_justify, f_justify;
  int bullet, bullet_pos;
};

struct Strut {
  unsigned int top, bottom, left, right;

  Strut(void): top(0), bottom(0), left(0), right(0) {}
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

  typedef std::list<Rootmenu*> RootmenuList;
  RootmenuList rootmenuList;

  typedef std::list<Netizen*> NetizenList;
  NetizenList netizenList;
  BlackboxWindowList iconList, windowList;

  Slit *slit;
  Toolbar *toolbar;
  Workspace *current_workspace;
  Workspacemenu *workspacemenu;

  unsigned int geom_w, geom_h;
  unsigned long event_mask;

  Rect usableArea;

  typedef std::list<Strut*> StrutList;
  StrutList strutList;
  typedef std::vector<std::string> WorkspaceNamesList;
  WorkspaceNamesList workspaceNames;
  typedef std::vector<Workspace*> WorkspaceList;
  WorkspaceList workspacesList;

  struct screen_resource {
    WindowStyle wstyle;
    ToolbarStyle tstyle;
    MenuStyle mstyle;

    bool toolbar_on_top, toolbar_auto_hide, sloppy_focus, auto_raise,
      auto_edge_balance, image_dither, ordered_dither, opaque_move, full_max,
      focus_new, focus_last, click_raise;
    BColor border_color;
    XrmDatabase stylerc;

    unsigned int workspaces;
    int toolbar_placement, toolbar_width_percent, placement_policy,
      edge_snap_threshold, row_direction, col_direction;

    bool slit_on_top, slit_auto_hide;
    int slit_placement, slit_direction;

    unsigned int handle_width, bevel_width, frame_width, border_width;

#ifdef    HAVE_STRFTIME
    std::string strftime_format;
#else // !HAVE_STRFTIME
    bool clock24hour;
    int date_format;
#endif // HAVE_STRFTIME

  } resource;

  BScreen(const BScreen&);
  BScreen& operator=(const BScreen&);

  bool parseMenuFile(FILE *file, Rootmenu *menu);

  BTexture readDatabaseTexture(const std::string &rname,
                               const std::string &rclass,
                               const std::string &default_color);
  BColor readDatabaseColor(const std::string &rname,
                           const std::string &rclass,
                           const std::string &default_color);
  XFontSet readDatabaseFontSet(const std::string &rname,
                               const std::string &rclass);
  XFontStruct *readDatabaseFont(const std::string &rname,
                                const std::string &rclass);
  XFontSet createFontSet(const std::string &fontname);

  void InitMenu(void);
  void LoadStyle(void);


public:
  enum { RowSmartPlacement = 1, ColSmartPlacement, CascadePlacement, LeftRight,
         RightLeft, TopBottom, BottomTop };
  enum { RoundBullet = 1, TriangleBullet, SquareBullet, NoBullet };
  enum { Restart = 1, RestartOther, Exit, Shutdown, Execute, Reconfigure,
         WindowShade, WindowIconify, WindowMaximize, WindowClose, WindowRaise,
         WindowLower, WindowStick, WindowKill, SetStyle };
  enum FocusModel { SloppyFocus, ClickToFocus };

  BScreen(Blackbox *bb, unsigned int scrn);
  ~BScreen(void);

  inline bool isToolbarOnTop(void) const
  { return resource.toolbar_on_top; }
  inline bool doToolbarAutoHide(void) const
  { return resource.toolbar_auto_hide; }
  inline bool isSloppyFocus(void) const
  { return resource.sloppy_focus; }
  inline bool isRootColormapInstalled(void) const
  { return root_colormap_installed; }
  inline bool doAutoRaise(void) const { return resource.auto_raise; }
  inline bool doClickRaise(void) const { return resource.click_raise; }
  inline bool isScreenManaged(void) const { return managed; }
  inline bool doImageDither(void) const
  { return resource.image_dither; }
  inline bool doOrderedDither(void) const
  { return resource.ordered_dither; }
  inline bool doOpaqueMove(void) const { return resource.opaque_move; }
  inline bool doFullMax(void) const { return resource.full_max; }
  inline bool doFocusNew(void) const { return resource.focus_new; }
  inline bool doFocusLast(void) const { return resource.focus_last; }

  inline const GC &getOpGC(void) const { return opGC; }

  inline Blackbox *getBlackbox(void) { return blackbox; }
  inline BColor *getBorderColor(void) { return &resource.border_color; }
  inline BImageControl *getImageControl(void) { return image_control; }
  inline Rootmenu *getRootmenu(void) { return rootmenu; }

  inline bool isSlitOnTop(void) const { return resource.slit_on_top; }
  inline bool doSlitAutoHide(void) const
  { return resource.slit_auto_hide; }
  inline Slit *getSlit(void) { return slit; }
  inline int getSlitPlacement(void) const
  { return resource.slit_placement; }
  inline int getSlitDirection(void) const
  { return resource.slit_direction; }
  inline void saveSlitPlacement(int p) { resource.slit_placement = p; }
  inline void saveSlitDirection(int d) { resource.slit_direction = d; }
  inline void saveSlitOnTop(bool t)    { resource.slit_on_top = t; }
  inline void saveSlitAutoHide(bool t) { resource.slit_auto_hide = t; }

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

  inline unsigned int getCurrentWorkspaceID(void)
  { return current_workspace->getID(); }
  inline unsigned int getWorkspaceCount(void)
  { return workspacesList.size(); }
  inline unsigned int getIconCount(void) { return iconList.size(); }
  inline unsigned int getNumberOfWorkspaces(void) const
  { return resource.workspaces; }
  inline int getToolbarPlacement(void) const
  { return resource.toolbar_placement; }
  inline int getToolbarWidthPercent(void) const
  { return resource.toolbar_width_percent; }
  inline int getPlacementPolicy(void) const
  { return resource.placement_policy; }
  inline int getEdgeSnapThreshold(void) const
  { return resource.edge_snap_threshold; }
  inline int getRowPlacementDirection(void) const
  { return resource.row_direction; }
  inline int getColPlacementDirection(void) const
  { return resource.col_direction; }

  inline void setRootColormapInstalled(bool r) { root_colormap_installed = r; }
  inline void saveSloppyFocus(bool s) { resource.sloppy_focus = s; }
  inline void saveAutoRaise(bool a) { resource.auto_raise = a; }
  inline void saveClickRaise(bool c) { resource.click_raise = c; }
  inline void saveWorkspaces(unsigned int w) { resource.workspaces = w; }
  inline void saveToolbarOnTop(bool r) { resource.toolbar_on_top = r; }
  inline void saveToolbarAutoHide(bool r) { resource.toolbar_auto_hide = r; }
  inline void saveToolbarWidthPercent(int w)
  { resource.toolbar_width_percent = w; }
  inline void saveToolbarPlacement(int p) { resource.toolbar_placement = p; }
  inline void savePlacementPolicy(int p) { resource.placement_policy = p; }
  inline void saveRowPlacementDirection(int d) { resource.row_direction = d; }
  inline void saveColPlacementDirection(int d) { resource.col_direction = d; }
  inline void saveEdgeSnapThreshold(int t)
  { resource.edge_snap_threshold = t; }
  inline void saveImageDither(bool d) { resource.image_dither = d; }
  inline void saveOpaqueMove(bool o) { resource.opaque_move = o; }
  inline void saveFullMax(bool f) { resource.full_max = f; }
  inline void saveFocusNew(bool f) { resource.focus_new = f; }
  inline void saveFocusLast(bool f) { resource.focus_last = f; }
  inline void iconUpdate(void) { iconmenu->update(); }

#ifdef    HAVE_STRFTIME
  inline const char *getStrftimeFormat(void)
  { return resource.strftime_format.c_str(); }
  void saveStrftimeFormat(const std::string& format);
#else // !HAVE_STRFTIME
  inline int getDateFormat(void) { return resource.date_format; }
  inline void saveDateFormat(int f) { resource.date_format = f; }
  inline bool isClock24Hour(void) { return resource.clock24hour; }
  inline void saveClock24Hour(bool c) { resource.clock24hour = c; }
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
  void removeWorkspaceNames(void);
  void addWorkspaceName(const std::string& name);
  const std::string getNameOfWorkspace(unsigned int id);
  void changeWorkspaceID(unsigned int id);

  void addNetizen(Netizen *n);
  void removeNetizen(Window w);

  void addIcon(BlackboxWindow *w);
  void removeIcon(BlackboxWindow *w);

  void manageWindow(Window w);
  void unmanageWindow(BlackboxWindow *w, bool remap);
  void raiseWindows(Window *workspace_stack, unsigned int num);
  void reassociateWindow(BlackboxWindow *w, unsigned int wkspc_id,
                         bool ignore_sticky);
  void propagateWindowName(const BlackboxWindow *bw);
  void prevFocus(void);
  void nextFocus(void);
  void raiseFocus(void);
  void reconfigure(void);
  void toggleFocusModel(FocusModel model);
  void updateFocusModel(void);
  void rereadMenu(void);
  void shutdown(void);
  void showPosition(int x, int y);
  void showGeometry(unsigned int gx, unsigned int gy);
  void hideGeometry(void);

  void buttonPressEvent(XButtonEvent *xbutton);

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
