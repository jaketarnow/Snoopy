CC = gcc
CFLAGS = -g -O0 -Wall
LDFLAGS = -g

LIBSRC = utils.c
LIBOBJ = $(LIBSRC:%.c=%.o)

all: main

main: main.o $(LIBOBJ)
	$(CC) $(LDFLAGS) $(LIBOBJ) main.o -o upnp

clean:
	rm -f $(LIBOBJ) main.o upnp

.PHONY: clean