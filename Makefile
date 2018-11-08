CC = gcc
CFLAGS = -Wall -Wextra -O3 -g -Wno-comment
SHARED = args tqueue wdictionary wcombinator tqueue tqueue_workers progress

all: client
client: directories binary_client
test: directories binary_test

binary_client: $(patsubst %,build/%.o,$(SHARED)) build/client.o
	$(CC) $(CFLAGS) -o client $^ -lcrypt -pthread -lrt -lm

binary_test:  $(patsubst %,build/%.o,$(SHARED)) build/test.o
	$(CC) $(CFLAGS) -o test $^ -lcrypt -pthread -lrt -lm

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

directories:
	mkdir -p build

clean:
	rm -rf client server test build/*.o

development: client
	./development.sh

test: binary_test
	./test