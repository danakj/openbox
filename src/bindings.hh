// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __binding_hh
#define __binding_hh

/*! @file binding.hh
  @brief I dunno.. some binding stuff?
*/

#include <string>
#include <vector>

namespace ob {

typedef struct Binding {
  unsigned int modifiers;
  unsigned int key;

  bool operator==(struct Binding &b2) { return key == b2.key &&
					  modifiers == b2.modifiers; }
  bool operator!=(struct Binding &b2) { return key != b2.key ||
					  modifiers != b2.modifiers; }
  Binding(unsigned int mod, unsigned int k) { modifiers = mod; key = k; }
} Binding;

typedef struct BindingTree {
  Binding binding;
  std::string text;
  int id;     // the id given for the binding in add()
  bool chain; // true if this is a chain to another key (not an action)

  struct BindingTree *next_sibling; // the next binding in the tree at the same
                                    // level
  struct BindingTree *first_child;  // the first child of this binding (next
                                    // binding in a chained sequence).
  BindingTree(int id) : binding(0, 0) {
    this->id = id; chain = true; next_sibling = first_child = 0;
  }
  BindingTree() : binding(0, 0) {
    this->id = -1; chain = true; next_sibling = first_child = 0;
  }
} BindingTree;

class OBBindings {
public:
  //! A list of strings
  typedef std::vector<std::string> StringVect;

private:
  BindingTree _keytree; // root node of the tree (this doesn't have siblings!)
  BindingTree *_curpos; // position in the keytree

  BindingTree _mousetree; // this tree is a list. it has only siblings
  
  int find_key(BindingTree *search);
  bool translate(const std::string &str, Binding &b, bool askey);
  BindingTree *buildtree(const StringVect &keylist, int id);
  void OBBindings::assimilate(BindingTree *node);
 
public:
  //! Initializes an OBBinding object
  OBBindings();
  //! Destroys the OBBinding object
  virtual ~OBBindings();

  //! Adds a new mouse binding
  /*!
    A binding will fail to be added if the binding already exists, or if the
    string is invalid.    
    @return true if the binding could be added; false if it could not.
  */
  bool add_mouse(const std::string &button, int id);

  //! Removes a mouse binding
  /*!
    @return The id of the binding that was removed, or '< 0' if none were
            removed.
  */
  int remove_mouse(const std::string &button);

  //! Adds a new key binding
  /*!
    A binding will fail to be added if the binding already exists (as part of
    a chain or not), or if any of the strings in the keylist are invalid.    
    @return true if the binding could be added; false if it could not.
  */
  bool add_key(const StringVect &keylist, int id);

  //! Removes a key binding
  /*!
    @return The id of the binding that was removed, or '< 0' if none were
            removed.
  */
  int remove_key(const StringVect &keylist);

  //! Removes all key bindings
  void remove_all();

  //! Finds a keybinding and returns its id or '< 0' if it isn't found.
  /*!
    @return -1 if the keybinding was not found but does not conflict with
    any others; -2 if the keybinding conflicts with another.
  */
  int find_key(const StringVect &keylist);

  void process(unsigned int modifiers, unsigned int key);

  // XXX: need an exec() function or something that will be used by openbox
  //      and hold state for which chain we're in etc. (it could have a timer
  //      for reseting too...)

  void display();
};

}

#endif // __binding_hh
