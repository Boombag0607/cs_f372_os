CC = gcc
CFLAGS = -Wall -Wextra

all: server client

server: src/server.c include/common.h
	$(CC) $(CFLAGS) -o server src/server.c -Iinclude -pthread

client: src/client.c include/common.h
	$(CC) $(CFLAGS) -o client src/client.c -Iinclude -pthread

clean:
	rm -f server client