// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// parser.cc for Epistrophy - a key handler for NETWM/EWMH window managers.
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

#ifdef    HAVE_CONFIG_H
#  include "../../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <stdio.h>
#include <string.h>
}

#include "parser.hh"
#include <string>
#include <iostream>

using std::string;
using std::cout;

parser::parser(keytree *kt, Config *conf)
  : _kt(kt), _config(conf), _mask(0), _action(Action::noaction),
    _key(""), _arg(""), _add(true)
{
}

parser::~parser()
{
  // nothing to see here. move along.
}

void parser::parse(string rc_file)
{
  extern int yyparse(void *);
  extern FILE *yyin;

  yyin = fopen(rc_file.c_str(), "r");

  yyparse(this);

  fclose(yyin);
  _kt->reset();
  _kt->initialize();
}

void parser::setKey(string key)
{ 
  KeySym sym = XStringToKeysym(key.c_str());

  if (sym == 0) {
    std::cerr << "ERROR: Invalid key (" << key << ")! This may cause odd behavior.\n";
    _add = false;
  } else {
    _key = key;
  }
}

void parser::setAction(string act)
{
  struct {
    const char* str;
    Action::ActionType act;
  }
  actions[] = {
    { "noaction", Action::noaction },
    { "execute", Action::execute },
    { "iconify", Action::iconify },
    { "raise", Action::raise },
    { "lower", Action::lower },
    { "close", Action::close },
    { "toggleShade", Action::toggleShade },
    { "toggleOmnipresent", Action::toggleOmnipresent },
    { "movewindowup", Action::moveWindowUp },
    { "movewindowdown", Action::moveWindowDown },
    { "movewindowleft", Action::moveWindowLeft },
    { "movewindowright", Action::moveWindowRight },
    { "resizewindowwidth", Action::resizeWindowWidth },
    { "resizewindowheight", Action::resizeWindowHeight },
    { "togglemaximizefull", Action::toggleMaximizeFull },
    { "togglemaximizevertical", Action::toggleMaximizeVertical },
    { "togglemaximizehorizontal", Action::toggleMaximizeHorizontal },
    { "sendtoworkspace", Action::sendToWorkspace },
    { "nextwindow", Action::nextWindow },
    { "prevwindow", Action::prevWindow },
    { "nextwindowonallworkspaces", Action::nextWindowOnAllWorkspaces },
    { "prevwindowonallworkspaces", Action::prevWindowOnAllWorkspaces },
    { "nextwindowonallscreens", Action::nextWindowOnAllScreens },
    { "prevwindowonallscreens", Action::prevWindowOnAllScreens },
    { "nextwindowofclass", Action::nextWindowOfClass },
    { "prevwindowofclass", Action::prevWindowOfClass },
    { "nextwindowofclassonallworkspaces", Action::nextWindowOfClassOnAllWorkspaces },
    { "prevwindowofclassonallworkspaces", Action::prevWindowOfClassOnAllWorkspaces },
    { "changeworkspace", Action::changeWorkspace },
    { "nextworkspace", Action::nextWorkspace },
    { "prevworkspace", Action::prevWorkspace },
    { "nextworkspacerow", Action::upWorkspace },
    { "prevworkspacerow", Action::downWorkspace },
    { "prevworkspacecolumn", Action::leftWorkspace },
    { "nextworkspacecolumn", Action::rightWorkspace },
    { "nextscreen", Action::nextScreen },
    { "prevscreen", Action::prevScreen },
    { "showrootmenu", Action::showRootMenu },
    { "showworkspacemenu", Action::showWorkspaceMenu },
    { "toggledecorations", Action::toggleDecorations },
    { "togglegrabs", Action::toggleGrabs },
    { "stringchain", Action::stringChain },
    { "keychain", Action::keyChain },
    { "numberchain", Action::numberChain },
    { "cancelchain", Action::cancelChain },
    { "", Action::noaction }
  };

  bool found = false;

  for (int i = 0; actions[i].str != ""; ++i) {
    if ( strcasecmp(actions[i].str, act.c_str()) == 0 ) {
      _action = actions[i].act;
      found = true;
      break;
    }
  }

  if (!found) {
    cout << "ERROR: Invalid action (" << act << "). Binding ignored.\n";
    _add = false;
  }
}

void parser::addModifier(string mod)
{
  struct {
    const char *str;
    unsigned int mask;
  }
  modifiers[] = {
    { "mod1", Mod1Mask },
    { "mod2", Mod2Mask },
    { "mod3", Mod3Mask },
    { "mod4", Mod4Mask },
    { "mod5", Mod5Mask },
    { "control", ControlMask },
    { "shift", ShiftMask },
    { "", 0 }
  };

  bool found = false;

  for (int i = 0; modifiers[i].str != ""; ++i) {
    if ( strcasecmp(modifiers[i].str, mod.c_str()) == 0 ) {
      _mask |= modifiers[i].mask;
      found = true;
      break;
    }
  }

  if (!found) {
    cout << "ERROR: Invalid modifier (" << mod << "). Binding ignored.\n";
    _add = false;
  }
}

void parser::endAction()
{
  if (_add)
    _kt->addAction(_action, _mask, _key, _arg);
  reset();

  _add = true;
}

void parser::startChain()
{
  _kt->advanceOnNewNode();
  setChainBinding();
  reset();
}

void parser::endChain()
{
  _kt->retract();
  reset();
}

void parser::setChainBinding()
{
  if (_add)
    _kt->setCurrentNodeProps(Action::noaction, _mask, _key, "");
  
  _add = true;
  reset();
}

void parser::reset()
{
  _mask = 0;
  _action = Action::noaction;
  _key = "";
  _arg = "";
}
