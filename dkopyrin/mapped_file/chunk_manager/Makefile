IFS=$(echo -en "\n\b")

all:
	$(CC) $(CFLAGS) -Wall -c chunk.c -o "$(MAKE_DIR)/chunk.o"
	$(CC) $(CFLAGS) -Wall -c chunk_manager.c -o "$(MAKE_DIR)/chunk_manager.o"

test:

clean:
	rm -f $(MAKE_DIR)/*.o
