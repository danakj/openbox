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

#include "blackbox.hh"
#include "font.hh"
#include "gccache.hh"
#include "image.hh"
#include "screen.hh"
#include "util.hh"
#include "window.hh"
#include "workspace.hh"
#include "util.hh"
#include "xatom.hh"

#ifndef   FONT_ELEMENT_SIZE
#define   FONT_ELEMENT_SIZE 50
#endif // FONT_ELEMENT_SIZE


static bool running = True;

static int anotherWMRunning(Display *display, XErrorEvent *) {
  fprintf(stderr,
          "BScreen::BScreen: an error occured while querying the X server.\n"
          "  another window manager already running on display %s.\n",
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

  fprintf(stderr, "BScreen::BScreen: managing screen %d "
          "using visual 0x%lx, depth %d\n",
          getScreenNumber(), XVisualIDFromVisual(getVisual()),
          getDepth());

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

  const char *s = "0: 0000 x 0: 0000";
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

  if (resource.workspaces > 0) {
    for (unsigned int i = 0; i < resource.workspaces; ++i) {
      Workspace *wkspc = new Workspace(this, workspacesList.size());
      workspacesList.push_back(wkspc);

    }
  } else {
    Workspace *wkspc = new Workspace(this, workspacesList.size());
    workspacesList.push_back(wkspc);
  }
  saveWorkspaceNames();

  updateNetizenWorkspaceCount();

  current_workspace = workspacesList.front();
  
  xatom->setValue(getRootWindow(), XAtom::net_current_desktop,
                  XAtom::cardinal, 0); //first workspace

  raiseWindows(0, 0);     // this also initializes the empty stacking list

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

  while (! systrayWindowList.empty())
    removeSystrayWindow(systrayWindowList[0]);

  delete image_control;

  if (resource.wstyle.font)
    delete resource.wstyle.font;

#ifdef    BITMAPBUTTONS
  if (resource.wstyle.close_button.mask != None)
    XFreePixmap(blackbox->getXDisplay(), resource.wstyle.close_button.mask);
  if (resource.wstyle.max_button.mask != None)
    XFreePixmap(blackbox->getXDisplay(), resource.wstyle.max_button.mask);
  if (resource.wstyle.icon_button.mask != None)
    XFreePixmap(blackbox->getXDisplay(), resource.wstyle.icon_button.mask);
  if (resource.wstyle.stick_button.mask != None)
    XFreePixmap(blackbox->getXDisplay(), resource.wstyle.stick_button.mask);

  resource.wstyle.max_button.mask = resource.wstyle.close_button.mask =
    resource.wstyle.icon_button.mask =
    resource.wstyle.stick_button.mask = None;
#endif // BITMAPBUTTONS
  
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
  config->setValue(screenstr + "antialiasFonts", resource.aa_fonts);
  reconfigure();
}


void BScreen::saveShadowFonts(bool f) {
  resource.shadow_fonts = f;
  config->setValue(screenstr + "dropShadowFonts", resource.shadow_fonts);
  reconfigure();
}


void BScreen::saveWindowToEdgeSnap(int s) {
  resource.snap_to_edges = s;

  const char *snap;
  switch (resource.snap_to_edges) {
  case WindowNoSnap: snap = "NoSnap"; break;
  case WindowResistance: snap = "Resistance"; break;
  case WindowSnap: default: snap = "Snap"; break;
  }
  config->setValue(screenstr + "windowToEdgeSnap", snap);
}


void BScreen::saveWindowToWindowSnap(int s) {
  resource.snap_to_windows = s;
  
  const char *snap;
  switch (resource.snap_to_windows) {
  case WindowNoSnap: snap = "NoSnap"; break;
  case WindowResistance: snap = "Resistance"; break;
  case WindowSnap: default: snap = "Snap"; break;
  }
  config->setValue(screenstr + "windowToWindowSnap", snap);
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


void BScreen::saveResistanceSize(int s) {
  resource.resistance_size = s;
  config->setValue(screenstr + "resistanceSize",
                   resource.resistance_size);
}


void BScreen::saveSnapThreshold(int t) {
  resource.snap_threshold = t;
  config->setValue(screenstr + "edgeSnapThreshold",
                   resource.snap_threshold);
}


void BScreen::saveSnapOffset(int t) {
  resource.snap_offset = t;
  config->setValue(screenstr + "edgeSnapOffset",
                   resource.snap_offset);
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


void BScreen::saveStrftimeFormat(const std::string& format) {
  resource.strftime_format = format;
  config->setValue(screenstr + "strftimeFormat", resource.strftime_format);
}


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
  saveShadowFonts(resource.shadow_fonts);
  saveAAFonts(resource.aa_fonts);
  saveResizeZones(resource.resize_zones);
  saveOpaqueMove(resource.opaque_move);
  saveFullMax(resource.full_max);
  saveFocusNew(resource.focus_new);
  saveFocusLast(resource.focus_last);
  saveWindowToWindowSnap(resource.snap_to_windows);
  saveWindowToEdgeSnap(resource.snap_to_edges);
  saveWindowCornerSnap(resource.window_corner_snap);
  saveWorkspaces(resource.workspaces);
  savePlacementPolicy(resource.placement_policy);
  saveSnapThreshold(resource.snap_threshold);
  saveSnapOffset(resource.snap_offset);
  saveResistanceSize(resource.resistance_size);
  saveRowPlacementDirection(resource.row_direction);
  saveColPlacementDirection(resource.col_direction);
  saveStrftimeFormat(resource.strftime_format); 
  savePlaceIgnoreShaded(resource.ignore_shaded);
  savePlaceIgnoreMaximized(resource.ignore_maximized);
  saveAllowScrollLock(resource.allow_scroll_lock);
  saveWorkspaceWarping(resource.workspace_warping);
  saveRootScrollDirection(resource.root_scroll);
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

  if (! resource.aa_fonts ||
      ! config->getValue(screenstr + "dropShadowFonts", resource.shadow_fonts))
    resource.shadow_fonts = false;

  if (! config->getValue(screenstr + "resizeZones", resource.resize_zones) ||
      (resource.resize_zones != 1 && resource.resize_zones != 2 &&
       resource.resize_zones != 4))
      resource.resize_zones = 4;

  resource.snap_to_windows = WindowResistance;
  if (config->getValue(screenstr + "windowToWindowSnap", s)) {
    if (s == "NoSnap")
      resource.snap_to_windows = WindowNoSnap;
    else if (s == "Snap")
      resource.snap_to_windows = WindowSnap;
  }

  resource.snap_to_edges = WindowResistance;
  if (config->getValue(screenstr + "windowToEdgeSnap", s)) {
    if (s == "NoSnap")
      resource.snap_to_edges = WindowNoSnap;
    else if (s == "Snap")
      resource.snap_to_edges = WindowSnap;
  }

  if (! config->getValue(screenstr + "windowCornerSnap",
                         resource.window_corner_snap))
    resource.window_corner_snap = true;

  if (! config->getValue(screenstr + "imageDither", b))
    b = true;
  image_control->setDither(b);

  if (! config->getValue(screenstr + "edgeSnapOffset",
                        resource.snap_offset))
    resource.snap_offset = 0;
  if (resource.snap_offset > 50)  // sanity check, setting this huge would
    resource.snap_offset = 50;    // seriously suck.
  
  if (! config->getValue(screenstr + "edgeSnapThreshold",
                        resource.snap_threshold))
    resource.snap_threshold = 4;
  
  if (! config->getValue(screenstr + "resistanceSize",
                        resource.resistance_size))
    resource.resistance_size = 18;
  
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

  if (! config->getValue(screenstr + "strftimeFormat",
                         resource.strftime_format))
    resource.strftime_format = "%I:%M %p";
  
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

  const char *s = "0: 0000 x 0: 0000";

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

  typedef std::vector<int> SubList;
  SubList remember_subs;

  raiseWindows(0, 0);

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

  resource.wstyle.font = readDatabaseFont("window.", style);

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

  //if neither of these can be found, we will use the previous resource
  resource.wstyle.b_pressed_focus =
    readDatabaseTexture("window.button.pressed.focus", "black", style, true);
  resource.wstyle.b_pressed_unfocus =
    readDatabaseTexture("window.button.pressed.unfocus", "black", style, true);

#ifdef    BITMAPBUTTONS
  if (resource.wstyle.close_button.mask != None)
    XFreePixmap(blackbox->getXDisplay(), resource.wstyle.close_button.mask);
  if (resource.wstyle.max_button.mask != None)
    XFreePixmap(blackbox->getXDisplay(), resource.wstyle.max_button.mask);
  if (resource.wstyle.icon_button.mask != None)
    XFreePixmap(blackbox->getXDisplay(), resource.wstyle.icon_button.mask);
  if (resource.wstyle.stick_button.mask != None)
    XFreePixmap(blackbox->getXDisplay(), resource.wstyle.stick_button.mask);

  resource.wstyle.close_button.mask = resource.wstyle.max_button.mask =
    resource.wstyle.icon_button.mask =
    resource.wstyle.icon_button.mask = None;
  
  readDatabaseMask("window.button.close.mask", resource.wstyle.close_button,
                   style);
  readDatabaseMask("window.button.max.mask", resource.wstyle.max_button,
                   style);
  readDatabaseMask("window.button.icon.mask", resource.wstyle.icon_button,
                   style);
  readDatabaseMask("window.button.stick.mask", resource.wstyle.stick_button,
                   style);
#endif // BITMAPBUTTONS

  // we create the window.frame texture by hand because it exists only to
  // make the code cleaner and is not actually used for display
  BColor color = readDatabaseColor("window.frame.focusColor", "white", style);
  resource.wstyle.f_focus = BTexture("solid flat", getBaseDisplay(),
                                     getScreenNumber(), image_control);
  resource.wstyle.f_focus.setColor(color);

  color = readDatabaseColor("window.frame.unfocusColor", "white", style);
  resource.wstyle.f_unfocus = BTexture("solid flat", getBaseDisplay(),
                                       getScreenNumber(), image_control);
  resource.wstyle.f_unfocus.setColor(color);

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

  // sanity checks
  if (resource.wstyle.t_focus.texture() == BTexture::Parent_Relative)
    resource.wstyle.t_focus = resource.wstyle.f_focus;
  if (resource.wstyle.t_unfocus.texture() == BTexture::Parent_Relative)
    resource.wstyle.t_unfocus = resource.wstyle.f_unfocus;
  if (resource.wstyle.h_focus.texture() == BTexture::Parent_Relative)
    resource.wstyle.h_focus = resource.wstyle.f_focus;
  if (resource.wstyle.h_unfocus.texture() == BTexture::Parent_Relative)
    resource.wstyle.h_unfocus = resource.wstyle.f_unfocus;

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
}


void BScreen::removeIcon(BlackboxWindow *w) {
  if (! w) return;

  iconList.remove(w);

  BlackboxWindowList::iterator it = iconList.begin(),
    end = iconList.end();
  for (int i = 0; it != end; ++it)
    (*it)->setWindowNumber(i++);
}


BlackboxWindow *BScreen::getIcon(unsigned int index) {
  if (index < iconList.size()) {
    BlackboxWindowList::iterator it = iconList.begin();
    while (index-- > 0) // increment to index
      ++it;
    return *it;
  }

  return (BlackboxWindow *) 0;
}


unsigned int BScreen::addWorkspace(void) {
  Workspace *wkspc = new Workspace(this, workspacesList.size());
  workspacesList.push_back(wkspc);
  saveWorkspaces(getWorkspaceCount());
  saveWorkspaceNames();

  return workspacesList.size();
}


unsigned int BScreen::removeLastWorkspace(void) {
  if (workspacesList.size() == 1)
    return 1;

  Workspace *wkspc = workspacesList.back();

  if (current_workspace->getID() == wkspc->getID())
    changeWorkspaceID(current_workspace->getID() - 1);

  wkspc->removeAll();

  workspacesList.pop_back();
  delete wkspc;

  saveWorkspaces(getWorkspaceCount());
  saveWorkspaceNames();

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

  // when we switch workspaces, unfocus whatever was focused if it is going
  // to be unmapped
  if (focused && ! focused->isStuck())
    blackbox->setFocusedWindow((BlackboxWindow *) 0);

  current_workspace->hideAll();

  current_workspace = getWorkspace(id);

  xatom->setValue(getRootWindow(), XAtom::net_current_desktop,
                  XAtom::cardinal, id);

  current_workspace->showAll();

  int x, y, rx, ry;
  Window c, r;
  unsigned int m;
  BlackboxWindow *win = (BlackboxWindow *) 0;
  bool f = False;

  XSync(blackbox->getXDisplay(), False);

  // If sloppy focus and we can find the client window under the pointer,
  // try to focus it.  
  if (resource.sloppy_focus &&
      XQueryPointer(blackbox->getXDisplay(), getRootWindow(), &r, &c,
                    &rx, &ry, &x, &y, &m) &&
      c != None) {
    if ( (win = blackbox->searchWindow(c)) )
      f = win->setInputFocus();
  }

  // If that fails, and we're doing focus_last, try to focus the last window.
  if (! f && resource.focus_last &&
      (win = current_workspace->getLastFocusedWindow()))
    f = win->setInputFocus();

  /*
    if we found a focus target, then we set the focused window explicitly
    because it is possible to switch off this workspace before the x server
    generates the FocusIn event for the window. if that happens, openbox would
    lose track of what window was the 'LastFocused' window on the workspace.

    if we did not find a focus target, then set the current focused window to
    nothing.
  */
  if (f)
    blackbox->setFocusedWindow(win);
  else
    blackbox->setFocusedWindow((BlackboxWindow *) 0);
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
  XGrabServer(blackbox->getXDisplay());
  
  XSelectInput(blackbox->getXDisplay(), window, StructureNotifyMask);
  systrayWindowList.push_back(window);
  xatom->setValue(getRootWindow(), XAtom::kde_net_system_tray_windows,
                  XAtom::window,
                  &systrayWindowList[0], systrayWindowList.size());
  blackbox->saveSystrayWindowSearch(window, this);

  XUngrabServer(blackbox->getXDisplay());
}


void BScreen::removeSystrayWindow(Window window) {
  XGrabServer(blackbox->getXDisplay());
  
  WindowList::iterator it = systrayWindowList.begin();
  const WindowList::iterator end = systrayWindowList.end();
  for (; it != end; ++it)
    if (*it == window) {
      systrayWindowList.erase(it);
      xatom->setValue(getRootWindow(), XAtom::kde_net_system_tray_windows,
                      XAtom::window,
                      &systrayWindowList[0], systrayWindowList.size());
      blackbox->removeSystrayWindowSearch(window);
      XSelectInput(blackbox->getXDisplay(), window, NoEventMask);
      break;
    }

  assert(it != end);    // not a systray window

  XUngrabServer(blackbox->getXDisplay());
}


void BScreen::manageWindow(Window w) {
  // is the window a KDE systray window?
  Window systray;
  if (xatom->getValue(w, XAtom::kde_net_wm_system_tray_window_for,
                      XAtom::window, systray) && systray != None) {
    addSystrayWindow(w);
    return;
  }

  // is the window a docking app
  XWMHints *wmhint = XGetWMHints(blackbox->getXDisplay(), w);
  if (wmhint && (wmhint->flags & StateHint) &&
      wmhint->initial_state == WithdrawnState) {
    //slit->addClient(w);
    return;
  }

  new BlackboxWindow(blackbox, w, this);

  BlackboxWindow *win = blackbox->searchWindow(w);
  if (! win)
    return;

  if (win->isDesktop()) {
    desktopWindowList.push_back(win->getFrameWindow());
  } else { // if (win->isNormal()) {
    // don't list desktop windows as managed windows
    windowList.push_back(win);
    updateClientList();
  
    if (win->isTopmost())
      specialWindowList.push_back(win->getFrameWindow());
  }
  
  XMapRequestEvent mre;
  mre.window = w;
  if (blackbox->isStartup() && win->isNormal()) win->restoreAttributes();
  win->mapRequestEvent(&mre);
}


void BScreen::unmanageWindow(BlackboxWindow *w, bool remap) {
  // is the window a KDE systray window?
  Window systray;
  if (xatom->getValue(w->getClientWindow(),
                      XAtom::kde_net_wm_system_tray_window_for,
                      XAtom::window, systray) && systray != None) {
    removeSystrayWindow(w->getClientWindow());
    return;
  }

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

  if (w->isDesktop()) {
    WindowList::iterator it = desktopWindowList.begin();
    const WindowList::iterator end = desktopWindowList.end();
    for (; it != end; ++it)
      if (*it == w->getFrameWindow()) {
        desktopWindowList.erase(it);
        break;
      }
    assert(it != end);  // the window wasnt a desktop window?
  } else { // if (w->isNormal()) {
    // we don't list desktop windows as managed windows
    windowList.remove(w);
    updateClientList();

    if (w->isTopmost()) {
      WindowList::iterator it = specialWindowList.begin();
      const WindowList::iterator end = specialWindowList.end();
      for (; it != end; ++it)
        if (*it == w->getFrameWindow()) {
          specialWindowList.erase(it);
          break;
        }
      assert(it != end);  // the window wasnt a special window?
    }
  }

  if (blackbox->getFocusedWindow() == w)
    blackbox->setFocusedWindow((BlackboxWindow *) 0);

  /*
    some managed windows can also be window group controllers.  when
    unmanaging such windows, we should also delete the window group.
  */
  BWindowGroup *group = blackbox->searchGroup(w->getClientWindow());
  delete group;

  delete w;
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


void BScreen::updateNetizenWorkspaceCount(void) {
  xatom->setValue(getRootWindow(), XAtom::net_number_of_desktops,
                  XAtom::cardinal, workspacesList.size());

  updateWorkArea();
}


void BScreen::updateNetizenWindowFocus(void) {
  Window f = ((blackbox->getFocusedWindow()) ?
              blackbox->getFocusedWindow()->getClientWindow() : None);

  xatom->setValue(getRootWindow(), XAtom::net_active_window,
                  XAtom::window, f);
}


void BScreen::raiseWindows(Window *workspace_stack, unsigned int num) {
  // the 13 represents the number of blackbox windows such as menus
  int bbwins = 15;
#ifdef    XINERAMA
  ++bbwins;
#endif // XINERAMA

  Window *session_stack = new
    Window[(num + specialWindowList.size() + bbwins)];
  unsigned int i = 0, k = num;

  WindowList::iterator sit, send = specialWindowList.end();
  for (sit = specialWindowList.begin(); sit != send; ++sit)
    *(session_stack + i++) = *sit;

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
  } else {
  }
}


void BScreen::nextFocus(void) const {
  BlackboxWindow *focused = blackbox->getFocusedWindow(),
    *next = focused;

  if (focused &&
      focused->getScreen()->getScreenNumber() == getScreenNumber() &&
      current_workspace->getCount() > 1) {
    do {
      next = current_workspace->getNextWindowInList(next);
    } while (next != focused && ! next->setInputFocus());

    if (next != focused)
      current_workspace->raiseWindow(next);
  } else if (current_workspace->getCount() > 0) {
    next = current_workspace->getTopWindowOnStack();
    next->setInputFocus();
    current_workspace->raiseWindow(next);
  }
}


void BScreen::prevFocus(void) const {
  BlackboxWindow *focused = blackbox->getFocusedWindow(),
    *next = focused;

  if (focused) {
    // if window is not on this screen, ignore it
    if (focused->getScreen()->getScreenNumber() != getScreenNumber())
      focused = (BlackboxWindow*) 0;
  }
  
  if (focused &&
      focused->getScreen()->getScreenNumber() == getScreenNumber() &&
      current_workspace->getCount() > 1) {
    // next is the next window to receive focus, current is a place holder
    do {
      next = current_workspace->getPrevWindowInList(next);
    } while (next != focused && ! next->setInputFocus());

    if (next != focused)
      current_workspace->raiseWindow(next);
  } else if (current_workspace->getCount() > 0) {
    next = current_workspace->getTopWindowOnStack();
    next->setInputFocus();
    current_workspace->raiseWindow(next);
  }
}


void BScreen::raiseFocus(void) const {
  BlackboxWindow *focused = blackbox->getFocusedWindow();
  if (! focused)
    return;

  // if on this Screen, raise it
  if (focused->getScreen()->getScreenNumber() == getScreenNumber()) {
    Workspace *workspace = getWorkspace(focused->getWorkspaceNumber());
    workspace->raiseWindow(focused);
  }
}


void BScreen::shutdown(void) {
  XSelectInput(blackbox->getXDisplay(), getRootWindow(), NoEventMask);
  XSync(blackbox->getXDisplay(), False);

  while(! windowList.empty())
    unmanageWindow(windowList.front(), True);

  while(! desktopWindowList.empty()) {
    BlackboxWindow *win = blackbox->searchWindow(desktopWindowList.front());
    assert(win);
    unmanageWindow(win, True);
  }
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

  sprintf(label, "X: %4d x Y: %4d", x, y);

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

  sprintf(label, "W: %4d x H: %4d", gx, gy);

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


Workspace* BScreen::getWorkspace(unsigned int index) const {
  assert(index < workspacesList.size());
  return workspacesList[index];
}


void BScreen::buttonPressEvent(const XButtonEvent *xbutton) {
  if (xbutton->button == 1) {
    if (! isRootColormapInstalled())
      image_control->installRootColormap();

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
      //workspacemenu->changeWorkspaceLabel((*it)->getID(), (*it)->getName());
    }
    //workspacemenu->update();
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

#ifdef    BITMAPBUTTONS
void BScreen::readDatabaseMask(const string &rname, PixmapMask &pixmapMask,
                               const Configuration &style) {
  string s;
  int hx, hy; //ignored
  int ret = BitmapOpenFailed; //default to failure.
  
  if (style.getValue(rname, s))
  {
    if (s[0] != '/' && s[0] != '~')
    {
      std::string xbmFile = std::string("~/.openbox/buttons/") + s;
      ret = XReadBitmapFile(blackbox->getXDisplay(), getRootWindow(),
                            expandTilde(xbmFile).c_str(), &pixmapMask.w,
                            &pixmapMask.h, &pixmapMask.mask, &hx, &hy);
    } else
      ret = XReadBitmapFile(blackbox->getXDisplay(), getRootWindow(),
                            expandTilde(s).c_str(), &pixmapMask.w,
                            &pixmapMask.h, &pixmapMask.mask, &hx, &hy);
    
    if (ret == BitmapSuccess)
      return;
  }

  pixmapMask.mask = None;
  pixmapMask.w = pixmapMask.h = 0;
}
#endif // BITMAPSUCCESS

BTexture BScreen::readDatabaseTexture(const string &rname,
                                      const string &default_color,
                                      const Configuration &style, 
                                      bool allowNoTexture) {
  BTexture texture;
  string s;

  if (style.getValue(rname, s))
    texture = BTexture(s);
  else if (allowNoTexture) //no default
    texture.setTexture(BTexture::NoTexture);
  else
    texture.setTexture(BTexture::Solid | BTexture::Flat);

  // associate this texture with this screen
  texture.setDisplay(getBaseDisplay(), getScreenNumber());
  texture.setImageControl(image_control);

  if (texture.texture() != BTexture::NoTexture) {
    texture.setColor(readDatabaseColor(rname + ".color", default_color,
                                       style));
    texture.setColorTo(readDatabaseColor(rname + ".colorTo", default_color,
                                         style));
    texture.setBorderColor(readDatabaseColor(rname + ".borderColor",
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

  int i;
  if (style.getValue(rbasename + "xft.font", s) &&
      style.getValue(rbasename + "xft.size", i)) {
    string family = s;
    bool bold = False;
    bool italic = False;
    bool dropShadow = False;

    if (style.getValue(rbasename + "xft.flags", s)) {
      if (s.find("bold") != string::npos)
        bold = True;
      if (s.find("italic") != string::npos)
        italic = True;
      if (s.find("shadow") != string::npos)
        dropShadow = True;
    }
    
    unsigned char offset = 1;
    if (style.getValue(rbasename + "xft.shadow.offset", s)) {
      offset = atoi(s.c_str()); //doesn't detect errors
      if (offset > CHAR_MAX)
        offset = 1;
    }

    unsigned char tint = 0x40;
    if (style.getValue(rbasename + "xft.shadow.tint", s)) {
      tint = atoi(s.c_str());
    }

    
    BFont *b = new BFont(blackbox->getXDisplay(), this, family, i, bold,
                         italic, dropShadow && resource.shadow_fonts, offset, 
                         tint, resource.aa_fonts);
    if (b->valid())
      return b;
    delete b;
  }
    
  exit(2);  // can't continue without a font
}
