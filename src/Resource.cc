// Resource.cc for Openbox
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

#include "Resource.h"

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#include <assert.h>

bool obResource::m_initialized = false;

obResource::obResource(const std::string &file) {
  setFile(file);
  m_modified = false;
  m_database = NULL;
  m_autosave = true;
  if (!m_initialized) {
    XrmInitialize();
    m_initialized = true;
  }
}

obResource::obResource() {
  m_modified = false;
  m_database = NULL;
  m_autosave = true;
  if (!m_initialized) {
    XrmInitialize();
    m_initialized = true;
  }
}

obResource::~obResource() {
  if (m_database != NULL)
    XrmDestroyDatabase(m_database);
}

void obResource::setFile(const std::string &file) {
  m_file = file;
}

void obResource::setAutoSave(bool autosave) {
  m_autosave = autosave;
}

void obResource::save() {
  assert(m_database != NULL);
  XrmPutFileDatabase(m_database, m_file.c_str());
  m_modified = false;
}

bool obResource::load() {
  if (m_database != NULL)
    XrmDestroyDatabase(m_database);
  m_modified = false;
  if (NULL == (m_database = XrmGetFileDatabase(m_file.c_str())))
    return false;
  return true;
}

void obResource::setValue(const std::string &rname, bool value) {
  assert(m_database != NULL);

  const char *val = (value ? "True" : "False");
  std::string rc_string = rname + ": " + val;
  XrmPutLineResource(&m_database, rc_string.c_str());

  m_modified = true;
  if (m_autosave)
    save();
}

void obResource::setValue(const std::string &rname, int value) {
  setValue(rname, (long)value);
}

void obResource::setValue(const std::string &rname, long value) {
  assert(m_database != NULL);
  
  char val[11];
  sprintf(val, "%ld", value);
  std::string rc_string = rname + ": " + val;
  XrmPutLineResource(&m_database, rc_string.c_str());

  m_modified = true;
  if (m_autosave)
    save();
}

void obResource::setValue(const std::string &rname, const char *value) {
  assert(m_database != NULL);
  
  std::string rc_string = rname + ": " + value;
  XrmPutLineResource(&m_database, rc_string.c_str());

  m_modified = true;
  if (m_autosave)
    save();
}

void obResource::setValue(const std::string &rname, const std::string &value) {
  assert(m_database != NULL);
  
  std::string rc_string = rname + ": " + value;
  XrmPutLineResource(&m_database, rc_string.c_str());

  m_modified = true;
  if (m_autosave)
    save();
}

bool obResource::getValue(const std::string &rname, const std::string &rclass,
                          bool &value) const {
  assert(rclass.c_str() != NULL);
  assert(m_database != NULL);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(m_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return false;
  std::string val = retvalue.addr;
  if (0 == strncasecmp(val.c_str(), "true", val.length()))
    value = true;
  else
    value = false;
  return true;
}

bool obResource::getValue(const std::string &rname, const std::string &rclass,
                          long &value) const {
  assert(m_database != NULL);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(m_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return false;
  char *end;
  value = strtol(retvalue.addr, &end, 10);
  if (end == retvalue.addr)
    return false;
  return true;
}

bool obResource::getValue(const std::string &rname, const std::string &rclass,
                          std::string &value) const {
  assert(m_database != NULL);
  
  char *rettype;
  XrmValue retvalue;
  if (0 == XrmGetResource(m_database, rname.c_str(), rclass.c_str(), 
                          &rettype, &retvalue) || retvalue.addr == NULL)
    return false;
  value = retvalue.addr;
  return true;
}
