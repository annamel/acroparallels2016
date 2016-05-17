CC              =  gcc
CFLAGS          = -Wall -g -O0
GCOV_FLAGS      = -fprofile-arcs -ftest-coverage

ifeq ($(GCOV),yes)
       CFLAGS += $(GCOV_FLAGS)
endif

SOURCES         = error.c hash_table.c $(EXECUTABLE).c
OBJECTS         = $(SOURCES:.c=.o)
EXECUTABLE      = test_hash


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c -MD $(CFLAGS) -o $@ $<

-include $(wildcard *.d)

valgrind:
	valgrind ./$(EXECUTABLE)

clean:
	rm -f *.o *.d

clean_wexec:
	rm -f *.o *.d $(EXECUTABLE)

clean_gcov:
	rm -f *.g*

clean_all:
	rm -f *.o *.d $(EXECUTABLE) *.g*



.PHONY: all clean

