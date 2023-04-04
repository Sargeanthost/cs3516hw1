CC = gcc
CFLAGS = -std=c11

.PHONY: all clean

all: client/http_client server/http_server

client/http_client: client/http_client.c
	$(CC) $(CFLAGS) $< -o $@

server/http_server: server/http_server.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f client/http_client server/http_server