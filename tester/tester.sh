#!/bin/sh

# Made by Denis Kopyrin
# This script goes through all dirs in ../ and makes cross-people tests
# Project is expected to be built by make at $USERNAME/mapped_file/Makefile
#at dir $USERNAME/mapped_file/out/
# Test files are build automatically and sources are expected to be at
#$USERNAME/mapped_file/test/
# Changing ROOT_LIB_DIR and you can process only users with such name
#(like ROOT_LIB_DIR="../dkopyrin ../ivanov")
# The same way you can change variable ROOT_TEST_DIR
# CFLAGS are supported

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
test_file=$(mktemp)
echo $test_file
echo "#!$PWD/roundup" > $test_file
echo "" >> $test_file
echo "describe \"Test Mapped File\"" >> $test_file

LIB_SOURCE_DIR="$PWD/../"
if [ -z "$ROOT_LIB_DIR" ]; then
	ROOT_LIB_DIR=$(ls -d -1 $LIB_SOURCE_DIR/**)
fi
TEST_SOURCE_DIR="$PWD/../"
if [ -z "$ROOT_TEST_DIR" ]; then
	ROOT_TEST_DIR=$(ls -d -1 $TEST_SOURCE_DIR/**)
fi
INCLUDE_DIR="$PWD/../include"

MF_SUFFIX="mapped_file"
TEST_SUFFIX="test"
LIBMAKE_SUFFIX="."
LIBOUT_SUFFIX="out"

for root_lib_dir  in $ROOT_LIB_DIR  ; do
	make_dir="$root_lib_dir/$MF_SUFFIX/$LIBMAKE_SUFFIX/"
	lib_dir="$root_lib_dir/$MF_SUFFIX/$LIBOUT_SUFFIX/"
	if [ -d $make_dir ]; then
	if [ -d $lib_dir ]; then
		for root_test_dir in $ROOT_TEST_DIR ; do
		for test in $root_test_dir/$MF_SUFFIX/$TEST_SUFFIX/* ; do
		if [ -f $test ]; then
			func_name="it_check_$(basename $root_lib_dir)_by_$(basename $root_test_dir)_$(basename $test .c)"
			out_dir="$root_lib_dir/$MF_SUFFIX/$LIBOUT_SUFFIX"
			mkdir -p $out_dir
			echo "$func_name() {" >> $test_file
			echo "    pushd $make_dir" >> $test_file
			echo "    make" >> $test_file
			echo "    gcc $CFLAGS -I$PWD/../include -o $out_dir/test $test $out_dir/*.o"
			echo "    popd" >> $test_file
			echo "}" >> $test_file
			echo "" >> $test_file
		fi
		done
		done
	fi
	fi
done

./roundup $test_file
