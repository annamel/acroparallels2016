.PHONY: all

all: client server

client: client.c log.c logger.c log.h logger.h
	gcc -o client client.c log.c logger.c -lpthread -lrt
	
server: server.c log.c logger.c log.h logger.h
	gcc -o server server.c log.c logger.c -lpthread -lrt

clean:
	rm -f server client

