prefix = /tmp/ob
exec_prefix = $(prefix)
libdir = $(exec_prefix)/lib

targets = libotk.so libotk.a
sources = init.c display.c screeninfo.c rect.c gccache.c color.c font.c \
          timer.c timerqueue.c imagecontrol.c
headers = init.h display.h screeninfo.h rect.h gccache.h color.h font.h \
          timer.h timerqueue.h imagecontrol.h

CFLAGS += -g -W -Wall -I/usr/gwar/include/python2.2 `pkg-config --cflags xft`
LDFLAGS += `pkg-config --libs xft`

.PHONY: all install clean

all: $(targets)

install: $(targets)
	install -d $(libdir)
	install $^ $(libdir)

clean:
	$(RM) $(targets) *.o core *\~ .\#*

libotk.so: $(sources:.c=.o)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

libotk.a: $(sources:.c=.o)
	$(AR) -cr $@ $^
