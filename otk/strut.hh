// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __strut_hh
#define __strut_hh

/*! @file strut.hh
  @brief The Strut struct defines a margin on 4 sides
*/

namespace otk {

//! Defines a margin on 4 sides
struct Strut {
  //! The margin on the top of the Strut
  unsigned int top;
  //! The margin on the bottom of the Strut
  unsigned int bottom;
  //! The margin on the left of the Strut
  unsigned int left;
  //! The margin on the right of the Strut
  unsigned int right;

  //! Constructs a new Strut with no margins
  Strut(void): top(0), bottom(0), left(0), right(0) {}
};

}

#endif // __strut_hh
