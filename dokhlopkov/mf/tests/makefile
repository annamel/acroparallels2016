CC=gcc

HASHTABLE = ../sources/hashtable/hashtable.c
HASHFUNCTION = ../sources/hashtable/hashfunction.c
LOGGER = ../sources/logger/logger.c

SOURCES = ../sources/hashtable/hashtable.c ../sources/hashtable/hashfunction.c ../sources/logger/logger.c

all: test

test: test_hashtable.c $(SOURCES)
	$(CC) test_hashtable.c $(SOURCES) -o test

clean:
	-rm -f test
	-rm -f *.o
