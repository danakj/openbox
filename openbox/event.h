#ifndef __events_h
#define __events_h

/*! Time at which the last event with a timestamp occured. */
extern Time event_lasttime;

/*! The value of the mask for the NumLock modifier */
extern unsigned int NumLockMask;
/*! The value of the mask for the ScrollLock modifier */
extern unsigned int ScrollLockMask;

void event_startup();
void event_shutdown();

void event_loop();

#endif
