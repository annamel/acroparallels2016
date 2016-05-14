all:
	$(CC) $(CFLAGS) -Wall -c -pthread mapped_file.c -o "$(MAKED)/mapped_file.o"
	$(CC) $(CFLAGS) -Wall -c -pthread hash_table.c -o "$(MAKED)/hash_table.o"
	$(CC) $(CFLAGS) -Wall -c -pthread chunk_manage.c -o "$(MAKED)/chunk_manage.o"
test:

clean:
	rm -f "$(MAKED)/test-log" "$(MAKED)/"*.o
