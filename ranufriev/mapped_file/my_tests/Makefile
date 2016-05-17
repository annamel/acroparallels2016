all:
	$(CC) $(CFLAGS) -Wall -c  ../hash_table/error.c -o "$(MAKED)/error.o"
	$(CC) $(CFLAGS) -Wall -c  ../hash_table/hash_table.c -o "$(MAKED)/hash_table.o"
	$(CC) $(CFLAGS) -Wall -c  ../mf_lib/mapped_file.c -o "$(MAKED)/mapped_file.o"

test:

clean:
	rm -f "$(MAKED)/test-log" "$(MAKED)/"*.o

