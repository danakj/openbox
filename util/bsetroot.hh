// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Window.cc for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh at debian.org>
// Copyright (c) 1997 - 2000, 2002 Brad Hughes <bhughes at trolltech.com>
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

#ifndef   __bsetroot2_hh
#define   __bsetroot2_hh

#include "../src/BaseDisplay.hh"
#include "../src/Image.hh"

#include <string>

class bsetroot : public BaseDisplay {
private:
  BImageControl **img_ctrl;

  std::string fore, back, grad;

  // no copying!!
  bsetroot(const bsetroot &);
  bsetroot& operator=(const bsetroot&);

  inline virtual void process_event(XEvent * /*unused*/) { }

public:
  bsetroot(int argc, char **argv, char *dpy_name = 0);
  ~bsetroot(void);

  inline virtual bool handleSignal(int /*unused*/) { return False; }

  void setPixmapProperty(int screen, Pixmap pixmap);
  Pixmap duplicatePixmap(int screen, Pixmap pixmap, int width, int height);

  void gradient(void);
  void modula(int x, int y);
  void solid(void);
  void usage(int exit_code = 0);
};


#endif // __bsetroot2_hh
