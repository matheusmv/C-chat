CC=gcc

CFLAGS=-pedantic-errors -Wall -Werror

LFLAGS=-lpthread

CLIENTLIBS=src/includes/*.c src/client/*.c
SERVERLIBS=src/includes/*.c src/server/*.c

CLIENTBIN=client
SERVERBIN=server

all: CFLAGS +=-O2
all: release

debug: CFLAGS +=-O0 -g
debug: release

release:
	$(CC) $(CFLAGS) $(CLIENTLIBS) -o $(CLIENTBIN) $(LFLAGS)
	$(CC) $(CFLAGS) $(SERVERLIBS) -o $(SERVERBIN) $(LFLAGS)

clean:
	$(RM) $(CLIENTBIN)
	$(RM) $(SERVERBIN)
