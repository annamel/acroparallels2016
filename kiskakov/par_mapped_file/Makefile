 MAKED := $(PWD)/out
DIR := par_mapped_file_sources
BUILD := $(DIR:%=build-%)
CLEAN := $(DIR:%=clean-%)

export CFLAGS MAKED

all: build_all $(BUILD)
	ar rcs "$(PWD)/out/libmappedfile.a" "$(PWD)/out"/*.o

build_all:
	mkdir -p "$(MAKED)"

$(DIR): $(BUILD)
$(BUILD):
	$(MAKE) -C $(@:build-%=%)

clean: build_all $(CLEAN)
	rm -rf "$(PWD)/out/"*.o
	rm -rf "$(PWD)/out/libmappedfile.a"

$(CLEAN):
	$(MAKE) -C $(@:clean-%=%) clean


