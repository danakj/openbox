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
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <X11/Xatom.h>
#include <X11/keysym.h>

// for strcasestr()
#ifndef _GNU_SOURCE
#  define   _GNU_SOURCE
#endif // _GNU_SOURCE

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

#include <algorithm>
#include <functional>
using std::string;

#include "i18n.hh"
#include "blackbox.hh"
#include "Clientmenu.hh"
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
  resource.stylerc = 0;

  resource.mstyle.t_fontset = resource.mstyle.f_fontset =
    resource.tstyle.fontset = resource.wstyle.fontset = (XFontSet) 0;
  resource.mstyle.t_font = resource.mstyle.f_font = resource.tstyle.font =
    resource.wstyle.font = (XFontStruct *) 0;

#ifdef    HAVE_GETPID
  pid_t bpid = getpid();

  XChangeProperty(blackbox->getXDisplay(), getRootWindow(),
                  blackbox->getBlackboxPidAtom(), XA_CARDINAL,
                  sizeof(pid_t) * 8, PropModeReplace,
                  (unsigned char *) &bpid, 1);
#endif // HAVE_GETPID

  XDefineCursor(blackbox->getXDisplay(), getRootWindow(),
                blackbox->getSessionCursor());

  // start off full screen, top left.
  usableArea.setSize(getWidth(), getHeight());

  image_control =
    new BImageControl(blackbox, this, True, blackbox->getColorsPerChannel(),
                      blackbox->getCacheLife(), blackbox->getCacheMax());
  image_control->installRootColormap();
  root_colormap_installed = True;

  blackbox->load_rc(this);

  image_control->setDither(resource.image_dither);

  LoadStyle();

  XGCValues gcv;
  unsigned long gc_value_mask = GCForeground;
  if (! i18n.multibyte()) gc_value_mask |= GCFont;

  gcv.foreground = WhitePixel(blackbox->getXDisplay(), getScreenNumber())
    ^ BlackPixel(blackbox->getXDisplay(), getScreenNumber());
  gcv.function = GXxor;
  gcv.subwindow_mode = IncludeInferiors;
  opGC = XCreateGC(blackbox->getXDisplay(), getRootWindow(),
                   GCForeground | GCFunction | GCSubwindowMode, &gcv);

  const char *s =  i18n(ScreenSet, ScreenPositionLength,
                        "0: 0000 x 0: 0000");
  int l = strlen(s);

  if (i18n.multibyte()) {
    XRectangle ink, logical;
    XmbTextExtents(resource.wstyle.fontset, s, l, &ink, &logical);
    geom_w = logical.width;

    geom_h = resource.wstyle.fontset_extents->max_ink_extent.height;
  } else {
    geom_h = resource.wstyle.font->ascent +
      resource.wstyle.font->descent;

    geom_w = XTextWidth(resource.wstyle.font, s, l);
  }

  geom_w += (resource.bevel_width * 2);
  geom_h += (resource.bevel_width * 2);

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
  if (! geom_pixmap)
    XSetWindowBackground(blackbox->getXDisplay(), geom_window,
                         texture->color().pixel());
  else
    XSetWindowBackgroundPixmap(blackbox->getXDisplay(),
                               geom_window, geom_pixmap);

  workspacemenu = new Workspacemenu(this);
  iconmenu = new Iconmenu(this);
  configmenu = new Configmenu(this);

  Workspace *wkspc = (Workspace *) 0;
  if (resource.workspaces != 0) {
    for (unsigned int i = 0; i < resource.workspaces; ++i) {
      wkspc = new Workspace(this, workspacesList.size());
      workspacesList.push_back(wkspc);
      workspacemenu->insert(wkspc->getName(), wkspc->getMenu());
    }
  } else {
    wkspc = new Workspace(this, workspacesList.size());
    workspacesList.push_back(wkspc);
    workspacemenu->insert(wkspc->getName(), wkspc->getMenu());
  }

  workspacemenu->insert(i18n(IconSet, IconIcons, "Icons"), iconmenu);
  workspacemenu->update();

  current_workspace = workspacesList.front();
  workspacemenu->setItemSelected(2, True);

  toolbar = new Toolbar(this);

  slit = new Slit(this);

  InitMenu();

  raiseWindows(0, 0);
  rootmenu->update();

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
    if (children[i] == None || (! blackbox->validateWindow(children[i])))
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

  delete rootmenu;
  delete workspacemenu;
  delete iconmenu;
  delete configmenu;
  delete slit;
  delete toolbar;
  delete image_control;

  if (resource.wstyle.fontset)
    XFreeFontSet(blackbox->getXDisplay(), resource.wstyle.fontset);
  if (resource.mstyle.t_fontset)
    XFreeFontSet(blackbox->getXDisplay(), resource.mstyle.t_fontset);
  if (resource.mstyle.f_fontset)
    XFreeFontSet(blackbox->getXDisplay(), resource.mstyle.f_fontset);
  if (resource.tstyle.fontset)
    XFreeFontSet(blackbox->getXDisplay(), resource.tstyle.fontset);

  if (resource.wstyle.font)
    XFreeFont(blackbox->getXDisplay(), resource.wstyle.font);
  if (resource.mstyle.t_font)
    XFreeFont(blackbox->getXDisplay(), resource.mstyle.t_font);
  if (resource.mstyle.f_font)
    XFreeFont(blackbox->getXDisplay(), resource.mstyle.f_font);
  if (resource.tstyle.font)
    XFreeFont(blackbox->getXDisplay(), resource.tstyle.font);

  XFreeGC(blackbox->getXDisplay(), opGC);
}


