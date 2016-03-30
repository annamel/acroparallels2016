MAKE_DIR := $(PWD)/out
DIRS := logger hash chunk_manager mf
BUILDDIRS := $(DIRS:%=build-%)
INSTALLDIRS := $(DIRS:%=install-%)
CLEANDIRS := $(DIRS:%=clean-%)
TESTDIRS := $(DIRS:%=test-%)

CFLAGS := -DDEBUG_LOG -DDEBUG_STDERRLOG -g
export CFLAGS MAKE_DIR

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEANDIRS)
	rm -rf *.o
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

test: all $(TESTDIRS)
$(TESTDIRS):
	$(MAKE) -C $(@:test-%=%) test
