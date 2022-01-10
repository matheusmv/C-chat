CC=gcc

CFLAGS=-O2 \
	-pedantic-errors \
	-Wall \
	-Werror \

CLIENTLIBS=src/includes/*.c src/client/*.c
SERVERLIBS=src/includes/*.c src/server/*.c

CLIENTBIN=client
SERVERBIN=server

all: release

debug: CFLAGS += -ggdb
debug: all

release:
	$(CC) $(CFLAGS) $(CLIENTLIBS) -o $(CLIENTBIN) -lpthread
	$(CC) $(CFLAGS) $(SERVERLIBS) -o $(SERVERBIN) -lpthread

clean:
	rm $(CLIENTBIN)
	rm $(SERVERBIN)
