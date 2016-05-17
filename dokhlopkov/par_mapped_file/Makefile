MAKED := $(PWD)/out
DIR := mflib
BUILD := $(DIR:%=build-%)
CLEAN := $(DIR:%=clean-%)

CFLAGS += -Ofast
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


