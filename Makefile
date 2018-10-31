CC = gcc
CFLAGS = -Wall -Wextra -O0 -g -Wno-unused-parameter
SHARED = args wbuffer wdictionary wpermutation solvers talkiewalkie tpool

all: client server
client: directories binary_server binary_client
server: directories binary_server
test: directories binary_test

binary_client: $(patsubst %,build/%.o,$(SHARED)) build/client.o
	$(CC) $(CFLAGS) -o client $^ -lcrypt -pthread -lm

binary_server: $(patsubst %,build/%.o,$(SHARED)) build/server.o
	$(CC) $(CFLAGS) -o server $^ -lcrypt -pthread -lm

binary_test: $(patsubst %,build/%.o,$(SHARED)) build/test.o
	$(CC) $(CFLAGS) -o test $^ -lcrypt -pthread -lm

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

directories:
	mkdir -p build

clean:
	rm -rf client server test build/*.o

runclient: client
	./client \$$1\$$ckvWM6T@\$$H6H/R5d4a/QjpB02Ri/V01

runserver: server
	./server

runtest: test
	./test