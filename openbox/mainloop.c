/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   mainloop.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "mainloop.h"
#include "event.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <signal.h>

typedef struct _ObMainLoopTimer             ObMainLoopTimer;
typedef struct _ObMainLoopSignal            ObMainLoopSignal;
typedef struct _ObMainLoopSignalHandlerType ObMainLoopSignalHandlerType;
typedef struct _ObMainLoopXHandlerType      ObMainLoopXHandlerType;
typedef struct _ObMainLoopFdHandlerType     ObMainLoopFdHandlerType;

/* this should be more than the number of possible signals on any
   architecture... */
#define NUM_SIGNALS 99

/* all created ObMainLoops. Used by the signal handler to pass along signals */
static GSList *all_loops;

/* signals are global to all loops */
struct {
    guint installed; /* a ref count */
    struct sigaction oldact;
} all_signals[NUM_SIGNALS];

/* a set of all possible signals */
sigset_t all_signals_set;

/* signals which cause a core dump, these can't be used for callbacks */
static gint core_signals[] =
{
    SIGABRT,
    SIGSEGV,
    SIGFPE,
    SIGILL,
    SIGQUIT,
    SIGTRAP,
    SIGSYS,
    SIGBUS,
    SIGXCPU,
    SIGXFSZ
};
#define NUM_CORE_SIGNALS (sizeof(core_signals) / sizeof(core_signals[0]))

static void sighandler(gint sig);
static void timer_dispatch(ObMainLoop *loop, GTimeVal **wait);
static void fd_handler_destroy(gpointer data);

struct _ObMainLoop
{
    Display *display;

    gboolean run;     /* do keep running */
    gboolean running; /* is still running */

    GSList *x_handlers;

    gint fd_x; /* The X fd is a special case! */
    gint fd_max;
    GHashTable *fd_handlers;
    fd_set fd_set;

    GSList *timers;
    GTimeVal now;
    GTimeVal ret_wait;

    gboolean signal_fired;
    guint signals_fired[NUM_SIGNALS];
    GSList *signal_handlers[NUM_SIGNALS];
};

struct _ObMainLoopTimer
{
    gulong delay;
    GSourceFunc func;
    gpointer data;
    GEqualFunc equal;
    GDestroyNotify destroy;

    /* The timer needs to be freed */
    gboolean del_me;
    /* The time the last fire should've been at */
    GTimeVal last;
    /* When this timer will next trigger */
    GTimeVal timeout;
};

struct _ObMainLoopSignalHandlerType
{
    ObMainLoop *loop;
    gint signal;
    gpointer data;
    ObMainLoopSignalHandler func;
    GDestroyNotify destroy;
};

struct _ObMainLoopXHandlerType
{
    ObMainLoop *loop;
    gpointer data;
    ObMainLoopXHandler func;
    GDestroyNotify destroy;
};

struct _ObMainLoopFdHandlerType
{
    ObMainLoop *loop;
    gint fd;
    gpointer data;
    ObMainLoopFdHandler func;
    GDestroyNotify destroy;
};

ObMainLoop *ob_main_loop_new(Display *display)
{
    ObMainLoop *loop;

    loop = g_new0(ObMainLoop, 1);
    loop->display = display;
    loop->fd_x = ConnectionNumber(display);
    FD_ZERO(&loop->fd_set);
    FD_SET(loop->fd_x, &loop->fd_set);
    loop->fd_max = loop->fd_x;

    loop->fd_handlers = g_hash_table_new_full(g_int_hash, g_int_equal,
                                              NULL, fd_handler_destroy);

    g_get_current_time(&loop->now);

    /* only do this if we're the first loop created */
    if (!all_loops) {
        guint i;
        struct sigaction action;
        sigset_t sigset;

        /* initialize the all_signals_set */
        sigfillset(&all_signals_set);

        sigemptyset(&sigset);
        action.sa_handler = sighandler;
        action.sa_mask = sigset;
        action.sa_flags = SA_NOCLDSTOP;

        /* grab all the signals that cause core dumps */
        for (i = 0; i < NUM_CORE_SIGNALS; ++i) {
            /* SIGABRT is curiously not grabbed here!! that's because when we
               get one of the core_signals, we use abort() to dump the core.
               And having the abort() only go back to our signal handler again
               is less than optimal */
            if (core_signals[i] != SIGABRT) {
                sigaction(core_signals[i], &action,
                          &all_signals[core_signals[i]].oldact);
                all_signals[core_signals[i]].installed++;
            }
        }
    }

    all_loops = g_slist_prepend(all_loops, loop);

    return loop;
}

