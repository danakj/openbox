// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// Configuration.hh for Blackbox - an X11 Window manager
// Copyright (c) 2002 - 2002 Ben Jansens (ben@orodu.net)
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

#include "../config.h"

#include "Configuration.hh"
#include "Util.hh"

#include <algorithm>

#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

using std::string;

bool Configuration::m_initialized = False;

Configuration::Configuration(const string &file) {
  setFile(file);
  m_modified = False;
  m_database = NULL;
  m_autosave = True;
  if (! m_initialized) {
    XrmInitialize();
    m_initialized = True;
  }
}

Configuration::Configuration() {
  m_modified = False;
  m_database = NULL;
  m_autosave = True;
  if (! m_initialized) {
    XrmInitialize();
    m_initialized = True;
  }
}

Configuration::~Configuration() {
  if (m_database != NULL)
    XrmDestroyDatabase(m_database);
}

void Configuration::setFile(const string &file) {
  m_file = file;
}

void Configuration::setAutoSave(bool autosave) {
  m_autosave = autosave;
}

void Configuration::save() {
  assert(m_database != NULL);
  XrmPutFileDatabase(m_database, m_file.c_str());
  m_modified = False;
}

bool Configuration::load() {
  if (m_database != NULL)
    XrmDestroyDatabase(m_database);
  m_modified = False;
  if (NULL == (m_database = XrmGetFileDatabase(m_file.c_str())))
    return False;
  return True;
}

void Configuration::create() {
  if (m_database != NULL)
    XrmDestroyDatabase(m_database);
  m_modified = False;
  assert(NULL != (m_database = XrmGetStringDatabase("")));
}

void Configuration::setValue(const string &rname, bool value) {
  assert(m_database != NULL);

  const char *val = (value ? "True" : "False");
  string rc_string = rname + ": " + val;
  XrmPutLineResource(&m_database, rc_string.c_str());

  m_modified = True;
  if (m_autosave)
    save();
}

void Configuration::setValue(const string &rname, unsigned long value) {
  assert(m_database != NULL);
  
  string rc_string = rname + ": " + itostring(value);
  XrmPutLineResource(&m_database, rc_string.c_str());

  m_modified = True;
  if (m_autosave)
    save();
}

void Configuration::setValue(const string &rname, long value) {
  assert(m_database != NULL);
  
  string rc_string = rname + ": " + itostring(value);
  XrmPutLineResource(&m_database, rc_string.c_str());

  m_modified = True;
  if (m_autosave)
    save();
}

void Configuration::setValue(const string &rname, const char *value) {
  assert(m_database != NULL);
  assert(value != NULL);
  
  string rc_string = rname + ": " + value;
  XrmPutLineResource(&m_database, rc_string.c_str());

  m_modified = True;
  if (m_autosave)
    save();
}

void Configuration::setValue(const string &rname, const string &value) {
  assert(m_database != NULL);
  
  string rc_string = rname + ": " + value;
  XrmPutLineResource(&m_database, rc_string.c_str());

  m_modified = True;
  if (m_autosave)
    save();
}

bool Configuration::getValue(const string &rname, bool &value) const {
  assert(m_database != NULL);
  
  string rclass = createClassName(rname);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(m_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return False;
  string val = retvalue.addr;
  if (val == "True" || val == "True")
    value = True;
  else
    value = False;
  return True;
}

bool Configuration::getValue(const string &rname, long &value) const {
  assert(m_database != NULL);
  
  string rclass = createClassName(rname);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(m_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return False;
  char *end;
  value = strtol(retvalue.addr, &end, 10);
  if (end == retvalue.addr)
    return False;
  return True;
}

bool Configuration::getValue(const string &rname, unsigned long &value) const {
  assert(m_database != NULL);
  
  string rclass = createClassName(rname);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(m_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return False;
  char *end;
  value = strtoul(retvalue.addr, &end, 10);
  if (end == retvalue.addr)
    return False;
  return True;
}

bool Configuration::getValue(const string &rname,
                             string &value) const {
  assert(m_database != NULL);
  
  string rclass = createClassName(rname);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(m_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return False;
  value = retvalue.addr;
  return True;
}
  

string Configuration::createClassName(const string &rname) const {
  string rclass(rname);

  string::iterator it = rclass.begin(), end = rclass.end();
  while (True) {
    *it = toUpper(*it);
    ++it;
    if (it == end) break;
    it = std::find(it, rclass.end(), '.');
    if (it == end) break;
    ++it;
    if (it == end) break;
  }
  return rclass;
}
  

char Configuration::toUpper(char c) const {
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 'A';
  return c;
}
