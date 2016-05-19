CC              =  gcc
CFLAGS          = -Wall -g3 -O0 -pthread

SOURCES         = ../error/error.c ../ring_buf/ring_buf.c ../logger/logger.c $(EXECUTABLE).c
OBJECTS         = $(SOURCES:.c=.o)
DEPENDENCIES    = $(SOURCES:.c=.d)
EXECUTABLE      = test_logger


# delete 'clean'
all: $(EXECUTABLE) clean

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c -MD $(CFLAGS) -o $@ $<

-include $(wildcard *.d)

valgrind:
	valgrind ./$(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(DEPENDENCIES)

clean_all:
	rm -f $(OBJECTS) $(DEPENDENCIES) $(EXECUTABLE)



.PHONY: all clean

