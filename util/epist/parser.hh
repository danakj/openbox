// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// parser.hh for Epistrophy - a key handler for NETWM/EWMH window managers.
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

#ifndef __parser_hh
#define __parser_hh

#include "actions.hh"
#include "keytree.hh"
#include "config.hh"

#include <string>

class parser {
public:
  parser(keytree *, Config *);
  ~parser();

  void parse(std::string);

  void setKey(std::string key)
  {  _key = key; }

  void setArgumentNum(std::string arg)
  { _arg = arg; }

  void setArgumentNegNum(std::string arg)
  { _arg = "-" + arg; }

  void setArgumentStr(std::string arg)
  { _arg = arg.substr(1, arg.size() - 2); }

  void setArgumentTrue(std::string)
  { _arg = "true"; }

  void setArgumentFalse(std::string)
  { _arg = "false"; }

  void setOption(std::string opt)
  { _config->addOption(opt, _arg); }

  void setAction(std::string);
  void addModifier(std::string);
  void endAction();
  void startChain();
  void setChainBinding();
  void endChain();

private:
  void reset();

  keytree *_kt;
  Config *_config;
  unsigned int _mask;
  Action::ActionType _action;
  std::string _key;
  std::string _arg;
};

#endif //__parser_hh
