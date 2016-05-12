all:
	$(CC) $(CFLAGS) -Wall -c  mapped_file.c -o "$(MAKED)/mapped_file.o"
	$(CC) $(CFLAGS) -Wall -c  hash_table.c -o "$(MAKED)/hash_table.o"
	$(CC) $(CFLAGS) -Wall -c  list.c -o "$(MAKED)/list.o"
	$(CC) $(CFLAGS) -Wall -c  i_list.c -o "$(MAKED)/i_list.o"
	$(CC) $(CFLAGS) -Wall -c  pool_object.c -o "$(MAKED)/pool_object.o"

test:

clean:
	rm -f "$(MAKED)/test-log" "$(MAKED)/"*.o
