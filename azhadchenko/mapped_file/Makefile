CC = gcc
CFLAGS = -Wall -std=gnu99
# -fprofile-arcs -ftest-coverage
STATICLIBS =  mmaped_file.c inverted_index.c chunk_manager.c
#EXECUTABLE = test.c

all : makeo
	mkdir out
	ar rcs out/libmappedfile.a $(STATICLIBS:.c=.o)
#	$(CC) $(CFLAGS) test.c -L. libmappedfile.a

makeo : $(STATICLIBS)
	$(CC) $(CFLAGS) -o mmaped_file.o    -c mmaped_file.c
	$(CC) $(CFLAGS) -o inverted_index.o -c inverted_index.c
	$(CC) $(CFLAGS) -o chunk_manager.o  -c chunk_manager.c


clean :
	rm $(STATICLIBS:.c=.o)

rmdir:
	rm -R out

