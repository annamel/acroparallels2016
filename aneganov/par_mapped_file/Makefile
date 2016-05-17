SUBDIRS = libmf

all:
	@set -e; \
	for d in $(SUBDIRS); do $(MAKE) -C $$d all ; done;
	mkdir -p ./out
	cp ./libmf/libmf.a ./out/libmappedfile.a

clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d clean ; done
	rm -f ./out/*
