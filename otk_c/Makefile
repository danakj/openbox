prefix=/tmp/ob
exec_prefix=$(prefix)
libdir=$(exec_prefix)/lib

targets = libotk.so libotk.a
sources = display.c
headers = display.h

CFLAGS+=-I/usr/gwar/include/python2.2

.PHONY: all install clean

all: $(targets)

install: $(targets)
	install -d $(libdir)
	install $^ $(libdir)

clean:
	$(RM) $(targets) *.o core

libotk.so: $(sources:.c=.o)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

libotk.a: $(sources:.c=.o)
	$(AR) -cr $@ $^
