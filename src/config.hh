// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __config_hh
#define   __config_hh

/*! @file config.hh
  @brief The Config class contains configuration options set by the user's
         scripts
*/

#include "otk/ustring.hh"

#include <vector>

namespace ob {

class Config {
  int _screen;

public:
  std::vector<otk::ustring> desktop_names;
  otk::ustring theme;
  otk::ustring titlebar_layout;
  long double_click_delay;
  long drag_threshold;
  long num_desktops;

  Config(int screen);
  ~Config();

  void load();
};

}

#endif // __config_hh