void BScreen::removeWorkspaceNames(void) {
  workspaceNames.clear();
}


void BScreen::reconfigure(void) {
  LoadStyle();

  XGCValues gcv;
  unsigned long gc_value_mask = GCForeground;
  if (! i18n.multibyte()) gc_value_mask |= GCFont;

  gcv.foreground = WhitePixel(blackbox->getXDisplay(),
                              getScreenNumber());
  gcv.function = GXinvert;
  gcv.subwindow_mode = IncludeInferiors;
  XChangeGC(blackbox->getXDisplay(), opGC,
            GCForeground | GCFunction | GCSubwindowMode, &gcv);

  const char *s = i18n(ScreenSet, ScreenPositionLength,
                       "0: 0000 x 0: 0000");
  int l = strlen(s);

  if (i18n.multibyte()) {
    XRectangle ink, logical;
    XmbTextExtents(resource.wstyle.fontset, s, l, &ink, &logical);
    geom_w = logical.width;

    geom_h = resource.wstyle.fontset_extents->max_ink_extent.height;
  } else {
    geom_w = XTextWidth(resource.wstyle.font, s, l);

    geom_h = resource.wstyle.font->ascent + resource.wstyle.font->descent;
  }

  geom_w += (resource.bevel_width * 2);
  geom_h += (resource.bevel_width * 2);

  BTexture* texture = &(resource.wstyle.l_focus);
  geom_pixmap = texture->render(geom_w, geom_h, geom_pixmap);
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

  int remember_sub = rootmenu->getCurrentSubmenu();
  InitMenu();
  raiseWindows(0, 0);
  rootmenu->reconfigure();
  rootmenu->drawSubmenu(remember_sub);

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
  resource.stylerc = XrmGetFileDatabase(blackbox->getStyleFilename());
  if (! resource.stylerc)
    resource.stylerc = XrmGetFileDatabase(DEFAULTSTYLE);

  XrmValue value;
  char *value_type;

  // load fonts/fontsets
  if (resource.wstyle.fontset)
    XFreeFontSet(blackbox->getXDisplay(), resource.wstyle.fontset);
  if (resource.tstyle.fontset)
    XFreeFontSet(blackbox->getXDisplay(), resource.tstyle.fontset);
  if (resource.mstyle.f_fontset)
    XFreeFontSet(blackbox->getXDisplay(), resource.mstyle.f_fontset);
  if (resource.mstyle.t_fontset)
    XFreeFontSet(blackbox->getXDisplay(), resource.mstyle.t_fontset);
  resource.wstyle.fontset = 0;
  resource.tstyle.fontset = 0;
  resource.mstyle.f_fontset = 0;
  resource.mstyle.t_fontset = 0;
  if (resource.wstyle.font)
    XFreeFont(blackbox->getXDisplay(), resource.wstyle.font);
  if (resource.tstyle.font)
    XFreeFont(blackbox->getXDisplay(), resource.tstyle.font);
  if (resource.mstyle.f_font)
    XFreeFont(blackbox->getXDisplay(), resource.mstyle.f_font);
  if (resource.mstyle.t_font)
    XFreeFont(blackbox->getXDisplay(), resource.mstyle.t_font);
  resource.wstyle.font = 0;
  resource.tstyle.font = 0;
  resource.mstyle.f_font = 0;
  resource.mstyle.t_font = 0;

  if (i18n.multibyte()) {
    resource.wstyle.fontset =
      readDatabaseFontSet("window.font", "Window.Font");
    resource.tstyle.fontset =
      readDatabaseFontSet("toolbar.font", "Toolbar.Font");
    resource.mstyle.t_fontset =
      readDatabaseFontSet("menu.title.font", "Menu.Title.Font");
    resource.mstyle.f_fontset =
      readDatabaseFontSet("menu.frame.font", "Menu.Frame.Font");

    resource.mstyle.t_fontset_extents =
      XExtentsOfFontSet(resource.mstyle.t_fontset);
    resource.mstyle.f_fontset_extents =
      XExtentsOfFontSet(resource.mstyle.f_fontset);
    resource.tstyle.fontset_extents =
      XExtentsOfFontSet(resource.tstyle.fontset);
    resource.wstyle.fontset_extents =
      XExtentsOfFontSet(resource.wstyle.fontset);
  } else {
    resource.wstyle.font =
      readDatabaseFont("window.font", "Window.Font");
    resource.tstyle.font =
      readDatabaseFont("toolbar.font", "Toolbar.Font");
    resource.mstyle.t_font =
      readDatabaseFont("menu.title.font", "Menu.Title.Font");
    resource.mstyle.f_font =
      readDatabaseFont("menu.frame.font", "Menu.Frame.Font");
  }

  // load window config
  resource.wstyle.t_focus =
    readDatabaseTexture("window.title.focus", "Window.Title.Focus", "white");
  resource.wstyle.t_unfocus =
    readDatabaseTexture("window.title.unfocus",
                        "Window.Title.Unfocus", "black");
  resource.wstyle.l_focus =
    readDatabaseTexture("window.label.focus", "Window.Label.Focus", "white" );
  resource.wstyle.l_unfocus =
    readDatabaseTexture("window.label.unfocus", "Window.Label.Unfocus",
                        "black");
  resource.wstyle.h_focus =
    readDatabaseTexture("window.handle.focus", "Window.Handle.Focus", "white");
  resource.wstyle.h_unfocus =
    readDatabaseTexture("window.handle.unfocus",
                        "Window.Handle.Unfocus", "black");
  resource.wstyle.g_focus =
    readDatabaseTexture("window.grip.focus", "Window.Grip.Focus", "white");
  resource.wstyle.g_unfocus =
    readDatabaseTexture("window.grip.unfocus", "Window.Grip.Unfocus", "black");
  resource.wstyle.b_focus =
    readDatabaseTexture("window.button.focus", "Window.Button.Focus", "white");
  resource.wstyle.b_unfocus =
    readDatabaseTexture("window.button.unfocus",
                        "Window.Button.Unfocus", "black");
  resource.wstyle.b_pressed =
    readDatabaseTexture("window.button.pressed",
                        "Window.Button.Pressed", "black");
  resource.wstyle.f_focus =
    readDatabaseColor("window.frame.focusColor",
                      "Window.Frame.FocusColor", "white");
  resource.wstyle.f_unfocus =
    readDatabaseColor("window.frame.unfocusColor",
                      "Window.Frame.UnfocusColor", "black");
  resource.wstyle.l_text_focus =
    readDatabaseColor("window.label.focus.textColor",
                      "Window.Label.Focus.TextColor", "black");
  resource.wstyle.l_text_unfocus =
    readDatabaseColor("window.label.unfocus.textColor",
                      "Window.Label.Unfocus.TextColor", "white");
  resource.wstyle.b_pic_focus =
    readDatabaseColor("window.button.focus.picColor",
                      "Window.Button.Focus.PicColor", "black");
  resource.wstyle.b_pic_unfocus =
    readDatabaseColor("window.button.unfocus.picColor",
                      "Window.Button.Unfocus.PicColor", "white");

  resource.wstyle.justify = LeftJustify;
  if (XrmGetResource(resource.stylerc, "window.justify", "Window.Justify",
                     &value_type, &value)) {
    if (strstr(value.addr, "right") || strstr(value.addr, "Right"))
      resource.wstyle.justify = RightJustify;
    else if (strstr(value.addr, "center") || strstr(value.addr, "Center"))
      resource.wstyle.justify = CenterJustify;
  }

  // load toolbar config
  resource.tstyle.toolbar =
    readDatabaseTexture("toolbar", "Toolbar", "black");
  resource.tstyle.label =
    readDatabaseTexture("toolbar.label", "Toolbar.Label", "black");
  resource.tstyle.window =
    readDatabaseTexture("toolbar.windowLabel", "Toolbar.WindowLabel", "black");
  resource.tstyle.button =
    readDatabaseTexture("toolbar.button", "Toolbar.Button", "white");
  resource.tstyle.pressed =
    readDatabaseTexture("toolbar.button.pressed",
                        "Toolbar.Button.Pressed", "black");
  resource.tstyle.clock =
    readDatabaseTexture("toolbar.clock", "Toolbar.Clock", "black");
  resource.tstyle.l_text =
    readDatabaseColor("toolbar.label.textColor",
                      "Toolbar.Label.TextColor", "white");
  resource.tstyle.w_text =
    readDatabaseColor("toolbar.windowLabel.textColor",
                      "Toolbar.WindowLabel.TextColor", "white");
  resource.tstyle.c_text =
    readDatabaseColor("toolbar.clock.textColor",
                      "Toolbar.Clock.TextColor", "white");
  resource.tstyle.b_pic =
    readDatabaseColor("toolbar.button.picColor",
                      "Toolbar.Button.PicColor", "black");

  resource.tstyle.justify = LeftJustify;
  if (XrmGetResource(resource.stylerc, "toolbar.justify",
                     "Toolbar.Justify", &value_type, &value)) {
    if (strstr(value.addr, "right") || strstr(value.addr, "Right"))
      resource.tstyle.justify = RightJustify;
    else if (strstr(value.addr, "center") || strstr(value.addr, "Center"))
      resource.tstyle.justify = CenterJustify;
  }

  // load menu config
  resource.mstyle.title =
    readDatabaseTexture("menu.title", "Menu.Title", "white");
  resource.mstyle.frame =
    readDatabaseTexture("menu.frame", "Menu.Frame", "black");
  resource.mstyle.hilite =
    readDatabaseTexture("menu.hilite", "Menu.Hilite", "white");
  resource.mstyle.t_text =
    readDatabaseColor("menu.title.textColor", "Menu.Title.TextColor", "black");
  resource.mstyle.f_text =
    readDatabaseColor("menu.frame.textColor", "Menu.Frame.TextColor", "white");
  resource.mstyle.d_text =
    readDatabaseColor("menu.frame.disableColor",
                      "Menu.Frame.DisableColor", "black");
  resource.mstyle.h_text =
    readDatabaseColor("menu.hilite.textColor",
                      "Menu.Hilite.TextColor", "black");

  resource.mstyle.t_justify = LeftJustify;
  if (XrmGetResource(resource.stylerc, "menu.title.justify",
                     "Menu.Title.Justify",
                     &value_type, &value)) {
    if (strstr(value.addr, "right") || strstr(value.addr, "Right"))
      resource.mstyle.t_justify = RightJustify;
    else if (strstr(value.addr, "center") || strstr(value.addr, "Center"))
      resource.mstyle.t_justify = CenterJustify;
  }

  resource.mstyle.f_justify = LeftJustify;
  if (XrmGetResource(resource.stylerc, "menu.frame.justify",
                     "Menu.Frame.Justify",
                     &value_type, &value)) {
    if (strstr(value.addr, "right") || strstr(value.addr, "Right"))
      resource.mstyle.f_justify = RightJustify;
    else if (strstr(value.addr, "center") || strstr(value.addr, "Center"))
      resource.mstyle.f_justify = CenterJustify;
  }

  resource.mstyle.bullet = Basemenu::Triangle;
  if (XrmGetResource(resource.stylerc, "menu.bullet", "Menu.Bullet",
                     &value_type, &value)) {
    if (! strncasecmp(value.addr, "empty", value.size))
      resource.mstyle.bullet = Basemenu::Empty;
    else if (! strncasecmp(value.addr, "square", value.size))
      resource.mstyle.bullet = Basemenu::Square;
    else if (! strncasecmp(value.addr, "diamond", value.size))
      resource.mstyle.bullet = Basemenu::Diamond;
  }

  resource.mstyle.bullet_pos = Basemenu::Left;
  if (XrmGetResource(resource.stylerc, "menu.bullet.position",
                     "Menu.Bullet.Position", &value_type, &value)) {
    if (! strncasecmp(value.addr, "right", value.size))
      resource.mstyle.bullet_pos = Basemenu::Right;
  }

  resource.border_color =
    readDatabaseColor("borderColor", "BorderColor", "black");

  unsigned int uint_value;

  // load bevel, border and handle widths
  resource.handle_width = 6;
  if (XrmGetResource(resource.stylerc, "handleWidth", "HandleWidth",
                     &value_type, &value) &&
      sscanf(value.addr, "%u", &uint_value) == 1 &&
      uint_value <= (getWidth() / 2) && uint_value != 0) {
    resource.handle_width = uint_value;
  }

  resource.border_width = 1;
  if (XrmGetResource(resource.stylerc, "borderWidth", "BorderWidth",
                     &value_type, &value) &&
      sscanf(value.addr, "%u", &uint_value) == 1) {
    resource.border_width = uint_value;
  }

  resource.bevel_width = 3;
  if (XrmGetResource(resource.stylerc, "bevelWidth", "BevelWidth",
                     &value_type, &value) &&
      sscanf(value.addr, "%u", &uint_value) == 1 &&
      uint_value <= (getWidth() / 2) && uint_value != 0) {
    resource.bevel_width = uint_value;
  }

  resource.frame_width = resource.bevel_width;
  if (XrmGetResource(resource.stylerc, "frameWidth", "FrameWidth",
                     &value_type, &value) &&
      sscanf(value.addr, "%u", &uint_value) == 1 &&
      uint_value <= (getWidth() / 2)) {
    resource.frame_width = uint_value;
  }

  if (XrmGetResource(resource.stylerc, "rootCommand", "RootCommand",
                     &value_type, &value)) {
    bexec(value.addr, displayString());
  }

  XrmDestroyDatabase(resource.stylerc);
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

  workspacemenu->insert(wkspc->getName(), wkspc->getMenu(),
                        wkspc->getID() + 2);
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

  workspacemenu->remove(wkspc->getID() + 2);
  workspacemenu->update();

  workspacesList.pop_back();
  delete wkspc;

  toolbar->reconfigure();

  updateNetizenWorkspaceCount();

  return workspacesList.size();
}


