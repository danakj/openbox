// Timer.cc for Openbox
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
#  define _GNU_SOURCE
#endif // _GNU_SOURCE

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "BaseDisplay.h"
#include "Timer.h"

BTimer::BTimer(BaseDisplay &d, TimeoutHandler &h) : display(d), handler(h) {
  once = timing = False;
}

BTimer::~BTimer(void) {
  if (timing) stop();
}

void BTimer::setTimeout(long t) {
  _timeout.tv_sec = t / 1000;
  _timeout.tv_usec = t;
  _timeout.tv_usec -= (_timeout.tv_sec * 1000);
  _timeout.tv_usec *= 1000;
  if (timing) {
    display.removeTimer(this);
    display.addTimer(this);     // reorder the display
  }
}

void BTimer::setTimeout(timeval t) {
  _timeout.tv_sec = t.tv_sec;
  _timeout.tv_usec = t.tv_usec;
}

void BTimer::start(void) {
  gettimeofday(&_start, 0);

  if (! timing) {
    timing = True;
    display.addTimer(this);
  }
}

void BTimer::stop(void) {
  if (timing) {
    timing = False;

    display.removeTimer(this);
  }
}

void BTimer::fireTimeout(void) {
  handler.timeout();
}
