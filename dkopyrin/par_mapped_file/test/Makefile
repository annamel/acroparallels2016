all:

test:
	$(CC) $(CFLAGS) -o "$(MAKE_DIR)/test_ht" test_ht.c "$(MAKE_DIR)/libmappedfile.a" -I../../../include/
	$(CC) $(CFLAGS) -o "$(MAKE_DIR)/test_ladder" test_ladder.c "$(MAKE_DIR)/libmappedfile.a" -I../../../include/
	$(CC) $(CFLAGS) -o "$(MAKE_DIR)/test_stairs" test_stairs.c "$(MAKE_DIR)/libmappedfile.a" -I../../../include/

clean:
	rm -f "$(MAKE_DIR)/test_ladder" "$(MAKE_DIR)/test_stairs" "$(MAKE_DIR)/test_pyramid" "$(MAKE_DIR)/test_ht_eq" "$(MAKE_DIR)/test_ht_par" "$(MAKE_DIR)/*.o"
