all:
	$(CC) $(CFLAGS) -Wall -c log.c -o "$(MAKE_DIR)/log.o"
	$(CC) $(CFLAGS) -Wall -c log_ring_buf.c -o "$(MAKE_DIR)/log_ring_buf.o"

test:
	$(CC) $(CFLAGS) -o "$(MAKE_DIR)/test-log" test.c "$(MAKE_DIR)/log.o" "$(MAKE_DIR)/log_ring_buf.o"

clean:
	rm -f "$(MAKE_DIR)/test-log" "$(MAKE_DIR)/*.o"
