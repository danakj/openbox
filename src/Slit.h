// Slit.h for Openbox
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
  
#ifndef   __Slit_hh
#define   __Slit_hh

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Basemenu.h"
#include "LinkedList.h"
#include "Geometry.h"

// forward declaration
class Slit;
class Slitmenu;

class Slitmenu : public Basemenu {
private: 
  class Directionmenu : public Basemenu {
  private:
    Slitmenu &slitmenu;

  protected:
    virtual void itemSelected(int, int);
    virtual void setValues();

  public:
    Directionmenu(Slitmenu &);
    void reconfigure();
  };

  class Placementmenu : public Basemenu {
  private:
    Slitmenu &slitmenu;

  protected: 
    virtual void itemSelected(int, int);

  public:
    Placementmenu(Slitmenu &);
  };

  Directionmenu *directionmenu;
  Placementmenu *placementmenu;

  Slit &slit;

  friend class Directionmenu;
  friend class Placementmenu;
  friend class Slit;


protected:
  virtual void itemSelected(int, int);
  virtual void internal_hide();
  virtual void setValues();

public:
  Slitmenu(Slit &);
  virtual ~Slitmenu();

  inline Basemenu *getDirectionmenu() { return directionmenu; }
  inline Basemenu *getPlacementmenu() { return placementmenu; }

  void reconfigure();
};


class Slit : public TimeoutHandler {
private:
  class SlitClient {
  public:
    Window window, client_window, icon_window;

    int x, y;
    unsigned int width, height;
  };

  bool m_ontop, m_autohide, m_hidden;
  int m_direction, m_placement; 
  Display *display;

  Openbox &openbox;
  BScreen &screen;
  Resource &config;
  BTimer *timer;

  LinkedList<SlitClient> *clientList;
  Slitmenu *slitmenu;

  struct frame {
    Pixmap pixmap;
    Window window;

    Rect area;
    Point hidden;
  } frame;


  friend class Slitmenu;
  friend class Slitmenu::Directionmenu;
  friend class Slitmenu::Placementmenu;


public:
  Slit(BScreen &, Resource &);
  virtual ~Slit();

  inline Slitmenu *getMenu() { return slitmenu; }

  inline const Window &getWindowID() const { return frame.window; }

  inline const Point &origin() const { return frame.area.origin(); }
  inline const Size &size() const { return frame.area.size(); }
  inline const Rect &area() const { return frame.area; }

  void addClient(Window);
  void removeClient(SlitClient *, Bool = True);
  void removeClient(Window, Bool = True);
  void reconfigure();
  void load();
  void save();
  void reposition();
  void shutdown();

  void buttonPressEvent(XButtonEvent *);
  void enterNotifyEvent(XCrossingEvent *);
  void leaveNotifyEvent(XCrossingEvent *);
  void configureRequestEvent(XConfigureRequestEvent *);

  virtual void timeout();

  inline bool isHidden() const { return m_hidden; }

  inline bool onTop() const { return m_ontop; }
  void setOnTop(bool);
  
  inline bool autoHide() const { return m_autohide; }
  void setAutoHide(bool);
  
  inline int placement() const { return m_placement; }
  void setPlacement(int);

  inline int direction() const { return m_direction; }
  void setDirection(int);

  enum { Vertical = 1, Horizontal };
  enum { TopLeft = 1, CenterLeft, BottomLeft, TopCenter, BottomCenter,
         TopRight, CenterRight, BottomRight };
};


#endif // __Slit_hh