void BScreen::changeWorkspaceID(unsigned int id) {
  if (! current_workspace) return;

  if (id != current_workspace->getID()) {
    current_workspace->hideAll();

    workspacemenu->setItemSelected(current_workspace->getID() + 2, False);

    if (blackbox->getFocusedWindow() &&
        blackbox->getFocusedWindow()->getScreen() == this &&
        (! blackbox->getFocusedWindow()->isStuck())) {
      current_workspace->setLastFocusedWindow(blackbox->getFocusedWindow());
      blackbox->setFocusedWindow((BlackboxWindow *) 0);
    }

    current_workspace = getWorkspace(id);

    workspacemenu->setItemSelected(current_workspace->getID() + 2, True);
    toolbar->redrawWorkspaceLabel(True);

    current_workspace->showAll();

    if (resource.focus_last && current_workspace->getLastFocusedWindow()) {
      XSync(blackbox->getXDisplay(), False);
      current_workspace->getLastFocusedWindow()->setInputFocus();
    }
  }

  updateNetizenCurrentWorkspace();
}


void BScreen::manageWindow(Window w) {
  new BlackboxWindow(blackbox, w, this);

  BlackboxWindow *win = blackbox->searchWindow(w);
  if (! win)
    return;

  windowList.push_back(win);

  XMapRequestEvent mre;
  mre.window = w;
  win->restoreAttributes();
  win->mapRequestEvent(&mre);
}


