CC = gcc
CFLAGS = -Wall -Wextra -Werror -O0 -g
TARGET = main

FILES = crypto dictionary main 

$(TARGET): directories build

directories:
	mkdir -p build

build: $(patsubst %,build/%.o,$(FILES))
	$(CC) $(CFLAGS) -o $(TARGET) $^ -lcrypt

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

devrun: build
	./$(TARGET) password ./misc/small.txt

clean:
	rm -rf $(TARGET) build/*.o