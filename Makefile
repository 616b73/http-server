CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = src/server.c
OBJS = $(SRCS:.c=.o)
TARGET = http-server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
