CC=gcc

HASHTABLE = ../sources/hashtable/hashtable.c ../sources/hashtable/hashfunction.c
LOGGER = ../sources/logger/logger.c

MAPPEDFILE = ../sources/mallocs.c ../sources/chunks.c ../sources/mapped_file.c

SOURCES = $(LOGGER) $(HASHTABLE) $(MAPPEDFILE)

all: mf

mf: test_mf.c $(SOURCES)
	$(CC) test_mf.c $(SOURCES) -o test

hashtable: test_hashtable.c $(HASHTABLE) $(LOGGER)
	$(CC) test_hashtable.c $(HASHTABLE) $(LOGGER) -o test

clean:
	-rm -f test
	-rm -f *.o
