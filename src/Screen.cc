// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Screen.cc for Blackbox - an X11 Window manager
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

#ifdef    HAVE_CONFIG_H
#include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <X11/Xatom.h>
#include <X11/keysym.h>

#ifdef    XINERAMA
#  include <X11/Xlib.h>
#  include <X11/extensions/Xinerama.h>
#endif // XINERAMA

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#ifdef    HAVE_CTYPE_H
#  include <ctype.h>
#endif // HAVE_CTYPE_H

#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef    HAVE_DIRENT_H
#  include <dirent.h>
#endif // HAVE_DIRENT_H

#ifdef    HAVE_LOCALE_H
#  include <locale.h>
#endif // HAVE_LOCALE_H

#ifdef    HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif // HAVE_SYS_STAT_H

#ifdef    HAVE_STDARG_H
#  include <stdarg.h>
#endif // HAVE_STDARG_H
}

#include <assert.h>

#include <algorithm>
#include <functional>
#include <string>
using std::string;

#include "i18n.hh"
#include "blackbox.hh"
#include "Clientmenu.hh"
#include "Font.hh"
#include "GCCache.hh"
#include "Iconmenu.hh"
#include "Image.hh"
#include "Screen.hh"
#include "Slit.hh"
#include "Rootmenu.hh"
#include "Toolbar.hh"
#include "Util.hh"
#include "Window.hh"
#include "Workspace.hh"
#include "Workspacemenu.hh"
#include "Util.hh"
#include "XAtom.hh"

#ifndef   FONT_ELEMENT_SIZE
#define   FONT_ELEMENT_SIZE 50
#endif // FONT_ELEMENT_SIZE


static bool running = True;

static int anotherWMRunning(Display *display, XErrorEvent *) {
  fprintf(stderr, i18n(ScreenSet, ScreenAnotherWMRunning,
          "BScreen::BScreen: an error occured while querying the X server.\n"
          "  another window manager already running on display %s.\n"),
          DisplayString(display));

  running = False;

  return(-1);
}


BScreen::BScreen(Blackbox *bb, unsigned int scrn) : ScreenInfo(bb, scrn) {
  blackbox = bb;
  screenstr = "session.screen" + itostring(scrn) + '.';
  config = blackbox->getConfig();
  xatom = blackbox->getXAtom();

  event_mask = ColormapChangeMask | EnterWindowMask | PropertyChangeMask |
    SubstructureRedirectMask | ButtonPressMask | ButtonReleaseMask;

  XErrorHandler old = XSetErrorHandler((XErrorHandler) anotherWMRunning);
  XSelectInput(getBaseDisplay()->getXDisplay(), getRootWindow(), event_mask);
  XSync(getBaseDisplay()->getXDisplay(), False);
  XSetErrorHandler((XErrorHandler) old);

  managed = running;
  if (! managed) return;

  fprintf(stderr, i18n(ScreenSet, ScreenManagingScreen,
                       "BScreen::BScreen: managing screen %d "
                       "using visual 0x%lx, depth %d\n"),
          getScreenNumber(), XVisualIDFromVisual(getVisual()),
          getDepth());

  rootmenu = 0;

  resource.mstyle.t_font = resource.mstyle.f_font = resource.tstyle.font =
    resource.wstyle.font = (BFont *) 0;

  geom_pixmap = None;

  xatom->setSupported(this);    // set-up netwm support
#ifdef    HAVE_GETPID
  xatom->setValue(getRootWindow(), XAtom::blackbox_pid, XAtom::cardinal,
                  (unsigned long) getpid());
#endif // HAVE_GETPID
  unsigned long geometry[] = { getWidth(),
                               getHeight()};
  xatom->setValue(getRootWindow(), XAtom::net_desktop_geometry,
                  XAtom::cardinal, geometry, 2);
  unsigned long viewport[] = {0,0};
  xatom->setValue(getRootWindow(), XAtom::net_desktop_viewport,
                  XAtom::cardinal, viewport, 2);
                  

  XDefineCursor(blackbox->getXDisplay(), getRootWindow(),
                blackbox->getSessionCursor());

  updateAvailableArea();

  image_control =
    new BImageControl(blackbox, this, True, blackbox->getColorsPerChannel(),
                      blackbox->getCacheLife(), blackbox->getCacheMax());
  image_control->installRootColormap();
  root_colormap_installed = True;

  load_rc();
  LoadStyle();

  XGCValues gcv;
  gcv.foreground = WhitePixel(blackbox->getXDisplay(), getScreenNumber())
    ^ BlackPixel(blackbox->getXDisplay(), getScreenNumber());
  gcv.function = GXxor;
  gcv.subwindow_mode = IncludeInferiors;
  opGC = XCreateGC(blackbox->getXDisplay(), getRootWindow(),
                   GCForeground | GCFunction | GCSubwindowMode, &gcv);

  const char *s =  i18n(ScreenSet, ScreenPositionLength,
                        "0: 0000 x 0: 0000");
  geom_w = resource.wstyle.font->measureString(s) + resource.bevel_width * 2;
  geom_h = resource.wstyle.font->height() + resource.bevel_width * 2;

  XSetWindowAttributes attrib;
  unsigned long mask = CWBorderPixel | CWColormap | CWSaveUnder;
  attrib.border_pixel = getBorderColor()->pixel();
  attrib.colormap = getColormap();
  attrib.save_under = True;

  geom_window = XCreateWindow(blackbox->getXDisplay(), getRootWindow(),
                              0, 0, geom_w, geom_h, resource.border_width,
                              getDepth(), InputOutput, getVisual(),
                              mask, &attrib);
  geom_visible = False;

  BTexture* texture = &(resource.wstyle.l_focus);
  geom_pixmap = texture->render(geom_w, geom_h, geom_pixmap);
  if (geom_pixmap == ParentRelative) {
    texture = &(resource.wstyle.t_focus);
    geom_pixmap = texture->render(geom_w, geom_h, geom_pixmap);
  }
  if (! geom_pixmap)
    XSetWindowBackground(blackbox->getXDisplay(), geom_window,
                         texture->color().pixel());
  else
    XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                               geom_window, geom_pixmap);

  workspacemenu = new Workspacemenu(this);
  iconmenu = new Iconmenu(this);
  configmenu = new Configmenu(this);

  if (resource.workspaces > 0) {
    for (unsigned int i = 0; i < resource.workspaces; ++i) {
      Workspace *wkspc = new Workspace(this, workspacesList.size());
      workspacesList.push_back(wkspc);
      workspacemenu->insertWorkspace(wkspc);
      workspacemenu->update();

    }
  } else {
    Workspace *wkspc = new Workspace(this, workspacesList.size());
    workspacesList.push_back(wkspc);
    workspacemenu->insertWorkspace(wkspc);
    workspacemenu->update();
  }
  saveWorkspaceNames();

  updateNetizenWorkspaceCount();

  workspacemenu->insert(i18n(IconSet, IconIcons, "Icons"), iconmenu);
  workspacemenu->update();

  current_workspace = workspacesList.front();
  
  xatom->setValue(getRootWindow(), XAtom::net_current_desktop,
                  XAtom::cardinal, 0); //first workspace

  workspacemenu->setItemSelected(2, True);

  toolbar = new Toolbar(this);

  slit = new Slit(this);

  InitMenu();

  raiseWindows(0, 0);     // this also initializes the empty stacking list
  rootmenu->update();

  updateClientList();     // initialize the client lists, which will be empty
  updateAvailableArea();

  changeWorkspaceID(0);

  unsigned int i, j, nchild;
  Window r, p, *children;
  XQueryTree(blackbox->getXDisplay(), getRootWindow(), &r, &p,
             &children, &nchild);

  // preen the window list of all icon windows... for better dockapp support
  for (i = 0; i < nchild; i++) {
    if (children[i] == None) continue;

    XWMHints *wmhints = XGetWMHints(blackbox->getXDisplay(),
                                    children[i]);

    if (wmhints) {
      if ((wmhints->flags & IconWindowHint) &&
          (wmhints->icon_window != children[i])) {
        for (j = 0; j < nchild; j++) {
          if (children[j] == wmhints->icon_window) {
            children[j] = None;
            break;
          }
        }
      }

      XFree(wmhints);
    }
  }

  // manage shown windows
  for (i = 0; i < nchild; ++i) {
    if (children[i] == None || ! blackbox->validateWindow(children[i]))
      continue;

    XWindowAttributes attrib;
    if (XGetWindowAttributes(blackbox->getXDisplay(), children[i], &attrib)) {
      if (attrib.override_redirect) continue;

      if (attrib.map_state != IsUnmapped) {
        manageWindow(children[i]);
      }
    }
  }

  XFree(children);

  // call this again just in case a window we found updates the Strut list
  updateAvailableArea();
}


BScreen::~BScreen(void) {
  if (! managed) return;

  if (geom_pixmap != None)
    image_control->removeImage(geom_pixmap);

  if (geom_window != None)
    XDestroyWindow(blackbox->getXDisplay(), geom_window);

  std::for_each(workspacesList.begin(), workspacesList.end(),
                PointerAssassin());

  std::for_each(iconList.begin(), iconList.end(), PointerAssassin());

  std::for_each(netizenList.begin(), netizenList.end(), PointerAssassin());

  while (! systrayWindowList.empty())
    removeSystrayWindow(systrayWindowList[0]);

  delete rootmenu;
  delete workspacemenu;
  delete iconmenu;
  delete configmenu;
  delete slit;
  delete toolbar;
  delete image_control;

  if (resource.wstyle.font)
    delete resource.wstyle.font;
  if (resource.mstyle.t_font)
    delete resource.mstyle.t_font;
  if (resource.mstyle.f_font)
    delete resource.mstyle.f_font;
  if (resource.tstyle.font)
    delete resource.tstyle.font;

  XFreeGC(blackbox->getXDisplay(), opGC);
}


