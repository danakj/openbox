#ifndef __events_h
#define __events_h

/*! Time at which the last event with a timestamp occured. */
extern Time event_lasttime;

void event_startup();
void event_shutdown();

void event_loop();

#endif
