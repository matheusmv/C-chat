CC=gcc

CFLAGS=-O2 \
	-pedantic-errors \
	-Wall \
	-Werror \

CLIENTLIBS=utils.c network.c client.c client-app.c
SERVERLIBS=utils.c network.c server.c server-app.c

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
