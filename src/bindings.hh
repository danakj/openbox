// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __binding_hh
#define __binding_hh

/*! @file bindings.hh
  @brief I dunno.. some binding stuff?
*/

#include "actions.hh"
#include "python.hh"
#include "otk/timer.hh"

extern "C" {
#include <Python.h>
}

#include <string>
#include <list>
#include <vector>

namespace ob {

class OBClient;

typedef struct Binding {
  unsigned int modifiers;
  unsigned int key;

  bool operator==(struct Binding &b2) { return key == b2.key &&
					  modifiers == b2.modifiers; }
  bool operator!=(struct Binding &b2) { return key != b2.key ||
					  modifiers != b2.modifiers; }
  Binding(unsigned int mod, unsigned int k) { modifiers = mod; key = k; }
} Binding;

typedef struct KeyBindingTree {
  Binding binding;
  PyObject *callback; // the callback given for the binding in add()
  bool chain;     // true if this is a chain to another key (not an action)

  struct KeyBindingTree *next_sibling; // the next binding in the tree at the same
                                    // level
  struct KeyBindingTree *first_child;  // the first child of this binding (next
                                    // binding in a chained sequence).
  KeyBindingTree(PyObject *callback) : binding(0, 0) {
    this->callback = callback; chain = true; next_sibling = first_child = 0;
  }
  KeyBindingTree() : binding(0, 0) {
    this->callback = 0; chain = true; next_sibling = first_child = 0;
  }
} KeyBindingTree;

typedef struct ButtonBinding {
  Binding binding;
  PyObject *callback[NUM_MOUSE_ACTION];
  ButtonBinding() : binding(0, 0) {
    for(int i=0; i<NUM_MOUSE_ACTION; ++i) callback[i] = 0;
  }
};

class OBBindings {
public:
  //! A list of strings
  typedef std::vector<std::string> StringVect;

private:
  // root node of the tree (this doesn't have siblings!)
  KeyBindingTree _keytree; 
  KeyBindingTree *_curpos; // position in the keytree

  Binding _resetkey; // the key which resets the key chain status

  otk::OBTimer _timer;
  
  PyObject *find(KeyBindingTree *search, bool *conflict) const;
  KeyBindingTree *buildtree(const StringVect &keylist,
                            PyObject *callback) const;
  void assimilate(KeyBindingTree *node);

  static void resetChains(OBBindings *self); // the timer's timeout function

  typedef std::list <ButtonBinding*> ButtonBindingList;
  ButtonBindingList _buttons[NUM_MOUSE_CONTEXT];

  void grabButton(bool grab, const Binding &b, MouseContext context,
                  OBClient *client);
  
public:
  //! Initializes an OBBindings object
  OBBindings();
  //! Destroys the OBBindings object
  virtual ~OBBindings();

  //! Translates a binding string into the actual Binding
  bool translate(const std::string &str, Binding &b, bool askey = true) const;
  
  //! Adds a new key binding
  /*!
    A binding will fail to be added if the binding already exists (as part of
    a chain or not), or if any of the strings in the keylist are invalid.    
    @return true if the binding could be added; false if it could not.
  */
  bool addKey(const StringVect &keylist, PyObject *callback);

  //! Removes a key binding
  /*!
    @return The callbackid of the binding, or '< 0' if there was no binding to
            be removed.
  */
  bool removeKey(const StringVect &keylist);

  //! Removes all key bindings
  void removeAllKeys();

  void fireKey(unsigned int modifiers,unsigned int key, Time time);

  void setResetKey(const std::string &key);

  void grabKeys(bool grab);

  bool addButton(const std::string &but, MouseContext context,
                 MouseAction action, PyObject *callback);

  void grabButtons(bool grab, OBClient *client);

  //! Removes all button bindings
  void removeAllButtons();

  void fireButton(ButtonData *data);
};

}

#endif // __binding_hh
