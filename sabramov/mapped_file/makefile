mfile: main.o hash_table.o mapped_file.o chunk_manage.o
	gcc main.o hash_table.o mapped_file.o chunk_manage.o -o mfile

main.o: main.c
	gcc -c main.c
	
hash_table.o: hash_table.c
	gcc -c hash_table.c

mapped_file.o: mapped_file.c
	gcc -c mapped_file.c

chunk_manage.o: chunk_manage.c
	gcc -c chunk_manage.c