void BScreen::saveSloppyFocus(bool s) {
  resource.sloppy_focus = s;

  string fmodel;
  if (resource.sloppy_focus) {
    fmodel = "SloppyFocus";
    if (resource.auto_raise) fmodel += " AutoRaise";
    if (resource.click_raise) fmodel += " ClickRaise";
  } else {
    fmodel = "ClickToFocus";
  }
  config->setValue(screenstr + "focusModel", fmodel);
}


void BScreen::saveAutoRaise(bool a) {
  resource.auto_raise = a;
  saveSloppyFocus(resource.sloppy_focus);
}


void BScreen::saveClickRaise(bool c) {
  resource.click_raise = c;
  saveSloppyFocus(resource.sloppy_focus);
}


void BScreen::saveImageDither(bool d) {
  image_control->setDither(d);
  config->setValue(screenstr + "imageDither", doImageDither());
}


void BScreen::saveOpaqueMove(bool o) {
  resource.opaque_move = o;
  config->setValue(screenstr + "opaqueMove", resource.opaque_move);
}


void BScreen::saveFullMax(bool f) {
  resource.full_max = f;
  config->setValue(screenstr + "fullMaximization", resource.full_max);
}


void BScreen::saveFocusNew(bool f) {
  resource.focus_new = f;
  config->setValue(screenstr + "focusNewWindows", resource.focus_new);
}


void BScreen::saveFocusLast(bool f) {
  resource.focus_last = f;
  config->setValue(screenstr + "focusLastWindow", resource.focus_last);
}


void BScreen::saveAAFonts(bool f) {
  resource.aa_fonts = f;
  reconfigure();
  config->setValue(screenstr + "antialiasFonts", resource.aa_fonts);
}


void BScreen::saveHideToolbar(bool h) {
  resource.hide_toolbar = h;
  if (resource.hide_toolbar)
    toolbar->unmapToolbar();
  else
    toolbar->mapToolbar();
  config->setValue(screenstr + "hideToolbar", resource.hide_toolbar);
}


void BScreen::saveWindowToWindowSnap(bool s) {
  resource.window_to_window_snap = s;
  config->setValue(screenstr + "windowToWindowSnap",
                   resource.window_to_window_snap);
}


void BScreen::saveResizeZones(unsigned int z) {
  resource.resize_zones = z;
  config->setValue(screenstr + "resizeZones", resource.resize_zones);
}


void BScreen::saveWindowCornerSnap(bool s) {
  resource.window_corner_snap = s;
  config->setValue(screenstr + "windowCornerSnap",
                   resource.window_corner_snap);
}


void BScreen::saveWorkspaces(unsigned int w) {
  resource.workspaces = w;
  config->setValue(screenstr + "workspaces", resource.workspaces);
}


void BScreen::savePlacementPolicy(int p) {
  resource.placement_policy = p; 
  const char *placement;
  switch (resource.placement_policy) {
  case CascadePlacement: placement = "CascadePlacement"; break;
  case UnderMousePlacement: placement = "UnderMousePlacement"; break;
  case ClickMousePlacement: placement = "ClickMousePlacement"; break;
  case ColSmartPlacement: placement = "ColSmartPlacement"; break;
  case RowSmartPlacement: default: placement = "RowSmartPlacement"; break;
  }
  config->setValue(screenstr + "windowPlacement", placement);
}


void BScreen::saveEdgeSnapThreshold(int t) {
  resource.edge_snap_threshold = t;
  config->setValue(screenstr + "edgeSnapThreshold",
                   resource.edge_snap_threshold);
}


void BScreen::saveRowPlacementDirection(int d) {
  resource.row_direction = d;
  config->setValue(screenstr + "rowPlacementDirection",
                   resource.row_direction == LeftRight ?
                   "LeftToRight" : "RightToLeft");
}


void BScreen::saveColPlacementDirection(int d) {
  resource.col_direction = d;
  config->setValue(screenstr + "colPlacementDirection",
                   resource.col_direction == TopBottom ?
                   "TopToBottom" : "BottomToTop");
}


#ifdef    HAVE_STRFTIME
void BScreen::saveStrftimeFormat(const std::string& format) {
  resource.strftime_format = format;
  config->setValue(screenstr + "strftimeFormat", resource.strftime_format);
}

#else // !HAVE_STRFTIME

void BScreen::saveDateFormat(int f) {
  resource.date_format = f;
  config->setValue(screenstr + "dateFormat",
                   resource.date_format == B_EuropeanDate ?
                   "European" : "American");
}


void BScreen::saveClock24Hour(bool c) {
  resource.clock24hour = c;
  config->setValue(screenstr + "clockFormat", resource.clock24hour ? 24 : 12);
}
#endif // HAVE_STRFTIME


void BScreen::saveWorkspaceNames() {
  string names;
 
  for (unsigned int i = 0; i < workspacesList.size(); ++i) {
    names += workspacesList[i]->getName();
    if (i < workspacesList.size() - 1)
      names += ',';
  }

  config->setValue(screenstr + "workspaceNames", names);
}


void BScreen::savePlaceIgnoreShaded(bool i) {
  resource.ignore_shaded = i;
  config->setValue(screenstr + "placementIgnoreShaded",
                   resource.ignore_shaded);
}


void BScreen::savePlaceIgnoreMaximized(bool i) {
  resource.ignore_maximized = i;
  config->setValue(screenstr + "placementIgnoreMaximized",
                   resource.ignore_maximized);
}


void BScreen::saveAllowScrollLock(bool a) {
  resource.allow_scroll_lock = a;
  config->setValue(screenstr + "disableBindingsWithScrollLock",
                   resource.allow_scroll_lock);
}


void BScreen::saveWorkspaceWarping(bool w) {
  resource.workspace_warping = w;
  config->setValue(screenstr + "workspaceWarping",
                   resource.workspace_warping);
}


void BScreen::saveRootScrollDirection(int d) {
  resource.root_scroll = d;
  const char *dir;
  switch (resource.root_scroll) {
  case NoScroll: dir = "None"; break;
  case ReverseScroll: dir = "Reverse"; break;
  case NormalScroll: default: dir = "Normal"; break;
  }
  config->setValue(screenstr + "rootScrollDirection", dir);
}


void BScreen::save_rc(void) {
  saveSloppyFocus(resource.sloppy_focus);
  saveAutoRaise(resource.auto_raise);
  saveImageDither(doImageDither());
  saveAAFonts(resource.aa_fonts);
  saveResizeZones(resource.resize_zones);
  saveOpaqueMove(resource.opaque_move);
  saveFullMax(resource.full_max);
  saveFocusNew(resource.focus_new);
  saveFocusLast(resource.focus_last);
  saveHideToolbar(resource.hide_toolbar);
  saveWindowToWindowSnap(resource.window_to_window_snap);
  saveWindowCornerSnap(resource.window_corner_snap);
  saveWorkspaces(resource.workspaces);
  savePlacementPolicy(resource.placement_policy);
  saveEdgeSnapThreshold(resource.edge_snap_threshold);
  saveRowPlacementDirection(resource.row_direction);
  saveColPlacementDirection(resource.col_direction);
#ifdef    HAVE_STRFTIME
  saveStrftimeFormat(resource.strftime_format); 
#else // !HAVE_STRFTIME
  saveDateFormat(resource.date_format);
  savwClock24Hour(resource.clock24hour);
#endif // HAVE_STRFTIME
  savePlaceIgnoreShaded(resource.ignore_shaded);
  savePlaceIgnoreMaximized(resource.ignore_maximized);
  saveAllowScrollLock(resource.allow_scroll_lock);
  saveWorkspaceWarping(resource.workspace_warping);
  saveRootScrollDirection(resource.root_scroll);

  toolbar->save_rc();
  slit->save_rc();
}


