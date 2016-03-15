CFLAGS = -Wall -O2 -Wextra -c
LDFLAGS = -rdynamic

all: test.o log.o
	$(CC) test.o log.o -o logtest $(LDFLAGS)

log.o: log.c
	$(CC) $(CFLAGS) -DLOG_ON $<

test.o: test.c
	$(CC) $(CFLAGS) -DLOG_ON $<

release: test-release
	$(CC) test.o -o logtest

test-release: test.c
	$(CC) $(CFLAGS) test.c

clean:
	rm -rf *.o logtest

.PHONY: all clean
