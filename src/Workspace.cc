// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Workspace.cc for Blackbox - an X11 Window manager
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
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H
}

#include <functional>
#include <string>

using std::string;

#include "i18n.hh"
#include "blackbox.hh"
#include "Clientmenu.hh"
#include "Netizen.hh"
#include "Screen.hh"
#include "Toolbar.hh"
#include "Util.hh"
#include "Window.hh"
#include "Workspace.hh"
#include "Windowmenu.hh"


Workspace::Workspace(BScreen *scrn, unsigned int i) {
  screen = scrn;

  cascade_x = cascade_y = 32;

  id = i;

  clientmenu = new Clientmenu(this);

  lastfocus = (BlackboxWindow *) 0;

  setName(screen->getNameOfWorkspace(id));
}


void Workspace::addWindow(BlackboxWindow *w, bool place) {
  assert(w != 0);

  if (place) placeWindow(w);

  w->setWorkspace(id);
  w->setWindowNumber(windowList.size());

  stackingList.push_front(w);
  windowList.push_back(w);

  clientmenu->insert(w->getTitle());
  clientmenu->update();

  screen->updateNetizenWindowAdd(w->getClientWindow(), id);

  raiseWindow(w);
}


unsigned int Workspace::removeWindow(BlackboxWindow *w) {
  assert(w != 0);

  stackingList.remove(w);

  if (w->isFocused() && ! screen->getBlackbox()->doShutdown()) {
    BlackboxWindow *newfocus = 0;
    if (w->isTransient())
      newfocus = w->getTransientFor();
    if (! newfocus && ! stackingList.empty())
      newfocus = stackingList.front();
    if (! newfocus || ! newfocus->setInputFocus())
      screen->getBlackbox()->setFocusedWindow(0);
  }

  if (lastfocus == w)
    lastfocus = (BlackboxWindow *) 0;

  windowList.remove(w);
  clientmenu->remove(w->getWindowNumber());
  clientmenu->update();

  screen->updateNetizenWindowDel(w->getClientWindow());

  BlackboxWindowList::iterator it = windowList.begin();
  const BlackboxWindowList::iterator end = windowList.end();
  unsigned int i = 0;
  for (; it != end; ++it, ++i)
    (*it)->setWindowNumber(i);

  if (i == 0)
    cascade_x = cascade_y = 32;

  return i;
}


void Workspace::showAll(void) {
  std::for_each(stackingList.begin(), stackingList.end(),
                std::mem_fun(&BlackboxWindow::show));
}


void Workspace::hideAll(void) {
  // withdraw in reverse order to minimize the number of Expose events

  BlackboxWindowList lst(stackingList.rbegin(), stackingList.rend());

  BlackboxWindowList::iterator it = lst.begin();
  const BlackboxWindowList::iterator end = lst.end();
  for (; it != end; ++it) {
    BlackboxWindow *bw = *it;
    if (! bw->isStuck())
      bw->withdraw();
  }
}


void Workspace::removeAll(void) {
  while (! windowList.empty())
    windowList.front()->iconify();
}


/*
 * returns the number of transients for win, plus the number of transients
 * associated with each transient of win
 */
static int countTransients(const BlackboxWindow * const win) {
  int ret = win->getTransients().size();
  if (ret > 0) {
    BlackboxWindowList::const_iterator it, end = win->getTransients().end();
    for (it = win->getTransients().begin(); it != end; ++it) {
      ret += countTransients(*it);
    }
  }
  return ret;
}


/*
 * puts the transients of win into the stack. windows are stacked above
 * the window before it in the stackvector being iterated, meaning
 * stack[0] is on bottom, stack[1] is above stack[0], stack[2] is above
 * stack[1], etc...
 */
void Workspace::raiseTransients(const BlackboxWindow * const win,
                                StackVector::iterator &stack) {
  if (win->getTransients().size() == 0) return; // nothing to do

  // put win's transients in the stack
  BlackboxWindowList::const_iterator it, end = win->getTransients().end();
  for (it = win->getTransients().begin(); it != end; ++it) {
    *stack++ = (*it)->getFrameWindow();
    screen->updateNetizenWindowRaise((*it)->getClientWindow());

    if (! (*it)->isIconic()) {
      Workspace *wkspc = screen->getWorkspace((*it)->getWorkspaceNumber());
      wkspc->stackingList.remove((*it));
      wkspc->stackingList.push_front((*it));
    }
  }

  // put transients of win's transients in the stack
  for (it = win->getTransients().begin(); it != end; ++it) {
    raiseTransients(*it, stack);
  }
}


