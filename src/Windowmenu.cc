// -*- mode: C++; indent-tabs-mode: nil; -*-
// Windowmenu.cc for Blackbox - an X11 Window manager
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
#ifdef HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H
}

#include "i18n.hh"
#include "blackbox.hh"
#include "Screen.hh"
#include "Window.hh"
#include "Windowmenu.hh"
#include "Workspace.hh"


Windowmenu::Windowmenu(BlackboxWindow *win) : Basemenu(win->getScreen()) {
  window = win;

  setTitleVisibility(False);
  setMovable(False);
  setInternalMenu();

  sendToMenu = new SendtoWorkspacemenu(this);
  insert(i18n(WindowmenuSet, WindowmenuSendTo, "Send To ..."),
         sendToMenu);
  insert(i18n(WindowmenuSet, WindowmenuShade, "Shade"),
         BScreen::WindowShade);
  insert(i18n(WindowmenuSet, WindowmenuIconify, "Iconify"),
         BScreen::WindowIconify);
  insert(i18n(WindowmenuSet, WindowmenuMaximize, "Maximize"),
         BScreen::WindowMaximize);
  insert(i18n(WindowmenuSet, WindowmenuRaise,"Raise"),
         BScreen::WindowRaise);
  insert(i18n(WindowmenuSet, WindowmenuLower, "Lower"),
         BScreen::WindowLower);
  insert(i18n(WindowmenuSet, WindowmenuStick, "Stick"),
         BScreen::WindowStick);
  insert(i18n(WindowmenuSet, WindowmenuKillClient, "Kill Client"),
         BScreen::WindowKill);
  insert(i18n(WindowmenuSet, WindowmenuClose, "Close"),
         BScreen::WindowClose);

  update();

  setItemEnabled(1, window->hasTitlebar());
  setItemEnabled(2, window->isIconifiable());
  setItemEnabled(3, window->isMaximizable());
  setItemEnabled(8, window->isClosable());
}


Windowmenu::~Windowmenu(void) {
  delete sendToMenu;
}


void Windowmenu::show(void) {
  if (isItemEnabled(1)) setItemSelected(1, window->isShaded());
  if (isItemEnabled(3)) setItemSelected(3, window->isMaximized());
  if (isItemEnabled(6)) setItemSelected(6, window->isStuck());

  Basemenu::show();
}


void Windowmenu::itemSelected(int button, unsigned int index) {
  BasemenuItem *item = find(index);

  hide();
  switch (item->function()) {
  case BScreen::WindowShade:
    window->shade();
    break;

  case BScreen::WindowIconify:
    window->iconify();
    break;

  case BScreen::WindowMaximize:
    window->maximize(button);
    break;

  case BScreen::WindowClose:
    window->close();
    break;

  case BScreen::WindowRaise: {
    Workspace *wkspc = getScreen()->getWorkspace(window->getWorkspaceNumber());
    wkspc->raiseWindow(window);
  }
    break;

  case BScreen::WindowLower: {
    Workspace *wkspc = getScreen()->getWorkspace(window->getWorkspaceNumber());
    wkspc->lowerWindow(window);
  }
    break;

  case BScreen::WindowStick:
    window->stick();
    break;

  case BScreen::WindowKill:
    XKillClient(getScreen()->getBaseDisplay()->getXDisplay(),
                window->getClientWindow());
    break;
  }
}


void Windowmenu::reconfigure(void) {
  setItemEnabled(1, window->hasTitlebar());
  setItemEnabled(2, window->isIconifiable());
  setItemEnabled(3, window->isMaximizable());
  setItemEnabled(8, window->isClosable());

  sendToMenu->reconfigure();

  Basemenu::reconfigure();
}


Windowmenu::SendtoWorkspacemenu::SendtoWorkspacemenu(Windowmenu *w)
  : Basemenu(w->getScreen()) {

  window = w->window;

  setTitleVisibility(False);
  setMovable(False);
  setInternalMenu();
  update();
}


void Windowmenu::SendtoWorkspacemenu::itemSelected(int button,
                                                   unsigned int index) {
  if (button > 2) return;

  if (index <= getScreen()->getWorkspaceCount()) {
    if (index == getScreen()->getCurrentWorkspaceID()) return;
    if (window->isStuck()) window->stick();

    if (button == 1) window->withdraw();
    getScreen()->reassociateWindow(window, index, True);
    if (button == 2) getScreen()->changeWorkspaceID(index);
  }
  hide();
}


void Windowmenu::SendtoWorkspacemenu::update(void) {
  unsigned int i, r = getCount(),
    workspace_count = getScreen()->getWorkspaceCount();
  if (r > workspace_count) {
    for (i = r; i < workspace_count; ++i)
      remove(0);
    r = getCount();
  }

  for (i = 0; i < workspace_count; ++i) {
    if (r < workspace_count) {
      insert(getScreen()->getWorkspace(i)->getName());
      ++r;
    } else {
      changeItemLabel(i, getScreen()->getWorkspace(i)->getName());
    }
  }

  Basemenu::update();
}


void Windowmenu::SendtoWorkspacemenu::show(void) {
  update();

  Basemenu::show();
}
