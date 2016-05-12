all:
	$(CC) $(CFLAGS) -Wall -c -std=gnu99 -Iinclude runtime/runtime.c runtime/rcu.c runtime/lwt.c runtime/mem.c runtime/random.c map/skiplist.c
	mv *.o $(MAKE_DIR)/

test:

clean:
	rm -f $(MAKE_DIR)/*.o
