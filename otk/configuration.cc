// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H
}

#include "configuration.hh"
#include "util.hh"

#include <algorithm>

namespace otk {

bool Configuration::_initialized = False;

Configuration::Configuration(const std::string &file, bool autosave) {
  setFile(file);
  _modified = False;
  _database = NULL;
  _autosave = autosave;
  if (! _initialized) {
    XrmInitialize();
    _initialized = True;
  }
}

Configuration::Configuration(bool autosave) {
  _modified = False;
  _database = NULL;
  _autosave = autosave;
  if (! _initialized) {
    XrmInitialize();
    _initialized = True;
  }
}

Configuration::~Configuration() {
  if (_database != NULL)
    XrmDestroyDatabase(_database);
}

void Configuration::setFile(const std::string &file) {
  _file = file;
}

void Configuration::setAutoSave(bool autosave) {
  _autosave = autosave;
}

void Configuration::save() {
  assert(_database != NULL);
  XrmPutFileDatabase(_database, _file.c_str());
  _modified = False;
}

bool Configuration::load() {
  if (_database != NULL)
    XrmDestroyDatabase(_database);
  _modified = False;
  if (NULL == (_database = XrmGetFileDatabase(_file.c_str())))
    return False;
  return True;
}

bool Configuration::merge(const std::string &file, bool overwrite) {
  if (XrmCombineFileDatabase(file.c_str(), &_database, overwrite) == 0)
    return False;
  _modified = True;
  if (_autosave)
    save();
  return True;
}

void Configuration::create() {
  if (_database != NULL)
    XrmDestroyDatabase(_database);
  _modified = False;
  assert(NULL != (_database = XrmGetStringDatabase("")));
}

void Configuration::setValue(const std::string &rname, bool value) {
  assert(_database != NULL);

  const char *val = (value ? "True" : "False");
  std::string rc_string = rname + ": " + val;
  XrmPutLineResource(&_database, rc_string.c_str());

  _modified = True;
  if (_autosave)
    save();
}

void Configuration::setValue(const std::string &rname, unsigned long value) {
  assert(_database != NULL);
  
  std::string rc_string = rname + ": " + itostring(value);
  XrmPutLineResource(&_database, rc_string.c_str());

  _modified = True;
  if (_autosave)
    save();
}

void Configuration::setValue(const std::string &rname, long value) {
  assert(_database != NULL);
  
  std::string rc_string = rname + ": " + itostring(value);
  XrmPutLineResource(&_database, rc_string.c_str());

  _modified = True;
  if (_autosave)
    save();
}

void Configuration::setValue(const std::string &rname, const char *value) {
  assert(_database != NULL);
  assert(value != NULL);
  
  std::string rc_string = rname + ": " + value;
  XrmPutLineResource(&_database, rc_string.c_str());

  _modified = True;
  if (_autosave)
    save();
}

void Configuration::setValue(const std::string &rname,
                             const std::string &value) {
  assert(_database != NULL);
  
  std::string rc_string = rname + ": " + value;
  XrmPutLineResource(&_database, rc_string.c_str());

  _modified = True;
  if (_autosave)
    save();
}

bool Configuration::getValue(const std::string &rname, bool &value) const {
  assert(_database != NULL);
  
  std::string rclass = createClassName(rname);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return False;
  std::string val = retvalue.addr;
  if (val == "True" || val == "True")
    value = True;
  else
    value = False;
  return True;
}

bool Configuration::getValue(const std::string &rname, long &value) const {
  assert(_database != NULL);
  
  std::string rclass = createClassName(rname);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return False;
  char *end;
  value = strtol(retvalue.addr, &end, 10);
  if (end == retvalue.addr)
    return False;
  return True;
}

bool Configuration::getValue(const std::string &rname, unsigned long &value) const {
  assert(_database != NULL);
  
  std::string rclass = createClassName(rname);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return False;
  char *end;
  value = strtoul(retvalue.addr, &end, 10);
  if (end == retvalue.addr)
    return False;
  return True;
}

bool Configuration::getValue(const std::string &rname,
                             std::string &value) const {
  assert(_database != NULL);
  
  std::string rclass = createClassName(rname);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return False;
  value = retvalue.addr;
  return True;
}
  

std::string Configuration::createClassName(const std::string &rname) const {
  std::string rclass(rname);

  std::string::iterator it = rclass.begin(), end = rclass.end();
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

}
