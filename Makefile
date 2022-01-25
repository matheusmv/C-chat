CC := gcc

CFLAGS := -pedantic-errors -Wall -Werror -Wextra -std=c11

CLIENTSRCDIRS := src/includes/ src/client/
SERVERSRCDIRS := src/includes/ src/server/

CLIENTSRCS := $(shell find $(CLIENTSRCDIRS) -name '*.c')
SERVERSRCS := $(shell find $(SERVERSRCDIRS) -name '*.c')

CLIENTBIN := client
SERVERBIN := server

LFLAGS := -lpthread

all: CFLAGS +=-O2
all: release

debug: CFLAGS +=-O0 -ggdb
debug: release

release:
	$(CC) $(CFLAGS) $(CLIENTSRCS) -o $(CLIENTBIN) $(LFLAGS) -DLCOLOR
	$(CC) $(CFLAGS) $(SERVERSRCS) -o $(SERVERBIN) $(LFLAGS) -DLCOLOR

clean:
	$(RM) $(CLIENTBIN)
	$(RM) $(SERVERBIN)