void Workspace::lowerTransients(const BlackboxWindow * const win,
                                StackVector::iterator &stack) {
  if (win->getTransients().size() == 0) return; // nothing to do

  // put transients of win's transients in the stack
  BlackboxWindowList::const_reverse_iterator it,
    end = win->getTransients().rend();
  for (it = win->getTransients().rbegin(); it != end; ++it) {
    lowerTransients(*it, stack);
  }

  // put win's transients in the stack
  for (it = win->getTransients().rbegin(); it != end; ++it) {
    *stack++ = (*it)->getFrameWindow();
    screen->updateNetizenWindowLower((*it)->getClientWindow());

    if (! (*it)->isIconic()) {
      Workspace *wkspc = screen->getWorkspace((*it)->getWorkspaceNumber());
      wkspc->stackingList.remove((*it));
      wkspc->stackingList.push_back((*it));
    }
  }

}


void Workspace::raiseWindow(BlackboxWindow *w) {
  BlackboxWindow *win = w;

  // walk up the transient_for's to the window that is not a transient
  while (win->isTransient()) {
    if (! win->getTransientFor()) break;
    win = win->getTransientFor();
  }

  // get the total window count (win and all transients)
  unsigned int i = 1 + countTransients(win);

  // stack the window with all transients above
  StackVector stack_vector(i);
  StackVector::iterator stack = stack_vector.begin();

  *(stack++) = win->getFrameWindow();
  screen->updateNetizenWindowRaise(win->getClientWindow());
  if (! win->isIconic()) {
    Workspace *wkspc = screen->getWorkspace(win->getWorkspaceNumber());
    wkspc->stackingList.remove(win);
    wkspc->stackingList.push_front(win);
  }

  raiseTransients(win, stack);

  screen->raiseWindows(&stack_vector[0], stack_vector.size());
}


void Workspace::lowerWindow(BlackboxWindow *w) {
  BlackboxWindow *win = w;

  // walk up the transient_for's to the window that is not a transient
  while (win->isTransient()) {
    if (! win->getTransientFor()) break;
    win = win->getTransientFor();
  }

  // get the total window count (win and all transients)
  unsigned int i = 1 + countTransients(win);

  // stack the window with all transients above
  StackVector stack_vector(i);
  StackVector::iterator stack = stack_vector.begin();

  lowerTransients(win, stack);

  *(stack++) = win->getFrameWindow();
  screen->updateNetizenWindowLower(win->getClientWindow());
  if (! win->isIconic()) {
    Workspace *wkspc = screen->getWorkspace(win->getWorkspaceNumber());
    wkspc->stackingList.remove(win);
    wkspc->stackingList.push_back(win);
  }

  XLowerWindow(screen->getBaseDisplay()->getXDisplay(), stack_vector.front());
  XRestackWindows(screen->getBaseDisplay()->getXDisplay(),
                  &stack_vector[0], stack_vector.size());
}


void Workspace::reconfigure(void) {
  clientmenu->reconfigure();
  std::for_each(windowList.begin(), windowList.end(),
                std::mem_fun(&BlackboxWindow::reconfigure));
}


void Workspace::updateFocusModel(void) {
  std::for_each(windowList.begin(), windowList.end(),
                std::mem_fun(&BlackboxWindow::updateFocusModel));
}


BlackboxWindow *Workspace::getWindow(unsigned int index) {
  if (index < windowList.size()) {
    BlackboxWindowList::iterator it = windowList.begin();
    for(; index > 0; --index, ++it); /* increment to index */
    return *it;
  }
  return 0;
}


BlackboxWindow*
Workspace::getNextWindowInList(BlackboxWindow *w) {
  BlackboxWindowList::iterator it = std::find(windowList.begin(),
                                              windowList.end(),
                                              w);
  assert(it != windowList.end());   // window must be in list
  ++it;                             // next window
  if (it == windowList.end())
    return windowList.front();      // if we walked off the end, wrap around

  return *it;
}