void BScreen::load_rc(void) {
  std::string s;
  bool b;

  if (! config->getValue(screenstr + "fullMaximization", resource.full_max))
    resource.full_max = false;

  if (! config->getValue(screenstr + "focusNewWindows", resource.focus_new))
    resource.focus_new = false;

  if (! config->getValue(screenstr + "focusLastWindow", resource.focus_last))
    resource.focus_last = false;

  if (! config->getValue(screenstr + "workspaces", resource.workspaces))
    resource.workspaces = 1;

  if (! config->getValue(screenstr + "opaqueMove", resource.opaque_move))
    resource.opaque_move = false;

  if (! config->getValue(screenstr + "antialiasFonts", resource.aa_fonts))
    resource.aa_fonts = true;

  if (! config->getValue(screenstr + "resizeZones", resource.resize_zones) ||
      (resource.resize_zones != 1 && resource.resize_zones != 2 &&
       resource.resize_zones != 4))
      resource.resize_zones = 4;

  if (! config->getValue(screenstr + "hideToolbar", resource.hide_toolbar))
    resource.hide_toolbar = false;

  if (! config->getValue(screenstr + "windowToWindowSnap",
                         resource.window_to_window_snap))
    resource.window_to_window_snap = true;

  if (! config->getValue(screenstr + "windowCornerSnap",
                         resource.window_corner_snap))
    resource.window_corner_snap = true;

  if (! config->getValue(screenstr + "imageDither", b))
    b = true;
  image_control->setDither(b);

  if (! config->getValue(screenstr + "edgeSnapThreshold",
                        resource.edge_snap_threshold))
    resource.edge_snap_threshold = 4;
  
  if (config->getValue(screenstr + "rowPlacementDirection", s) &&
      s == "RightToLeft")
    resource.row_direction = RightLeft;
  else
    resource.row_direction = LeftRight;

  if (config->getValue(screenstr + "colPlacementDirection", s) &&
      s == "BottomToTop")
    resource.col_direction = BottomTop;
  else
    resource.col_direction = TopBottom;

  if (config->getValue(screenstr + "workspaceNames", s)) {
    XAtom::StringVect workspaceNames;

    string::const_iterator it = s.begin(), end = s.end();
    while(1) {
      string::const_iterator tmp = it;     // current string.begin()
      it = std::find(tmp, end, ',');       // look for comma between tmp and end
      workspaceNames.push_back(string(tmp, it)); // s[tmp:it]
      if (it == end)
        break;
      ++it;
    }

    xatom->setValue(getRootWindow(), XAtom::net_desktop_names, XAtom::utf8,
                    workspaceNames);
  }

  resource.sloppy_focus = true;
  resource.auto_raise = false;
  resource.click_raise = false;
  if (config->getValue(screenstr + "focusModel", s)) {
    if (s.find("ClickToFocus") != string::npos) {
      resource.sloppy_focus = false;
    } else {
      // must be sloppy
      if (s.find("AutoRaise") != string::npos)
        resource.auto_raise = true;
      if (s.find("ClickRaise") != string::npos)
        resource.click_raise = true;
    }
  }

  if (config->getValue(screenstr + "windowPlacement", s)) {
    if (s == "CascadePlacement")
      resource.placement_policy = CascadePlacement;
    else if (s == "UnderMousePlacement")
      resource.placement_policy = UnderMousePlacement;
    else if (s == "ClickMousePlacement")
      resource.placement_policy = ClickMousePlacement;
    else if (s == "ColSmartPlacement")
      resource.placement_policy = ColSmartPlacement;
    else //if (s == "RowSmartPlacement")
      resource.placement_policy = RowSmartPlacement;
  } else
    resource.placement_policy = RowSmartPlacement;

#ifdef    HAVE_STRFTIME
  if (! config->getValue(screenstr + "strftimeFormat",
                         resource.strftime_format))
    resource.strftime_format = "%I:%M %p";
#else // !HAVE_STRFTIME
  long l;

  if (config->getValue(screenstr + "dateFormat", s) && s == "European")
    resource.date_format = B_EuropeanDate;
 else
    resource.date_format = B_AmericanDate;

  if (! config->getValue(screenstr + "clockFormat", l))
    l = 12;
  resource.clock24hour = l == 24;
#endif // HAVE_STRFTIME
  
  if (! config->getValue(screenstr + "placementIgnoreShaded",
                         resource.ignore_shaded))
    resource.ignore_shaded = true;

  if (! config->getValue(screenstr + "placementIgnoreMaximized",
                         resource.ignore_maximized))
    resource.ignore_maximized = true;

  if (! config->getValue(screenstr + "disableBindingsWithScrollLock",
                       resource.allow_scroll_lock))
    resource.allow_scroll_lock = false;

  if (! config->getValue(screenstr + "workspaceWarping",
                         resource.workspace_warping))
    resource.workspace_warping = false;

  resource.root_scroll = NormalScroll;
  if (config->getValue(screenstr + "rootScrollDirection", s)) {
    if (s == "None")
      resource.root_scroll = NoScroll;
    else if (s == "Reverse")
      resource.root_scroll = ReverseScroll;
  }
}


void BScreen::changeWorkspaceCount(unsigned int new_count) {
  assert(new_count > 0);

  if (new_count < workspacesList.size()) {
    // shrink
    for (unsigned int i = workspacesList.size(); i > new_count; --i)
      removeLastWorkspace();
    // removeLast already sets the current workspace to the 
    // last available one.
  } else if (new_count > workspacesList.size()) {
    // grow
    for(unsigned int i = workspacesList.size(); i < new_count; ++i)
      addWorkspace();
  }
}


void BScreen::reconfigure(void) {
  // don't reconfigure while saving the initial rc file, it's a waste and it
  // breaks somethings (workspace names)
  if (blackbox->isStartup()) return;

  load_rc();
  toolbar->load_rc();
  slit->load_rc();
  LoadStyle();

  // we need to do this explicitly, because just loading this value from the rc
  // does nothing
  changeWorkspaceCount(resource.workspaces);

  XGCValues gcv;
  gcv.foreground = WhitePixel(blackbox->getXDisplay(),
                              getScreenNumber());
  gcv.function = GXinvert;
  gcv.subwindow_mode = IncludeInferiors;
  XChangeGC(blackbox->getXDisplay(), opGC,
            GCForeground | GCFunction | GCSubwindowMode, &gcv);

  const char *s = i18n(ScreenSet, ScreenPositionLength,
                       "0: 0000 x 0: 0000");

  geom_w = resource.wstyle.font->measureString(s) + resource.bevel_width * 2;
  geom_h = resource.wstyle.font->height() + resource.bevel_width * 2;

  BTexture* texture = &(resource.wstyle.l_focus);
  geom_pixmap = texture->render(geom_w, geom_h, geom_pixmap);
  if (geom_pixmap == ParentRelative) {
    texture = &(resource.wstyle.t_focus);
    geom_pixmap = texture->render(geom_w, geom_h, geom_pixmap);
  }
  if (! geom_pixmap)
    XSetWindowBackground(blackbox->getXDisplay(), geom_window,
                         texture->color().pixel());
  else
    XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                               geom_window, geom_pixmap);

  XSetWindowBorderWidth(blackbox->getXDisplay(), geom_window,
                        resource.border_width);
  XSetWindowBorder(blackbox->getXDisplay(), geom_window,
                   resource.border_color.pixel());

  workspacemenu->reconfigure();
  iconmenu->reconfigure();

  typedef std::vector<int> SubList;
  SubList remember_subs;

  // save the current open menus
  Basemenu *menu = rootmenu;
  int submenu;
  while ((submenu = menu->getCurrentSubmenu()) >= 0) {
    remember_subs.push_back(submenu);
    menu = menu->find(submenu)->submenu();
    assert(menu);
  }
  
  InitMenu();
  raiseWindows(0, 0);
  rootmenu->reconfigure();

  // reopen the saved menus
  menu = rootmenu;
  const SubList::iterator subs_end = remember_subs.end();
  for (SubList::iterator it = remember_subs.begin(); it != subs_end; ++it) {
    menu->drawSubmenu(*it);
    menu = menu->find(*it)->submenu();
    if (! menu)
      break;
  }

  configmenu->reconfigure();

  toolbar->reconfigure();

  slit->reconfigure();

  std::for_each(workspacesList.begin(), workspacesList.end(),
                std::mem_fun(&Workspace::reconfigure));

  BlackboxWindowList::iterator iit = iconList.begin();
  for (; iit != iconList.end(); ++iit) {
    BlackboxWindow *bw = *iit;
    if (bw->validateClient())
      bw->reconfigure();
  }

  image_control->timeout();
}


void BScreen::rereadMenu(void) {
  InitMenu();
  raiseWindows(0, 0);

  rootmenu->reconfigure();
}


