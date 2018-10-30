CC = gcc
CFLAGS = -Wall -Wextra -O0 -g -Wno-unused-parameter
SHARED = args config wbuffer wdictionary wpermutation solvers tpool talkiewalkie

all: client server
client: directories binary_client
server: directories binary_server

binary_client: $(patsubst %,build/%.o,$(SHARED)) build/main.o
	$(CC) $(CFLAGS) -o client $^ -lcrypt -pthread -lm

binary_server: $(patsubst %,build/%.o,$(SHARED)) build/server.o
	$(CC) $(CFLAGS) -o server $^ -lcrypt -pthread -lm

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

directories:
	mkdir -p build

clean:
	rm -rf client server build/*.o

runserver: server
	./server -h 192.168.1.1 -p 2000 -t 10 $$1$$ckvWM6T@$$H6H/R5d4a/QjpB02Ri/V01