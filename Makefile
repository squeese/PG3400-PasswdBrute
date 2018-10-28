CC = gcc
CFLAGS = -Wall -Wextra -O0 -g -Wno-unused-parameter
TARGET = main

FILES = wbuffer wdictionary tpool main 

$(TARGET): directories build

directories:
	mkdir -p build

build: $(patsubst %,build/%.o,$(FILES))
	$(CC) $(CFLAGS) -o $(TARGET) $^ -lcrypt -pthread

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

devrun: build
	./$(TARGET) password ./misc/dictionary.txt 10

clean:
	rm -rf $(TARGET) build/*.o
