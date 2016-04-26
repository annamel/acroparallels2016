all:

test:
	$(CC) $(CFLAGS) -o "$(MAKE_DIR)/test-mf" test.c "$(MAKE_DIR)/libmappedfile.a" -I../../../include/
	$(CC) $(CFLAGS) -o "$(MAKE_DIR)/test-mf_map" test1.c "$(MAKE_DIR)/libmappedfile.a" -I../../../include/
clean:
	rm -f "$(MAKE_DIR)/test-mf" "$(MAKE_DIR)/test-mf_map" "$(MAKE_DIR)/*.o"
