files=$(wildcard *.c)

all: $(files:.c=)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^ -lX11 -L/usr/X11R6/lib -I/usr/X11R6/include
