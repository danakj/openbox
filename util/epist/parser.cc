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

using std::string;

parser::parser(keytree *kt, Config *conf)
  : _kt(kt), _config(conf), _mask(0), _action(Action::noaction),
    _key(""), _arg("")
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
    }
  }

  if (!found)
    _action = Action::noaction;
}

void parser::addModifier(string mod)
{
  struct {
    string str;
    unsigned int mask;
  }
  modifiers[] = {
    { "Mod1", Mod1Mask },
    { "Mod2", Mod2Mask },
    { "Mod3", Mod3Mask },
    { "Mod4", Mod4Mask },
    { "Control", ControlMask },
    { "Shift", ShiftMask },
    { "", 0 }
  };

  for (int i = 0; modifiers[i].str != ""; ++i) {
    if (modifiers[i].str == mod)
      _mask |= modifiers[i].mask;
  }
}

void parser::endAction()
{
  _kt->addAction(_action, _mask, _key, _arg);
  reset();
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
  if (_mask != 0 && _key != "") {
    _kt->setCurrentNodeProps(Action::noaction, _mask, _key, "");
    reset();
  }
}

void parser::reset()
{
  _mask = 0;
  _action = Action::noaction;
  _key = "";
  _arg = "";
}
