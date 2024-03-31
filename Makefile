CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Werror
LDFLAGS=-lm -lraylib

main: main.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
