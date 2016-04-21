SUBDIRS = libmf unittests tests

all:
	@set -e; \
	for d in $(SUBDIRS); do $(MAKE) -C $$d all ; done

clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d clean ; done