void ob_main_loop_destroy(ObMainLoop *loop)
{
    guint i;
    GSList *it, *next;

    if (loop) {
        g_assert(loop->running == FALSE);

        for (it = loop->x_handlers; it; it = next) {
            ObMainLoopXHandlerType *h = it->data;
            next = g_slist_next(it);
            ob_main_loop_x_remove(loop, h->func);
        }

        g_hash_table_destroy(loop->fd_handlers);

        for (it = loop->timers; it; it = g_slist_next(it)) {
            ObMainLoopTimer *t = it->data;
            if (t->destroy) t->destroy(t->data);
            g_free(t);
        }
        g_slist_free(loop->timers);
        loop->timers = NULL;

        for (i = 0; i < NUM_SIGNALS; ++i)
            for (it = loop->signal_handlers[i]; it; it = next) {
                ObMainLoopSignalHandlerType *h = it->data;
                next = g_slist_next(it);
                ob_main_loop_signal_remove(loop, h->func);
            }

        all_loops = g_slist_remove(all_loops, loop);

        /* only do this if we're the last loop destroyed */
        if (!all_loops) {
            /* grab all the signals that cause core dumps */
            for (i = 0; i < NUM_CORE_SIGNALS; ++i) {
                if (all_signals[core_signals[i]].installed) {
                    sigaction(core_signals[i],
                              &all_signals[core_signals[i]].oldact, NULL);
                    all_signals[core_signals[i]].installed--;
                }
            }
        }

        g_free(loop);
    }
}

static void fd_handle_foreach(gpointer key,
                              gpointer value,
                              gpointer data)
{
    ObMainLoopFdHandlerType *h = value;
    fd_set *set = data;

    if (FD_ISSET(h->fd, set))
        h->func(h->fd, h->data);
}

void ob_main_loop_run(ObMainLoop *loop)
{
    XEvent e;
    struct timeval *wait;
    fd_set selset;
    GSList *it;

    loop->run = TRUE;
    loop->running = TRUE;

    while (loop->run) {
        if (loop->signal_fired) {
            guint i;
            sigset_t oldset;

            /* block signals so that we can do this without the data changing
               on us */
            sigprocmask(SIG_SETMASK, &all_signals_set, &oldset);

            for (i = 0; i < NUM_SIGNALS; ++i) {
                while (loop->signals_fired[i]) {
                    for (it = loop->signal_handlers[i];
                            it; it = g_slist_next(it)) {
                        ObMainLoopSignalHandlerType *h = it->data;
                        h->func(i, h->data);
                    }
                    loop->signals_fired[i]--;
                }
            }
            loop->signal_fired = FALSE;

            sigprocmask(SIG_SETMASK, &oldset, NULL);
        } else if (XPending(loop->display)) {
            do {
                XNextEvent(loop->display, &e);

                for (it = loop->x_handlers; it; it = g_slist_next(it)) {
                    ObMainLoopXHandlerType *h = it->data;
                    h->func(&e, h->data);
                }
            } while (XPending(loop->display) && loop->run);
        } else {
            /* this only runs if there were no x events received */

            timer_dispatch(loop, (GTimeVal**)&wait);

            selset = loop->fd_set;
            /* there is a small race condition here. if a signal occurs
               between this if() and the select() then we will not process
               the signal until 'wait' expires. possible solutions include
               using GStaticMutex, and having the signal handler set 'wait'
               to 0 */
            if (!loop->signal_fired)
                select(loop->fd_max + 1, &selset, NULL, NULL, wait);

            /* handle the X events with highest prioirity */
            if (FD_ISSET(loop->fd_x, &selset))
                continue;

            g_hash_table_foreach(loop->fd_handlers,
                                 fd_handle_foreach, &selset);
        }
    }

    loop->running = FALSE;
}

void ob_main_loop_exit(ObMainLoop *loop)
{
    loop->run = FALSE;
}

/*** XEVENT WATCHERS ***/

void ob_main_loop_x_add(ObMainLoop *loop,
                        ObMainLoopXHandler handler,
                        gpointer data,
                        GDestroyNotify notify)
{
    ObMainLoopXHandlerType *h;

    h = g_new(ObMainLoopXHandlerType, 1);
    h->loop = loop;
    h->func = handler;
    h->data = data;
    h->destroy = notify;
    loop->x_handlers = g_slist_prepend(loop->x_handlers, h);
}

void ob_main_loop_x_remove(ObMainLoop *loop,
                           ObMainLoopXHandler handler)
{
    GSList *it, *next;

    for (it = loop->x_handlers; it; it = next) {
        ObMainLoopXHandlerType *h = it->data;
        next = g_slist_next(it);
        if (h->func == handler) {
            loop->x_handlers = g_slist_delete_link(loop->x_handlers, it);
            if (h->destroy) h->destroy(h->data);
            g_free(h);
        }
    }
}

