SUBDIRS = libmf unittests tests

all:
	@set -e; \
	for d in $(SUBDIRS); do $(MAKE) -C $$d all ; done;
	cp ./libmf/libmf.a ./out/libmappedfile.a

clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d clean ; done
