// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// keytree.hh for Epistrophy - a key handler for NETWM/EWMH window managers.
// Copyright (c) 2002 - 2002 Ben Jansens <ben at orodu.net>
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

#ifndef _keytree_hh
#define _keytree_hh

#include <list>
#include "../../src/Timer.hh"
#include "actions.hh"
#include "screen.hh"

struct keynode; // forward declaration
typedef std::list<keynode *> ChildList;

struct keynode {
  Action *action;
  keynode *parent;
  ChildList children;
};

class keytree : public TimeoutHandler {
public:
  keytree(Display *, epist *);
  virtual ~keytree();

  void grabDefaults(screen *);
  void ungrabDefaults(screen *);
  const Action * getAction(const XEvent&, unsigned int, screen *);
  void unloadBindings();
  void timeout();

private:
  // only mister parser needs to know about our sekrets (BUMMY)
  friend class parser;
    
  void grabChildren(keynode *, screen *);
  void ungrabChildren(keynode *, screen *);

  void addAction(Action::ActionType, unsigned int, std::string, std::string);
  void advanceOnNewNode();
  void retract();
  void setCurrentNodeProps(Action::ActionType, unsigned int, std::string, std::string);
  void initialize();

  void reset()
  { _current = _head; }

  bool isLeaf(keynode *node)
  { return node->children.empty(); }

  void clearTree(keynode *);

  keynode *_head;
  keynode *_current;
  Display *_display;
  screen *_timeout_screen;
  BTimer *_timer;
  epist *_epist;
};

#endif // _keytree_hh
