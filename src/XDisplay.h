// XDisplay.h for Openbox
// Copyright (c) 2002 - 2002 Ben Janens (ben at orodu.net)
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

#ifndef   __XDisplay_h
#define   __XDisplay_h

#include <X11/Xlib.h>
#include <string>
#include <vector>

class Xdisplay {
  friend XAtom::XAtom();
  //friend class XAtom;

private:
  Display       *_display;
  std::string    _name;
  unsigned int   _grabs;
  bool           _hasshape;
  int            _shape_event_base;

  typedef std::vector<XScreen*> XScreenList;
  XScreenList    _screens;
 
  int XErrorHandler(Display *d, XErrorEvent *e);

  // no copying!!
  XDisplay(const XDisplay &);
  XDisplay& operator=(const XDisplay&);
  
protected:
  virtual void process_event(XEvent *) = 0;

public:
  Xdisplay(const char *dpyname = 0);
  virtual ~Xdisplay();

  XScreen *screen(unsigned int s) const;
  inline unsigned int screenCount() const { return _screens.size(); }
  
  inline bool hasShape() const { return _hasshape; }
  inline int shapeEventBase() const { return shape.event_basep; }

  //inline Display *display() const { return _display; }

  inline std::string name() const { return name; }

  // these belong in Xwindow
  //const bool validateWindow(Window);
  //void grabButton(unsigned int, unsigned int, Window, Bool, unsigned int, int,
  //    int, Window, Cursor) const;
  //void ungrabButton(unsigned int button, unsigned int modifiers,
  //    Window grab_window) const;
  
  void grab();
  void ungrab();
};

#endif // _XDisplay_h
