// Windowmenu.cc for Openbox
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

// stupid macros needed to access some functions in version 2 of the GNU C
// library
#ifndef   _GNU_SOURCE
#define   _GNU_SOURCE
#endif // _GNU_SOURCE

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "i18n.h"
#include "openbox.h"
#include "Screen.h"
#include "Window.h"
#include "Windowmenu.h"
#include "Workspace.h"

#ifdef    STDC_HEADERS
#  include <string.h>
#endif // STDC_HEADERS


Windowmenu::Windowmenu(OpenboxWindow &win) : Basemenu(*win.getScreen()),
  window(win), screen(*win.getScreen())
{

  setTitleVisibility(False);
  setMovable(False);
  setInternalMenu();

  sendToMenu = new SendtoWorkspacemenu(*this);
  insert(i18n->getMessage(WindowmenuSet, WindowmenuSendTo, "Send To ..."),
	 sendToMenu);
  insert(i18n->getMessage(WindowmenuSet, WindowmenuShade, "Shade"),
	 BScreen::WindowShade);
  insert(i18n->getMessage(WindowmenuSet, WindowmenuIconify, "Iconify"),
	 BScreen::WindowIconify);
  insert(i18n->getMessage(WindowmenuSet, WindowmenuMaximize, "Maximize"),
	 BScreen::WindowMaximize);
  insert(i18n->getMessage(WindowmenuSet, WindowmenuRaise,"Raise"),
	 BScreen::WindowRaise);
  insert(i18n->getMessage(WindowmenuSet, WindowmenuLower, "Lower"),
	 BScreen::WindowLower);
  insert(i18n->getMessage(WindowmenuSet, WindowmenuStick, "Stick"),
	 BScreen::WindowStick);
  insert(i18n->getMessage(WindowmenuSet, WindowmenuKillClient, "Kill Client"),
	 BScreen::WindowKill);
  insert(i18n->getMessage(WindowmenuSet, WindowmenuClose, "Close"),
	 BScreen::WindowClose);

  update();

  setItemEnabled(1, window.hasTitlebar());
  setItemEnabled(2, window.isIconifiable());
  setItemEnabled(3, window.isMaximizable());
  setItemEnabled(8, window.isClosable());
}


Windowmenu::~Windowmenu(void) {
  delete sendToMenu;
}


void Windowmenu::show(void) {
  if (isItemEnabled(1)) setItemSelected(1, window.isShaded());
  if (isItemEnabled(3)) setItemSelected(3, window.isMaximized());
  if (isItemEnabled(6)) setItemSelected(6, window.isStuck());

  Basemenu::show();
}


void Windowmenu::itemSelected(int button, int index) {
  BasemenuItem *item = find(index);

  /* Added by Scott Moynes, April 8, 2002
     Ignore the middle button for every item except the maximize
     button in the window menu. Maximize needs it for
     horizontal/vertical maximize, however, for the others it is
     inconsistent with the rest of the window behaviour.
     */
  if(button != 2) {
    hide();
    switch (item->function()) {
    case BScreen::WindowShade:
      window.shade();
      break;

    case BScreen::WindowIconify:
      window.iconify();
      break;

    case BScreen::WindowMaximize:
      window.maximize((unsigned int) button);
      break;

    case BScreen::WindowClose:
      window.close();
      break;

    case BScreen::WindowRaise:
      screen.getWorkspace(window.getWorkspaceNumber())->raiseWindow(&window);
      break;

    case BScreen::WindowLower:
      screen.getWorkspace(window.getWorkspaceNumber())->lowerWindow(&window);
      break;

    case BScreen::WindowStick:
      window.stick();
      break;

    case BScreen::WindowKill:
      XKillClient(screen.getBaseDisplay().getXDisplay(),
                  window.getClientWindow());
      break;
    }
  } else if (item->function() == BScreen::WindowMaximize) {
    hide();
    window.maximize((unsigned int) button);
  }
}


void Windowmenu::reconfigure(void) {
  setItemEnabled(1, window.hasTitlebar());
  setItemEnabled(2, window.isIconifiable());
  setItemEnabled(3, window.isMaximizable());
  setItemEnabled(8, window.isClosable());

  sendToMenu->reconfigure();

  Basemenu::reconfigure();
}


Windowmenu::SendtoWorkspacemenu::SendtoWorkspacemenu(Windowmenu &w)
  : Basemenu(w.screen), windowmenu(w) {
  setTitleVisibility(False);
  setMovable(False);
  setInternalMenu();
  update();
}


void Windowmenu::SendtoWorkspacemenu::itemSelected(int button, int index) {
  if (button > 2) return;

  if (index <= windowmenu.screen.getWorkspaceCount()) {
    if (index == windowmenu.screen.getCurrentWorkspaceID()) return;
    if (windowmenu.window.isStuck()) windowmenu.window.stick();

    if (button == 1) windowmenu.window.withdraw();
    windowmenu.screen.reassociateWindow(&(windowmenu.window), index, True);
    if (button == 2) windowmenu.screen.changeWorkspaceID(index);
  }
  hide();
}


void Windowmenu::SendtoWorkspacemenu::update(void) {
  int i, r = getCount();

  if (r != 0)
    for (i = 0; i < r; ++i)
      remove(0);

  for (i = 0; i < windowmenu.screen.getWorkspaceCount(); ++i)
    insert(windowmenu.screen.getWorkspace(i)->getName());

  Basemenu::update();
}


void Windowmenu::SendtoWorkspacemenu::show(void) {
  update();

  Basemenu::show();
}