void BScreen::LoadStyle(void) {
  Configuration style(False);

  const char *sfile = blackbox->getStyleFilename();
  if (sfile != NULL) {
    style.setFile(sfile);
    if (! style.load()) {
      style.setFile(DEFAULTSTYLE);
      if (! style.load())
        style.create();  // hardcoded default values will be used.
    }
  }

  // merge in the rc file
  style.merge(config->file(), True);

  string s;

  // load fonts/fontsets
  if (resource.wstyle.font)
    delete resource.wstyle.font;
  if (resource.tstyle.font)
    delete resource.tstyle.font;
  if (resource.mstyle.f_font)
    delete resource.mstyle.f_font;
  if (resource.mstyle.t_font)
    delete resource.mstyle.t_font;
  resource.wstyle.font = resource.tstyle.font = resource.mstyle.f_font =
    resource.mstyle.t_font = (BFont *) 0;

  resource.wstyle.font = readDatabaseFont("window.", style);
  resource.tstyle.font = readDatabaseFont("toolbar.", style);
  resource.mstyle.t_font = readDatabaseFont("menu.title.", style);
  resource.mstyle.f_font = readDatabaseFont("menu.frame.", style);

  // load window config
  resource.wstyle.t_focus =
    readDatabaseTexture("window.title.focus", "white", style);
  resource.wstyle.t_unfocus =
    readDatabaseTexture("window.title.unfocus", "black", style);
  resource.wstyle.l_focus =
    readDatabaseTexture("window.label.focus", "white", style);
  resource.wstyle.l_unfocus =
    readDatabaseTexture("window.label.unfocus", "black", style);
  resource.wstyle.h_focus =
    readDatabaseTexture("window.handle.focus", "white", style);
  resource.wstyle.h_unfocus =
    readDatabaseTexture("window.handle.unfocus", "black", style);
  resource.wstyle.g_focus =
    readDatabaseTexture("window.grip.focus", "white", style);
  resource.wstyle.g_unfocus =
    readDatabaseTexture("window.grip.unfocus", "black", style);
  resource.wstyle.b_focus =
    readDatabaseTexture("window.button.focus", "white", style);
  resource.wstyle.b_unfocus =
    readDatabaseTexture("window.button.unfocus", "black", style);
  resource.wstyle.b_pressed =
    readDatabaseTexture("window.button.pressed", "black", style);
  resource.wstyle.f_focus =
    readDatabaseColor("window.frame.focusColor", "white", style);
  resource.wstyle.f_unfocus =
    readDatabaseColor("window.frame.unfocusColor", "black", style);
  resource.wstyle.l_text_focus =
    readDatabaseColor("window.label.focus.textColor", "black", style);
  resource.wstyle.l_text_unfocus =
    readDatabaseColor("window.label.unfocus.textColor", "white", style);
  resource.wstyle.b_pic_focus =
    readDatabaseColor("window.button.focus.picColor", "black", style);
  resource.wstyle.b_pic_unfocus =
    readDatabaseColor("window.button.unfocus.picColor", "white", style);

  resource.wstyle.justify = LeftJustify;
  if (style.getValue("window.justify", s)) {
    if (s == "right" || s == "Right")
      resource.wstyle.justify = RightJustify;
    else if (s == "center" || s == "Center")
      resource.wstyle.justify = CenterJustify;
  }

  // load toolbar config
  resource.tstyle.toolbar =
    readDatabaseTexture("toolbar", "black", style);
  resource.tstyle.label =
    readDatabaseTexture("toolbar.label", "black", style);
  resource.tstyle.window =
    readDatabaseTexture("toolbar.windowLabel", "black", style);
  resource.tstyle.button =
    readDatabaseTexture("toolbar.button", "white", style);
  resource.tstyle.pressed =
    readDatabaseTexture("toolbar.button.pressed", "black", style);
  resource.tstyle.clock =
    readDatabaseTexture("toolbar.clock", "black", style);
  resource.tstyle.l_text =
    readDatabaseColor("toolbar.label.textColor", "white", style);
  resource.tstyle.w_text =
    readDatabaseColor("toolbar.windowLabel.textColor", "white", style);
  resource.tstyle.c_text =
    readDatabaseColor("toolbar.clock.textColor", "white", style);
  resource.tstyle.b_pic =
    readDatabaseColor("toolbar.button.picColor", "black", style);

  resource.tstyle.justify = LeftJustify;
  if (style.getValue("toolbar.justify", s)) {
    if (s == "right" || s == "Right")
      resource.tstyle.justify = RightJustify;
    else if (s == "center" || s == "Center")
      resource.tstyle.justify = CenterJustify;
  }

  // load menu config
  resource.mstyle.title =
    readDatabaseTexture("menu.title", "white", style);
  resource.mstyle.frame =
    readDatabaseTexture("menu.frame", "black", style);
  resource.mstyle.hilite =
    readDatabaseTexture("menu.hilite", "white", style);
  resource.mstyle.t_text =
    readDatabaseColor("menu.title.textColor", "black", style);
  resource.mstyle.f_text =
    readDatabaseColor("menu.frame.textColor", "white", style);
  resource.mstyle.d_text =
    readDatabaseColor("menu.frame.disableColor", "black", style);
  resource.mstyle.h_text =
    readDatabaseColor("menu.hilite.textColor", "black", style);

  resource.mstyle.t_justify = LeftJustify;
  if (style.getValue("menu.title.justify", s)) {
    if (s == "right" || s == "Right")
      resource.mstyle.t_justify = RightJustify;
    else if (s == "center" || s == "Center")
      resource.mstyle.t_justify = CenterJustify;
  }

  resource.mstyle.f_justify = LeftJustify;
  if (style.getValue("menu.frame.justify", s)) {
    if (s == "right" || s == "Right")
      resource.mstyle.f_justify = RightJustify;
    else if (s == "center" || s == "Center")
      resource.mstyle.f_justify = CenterJustify;
  }

  resource.mstyle.bullet = Basemenu::Triangle;
  if (style.getValue("menu.bullet", s)) {
    if (s == "empty" || s == "Empty")
      resource.mstyle.bullet = Basemenu::Empty;
    else if (s == "square" || s == "Square")
      resource.mstyle.bullet = Basemenu::Square;
    else if (s == "diamond" || s == "Diamond")
      resource.mstyle.bullet = Basemenu::Diamond;
  }

  resource.mstyle.bullet_pos = Basemenu::Left;
  if (style.getValue("menu.bullet.position", s)) {
    if (s == "right" || s == "Right")
      resource.mstyle.bullet_pos = Basemenu::Right;
  }

  resource.border_color =
    readDatabaseColor("borderColor", "black", style);

  // load bevel, border and handle widths
  if (! style.getValue("handleWidth", resource.handle_width) ||
      resource.handle_width > (getWidth() / 2) || resource.handle_width == 0)
    resource.handle_width = 6;

  if (! style.getValue("borderWidth", resource.border_width))
    resource.border_width = 1;

  if (! style.getValue("bevelWidth", resource.bevel_width) ||
      resource.bevel_width > (getWidth() / 2) || resource.bevel_width == 0)
    resource.bevel_width = 3;

  if (! style.getValue("frameWidth", resource.frame_width) ||
      resource.frame_width > (getWidth() / 2))
    resource.frame_width = resource.bevel_width;

  if (style.getValue("rootCommand", s))
    bexec(s, displayString());
}


void BScreen::addIcon(BlackboxWindow *w) {
  if (! w) return;

  w->setWorkspace(BSENTINEL);
  w->setWindowNumber(iconList.size());

  iconList.push_back(w);

  const char* title = w->getIconTitle();
  iconmenu->insert(title);
  iconmenu->update();
}


void BScreen::removeIcon(BlackboxWindow *w) {
  if (! w) return;

  iconList.remove(w);

  iconmenu->remove(w->getWindowNumber());
  iconmenu->update();

  BlackboxWindowList::iterator it = iconList.begin(),
    end = iconList.end();
  for (int i = 0; it != end; ++it)
    (*it)->setWindowNumber(i++);
}


BlackboxWindow *BScreen::getIcon(unsigned int index) {
  if (index < iconList.size()) {
    BlackboxWindowList::iterator it = iconList.begin();
    for (; index > 0; --index, ++it) ; /* increment to index */
    return *it;
  }

  return (BlackboxWindow *) 0;
}


unsigned int BScreen::addWorkspace(void) {
  Workspace *wkspc = new Workspace(this, workspacesList.size());
  workspacesList.push_back(wkspc);
  saveWorkspaces(getWorkspaceCount());
  saveWorkspaceNames();

  workspacemenu->insertWorkspace(wkspc);
  workspacemenu->update();

  toolbar->reconfigure();

  updateNetizenWorkspaceCount();

  return workspacesList.size();
}


unsigned int BScreen::removeLastWorkspace(void) {
  if (workspacesList.size() == 1)
    return 1;

  Workspace *wkspc = workspacesList.back();

  if (current_workspace->getID() == wkspc->getID())
    changeWorkspaceID(current_workspace->getID() - 1);

  wkspc->removeAll();

  workspacemenu->removeWorkspace(wkspc);
  workspacemenu->update();

  workspacesList.pop_back();
  delete wkspc;

  saveWorkspaces(getWorkspaceCount());
  saveWorkspaceNames();

  toolbar->reconfigure();

  updateNetizenWorkspaceCount();

  return workspacesList.size();
}


void BScreen::changeWorkspaceID(unsigned int id) {
  if (! current_workspace || id == current_workspace->getID()) return;

  BlackboxWindow *focused = blackbox->getFocusedWindow();
  if (focused && focused->getScreen() == this) {
    assert(focused->isStuck() ||
           focused->getWorkspaceNumber() == current_workspace->getID());

    current_workspace->setLastFocusedWindow(focused);
  } else {
    // if no window had focus, no need to store a last focus
    current_workspace->setLastFocusedWindow((BlackboxWindow *) 0);
  }

  // when we switch workspaces, unfocus whatever was focused
  blackbox->setFocusedWindow((BlackboxWindow *) 0);
    
  current_workspace->hideAll();
  workspacemenu->setItemSelected(current_workspace->getID() + 2, False);

  current_workspace = getWorkspace(id);

  xatom->setValue(getRootWindow(), XAtom::net_current_desktop,
                  XAtom::cardinal, id);

  workspacemenu->setItemSelected(current_workspace->getID() + 2, True);
  toolbar->redrawWorkspaceLabel(True);

  current_workspace->showAll();

  if (resource.focus_last && current_workspace->getLastFocusedWindow()) {
    XSync(blackbox->getXDisplay(), False);
    current_workspace->getLastFocusedWindow()->setInputFocus();
  }

  updateNetizenCurrentWorkspace();
}


/*
 * Set the _NET_CLIENT_LIST root window property.
 */
