// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "bindings.hh"
#include "screen.hh"
#include "openbox.hh"
#include "client.hh"
#include "frame.hh"
#include "python.hh"
#include "otk/display.hh"

extern "C" {
#include <X11/Xlib.h>

#include "gettext.h"
#define _(str) gettext(str)
}

namespace ob {

static bool buttonvalue(const std::string &button, unsigned int *val)
{
  if (button == "1" || button == "Button1") {
    *val |= Button1;
  } else if (button == "2" || button == "Button2") {
    *val |= Button2;
  } else if (button == "3" || button == "Button3") {
    *val |= Button3;
  } else if (button == "4" || button == "Button4") {
    *val |= Button4;
  } else if (button == "5" || button == "Button5") {
    *val |= Button5;
  } else
    return false;
  return true;
}

static bool modvalue(const std::string &mod, unsigned int *val)
{
  if (mod == "C") {           // control
    *val |= ControlMask;
  } else if (mod == "S") {    // shift
    *val |= ShiftMask;
  } else if (mod == "A" ||    // alt/mod1
             mod == "M" ||
             mod == "Mod1" ||
             mod == "M1") {
    *val |= Mod1Mask;
  } else if (mod == "Mod2" ||   // mod2
             mod == "M2") {
    *val |= Mod2Mask;
  } else if (mod == "Mod3" ||   // mod3
             mod == "M3") {
    *val |= Mod3Mask;
  } else if (mod == "W" ||    // windows/mod4
             mod == "Mod4" ||
             mod == "M4") {
    *val |= Mod4Mask;
  } else if (mod == "Mod5" ||   // mod5
             mod == "M5") {
    *val |= Mod5Mask;
  } else {                    // invalid
    return false;
  }
  return true;
}

bool OBBindings::translate(const std::string &str, Binding &b,bool askey) const
{
  // parse out the base key name
  std::string::size_type keybegin = str.find_last_of('-');
  keybegin = (keybegin == std::string::npos) ? 0 : keybegin + 1;
  std::string key(str, keybegin);

  // parse out the requested modifier keys
  unsigned int modval = 0;
  std::string::size_type begin = 0, end;
  while (begin != keybegin) {
    end = str.find_first_of('-', begin);

    std::string mod(str, begin, end-begin);
    if (!modvalue(mod, &modval)) {
      printf(_("Invalid modifier element in key binding: %s\n"), mod.c_str());
      return false;
    }
    
    begin = end + 1;
  }

  // set the binding
  b.modifiers = modval;
  if (askey) {
    KeySym sym = XStringToKeysym(const_cast<char *>(key.c_str()));
    if (sym == NoSymbol) {
      printf(_("Invalid Key name in key binding: %s\n"), key.c_str());
      return false;
    }
    if (!(b.key = XKeysymToKeycode(otk::OBDisplay::display, sym)))
      printf(_("No valid keycode for Key in key binding: %s\n"), key.c_str());
    return b.key != 0;
  } else {
    return buttonvalue(key, &b.key);
  }
}

static void destroytree(KeyBindingTree *tree)
{
  while (tree) {
    KeyBindingTree *c = tree->first_child;
    delete tree;
    tree = c;
  }
}

KeyBindingTree *OBBindings::buildtree(const StringVect &keylist,
                                   PyObject *callback) const
{
  if (keylist.empty()) return 0; // nothing in the list.. return 0

  KeyBindingTree *ret = 0, *p;

  StringVect::const_reverse_iterator it, end = keylist.rend();
  for (it = keylist.rbegin(); it != end; ++it) {
    p = ret;
    ret = new KeyBindingTree(callback);
    if (!p) ret->chain = false; // only the first built node
    ret->first_child = p;
    if (!translate(*it, ret->binding)) {
      destroytree(ret);
      ret = 0;
      break;
    }
  }
  return ret;
}


OBBindings::OBBindings()
  : _curpos(&_keytree),
    _resetkey(0,0),
    _timer(Openbox::instance->timerManager(),
           (otk::OBTimeoutHandler)reset, this)
{
  _timer.setTimeout(5000); // chains reset after 5 seconds
  
  setResetKey("C-g"); // set the default reset key
}


OBBindings::~OBBindings()
{
  grabKeys(false);
  removeAll();
}


void OBBindings::assimilate(KeyBindingTree *node)
{
  KeyBindingTree *a, *b, *tmp, *last;

  if (!_keytree.first_child) {
    // there are no nodes at this level yet
    _keytree.first_child = node;
  } else {
    a = _keytree.first_child;
    last = a;
    b = node;
    while (a) {
      last = a;
      if (a->binding != b->binding) {
        a = a->next_sibling;
      } else {
        tmp = b;
        b = b->first_child;
        delete tmp;
        a = a->first_child;
      }
    }
    if (last->binding != b->binding)
      last->next_sibling = b;
    else {
      last->first_child = b->first_child;
      delete b;
    }
  }
}


PyObject *OBBindings::find(KeyBindingTree *search, bool *conflict) const {
  *conflict = false;
  KeyBindingTree *a, *b;
  a = _keytree.first_child;
  b = search;
  while (a && b) {
    if (a->binding != b->binding) {
      a = a->next_sibling;
    } else {
      if (a->chain == b->chain) {
	if (!a->chain) {
          // found it! (return the actual id, not the search's)
	  return a->callback;
        }
      } else {
        *conflict = true;
        return 0; // the chain status' don't match (conflict!)
      }
      b = b->first_child;
      a = a->first_child;
    }
  }
  return 0; // it just isn't in here
}


bool OBBindings::add(const StringVect &keylist, PyObject *callback)
{
  KeyBindingTree *tree;
  bool conflict;

  if (!(tree = buildtree(keylist, callback)))
    return false; // invalid binding requested

  if (find(tree, &conflict) || conflict) {
    // conflicts with another binding
    destroytree(tree);
    return false;
  }

  grabKeys(false);
  
  // assimilate this built tree into the main tree
  assimilate(tree); // assimilation destroys/uses the tree

  Py_INCREF(callback);

  grabKeys(true); 
 
  return true;
}


bool OBBindings::remove(const StringVect &keylist)
{
  assert(false); // XXX: function not implemented yet

  KeyBindingTree *tree;
  bool conflict;

  if (!(tree = buildtree(keylist, 0)))
    return false; // invalid binding requested

  PyObject *func = find(tree, &conflict);
  if (func) {
    grabKeys(false);

    _curpos = &_keytree;
    
    // XXX do shit here ...
    Py_DECREF(func);
    
    grabKeys(true);
    return true;
  }
  return false;
}


void OBBindings::setResetKey(const std::string &key)
{
  Binding b(0, 0);
  if (translate(key, b)) {
    grabKeys(false);
    _resetkey.key = b.key;
    _resetkey.modifiers = b.modifiers;
    grabKeys(true);
  }
}


static void remove_branch(KeyBindingTree *first)
{
  KeyBindingTree *p = first;

  while (p) {
    if (p->first_child)
      remove_branch(p->first_child);
    KeyBindingTree *s = p->next_sibling;
    Py_XDECREF(p->callback);
    delete p;
    p = s;
  }
}


void OBBindings::removeAll()
{
  if (_keytree.first_child) {
    remove_branch(_keytree.first_child);
    _keytree.first_child = 0;
  }
}


void OBBindings::grabKeys(bool grab)
{
  for (int i = 0; i < Openbox::instance->screenCount(); ++i) {
    Window root = otk::OBDisplay::screenInfo(i)->rootWindow();

    KeyBindingTree *p = _curpos->first_child;
    while (p) {
      if (grab) {
        otk::OBDisplay::grabKey(p->binding.key, p->binding.modifiers,
                                root, false, GrabModeAsync, GrabModeAsync,
                                false);
      }
      else
        otk::OBDisplay::ungrabKey(p->binding.key, p->binding.modifiers,
                                  root);
      p = p->next_sibling;
    }

    if (grab)
      otk::OBDisplay::grabKey(_resetkey.key, _resetkey.modifiers,
                              root, true, GrabModeAsync, GrabModeAsync,
                              false);
    else
      otk::OBDisplay::ungrabKey(_resetkey.key, _resetkey.modifiers,
                                root);
  }
}


void OBBindings::fire(unsigned int modifiers, unsigned int key, Time time)
{
  if (key == _resetkey.key && modifiers == _resetkey.modifiers) {
    reset(this);
  } else {
    KeyBindingTree *p = _curpos->first_child;
    while (p) {
      if (p->binding.key == key && p->binding.modifiers == modifiers) {
        if (p->chain) {
          _timer.start(); // start/restart the timer
          grabKeys(false);
          _curpos = p;
          grabKeys(true);
        } else {
          Window win = None;
          OBClient *c = Openbox::instance->focusedClient();
          if (c) win = c->window();
          KeyData *data = new_key_data(win, time, modifiers, key);
          python_callback(p->callback, (PyObject*)data);
          Py_DECREF((PyObject*)data);
          reset(this);
        }
        break;
      }
      p = p->next_sibling;
    }
  }
}

void OBBindings::reset(OBBindings *self)
{
  self->_timer.stop();
  self->grabKeys(false);
  self->_curpos = &self->_keytree;
  self->grabKeys(true);
}


bool OBBindings::addButton(const std::string &but, MouseContext context,
                           MouseAction action, PyObject *callback)
{
  assert(context >= 0 && context < NUM_MOUSE_CONTEXT);
  
  Binding b(0,0);
  if (!translate(but, b, false))
    return false;

  ButtonBindingList::iterator it, end = _buttons[context].end();

  // look for a duplicate binding
  for (it = _buttons[context].begin(); it != end; ++it)
    if ((*it)->binding.key == b.key &&
        (*it)->binding.modifiers == b.modifiers) {
      ButtonBinding::CallbackList::iterator c_it,
        c_end = (*it)->callback[action].end();
      for (c_it = (*it)->callback[action].begin(); c_it != c_end; ++c_it)
        if (*c_it == callback)
          return true; // already bound
      break;
    }

  ButtonBinding *bind;
  
  // the binding didnt exist yet, add it
  if (it == end) {
    bind = new ButtonBinding();
    bind->binding.key = b.key;
    bind->binding.modifiers = b.modifiers;
    _buttons[context].push_back(bind);
    printf("adding %d.%d to %d\n", b.key, b.modifiers, context);
    // XXX GRAB the new button everywhere!
  } else
    bind = *it;
  bind->callback[action].push_back(callback);
  Py_INCREF(callback);
  return true;
}

void OBBindings::grabButtons(bool grab, OBClient *client)
{
  for (int i = 0; i < NUM_MOUSE_CONTEXT; ++i) {
    Window win;
    int mode = GrabModeAsync;
    switch (i) {
    case MC_Frame:
      win = client->frame->window();
      break;
    case MC_Window:
      win = client->frame->plate();
      mode = GrabModeSync; // this is handled in the plate's buttonPressHandler
      break;
    default:
      continue;
    }
    ButtonBindingList::iterator it, end = _buttons[i].end();
    for (it = _buttons[i].begin(); it != end; ++it)
      if (grab)
        otk::OBDisplay::grabButton((*it)->binding.key,
                                   (*it)->binding.modifiers, win, false,
                                   ButtonPressMask | ButtonMotionMask |
                                   ButtonReleaseMask, mode, GrabModeAsync,
                                   None, None, false);
      else
        otk::OBDisplay::ungrabButton((*it)->binding.key,
                                     (*it)->binding.modifiers, win);
  }
}

void OBBindings::fire(ButtonData *data)
{
  printf("but.mods %d.%d\n", data->button, data->state);
  
  ButtonBindingList::iterator it, end = _buttons[data->context].end();
  for (it = _buttons[data->context].begin(); it != end; ++it)
    if ((*it)->binding.key == data->button &&
        (*it)->binding.modifiers == data->state) {
      ButtonBinding::CallbackList::iterator c_it,
        c_end = (*it)->callback[data->action].end();
      for (c_it = (*it)->callback[data->action].begin(); c_it != c_end; ++c_it)
        python_callback(*c_it, (PyObject*)data);
    }
}

}
