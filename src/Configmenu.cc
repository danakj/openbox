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
  update();

  setItemSelected(2, getScreen()->getImageControl()->doDither());
  setItemSelected(3, getScreen()->doOpaqueMove());
  setItemSelected(4, getScreen()->doFullMax());
  setItemSelected(5, getScreen()->doFocusNew());
  setItemSelected(6, getScreen()->doFocusLast());
}

Configmenu::~Configmenu(void) {
  delete focusmenu;
  delete placementmenu;
}

void Configmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (!item->function())
    return;

  switch(item->function()) {
  case 1: { // dither
    getScreen()->getImageControl()->
      setDither((! getScreen()->getImageControl()->doDither()));

    setItemSelected(index, getScreen()->getImageControl()->doDither());

    break;
  }

  case 2: { // opaque move
    getScreen()->saveOpaqueMove((! getScreen()->doOpaqueMove()));

    setItemSelected(index, getScreen()->doOpaqueMove());

    break;
  }

  case 3: { // full maximization
    getScreen()->saveFullMax((! getScreen()->doFullMax()));

    setItemSelected(index, getScreen()->doFullMax());

    break;
  }
  case 4: { // focus new windows
    getScreen()->saveFocusNew((! getScreen()->doFocusNew()));

    setItemSelected(index, getScreen()->doFocusNew());
    break;
  }

  case 5: { // focus last window on workspace
    getScreen()->saveFocusLast((! getScreen()->doFocusLast()));
    setItemSelected(index, getScreen()->doFocusLast());
    break;
  }
  } // switch
}


void Configmenu::reconfigure(void) {
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

  setItemSelected(0, (! getScreen()->isSloppyFocus()));
  setItemSelected(1, getScreen()->isSloppyFocus());
  setItemEnabled(2, getScreen()->isSloppyFocus());
  setItemSelected(2, getScreen()->doAutoRaise());
  setItemEnabled(3, getScreen()->isSloppyFocus());
  setItemSelected(3, getScreen()->doClickRaise());
}


void Configmenu::Focusmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (!item->function())
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
    getScreen()->updateFocusModel();
    break;
  }

  setItemSelected(0, (! getScreen()->isSloppyFocus()));
  setItemSelected(1, getScreen()->isSloppyFocus());
  setItemEnabled(2, getScreen()->isSloppyFocus());
  setItemSelected(2, getScreen()->doAutoRaise());
  setItemEnabled(3, getScreen()->isSloppyFocus());
  setItemSelected(3, getScreen()->doClickRaise());
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
  insert(i18n(ConfigmenuSet, ConfigmenuLeftRight, "Left to Right"),
         BScreen::LeftRight);
  insert(i18n(ConfigmenuSet, ConfigmenuRightLeft, "Right to Left"),
         BScreen::RightLeft);
  insert(i18n(ConfigmenuSet, ConfigmenuTopBottom, "Top to Bottom"),
         BScreen::TopBottom);
  insert(i18n(ConfigmenuSet, ConfigmenuBottomTop, "Bottom to Top"),
         BScreen::BottomTop);
  update();

  switch (getScreen()->getPlacementPolicy()) {
  case BScreen::RowSmartPlacement:
    setItemSelected(0, True);
    break;

  case BScreen::ColSmartPlacement:
    setItemSelected(1, True);
    break;

  case BScreen::CascadePlacement:
    setItemSelected(2, True);
    break;
  }

  bool rl = (getScreen()->getRowPlacementDirection() ==
             BScreen::LeftRight),
    tb = (getScreen()->getColPlacementDirection() ==
          BScreen::TopBottom);

  setItemSelected(3, rl);
  setItemSelected(4, ! rl);

  setItemSelected(5, tb);
  setItemSelected(6, ! tb);
}


void Configmenu::Placementmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (!item->function())
    return;

  switch (item->function()) {
  case BScreen::RowSmartPlacement:
    getScreen()->savePlacementPolicy(item->function());

    setItemSelected(0, True);
    setItemSelected(1, False);
    setItemSelected(2, False);

    break;

  case BScreen::ColSmartPlacement:
    getScreen()->savePlacementPolicy(item->function());

    setItemSelected(0, False);
    setItemSelected(1, True);
    setItemSelected(2, False);

    break;

  case BScreen::CascadePlacement:
    getScreen()->savePlacementPolicy(item->function());

    setItemSelected(0, False);
    setItemSelected(1, False);
    setItemSelected(2, True);

    break;

  case BScreen::LeftRight:
    getScreen()->saveRowPlacementDirection(BScreen::LeftRight);

    setItemSelected(3, True);
    setItemSelected(4, False);

    break;

  case BScreen::RightLeft:
    getScreen()->saveRowPlacementDirection(BScreen::RightLeft);

    setItemSelected(3, False);
    setItemSelected(4, True);

    break;

  case BScreen::TopBottom:
    getScreen()->saveColPlacementDirection(BScreen::TopBottom);

    setItemSelected(5, True);
    setItemSelected(6, False);

    break;

  case BScreen::BottomTop:
    getScreen()->saveColPlacementDirection(BScreen::BottomTop);

    setItemSelected(5, False);
    setItemSelected(6, True);

    break;
  }
}
