// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// config.cc for Epistrophy - a key handler for NETWM/EWMH window managers.
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

#include "config.hh"

using std::string;

Config::Config() {}

Config::~Config()
{
  ItemList::const_iterator it = items.begin(), end = items.end();
  for (; it != end; ++it)
    delete *it;
  items.clear();
}


const string &Config::getStringValue(Config::ItemType type) const
{
  ItemList::const_iterator it = items.begin(), end = items.end();
  for (; it != end; ++it) {
    if ((*it)->getType() == type)
      return (*it)->getStringValue();
  }
}


int Config::getNumberValue(Config::ItemType type) const
{
  ItemList::const_iterator it = items.begin(), end = items.end();
  for (; it != end; ++it) {
    if ((*it)->getType() == type)
      return (*it)->getNumberValue();
  }

  return 0;
}


void Config::addOption(ConfigItem *item)
{
  items.push_back(item);    
}


void Config::addOption(const std::string &name, const std::string &value)
{
  const struct {
    const char *name;
    Config::ItemType type;
  }
  options[] = {
    { "notype", Config::noType },
    { "chaintimeout", Config::chainTimeout },
    { "workspacecolumns", Config::workspaceColumns },
    { "", numTypes }
  };

  size_t i = 0;
  while (options[i].type != numTypes) {
    if (strcasecmp(name.c_str(), options[i].name) == 0) {
      ConfigItem *item = new ConfigItem(options[i].type, value);
      items.push_back(item);
      break;
    }
    i++;
  }
}
