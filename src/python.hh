// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __python_hh
#define   __python_hh

/*! @file python.hh
  @brief Python stuff
*/

#include <python2.2/Python.h>

namespace ob {

extern "C" {

void initopenbox();

}
}

#endif // __python_hh
