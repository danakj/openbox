// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// i18n.cc for Openbox
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

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <X11/Xlocale.h>

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef HAVE_STRING_H
#  include <string.h>
#endif // HAVE_STRING_H

#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_LOCALE_H
#  include <locale.h>
#endif // HAVE_LOCALE_H
}

#include <string>
using std::string;

#include "i18n.h"

I18n::I18n(const char *catalog) {
  mb = false;
#ifdef    HAVE_SETLOCALE
  locale = setlocale(LC_ALL, "");
  if (! locale) {
    fprintf(stderr, "failed to set locale, reverting to \"C\"\n");
#endif // HAVE_SETLOCALE
    locale = "C";
#ifdef    HAVE_SETLOCALE
  } else {
    // MB_CUR_MAX returns the size of a char in the current locale
    if (MB_CUR_MAX > 1)
      mb = true;
    // truncate any encoding off the end of the locale
    char *l = strchr(locale, '@');
    if (l) *l = '\0';
    l = strchr(locale, '.');
    if (l) *l = '\0';
  }

#ifdef HAVE_CATOPEN
  catalog_fd = (nl_catd) -1;
#endif
#endif // HAVE_SETLOCALE
  if (catalog)
    openCatalog(catalog);
}


I18n::~I18n() {
#if defined(NLS) && defined(HAVE_CATCLOSE)
  if (catalog_fd != (nl_catd) -1)
    catclose(catalog_fd);
#endif // HAVE_CATCLOSE
}


#if defined(NLS) && defined(HAVE_CATOPEN)
void I18n::openCatalog(const char *catalog) {
  int lp = strlen(LOCALEPATH), lc = strlen(locale),
      ct = strlen(catalog), len = lp + lc + ct + 3;
  catalog_filename = new char[len];

  strncpy(catalog_filename, LOCALEPATH, lp);
  *(catalog_filename + lp) = '/';
  strncpy(catalog_filename + lp + 1, locale, lc);
  *(catalog_filename + lp + lc + 1) = '/';
  strncpy(catalog_filename + lp + lc + 2, catalog, ct + 1);

#  ifdef    MCLoadBySet
  catalog_fd = catopen(catalog_filename.c_str(), MCLoadBySet);
#  else // !MCLoadBySet
  catalog_fd = catopen(catalog_filename.c_str(), NL_CAT_LOCALE);
#  endif // MCLoadBySet

  if (catalog_fd == (nl_catd) -1)
    fprintf(stderr, "failed to open catalog, using default messages\n");
}
#else // !HAVE_CATOPEN
void I18n::openCatalog(const char *) {
  catalog_filename = (char *) 0;
}
#endif // HAVE_CATOPEN


#if   defined(NLS) && defined(HAVE_CATGETS)
const char *I18n::getMessage(int set, int msg, const char *msgString) const {
  if (catalog_fd != (nl_catd) -1)
    return catgets(catalog_fd, set, msg, msgString);
  else
    return msgString;
}
#else
const char *I18n::getMessage(int, int, const char *msgString) const {
  return msgString;
}
#endif
