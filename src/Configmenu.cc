// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Configmenu.cc for Blackbox - An X11 Window Manager
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
# include "../config.h"
#endif // HAVE_CONFIG_H

#include "i18n.hh"
#include "Configmenu.hh"
#include "Image.hh"
#include "Toolbar.hh"
#include "Window.hh"
#include "Screen.hh"

Configmenu::Configmenu(BScreen *scr) : Basemenu(scr) {
  setLabel(i18n(ConfigmenuSet, ConfigmenuConfigOptions, "Config options"));
  setInternalMenu();

  focusmenu = new Focusmenu(this);
  placementmenu = new Placementmenu(this);
  windowsnapmenu = new WindowToWindowSnapmenu(this);
  edgesnapmenu = new WindowToEdgeSnapmenu(this);
#ifdef    XINERAMA
  xineramamenu = new Xineramamenu(this);
#endif // XINERAMA

  insert(i18n(ConfigmenuSet, ConfigmenuFocusModel,
              "Focus Model"), focusmenu);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowPlacement,
              "Window Placement"), placementmenu);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowToWindowSnap,
              "Window-To-Window Snapping"), windowsnapmenu);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowToEdgeSnap,
              "Window-To-Edge Snapping"), edgesnapmenu);
#ifdef    XINERAMA
  insert(i18n(ConfigmenuSet, ConfigmenuXineramaSupport,
              "XineramaSupport"), xineramamenu);
#endif // XINERAMA
  insert(i18n(ConfigmenuSet, ConfigmenuImageDithering,
              "Image Dithering"), 1);
  insert(i18n(ConfigmenuSet, ConfigmenuOpaqueMove,
              "Opaque Window Moving"), 2);
  insert(i18n(ConfigmenuSet, ConfigmenuWorkspaceWarping,
              "Workspace Warping"), 3);
  insert(i18n(ConfigmenuSet, ConfigmenuFullMax,
              "Full Maximization"), 4);
  insert(i18n(ConfigmenuSet, ConfigmenuFocusNew,
              "Focus New Windows"), 5);
  insert(i18n(ConfigmenuSet, ConfigmenuFocusLast,
              "Focus Last Window on Workspace"), 6);
  insert(i18n(ConfigmenuSet, ConfigmenuDisableBindings,
              "Disable Mouse with Scroll Lock"), 7);
  insert(i18n(ConfigmenuSet, ConfigmenuHideToolbar,
              "Hide Toolbar"), 8);
  update();
  setValues();
}


void Configmenu::setValues(void) {
  int index = 4;
#ifdef    XINERAMA
  ++index;
#endif // XINERAMA
  setItemSelected(index++, getScreen()->doImageDither());
  setItemSelected(index++, getScreen()->doOpaqueMove());
  setItemSelected(index++, getScreen()->doWorkspaceWarping());
  setItemSelected(index++, getScreen()->doFullMax());
  setItemSelected(index++, getScreen()->doFocusNew());
  setItemSelected(index++, getScreen()->doFocusLast());
  setItemSelected(index++, getScreen()->allowScrollLock());
  setItemSelected(index++, getScreen()->doHideToolbar());
}


Configmenu::~Configmenu(void) {
  delete focusmenu;
  delete placementmenu;
  delete windowsnapmenu;
  delete edgesnapmenu;
#ifdef    XINERAMA
  delete xineramamenu;
#endif // XINERAMA
}


void Configmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (! item->function())
    return;

  switch(item->function()) {
  case 1: // dither
    getScreen()->saveImageDither(! getScreen()->doImageDither());
    setItemSelected(index, getScreen()->doImageDither());
    break;

  case 2: // opaque move
    getScreen()->saveOpaqueMove(! getScreen()->doOpaqueMove());
    setItemSelected(index, getScreen()->doOpaqueMove());
    break;

  case 3: // workspace wrapping
    getScreen()->saveWorkspaceWarping(! getScreen()->doWorkspaceWarping());
    setItemSelected(index, getScreen()->doWorkspaceWarping());
    break;

  case 4: // full maximization
    getScreen()->saveFullMax(! getScreen()->doFullMax());
    setItemSelected(index, getScreen()->doFullMax());
    break;

  case 5: // focus new windows
    getScreen()->saveFocusNew(! getScreen()->doFocusNew());
    setItemSelected(index, getScreen()->doFocusNew());
    break;

  case 6: // focus last window on workspace
    getScreen()->saveFocusLast(! getScreen()->doFocusLast());
    setItemSelected(index, getScreen()->doFocusLast());
    break;

  case 7: // disable mouse bindings with Scroll Lock
    getScreen()->saveAllowScrollLock(! getScreen()->allowScrollLock());
    setItemSelected(index, getScreen()->allowScrollLock());
    getScreen()->reconfigure();
    break;

  case 8: // hide toolbar
    getScreen()->saveHideToolbar(! getScreen()->doHideToolbar());
    setItemSelected(index, getScreen()->doHideToolbar());
    break;
  }
}


