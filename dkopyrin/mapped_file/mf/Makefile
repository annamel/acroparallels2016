all:
	$(CC) $(CFLAGS) -Wall -c mf.c -o "$(MAKE_DIR)/mf.o"

test:
	$(CC) $(CFLAGS) -o "$(MAKE_DIR)/test-mf" test.c "$(MAKE_DIR)/"*.o

clean:
	rm -f "$(MAKE_DIR)/test-mf" "$(MAKE_DIR)/"*.o
