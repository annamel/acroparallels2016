all:
	$(CC) $(CFLAGS) -c hash_table.c -o "$(MAKE_DIR)/hash_table.o"

test:
	$(CC) $(CFLAGS) -o "$(MAKE_DIR)/test-hash" test.c "$(MAKE_DIR)/hash_table.o" "$(MAKE_DIR)/log.o" "$(MAKE_DIR)/log_ring_buf.o"

clean:
	rm -f "$(MAKE_DIR)/test-hash" "$(MAKE_DIR)/"*.o
