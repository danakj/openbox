// Workspace.cc for Openbox
// Copyright (c) 2002 - 2002 Ben Jansens (ben@orodu.net)
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

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "i18n.h"
#include "openbox.h"
#include "Clientmenu.h"
#include "Screen.h"
#include "Toolbar.h"
#include "Window.h"
#include "Workspace.h"

#include "Windowmenu.h"
#include "Geometry.h"
#include "Util.h"

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef    HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#include <algorithm>
#include <vector>
typedef std::vector<Rect> rectList;

Workspace::Workspace(BScreen &scrn, int i) : screen(scrn) {
  cascade_x = cascade_y = 0;
  _focused = (OpenboxWindow *) 0;
  id = i;

  stackingList = new LinkedList<OpenboxWindow>;
  windowList = new LinkedList<OpenboxWindow>;
  clientmenu = new Clientmenu(*this);

  lastfocus = (OpenboxWindow *) 0;

  name = (char *) 0;
  char *tmp = screen.getNameOfWorkspace(id);
  setName(tmp);
}


Workspace::~Workspace(void) {
  delete stackingList;
  delete windowList;
  delete clientmenu;

  if (name)
    delete [] name;
}


const int Workspace::addWindow(OpenboxWindow *w, Bool place) {
  if (! w) return -1;

  if (place) placeWindow(*w);

  w->setWorkspace(id);
  w->setWindowNumber(windowList->count());

  stackingList->insert(w, 0);
  windowList->insert(w);

  clientmenu->insert((const char **) w->getTitle());
  clientmenu->update();

  screen.updateNetizenWindowAdd(w->getClientWindow(), id);

  raiseWindow(w);

  return w->getWindowNumber();
}


const int Workspace::removeWindow(OpenboxWindow *w) {
  if (! w) return -1;

  stackingList->remove(w);

  if (w->isFocused()) {
    if (w->isTransient() && w->getTransientFor() &&
	w->getTransientFor()->isVisible()) {
      w->getTransientFor()->setInputFocus();
    } else if (screen.sloppyFocus()) {
      screen.getOpenbox().focusWindow((OpenboxWindow *) 0);
    } else {
      OpenboxWindow *top = stackingList->first();
      if (! top || ! top->setInputFocus()) {
	screen.getOpenbox().focusWindow((OpenboxWindow *) 0);
	XSetInputFocus(screen.getOpenbox().getXDisplay(),
		       screen.getToolbar()->getWindowID(),
		       RevertToParent, CurrentTime);
      }
    }
  }
  
  if (lastfocus == w)
    lastfocus = (OpenboxWindow *) 0;

  windowList->remove(w->getWindowNumber());
  clientmenu->remove(w->getWindowNumber());
  clientmenu->update();

  screen.updateNetizenWindowDel(w->getClientWindow());

  LinkedListIterator<OpenboxWindow> it(windowList);
  OpenboxWindow *bw = it.current();
  for (int i = 0; bw; it++, i++, bw = it.current())
    bw->setWindowNumber(i);

  return windowList->count();
}


void Workspace::focusWindow(OpenboxWindow *win) {
  if (win != (OpenboxWindow *) 0)
    clientmenu->setItemSelected(win->getWindowNumber(), true);
  if (_focused != (OpenboxWindow *) 0)
    clientmenu->setItemSelected(_focused->getWindowNumber(), false);
  _focused = win;
}


void Workspace::showAll(void) {
  LinkedListIterator<OpenboxWindow> it(stackingList);
  for (OpenboxWindow *bw = it.current(); bw; it++, bw = it.current())
    bw->deiconify(False, False);
}


void Workspace::hideAll(void) {
  LinkedList<OpenboxWindow> lst;

  LinkedListIterator<OpenboxWindow> it(stackingList);
  for (OpenboxWindow *bw = it.current(); bw; it++, bw = it.current())
    lst.insert(bw, 0);

  LinkedListIterator<OpenboxWindow> it2(&lst);
  for (OpenboxWindow *bw = it2.current(); bw; it2++, bw = it2.current())
    if (! bw->isStuck())
      bw->withdraw();
}


void Workspace::removeAll(void) {
  LinkedListIterator<OpenboxWindow> it(windowList);
  for (OpenboxWindow *bw = it.current(); bw; it++, bw = it.current())
    bw->iconify();
}


