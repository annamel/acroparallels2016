mfile: test.o hash_table.o mapped_file.o chunkmanager.o list.o logger.o
	gcc test.o hash_table.o mapped_file.o chunkmanager.o list.o logger.o -o mfile

test.o: test.c
	gcc -c test.c
	
hash_table.o: hash_table.c
	gcc -c hash_table.c

mapped_file.o: mapped_file.c
	gcc -c mapped_file.c

chunk_manage.o: chunkmanager.c
	gcc -c chunkmanager.c

list.o: list.c
	gcc -c list.c

logger.o: logger.c
	gcc -c logger.c
