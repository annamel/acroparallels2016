SUBDIRS = libmf unittests test

all:
	@set -e; \
	for d in $(SUBDIRS); do $(MAKE) -C $$d all ; done;
	cp ./libmf/libmf.a ./out/libmappedfile.a

clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d clean ; done
	rm ./out/*
