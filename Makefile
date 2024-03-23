CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g
OBJ = Hashtable.o LinkedList.o main.o\
load_balancer.o server.o
DEPS = Hashtable.h LinkedList.h\
load_balancer.h server.h utils.h

.PHONY: build clean

build: $(DEPS) tema2

tema2: $(OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o tema2 *.h.gch
