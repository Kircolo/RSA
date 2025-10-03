SHELL := /bin/sh
CC = clang
CFLAGS = -g -Wall -Wpedantic -Werror -Wextra $(shell pkg-config --cflags gmp)
LIBFLAGS = -lm $(shell pkg-config --libs gmp)

.PHONY: all clean

all: keygen encrypt decrypt	

tests: tests_numtheory tests_ss

check-numtheory: tests_numtheory
	./tests_numtheory

check-ss: tests_ss
	./tests_ss

keygen: keygen.o ss.o randstate.o numtheory.o
	$(CC) -o $@ $^ $(LIBFLAGS)

encrypt: encrypt.o ss.o randstate.o numtheory.o
	$(CC) -o $@ $^ $(LIBFLAGS)

decrypt: decrypt.o ss.o randstate.o numtheory.o
	$(CC) -o $@ $^ $(LIBFLAGS)

tests_numtheory: tests_numtheory.o numtheory.o randstate.o
	$(CC) -o $@ $^ $(LIBFLAGS)

tests_ss: tests_ss.o ss.o numtheory.o randstate.o
	$(CC) -o $@ $^ $(LIBFLAGS)

clean:
	rm -f keygen encrypt decrypt  tests_numtheory tests_ss *.o

%.o : %.c
	$(CC) $(CFLAGS) -c $<

format:
	clang-format -i -style=file *.[ch]
