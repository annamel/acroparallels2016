include ../Makefile.inc

CFLAGS += -I../libmf

PROGS = test_chunkmanager test_log test_skiplist
OBJS=$(PROGS:=.o)
OBJS += memtest.o

LDFLAGS += -L../libmf
LDLIBS  += -lmf

ifeq (.depend,$(wildcard .depend))
include .depend
else
all: depend
endif

all: $(PROGS)

depend dep:
	$(CC) $(CFLAGS) -M $(OBJS:.o=.c) > .depend

.PHONY: clean
clean:
	rm -f $(PROGS) *.o *.a *.8 .depend
	rm -f *.gcda *.gcno
