// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __configuration_hh
#define   __configuration_hh

/*! @file configuration.hh
  @brief Loads, saves, and provides configuration options for the window
         manager
*/

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xresource.h>
}

#include <string>

namespace otk {

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
    return static_cast<const std::string &>(_file);
  }
  void setFile(const std::string &file);

  // defaults to true!
  inline bool autoSave() const {
    return _autosave;
  }
  void setAutoSave(bool);

  inline bool isModified() const {
    return _modified;
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
  
  static bool _initialized;
  std::string _file;
  bool _modified;
  bool _autosave;
  XrmDatabase _database;
};

}

#endif // __configuration_hh
