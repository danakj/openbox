#include "timer.h"

#ifdef    HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

static GTimeVal now;
static GTimeVal ret_wait;
static GSList *timers; /* nearest timer is at the top */

#define NEAREST_TIMEOUT (((Timer*)timers->data)->timeout)

static long timecompare(GTimeVal *a, GTimeVal *b)
{
    long r;

    if ((r = b->tv_sec - a->tv_sec)) return r;
    return b->tv_usec - a->tv_usec;
    
}

static void insert_timer(Timer *self)
{
    GSList *it;
    for (it = timers; it != NULL; it = it->next) {
	Timer *t = it->data;
        if (timecompare(&self->timeout, &t->timeout) <= 0) {
	    timers = g_slist_insert_before(timers, it, self);
	    break;
	}
    }
    if (it == NULL) /* didnt fit anywhere in the list */
	timers = g_slist_append(timers, self);
}

void timer_startup()
{
    g_get_current_time(&now);
    timers = NULL;
}

void timer_shutdown()
{
    GSList *it;
    for (it = timers; it != NULL; it = it->next) {
	g_free(it->data);
    }
    g_slist_free(timers);
    timers = NULL;
}

Timer *timer_start(long delay, TimeoutHandler cb, void *data)
{
    Timer *self = g_new(Timer, 1);
    self->delay = delay;
    self->action = cb;
    self->data = data;
    self->del_me = FALSE;
    self->last = self->timeout = now;
    g_time_val_add(&self->timeout, delay);

    insert_timer(self);

    return self;
}

void timer_stop(Timer *self)
{
    self->del_me = TRUE;
}

/* find the time to wait for the nearest timeout */
static gboolean nearest_timeout_wait(GTimeVal *tm)
{
  if (timers == NULL)
    return FALSE;

  tm->tv_sec = NEAREST_TIMEOUT.tv_sec - now.tv_sec;
  tm->tv_usec = NEAREST_TIMEOUT.tv_usec - now.tv_usec;

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


void timer_dispatch(GTimeVal **wait)
{
    g_get_current_time(&now);

    while (timers != NULL) {
	Timer *curr = timers->data; /* get the top element */
	/* since timer_stop doesn't actually free the timer, we have to do our
	   real freeing in here.
	*/
	if (curr->del_me) {
	    timers = g_slist_delete_link(timers, timers); /* delete the top */
	    g_free(curr);
	    continue;
	}
	
	/* the queue is sorted, so if this timer shouldn't fire, none are 
	   ready */
        if (timecompare(&NEAREST_TIMEOUT, &now) <= 0)
	    break;

	/* we set the last fired time to delay msec after the previous firing,
	   then re-insert.  timers maintain their order and may trigger more
	   than once if they've waited more than one delay's worth of time.
	*/
	timers = g_slist_delete_link(timers, timers);
	g_time_val_add(&curr->last, curr->delay);
	curr->action(curr->data);
	g_time_val_add(&curr->timeout, curr->delay);
	insert_timer(curr);

	/* if at least one timer fires, then don't wait on X events, as there
	   may already be some in the queue from the timer callbacks.
	*/
	ret_wait.tv_sec = ret_wait.tv_usec = 0;
	*wait = &ret_wait;
	return;
    }

    if (nearest_timeout_wait(&ret_wait))
	*wait = &ret_wait;
    else
	*wait = NULL;
}
