CC = gcc
CFLAGS = -Wall -Wextra -O0 -g -Wno-unused-parameter
TARGET = main

FILES = wbuffer wdictionary wpermutation solvers tpool client main 

$(TARGET): directories build

directories:
	mkdir -p build

build: $(patsubst %,build/%.o,$(FILES))
	$(CC) $(CFLAGS) -o $(TARGET) $^ -lcrypt -pthread -lm

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET) build/*.o

dev: build
	./$(TARGET) password ./misc/dictionary.txt 10

dev_client: build/client.o
	$(CC) $(CFLAGS) -o client build/client.o
	./client

dev_server: build/server.o
	$(CC) $(CFLAGS) -o server build/server.o
	./server