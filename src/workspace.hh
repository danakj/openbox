// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __Workspace_hh
#define   __Workspace_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <list>
#include <string>
#include <vector>

#include "atom.hh"

namespace ob {

class BScreen;
class Workspace;
class BlackboxWindow;

typedef std::list<BlackboxWindow*> BlackboxWindowList;
typedef std::vector<Window> StackVector;

class Workspace {
private:
  BScreen *screen;
  BlackboxWindow *lastfocus;
  OBAtom *xatom;

  BlackboxWindowList stackingList, windowList;

  std::string name;
  unsigned int id;
  unsigned int cascade_x, cascade_y;
#ifdef    XINERAMA
  unsigned int cascade_region;
#endif // XINERAMA

  Workspace(const Workspace&);
  Workspace& operator=(const Workspace&);

  void raiseTransients(const BlackboxWindow * const win,
                       StackVector::iterator &stack);
  void lowerTransients(const BlackboxWindow * const win,
                       StackVector::iterator &stack);

  typedef std::vector<otk::Rect> rectList;
  rectList calcSpace(const otk::Rect &win, const rectList &spaces) const;

  void placeWindow(BlackboxWindow *win);
  bool cascadePlacement(otk::Rect& win, const int offset);
  bool smartPlacement(otk::Rect& win);
  bool underMousePlacement(otk::Rect& win);

public:
  Workspace(BScreen *scrn, unsigned int i = 0);

  inline BScreen *getScreen(void) { return screen; }

  inline BlackboxWindow *getLastFocusedWindow(void) { return lastfocus; }

  inline const std::string& getName(void) const { return name; }

  inline unsigned int getID(void) const { return id; }

  inline void setLastFocusedWindow(BlackboxWindow *w) { lastfocus = w; }

  inline const BlackboxWindowList& getStackingList() const
  { return stackingList; }

  BlackboxWindow* getWindow(unsigned int index);
  BlackboxWindow* getNextWindowInList(BlackboxWindow *w);
  BlackboxWindow* getPrevWindowInList(BlackboxWindow *w);
  BlackboxWindow* getTopWindowOnStack(void) const;
  void focusFallback(const BlackboxWindow *old_window);

  bool isCurrent(void) const;
  bool isLastWindow(const BlackboxWindow* w) const;

  void addWindow(BlackboxWindow *w, bool place = False, bool sticky = False);
  void removeWindow(BlackboxWindow *w, bool sticky = False);
  unsigned int getCount(void) const;
  void appendStackOrder(BlackboxWindowList &stack_order) const;

  void showAll(void);
  void hideAll(void);
  void removeAll(void);
  void raiseWindow(BlackboxWindow *w);
  void lowerWindow(BlackboxWindow *w);
  void reconfigure(void);
  void setCurrent(void);
  void readName();
  void setName(const std::string& new_name);
};

}

#endif // __Workspace_hh

