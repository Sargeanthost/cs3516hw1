CC = gcc
CFLAGS = -std=c11
LDFLAGS =
TARGET = http_client

all: $(TARGET)

$(TARGET): http_client.o
	$(CC) $(LDFLAGS) -o $@ $<

http_client.o: http_client.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) *.o