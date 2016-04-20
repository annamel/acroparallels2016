#!/bin/sh

test_file=$(mktemp)
echo $test_file
echo "#!$(pwd)/roundup" > $test_file
echo "" >> $test_file
echo "describe \"Test Mapped File\"" >> $test_file

MF_SUFFIX="mapped_file"
TEST_SUFFIX="test"
LIBMAKE_SUFFIX="."
LIBOUT_SUFFIX="out"

LIB_SOURCE_DIR="$(pwd)/../"
TEST_SOURCE_DIR="$(pwd)/../"

for root_lib_dir  in $LIB_SOURCE_DIR/*/ ; do
for root_test_dir in $LIB_SOURCE_DIR/*/ ; do
for test in $root_test_dir/$MF_SUFFIX/$TEST_SUFFIX/* ; do
	if [ -f $test ]; then
		make_dir="$root_lib_dir/$MF_SUFFIX/$LIBMAKE_SUFFIX/"
		lib_dir="$root_lib_dir/$MF_SUFFIX/$LIBOUT_SUFFIX/"
		func_name="it_check_$(basename $root_lib_dir)_by_$(basename $root_test_dir)_$(basename $test .c)"
		echo "$func_name() {" >> $test_file
		echo "    pushd $make_dir" >> $test_file
		echo "    make" >> $test_file
		echo "    "
		echo "    popd" >> $test_file
		echo "}" >> $test_file
		echo "" >> $test_file
	fi
done
done
done
