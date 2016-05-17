CC = gcc
CFLAGS = -std=c99 -pthread -O3 

all: main

main: logger.o main.o
	$(CC) $(CFLAGS) logger.o main.o -o main

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

logger.o: logger.c
	$(CC) $(CFLAGS) -c logger.c

clean:
	rm -f *.o main logfile.txt