void BScreen::updateClientList(void) {
  if (windowList.size() > 0) {
    Window *windows = new Window[windowList.size()];
    Window *win_it = windows;
    BlackboxWindowList::iterator it = windowList.begin();
    const BlackboxWindowList::iterator end = windowList.end();
    for (; it != end; ++it, ++win_it)
      *win_it = (*it)->getClientWindow();
    xatom->setValue(getRootWindow(), XAtom::net_client_list, XAtom::window,
                    windows, windowList.size());
    delete [] windows;
  } else
    xatom->setValue(getRootWindow(), XAtom::net_client_list, XAtom::window,
                    0, 0);

  updateStackingList();
}


/*
 * Set the _NET_CLIENT_LIST_STACKING root window property.
 */
void BScreen::updateStackingList(void) {

  BlackboxWindowList stack_order;

  /*
   * Get the stacking order from all of the workspaces.
   * We start with the current workspace so that the sticky windows will be
   * in the right order on the current workspace.
   * XXX: Do we need to have sticky windows in the list once for each workspace?
   */
  getCurrentWorkspace()->appendStackOrder(stack_order);
  for (unsigned int i = 0; i < getWorkspaceCount(); ++i)
    if (i != getCurrentWorkspaceID())
      getWorkspace(i)->appendStackOrder(stack_order);

  if (stack_order.size() > 0) {
    // set the client list atoms
    Window *windows = new Window[stack_order.size()];
    Window *win_it = windows;
    BlackboxWindowList::iterator it = stack_order.begin(),
                                 end = stack_order.end();
    for (; it != end; ++it, ++win_it)
      *win_it = (*it)->getClientWindow();
    xatom->setValue(getRootWindow(), XAtom::net_client_list_stacking,
                    XAtom::window, windows, stack_order.size());
    delete [] windows;
  } else
    xatom->setValue(getRootWindow(), XAtom::net_client_list_stacking,
                    XAtom::window, 0, 0);
}


void BScreen::addSystrayWindow(Window window) {
  systrayWindowList.push_back(window);
  xatom->setValue(getRootWindow(), XAtom::kde_net_system_tray_windows,
                  XAtom::window,
                  &systrayWindowList[0], systrayWindowList.size());
  blackbox->saveSystrayWindowSearch(window, this);
}


void BScreen::removeSystrayWindow(Window window) {
  WindowList::iterator it = systrayWindowList.begin();
  const WindowList::iterator end = systrayWindowList.end();
  for (; it != end; ++it)
    if (*it == window) {
      systrayWindowList.erase(it);
      xatom->setValue(getRootWindow(), XAtom::kde_net_system_tray_windows,
                      XAtom::window,
                      &systrayWindowList[0], systrayWindowList.size());
      blackbox->removeSystrayWindowSearch(window);
      break;
    }
}


void BScreen::manageWindow(Window w) {
  // is the window a KDE systray window?
  Window systray;
  if (xatom->getValue(w, XAtom::kde_net_wm_system_tray_window_for,
                      XAtom::window, systray) && systray) {
    addSystrayWindow(w);
    return;
  }

  // is the window a docking app
  XWMHints *wmhint = XGetWMHints(blackbox->getXDisplay(), w);
  if (wmhint && (wmhint->flags & StateHint) &&
      wmhint->initial_state == WithdrawnState) {
    slit->addClient(w);
    return;
  }

  new BlackboxWindow(blackbox, w, this);

  BlackboxWindow *win = blackbox->searchWindow(w);
  if (! win)
    return;


  if (win->isNormal()) {
    // don't list non-normal windows as managed windows
    windowList.push_back(win);
    updateClientList();
  } else if (win->isDesktop()) {
    desktopWindowList.push_back(win->getFrameWindow());
  }

  XMapRequestEvent mre;
  mre.window = w;
  if (blackbox->isStartup() && win->isNormal()) win->restoreAttributes();
  win->mapRequestEvent(&mre);
}


void BScreen::unmanageWindow(BlackboxWindow *w, bool remap) {
  w->restore(remap);

  // Remove the modality so that its parent won't try to re-focus the window
  if (w->isModal()) w->setModal(False);
  
  if (w->getWorkspaceNumber() != BSENTINEL &&
      w->getWindowNumber() != BSENTINEL) {
    getWorkspace(w->getWorkspaceNumber())->removeWindow(w);
    if (w->isStuck()) {
      for (unsigned int i = 0; i < getNumberOfWorkspaces(); ++i)
        if (i != w->getWorkspaceNumber())
          getWorkspace(i)->removeWindow(w, True);
    }
  } else if (w->isIconic())
    removeIcon(w);

  if (w->isNormal()) {
    // we don't list non-normal windows as managed windows
    windowList.remove(w);
    updateClientList();
  } else if (w->isDesktop()) {
    WindowList::iterator it = desktopWindowList.begin();
    const WindowList::iterator end = desktopWindowList.end();
    for (; it != end; ++it)
      if (*it == w->getFrameWindow()) {
        desktopWindowList.erase(it);
        break;
      }
    assert(it != end);  // the window wasnt a desktop window?
  }

  if (blackbox->getFocusedWindow() == w)
    blackbox->setFocusedWindow((BlackboxWindow *) 0);

  removeNetizen(w->getClientWindow());

  /*
    some managed windows can also be window group controllers.  when
    unmanaging such windows, we should also delete the window group.
  */
  BWindowGroup *group = blackbox->searchGroup(w->getClientWindow());
  delete group;

  delete w;
}


void BScreen::addNetizen(Netizen *n) {
  netizenList.push_back(n);

  n->sendWorkspaceCount();
  n->sendCurrentWorkspace();

  WorkspaceList::iterator it = workspacesList.begin();
  const WorkspaceList::iterator end = workspacesList.end();
  for (; it != end; ++it)
    (*it)->sendWindowList(*n);

  Window f = ((blackbox->getFocusedWindow()) ?
              blackbox->getFocusedWindow()->getClientWindow() : None);
  n->sendWindowFocus(f);
}


void BScreen::removeNetizen(Window w) {
  NetizenList::iterator it = netizenList.begin();
  for (; it != netizenList.end(); ++it) {
    if ((*it)->getWindowID() == w) {
      delete *it;
      netizenList.erase(it);
      break;
    }
  }
}


void BScreen::updateWorkArea(void) {
  if (workspacesList.size() > 0) {
    unsigned long *dims = new unsigned long[4 * workspacesList.size()];
    for (unsigned int i = 0, m = workspacesList.size(); i < m; ++i) {
      // XXX: this could be different for each workspace
      const Rect &area = availableArea();
      dims[(i * 4) + 0] = area.x();
      dims[(i * 4) + 1] = area.y();
      dims[(i * 4) + 2] = area.width();
      dims[(i * 4) + 3] = area.height();
    }
    xatom->setValue(getRootWindow(), XAtom::net_workarea, XAtom::cardinal,
                    dims, 4 * workspacesList.size());
    delete [] dims;
  } else
    xatom->setValue(getRootWindow(), XAtom::net_workarea, XAtom::cardinal,
                    0, 0);
}


void BScreen::updateNetizenCurrentWorkspace(void) {
  std::for_each(netizenList.begin(), netizenList.end(),
                std::mem_fun(&Netizen::sendCurrentWorkspace));
}


void BScreen::updateNetizenWorkspaceCount(void) {
  xatom->setValue(getRootWindow(), XAtom::net_number_of_desktops,
                  XAtom::cardinal, workspacesList.size());

  updateWorkArea();
  
  std::for_each(netizenList.begin(), netizenList.end(),
                std::mem_fun(&Netizen::sendWorkspaceCount));
}


void BScreen::updateNetizenWindowFocus(void) {
  Window f = ((blackbox->getFocusedWindow()) ?
              blackbox->getFocusedWindow()->getClientWindow() : None);

  xatom->setValue(getRootWindow(), XAtom::net_active_window,
                  XAtom::window, f);

  NetizenList::iterator it = netizenList.begin();
  for (; it != netizenList.end(); ++it)
    (*it)->sendWindowFocus(f);
}


void BScreen::updateNetizenWindowAdd(Window w, unsigned long p) {
  NetizenList::iterator it = netizenList.begin();
  for (; it != netizenList.end(); ++it) {
    (*it)->sendWindowAdd(w, p);
  }
}


void BScreen::updateNetizenWindowDel(Window w) {
  NetizenList::iterator it = netizenList.begin();
  for (; it != netizenList.end(); ++it)
    (*it)->sendWindowDel(w);
}


void BScreen::updateNetizenWindowRaise(Window w) {
  NetizenList::iterator it = netizenList.begin();
  for (; it != netizenList.end(); ++it)
    (*it)->sendWindowRaise(w);
}


void BScreen::updateNetizenWindowLower(Window w) {
  NetizenList::iterator it = netizenList.begin();
  for (; it != netizenList.end(); ++it)
    (*it)->sendWindowLower(w);
}


void BScreen::updateNetizenConfigNotify(XEvent *e) {
  NetizenList::iterator it = netizenList.begin();
  for (; it != netizenList.end(); ++it)
    (*it)->sendConfigNotify(e);
}


