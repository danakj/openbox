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

#include <assert.h>

#include <functional>
#include <string>

using std::string;

#include "i18n.hh"
#include "blackbox.hh"
#include "Clientmenu.hh"
#include "Font.hh"
#include "Netizen.hh"
#include "Screen.hh"
#include "Toolbar.hh"
#include "Util.hh"
#include "Window.hh"
#include "Workspace.hh"
#include "Windowmenu.hh"
#include "XAtom.hh"


Workspace::Workspace(BScreen *scrn, unsigned int i) {
  screen = scrn;
  xatom = screen->getBlackbox()->getXAtom();

  cascade_x = cascade_y = 0;
#ifdef    XINERAMA
  cascade_region = 0;
#endif // XINERAMA

  id = i;

  clientmenu = new Clientmenu(this);

  lastfocus = (BlackboxWindow *) 0;

  readName();
}


void Workspace::addWindow(BlackboxWindow *w, bool place, bool sticky) {
  assert(w != 0);

  if (place) placeWindow(w);

  stackingList.push_front(w);
    
  // if the window is sticky, then it needs to be added on all other
  // workspaces too!
  if (! sticky && w->isStuck()) {
    for (unsigned int i = 0; i < screen->getWorkspaceCount(); ++i)
      if (i != id)
        screen->getWorkspace(i)->addWindow(w, place, True);
  }

  if (w->isNormal()) {
    if (! sticky) {
      w->setWorkspace(id);
      w->setWindowNumber(windowList.size());
    }

    windowList.push_back(w);

    clientmenu->insert(w->getTitle());
    clientmenu->update();

    if (! sticky)
      screen->updateNetizenWindowAdd(w->getClientWindow(), id);

    if (screen->doFocusNew() || (w->isTransient() && w->getTransientFor() &&
                                 w->getTransientFor()->isFocused())) {
      if (id == screen->getCurrentWorkspaceID())
        w->setInputFocus();
      else {
        /*
           not on the focused workspace, so the window is not going to get focus
           but if the user wants new windows focused, then it should get focus
           when this workspace does become focused.
        */
        lastfocus = w;
      }
    }
  }

  if (! w->isDesktop())
    raiseWindow(w);
  else
    lowerWindow(w);
}


void Workspace::removeWindow(BlackboxWindow *w, bool sticky) {
  assert(w != 0);

  stackingList.remove(w);

  // pass focus to the next appropriate window
  if ((w->isFocused() || w == lastfocus) &&
      ! screen->getBlackbox()->doShutdown()) {
    focusFallback(w);
  }
    
  // if the window is sticky, then it needs to be removed on all other
  // workspaces too!
  if (! sticky && w->isStuck()) {
    for (unsigned int i = 0; i < screen->getWorkspaceCount(); ++i)
      if (i != id)
        screen->getWorkspace(i)->removeWindow(w, True);
  }

  if (! w->isNormal()) return;

  BlackboxWindowList::iterator it, end = windowList.end();
  int i;
  for (i = 0, it = windowList.begin(); it != end; ++it, ++i)
    if (*it == w)
      break;
  assert(it != end);
  
  windowList.erase(it);
  clientmenu->remove(i);
  clientmenu->update();

  if (! sticky) {
    screen->updateNetizenWindowDel(w->getClientWindow());

    BlackboxWindowList::iterator it = windowList.begin();
    const BlackboxWindowList::iterator end = windowList.end();
    unsigned int i = 0;
    for (; it != end; ++it, ++i)
      (*it)->setWindowNumber(i);
  }

  if (i == 0) {
    cascade_x = cascade_y = 0;
#ifdef    XINERAMA
    cascade_region = 0;
#endif // XINERAMA
  }
}


void Workspace::focusFallback(const BlackboxWindow *old_window) {
  BlackboxWindow *newfocus = 0;

  if (id == screen->getCurrentWorkspaceID()) {
    // The window is on the visible workspace.

    // if it's a transient, then try to focus its parent
    if (old_window && old_window->isTransient()) {
      newfocus = old_window->getTransientFor();

      if (! newfocus ||
          newfocus->isIconic() ||                  // do not focus icons
          newfocus->getWorkspaceNumber() != id ||  // or other workspaces
          ! newfocus->setInputFocus())
        newfocus = 0;
    }

    if (! newfocus) {
      BlackboxWindowList::iterator it = stackingList.begin(),
                                  end = stackingList.end();
      for (; it != end; ++it) {
        BlackboxWindow *tmp = *it;
        if (tmp && tmp->isNormal() && tmp->setInputFocus()) {
          // we found our new focus target
          newfocus = tmp;
          break;
        }
      }
    }

    screen->getBlackbox()->setFocusedWindow(newfocus);
  } else {
    // The window is not on the visible workspace.

    if (old_window && lastfocus == old_window) {
      // The window was the last-focus target, so we need to replace it.
      BlackboxWindow *win = (BlackboxWindow*) 0;
      if (! stackingList.empty())
        win = stackingList.front();
      setLastFocusedWindow(win);
    }
  }
}


