// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// config.hh for Epistrophy - a key handler for NETWM/EWMH window managers.
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

#ifndef __config_hh
#define __config_hh

#include <string>
#include <list>

// forward declarations
struct BoolItem;
struct StringItem;
struct NumberItem;

class Config {
public:
  enum BoolType {
    NO_BOOL_TYPE,
    stackedCycling,
    NUM_BOOL_TYPES
  };

  enum StringType {
    NO_STRING_TYPE,
    NUM_STRING_TYPES
  };

  enum NumberType {
    NO_NUMBER_TYPE,
    chainTimeout,
    workspaceColumns,
    NUM_NUMBER_TYPES
  };

private:
  typedef std::list<BoolItem *> BoolItemList;
  typedef std::list<StringItem *> StringItemList;
  typedef std::list<NumberItem *> NumberItemList;
  BoolItemList bool_items;
  StringItemList string_items;
  NumberItemList number_items;

public:
  Config();
  ~Config();

  bool getValue(BoolType, bool &) const;
  bool getValue(StringType, std::string &) const;
  bool getValue(NumberType, int &) const;

  void addOption(const std::string &, const std::string &);
};

struct BoolItem {
  Config::BoolType type;
  bool value;
};

struct StringItem {
  Config::StringType type;
  std::string value;
};

struct NumberItem {
  Config::NumberType type;
  int value;
};

#endif // __config_hh