void BScreen::unmanageWindow(BlackboxWindow *w, bool remap) {
  w->restore(remap);

  if (w->getWorkspaceNumber() != BSENTINEL &&
      w->getWindowNumber() != BSENTINEL)
    getWorkspace(w->getWorkspaceNumber())->removeWindow(w);
  else if (w->isIconic())
    removeIcon(w);

  windowList.remove(w);

  if (blackbox->getFocusedWindow() == w)
    blackbox->setFocusedWindow((BlackboxWindow *) 0);

  removeNetizen(w->getClientWindow());

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


void BScreen::updateNetizenCurrentWorkspace(void) {
  std::for_each(netizenList.begin(), netizenList.end(),
                std::mem_fun(&Netizen::sendCurrentWorkspace));
}


void BScreen::updateNetizenWorkspaceCount(void) {
  std::for_each(netizenList.begin(), netizenList.end(),
                std::mem_fun(&Netizen::sendWorkspaceCount));
}


void BScreen::updateNetizenWindowFocus(void) {
  Window f = ((blackbox->getFocusedWindow()) ?
              blackbox->getFocusedWindow()->getClientWindow() : None);
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
  // XXX: why 13??
  Window *session_stack = new
    Window[(num + workspacesList.size() + rootmenuList.size() + 13)];
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
}


#ifdef    HAVE_STRFTIME
void BScreen::saveStrftimeFormat(const string& format) {
  resource.strftime_format = format;
}
#endif // HAVE_STRFTIME


void BScreen::addWorkspaceName(const string& name) {
  workspaceNames.push_back(name);
}


/*
 * I would love to kill this function and the accompanying workspaceNames
 * list.  However, we have a chicken and egg situation.  The names are read
 * in during load_rc() which happens before the workspaces are created.
 * The current solution is to read the names into a list, then use the list
 * later for constructing the workspaces.  It is only used during initial
 * BScreen creation.
 */
const string BScreen::getNameOfWorkspace(unsigned int id) {
  if (id < workspaceNames.size())
    return workspaceNames[id];
  return string("");
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
  } else if (ignore_sticky || ! w->isStuck()) {
    getWorkspace(w->getWorkspaceNumber())->removeWindow(w);
    getWorkspace(wkspc_id)->addWindow(w);
  }
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
    } while(!next->setInputFocus() && next != focused);

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
    } while(!next->setInputFocus() && next != focused);

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

  if (blackbox->getMenuFilename()) {
    FILE *menu_file = fopen(blackbox->getMenuFilename(), "r");

    if (!menu_file) {
      perror(blackbox->getMenuFilename());
    } else {
      if (feof(menu_file)) {
        fprintf(stderr, i18n(ScreenSet, ScreenEmptyMenuFile,
                             "%s: Empty menu file"),
                blackbox->getMenuFilename());
      } else {
        char line[1024], label[1024];
        memset(line, 0, 1024);
        memset(label, 0, 1024);

        while (fgets(line, 1024, menu_file) && ! feof(menu_file)) {
          if (line[0] != '#') {
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
              break;
            }
          }
        }
      }
      fclose(menu_file);
    }
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
                            "Blackbox Menu"));
  } else {
    blackbox->saveMenuFilename(blackbox->getMenuFilename());
  }
}


