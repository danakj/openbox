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

class ConfigItem;

class Config {
public:
  enum ItemType {
    noType,
    chainTimeout,
    workspaceColumns,
    numTypes
  };

private:
  typedef std::list<ConfigItem *> ItemList;
  ItemList items;

public:
  Config();
  ~Config();

  bool getStringValue(Config::ItemType, std::string &) const;
  int getNumberValue(Config::ItemType) const;
  void addOption(ConfigItem *);
  void addOption(const std::string &, const std::string &);
};


class ConfigItem {
private:
  Config::ItemType _type;
  std::string _value;

public:
  ConfigItem(Config::ItemType type, std::string value)
    : _type(type), _value(value) {}
  ~ConfigItem() {}

  inline const std::string &getStringValue() const
  { return _value; }

  inline int getNumberValue() const
  { return atoi(_value.c_str()); }

  inline Config::ItemType getType() const
  { return _type; }
};

#endif // __config_hh