/*** SIGNAL WATCHERS ***/

static void sighandler(gint sig)
{
    GSList *it;
    guint i;

    g_return_if_fail(sig < NUM_SIGNALS);

    for (i = 0; i < NUM_CORE_SIGNALS; ++i)
        if (sig == core_signals[i]) {
            /* XXX special case for signals that default to core dump.
               but throw some helpful output here... */

            fprintf(stderr, "How are you gentlemen? All your base are"
                    " belong to us. (Openbox received signal %d)\n", sig);

            /* die with a core dump */
            abort();
        }

    for (it = all_loops; it; it = g_slist_next(it)) {
        ObMainLoop *loop = it->data;
        loop->signal_fired = TRUE;
        loop->signals_fired[sig]++;
    }
}

void ob_main_loop_signal_add(ObMainLoop *loop,
                             gint signal,
                             ObMainLoopSignalHandler handler,
                             gpointer data,
                             GDestroyNotify notify)
{
    ObMainLoopSignalHandlerType *h;

    g_return_if_fail(signal < NUM_SIGNALS);

    h = g_new(ObMainLoopSignalHandlerType, 1);
    h->loop = loop;
    h->signal = signal;
    h->func = handler;
    h->data = data;
    h->destroy = notify;
    loop->signal_handlers[h->signal] =
        g_slist_prepend(loop->signal_handlers[h->signal], h);

    if (!all_signals[signal].installed) {
        struct sigaction action;
        sigset_t sigset;

        sigemptyset(&sigset);
        action.sa_handler = sighandler;
        action.sa_mask = sigset;
        action.sa_flags = SA_NOCLDSTOP;

        sigaction(signal, &action, &all_signals[signal].oldact);
    }

    all_signals[signal].installed++;
}

void ob_main_loop_signal_remove(ObMainLoop *loop,
                                ObMainLoopSignalHandler handler)
{
    guint i;
    GSList *it, *next;

    for (i = 0; i < NUM_SIGNALS; ++i) {
        for (it = loop->signal_handlers[i]; it; it = next) {
            ObMainLoopSignalHandlerType *h = it->data;

            next = g_slist_next(it);

            if (h->func == handler) {
                g_assert(all_signals[h->signal].installed > 0);

                all_signals[h->signal].installed--;
                if (!all_signals[h->signal].installed) {
                    sigaction(h->signal, &all_signals[h->signal].oldact, NULL);
                }

                loop->signal_handlers[i] =
                    g_slist_delete_link(loop->signal_handlers[i], it);
                if (h->destroy) h->destroy(h->data);

                g_free(h);
            }
        }
    }

}

/*** FILE DESCRIPTOR WATCHERS ***/

static void max_fd_func(gpointer key, gpointer value, gpointer data)
{
    ObMainLoop *loop = data;

    /* key is the fd */
    loop->fd_max = MAX(loop->fd_max, *(gint*)key);
}

static void calc_max_fd(ObMainLoop *loop)
{
    loop->fd_max = loop->fd_x;

    g_hash_table_foreach(loop->fd_handlers, max_fd_func, loop);
}

void ob_main_loop_fd_add(ObMainLoop *loop,
                         gint fd,
                         ObMainLoopFdHandler handler,
                         gpointer data,
                         GDestroyNotify notify)
{
    ObMainLoopFdHandlerType *h;

    h = g_new(ObMainLoopFdHandlerType, 1);
    h->loop = loop;
    h->fd = fd;
    h->func = handler;
    h->data = data;
    h->destroy = notify;

    g_hash_table_replace(loop->fd_handlers, &h->fd, h);
    FD_SET(h->fd, &loop->fd_set);
    calc_max_fd(loop);
}

static void fd_handler_destroy(gpointer data)
{
    ObMainLoopFdHandlerType *h = data;

    FD_CLR(h->fd, &h->loop->fd_set);

    if (h->destroy)
        h->destroy(h->data);
}

void ob_main_loop_fd_remove(ObMainLoop *loop,
                            gint fd)
{
    g_hash_table_remove(loop->fd_handlers, &fd);
}

/*** TIMEOUTS ***/

#define NEAREST_TIMEOUT(loop) \
    (((ObMainLoopTimer*)(loop)->timers->data)->timeout)

static glong timecompare(GTimeVal *a, GTimeVal *b)
{
    glong r;

    if ((r = b->tv_sec - a->tv_sec)) return r;
    return b->tv_usec - a->tv_usec;

}

