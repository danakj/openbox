CFLAGS=-ansi -pedantic -Wall `pkg-config --cflags glib-2.0`
LIBS=`pkg-config --libs glib-2.0` -ll

targets = cltest

sources = obcl.c main.c parse.c lex.c
headers = obcl.h

.PHONY: all clean

all: $(targets)

$(targets): $(sources:.c=.o)
	$(CC) -o $@ $^ $(LIBS)

parse.c: parse.y
	$(YACC) -d -o$@ $^

lex.c: lex.l
	$(LEX) -o$@ $^

clean:
	$(RM) $(targets) *.o core *~ lex.c parse.c parse.h
