// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "bindings.hh"
#include "otk/display.hh"

extern "C" {
#include <X11/Xlib.h>
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
    BindingTree *s = p->next_sibling;
    delete p;
    p = s;
  }
}


void OBBindings::display()
{
  if (_bindings.first_child)
    print_branch(_bindings.first_child, "");
}



static bool translate(const std::string str, Binding &b)
{
  std::string::size_type keybegin = str.find_last_of('-');
  std::string key(str, keybegin != std::string::npos ? keybegin + 1 : 0);

  // XXX: get some modifiers up in the hizzie
  
  KeySym sym = XStringToKeysym(const_cast<char *>(key.c_str()));
  if (sym == NoSymbol) return false;
  b.modifiers = Mod1Mask; // XXX: no way
  b.key = XKeysymToKeycode(otk::OBDisplay::display, sym);
  return b.key != 0;
}

static BindingTree *buildtree(const OBBindings::StringVect &keylist, int id)
{
  if (keylist.empty()) return 0; // nothing in the list.. return 0

  BindingTree *ret = new BindingTree(id), *p = 0;

  OBBindings::StringVect::const_iterator it, end = keylist.end();
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

  printf("BUILDING:\n");
  print_branch(ret, "");
  
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


static void assimilate(BindingTree *parent, BindingTree *node)
{
  BindingTree *p, *lastsib, *nextparent, *nextnode = node->first_child;

  if (!parent->first_child) {
    // there are no nodes at this level yet
    parent->first_child = node;
    nextparent = node;
  } else {
    p = lastsib = parent->first_child;

    while (p->next_sibling) {
      p = p->next_sibling;
      lastsib = p; // finds the last sibling
      if (p->binding == node->binding) {
	// found an identical binding..
	assert(node->chain && p->chain);
	delete node; // kill the one we aren't using
	break;
      }
    }
    if (!p) {
      // couldn't find an existing binding, use this new one, and insert it
      // into the list
      p = lastsib->next_sibling = node;
    }
    nextparent = p;
  }

  if (nextnode)
    assimilate(nextparent, nextnode);
}


static int find_bind(BindingTree *tree, BindingTree *search) {
  BindingTree *a, *b;
  a = tree;
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

  if (find_bind(_bindings.first_child, tree) < -1) {
    // conflicts with another binding
    destroytree(tree);
    return false;
  }

  // assimilate this built tree into the main tree
  assimilate(&_bindings, tree); // assimilation destroys/uses the tree
  return true;
}


int OBBindings::find(const StringVect &keylist)
{
  BindingTree *tree;
  bool ret;

  if (!(tree = buildtree(keylist, 0)))
    return false; // invalid binding requested

  ret = find_bind(_bindings.first_child, tree) >= 0;

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
  if (_bindings.first_child)
    remove_branch(_bindings.first_child);
}

}
