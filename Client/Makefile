CC=gcc
CFLAGS=-Wall -Wextra -pthread

.PHONY: all clean

all: server client

server: server.o
	$(CC) $(CFLAGS) -o $@ $<

client: client.o
	$(CC) $(CFLAGS) -o $@ $<
	
clear:
	rm -f *.o server client