void Configmenu::reconfigure(void) {
  setValues();
  focusmenu->reconfigure();
  placementmenu->reconfigure();
  windowsnapmenu->reconfigure();
  edgesnapmenu->reconfigure();
#ifdef    XINERAMA
  xineramamenu->reconfigure();
#endif // XINERAMA

  Basemenu::reconfigure();
}


Configmenu::Focusmenu::Focusmenu(Configmenu *cm) : Basemenu(cm->getScreen()) {
  setLabel(i18n(ConfigmenuSet, ConfigmenuFocusModel, "Focus Model"));
  setInternalMenu();

  insert(i18n(ConfigmenuSet, ConfigmenuClickToFocus, "Click To Focus"), 1);
  insert(i18n(ConfigmenuSet, ConfigmenuSloppyFocus, "Sloppy Focus"), 2);
  insert(i18n(ConfigmenuSet, ConfigmenuAutoRaise, "Auto Raise"), 3);
  insert(i18n(ConfigmenuSet, ConfigmenuClickRaise, "Click Raise"), 4);
  update();
  setValues();
}


void Configmenu::Focusmenu::setValues(void) {
  setItemSelected(0, ! getScreen()->isSloppyFocus());
  setItemSelected(1, getScreen()->isSloppyFocus());
  setItemEnabled(2, getScreen()->isSloppyFocus());
  setItemSelected(2, getScreen()->doAutoRaise());
  setItemEnabled(3, getScreen()->isSloppyFocus());
  setItemSelected(3, getScreen()->doClickRaise());
}


void Configmenu::Focusmenu::reconfigure(void) {
  setValues();
  Basemenu::reconfigure();
}


void Configmenu::Focusmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (! item->function())
    return;

  switch (item->function()) {
  case 1: // click to focus
    getScreen()->toggleFocusModel(BScreen::ClickToFocus);
    break;

  case 2: // sloppy focus
    getScreen()->toggleFocusModel(BScreen::SloppyFocus);
    break;

  case 3: // auto raise with sloppy focus
    getScreen()->saveAutoRaise(! getScreen()->doAutoRaise());
    break;

  case 4: // click raise with sloppy focus
    getScreen()->saveClickRaise(! getScreen()->doClickRaise());
    // make sure the appropriate mouse buttons are grabbed on the windows
    getScreen()->toggleFocusModel(BScreen::SloppyFocus);
    break;
  }
  setValues();
}


Configmenu::Placementmenu::Placementmenu(Configmenu *cm):
  Basemenu(cm->getScreen()) {
  setLabel(i18n(ConfigmenuSet, ConfigmenuWindowPlacement, "Window Placement"));
  setInternalMenu();

  insert(i18n(ConfigmenuSet, ConfigmenuSmartRows, "Smart Placement (Rows)"),
         BScreen::RowSmartPlacement);
  insert(i18n(ConfigmenuSet, ConfigmenuSmartCols, "Smart Placement (Columns)"),
         BScreen::ColSmartPlacement);
  insert(i18n(ConfigmenuSet, ConfigmenuCascade, "Cascade Placement"),
         BScreen::CascadePlacement);
  insert(i18n(ConfigmenuSet, ConfigmenuUnderMouse, "Under Mouse Placement"),
         BScreen::UnderMousePlacement);
  insert(i18n(ConfigmenuSet, ConfigmenuClickMouse, "Click Mouse Placement"),
         BScreen::ClickMousePlacement);
  insert(i18n(ConfigmenuSet, ConfigmenuLeftRight, "Left to Right"),
         BScreen::LeftRight);
  insert(i18n(ConfigmenuSet, ConfigmenuRightLeft, "Right to Left"),
         BScreen::RightLeft);
  insert(i18n(ConfigmenuSet, ConfigmenuTopBottom, "Top to Bottom"),
         BScreen::TopBottom);
  insert(i18n(ConfigmenuSet, ConfigmenuBottomTop, "Bottom to Top"),
         BScreen::BottomTop);
  insert(i18n(ConfigmenuSet, ConfigmenuIgnoreShaded, "Ignore Shaded Windows"),
         BScreen::IgnoreShaded);
  insert(i18n(ConfigmenuSet, ConfigmenuIgnoreMax,
              "Ignore Full-Maximized Windows"),
         BScreen::IgnoreMaximized);
  update();
  setValues();
}


