// Screen.h for Openbox
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

#ifndef   __Screen_hh
#define   __Screen_hh

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

#include "BaseDisplay.h"
#include "Configmenu.h"
#include "Iconmenu.h"
#include "LinkedList.h"
#include "Netizen.h"
#include "Rootmenu.h"
#include "Timer.h"
#include "Workspace.h"
#include "Workspacemenu.h"
#include "openbox.h"
#ifdef    SLIT
#  include "Slit.h"
#endif // SLIT
#include "Image.h"
#include "Resource.h"

// forward declaration
class BScreen;

struct WindowStyle {
  BColor f_focus, f_unfocus, l_text_focus, l_text_unfocus, b_pic_focus,
    b_pic_unfocus;
  BTexture t_focus, t_unfocus, l_focus, l_unfocus, h_focus, h_unfocus,
    b_focus, b_unfocus, b_pressed, g_focus, g_unfocus;
  GC l_text_focus_gc, l_text_unfocus_gc, b_pic_focus_gc, b_pic_unfocus_gc;

  XFontSet fontset;
  XFontSetExtents *fontset_extents;
  XFontStruct *font;
  
  int justify;
};

struct ToolbarStyle {
  BColor l_text, w_text, c_text, b_pic;
  BTexture toolbar, label, window, button, pressed, clock;
  GC l_text_gc, w_text_gc, c_text_gc, b_pic_gc;

  XFontSet fontset;
  XFontSetExtents *fontset_extents;
  XFontStruct *font;
  
  int justify;
};

struct MenuStyle {
  BColor t_text, f_text, h_text, d_text;
  BTexture title, frame, hilite;
  GC t_text_gc, f_text_gc, h_text_gc, d_text_gc, hilite_gc;

  XFontSet t_fontset, f_fontset;
  XFontSetExtents *t_fontset_extents, *f_fontset_extents;
  XFontStruct *t_font, *f_font;

  int t_justify, f_justify, bullet, bullet_pos;
};


class BScreen : public ScreenInfo {
private:
  Bool root_colormap_installed, managed, geom_visible;
  GC opGC;
  Pixmap geom_pixmap;
  Window geom_window;

  Openbox *openbox;
  BImageControl *image_control;
  Configmenu *configmenu;
  Iconmenu *iconmenu;
  Rootmenu *rootmenu;

  LinkedList<Rootmenu> *rootmenuList;
  LinkedList<Netizen> *netizenList;
  LinkedList<OpenboxWindow> *iconList;

#ifdef    SLIT
  Slit *slit;
#endif // SLIT

  Toolbar *toolbar;
  Workspace *current_workspace;
  Workspacemenu *workspacemenu;

  unsigned int geom_w, geom_h;
  unsigned long event_mask;

  LinkedList<char> *workspaceNames;
  LinkedList<Workspace> *workspacesList;

  struct resource {
    WindowStyle wstyle;
    ToolbarStyle tstyle;
    MenuStyle mstyle;

    Bool toolbar_on_top, toolbar_auto_hide, sloppy_focus, auto_raise,
      auto_edge_balance, image_dither, ordered_dither, opaque_move, full_max,
      focus_new, focus_last;
    BColor border_color;
    obResource styleconfig;

    int workspaces, toolbar_placement, toolbar_width_percent, placement_policy,
      edge_snap_threshold, row_direction, col_direction;

#ifdef    SLIT
    Bool slit_on_top, slit_auto_hide;
    int slit_placement, slit_direction;
#endif // SLIT

    unsigned int handle_width, bevel_width, frame_width, border_width;
    unsigned int zones; // number of zones to be used when alt-resizing a window

#ifdef    HAVE_STRFTIME
    char *strftime_format;
#else // !HAVE_STRFTIME
    Bool clock24hour;
    int date_format;
#endif // HAVE_STRFTIME

    char *root_command;
  } resource;


protected:
  Bool parseMenuFile(FILE *, Rootmenu *);

  void readDatabaseTexture(const char *, const char *, BTexture *,
                           unsigned long);
  void readDatabaseColor(const char *, const char *, BColor *, unsigned long);

  void readDatabaseFontSet(const char *, const char *, XFontSet *);
  XFontSet createFontSet(const char *);
  void readDatabaseFont(const char *, const char *, XFontStruct **);

