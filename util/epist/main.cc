// -*- mode: C++; indent-tabs-mode: nil; -*-
// main.cc for Epistory - a key handler for NETWM/EWMH window managers.
// Copyright (c) 2002 - 2002 Ben Jansens <ben at orodu.net>
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
#  include "../../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef    HAVE_SIGNAL_H
#  include <signal.h>
#endif // HAVE_SIGNAL_H

#ifdef    HAVE_SYS_SIGNAL_H
#  include <sys/signal.h>
#endif // HAVE_SYS_SIGNAL_H

#ifdef    HAVE_LIBGEN_H
#  include <libgen.h>
#endif // HAVE_LIBGEN_H
}

#include <iostream>

using std::cout;
using std::endl;

bool _shutdown = false;
char **_argv;

#ifdef   HAVE_SIGACTION
static void signalhandler(int sig)
#else //  HAVE_SIGACTION
static RETSIGTYPE signalhandler(int sig)
#endif // HAVE_SIGACTION
{
  switch (sig) {
  case SIGSEGV:
    cout << "Segmentation fault. Aborting and dumping core.\n";
    abort();
  case SIGHUP:
    cout << "Restarting on request.\n";
    execvp(_argv[0], _argv);
    execvp(basename(_argv[0]), _argv);
  }
  _shutdown = true;

#ifndef   HAVE_SIGACTION
  // assume broken, braindead sysv signal semantics
  signal(sig, (RETSIGTYPE (*)(int)) signalhandler);
#endif // HAVE_SIGACTION
}

int main(int, char **argv) {
  _argv = argv;

#ifdef    HAVE_SIGACTION
  struct sigaction action;

  action.sa_handler = signalhandler;
  action.sa_mask = sigset_t();
  action.sa_flags = SA_NOCLDSTOP | SA_NODEFER;

  sigaction(SIGPIPE, &action, NULL);
  sigaction(SIGSEGV, &action, NULL);
  sigaction(SIGFPE, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGHUP, &action, NULL);
#else // !HAVE_SIGACTION
  signal(SIGPIPE, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGSEGV, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGFPE, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGTERM, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGINT, (RETSIGTYPE (*)(int)) signalhandler);
  signal(SIGHUP, (RETSIGTYPE (*)(int)) signalhandler);
#endif // HAVE_SIGACTION
  
  while (! _shutdown) {
    usleep(500);
  }
  return 0;
}