BlackboxWindow* Workspace::getPrevWindowInList(BlackboxWindow *w) {
  BlackboxWindowList::iterator it = std::find(windowList.begin(),
                                              windowList.end(),
                                              w);
  assert(it != windowList.end()); // window must be in list
  if (it == windowList.begin())
    return windowList.back();     // if we walked of the front, wrap around

  return *(--it);
}


BlackboxWindow* Workspace::getTopWindowOnStack(void) const {
  return stackingList.front();
}


void Workspace::sendWindowList(Netizen &n) {
  BlackboxWindowList::iterator it = windowList.begin(),
    end = windowList.end();
  for(; it != end; ++it)
    n.sendWindowAdd((*it)->getClientWindow(), getID());
}


unsigned int Workspace::getCount(void) const {
  return windowList.size();
}


bool Workspace::isCurrent(void) const {
  return (id == screen->getCurrentWorkspaceID());
}


bool Workspace::isLastWindow(const BlackboxWindow* const w) const {
  return (w == windowList.back());
}

void Workspace::setCurrent(void) {
  screen->changeWorkspaceID(id);
}


void Workspace::setName(const string& new_name) {
  if (! new_name.empty()) {
    name = new_name;
  } else {
    string tmp =i18n(WorkspaceSet, WorkspaceDefaultNameFormat, "Workspace %d");
    assert(tmp.length() < 32);
    char default_name[32];
    sprintf(default_name, tmp.c_str(), id + 1);
    name = default_name;
  }

  clientmenu->setLabel(name);
  clientmenu->update();
}


/*
 * Calculate free space available for window placement.
 */
typedef std::vector<Rect> rectList;

static rectList calcSpace(const Rect &win, const rectList &spaces) {
  Rect isect, extra;
  rectList result;
  rectList::const_iterator siter, end = spaces.end();
  for (siter = spaces.begin(); siter != end; ++siter) {
    const Rect &curr = *siter;

    if(! win.intersects(curr)) {
      result.push_back(curr);
      continue;
    }

    /* Use an intersection of win and curr to determine the space around
     * curr that we can use.
     *
     * NOTE: the spaces calculated can overlap.
     */
    isect = curr & win;

    // left
    extra.setCoords(curr.left(), curr.top(),
                    isect.left() - 1, curr.bottom());
    if (extra.valid()) result.push_back(extra);

    // top
    extra.setCoords(curr.left(), curr.top(),
                    curr.right(), isect.top() - 1);
    if (extra.valid()) result.push_back(extra);

    // right
    extra.setCoords(isect.right() + 1, curr.top(),
                    curr.right(), curr.bottom());
    if (extra.valid()) result.push_back(extra);

    // bottom
    extra.setCoords(curr.left(), isect.bottom() + 1,
                    curr.right(), curr.bottom());
    if (extra.valid()) result.push_back(extra);
  }
  return result;
}


static bool rowRLBT(const Rect &first, const Rect &second) {
  if (first.bottom() == second.bottom())
    return first.right() > second.right();
  return first.bottom() > second.bottom();
}

static bool rowRLTB(const Rect &first, const Rect &second) {
  if (first.y() == second.y())
    return first.right() > second.right();
  return first.y() < second.y();
}

static bool rowLRBT(const Rect &first, const Rect &second) {
  if (first.bottom() == second.bottom())
    return first.x() < second.x();
  return first.bottom() > second.bottom();
}

static bool rowLRTB(const Rect &first, const Rect &second) {
  if (first.y() == second.y())
    return first.x() < second.x();
  return first.y() < second.y();
}

static bool colLRTB(const Rect &first, const Rect &second) {
  if (first.x() == second.x())
    return first.y() < second.y();
  return first.x() < second.x();
}

static bool colLRBT(const Rect &first, const Rect &second) {
  if (first.x() == second.x())
    return first.bottom() > second.bottom();
  return first.x() < second.x();
}

static bool colRLTB(const Rect &first, const Rect &second) {
  if (first.right() == second.right())
    return first.y() < second.y();
  return first.right() > second.right();
}

static bool colRLBT(const Rect &first, const Rect &second) {
  if (first.right() == second.right())
    return first.bottom() > second.bottom();
  return first.right() > second.right();
}


