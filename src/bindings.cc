// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "bindings.hh"
#include "otk/display.hh"

extern "C" {
#include <X11/Xlib.h>

#include "gettext.h"
#define _(str) gettext(str)
}

namespace ob {

#include <stdio.h>
static void print_branch(BindingTree *first, std::string str)
{
  BindingTree *p = first;
  
  while (p) {
    if (p->first_child)
      print_branch(p->first_child, str + " " + p->text);
    if (!p->chain)
      printf("%d%s\n", p->id, (str + " " + p->text).c_str());
    p = p->next_sibling;
  }
}


void OBBindings::display()
{
  if (_keytree.first_child) {
    printf("Key Tree:\n");
    print_branch(_keytree.first_child, "");
  }
  if (_mousetree) {
    printf("Mouse Tree:\n");
    BindingTree *p = _mousetree;
    while (p) {
      printf("%d %s\n", p->id, p->text.c_str());
      p = p->next_sibling;
    }
  }
}


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

bool OBBindings::translate(const std::string &str, Binding &b, bool askey)
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
//      printf(_("Invalid modifier element in key binding: %s\n"), mod.c_str());
      return false;
    }
    
    begin = end + 1;
  }

  // set the binding
  b.modifiers = modval;
  if (askey) {
    KeySym sym = XStringToKeysym(const_cast<char *>(key.c_str()));
    if (sym == NoSymbol) return false;
    b.key = XKeysymToKeycode(otk::OBDisplay::display, sym);
    return b.key != 0;
  } else {
    return buttonvalue(key, &b.key);
  }
}

static void destroytree(BindingTree *tree)
{
  while (tree) {
    BindingTree *c = tree->first_child;
    delete tree;
    tree = c;
  }
}

BindingTree *OBBindings::buildtree(const StringVect &keylist, int id)
{
  if (keylist.empty()) return 0; // nothing in the list.. return 0

  BindingTree *ret = 0, *p;

  StringVect::const_reverse_iterator it, end = keylist.rend();
  for (it = keylist.rbegin(); it != end; ++it) {
    p = ret;
    ret = new BindingTree(id);
    if (!p) ret->chain = false;
    ret->first_child = p;
    if (!translate(*it, ret->binding, true)) {
      destroytree(ret);
      ret = 0;
      break;
    }
    ret->text = *it; // XXX: rm me
  }
  return ret;
}


OBBindings::OBBindings()
  : _curpos(&_keytree), _mousetree(0)
{
}


OBBindings::~OBBindings()
{
  remove_all();
}


bool OBBindings::add_mouse(const std::string &button, int id)
{
  BindingTree n;

  if (!translate(button, n.binding, false))
    return false;

  BindingTree *p = _mousetree, **newp = &_mousetree;
  while (p) {
    if (p->binding == n.binding)
      return false; // conflict
    p = p->next_sibling;
    newp = &p->next_sibling;
  }
  display();
  *newp = new BindingTree(id);
  display();
  (*newp)->text = button;
  (*newp)->chain = false;
  (*newp)->binding.key = n.binding.key;
  (*newp)->binding.modifiers = n.binding.modifiers;
 
  return true;
}


int OBBindings::remove_mouse(const std::string &button)
{
  (void)button;
  assert(false); // XXX: function not implemented yet
}


void OBBindings::assimilate(BindingTree *node)
{
  BindingTree *a, *b, *tmp, *last;

  printf("node=%lx\n", (long)node);
  if (!_keytree.first_child) {
    // there are no nodes at this level yet
    _keytree.first_child = node;
    return;
  } else {
    a = _keytree.first_child;
    last = a;
    b = node;
    while (a) {
  printf("in while.. b=%lx\n", (long)b);
      last = a;
      if (a->binding != b->binding) {
        a = a->next_sibling;
      } else {
        printf("a: %s %d %d\n", a->text.c_str(), a->binding.key, a->binding.modifiers);
        printf("b: %s %d %d\n", b->text.c_str(), b->binding.key, b->binding.modifiers);
        printf("moving up one in b\n");
        tmp = b;
        b = b->first_child;
        delete tmp;
        a = a->first_child;
      }
    }
  printf("after while.. b=%lx\n", (long)b);
    if (last->binding != b->binding)
      last->next_sibling = b;
    else
      last->first_child = b->first_child;
    delete b;
  }
}


int OBBindings::find_key(BindingTree *search) {
  BindingTree *a, *b;
  a = _keytree.first_child;
  b = search;
  while (a && b) {
    if (a->binding != b->binding) {
      a = a->next_sibling;
    } else {
      if (a->chain == b->chain) {
	if (!a->chain)
	  return a->id; // found it! (return the actual id, not the search's)
      } else
	  return -2; // the chain status' don't match (conflict!)
      b = b->first_child;
      a = a->first_child;
    }
  }
  return -1; // it just isn't in here
}

bool OBBindings::add_key(const StringVect &keylist, int id)
{
  BindingTree *tree;

  if (!(tree = buildtree(keylist, id)))
    return false; // invalid binding requested

  if (find_key(tree) != -1) {
    // conflicts with another binding
    destroytree(tree);
    return false;
  }

  // assimilate this built tree into the main tree
  assimilate(tree); // assimilation destroys/uses the tree
  return true;
}


int OBBindings::find_key(const StringVect &keylist)
{
  BindingTree *tree;
  bool ret;

  if (!(tree = buildtree(keylist, 0)))
    return false; // invalid binding requested

  ret = find_key(tree) >= 0;

  destroytree(tree);

  return ret;
}


int OBBindings::remove_key(const StringVect &keylist)
{
  (void)keylist;
  assert(false); // XXX: function not implemented yet
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
  if (_keytree.first_child) {
    remove_branch(_keytree.first_child);
    _keytree.first_child = 0;
  }
  BindingTree *p = _mousetree;
  while (p) {
    BindingTree *n = p->next_sibling;
    delete p;
    p = n;
  }
  _mousetree = 0;
}


void OBBindings::process(unsigned int modifiers, unsigned int key)
{
  BindingTree *c = _curpos->first_child;

  while (c) {
    if (c->binding.key == key && c->binding.modifiers == modifiers) {
      _curpos = c;
      break;
    }
  }
  if (c) {
    if (!_curpos->chain) {
      // XXX execute command for _curpos->id
      _curpos = &_keytree; // back to the start
    }
  }
}

}
