// Timer.h for Openbox
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
  
#ifndef   __Timer_hh
#define   __Timer_hh

#ifdef    TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h> 
#else // !TIME_WITH_SYS_TIME 
#  ifdef    HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else // !HAVE_SYS_TIME_H
#    include <time.h>
#  endif // HAVE_SYS_TIME_H
#endif // TIME_WITH_SYS_TIME

// forward declaration
class BTimer;
class TimeoutHandler;
class BaseDisplay;

class TimeoutHandler {
public:
  virtual void timeout(void) = 0;
};

class BTimer {
  friend class BaseDisplay;
private:
  BaseDisplay &display;
  TimeoutHandler &handler;
  int timing, once;

  timeval _start, _timeout;

protected:
  void fireTimeout(void);

public:
  BTimer(BaseDisplay &, TimeoutHandler &);
  virtual ~BTimer(void);

  inline const int &isTiming(void) const { return timing; } 
  inline const int &doOnce(void) const { return once; }

  inline const timeval &getTimeout(void) const { return _timeout; }
  inline const timeval &getStartTime(void) const { return _start; }

  inline void fireOnce(int o) { once = o; }

  void setTimeout(long);
  void setTimeout(timeval);
  void start(void);
  void stop(void);
};

#endif // __Timer_hh

