// Workspace.h for Openbox
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

#ifndef   __Workspace_hh
#define   __Workspace_hh

#include <X11/Xlib.h>

#include "LinkedList.h"

class BScreen;
class Clientmenu;
class Workspace;
class OpenboxWindow;
class Size;
class Rect;

class Workspace {
private:
  BScreen &screen;
  OpenboxWindow *lastfocus;
  Clientmenu *clientmenu;

  LinkedList<OpenboxWindow> *stackingList, *windowList;

  char *name;
  int id, cascade_x, cascade_y;

  OpenboxWindow *_focused;

protected:
  void placeWindow(OpenboxWindow &);
  Point *bestFitPlacement(const Size &win_size, const Rect &space);
  Point *underMousePlacement(const Size &win_size, const Rect &space);
  Point *rowSmartPlacement(const Size &win_size, const Rect &space);
  Point *colSmartPlacement(const Size &win_size, const Rect &space);
  Point *const cascadePlacement(const OpenboxWindow &window, const Rect &space);

public:
  Workspace(BScreen &, int = 0);
  ~Workspace(void);

  inline BScreen &getScreen(void) { return screen; }
  inline OpenboxWindow *getLastFocusedWindow(void) { return lastfocus; }
  inline Clientmenu *getMenu(void) { return clientmenu; }
  inline const char *getName(void) const { return name; }
  inline const int &getWorkspaceID(void) const { return id; }
  inline void setLastFocusedWindow(OpenboxWindow *w) { lastfocus = w; }
  inline OpenboxWindow *focusedWindow() { return _focused; }
  void focusWindow(OpenboxWindow *win);
  OpenboxWindow *getWindow(int);
  Bool isCurrent(void);
  Bool isLastWindow(OpenboxWindow *);
  const int addWindow(OpenboxWindow *, Bool = False);
  const int removeWindow(OpenboxWindow *);
  const int getCount(void);
  void showAll(void);
  void hideAll(void);
  void removeAll(void);
  void raiseWindow(OpenboxWindow *);
  void lowerWindow(OpenboxWindow *);
  void reconfigure();
  void update();
  void setCurrent(void);
  void setName(char *);
  void shutdown(void);
};


#endif // __Workspace_hh