void Workspace::raiseWindow(OpenboxWindow *w) {
  OpenboxWindow *win = (OpenboxWindow *) 0, *bottom = w;

  while (bottom->isTransient() && bottom->getTransientFor())
    bottom = bottom->getTransientFor();

  int i = 1;
  win = bottom;
  while (win->hasTransient() && win->getTransient()) {
    win = win->getTransient();

    i++;
  }

  Window *nstack = new Window[i], *curr = nstack;
  Workspace *wkspc;

  win = bottom;
  while (True) {
    *(curr++) = win->getFrameWindow();
    screen.updateNetizenWindowRaise(win->getClientWindow());

    if (! win->isIconic()) {
      wkspc = screen.getWorkspace(win->getWorkspaceNumber());
      wkspc->stackingList->remove(win);
      wkspc->stackingList->insert(win, 0);
    }

    if (! win->hasTransient() || ! win->getTransient())
      break;

    win = win->getTransient();
  }

  screen.raiseWindows(nstack, i);

  delete [] nstack;
}


void Workspace::lowerWindow(OpenboxWindow *w) {
  OpenboxWindow *win = (OpenboxWindow *) 0, *bottom = w;

  while (bottom->isTransient() && bottom->getTransientFor())
    bottom = bottom->getTransientFor();

  int i = 1;
  win = bottom;
  while (win->hasTransient() && win->getTransient()) {
    win = win->getTransient();

    i++;
  }

  Window *nstack = new Window[i], *curr = nstack;
  Workspace *wkspc;

  while (True) {
    *(curr++) = win->getFrameWindow();
    screen.updateNetizenWindowLower(win->getClientWindow());

    if (! win->isIconic()) {
      wkspc = screen.getWorkspace(win->getWorkspaceNumber());
      wkspc->stackingList->remove(win);
      wkspc->stackingList->insert(win);
    }

    if (! win->getTransientFor())
      break;

    win = win->getTransientFor();
  }

  screen.getOpenbox().grab();

  XLowerWindow(screen.getBaseDisplay().getXDisplay(), *nstack);
  XRestackWindows(screen.getBaseDisplay().getXDisplay(), nstack, i);

  screen.getOpenbox().ungrab();

  delete [] nstack;
}


void Workspace::reconfigure(void) {
  clientmenu->reconfigure();

  LinkedListIterator<OpenboxWindow> it(windowList);
  for (OpenboxWindow *bw = it.current(); bw; it++, bw = it.current()) {
    if (bw->validateClient())
      bw->reconfigure();
  }
}


OpenboxWindow *Workspace::getWindow(int index) {
  if ((index >= 0) && (index < windowList->count()))
    return windowList->find(index);
  else
    return 0;
}


const int Workspace::getCount(void) {
  return windowList->count();
}


void Workspace::update(void) {
  clientmenu->update();
  screen.getToolbar()->redrawWindowLabel(True);
}


Bool Workspace::isCurrent(void) {
  return (id == screen.getCurrentWorkspaceID());
}


Bool Workspace::isLastWindow(OpenboxWindow *w) {
  return (w == windowList->last());
}

void Workspace::setCurrent(void) {
  screen.changeWorkspaceID(id);
}


void Workspace::setName(char *new_name) {
  if (name)
    delete [] name;

  if (new_name) {
    name = bstrdup(new_name);
  } else {
    name = new char[128];
    sprintf(name, i18n->getMessage(WorkspaceSet, WorkspaceDefaultNameFormat,
				   "Workspace %d"), id + 1);
  }
  
  clientmenu->setLabel(name);
  clientmenu->update();
  screen.saveWorkspaceNames();
}


void Workspace::shutdown(void) {
  while (windowList->count()) {
    windowList->first()->restore();
    delete windowList->first();
  }
}

static rectList calcSpace(const Rect &win, const rectList &spaces) {
  rectList result;
  rectList::const_iterator siter;
  for(siter=spaces.begin(); siter!=spaces.end(); ++siter) {
    if(win.Intersect(*siter)) {
      //Check for space to the left of the window
      if(win.x() > siter->x())
        result.push_back(Rect(siter->x(), siter->y(),
                              win.x() - siter->x() - 1,
                              siter->h()));
      //Check for space above the window
      if(win.y() > siter->y())
        result.push_back(Rect(siter->x(), siter->y(),
                              siter->w(),
                              win.y() - siter->y() - 1));
      //Check for space to the right of the window
      if((win.x()+win.w()) <
         (siter->x()+siter->w()))
        result.push_back(Rect(win.x() + win.w() + 1,
                              siter->y(),
                              siter->x() + siter->w() -
                              win.x() - win.w() - 1,
                              siter->h()));
      //Check for space below the window
      if((win.y()+win.h()) <
         (siter->y()+siter->h()))
        result.push_back(Rect(siter->x(),
                              win.y() + win.h() + 1,
                              siter->w(),
                              siter->y() + siter->h()-
                              win.y() - win.h() - 1));

    }
    else
      result.push_back(*siter);
  }
  return result;
}

