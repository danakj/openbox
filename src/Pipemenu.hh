// -*- mode: C++; indent-tabs-mode: nil; -*-
// Pipemenu.hh for Openbox - an X11 Window manager
// Copyright (c) 2002 Scott Moynes <smoynes@nexus.carleton.ca>
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

#ifndef   __Pipemenu_hh
#define   __Pipemenu_hh

// forward declarations
class BScreen;

#include <string>
using std::string;

#include "Rootmenu.hh"

class Pipemenu : public Rootmenu {
private:
  Pipemenu(const Pipemenu&);
  Pipemenu& operator=(const Pipemenu&);

public:
  Pipemenu(BScreen *scrn, const string& command);
  //virtual void update(void);
  virtual void show(void);
  bool readPipe();
private:
  string _command;
};


#endif // __Pipemenu_hh

