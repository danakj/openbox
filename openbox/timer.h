#ifndef __timer_h
#define __timer_h

#include <glib.h>

/*! Data type of Timer callback */
typedef void (*TimeoutHandler)(void *data);

typedef struct Timer {
    /*! Milliseconds between timer firings */
    long delay;
    /*! Callback for timer expiry */
    TimeoutHandler action;
    /*! Data sent to callback */
    void *data;
    /*! We overload the delete operator to just set this to true */
    gboolean del_me;
    /*! The time the last fire should've been at */
    GTimeVal last;
    /*! When this timer will next trigger */
    GTimeVal timeout;
} Timer;

/*! Initializes the timer subsection */
void timer_startup();
/*! Destroys the timer subsection */
void timer_shutdown();

/* Creates a new timer with a given delay */
Timer *timer_start(long delay, TimeoutHandler cb, void *data);
/* Stops and frees a timer */
void timer_stop(Timer *self);

/*! Dispatch all pending timers. Sets wait to the amount of time to wait for
  the next timer, or NULL if there are no timers to wait for */
void timer_dispatch(GTimeVal **wait);

#endif