void Configmenu::Placementmenu::setValues(void) {
  int placement = getScreen()->getPlacementPolicy();
  
  setItemSelected(0, placement == BScreen::RowSmartPlacement);
  setItemSelected(1, placement == BScreen::ColSmartPlacement);
  setItemSelected(2, placement == BScreen::CascadePlacement);
  setItemSelected(3, placement == BScreen::UnderMousePlacement);
  setItemSelected(4, placement == BScreen::ClickMousePlacement);

  bool rl = (getScreen()->getRowPlacementDirection() == BScreen::LeftRight),
       tb = (getScreen()->getColPlacementDirection() == BScreen::TopBottom),
       e = (placement == BScreen::RowSmartPlacement ||
            placement == BScreen::ColSmartPlacement);

  setItemSelected(5, rl);
  setItemSelected(6, ! rl);
  setItemEnabled(5, e);
  setItemEnabled(6, e);

  setItemSelected(7, tb);
  setItemSelected(8, ! tb);
  setItemEnabled(7, e);
  setItemEnabled(8, e);
  
  setItemSelected(9, getScreen()->getPlaceIgnoreShaded());
  setItemSelected(10, getScreen()->getPlaceIgnoreMaximized());
  setItemEnabled(9, e);
  setItemEnabled(10, e);
}


void Configmenu::Placementmenu::reconfigure(void) {
  setValues();
  Basemenu::reconfigure();
}


void Configmenu::Placementmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (! item->function())
    return;

  switch (item->function()) {
  case BScreen::RowSmartPlacement:
    getScreen()->savePlacementPolicy(item->function());
    break;

  case BScreen::ColSmartPlacement:
    getScreen()->savePlacementPolicy(item->function());
    break;

  case BScreen::CascadePlacement:
    getScreen()->savePlacementPolicy(item->function());
    break;

  case BScreen::UnderMousePlacement:
    getScreen()->savePlacementPolicy(item->function());
    break;

  case BScreen::ClickMousePlacement:
    getScreen()->savePlacementPolicy(item->function());
    break;

  case BScreen::LeftRight:
    getScreen()->saveRowPlacementDirection(BScreen::LeftRight);
    break;

  case BScreen::RightLeft:
    getScreen()->saveRowPlacementDirection(BScreen::RightLeft);
    break;

  case BScreen::TopBottom:
    getScreen()->saveColPlacementDirection(BScreen::TopBottom);
    break;

  case BScreen::BottomTop:
    getScreen()->saveColPlacementDirection(BScreen::BottomTop);
    break;
  
  case BScreen::IgnoreShaded:
    getScreen()->savePlaceIgnoreShaded(! getScreen()->getPlaceIgnoreShaded());
    break;

  case BScreen::IgnoreMaximized:
    getScreen()->
      savePlaceIgnoreMaximized(! getScreen()->getPlaceIgnoreMaximized());
    break;
  }
  setValues();
}


Configmenu::WindowToWindowSnapmenu::WindowToWindowSnapmenu(Configmenu *cm) :
    Basemenu(cm->getScreen()) {
  setLabel(i18n(ConfigmenuSet, ConfigmenuWindowToWindowSnap,
                "Window-To-Window Snapping"));
  setInternalMenu();

  insert(i18n(ConfigmenuSet, ConfigmenuWindowDoSnapNo, "No Snapping"), 1);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowDoSnap, "Edge Snapping"), 2);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowDoResistance,
              "Edge Resistance"), 3);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowCornerSnap,
              "Window Corner Snapping"), 4);
  update();
  setValues();
}


void Configmenu::WindowToWindowSnapmenu::setValues(void) {
  setItemSelected(0, (getScreen()->getWindowToWindowSnap() ==
                      BScreen::WindowNoSnap));
  setItemSelected(1, (getScreen()->getWindowToWindowSnap() ==
                      BScreen::WindowSnap));
  setItemSelected(2, (getScreen()->getWindowToWindowSnap() ==
                      BScreen::WindowResistance));

  setItemEnabled(3, getScreen()->getWindowToWindowSnap());
  setItemSelected(3, getScreen()->getWindowCornerSnap());
}