void BScreen::raiseWindows(Window *workspace_stack, unsigned int num) {
  // the 13 represents the number of blackbox windows such as menus
  int bbwins = 13;
#ifdef    XINERAMA
  ++bbwins;
#endif // XINERAMA

  Window *session_stack = new
    Window[(num + workspacesList.size() + rootmenuList.size() + bbwins)];
  unsigned int i = 0, k = num;

  XRaiseWindow(blackbox->getXDisplay(), iconmenu->getWindowID());
  *(session_stack + i++) = iconmenu->getWindowID();

  WorkspaceList::iterator wit = workspacesList.begin();
  const WorkspaceList::iterator w_end = workspacesList.end();
  for (; wit != w_end; ++wit)
    *(session_stack + i++) = (*wit)->getMenu()->getWindowID();

  *(session_stack + i++) = workspacemenu->getWindowID();

  *(session_stack + i++) = configmenu->getFocusmenu()->getWindowID();
  *(session_stack + i++) = configmenu->getPlacementmenu()->getWindowID();
#ifdef    XINERAMA
  *(session_stack + i++) = configmenu->getXineramamenu()->getWindowID();
#endif // XINERAMA
  *(session_stack + i++) = configmenu->getWindowID();

  *(session_stack + i++) = slit->getMenu()->getDirectionmenu()->getWindowID();
  *(session_stack + i++) = slit->getMenu()->getPlacementmenu()->getWindowID();
  *(session_stack + i++) = slit->getMenu()->getWindowID();

  *(session_stack + i++) =
    toolbar->getMenu()->getPlacementmenu()->getWindowID();
  *(session_stack + i++) = toolbar->getMenu()->getWindowID();

  RootmenuList::iterator rit = rootmenuList.begin();
  for (; rit != rootmenuList.end(); ++rit)
    *(session_stack + i++) = (*rit)->getWindowID();
  *(session_stack + i++) = rootmenu->getWindowID();

  if (toolbar->isOnTop())
    *(session_stack + i++) = toolbar->getWindowID();

  if (slit->isOnTop())
    *(session_stack + i++) = slit->getWindowID();

  while (k--)
    *(session_stack + i++) = *(workspace_stack + k);

  XRestackWindows(blackbox->getXDisplay(), session_stack, i);

  delete [] session_stack;

  updateStackingList();
}


void BScreen::lowerWindows(Window *workspace_stack, unsigned int num) {
  assert(num > 0);  // this would cause trouble in the XRaiseWindow call

  Window *session_stack = new Window[(num + desktopWindowList.size())];
  unsigned int i = 0, k = num;

  XLowerWindow(blackbox->getXDisplay(), workspace_stack[0]);

  while (k--)
    *(session_stack + i++) = *(workspace_stack + k);

  WindowList::iterator dit = desktopWindowList.begin();
  const WindowList::iterator d_end = desktopWindowList.end();
  for (; dit != d_end; ++dit)
    *(session_stack + i++) = *dit;

  XRestackWindows(blackbox->getXDisplay(), session_stack, i);

  delete [] session_stack;

  updateStackingList();
}


void BScreen::reassociateWindow(BlackboxWindow *w, unsigned int wkspc_id,
                                bool ignore_sticky) {
  if (! w) return;

  if (wkspc_id == BSENTINEL)
    wkspc_id = current_workspace->getID();

  if (w->getWorkspaceNumber() == wkspc_id)
    return;

  if (w->isIconic()) {
    removeIcon(w);
    getWorkspace(wkspc_id)->addWindow(w);
    if (w->isStuck())
      for (unsigned int i = 0; i < getNumberOfWorkspaces(); ++i)
        if (i != w->getWorkspaceNumber())
          getWorkspace(i)->addWindow(w, True);
  } else if (ignore_sticky || ! w->isStuck()) {
    if (w->isStuck())
      w->stick();
    getWorkspace(w->getWorkspaceNumber())->removeWindow(w);
    getWorkspace(wkspc_id)->addWindow(w);
  }
  updateStackingList();
}


void BScreen::propagateWindowName(const BlackboxWindow *bw) {
  if (bw->isIconic()) {
    iconmenu->changeItemLabel(bw->getWindowNumber(), bw->getIconTitle());
    iconmenu->update();
  }
  else {
    Clientmenu *clientmenu = getWorkspace(bw->getWorkspaceNumber())->getMenu();
    clientmenu->changeItemLabel(bw->getWindowNumber(), bw->getTitle());
    clientmenu->update();

    if (blackbox->getFocusedWindow() == bw)
      toolbar->redrawWindowLabel(True);
  }
}


void BScreen::nextFocus(void) {
  BlackboxWindow *focused = blackbox->getFocusedWindow(),
    *next = focused;

  if (focused) {
    // if window is not on this screen, ignore it
    if (focused->getScreen()->getScreenNumber() != getScreenNumber())
      focused = (BlackboxWindow*) 0;
  }

  if (focused && current_workspace->getCount() > 1) {
    // next is the next window to recieve focus, current is a place holder
    BlackboxWindow *current;
    do {
      current = next;
      next = current_workspace->getNextWindowInList(current);
    } while(! next->setInputFocus() && next != focused);

    if (next != focused)
      current_workspace->raiseWindow(next);
  } else if (current_workspace->getCount() >= 1) {
    next = current_workspace->getTopWindowOnStack();

    current_workspace->raiseWindow(next);
    next->setInputFocus();
  }
}


void BScreen::prevFocus(void) {
  BlackboxWindow *focused = blackbox->getFocusedWindow(),
    *next = focused;

  if (focused) {
    // if window is not on this screen, ignore it
    if (focused->getScreen()->getScreenNumber() != getScreenNumber())
      focused = (BlackboxWindow*) 0;
  }

  if (focused && current_workspace->getCount() > 1) {
    // next is the next window to recieve focus, current is a place holder
    BlackboxWindow *current;
    do {
      current = next;
      next = current_workspace->getPrevWindowInList(current);
    } while(! next->setInputFocus() && next != focused);

    if (next != focused)
      current_workspace->raiseWindow(next);
  } else if (current_workspace->getCount() >= 1) {
    next = current_workspace->getTopWindowOnStack();

    current_workspace->raiseWindow(next);
    next->setInputFocus();
  }
}


void BScreen::raiseFocus(void) {
  BlackboxWindow *focused = blackbox->getFocusedWindow();
  if (! focused)
    return;

  // if on this Screen, raise it
  if (focused->getScreen()->getScreenNumber() == getScreenNumber()) {
    Workspace *workspace = getWorkspace(focused->getWorkspaceNumber());
    workspace->raiseWindow(focused);
  }
}


void BScreen::InitMenu(void) {
  if (rootmenu) {
    rootmenuList.clear();

    while (rootmenu->getCount())
      rootmenu->remove(0);
  } else {
    rootmenu = new Rootmenu(this);
  }
  bool defaultMenu = True;

  FILE *menu_file = (FILE *) 0;
  const char *menu_filename = blackbox->getMenuFilename();

  if (menu_filename) 
    if (! (menu_file = fopen(menu_filename, "r")))
      perror(menu_filename);
  if (! menu_file) {     // opening the menu file failed, try the default menu
    menu_filename = DEFAULTMENU;
    if (! (menu_file = fopen(menu_filename, "r")))
      perror(menu_filename);
  } 

  if (menu_file) {
    if (feof(menu_file)) {
      fprintf(stderr, i18n(ScreenSet, ScreenEmptyMenuFile,
                           "%s: Empty menu file"),
              menu_filename);
    } else {
      char line[1024], label[1024];
      memset(line, 0, 1024);
      memset(label, 0, 1024);

      while (fgets(line, 1024, menu_file) && ! feof(menu_file)) {
        if (line[0] == '#')
          continue;

        int i, key = 0, index = -1, len = strlen(line);

        for (i = 0; i < len; i++) {
          if (line[i] == '[') index = 0;
          else if (line[i] == ']') break;
          else if (line[i] != ' ')
            if (index++ >= 0)
              key += tolower(line[i]);
        }

        if (key == 517) { // [begin]
          index = -1;
          for (i = index; i < len; i++) {
            if (line[i] == '(') index = 0;
            else if (line[i] == ')') break;
            else if (index++ >= 0) {
              if (line[i] == '\\' && i < len - 1) i++;
              label[index - 1] = line[i];
            }
          }

          if (index == -1) index = 0;
          label[index] = '\0';

          rootmenu->setLabel(label);
          defaultMenu = parseMenuFile(menu_file, rootmenu);
          if (! defaultMenu)
            blackbox->addMenuTimestamp(menu_filename);
          break;
        }
      }
    }
    fclose(menu_file);
  }

  if (defaultMenu) {
    rootmenu->setInternalMenu();
    rootmenu->insert(i18n(ScreenSet, Screenxterm, "xterm"),
                     BScreen::Execute,
                     i18n(ScreenSet, Screenxterm, "xterm"));
    rootmenu->insert(i18n(ScreenSet, ScreenRestart, "Restart"),
                     BScreen::Restart);
    rootmenu->insert(i18n(ScreenSet, ScreenExit, "Exit"),
                     BScreen::Exit);
    rootmenu->setLabel(i18n(BasemenuSet, BasemenuBlackboxMenu,
                            "Openbox Menu"));
  }
}


static
void string_within(char begin, char end, const char *input, size_t length,
                   char *output) {
  bool parse = False;
  size_t index = 0;

  for (size_t i = 0; i < length; ++i) {
    if (input[i] == begin) {
      parse = True;
    } else if (input[i] == end) {
      break;
    } else if (parse) {
      if (input[i] == '\\' && i < length - 1) i++;
      output[index++] = input[i];
    } 
  }

  if (parse)
    output[index] = '\0';
  else
    output[0] = '\0';
}


