CC = gcc
CFLAGS = -Wall -Wextra -O0 -g -Wno-unused-parameter
SHARED = config wbuffer wdictionary wpermutation solvers tpool talkiewalkie

all: client server

client: directories $(patsubst %,build/%.o,$(SHARED)) build/main.o
	$(CC) $(CFLAGS) -o client $^ build/main.o -lcrypt -pthread -lm

server: directories $(patsubst %,build/%.o,$(SHARED)) build/server.o
	$(CC) $(CFLAGS) -o client $^ build/main.o -lcrypt -pthread -lm

directories:
	mkdir -p build

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET) build/*.o

dev: build
	./$(TARGET) password ./misc/dictionary.txt 10

dev_client: build/talkiewalkie.o build/config.o build/client.o
	$(CC) $(CFLAGS) -o client build/talkiewalkie.o build/config.o build/client.o
	./client -t 12 -s 128 \$$1\$$9779ofJE\$$AGS41EkDh6j.usuCUld3a0

dev_server: build/talkiewalkie.o build/config.o build/server.o
	$(CC) $(CFLAGS) -o server build/talkiewalkie.o build/config.o build/server.o
	./server -p 8000