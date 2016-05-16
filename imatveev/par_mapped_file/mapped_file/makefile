all:
	$(CC) $(CFLAGS) -Wall -c  -pthread mapped_file.c -o "$(MAKED)/mapped_file.o"
	$(CC) $(CFLAGS) -Wall -c  -pthread hash_table.c -o "$(MAKED)/hash_table.o"
	$(CC) $(CFLAGS) -Wall -c  -pthread list.c -o "$(MAKED)/list.o"
	$(CC) $(CFLAGS) -Wall -c  -pthread i_list.c -o "$(MAKED)/i_list.o"
	$(CC) $(CFLAGS) -Wall -c  -pthread pool_object.c -o "$(MAKED)/pool_object.o"
	$(CC) $(CFLAGS) -Wall -c  -pthread logger.c -o "$(MAKED)/logger.o"

test:

clean:
	rm -f "$(MAKED)/test-log" "$(MAKED)/"*.o