bool rowRLBT(const Rect &first, const Rect &second){
  if (first.y()+first.h()==second.y()+second.h())
     return first.x()+first.w()>second.x()+second.w();
  return first.y()+first.h()>second.y()+second.h();
}
 
bool rowRLTB(const Rect &first, const Rect &second){
  if (first.y()==second.y())
     return first.x()+first.w()>second.x()+second.w();
  return first.y()<second.y();
}

bool rowLRBT(const Rect &first, const Rect &second){
  if (first.y()+first.h()==second.y()+second.h())
     return first.x()<second.x();
  return first.y()+first.h()>second.y()+second.h();
}
 
bool rowLRTB(const Rect &first, const Rect &second){
  if (first.y()==second.y())
     return first.x()<second.x();
  return first.y()<second.y();
}
 
bool colLRTB(const Rect &first, const Rect &second){
  if (first.x()==second.x())
     return first.y()<second.y();
  return first.x()<second.x();
}
 
bool colLRBT(const Rect &first, const Rect &second){
  if (first.x()==second.x())
     return first.y()+first.h()>second.y()+second.h();
  return first.x()<second.x();
}

bool colRLTB(const Rect &first, const Rect &second){
  if (first.x()+first.w()==second.x()+second.w())
     return first.y()<second.y();
  return first.x()+first.w()>second.x()+second.w();
}
 
bool colRLBT(const Rect &first, const Rect &second){
  if (first.x()+first.w()==second.x()+second.w())
     return first.y()+first.h()>second.y()+second.h();
  return first.x()+first.w()>second.x()+second.w();
}


//BestFitPlacement finds the smallest free space that fits the window
//to be placed. It currentl ignores whether placement is right to left or top
//to bottom.
Point *Workspace::bestFitPlacement(const Size &win_size, const Rect &space) {
  const Rect *best;
  rectList spaces;
  LinkedListIterator<OpenboxWindow> it(windowList);
  rectList::const_iterator siter;
  spaces.push_back(space); //initially the entire screen is free
  it.reset();
  
  //Find Free Spaces
  for (OpenboxWindow *cur=it.current(); cur!=NULL; it++, cur=it.current())
     spaces = calcSpace(cur->area().Inflate(screen.getBorderWidth() * 4),
                        spaces);
  
  //Find first space that fits the window
  best = NULL;
  for (siter=spaces.begin(); siter!=spaces.end(); ++siter) {
    if ((siter->w() >= win_size.w()) && (siter->h() >= win_size.h())) {
      if (best==NULL)
        best = &*siter;
      else if(siter->w()*siter->h()<best->h()*best->w())
        best = &*siter;
    }
  }
  if (best != NULL) {
    Point *pt = new Point(best->origin());
    if (screen.colPlacementDirection() != BScreen::TopBottom)
      pt->setY(pt->y() + (best->h() - win_size.h()));
    if (screen.rowPlacementDirection() != BScreen::LeftRight)
      pt->setX(pt->x() + (best->w() - win_size.w()));
    return pt;
  } else
    return NULL; //fall back to cascade
}

Point *Workspace::underMousePlacement(const Size &win_size, const Rect &space) {
  Point *pt;

  int x, y, rx, ry;
  Window c, r;
  unsigned int m;
  XQueryPointer(screen.getOpenbox().getXDisplay(), screen.getRootWindow(),
                &r, &c, &rx, &ry, &x, &y, &m);
  pt = new Point(rx - win_size.w() / 2, ry - win_size.h() / 2);

  if (pt->x() < space.x())
    pt->setX(space.x());
  if (pt->y() < space.y())
    pt->setY(space.y());
  if (pt->x() + win_size.w() > space.x() + space.w())
    pt->setX(space.x() + space.w() - win_size.w());
  if (pt->y() + win_size.h() > space.y() + space.h())
    pt->setY(space.y() + space.h() - win_size.h());
  return pt;
}