void Workspace::setFocused(const BlackboxWindow *w, bool focused) {
  BlackboxWindowList::iterator it, end = windowList.end();
  int i;
  for (i = 0, it = windowList.begin(); it != end; ++it, ++i)
    if (*it == w)
      break;
  // if its == end, then a window thats not in the windowList
  // got focused, such as a !isNormal() window.
  if (it != end)
    clientmenu->setItemSelected(i, focused);
}


void Workspace::showAll(void) {
  BlackboxWindowList::iterator it = stackingList.begin();
  const BlackboxWindowList::iterator end = stackingList.end();
  for (; it != end; ++it) {
    BlackboxWindow *bw = *it;
    bw->show();
  }
}


void Workspace::hideAll(void) {
  // withdraw in reverse order to minimize the number of Expose events
  BlackboxWindowList::reverse_iterator it = stackingList.rbegin();
  const BlackboxWindowList::reverse_iterator end = stackingList.rend();
  while (it != end) {
    BlackboxWindow *bw = *it;
    ++it; // withdraw removes the current item from the list so we need the next
          // iterator before that happens
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

  if (win->isDesktop()) return;

  // walk up the transient_for's to the window that is not a transient
  while (win->isTransient() && ! win->isDesktop()) {
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
  if (! (win->isIconic() || win->isDesktop())) {
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
  while (win->isTransient() && ! win->isDesktop()) {
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
  if (! (win->isIconic() || win->isDesktop())) {
    Workspace *wkspc = screen->getWorkspace(win->getWorkspaceNumber());
    wkspc->stackingList.remove(win);
    wkspc->stackingList.push_back(win);
  }

  screen->lowerWindows(&stack_vector[0], stack_vector.size());
}


void Workspace::reconfigure(void) {
  clientmenu->reconfigure();
  std::for_each(windowList.begin(), windowList.end(),
                std::mem_fun(&BlackboxWindow::reconfigure));
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


void Workspace::appendStackOrder(BlackboxWindowList &stack_order) const {
  BlackboxWindowList::const_reverse_iterator it = stackingList.rbegin();
  const BlackboxWindowList::const_reverse_iterator end = stackingList.rend();
  for (; it != end; ++it)
    if ((*it)->isNormal())
      stack_order.push_back(*it);
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


void Workspace::readName(void) {
  XAtom::StringVect namesList;
  unsigned long numnames = id + 1;
    
  // attempt to get from the _NET_WM_DESKTOP_NAMES property
  if (xatom->getValue(screen->getRootWindow(), XAtom::net_desktop_names,
                      XAtom::utf8, numnames, namesList) &&
      namesList.size() > id) {
    name = namesList[id];
  
    clientmenu->setLabel(name);
    clientmenu->update();
  } else {
    /*
       Use a default name. This doesn't actually change the class. That will
       happen after the setName changes the root property, and that change
       makes its way back to this function.
    */
    string tmp =i18n(WorkspaceSet, WorkspaceDefaultNameFormat,
                     "Workspace %d");
    assert(tmp.length() < 32);
    char default_name[32];
    sprintf(default_name, tmp.c_str(), id + 1);
    
    setName(default_name);  // save this into the _NET_WM_DESKTOP_NAMES property
  }
}


void Workspace::setName(const string& new_name) {
  // set the _NET_WM_DESKTOP_NAMES property with the new name
  XAtom::StringVect namesList;
  unsigned long numnames = (unsigned) -1;
  if (xatom->getValue(screen->getRootWindow(), XAtom::net_desktop_names,
                      XAtom::utf8, numnames, namesList) &&
      namesList.size() > id)
    namesList[id] = new_name;
  else
    namesList.push_back(new_name);

  xatom->setValue(screen->getRootWindow(), XAtom::net_desktop_names,
                  XAtom::utf8, namesList);
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


bool Workspace::smartPlacement(Rect& win) {
  rectList spaces;
 
  //initially the entire screen is free
#ifdef    XINERAMA
  if (screen->isXineramaActive() &&
      screen->getBlackbox()->doXineramaPlacement()) {
    RectList availableAreas = screen->allAvailableAreas();
    RectList::iterator it, end = availableAreas.end();

    for (it = availableAreas.begin(); it != end; ++it)
      spaces.push_back(*it);
  } else
#endif // XINERAMA
    spaces.push_back(screen->availableArea());

  //Find Free Spaces
  BlackboxWindowList::const_iterator wit = windowList.begin(),
    end = windowList.end();
  Rect tmp;
  for (; wit != end; ++wit) {
    const BlackboxWindow* const curr = *wit;

    // watch for shaded windows and full-maxed windows
    if (curr->isShaded()) {
      if (screen->getPlaceIgnoreShaded()) continue;
    } else if (curr->isMaximizedFull()) {
      if (screen->getPlaceIgnoreMaximized()) continue;
    }

    tmp.setRect(curr->frameRect().x(), curr->frameRect().y(),
                curr->frameRect().width() + screen->getBorderWidth(),
                curr->frameRect().height() + screen->getBorderWidth());

    spaces = calcSpace(tmp, spaces);
  }

  if (screen->getPlacementPolicy() == BScreen::RowSmartPlacement) {
    if(screen->getRowPlacementDirection() == BScreen::LeftRight) {
      if(screen->getColPlacementDirection() == BScreen::TopBottom)
        std::sort(spaces.begin(), spaces.end(), rowLRTB);
      else
        std::sort(spaces.begin(), spaces.end(), rowLRBT);
    } else {
      if(screen->getColPlacementDirection() == BScreen::TopBottom)
        std::sort(spaces.begin(), spaces.end(), rowRLTB);
      else
        std::sort(spaces.begin(), spaces.end(), rowRLBT);
    }
  } else {
    if(screen->getColPlacementDirection() == BScreen::TopBottom) {
      if(screen->getRowPlacementDirection() == BScreen::LeftRight)
        std::sort(spaces.begin(), spaces.end(), colLRTB);
      else
        std::sort(spaces.begin(), spaces.end(), colRLTB);
    } else {
      if(screen->getRowPlacementDirection() == BScreen::LeftRight)
        std::sort(spaces.begin(), spaces.end(), colLRBT);
      else
        std::sort(spaces.begin(), spaces.end(), colRLBT);
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


bool Workspace::underMousePlacement(Rect &win) {
  int x, y, rx, ry;
  Window c, r;
  unsigned int m;
  XQueryPointer(screen->getBlackbox()->getXDisplay(), screen->getRootWindow(),
                &r, &c, &rx, &ry, &x, &y, &m);

  Rect area;
#ifdef    XINERAMA
  if (screen->isXineramaActive() &&
      screen->getBlackbox()->doXineramaPlacement()) {
    RectList availableAreas = screen->allAvailableAreas();
    RectList::iterator it, end = availableAreas.end();

    for (it = availableAreas.begin(); it != end; ++it)
      if (it->contains(rx, ry)) break;
    assert(it != end);  // the mouse isn't inside an area?
    area = *it;
  } else
#endif // XINERAMA
    area = screen->availableArea();
  
  x = rx - win.width() / 2;
  y = ry - win.height() / 2;

  if (x < area.x())
    x = area.x();
  if (y < area.y())
    y = area.y();
  if (x + win.width() > area.x() + area.width())
    x = area.x() + area.width() - win.width();
  if (y + win.height() > area.y() + area.height())
    y = area.y() + area.height() - win.height();

  win.setX(x);
  win.setY(y);

  return True;
}


bool Workspace::cascadePlacement(Rect &win, const int offset) {
  Rect area;
  
#ifdef    XINERAMA
  if (screen->isXineramaActive() &&
      screen->getBlackbox()->doXineramaPlacement()) {
    area = screen->allAvailableAreas()[cascade_region];
  } else
#endif // XINERAMA
    area = screen->availableArea();

  if ((static_cast<signed>(cascade_x + win.width()) > area.right() + 1) ||
      (static_cast<signed>(cascade_y + win.height()) > area.bottom() + 1)) {
    cascade_x = cascade_y = 0;
#ifdef    XINERAMA
    if (screen->isXineramaActive() &&
        screen->getBlackbox()->doXineramaPlacement()) {
      // go to the next xinerama region, and use its area
      if (++cascade_region >= screen->allAvailableAreas().size())
        cascade_region = 0;
      area = screen->allAvailableAreas()[cascade_region];
    }
#endif // XINERAMA
  }

  if (cascade_x == 0) {
    cascade_x = area.x() + offset;
    cascade_y = area.y() + offset;
  }

  win.setPos(cascade_x, cascade_y);

  cascade_x += offset;
  cascade_y += offset;

  return True;
}


void Workspace::placeWindow(BlackboxWindow *win) {
  Rect new_win(0, 0, win->frameRect().width(), win->frameRect().height());
  bool placed = False;

  switch (screen->getPlacementPolicy()) {
  case BScreen::RowSmartPlacement:
  case BScreen::ColSmartPlacement:
    placed = smartPlacement(new_win);
    break;
  case BScreen::UnderMousePlacement:
  case BScreen::ClickMousePlacement:
    placed = underMousePlacement(new_win);
  default:
    break; // handled below
  } // switch

  if (placed == False)
    cascadePlacement(new_win, (win->getTitleHeight() +
                               screen->getBorderWidth() * 2));

  if (new_win.right() > screen->availableArea().right())
    new_win.setX(screen->availableArea().left());
  if (new_win.bottom() > screen->availableArea().bottom())
    new_win.setY(screen->availableArea().top());

  win->configure(new_win.x(), new_win.y(), new_win.width(), new_win.height());
}