  void InitMenu(void);
  void LoadStyle(void);


public:
  BScreen(Openbox *, int);
  ~BScreen(void);

  inline const Bool &isToolbarOnTop(void) const
  { return resource.toolbar_on_top; }
  inline const Bool &doToolbarAutoHide(void) const
  { return resource.toolbar_auto_hide; }
  inline const Bool &isSloppyFocus(void) const
  { return resource.sloppy_focus; }
  inline const Bool &isRootColormapInstalled(void) const
  { return root_colormap_installed; }
  inline const Bool &doAutoRaise(void) const { return resource.auto_raise; }
  inline const Bool &isScreenManaged(void) const { return managed; }
  inline const Bool &doImageDither(void) const
  { return resource.image_dither; }
  inline const Bool &doOrderedDither(void) const
  { return resource.ordered_dither; }
  inline const Bool &doOpaqueMove(void) const { return resource.opaque_move; }
  inline const Bool &doFullMax(void) const { return resource.full_max; }
  inline const Bool &doFocusNew(void) const { return resource.focus_new; }
  inline const Bool &doFocusLast(void) const { return resource.focus_last; }

  inline const GC &getOpGC() const { return opGC; }

  inline Openbox *getOpenbox(void) { return openbox; }
  inline BColor *getBorderColor(void) { return &resource.border_color; }
  inline BImageControl *getImageControl(void) { return image_control; }
  inline Rootmenu *getRootmenu(void) { return rootmenu; }

#ifdef   SLIT
  inline const Bool &isSlitOnTop(void) const { return resource.slit_on_top; }
  inline const Bool &doSlitAutoHide(void) const
  { return resource.slit_auto_hide; }
  inline Slit *getSlit(void) { return slit; }
  inline const int &getSlitPlacement(void) const
  { return resource.slit_placement; }
  inline const int &getSlitDirection(void) const
  { return resource.slit_direction; }
  inline void saveSlitPlacement(int p) { resource.slit_placement = p; }
  inline void saveSlitDirection(int d) { resource.slit_direction = d; }
  inline void saveSlitOnTop(Bool t)    { resource.slit_on_top = t; }
  inline void saveSlitAutoHide(Bool t) { resource.slit_auto_hide = t; }
#endif // SLIT

  inline int getWindowZones(void) const
  { return resource.zones; }
  inline void saveWindowZones(int z) { resource.zones = z; }
  
  inline Toolbar *getToolbar(void) { return toolbar; }

  inline Workspace *getWorkspace(int w) { return workspacesList->find(w); }
  inline Workspace *getCurrentWorkspace(void) { return current_workspace; }

  inline Workspacemenu *getWorkspacemenu(void) { return workspacemenu; }

  inline const unsigned int &getHandleWidth(void) const
  { return resource.handle_width; }
  inline const unsigned int &getBevelWidth(void) const
  { return resource.bevel_width; }
  inline const unsigned int &getFrameWidth(void) const
  { return resource.frame_width; }
  inline const unsigned int &getBorderWidth(void) const
  { return resource.border_width; }

  inline const int getCurrentWorkspaceID()
  { return current_workspace->getWorkspaceID(); }
  inline const int getWorkspaceCount(void) { return workspacesList->count(); }
  inline const int getIconCount(void) { return iconList->count(); }
  inline const int &getNumberOfWorkspaces(void) const
  { return resource.workspaces; }
  inline const int &getToolbarPlacement(void) const
  { return resource.toolbar_placement; }
  inline const int &getToolbarWidthPercent(void) const
  { return resource.toolbar_width_percent; }
  inline const int &getPlacementPolicy(void) const
  { return resource.placement_policy; }
  inline const int &getEdgeSnapThreshold(void) const
  { return resource.edge_snap_threshold; }
  inline const int &getRowPlacementDirection(void) const
  { return resource.row_direction; }
  inline const int &getColPlacementDirection(void) const
  { return resource.col_direction; }

  inline void saveRootCommand(const char *cmd) {
    if (resource.root_command != NULL)
      delete [] resource.root_command;
    if (cmd != NULL)
      resource.root_command = bstrdup(cmd);
    else
      resource.root_command = NULL;
  }
  inline const char *getRootCommand(void) const
  { return resource.root_command; }
  
