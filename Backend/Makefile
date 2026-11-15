CC = clang

CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRC = src/main.c \
      src/graph.c \
      src/router.c \
      src/stack.c \
      src/queue.c \
      src/history.c \
      src/hashtable.c \
      src/bst.c \
      src/storage.c \
      src/server.c \
      src/mongoose.c

TARGET = network_server

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)