// Resource.h for Openbox
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

#ifndef   __Resource_hh
#define   __Resource_hh

#include <string>

#include <X11/Xlib.h>
#include <X11/Xresource.h>

class obResource {
public:
  obResource(const std::string &file);
  obResource();
  virtual ~obResource();

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

  void setValue(const std::string &rname, bool value);
  void setValue(const std::string &rname, int value);
  void setValue(const std::string &rname, long value);
  void setValue(const std::string &rname, const std::string &value);
  void setValue(const std::string &rname, const char *value);

  bool getValue(const std::string &rname, const std::string &rclass,
                bool &value) const;
  bool getValue(const std::string &rname, const std::string &rclass,
                long &value) const;
  bool getValue(const std::string &rname, const std::string &rclass,
                std::string &value) const;

private:
  std::string m_file;
  bool m_modified;
  bool m_autosave;
  XrmDatabase m_database;
};

#endif // __Resource_hh