  inline void setRootColormapInstalled(Bool r) { root_colormap_installed = r; }
  inline void saveSloppyFocus(Bool s) { resource.sloppy_focus = s; }
  inline void saveAutoRaise(Bool a) { resource.auto_raise = a; }
  inline void saveWorkspaces(int w) { resource.workspaces = w; }
  inline void saveToolbarOnTop(Bool r) { resource.toolbar_on_top = r; }
  inline void saveToolbarAutoHide(Bool r) { resource.toolbar_auto_hide = r; }
  inline void saveToolbarWidthPercent(int w)
  { resource.toolbar_width_percent = w; }
  inline void saveToolbarPlacement(int p) { resource.toolbar_placement = p; }
  inline void savePlacementPolicy(int p) { resource.placement_policy = p; }
  inline void saveRowPlacementDirection(int d) { resource.row_direction = d; }
  inline void saveColPlacementDirection(int d) { resource.col_direction = d; }
  inline void saveEdgeSnapThreshold(int t)
  { resource.edge_snap_threshold = t; }
  inline void saveImageDither(Bool d) { resource.image_dither = d; }
  inline void saveOpaqueMove(Bool o) { resource.opaque_move = o; }
  inline void saveFullMax(Bool f) { resource.full_max = f; }
  inline void saveFocusNew(Bool f) { resource.focus_new = f; }
  inline void saveFocusLast(Bool f) { resource.focus_last = f; }
  inline void iconUpdate(void) { iconmenu->update(); }

#ifdef    HAVE_STRFTIME
  inline char *getStrftimeFormat(void) { return resource.strftime_format; }
  void saveStrftimeFormat(const char *);
#else // !HAVE_STRFTIME
  inline int getDateFormat(void) { return resource.date_format; }
  inline void saveDateFormat(int f) { resource.date_format = f; }
  inline Bool isClock24Hour(void) { return resource.clock24hour; }
  inline void saveClock24Hour(Bool c) { resource.clock24hour = c; }
#endif // HAVE_STRFTIME

  inline WindowStyle *getWindowStyle(void) { return &resource.wstyle; }
  inline MenuStyle *getMenuStyle(void) { return &resource.mstyle; }
  inline ToolbarStyle *getToolbarStyle(void) { return &resource.tstyle; }

  OpenboxWindow *getIcon(int);

  int addWorkspace(void);
  int removeLastWorkspace(void);

  void removeWorkspaceNames(void);
  void addWorkspaceName(const char *);
  void addNetizen(Netizen *);
  void removeNetizen(Window);
  void addIcon(OpenboxWindow *);
  void removeIcon(OpenboxWindow *);
  char* getNameOfWorkspace(int);
  void changeWorkspaceID(int);
  void raiseWindows(Window *, int);
  void reassociateWindow(OpenboxWindow *, int, Bool);
  void prevFocus(void);
  void nextFocus(void);
  void raiseFocus(void);
  void reconfigure(void);
  void rereadMenu(void);
  void shutdown(void);
  void showPosition(int, int);
  void showGeometry(unsigned int, unsigned int);
  void hideGeometry(void);

  void updateNetizenCurrentWorkspace(void);
  void updateNetizenWorkspaceCount(void);
  void updateNetizenWindowFocus(void);
  void updateNetizenWindowAdd(Window, unsigned long);
  void updateNetizenWindowDel(Window);
  void updateNetizenConfigNotify(XEvent *);
  void updateNetizenWindowRaise(Window);
  void updateNetizenWindowLower(Window);

  enum { RowSmartPlacement = 1, ColSmartPlacement, CascadePlacement, LeftRight,
         RightLeft, TopBottom, BottomTop };
  enum { LeftJustify = 1, RightJustify, CenterJustify };
  enum { RoundBullet = 1, TriangleBullet, SquareBullet, NoBullet };
  enum { Restart = 1, RestartOther, Exit, Shutdown, Execute, Reconfigure,
         WindowShade, WindowIconify, WindowMaximize, WindowClose, WindowRaise,
         WindowLower, WindowStick, WindowKill, SetStyle };
};


#endif // __Screen_hh
