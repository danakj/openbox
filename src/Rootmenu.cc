// -*- mode: C++; indent-tabs-mode: nil; -*-
// Rootmenu.cc for Blackbox - an X11 Window manager
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
#include "Rootmenu.hh"
#include "Screen.hh"
#include "Util.hh"


Rootmenu::Rootmenu(BScreen *scrn) : Basemenu(scrn) { }


void Rootmenu::itemSelected(int button, unsigned int index) {
  if (button != 1)
    return;

  BasemenuItem *item = find(index);

  if (!item->function())
    return;

  switch (item->function()) {
  case BScreen::Execute:
    if (item->exec())
      bexec(item->exec(), getScreen()->displayString());
    break;

  case BScreen::Restart:
    getScreen()->getBlackbox()->restart();
    break;

  case BScreen::RestartOther:
    if (item->exec())
      getScreen()->getBlackbox()->restart(item->exec());
    break;

  case BScreen::Exit:
    getScreen()->getBlackbox()->shutdown();
    break;

  case BScreen::SetStyle:
    if (item->exec())
      getScreen()->getBlackbox()->saveStyleFilename(item->exec());

  case BScreen::Reconfigure:
    getScreen()->getBlackbox()->reconfigure();
    return;
  }

  if (! (getScreen()->getRootmenu()->isTorn() || isTorn()) &&
      item->function() != BScreen::Reconfigure &&
      item->function() != BScreen::SetStyle)
    hide();
}