bool BScreen::parseMenuFile(FILE *file, Rootmenu *menu) {
  char line[1024], keyword[1024], label[1024], command[1024];
  bool done = False;

  while (! (done || feof(file))) {
    memset(line, 0, 1024);
    memset(label, 0, 1024);
    memset(command, 0, 1024);

    if (! fgets(line, 1024, file))
      continue;

    if (line[0] == '#') // comment, skip it
      continue;

    size_t line_length = strlen(line);
    unsigned int key = 0;

    // get the keyword enclosed in []'s
    string_within('[', ']', line, line_length, keyword);

    if (keyword[0] == '\0') {  // no keyword, no menu entry
      continue;
    } else {
      size_t len = strlen(keyword);
      for (size_t i = 0; i < len; ++i) {
        if (keyword[i] != ' ')
          key += tolower(keyword[i]);
      }
    }

    // get the label enclosed in ()'s
    string_within('(', ')', line, line_length, label);

    // get the command enclosed in {}'s
    string_within('{', '}', line, line_length, command);

    switch (key) {
    case 311: // end
      done = True;

      break;

    case 333: // nop
      if (! *label)
        label[0] = '\0';
      menu->insert(label);

      break;

    case 421: // exec
      if (! (*label && *command)) {
        fprintf(stderr, i18n(ScreenSet, ScreenEXECError,
                             "BScreen::parseMenuFile: [exec] error, "
                             "no menu label and/or command defined\n"));
        continue;
      }

      menu->insert(label, BScreen::Execute, command);

      break;

    case 442: // exit
      if (! *label) {
        fprintf(stderr, i18n(ScreenSet, ScreenEXITError,
                             "BScreen::parseMenuFile: [exit] error, "
                             "no menu label defined\n"));
        continue;
      }

      menu->insert(label, BScreen::Exit);

      break;

    case 561: { // style
      if (! (*label && *command)) {
        fprintf(stderr,
                i18n(ScreenSet, ScreenSTYLEError,
                     "BScreen::parseMenuFile: [style] error, "
                     "no menu label and/or filename defined\n"));
        continue;
      }

      string style = expandTilde(command);

      menu->insert(label, BScreen::SetStyle, style.c_str());
    }
      break;

    case 630: // config
      if (! *label) {
        fprintf(stderr, i18n(ScreenSet, ScreenCONFIGError,
                             "BScreen::parseMenufile: [config] error, "
                             "no label defined"));
        continue;
      }

      menu->insert(label, configmenu);

      break;

    case 740: { // include
      if (! *label) {
        fprintf(stderr, i18n(ScreenSet, ScreenINCLUDEError,
                             "BScreen::parseMenuFile: [include] error, "
                             "no filename defined\n"));
        continue;
      }

      string newfile = expandTilde(label);
      FILE *submenufile = fopen(newfile.c_str(), "r");

      if (! submenufile) {
        perror(newfile.c_str());
        continue;
      }

      struct stat buf;
      if (fstat(fileno(submenufile), &buf) ||
          ! S_ISREG(buf.st_mode)) {
        fprintf(stderr,
                i18n(ScreenSet, ScreenINCLUDEErrorReg,
                     "BScreen::parseMenuFile: [include] error: "
                     "'%s' is not a regular file\n"), newfile.c_str());
        break;
      }

      if (! feof(submenufile)) {
        if (! parseMenuFile(submenufile, menu))
          blackbox->addMenuTimestamp(newfile);

        fclose(submenufile);
      }
    }

      break;

    case 767: { // submenu
      if (! *label) {
        fprintf(stderr, i18n(ScreenSet, ScreenSUBMENUError,
                             "BScreen::parseMenuFile: [submenu] error, "
                             "no menu label defined\n"));
        continue;
      }

      Rootmenu *submenu = new Rootmenu(this);

      if (*command)
        submenu->setLabel(command);
      else
        submenu->setLabel(label);

      parseMenuFile(file, submenu);
      submenu->update();
      menu->insert(label, submenu);
      rootmenuList.push_back(submenu);
    }

      break;

    case 773: { // restart
      if (! *label) {
        fprintf(stderr, i18n(ScreenSet, ScreenRESTARTError,
                             "BScreen::parseMenuFile: [restart] error, "
                             "no menu label defined\n"));
        continue;
      }

      if (*command)
        menu->insert(label, BScreen::RestartOther, command);
      else
        menu->insert(label, BScreen::Restart);
    }

      break;

    case 845: { // reconfig
      if (! *label) {
        fprintf(stderr,
                i18n(ScreenSet, ScreenRECONFIGError,
                     "BScreen::parseMenuFile: [reconfig] error, "
                     "no menu label defined\n"));
        continue;
      }

      menu->insert(label, BScreen::Reconfigure);
    }

      break;

    case 995:    // stylesdir
    case 1113: { // stylesmenu
      bool newmenu = ((key == 1113) ? True : False);

      if (! *label || (! *command && newmenu)) {
        fprintf(stderr,
                i18n(ScreenSet, ScreenSTYLESDIRError,
                     "BScreen::parseMenuFile: [stylesdir/stylesmenu]"
                     " error, no directory defined\n"));
        continue;
      }

      char *directory = ((newmenu) ? command : label);

      string stylesdir = expandTilde(directory);

      struct stat statbuf;

      if (stat(stylesdir.c_str(), &statbuf) == -1) {
        fprintf(stderr,
                i18n(ScreenSet, ScreenSTYLESDIRErrorNoExist,
                     "BScreen::parseMenuFile: [stylesdir/stylesmenu]"
                     " error, %s does not exist\n"), stylesdir.c_str());
        continue;
      }
      if (! S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr,
                i18n(ScreenSet, ScreenSTYLESDIRErrorNotDir,
                     "BScreen::parseMenuFile:"
                     " [stylesdir/stylesmenu] error, %s is not a"
                     " directory\n"), stylesdir.c_str());
        continue;
      }

      Rootmenu *stylesmenu;

      if (newmenu)
        stylesmenu = new Rootmenu(this);
      else
        stylesmenu = menu;

      DIR *d = opendir(stylesdir.c_str());
      struct dirent *p;
      std::vector<string> ls;

      while((p = readdir(d)))
        ls.push_back(p->d_name);

      closedir(d);

      std::sort(ls.begin(), ls.end());

      std::vector<string>::iterator it = ls.begin(),
        end = ls.end();
      for (; it != end; ++it) {
        const string& fname = *it;

        if (fname[fname.size()-1] == '~')
          continue;

        string style = stylesdir;
        style += '/';
        style += fname;

        if (! stat(style.c_str(), &statbuf) && S_ISREG(statbuf.st_mode))
          stylesmenu->insert(fname, BScreen::SetStyle, style);
      }

      stylesmenu->update();

      if (newmenu) {
        stylesmenu->setLabel(label);
        menu->insert(label, stylesmenu);
        rootmenuList.push_back(stylesmenu);
      }

      blackbox->addMenuTimestamp(stylesdir);
    }
      break;

    case 1090: { // workspaces
      if (! *label) {
        fprintf(stderr,
                i18n(ScreenSet, ScreenWORKSPACESError,
                     "BScreen:parseMenuFile: [workspaces] error, "
                     "no menu label defined\n"));
        continue;
      }

      menu->insert(label, workspacemenu);
    }
      break;
    }
  }

  return ((menu->getCount() == 0) ? True : False);
}


void BScreen::shutdown(void) {
  XSelectInput(blackbox->getXDisplay(), getRootWindow(), NoEventMask);
  XSync(blackbox->getXDisplay(), False);

  while(! windowList.empty())
    unmanageWindow(windowList.front(), True);

  slit->shutdown();
}


void BScreen::showPosition(int x, int y) {
  if (! geom_visible) {
    XMoveResizeWindow(blackbox->getXDisplay(), geom_window,
                      (getWidth() - geom_w) / 2,
                      (getHeight() - geom_h) / 2, geom_w, geom_h);
    XMapWindow(blackbox->getXDisplay(), geom_window);
    XRaiseWindow(blackbox->getXDisplay(), geom_window);

    geom_visible = True;
  }

  char label[1024];

  sprintf(label, i18n(ScreenSet, ScreenPositionFormat,
                      "X: %4d x Y: %4d"), x, y);

  XClearWindow(blackbox->getXDisplay(), geom_window);

  resource.wstyle.font->drawString(geom_window,
                                   resource.bevel_width, resource.bevel_width,
                                   resource.wstyle.l_text_focus,
                                   label);
}


void BScreen::showGeometry(unsigned int gx, unsigned int gy) {
  if (! geom_visible) {
    XMoveResizeWindow(blackbox->getXDisplay(), geom_window,
                      (getWidth() - geom_w) / 2,
                      (getHeight() - geom_h) / 2, geom_w, geom_h);
    XMapWindow(blackbox->getXDisplay(), geom_window);
    XRaiseWindow(blackbox->getXDisplay(), geom_window);

    geom_visible = True;
  }

  char label[1024];

  sprintf(label, i18n(ScreenSet, ScreenGeometryFormat,
                      "W: %4d x H: %4d"), gx, gy);

  XClearWindow(blackbox->getXDisplay(), geom_window);

  resource.wstyle.font->drawString(geom_window,
                                   resource.bevel_width, resource.bevel_width,
                                   resource.wstyle.l_text_focus,
                                   label);
}


