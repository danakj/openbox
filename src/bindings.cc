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
  if (_tree.first_child)
    print_branch(_tree.first_child, "");
}



bool OBBindings::translate(const std::string &str, Binding &b)
{
  unsigned int mods = 0;
  
  // parse out the base key name
  std::string::size_type keybegin = str.find_last_of('-');
  keybegin = (keybegin == std::string::npos) ? 0 : keybegin + 1;
  std::string key(str, keybegin);

  // XXX: get some modifiers up in the hizzie
  // parse out the requested modifier keys
  std::string::size_type begin = 0, end;
  while (begin != keybegin) {
    end = str.find_first_of('-', begin);

    std::string mod(str, begin, end-begin);

    if (mod == "C") {           // control
      mods |= ControlMask;
    } else if (mod == "S") {    // shift
      mods |= ShiftMask;
    } else if (mod == "A" ||    // alt/mod1
               mod == "M" ||
               mod == "M1" ||
               mod == "Mod1") {
      mods |= Mod1Mask;
    } else if (mod == "M2" ||   // mod2
               mod == "Mod2") {
      mods |= Mod2Mask;
    } else if (mod == "M3" ||   // mod3
               mod == "Mod3") {
      mods |= Mod3Mask;
    } else if (mod == "W" ||    // windows/mod4
               mod == "M4" ||
               mod == "Mod4") {
      mods |= Mod4Mask;
    } else if (mod == "M5" ||   // mod5
               mod == "Mod5") {
      mods |= Mod5Mask;
    } else {                    // invalid
      printf(_("Invalid modifier element in key binding: %s\n"), mod.c_str());
      return false;
    }
    begin = end + 1;
  }
  
  KeySym sym = XStringToKeysym(const_cast<char *>(key.c_str()));
  if (sym == NoSymbol) return false;
  b.modifiers = mods;
  b.key = XKeysymToKeycode(otk::OBDisplay::display, sym);
  return b.key != 0;
}

BindingTree *OBBindings::buildtree(const StringVect &keylist, int id)
{
  if (keylist.empty()) return 0; // nothing in the list.. return 0

  BindingTree *ret = new BindingTree(id), *p = 0;

  StringVect::const_iterator it, end = keylist.end();
  for (it = keylist.begin(); it != end; ++it) {
    if (p)
      p = p->first_child = new BindingTree(id);
    else
      p = ret; // the first node
    
    if (!translate(*it, p->binding))
      break;
    p->text = *it;
  }
  if (it != end) {
    // build failed.. clean up and return 0
    p = ret;
    while (p->first_child) {
      BindingTree *c = p->first_child;
      delete p;
      p = c;      
    }
    delete p;
    return 0;
  } else {
    // set the proper chain status on the last node
    p->chain = false;
  }

//  printf("BUILDING:\n");
//  print_branch(ret, "");
  
  // successfully built a tree
  return ret;
}

static void destroytree(BindingTree *tree)
{
  while (tree) {
    BindingTree *c = tree->first_child;
    delete tree;
    tree = c;
  }
}

OBBindings::OBBindings()
{
}


OBBindings::~OBBindings()
{
  remove_all();
}


void OBBindings::assimilate(BindingTree *node)
{
  BindingTree *a, *b, *tmp, *last;

  if (!_tree.first_child) {
    // there are no nodes at this level yet
    _tree.first_child = node;
    return;
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
    else
      last->first_child = b->first_child;
    delete b;
  }
}


int OBBindings::find(BindingTree *search) {
  BindingTree *a, *b;
  a = _tree.first_child;
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

/*
static int find(BindingTree *parent, BindingTree *node) {
  BindingTree *p, *lastsib, *nextparent, *nextnode = node->first_child;

  if (!parent->first_child)
    return -1;

  p = parent->first_child;
  while (p) {
    if (node->binding == p->binding) {
      if (node->chain == p->chain) {
	if (!node->chain) {
	  return p->id; // found it! (return the actual id, not the search's)
	} else {
	  break; // go on to the next child in the chain
	}
      } else {
	return -2; // the chain status' don't match (conflict!)
      }
    }
    p = p->next_sibling;
  }
  if (!p) return -1; // doesn't exist

  if (node->chain) {
    assert(node->first_child);
    return find(p, node->first_child);
  } else
    return -1; // it just isnt in here
}
*/

bool OBBindings::add(const StringVect &keylist, int id)
{
  BindingTree *tree;

  if (!(tree = buildtree(keylist, id)))
    return false; // invalid binding requested

  if (find(tree) < -1) {
    // conflicts with another binding
    destroytree(tree);
    return false;
  }

  // assimilate this built tree into the main tree
  assimilate(tree); // assimilation destroys/uses the tree
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
  if (_tree.first_child)
    remove_branch(_tree.first_child);
}

}
