// -*- mode: C++; indent-tabs-mode: nil; -*-
// i18n.hh for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
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

#ifndef   __i18n_h
#define   __i18n_h

// always include this just for the #defines
// this keeps the calls to i18n->getMessage clean, otherwise we have to
// add ifdefs to every call to getMessage
#include "../nls/blackbox-nls.hh"

extern "C" {
#ifdef    HAVE_LOCALE_H
#  include <locale.h>
#endif // HAVE_LOCALE_H

#ifdef    HAVE_NL_TYPES_H
#  include <nl_types.h>
#endif // HAVE_NL_TYPES_H
}


class I18n {
private:
  char *locale;
  bool mb;
#ifdef HAVE_NL_TYPES_H
  nl_catd catalog_fd;
#endif

public:
  I18n(void);
  ~I18n(void);

  inline bool multibyte(void) const { return mb; }

  const char* operator()(int set, int msg, const char *msgString) const;
  void openCatalog(const char *catalog);
};

extern I18n i18n;

#endif // __i18n_h
