// -*- mode: C++; indent-tabs-mode: nil; -*-
// Pipemenu.cc for Openbox - an X11 Window manager
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

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#ifdef    HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif // HAVE_SYS_PARAM_H
}

#include "blackbox.hh"
#include "Pipemenu.hh"
#include "Screen.hh"
#include "Util.hh"


Pipemenu::Pipemenu(BScreen *scrn, const string& command) : Rootmenu(scrn),
                                                           _command(command){ }

bool Pipemenu::readPipe() {
  FILE* file = popen(_command.c_str(), "r");
  
  if (file != NULL)
  {
    printf("popen %s\n", _command.c_str());
    getScreen()->parseMenuFile(file, this);
    pclose(file);
    return true;
  }
  else
  {
    perror("bummy");
    return false;
  }
}

void Pipemenu::show(void) {
  printf("%s\n", __PRETTY_FUNCTION__);
  readPipe();
  update();
  visible = true;
  getParent()->drawSubmenu(getParent()->getCurrentSubmenu());
  Rootmenu::show();
}
