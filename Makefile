CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

SRCS = src/server.c src/http_parser.c src/router.c src/response.c src/queue.c src/cgi.c
OBJS = $(SRCS:.c=.o)
TARGET = http-server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