static void insert_timer(ObMainLoop *loop, ObMainLoopTimer *ins)
{
    GSList *it;
    for (it = loop->timers; it; it = g_slist_next(it)) {
        ObMainLoopTimer *t = it->data;
        if (timecompare(&ins->timeout, &t->timeout) >= 0) {
            loop->timers = g_slist_insert_before(loop->timers, it, ins);
            break;
        }
    }
    if (it == NULL) /* didnt fit anywhere in the list */
        loop->timers = g_slist_append(loop->timers, ins);
}

void ob_main_loop_timeout_add(ObMainLoop *loop,
                              gulong microseconds,
                              GSourceFunc handler,
                              gpointer data,
                              GEqualFunc cmp,
                              GDestroyNotify notify)
{
    ObMainLoopTimer *t = g_new(ObMainLoopTimer, 1);
    t->delay = microseconds;
    t->func = handler;
    t->data = data;
    t->equal = cmp;
    t->destroy = notify;
    t->del_me = FALSE;
    g_get_current_time(&loop->now);
    t->last = t->timeout = loop->now;
    g_time_val_add(&t->timeout, t->delay);

    insert_timer(loop, t);
}

void ob_main_loop_timeout_remove(ObMainLoop *loop,
                                 GSourceFunc handler)
{
    GSList *it;

    for (it = loop->timers; it; it = g_slist_next(it)) {
        ObMainLoopTimer *t = it->data;
        if (t->func == handler)
            t->del_me = TRUE;
    }
}

void ob_main_loop_timeout_remove_data(ObMainLoop *loop, GSourceFunc handler,
                                      gpointer data, gboolean cancel_dest)
{
    GSList *it;

    for (it = loop->timers; it; it = g_slist_next(it)) {
        ObMainLoopTimer *t = it->data;
        if (t->func == handler && t->equal(t->data, data)) {
            t->del_me = TRUE;
            if (cancel_dest)
                t->destroy = NULL;
        }
    }
}

/* find the time to wait for the nearest timeout */
static gboolean nearest_timeout_wait(ObMainLoop *loop, GTimeVal *tm)
{
  if (loop->timers == NULL)
    return FALSE;

  tm->tv_sec = NEAREST_TIMEOUT(loop).tv_sec - loop->now.tv_sec;
  tm->tv_usec = NEAREST_TIMEOUT(loop).tv_usec - loop->now.tv_usec;

  while (tm->tv_usec < 0) {
    tm->tv_usec += G_USEC_PER_SEC;
    tm->tv_sec--;
  }
  tm->tv_sec += tm->tv_usec / G_USEC_PER_SEC;
  tm->tv_usec %= G_USEC_PER_SEC;
  if (tm->tv_sec < 0)
    tm->tv_sec = 0;

  return TRUE;
}

static void timer_dispatch(ObMainLoop *loop, GTimeVal **wait)
{
    GSList *it, *next;

    gboolean fired = FALSE;

    g_get_current_time(&loop->now);

    for (it = loop->timers; it; it = next) {
        ObMainLoopTimer *curr;

        next = g_slist_next(it);

        curr = it->data;

        /* since timer_stop doesn't actually free the timer, we have to do our
           real freeing in here.
        */
        if (curr->del_me) {
            /* delete the top */
            loop->timers = g_slist_delete_link(loop->timers, it);
            if (curr->destroy)
                curr->destroy(curr->data);
            g_free(curr);
            continue;
        }

        /* the queue is sorted, so if this timer shouldn't fire, none are
           ready */
        if (timecompare(&NEAREST_TIMEOUT(loop), &loop->now) < 0)
            break;

        /* we set the last fired time to delay msec after the previous firing,
           then re-insert.  timers maintain their order and may trigger more
           than once if they've waited more than one delay's worth of time.
        */
        loop->timers = g_slist_delete_link(loop->timers, it);
        g_time_val_add(&curr->last, curr->delay);
        if (curr->func(curr->data)) {
            g_time_val_add(&curr->timeout, curr->delay);
            insert_timer(loop, curr);
        } else {
            if (curr->destroy)
                curr->destroy(curr->data);
            g_free(curr);
        }

        fired = TRUE;
    }

    if (fired) {
        /* if at least one timer fires, then don't wait on X events, as there
           may already be some in the queue from the timer callbacks.
        */
        loop->ret_wait.tv_sec = loop->ret_wait.tv_usec = 0;
        *wait = &loop->ret_wait;
    } else if (nearest_timeout_wait(loop, &loop->ret_wait))
        *wait = &loop->ret_wait;
    else
        *wait = NULL;
}
