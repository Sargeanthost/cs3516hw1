CC = gcc
CFLAGS = -std=c11
LDFLAGS =
TARGET = http_server

all: $(TARGET)

$(TARGET): http_server.o
	$(CC) $(LDFLAGS) -o $@ $<

http_client.o: http_server.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) *.o