void Configmenu::WindowToWindowSnapmenu::reconfigure(void) {
  setValues();
  Basemenu::reconfigure();
}


void Configmenu::WindowToWindowSnapmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (! item->function())
    return;

  switch (item->function()) {
  case 1: // none
    getScreen()->saveWindowToWindowSnap(BScreen::WindowNoSnap);
    break;

  case 2: // edge snapping
    getScreen()->saveWindowToWindowSnap(BScreen::WindowSnap);
    break;

  case 3: // edge resistance
    getScreen()->saveWindowToWindowSnap(BScreen::WindowResistance);
    break;

  case 4: // window corner snapping
    getScreen()->saveWindowCornerSnap(! getScreen()->getWindowCornerSnap());
    break;

  }
  setValues();
}


Configmenu::WindowToEdgeSnapmenu::WindowToEdgeSnapmenu(Configmenu *cm) :
    Basemenu(cm->getScreen()) {
  setLabel(i18n(ConfigmenuSet, ConfigmenuWindowToEdgeSnap,
                "Window-To-Edge Snapping"));
  setInternalMenu();

  insert(i18n(ConfigmenuSet, ConfigmenuWindowDoSnapNo, "No Snapping"), 1);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowDoSnap, "Edge Snapping"), 2);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowDoResistance,
              "Edge Resistance"), 3);
  update();
  setValues();
}


void Configmenu::WindowToEdgeSnapmenu::setValues(void) {
  setItemSelected(0, (getScreen()->getWindowToEdgeSnap() ==
                      BScreen::WindowNoSnap));
  setItemSelected(1, (getScreen()->getWindowToEdgeSnap() ==
                      BScreen::WindowSnap));
  setItemSelected(2, (getScreen()->getWindowToEdgeSnap() ==
                      BScreen::WindowResistance));
}


void Configmenu::WindowToEdgeSnapmenu::reconfigure(void) {
  setValues();
  Basemenu::reconfigure();
}


void Configmenu::WindowToEdgeSnapmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (! item->function())
    return;

  switch (item->function()) {
  case 1: // none
    getScreen()->saveWindowToEdgeSnap(BScreen::WindowNoSnap);
    break;

  case 2: // edge snapping
    getScreen()->saveWindowToEdgeSnap(BScreen::WindowSnap);
    break;

  case 3: // edge resistance
    getScreen()->saveWindowToEdgeSnap(BScreen::WindowResistance);
    break;
  }
  setValues();
}


#ifdef    XINERAMA
Configmenu::Xineramamenu::Xineramamenu(Configmenu *cm):
  Basemenu(cm->getScreen()) {
  setLabel(i18n(ConfigmenuSet, ConfigmenuXineramaSupport, "Xinerama Support"));
  setInternalMenu();

  insert(i18n(ConfigmenuSet, ConfigmenuXineramaPlacement, "Window Placement"),
         1);
  insert(i18n(ConfigmenuSet, ConfigmenuXineramaMaximizing, "Window Maximizing"),
         2);
  insert(i18n(ConfigmenuSet, ConfigmenuXineramaSnapping, "Window Snapping"),
         3);

  update();
  setValues();
}


void Configmenu::Xineramamenu::setValues(void) {
  setItemSelected(0, getScreen()->getBlackbox()->doXineramaPlacement());
  setItemSelected(1, getScreen()->getBlackbox()->doXineramaMaximizing());
  setItemSelected(2, getScreen()->getBlackbox()->doXineramaSnapping());
}


void Configmenu::Xineramamenu::reconfigure(void) {
  setValues();
  Basemenu::reconfigure();
}


void Configmenu::Xineramamenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (! item->function())
    return;

  Blackbox *bb = getScreen()->getBlackbox();

  switch (item->function()) {
  case 1: // window placement
    bb->saveXineramaPlacement(! bb->doXineramaPlacement());
    setItemSelected(0, bb->doXineramaPlacement());
    break;

  case 2: // window maximizing
    bb->saveXineramaMaximizing(! bb->doXineramaMaximizing());
    setItemSelected(1, bb->doXineramaMaximizing());
    break;

  case 3: // window snapping
    bb->saveXineramaSnapping(! bb->doXineramaSnapping());
    setItemSelected(2, bb->doXineramaSnapping());
    break;
  }
}
#endif // XINERAMA
