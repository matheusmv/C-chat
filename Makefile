CFLAGS=-O2 \
	-pedantic-errors \
	-Wall \
	-Werror \
	-std=c11 \

CLIENTBIN=client
SERVERBIN=server

all: release

debug: CFLAGS += -ggdb
debug: all

release:
	$(CC) $(CFLAGS) client.c -o $(CLIENTBIN) -lpthread
	$(CC) $(CFLAGS) server.c -o $(SERVERBIN) -lpthread

clean:
	rm $(CLIENTBIN)
	rm $(SERVERBIN)
