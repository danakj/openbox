#ifndef __framerender_h
#define __framerender_h

#include "frame.h"

void framerender_frame(Frame *self);

void framerender_popup_label(Window win, Size *sz, char *text);
void framerender_size_popup_label(char *text, Size *sz);

#endif