bool BScreen::parseMenuFile(FILE *file, Rootmenu *menu) {
  char line[1024], label[1024], command[1024];

  while (! feof(file)) {
    memset(line, 0, 1024);
    memset(label, 0, 1024);
    memset(command, 0, 1024);

    if (fgets(line, 1024, file)) {
      if (line[0] != '#') {
        int i, key = 0, parse = 0, index = -1, line_length = strlen(line);

        // determine the keyword
        for (i = 0; i < line_length; i++) {
          if (line[i] == '[') parse = 1;
          else if (line[i] == ']') break;
          else if (line[i] != ' ')
            if (parse)
              key += tolower(line[i]);
        }

        // get the label enclosed in ()'s
        parse = 0;

        for (i = 0; i < line_length; i++) {
          if (line[i] == '(') {
            index = 0;
            parse = 1;
          } else if (line[i] == ')') break;
          else if (index++ >= 0) {
            if (line[i] == '\\' && i < line_length - 1) i++;
            label[index - 1] = line[i];
          }
        }

        if (parse) {
          label[index] = '\0';
        } else {
          label[0] = '\0';
        }

        // get the command enclosed in {}'s
        parse = 0;
        index = -1;
        for (i = 0; i < line_length; i++) {
          if (line[i] == '{') {
            index = 0;
            parse = 1;
          } else if (line[i] == '}') break;
          else if (index++ >= 0) {
            if (line[i] == '\\' && i < line_length - 1) i++;
            command[index - 1] = line[i];
          }
        }

        if (parse) {
          command[index] = '\0';
        } else {
          command[0] = '\0';
        }

        switch (key) {
        case 311: // end
          return ((menu->getCount() == 0) ? True : False);

          break;

        case 333: // nop
          if (! *label)
            label[0] = '\0';
          menu->insert(label);

          break;

        case 421: // exec
          if ((! *label) && (! *command)) {
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

        case 561: // style
          {
            if ((! *label) || (! *command)) {
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

        case 740: // include
          {
            if (! *label) {
              fprintf(stderr, i18n(ScreenSet, ScreenINCLUDEError,
                                   "BScreen::parseMenuFile: [include] error, "
                                   "no filename defined\n"));
              continue;
            }

            string newfile = expandTilde(label);
            FILE *submenufile = fopen(newfile.c_str(), "r");

            if (submenufile) {
              struct stat buf;
              if (fstat(fileno(submenufile), &buf) ||
                  (! S_ISREG(buf.st_mode))) {
                fprintf(stderr,
                        i18n(ScreenSet, ScreenINCLUDEErrorReg,
                             "BScreen::parseMenuFile: [include] error: "
                             "'%s' is not a regular file\n"), newfile.c_str());
                break;
              }

              if (! feof(submenufile)) {
                if (! parseMenuFile(submenufile, menu))
                  blackbox->saveMenuFilename(newfile);

                fclose(submenufile);
              }
            } else {
              perror(newfile.c_str());
            }
          }

          break;

        case 767: // submenu
          {
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

        case 773: // restart
          {
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

        case 845: // reconfig
          {
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

        case 995: // stylesdir
        case 1113: // stylesmenu
          {
            bool newmenu = ((key == 1113) ? True : False);

            if ((! *label) || ((! *command) && newmenu)) {
              fprintf(stderr,
                      i18n(ScreenSet, ScreenSTYLESDIRError,
                           "BScreen::parseMenuFile: [stylesdir/stylesmenu]"
                           " error, no directory defined\n"));
              continue;
            }

            char *directory = ((newmenu) ? command : label);

            string stylesdir = expandTilde(directory);

            struct stat statbuf;

            if (! stat(stylesdir.c_str(), &statbuf)) {
              if (S_ISDIR(statbuf.st_mode)) {
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

                  if ((! stat(style.c_str(), &statbuf)) &&
                      S_ISREG(statbuf.st_mode))
                    stylesmenu->insert(fname, BScreen::SetStyle, style);
                }

                stylesmenu->update();

                if (newmenu) {
                  stylesmenu->setLabel(label);
                  menu->insert(label, stylesmenu);
                  rootmenuList.push_back(stylesmenu);
                }

                blackbox->saveMenuFilename(stylesdir);
              } else {
                fprintf(stderr,
                        i18n(ScreenSet, ScreenSTYLESDIRErrorNotDir,
                             "BScreen::parseMenuFile:"
                             " [stylesdir/stylesmenu] error, %s is not a"
                             " directory\n"), stylesdir.c_str());
              }
            } else {
              fprintf(stderr,
                      i18n(ScreenSet, ScreenSTYLESDIRErrorNoExist,
                           "BScreen::parseMenuFile: [stylesdir/stylesmenu]"
                           " error, %s does not exist\n"), stylesdir.c_str());
            }
            break;
          }

        case 1090: // workspaces
          {
            if (! *label) {
              fprintf(stderr,
                      i18n(ScreenSet, ScreenWORKSPACESError,
                           "BScreen:parseMenuFile: [workspaces] error, "
                           "no menu label defined\n"));
              continue;
            }

            menu->insert(label, workspacemenu);

            break;
          }
        }
      }
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

  BPen pen(resource.wstyle.l_text_focus, resource.wstyle.font);
  if (i18n.multibyte()) {
    XmbDrawString(blackbox->getXDisplay(), geom_window,
                  resource.wstyle.fontset, pen.gc(),
                  resource.bevel_width, resource.bevel_width -
                  resource.wstyle.fontset_extents->max_ink_extent.y,
                  label, strlen(label));
  } else {
    XDrawString(blackbox->getXDisplay(), geom_window,
                pen.gc(), resource.bevel_width,
                resource.wstyle.font->ascent + resource.bevel_width,
                label, strlen(label));
  }
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

  BPen pen(resource.wstyle.l_text_focus, resource.wstyle.font);
  if (i18n.multibyte()) {
    XmbDrawString(blackbox->getXDisplay(), geom_window,
                  resource.wstyle.fontset, pen.gc(),
                  resource.bevel_width, resource.bevel_width -
                  resource.wstyle.fontset_extents->max_ink_extent.y,
                  label, strlen(label));
  } else {
    XDrawString(blackbox->getXDisplay(), geom_window,
                pen.gc(), resource.bevel_width,
                resource.wstyle.font->ascent +
                resource.bevel_width, label, strlen(label));
  }
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


void BScreen::updateAvailableArea(void) {
  Rect old_area = usableArea;
  usableArea = getRect(); // reset to full screen

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

  if (old_area != usableArea) {
    BlackboxWindowList::iterator it = windowList.begin(),
      end = windowList.end();
    for (; it != end; ++it)
      if ((*it)->isMaximized()) (*it)->remaximize();
  }
}


Workspace* BScreen::getWorkspace(unsigned int index) {
  assert(index < workspacesList.size());
  return workspacesList[index];
}


void BScreen::buttonPressEvent(XButtonEvent *xbutton) {
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
  }
}


void BScreen::toggleFocusModel(FocusModel model) {
  if (model == SloppyFocus) {
    saveSloppyFocus(True);
  } else {
    saveSloppyFocus(False);
    saveAutoRaise(False);
    saveClickRaise(False);
  }

  updateFocusModel();
}


void BScreen::updateFocusModel()
{
  std::for_each(workspacesList.begin(), workspacesList.end(),
                std::mem_fun(&Workspace::updateFocusModel));
}


BTexture BScreen::readDatabaseTexture(const string &rname,
                                      const string &rclass,
                                      const string &default_color) {
  BTexture texture;
  XrmValue value;
  char *value_type;

  if (XrmGetResource(resource.stylerc, rname.c_str(), rclass.c_str(),
                     &value_type, &value))
    texture = BTexture(value.addr);
  else
    texture.setTexture(BTexture::Solid | BTexture::Flat);

  // associate this texture with this screen
  texture.setDisplay(getBaseDisplay(), getScreenNumber());
  texture.setImageControl(image_control);

  if (texture.texture() & BTexture::Solid) {
    texture.setColor(readDatabaseColor(rname + ".color",
                                       rclass + ".Color",
                                       default_color));
    texture.setColorTo(readDatabaseColor(rname + ".colorTo",
                                         rclass + ".ColorTo",
                                         default_color));
  } else if (texture.texture() & BTexture::Gradient) {
    texture.setColor(readDatabaseColor(rname + ".color",
                                       rclass + ".Color",
                                       default_color));
    texture.setColorTo(readDatabaseColor(rname + ".colorTo",
                                         rclass + ".ColorTo",
                                         default_color));
  }

  return texture;
}


BColor BScreen::readDatabaseColor(const string &rname, const string &rclass,
				  const string &default_color) {
  BColor color;
  XrmValue value;
  char *value_type;
  if (XrmGetResource(resource.stylerc, rname.c_str(), rclass.c_str(),
                     &value_type, &value))
    color = BColor(value.addr, getBaseDisplay(), getScreenNumber());
  else
    color = BColor(default_color, getBaseDisplay(), getScreenNumber());
  return color;
}


XFontSet BScreen::readDatabaseFontSet(const string &rname,
                                      const string &rclass) {
  char *defaultFont = "fixed";

  bool load_default = True;
  XrmValue value;
  char *value_type;
  XFontSet fontset = 0;
  if (XrmGetResource(resource.stylerc, rname.c_str(), rclass.c_str(),
                     &value_type, &value) &&
      (fontset = createFontSet(value.addr))) {
    load_default = False;
  }

  if (load_default) {
    fontset = createFontSet(defaultFont);

    if (! fontset) {
      fprintf(stderr,
              i18n(ScreenSet, ScreenDefaultFontLoadFail,
                   "BScreen::setCurrentStyle(): couldn't load default font.\n"));
      exit(2);
    }
  }

  return fontset;
}


XFontStruct *BScreen::readDatabaseFont(const string &rname,
                                       const string &rclass) {
  char *defaultFont = "fixed";

  bool load_default = False;
  XrmValue value;
  char *value_type;
  XFontStruct *font = 0;
  if (XrmGetResource(resource.stylerc, rname.c_str(), rclass.c_str(),
                     &value_type, &value)) {
    if ((font = XLoadQueryFont(blackbox->getXDisplay(), value.addr)) == NULL) {
      fprintf(stderr,
              i18n(ScreenSet, ScreenFontLoadFail,
                   "BScreen::setCurrentStyle(): couldn't load font '%s'\n"),
              value.addr);

      load_default = True;
    }
  } else {
    load_default = True;
  }

  if (load_default) {
    font = XLoadQueryFont(blackbox->getXDisplay(), defaultFont);
    if (font == NULL) {
      fprintf(stderr,
              i18n(ScreenSet, ScreenDefaultFontLoadFail,
                   "BScreen::setCurrentStyle(): couldn't load default font.\n"));
      exit(2);
    }
  }

  return font;
}


#ifndef    HAVE_STRCASESTR
static const char * strcasestr(const char *str, const char *ptn) {
  const char *s2, *p2;
  for(; *str; str++) {
    for(s2=str,p2=ptn; ; s2++,p2++) {
      if (!*p2) return str;
      if (toupper(*s2) != toupper(*p2)) break;
    }
  }
  return NULL;
}
#endif // HAVE_STRCASESTR


static const char *getFontElement(const char *pattern, char *buf,
                                  int bufsiz, ...) {
  const char *p, *v;
  char *p2;
  va_list va;

  va_start(va, bufsiz);
  buf[bufsiz-1] = 0;
  buf[bufsiz-2] = '*';
  while((v = va_arg(va, char *)) != NULL) {
    p = strcasestr(pattern, v);
    if (p) {
      strncpy(buf, p+1, bufsiz-2);
      p2 = strchr(buf, '-');
      if (p2) *p2=0;
      va_end(va);
      return p;
    }
  }
  va_end(va);
  strncpy(buf, "*", bufsiz);
  return NULL;
}


static const char *getFontSize(const char *pattern, int *size) {
  const char *p;
  const char *p2=NULL;
  int n=0;

  for (p=pattern; 1; p++) {
    if (!*p) {
      if (p2!=NULL && n>1 && n<72) {
        *size = n; return p2+1;
      } else {
        *size = 16; return NULL;
      }
    } else if (*p=='-') {
      if (n>1 && n<72 && p2!=NULL) {
        *size = n;
        return p2+1;
      }
      p2=p; n=0;
    } else if (*p>='0' && *p<='9' && p2!=NULL) {
      n *= 10;
      n += *p-'0';
    } else {
      p2=NULL; n=0;
    }
  }
}


XFontSet BScreen::createFontSet(const string &fontname) {
  XFontSet fs;
  char **missing, *def = "-";
  int nmissing, pixel_size = 0, buf_size = 0;
  char weight[FONT_ELEMENT_SIZE], slant[FONT_ELEMENT_SIZE];

  fs = XCreateFontSet(blackbox->getXDisplay(),
                      fontname.c_str(), &missing, &nmissing, &def);
  if (fs && (! nmissing))
    return fs;

  const char *nfontname = fontname.c_str();
#ifdef    HAVE_SETLOCALE
  if (! fs) {
    if (nmissing) XFreeStringList(missing);

    setlocale(LC_CTYPE, "C");
    fs = XCreateFontSet(blackbox->getXDisplay(), fontname.c_str(),
                        &missing, &nmissing, &def);
    setlocale(LC_CTYPE, "");
  }
#endif // HAVE_SETLOCALE

  if (fs) {
    XFontStruct **fontstructs;
    char **fontnames;
    XFontsOfFontSet(fs, &fontstructs, &fontnames);
    nfontname = fontnames[0];
  }

  getFontElement(nfontname, weight, FONT_ELEMENT_SIZE,
                 "-medium-", "-bold-", "-demibold-", "-regular-", NULL);
  getFontElement(nfontname, slant, FONT_ELEMENT_SIZE,
                 "-r-", "-i-", "-o-", "-ri-", "-ro-", NULL);
  getFontSize(nfontname, &pixel_size);

  if (! strcmp(weight, "*"))
    strncpy(weight, "medium", FONT_ELEMENT_SIZE);
  if (! strcmp(slant, "*"))
    strncpy(slant, "r", FONT_ELEMENT_SIZE);
  if (pixel_size < 3)
    pixel_size = 3;
  else if (pixel_size > 97)
    pixel_size = 97;

  buf_size = strlen(nfontname) + (FONT_ELEMENT_SIZE * 2) + 64;
  char *pattern2 = new char[buf_size];
  sprintf(pattern2,
           "%s,"
           "-*-*-%s-%s-*-*-%d-*-*-*-*-*-*-*,"
           "-*-*-*-*-*-*-%d-*-*-*-*-*-*-*,*",
           nfontname, weight, slant, pixel_size, pixel_size);
  nfontname = pattern2;

  if (nmissing)
    XFreeStringList(missing);
  if (fs)
    XFreeFontSet(blackbox->getXDisplay(), fs);

  fs = XCreateFontSet(blackbox->getXDisplay(), nfontname, &missing,
                      &nmissing, &def);

  delete [] pattern2;

  return fs;
}
