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

  insert(i18n(ConfigmenuSet, ConfigmenuFocusModel,
              "Focus Model"), focusmenu);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowPlacement,
              "Window Placement"), placementmenu);
  insert(i18n(ConfigmenuSet, ConfigmenuImageDithering,
              "Image Dithering"), 1);
  insert(i18n(ConfigmenuSet, ConfigmenuOpaqueMove,
              "Opaque Window Moving"), 2);
  insert(i18n(ConfigmenuSet, ConfigmenuFullMax,
              "Full Maximization"), 3);
  insert(i18n(ConfigmenuSet, ConfigmenuFocusNew,
              "Focus New Windows"), 4);
  insert(i18n(ConfigmenuSet, ConfigmenuFocusLast,
              "Focus Last Window on Workspace"), 5);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowToWindowSnap,
              "Window-To-Window Snapping"), 6);
  insert(i18n(ConfigmenuSet, ConfigmenuWindowCornerSnap,
              "Window Corner Snapping"), 7);
  insert(i18n(ConfigmenuSet, ConfigmenuHideToolbar,
              "Hide Toolbar"), 8);
  update();
  setValues();
}


void Configmenu::setValues(void) {
  setItemSelected(2, getScreen()->doImageDither());
  setItemSelected(3, getScreen()->doOpaqueMove());
  setItemSelected(4, getScreen()->doFullMax());
  setItemSelected(5, getScreen()->doFocusNew());
  setItemSelected(6, getScreen()->doFocusLast());
  setItemSelected(7, getScreen()->getWindowToWindowSnap());

  setItemSelected(8, getScreen()->getWindowCornerSnap());
  setItemEnabled(8, getScreen()->getWindowToWindowSnap());
  
  setItemSelected(9, getScreen()->doHideToolbar());
}


Configmenu::~Configmenu(void) {
  delete focusmenu;
  delete placementmenu;
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

  case 3: // full maximization
    getScreen()->saveFullMax(! getScreen()->doFullMax());
    setItemSelected(index, getScreen()->doFullMax());
    break;

  case 4: // focus new windows
    getScreen()->saveFocusNew(! getScreen()->doFocusNew());
    setItemSelected(index, getScreen()->doFocusNew());
    break;

  case 5: // focus last window on workspace
    getScreen()->saveFocusLast(! getScreen()->doFocusLast());
    setItemSelected(index, getScreen()->doFocusLast());
    break;

  case 6: // window-to-window snapping
    getScreen()->saveWindowToWindowSnap(! getScreen()->getWindowToWindowSnap());
    setItemSelected(index, getScreen()->getWindowToWindowSnap());
    setItemEnabled(index + 1, getScreen()->getWindowToWindowSnap());
    break;

  case 7: // window corner snapping
    getScreen()->saveWindowCornerSnap(! getScreen()->getWindowCornerSnap());
    setItemSelected(index, getScreen()->getWindowCornerSnap());
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
  insert(i18n(ConfigmenuSet, ConfigmenuIgnoreShaded, "Ignore shaded windows"),
         BScreen::IgnoreShaded);
  insert(i18n(ConfigmenuSet, ConfigmenuIgnoreMax,
              "Ignore full-maximized windows"),
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
       e = placement != BScreen::UnderMousePlacement;

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

    setItemSelected(0, true);
    setItemSelected(1, false);
    setItemSelected(2, false);
    setItemSelected(3, false);
    setItemSelected(4, false);
    setItemEnabled(5, true);
    setItemEnabled(6, true);
    setItemEnabled(7, true);
    setItemEnabled(8, true);

    break;

  case BScreen::ColSmartPlacement:
    getScreen()->savePlacementPolicy(item->function());

    setItemSelected(0, false);
    setItemSelected(1, true);
    setItemSelected(2, false);
    setItemSelected(3, false);
    setItemSelected(4, false);
    setItemEnabled(5, true);
    setItemEnabled(6, true);
    setItemEnabled(7, true);
    setItemEnabled(8, true);

    break;

  case BScreen::CascadePlacement:
    getScreen()->savePlacementPolicy(item->function());

    setItemSelected(0, false);
    setItemSelected(1, false);
    setItemSelected(2, true);
    setItemSelected(3, false);
    setItemSelected(4, false);
    setItemEnabled(5, true);
    setItemEnabled(6, true);
    setItemEnabled(7, true);
    setItemEnabled(8, true);

    break;

  case BScreen::UnderMousePlacement:
    getScreen()->savePlacementPolicy(item->function());

    setItemSelected(0, false);
    setItemSelected(1, false);
    setItemSelected(2, false);
    setItemSelected(3, true);
    setItemSelected(4, false);
    setItemEnabled(5, false);
    setItemEnabled(6, false);
    setItemEnabled(7, false);
    setItemEnabled(8, false);

    break;

  case BScreen::ClickMousePlacement:
    getScreen()->savePlacementPolicy(item->function());

    setItemSelected(0, false);
    setItemSelected(1, false);
    setItemSelected(2, false);
    setItemSelected(3, false);
    setItemSelected(4, true);
    setItemEnabled(5, false);
    setItemEnabled(6, false);
    setItemEnabled(7, false);
    setItemEnabled(8, false);

    break;

  case BScreen::LeftRight:
    getScreen()->saveRowPlacementDirection(BScreen::LeftRight);

    setItemSelected(5, true);
    setItemSelected(6, false);

    break;

  case BScreen::RightLeft:
    getScreen()->saveRowPlacementDirection(BScreen::RightLeft);

    setItemSelected(5, false);
    setItemSelected(6, true);

    break;

  case BScreen::TopBottom:
    getScreen()->saveColPlacementDirection(BScreen::TopBottom);

    setItemSelected(7, true);
    setItemSelected(8, false);

    break;

  case BScreen::BottomTop:
    getScreen()->saveColPlacementDirection(BScreen::BottomTop);

    setItemSelected(7, false);
    setItemSelected(8, true);

    break;
  
  case BScreen::IgnoreShaded:
    getScreen()->savePlaceIgnoreShaded(! getScreen()->getPlaceIgnoreShaded());

    setItemSelected(9, getScreen()->getPlaceIgnoreShaded());

    break;

  case BScreen::IgnoreMaximized:
    getScreen()->
      savePlaceIgnoreMaximized(! getScreen()->getPlaceIgnoreMaximized());

    setItemSelected(10, getScreen()->getPlaceIgnoreMaximized());

    break;
  }
}
