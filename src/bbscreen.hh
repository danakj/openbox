// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
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

#include "otk/color.hh"
#include "otk/font.hh"
#include "otk/texture.hh"
#include "otk/image.hh"
#include "otk/strut.hh"
#include "otk/property.hh"
#include "otk/configuration.hh"
#include "otk/style.hh"
#include "timer.hh"
#include "workspace.hh"
#include "blackbox.hh"

namespace ob {

class BScreen : public otk::ScreenInfo {
private:
  bool root_colormap_installed, managed, geom_visible;
  GC opGC;
  Pixmap geom_pixmap;
  Window geom_window;

  Blackbox *blackbox;
  otk::BImageControl *image_control;
  otk::Configuration *config;
  otk::OBProperty *xatom;

  BlackboxWindowList iconList, windowList;

  typedef std::vector<Window> WindowList;
  WindowList specialWindowList, desktopWindowList, systrayWindowList;

  Workspace *current_workspace;

  unsigned int geom_w, geom_h;
  unsigned long event_mask;

  otk::Rect usableArea;
#ifdef    XINERAMA
  RectList xineramaUsableArea;
#endif // XINERAMA

  typedef std::list<otk::Strut*> StrutList;
  StrutList strutList;
  typedef std::vector<Workspace*> WorkspaceList;
  WorkspaceList workspacesList;

  struct screen_resource {
    otk::Style wstyle;

    bool sloppy_focus, auto_raise, auto_edge_balance, ordered_dither,
      opaque_move, full_max, focus_new, focus_last, click_raise,
      allow_scroll_lock, window_corner_snap, aa_fonts,
      ignore_shaded, ignore_maximized, workspace_warping, shadow_fonts;

    int snap_to_windows, snap_to_edges;
    unsigned int snap_offset;

    unsigned int workspaces;
    int placement_policy,
      snap_threshold, row_direction, col_direction, root_scroll,
      resistance_size;

    unsigned int resize_zones;

    std::string strftime_format;

  } resource;
  std::string screenstr;

  BScreen(const BScreen&);
  BScreen& operator=(const BScreen&);

  void updateWorkArea(void);

public:
  // XXX: temporary
  void updateNetizenWorkspaceCount();
  void updateNetizenWindowFocus();
  

  enum { WindowNoSnap = 0, WindowSnap, WindowResistance };
  enum { RowSmartPlacement = 1, ColSmartPlacement, CascadePlacement,
         UnderMousePlacement, ClickMousePlacement, LeftRight, RightLeft,
         TopBottom, BottomTop, IgnoreShaded, IgnoreMaximized };
  enum { Restart = 1, RestartOther, Exit, Shutdown, Execute, Reconfigure,
         WindowShade, WindowIconify, WindowMaximize, WindowClose, WindowRaise,
         WindowLower, WindowStick, WindowKill, SetStyle };
  enum FocusModel { SloppyFocus, ClickToFocus };
  enum RootScrollDirection { NoScroll = 0, NormalScroll, ReverseScroll };

  BScreen(Blackbox *bb, unsigned int scrn);
  ~BScreen(void);

  void LoadStyle(void);

  inline bool isSloppyFocus(void) const { return resource.sloppy_focus; }
  inline bool isRootColormapInstalled(void) const
  { return root_colormap_installed; }
  inline bool doAutoRaise(void) const { return resource.auto_raise; }
  inline bool doClickRaise(void) const { return resource.click_raise; }
  inline bool isScreenManaged(void) const { return managed; }
  inline bool doShadowFonts(void) const { return resource.shadow_fonts; }
  inline bool doAAFonts(void) const { return resource.aa_fonts; }
  inline bool doImageDither(void) const { return image_control->doDither(); }
  inline bool doOrderedDither(void) const { return resource.ordered_dither; }
  inline bool doOpaqueMove(void) const { return resource.opaque_move; }
  inline bool doFullMax(void) const { return resource.full_max; }
  inline bool doFocusNew(void) const { return resource.focus_new; }
  inline bool doFocusLast(void) const { return resource.focus_last; }
  inline int getWindowToWindowSnap(void) const
    { return resource.snap_to_windows; }
  inline int getWindowToEdgeSnap(void) const
    { return resource.snap_to_edges; }
  inline bool getWindowCornerSnap(void) const
    { return resource.window_corner_snap; }
  inline bool allowScrollLock(void) const { return resource.allow_scroll_lock; }
  inline bool doWorkspaceWarping(void) const
    { return resource.workspace_warping; }
  inline int rootScrollDirection(void) const { return resource.root_scroll; }

  inline const GC &getOpGC(void) const { return opGC; }

  inline Blackbox *getBlackbox(void) { return blackbox; }
  inline otk::BColor *getBorderColor(void) {
    return &resource.wstyle.border_color;
  }
  inline otk::BImageControl *getImageControl(void) { return image_control; }

  Workspace *getWorkspace(unsigned int index) const;

  inline Workspace *getCurrentWorkspace(void) { return current_workspace; }

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
  inline int getSnapOffset(void) const
  { return resource.snap_offset; }
  inline int getSnapThreshold(void) const
  { return resource.snap_threshold; }
  inline int getResistanceSize(void) const
  { return resource.resistance_size; }
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
  void saveSnapThreshold(int t);
  void saveSnapOffset(int o);
  void saveResistanceSize(int s);
  void saveImageDither(bool d);
  void saveShadowFonts(bool f);
  void saveAAFonts(bool f);
  void saveOpaqueMove(bool o);
  void saveFullMax(bool f);
  void saveFocusNew(bool f);
  void saveFocusLast(bool f);
  void saveWindowToEdgeSnap(int s);
  void saveWindowToWindowSnap(int s);
  void saveWindowCornerSnap(bool s);
  void saveResizeZones(unsigned int z);
  void savePlaceIgnoreShaded(bool i);
  void savePlaceIgnoreMaximized(bool i);
  void saveAllowScrollLock(bool a);
  void saveWorkspaceWarping(bool w);
  void saveRootScrollDirection(int d);

  inline const char *getStrftimeFormat(void)
  { return resource.strftime_format.c_str(); }
  void saveStrftimeFormat(const std::string& format);

  inline otk::Style *getWindowStyle(void) { return &resource.wstyle; }

  BlackboxWindow *getIcon(unsigned int index);

  // allAvailableAreas should be used whenever possible instead of this function
  // as then Xinerama will work correctly.
  const otk::Rect& availableArea(void) const;
#ifdef    XINERAMA
  const RectList& allAvailableAreas(void) const;
#endif // XINERAMA
  void updateAvailableArea(void);
  void addStrut(otk::Strut *strut);
  void removeStrut(otk::Strut *strut);

  unsigned int addWorkspace(void);
  unsigned int removeLastWorkspace(void);
  void changeWorkspaceID(unsigned int id);
  void saveWorkspaceNames(void);

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
  void prevFocus(void) const;
  void nextFocus(void) const;
  void raiseFocus(void) const;
  void load_rc(void);
  void save_rc(void);
  void reconfigure(void);
  void toggleFocusModel(FocusModel model);
  void shutdown(void);
  void showPosition(int x, int y);
  void showGeometry(unsigned int gx, unsigned int gy);
  void hideGeometry(void);

  void buttonPressEvent(const XButtonEvent *xbutton);
  void propertyNotifyEvent(const XPropertyEvent *pe);
};

}

#endif // __Screen_hh
