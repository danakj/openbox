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
  // deallocate memory for the 3 lists
  BoolItemList::const_iterator b_it, b_end = bool_items.end();
  for (b_it = bool_items.begin(); b_it != b_end; ++b_it)
    delete *b_it;
  bool_items.clear();

  NumberItemList::const_iterator n_it, n_end = number_items.end();
  for (n_it = number_items.begin(); n_it != n_end; ++n_it)
    delete *n_it;
  number_items.clear();

  StringItemList::const_iterator s_it, s_end = string_items.end();
  for (s_it = string_items.begin(); s_it != s_end; ++s_it)
    delete *s_it;
  string_items.clear();
}


bool Config::getValue(Config::StringType type, string &ret) const
{
  StringItemList::const_iterator it = string_items.begin(), end = string_items.end();
  for (; it != end; ++it) {
    if ((*it)->type == type) {
      ret = (*it)->value;
      return true;
    }
  }
  return false;
}


bool Config::getValue(Config::NumberType type, int &ret) const
{
  NumberItemList::const_iterator it = number_items.begin(), end = number_items.end();
  for (; it != end; ++it) {
    if ((*it)->type == type) {
      ret = (*it)->value;
      return true;
    }
  }
  return false;
}


bool Config::getValue(Config::BoolType type, bool &ret) const
{
  BoolItemList::const_iterator it = bool_items.begin(), end = bool_items.end();
  for (; it != end; ++it) {
    if ((*it)->type == type) {
      ret = (*it)->value;
      return true;
    }
  }
  return false;
}


void Config::addOption(const std::string &name, const std::string &value)
{
  const struct {
    const char *name;
    Config::BoolType type;
  }
  bool_options[] = {
    { "stackedcycling", Config::stackedCycling },
    { "", NUM_BOOL_TYPES }
  };

  const struct {
    const char *name;
    Config::StringType type;
  }
  string_options[] = {
    { "", NUM_STRING_TYPES }
  };

  const struct {
    const char *name;
    Config::NumberType type;
  }
  number_options[] = {
    { "chaintimeout", chainTimeout },
    { "workspacecolumns", workspaceColumns },
    { "", NUM_NUMBER_TYPES }
  };

  // if it's bool option, add it to the bool_items list
  size_t i = 0;
  while (bool_options[i].type != NUM_BOOL_TYPES) {
    if (strcasecmp(name.c_str(), bool_options[i].name) == 0) {
      BoolItem *item = new BoolItem;
      const char *tmp = value.c_str();

      item->type = bool_options[i].type;

      if (strcasecmp(tmp, "true") == 0 || strcasecmp(tmp, "1") == 0 ||
          strcasecmp(tmp, "on") == 0)
        item->value = true;
      else
        item->value = false;

      bool_items.push_back(item);
      return;
    }
    i++;
  }

  // if it's a string, add it to the string_items list
  i = 0;
  while (string_options[i].type != NUM_STRING_TYPES) {
    if (strcasecmp(name.c_str(), string_options[i].name) == 0) {
      StringItem *item = new StringItem;
      item->type = string_options[i].type;
      item->value = value;

      string_items.push_back(item);
      return;
    }
    i++;
  }

  // if it's a number, add it to the number_items list
  i = 0;
  while (number_options[i].type != NUM_NUMBER_TYPES) {
    if (strcasecmp(name.c_str(), number_options[i].name) == 0) {
      NumberItem *item = new NumberItem;
      item->type = number_options[i].type;
      item->value = atoi( value.c_str() );

      number_items.push_back(item);
      return;
    }
    i++;
  }
}
