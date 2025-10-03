SHELL := /bin/sh
CC = clang
CFLAGS = -g -Wall -Wpedantic -Werror -Wextra $(shell pkg-config --cflags gmp)
LIBFLAGS = -lm $(shell pkg-config --libs gmp)

.PHONY: all clean

all: keygen encrypt decrypt

keygen: keygen.o ss.o randstate.o numtheory.o
	$(CC) -o $@ $^ $(LIBFLAGS)

encrypt: encrypt.o ss.o randstate.o numtheory.o
	$(CC) -o $@ $^ $(LIBFLAGS)

decrypt: decrypt.o ss.o randstate.o numtheory.o
	$(CC) -o $@ $^ $(LIBFLAGS)

clean:
	rm -f keygen encrypt decrypt *.o

%.o : %.c
	$(CC) $(CFLAGS) -c $<

format:
	clang-format -i -style=file *.[ch]
