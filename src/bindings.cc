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

bool OBBindings::translate(const std::string &str, Binding &b) const
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
  KeySym sym = XStringToKeysym(const_cast<char *>(key.c_str()));
  if (sym == NoSymbol) {
    printf(_("Invalid Key name in key binding: %s\n"), key.c_str());
    return false;
  }
  if (!(b.key = XKeysymToKeycode(otk::OBDisplay::display, sym)))
    printf(_("No valid keycode for Key in key binding: %s\n"), key.c_str());
  return b.key != 0;
}

static void destroytree(BindingTree *tree)
{
  while (tree) {
    BindingTree *c = tree->first_child;
    delete tree;
    tree = c;
  }
}

BindingTree *OBBindings::buildtree(const StringVect &keylist, int id) const
{
  if (keylist.empty()) return 0; // nothing in the list.. return 0

  BindingTree *ret = 0, *p;

  StringVect::const_reverse_iterator it, end = keylist.rend();
  for (it = keylist.rbegin(); it != end; ++it) {
    p = ret;
    ret = new BindingTree(id);
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
  : _curpos(&_tree), _resetkey(0,0)
{
  setResetKey("C-g"); // set the default reset key
}


OBBindings::~OBBindings()
{
  grabKeys(false);
  remove_all();
}


void OBBindings::assimilate(BindingTree *node)
{
  BindingTree *a, *b, *tmp, *last;

  if (!_tree.first_child) {
    // there are no nodes at this level yet
    _tree.first_child = node;
  } else {
    a = _tree.first_child;
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


int OBBindings::find(BindingTree *search) const {
  BindingTree *a, *b;
  a = _tree.first_child;
  b = search;
  while (a && b) {
    if (a->binding != b->binding) {
      a = a->next_sibling;
    } else {
      if (a->chain == b->chain) {
	if (!a->chain) {
	  return a->id; // found it! (return the actual id, not the search's)
        }
      } else {
        return -2; // the chain status' don't match (conflict!)
      }
      b = b->first_child;
      a = a->first_child;
    }
  }
  return -1; // it just isn't in here
}


bool OBBindings::add(const StringVect &keylist, int id)
{
  BindingTree *tree;

  if (!(tree = buildtree(keylist, id)))
    return false; // invalid binding requested

  if (find(tree) != -1) {
    // conflicts with another binding
    destroytree(tree);
    return false;
  }

  grabKeys(false);
  
  // assimilate this built tree into the main tree
  assimilate(tree); // assimilation destroys/uses the tree

  grabKeys(true); 
 
  return true;
}


int OBBindings::find(const StringVect &keylist)
{
  BindingTree *tree;
  bool ret;

  if (!(tree = buildtree(keylist, 0)))
    return false; // invalid binding requested

  ret = find(tree) >= 0;

  destroytree(tree);

  return ret;
}


int OBBindings::remove(const StringVect &keylist)
{
  (void)keylist;
  assert(false); // XXX: function not implemented yet

  grabKeys(false);
  _curpos = &_tree;

  // do shit here...
  
  grabKeys(true);

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


static void remove_branch(BindingTree *first)
{
  BindingTree *p = first;

  while (p) {
    if (p->first_child)
      remove_branch(p->first_child);
    BindingTree *s = p->next_sibling;
    delete p;
    p = s;
  }
}


void OBBindings::remove_all()
{
  if (_tree.first_child) {
    remove_branch(_tree.first_child);
    _tree.first_child = 0;
  }
}


void OBBindings::grabKeys(bool grab)
{
  for (int i = 0; i < Openbox::instance->screenCount(); ++i) {
    Window root = otk::OBDisplay::screenInfo(i)->rootWindow();

    BindingTree *p = _curpos->first_child;
    while (p) {
      if (grab)
        otk::OBDisplay::grabKey(p->binding.key, p->binding.modifiers,
                                root, false, GrabModeAsync, GrabModeAsync,
                                false);
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


void OBBindings::fire(Window window, unsigned int modifiers, unsigned int key,
                      Time time)
{
  if (key == _resetkey.key && modifiers == _resetkey.modifiers) {
    grabKeys(false);
    _curpos = &_tree;
    grabKeys(true);
  } else {
    BindingTree *p = _curpos->first_child;
    while (p) {
      if (p->binding.key == key && p->binding.modifiers == modifiers) {
        if (p->chain) {
          grabKeys(false);
          _curpos = p;
          grabKeys(true);
        } else {
          python_callback_binding(p->id, window, modifiers, key, time);
          grabKeys(false);
          _curpos = &_tree;
          grabKeys(true);
        }
        break;
      }
      p = p->next_sibling;
    }
  }
}

}
