// -*- mode: C++; indent-tabs-mode: nil; -*-
// Configmenu.hh for Blackbox - An X11 Window Manager
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

#ifndef   __Configmenu_hh
#define   __Configmenu_hh

#include "Basemenu.hh"

// forward declaration
class Blackbox;
class BScreen;
class Configmenu;

class Configmenu : public Basemenu {
private:
  class Focusmenu : public Basemenu {
  private:
    Focusmenu(const Focusmenu&);
    Focusmenu& operator=(const Focusmenu&);

  protected:
    virtual void itemSelected(int button, unsigned int index);
    virtual void setValues(void);

  public:
    Focusmenu(Configmenu *cm);
    virtual void reconfigure(void);
  };

  class Placementmenu : public Basemenu {
  private:
    Placementmenu(const Placementmenu&);
    Placementmenu& operator=(const Placementmenu&);

  protected:
    virtual void itemSelected(int button, unsigned int index);
    virtual void setValues(void);

  public:
    Placementmenu(Configmenu *cm);
    virtual void reconfigure(void);
  };

  class WindowToWindowSnapmenu : public Basemenu {
  private:
    WindowToWindowSnapmenu(const WindowToWindowSnapmenu&);
    WindowToWindowSnapmenu& operator=(const WindowToWindowSnapmenu&);

  protected:
    virtual void itemSelected(int button, unsigned int index);
    virtual void setValues(void);

  public:
    WindowToWindowSnapmenu(Configmenu *cm);
    virtual void reconfigure(void);
  };

  class WindowToEdgeSnapmenu : public Basemenu {
  private:
    WindowToEdgeSnapmenu(const WindowToEdgeSnapmenu&);
    WindowToEdgeSnapmenu& operator=(const WindowToEdgeSnapmenu&);

  protected:
    virtual void itemSelected(int button, unsigned int index);
    virtual void setValues(void);

  public:
    WindowToEdgeSnapmenu(Configmenu *cm);
    virtual void reconfigure(void);
  };

#ifdef    XINERAMA
  class Xineramamenu : public Basemenu {
  private:
    Xineramamenu(const Xineramamenu&);
    Xineramamenu& operator=(const Xineramamenu&);

  protected:
    virtual void itemSelected(int button, unsigned int index);
    virtual void setValues(void);

  public:
    Xineramamenu(Configmenu *cm);
    virtual void reconfigure(void);
  };
#endif // XINERAMA

  Focusmenu *focusmenu;
  Placementmenu *placementmenu;
  WindowToWindowSnapmenu *windowsnapmenu;
  WindowToEdgeSnapmenu *edgesnapmenu;
#ifdef    XINERAMA
  Xineramamenu *xineramamenu;
#endif // XINERAMA

//  friend class Focusmenu;
//  friend class Placementmenu;

  Configmenu(const Configmenu&);
  Configmenu& operator=(const Configmenu&);

protected:
  virtual void itemSelected(int button, unsigned int index);
  virtual void setValues(void);

public:
  Configmenu(BScreen *scr);
  virtual ~Configmenu(void);

  inline Basemenu *getFocusmenu(void) { return focusmenu; }
  inline Basemenu *getPlacementmenu(void) { return placementmenu; }
  inline Basemenu *getWindowSnapmenu(void) { return windowsnapmenu; }
  inline Basemenu *getEdgeSnapmenu(void) { return edgesnapmenu; }
#ifdef    XINERAMA
  inline Basemenu *getXineramamenu(void) { return xineramamenu; }
#endif // XINERAMA

  virtual void reconfigure(void);
};

#endif // __Configmenu_hh
