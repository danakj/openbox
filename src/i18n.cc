// i18n.cc for Openbox
// Copyright (c) 2001 Sean 'Shaleh' Perry <shaleh@debian.org>
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

// stupid macros needed to access some functions in version 2 of the GNU C
// library
#ifndef   _GNU_SOURCE
#define   _GNU_SOURCE
#endif // _GNU_SOURCE

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "i18n.h"

#include <X11/Xlocale.h>

#ifdef    STDC_HEADERS
#  include <stdlib.h>
#  include <string.h>
#  include <stdio.h>
#endif // STDC_HEADERS

#ifdef    HAVE_LOCALE_H
#  include <locale.h>
#endif // HAVE_LOCALE_H

// the rest of bb source uses True and False from X, so we continue that
#define True true
#define False false

static I18n static_i18n;
I18n *i18n;

void NLSInit(const char *catalog) {
  i18n = &static_i18n;

  i18n->openCatalog(catalog);
}


I18n::I18n(void) {
  mb = False;
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
      mb = True;
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

  catalog_filename = (char *) 0;
}


I18n::~I18n(void) {
  delete [] catalog_filename;

#if defined(NLS) && defined(HAVE_CATCLOSE)
  if (catalog_fd != (nl_catd) -1)
    catclose(catalog_fd);
#endif // HAVE_CATCLOSE
}


void I18n::openCatalog(const char *catalog) {
#if defined(NLS) && defined(HAVE_CATOPEN)
  int lp = strlen(LOCALEPATH), lc = strlen(locale),
      ct = strlen(catalog), len = lp + lc + ct + 3;
  catalog_filename = new char[len];

  strncpy(catalog_filename, LOCALEPATH, lp);
  *(catalog_filename + lp) = '/';
  strncpy(catalog_filename + lp + 1, locale, lc);
  *(catalog_filename + lp + lc + 1) = '/';
  strncpy(catalog_filename + lp + lc + 2, catalog, ct + 1);

#  ifdef    MCLoadBySet
  catalog_fd = catopen(catalog_filename, MCLoadBySet);
#  else // !MCLoadBySet
  catalog_fd = catopen(catalog_filename, NL_CAT_LOCALE);
#  endif // MCLoadBySet

  if (catalog_fd == (nl_catd) -1)
    fprintf(stderr, "failed to open catalog, using default messages\n");
#else // !HAVE_CATOPEN

  catalog_filename = (char *) 0;
#endif // HAVE_CATOPEN
}


const char *I18n::getMessage(int set, int msg, const char *msgString) const {
#if   defined(NLS) && defined(HAVE_CATGETS)
  if (catalog_fd != (nl_catd) -1)
    return (const char *) catgets(catalog_fd, set, msg, msgString);
  else
#endif
    return msgString;
}