bool Workspace::smartPlacement(Rect& win, const Rect& availableArea) {
  rectList spaces;
  spaces.push_back(availableArea); //initially the entire screen is free

  //Find Free Spaces
  BlackboxWindowList::iterator wit = windowList.begin(),
                               end = windowList.end();
  Rect tmp;
  for (; wit != end; ++wit) {
    const BlackboxWindow* const curr = *wit;
    tmp.setRect(curr->frameRect().x(), curr->frameRect().y(),
                curr->frameRect().width() + screen->getBorderWidth(),
                curr->frameRect().height() + screen->getBorderWidth());

    spaces = calcSpace(tmp, spaces);
  }

  if (screen->getPlacementPolicy() == BScreen::RowSmartPlacement) {
    if(screen->getRowPlacementDirection() == BScreen::LeftRight) {
      if(screen->getColPlacementDirection() == BScreen::TopBottom)
        sort(spaces.begin(), spaces.end(), rowLRTB);
      else
        sort(spaces.begin(), spaces.end(), rowLRBT);
    } else {
      if(screen->getColPlacementDirection() == BScreen::TopBottom)
        sort(spaces.begin(), spaces.end(), rowRLTB);
      else
        sort(spaces.begin(), spaces.end(), rowRLBT);
    }
  } else {
    if(screen->getColPlacementDirection() == BScreen::TopBottom) {
      if(screen->getRowPlacementDirection() == BScreen::LeftRight)
        sort(spaces.begin(), spaces.end(), colLRTB);
      else
        sort(spaces.begin(), spaces.end(), colRLTB);
    } else {
      if(screen->getRowPlacementDirection() == BScreen::LeftRight)
        sort(spaces.begin(), spaces.end(), colLRBT);
      else
        sort(spaces.begin(), spaces.end(), colRLBT);
    }
  }

  rectList::const_iterator sit = spaces.begin(), spaces_end = spaces.end();
  for(; sit != spaces_end; ++sit) {
    if (sit->width() >= win.width() && sit->height() >= win.height())
      break;
  }

  if (sit == spaces_end)
    return False;

  //set new position based on the empty space found
  const Rect& where = *sit;
  win.setX(where.x());
  win.setY(where.y());

  // adjust the location() based on left/right and top/bottom placement
  if (screen->getPlacementPolicy() == BScreen::RowSmartPlacement) {
    if (screen->getRowPlacementDirection() == BScreen::RightLeft)
      win.setX(where.right() - win.width());
    if (screen->getColPlacementDirection() == BScreen::BottomTop)
      win.setY(where.bottom() - win.height());
  } else {
    if (screen->getColPlacementDirection() == BScreen::BottomTop)
      win.setY(win.y() + where.height() - win.height());
    if (screen->getRowPlacementDirection() == BScreen::RightLeft)
      win.setX(win.x() + where.width() - win.width());
  }
  return True;
}


bool Workspace::cascadePlacement(Rect &win, const Rect &availableArea) {
  if ((cascade_x > static_cast<signed>(availableArea.width() / 2)) ||
      (cascade_y > static_cast<signed>(availableArea.height() / 2)))
    cascade_x = cascade_y = 32;

  if (cascade_x == 32) {
    cascade_x += availableArea.x();
    cascade_y += availableArea.y();
  }

  win.setPos(cascade_x, cascade_y);

  return True;
}


void Workspace::placeWindow(BlackboxWindow *win) {
  Rect availableArea(screen->availableArea()),
    new_win(availableArea.x(), availableArea.y(),
            win->frameRect().width(), win->frameRect().height());
  bool placed = False;

  switch (screen->getPlacementPolicy()) {
  case BScreen::RowSmartPlacement:
  case BScreen::ColSmartPlacement:
    placed = smartPlacement(new_win, availableArea);
    break;
  default:
    break; // handled below
  } // switch

  if (placed == False) {
    cascadePlacement(new_win, availableArea);
    cascade_x += win->getTitleHeight() + (screen->getBorderWidth() * 2);
    cascade_y += win->getTitleHeight() + (screen->getBorderWidth() * 2);
  }

  if (new_win.right() > availableArea.right())
    new_win.setX(availableArea.left());
  if (new_win.bottom() > availableArea.bottom())
    new_win.setY(availableArea.top());
  win->configure(new_win.x(), new_win.y(), new_win.width(), new_win.height());
}
