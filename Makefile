CC=gcc

CFLAGS=-O2 \
	-pedantic-errors \
	-Wall \
	-Werror \

CLIENTBIN=client
SERVERBIN=server

all: release

debug: CFLAGS += -ggdb
debug: all

release:
	$(CC) $(CFLAGS) utils.c client.c client-app.c -o $(CLIENTBIN) -lpthread
	$(CC) $(CFLAGS) utils.c server.c server-app.c -o $(SERVERBIN) -lpthread

clean:
	rm $(CLIENTBIN)
	rm $(SERVERBIN)