void BScreen::hideGeometry(void) {
  if (geom_visible) {
    XUnmapWindow(blackbox->getXDisplay(), geom_window);
    geom_visible = False;
  }
}


void BScreen::addStrut(Strut *strut) {
  strutList.push_back(strut);
}


void BScreen::removeStrut(Strut *strut) {
  strutList.remove(strut);
}


const Rect& BScreen::availableArea(void) const {
  if (doFullMax())
    return getRect(); // return the full screen
  return usableArea;
}


#ifdef    XINERAMA
const RectList& BScreen::allAvailableAreas(void) const {
  assert(isXineramaActive());
  assert(xineramaUsableArea.size() > 0);
  fprintf(stderr, "1found x %d y %d w %d h %d\n",
          xineramaUsableArea[0].x(), xineramaUsableArea[0].y(),
          xineramaUsableArea[0].width(), xineramaUsableArea[0].height());
  return xineramaUsableArea;
}
#endif // XINERAMA


void BScreen::updateAvailableArea(void) {
  Rect old_area = usableArea;
  usableArea = getRect(); // reset to full screen

#ifdef    XINERAMA
  // reset to the full areas
  if (isXineramaActive())
    xineramaUsableArea = getXineramaAreas();
#endif // XINERAMA

  /* these values represent offsets from the screen edge
   * we look for the biggest offset on each edge and then apply them
   * all at once
   * do not be confused by the similarity to the names of Rect's members
   */
  unsigned int current_left = 0, current_right = 0, current_top = 0,
    current_bottom = 0;

  StrutList::const_iterator it = strutList.begin(), end = strutList.end();

  for(; it != end; ++it) {
    Strut *strut = *it;
    if (strut->left > current_left)
      current_left = strut->left;
    if (strut->top > current_top)
      current_top = strut->top;
    if (strut->right > current_right)
      current_right = strut->right;
    if (strut->bottom > current_bottom)
      current_bottom = strut->bottom;
  }

  usableArea.setPos(current_left, current_top);
  usableArea.setSize(usableArea.width() - (current_left + current_right),
                     usableArea.height() - (current_top + current_bottom));

#ifdef    XINERAMA
  if (isXineramaActive()) {
    // keep each of the ximerama-defined areas inside the strut
    RectList::iterator xit, xend = xineramaUsableArea.end();
    for (xit = xineramaUsableArea.begin(); xit != xend; ++xit) {
      if (xit->x() < usableArea.x()) {
        xit->setX(usableArea.x());
        xit->setWidth(xit->width() - usableArea.x());
      }
      if (xit->y() < usableArea.y()) {
        xit->setY(usableArea.y());
        xit->setHeight(xit->height() - usableArea.y());
      }
      if (xit->x() + xit->width() > usableArea.width())
        xit->setWidth(usableArea.width() - xit->x());
      if (xit->y() + xit->height() > usableArea.height())
        xit->setHeight(usableArea.height() - xit->y());
    }
  }
#endif // XINERAMA

  if (old_area != usableArea) {
    BlackboxWindowList::iterator it = windowList.begin(),
      end = windowList.end();
    for (; it != end; ++it)
      if ((*it)->isMaximized()) (*it)->remaximize();
  }

  updateWorkArea();  
}


Workspace* BScreen::getWorkspace(unsigned int index) {
  assert(index < workspacesList.size());
  return workspacesList[index];
}


void BScreen::buttonPressEvent(const XButtonEvent *xbutton) {
  if (xbutton->button == 1) {
    if (! isRootColormapInstalled())
      image_control->installRootColormap();

    if (workspacemenu->isVisible())
      workspacemenu->hide();

    if (rootmenu->isVisible())
      rootmenu->hide();
  } else if (xbutton->button == 2) {
    int mx = xbutton->x_root - (workspacemenu->getWidth() / 2);
    int my = xbutton->y_root - (workspacemenu->getTitleHeight() / 2);

    if (mx < 0) mx = 0;
    if (my < 0) my = 0;

    if (mx + workspacemenu->getWidth() > getWidth())
      mx = getWidth() - workspacemenu->getWidth() - getBorderWidth();

    if (my + workspacemenu->getHeight() > getHeight())
      my = getHeight() - workspacemenu->getHeight() - getBorderWidth();

    workspacemenu->move(mx, my);

    if (! workspacemenu->isVisible()) {
      workspacemenu->removeParent();
      workspacemenu->show();
    }
  } else if (xbutton->button == 3) {
    int mx = xbutton->x_root - (rootmenu->getWidth() / 2);
    int my = xbutton->y_root - (rootmenu->getTitleHeight() / 2);

    if (mx < 0) mx = 0;
    if (my < 0) my = 0;

    if (mx + rootmenu->getWidth() > getWidth())
      mx = getWidth() - rootmenu->getWidth() - getBorderWidth();

    if (my + rootmenu->getHeight() > getHeight())
      my = getHeight() - rootmenu->getHeight() - getBorderWidth();

    rootmenu->move(mx, my);

    if (! rootmenu->isVisible()) {
      blackbox->checkMenu();
      rootmenu->show();
    }
  // mouse wheel up
  } else if ((xbutton->button == 4 && resource.root_scroll == NormalScroll) ||
             (xbutton->button == 5 && resource.root_scroll == ReverseScroll)) {
    if (getCurrentWorkspaceID() >= getWorkspaceCount() - 1)
      changeWorkspaceID(0);
    else
      changeWorkspaceID(getCurrentWorkspaceID() + 1);
  // mouse wheel down
  } else if ((xbutton->button == 5 && resource.root_scroll == NormalScroll) ||
             (xbutton->button == 4 && resource.root_scroll == ReverseScroll)) {
    if (getCurrentWorkspaceID() == 0)
      changeWorkspaceID(getWorkspaceCount() - 1);
    else
      changeWorkspaceID(getCurrentWorkspaceID() - 1);
  }
}


void BScreen::propertyNotifyEvent(const XPropertyEvent *pe) {
  if (pe->atom == xatom->getAtom(XAtom::net_desktop_names)) {
    // _NET_WM_DESKTOP_NAMES
    WorkspaceList::iterator it = workspacesList.begin();
    const WorkspaceList::iterator end = workspacesList.end();
    for (; it != end; ++it) {
      (*it)->readName(); // re-read its name from the window property
      workspacemenu->changeWorkspaceLabel((*it)->getID(), (*it)->getName());
    }
    workspacemenu->update();
    toolbar->reconfigure();
    saveWorkspaceNames();
  }
}


void BScreen::toggleFocusModel(FocusModel model) {
  std::for_each(windowList.begin(), windowList.end(),
                std::mem_fun(&BlackboxWindow::ungrabButtons));

  if (model == SloppyFocus) {
    saveSloppyFocus(True);
  } else {
    // we're cheating here to save writing the config file 3 times
    resource.auto_raise = False;
    resource.click_raise = False;
    saveSloppyFocus(False);
  }

  std::for_each(windowList.begin(), windowList.end(),
                std::mem_fun(&BlackboxWindow::grabButtons));
}


BTexture BScreen::readDatabaseTexture(const string &rname,
                                      const string &default_color,
                                      const Configuration &style) {
  BTexture texture;
  string s;

  if (style.getValue(rname, s))
    texture = BTexture(s);
  else
    texture.setTexture(BTexture::Solid | BTexture::Flat);

  // associate this texture with this screen
  texture.setDisplay(getBaseDisplay(), getScreenNumber());
  texture.setImageControl(image_control);

  if (texture.texture() & BTexture::Solid) {
    texture.setColor(readDatabaseColor(rname + ".color",
                                       default_color, style));
    texture.setColorTo(readDatabaseColor(rname + ".colorTo",
                                         default_color, style));
  } else if (texture.texture() & BTexture::Gradient) {
    texture.setColor(readDatabaseColor(rname + ".color",
                                       default_color, style));
    texture.setColorTo(readDatabaseColor(rname + ".colorTo",
                                         default_color, style));
  }

  return texture;
}


BColor BScreen::readDatabaseColor(const string &rname,
                                  const string &default_color,
                                  const Configuration &style) {
  BColor color;
  string s;
  if (style.getValue(rname, s))
    color = BColor(s, getBaseDisplay(), getScreenNumber());
  else
    color = BColor(default_color, getBaseDisplay(), getScreenNumber());
  return color;
}


BFont *BScreen::readDatabaseFont(const string &rbasename,
                                 const Configuration &style) {
  string fontname;

  string s;

#ifdef    XFT
  int i;
  if (style.getValue(rbasename + "xft.font", s) &&
      style.getValue(rbasename + "xft.size", i)) {
    string family = s;
    bool bold = False;
    bool italic = False;
    if (style.getValue(rbasename + "xft.flags", s)) {
      if (s.find("bold") != string::npos)
        bold = True;
      if (s.find("italic") != string::npos)
        italic = True;
    }
    
    BFont *b = new BFont(blackbox->getXDisplay(), this, family, i, bold,
                         italic, resource.aa_fonts);
    if (b->valid())
      return b;
    else
      delete b; // fall back to the normal X font stuff
  }
#endif // XFT

  style.getValue(rbasename + "font", s);
  // if this fails, a blank string will be used, which will cause the fallback
  // font to load.

  BFont *b = new BFont(blackbox->getXDisplay(), this, s);
  if (! b->valid())
    exit(2);  // can't continue without a font
  return b;
}
