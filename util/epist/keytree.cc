// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// keytree.cc for Epistrophy - a key handler for NETWM/EWMH window managers.
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
#include "epist.hh"
#include "config.hh"

#include <string>

using std::string;

keytree::keytree(Display *display, epist *ep)
  : _display(display), _timeout_screen(NULL), _timer(NULL), _epist(ep)
{
  _head = new keynode;
  _head->parent = NULL;
  _head->action = NULL; // head's action is always NULL
  _current = _head;
  // for complete initialization, initialize() has to be called as well. We
  // call initialize() when we are certain that the config object (which the
  // timer uses) has been fully initialized. (see parser::parse())
}

keytree::~keytree()
{
  clearTree(_head);
  delete _timer;
}

void keytree::unloadBindings()
{
  ChildList::iterator it, end = _head->children.end();
  for (it = _head->children.begin(); it != end; ++it)
    clearTree(*it);

  _head->children.clear();
  reset();
}

void keytree::clearTree(keynode *node)
{
  if (!node)
    return;

  ChildList::iterator it, end = node->children.end();
  for (it = node->children.begin(); it != end; ++it)
    clearTree(*it);

  node->children.clear();

  if (node->action)
    delete node->action;
  delete node;
  node = NULL;
}

void keytree::grabDefaults(screen *scr)
{
  grabChildren(_head, scr);
}

void keytree::ungrabDefaults(screen *scr)
{
  ChildList::const_iterator it, end = _head->children.end();
  for (it = _head->children.begin(); it != end; ++it)
    if ( (*it)->action && (*it)->action->type() != Action::toggleGrabs)
      scr->ungrabKey( (*it)->action->keycode(), (*it)->action->modifierMask() );
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
  ChildList::const_iterator head_it, head_end = _head->children.end();
  ChildList::const_iterator it, end = node->children.end();
  bool ungrab = true;
 
  // when ungrabbing children, make sure that we don't ungrab any topmost keys
  // (children of the head node) This would render those topmost keys useless.
  // Topmost keys are _never_ ungrabbed, since they are only grabbed at startup
  
  for (it = node->children.begin(); it != end; ++it) {
    if ( (*it)->action ) {
      for (head_it = _head->children.begin(); head_it != head_end; ++head_it) {
        if ( (*it)->action->modifierMask() == (*head_it)->action->modifierMask() &&
             (*it)->action->keycode() == (*head_it)->action->keycode())
        {
          ungrab = false;
          break;
        }
      }
      
      if (ungrab) 
        scr->ungrabKey( (*it)->action->keycode(), (*it)->action->modifierMask());
      
      ungrab = true;
    }
  }
}

const Action * keytree::getAction(const XEvent &e, unsigned int state,
				  screen *scr)
{
  Action *act;

  // we're done with the children. ungrab them
  if (_current != _head)
    ungrabChildren(_current, scr);

  ChildList::const_iterator it, end = _current->children.end();
  for (it = _current->children.begin(); it != end; ++it) {
    act = (*it)->action;
    if (e.xkey.keycode == act->keycode() && state == act->modifierMask()) {
      if (act->type() == Action::cancelChain) {
        // user is cancelling the chain explicitly
        _current = _head;
        return (const Action *)NULL;
      }
      else if ( isLeaf(*it) ) {
        // node is a leaf, so an action will be executed
        if (_timer->isTiming()) {
          _timer->stop();
          _timeout_screen = NULL;
        }

        _current = _head;
        return act;
      }
      else {
        // node is not a leaf, so we advance down the tree, and grab the
        // children of the new current node. no action is executed
        if (_timer->isTiming())
          _timer->stop();
        _timer->start();
        _timeout_screen = scr;

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
  keynode *tmp = new keynode;

  if (action == Action::toggleGrabs && _current != _head) {
    // the toggleGrabs key can only be set up as a root key, since if
    // it was a chain key, we'd have to not ungrab the whole chain up
    // to that key. which kinda defeats the purpose of this function.
    return;
  }

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

void keytree::initialize(void)
{
  int tval = _epist->getConfig()->getNumberValue(Config::chainTimeout);
  _timer = new BTimer(_epist, this);

  if (tval <= 0)
    tval = 3000; // set default timeout to 3 seconds

  _timer->setTimeout(tval);
}

void keytree::timeout(void)
{
  assert(_timeout_screen != NULL);

  if (_current != _head) {
    ungrabChildren(_current, _timeout_screen);
    _current = _head;
  }
  _timeout_screen = NULL;
}
