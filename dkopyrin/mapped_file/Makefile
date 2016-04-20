MAKE_DIR := $(PWD)/out
DIRS := logger hash chunk_manager mf mapped_file
BUILDDIRS := $(DIRS:%=build-%)
INSTALLDIRS := $(DIRS:%=install-%)
CLEANDIRS := $(DIRS:%=clean-%)
TESTDIRS := $(DIRS:%=test-%)

CFLAGS := -DDEBUG_LOG -DDEBUG_STDERRLOG -g
export CFLAGS MAKE_DIR

all: make_dir_init $(BUILDDIRS)

make_dir_init:
	mkdir -p $(MAKE_DIR)

$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

clean: make_dir_init $(CLEANDIRS)
	rm -rf *.o
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

test: make_dir_init all $(TESTDIRS)
$(TESTDIRS):
	$(MAKE) -C $(@:test-%=%) test
