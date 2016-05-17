include ../Makefile.inc

CFLAGS += -I../../../include
LIBMF = libmf.a
LIBOBJS = map.o mf_malloc.o log.o chunk_manager.o mf_iterator.o mapped_file.o skiplist.o

ifeq (.depend,$(wildcard .depend))
include .depend
else
all: depend
endif

all: $(LIBMF)

$(LIBMF): $(LIBOBJS)
	$(AR) rcs $(LIBMF) $(LIBOBJS)

depend dep:
	$(CC) $(CFLAGS) -M $(LIBOBJS:.o=.c) > .depend

clean:
	rm -f *.o *.a *.8 .depend
	rm -f *.gcda *.gcno *.gcov
