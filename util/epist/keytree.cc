// -*- mode: C++; indent-tabs-mode: nil; -*-
// keytree.cc for Epistophy - a key handler for NETWM/EWMH window managers.
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

#include "keytree.hh"

keytree::keytree(Display *display) : _display(display)
{
  _head = new keynode;
  _head->parent = NULL;
  _head->action = NULL; // head's action is always NULL
  _current = _head;
}

keytree::~keytree()
{
  clearTree(_head);
}

void keytree::clearTree(keynode *node)
{
  if (!node)
    return;

  ChildList::iterator it, end = node->children.end();
  for (it = node->children.begin(); it != end; ++it)
    clearTree(*it);

  if (node->action)
    delete node->action;
  delete node;
}

void keytree::grabDefaults(screen *scr)
{
  grabChildren(_head, scr);
}

void keytree::grabChildren(keynode *node, screen *scr)
{
  ChildList::const_iterator it, end = node->children.end();
  for (it = node->children.begin(); it != end; ++it)
    if ( (*it)->action )
      scr->grabKey( (*it)->action->keycode(), (*it)->action->modifierMask() );
}

void keytree::ungrabChildren(keynode *node, screen *scr)
{
  ChildList::const_iterator it, end = node->children.end();
  for (it = node->children.begin(); it != end; ++it)
    if ( (*it)->action )
      scr->ungrabKey( (*it)->action->keycode(), (*it)->action->modifierMask());
}

const Action * keytree::getAction(const XEvent &e, unsigned int state,
				  screen *scr)
{
  Action *act;

  if (_current != _head)
    ungrabChildren(_current, scr);
  
  ChildList::const_iterator it, end = _current->children.end();
  for (it = _current->children.begin(); it != end; ++it) {
    act = (*it)->action;
    if (e.xkey.keycode == act->keycode() && state == act->modifierMask()) {
      if ( isLeaf(*it) ) {
	if (_current != _head)
	  ungrabChildren(_current, scr);
	_current = _head;
	return act;
      }
      else {
	_current = *it;
	grabChildren(_current, scr);
	return (const Action *)NULL;
      }
    }
  }

  // action not found. back to the head
  _current = _head;
  return (const Action *)NULL;
}

void keytree::addAction(Action::ActionType action, unsigned int mask,
                        string key, string arg)
{
  // can't grab non-modifier as topmost key
  if (_current == _head && (mask == 0 || mask == ShiftMask))
    return;

  keynode *tmp = new keynode;
  tmp->action = new Action(action,
			   XKeysymToKeycode(_display,
					    XStringToKeysym(key.c_str())),
			   mask, arg);
  tmp->parent = _current;
  _current->children.push_back(tmp);
}

void keytree::advanceOnNewNode()
{
  keynode *tmp = new keynode;
  tmp->action = NULL;
  tmp->parent = _current;
  _current->children.push_back(tmp);
  _current = tmp;
}

void keytree::retract()
{
  if (_current != _head)
    _current = _current->parent;
}

void keytree::setCurrentNodeProps(Action::ActionType action, unsigned int mask,
                                  string key, string arg)
{
  if (_current->action)
    delete _current->action;
  _current->action = new Action(action,
				XKeysymToKeycode(_display,
						 XStringToKeysym(key.c_str())),
				mask, arg);
}
