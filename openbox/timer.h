#ifndef __timer_h
#define __timer_h

#include <glib.h>

typedef struct _ObTimer ObTimer;

/*! Data type of Timer callback */
typedef void (*ObTimeoutHandler)(void *data);

struct _ObTimer
{
    /*! Microseconds between timer firings */
    long delay;
    /*! Callback for timer expiry */
    ObTimeoutHandler action;
    /*! Data sent to callback */
    void *data;
    /*! We overload the delete operator to just set this to true */
    gboolean del_me;
    /*! The time the last fire should've been at */
    GTimeVal last;
    /*! When this timer will next trigger */
    GTimeVal timeout;
};

/*! Initializes the timer subsection */
void timer_startup();
/*! Destroys the timer subsection */
void timer_shutdown();

/* Creates a new timer with a given delay */
ObTimer *timer_start(long delay, ObTimeoutHandler cb, void *data);
/* Stops and frees a timer */
void timer_stop(ObTimer *self);

/*! Dispatch all pending timers. Sets wait to the amount of time to wait for
  the next timer, or NULL if there are no timers to wait for */
void timer_dispatch(GTimeVal **wait);

#endif
