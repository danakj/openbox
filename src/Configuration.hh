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

#ifndef   __Configuration_hh
#define   __Configuration_hh

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <string>

/*
 * The Configuration class is a generic wrapper for configuration settings.
 *
 * This class is used for the global rc/config file, and for styles.
 *
 * This implementation of the Configuration class wraps an X resource database
 * file.
 */
class Configuration {
public:
  explicit Configuration(const std::string &file, bool autosave = True);
  Configuration(bool autosave = True);
  virtual ~Configuration();

  inline const std::string &file() const {
    return static_cast<const std::string &>(m_file);
  }
  void setFile(const std::string &file);

  // defaults to true!
  inline bool autoSave() const {
    return m_autosave;
  }
  void setAutoSave(bool);

  inline bool isModified() const {
    return m_modified;
  }

  void save();
  bool load();
  bool merge(const std::string &file, bool overwrite = False);
  void create();

  void setValue(const std::string &rname, bool value);
  inline void setValue(const std::string &rname, int value) {
    setValue(rname, (long) value);
  }
  inline void setValue(const std::string &rname, unsigned int value) {
    setValue(rname, (unsigned long) value);
  }
  void setValue(const std::string &rname, long value);
  void setValue(const std::string &rname, unsigned long value);
  void setValue(const std::string &rname, const std::string &value);
  void setValue(const std::string &rname, const char *value);

  bool getValue(const std::string &rname, bool &value) const;
  inline bool getValue(const std::string &rname, int &value) const {
    return getValue(rname, (long) value);
  }
  inline bool getValue(const std::string &rname, unsigned int &value) const {
    return getValue(rname, (unsigned long) value);
  }
  bool getValue(const std::string &rname, long &value) const;
  bool getValue(const std::string &rname, unsigned long &value) const;
  bool getValue(const std::string &rname, std::string &value) const;

private:
  std::string createClassName(const std::string &rname) const;
  char toUpper(char) const;
  
  static bool m_initialized;
  std::string m_file;
  bool m_modified;
  bool m_autosave;
  XrmDatabase m_database;
};

#endif // __Configuration_hh