Point *Workspace::rowSmartPlacement(const Size &win_size, const Rect &space) {
  const Rect *best;
  rectList spaces;
  LinkedListIterator<OpenboxWindow> it(windowList);

  rectList::const_iterator siter;
  spaces.push_back(space); //initially the entire screen is free
  it.reset();
  
  //Find Free Spaces
  for (OpenboxWindow *cur=it.current(); cur!=NULL; it++, cur=it.current())
     spaces = calcSpace(cur->area().Inflate(screen.getBorderWidth() * 4),
                        spaces);
  //Sort spaces by preference
  if(screen.rowPlacementDirection() == BScreen::RightLeft)
     if(screen.colPlacementDirection() == BScreen::TopBottom)
        sort(spaces.begin(),spaces.end(),rowRLTB);
     else
        sort(spaces.begin(),spaces.end(),rowRLBT);
  else
     if(screen.colPlacementDirection() == BScreen::TopBottom)
        sort(spaces.begin(),spaces.end(),rowLRTB);
     else
        sort(spaces.begin(),spaces.end(),rowLRBT);
  best = NULL;
  for (siter=spaces.begin(); siter!=spaces.end(); ++siter)
    if ((siter->w() >= win_size.w()) && (siter->h() >= win_size.h())) {
      best = &*siter;
      break;
    }

  if (best != NULL) {
    Point *pt = new Point(best->origin());
    if (screen.colPlacementDirection() != BScreen::TopBottom)
      pt->setY(best->y() + best->h() - win_size.h());
    if (screen.rowPlacementDirection() != BScreen::LeftRight)
      pt->setX(best->x()+best->w()-win_size.w());
    return pt;
  } else
    return NULL; //fall back to cascade
}

Point *Workspace::colSmartPlacement(const Size &win_size, const Rect &space) {
  const Rect *best;
  rectList spaces;
  LinkedListIterator<OpenboxWindow> it(windowList);

  rectList::const_iterator siter;
  spaces.push_back(space); //initially the entire screen is free
  it.reset();
  
  //Find Free Spaces
  for (OpenboxWindow *cur=it.current(); cur!=NULL; it++, cur=it.current())
     spaces = calcSpace(cur->area().Inflate(screen.getBorderWidth() * 4),
                        spaces);
  //Sort spaces by user preference
  if(screen.colPlacementDirection() == BScreen::TopBottom)
     if(screen.rowPlacementDirection() == BScreen::LeftRight)
        sort(spaces.begin(),spaces.end(),colLRTB);
     else
        sort(spaces.begin(),spaces.end(),colRLTB);
  else
     if(screen.rowPlacementDirection() == BScreen::LeftRight)
        sort(spaces.begin(),spaces.end(),colLRBT);
     else
        sort(spaces.begin(),spaces.end(),colRLBT);

  //Find first space that fits the window
  best = NULL;
  for (siter=spaces.begin(); siter!=spaces.end(); ++siter)
    if ((siter->w() >= win_size.w()) && (siter->h() >= win_size.h())) {
      best = &*siter;
      break;
    }

  if (best != NULL) {
    Point *pt = new Point(best->origin());
    if (screen.colPlacementDirection() != BScreen::TopBottom)
      pt->setY(pt->y() + (best->h() - win_size.h()));
    if (screen.rowPlacementDirection() != BScreen::LeftRight)
      pt->setX(pt->x() + (best->w() - win_size.w()));
    return pt;
  } else
    return NULL; //fall back to cascade
}


Point *const Workspace::cascadePlacement(const OpenboxWindow &win,
                                         const Rect &space) {
  if ((cascade_x + win.area().w() + screen.getBorderWidth() * 2 >
       (space.x() + space.w())) ||
      (cascade_y + win.area().h() + screen.getBorderWidth() * 2 >
       (space.y() + space.h())))
    cascade_x = cascade_y = 0;
  if (cascade_x < space.x() || cascade_y < space.y()) {
    cascade_x = space.x();
    cascade_y = space.y();
  }

  Point *p = new Point(cascade_x, cascade_y);
  cascade_x += win.getTitleHeight();
  cascade_y += win.getTitleHeight();
  return p;
}


void Workspace::placeWindow(OpenboxWindow &win) {
  Rect space = screen.availableArea();
  const Size window_size(win.area().w()+screen.getBorderWidth() * 2,
                         win.area().h()+screen.getBorderWidth() * 2);
  Point *place = NULL;
  LinkedListIterator<OpenboxWindow> it(windowList);

  switch (screen.placementPolicy()) {
  case BScreen::BestFitPlacement:
    place = bestFitPlacement(window_size, space);
    break;
  case BScreen::RowSmartPlacement:
    place = rowSmartPlacement(window_size, space);
    break;
  case BScreen::ColSmartPlacement:
    place = colSmartPlacement(window_size, space);
    break;
  case BScreen::UnderMousePlacement:
  case BScreen::ClickMousePlacement:
    place = underMousePlacement(window_size, space);
    break;
  } // switch

  if (place == NULL)
    place = cascadePlacement(win, space);
 
  ASSERT(place != NULL);  
  if (place->x() + window_size.w() > (signed) space.x() + space.w())
    place->setX(((signed) space.x() + space.w() - window_size.w()) / 2);
  if (place->y() + window_size.h() > (signed) space.y() + space.h())
    place->setY(((signed) space.y() + space.h() - window_size.h()) / 2);

  win.configure(place->x(), place->y(), win.area().w(), win.area().h());
  delete place;
}
