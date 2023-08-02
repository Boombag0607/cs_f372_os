CC = gcc
CFLAGS = -Wall -Wextra

all: server client

server: server.c common.h
    $(CC) $(CFLAGS) -o server server.c -pthread

client: client.c common.h
    $(CC) $(CFLAGS) -o client client.c -pthread

clean:
    rm -f server client
