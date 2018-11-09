CC = gcc
DFLAGS = -Wall -Wextra -O3 -g -Wno-comment
BFLAGS = -O3
SHARED = args wcombinator tqueue tqueue_workers progress vmap

all: client
client: directories binary_client

build: $(patsubst %,build/%.o,$(SHARED)) build/client.o
	$(CC) $(BFLAGS) -o client $^ -lcrypt -pthread -lrt -lm

binary_client: $(patsubst %,build/%.o,$(SHARED)) build/client.o
	$(CC) $(CFLAGS) -o client $^ -lcrypt -pthread -lrt -lm

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

directories:
	mkdir -p build

development: client
	./development.sh

clean:
	rm -rf client server test build/*.o