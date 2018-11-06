CC = gcc
CFLAGS = -Wall -Wextra -O0 -g -Wno-unused-parameter
SHARED = args wbuffer tpool client_thandlers wpermutation progress

all: client server
client: directories binary_client
server: directories binary_server
test: directories binary_test

binary_client: $(patsubst %,build/%.o,$(SHARED)) build/client.o
	$(CC) $(CFLAGS) -o client $^ -lcrypt -pthread -lm -lrt

binary_server: $(patsubst %,build/%.o,$(SHARED)) build/server.o
	$(CC) $(CFLAGS) -o server $^ -lcrypt -pthread -lm -lrt

binary_test: $(patsubst %,build/%.o,$(SHARED)) build/test.o
	$(CC) $(CFLAGS) -o test $^ -lcrypt -pthread -lm -lrt

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

directories:
	mkdir -p build

clean:
	rm -rf client server test build/*.o

runclient: client
	./client -l 2 \$$1\$$9779ofJE\$$c.p.EwsI57yV2xjeorQbs1

runclient_great: client
	./client \$$1\$$9779ofJE\$$MKAskbSv72cuWHNmBHTwX.

runserver: server
	./server

runtest: test
	./test

runshell: client
	./cron.sh