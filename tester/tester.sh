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
if [ -z "PREC" ]; then
	PREC=""
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
		out_dir="$root_lib_dir/$MF_SUFFIX/$LIBOUT_SUFFIX"
		mkdir -p $out_dir
		func_name="it_check_build_$(basename $root_lib_dir)"
		echo "$func_name() {" >> $test_file
		echo "    pushd $make_dir" >> $test_file
		if [ -f $make_dir/CMakeLists.txt ]; then
			echo "    cmake ." >> $test_file
			echo ""
		fi
		echo "    make" >> $test_file
		echo "    popd" >> $test_file
		echo "}" >> $test_file
		echo "" >> $test_file
		for root_test_dir in $ROOT_TEST_DIR ; do
			for test in $root_test_dir/$MF_SUFFIX/$TEST_SUFFIX/*.c ; do
			if [ -f $test ]; then
				func_name="it_check_$(basename $root_lib_dir)_by_$(basename $root_test_dir)_$(basename $test .c)"
				test_out_name="$out_dir/$(basename $test .c)"
				test_object_name="$test_out_name.o"
				echo "$func_name() {" >> $test_file
				echo "    gcc $CFLAGS -I$PWD/../include -c -o $test_object_name $test" >> $test_file
				echo "    g++ -g -o $test_out_name $test_object_name -L$out_dir -lmappedfile -lm -lrt -lpthread" >> $test_file
				echo "    set -x" >> $test_file
				echo "    $PREC $test_out_name $PWD/small.txt $PWD/out.txt" >> $test_file
				echo "    $PREC $test_out_name $PWD/medium.txt $PWD/out.txt" >> $test_file
				echo "    $PREC $test_out_name $PWD/gpl.txt $PWD/out.txt" >> $test_file
				echo "    set +x" >> $test_file
				echo "    rm -f $test_out_name" >> $test_file
				echo "    rm -f $test_object_name" >> $test_file
				echo "}" >> $test_file
				echo "" >> $test_file
			fi
			done

			for test in $root_test_dir/$MF_SUFFIX/$TEST_SUFFIX/*.cpp ; do
			if [ -f $test ]; then
				func_name="it_check_$(basename $root_lib_dir)_by_$(basename $root_test_dir)_$(basename $test .cpp)"
				test_out_name="$out_dir/$(basename $test .cpp)"
				test_object_name="$test_out_name.o"
				echo "$func_name() {" >> $test_file
				echo "    g++ -std=c++14 $CFLAGS -I$PWD/../include -c -o $test_object_name $test" >> $test_file
				echo "    g++ -g -o $test_out_name $test_object_name -L$out_dir -lmappedfile -lm -lrt -lpthread" >> $test_file
				echo "    set -x" >> $test_file
				echo "    $PREC $test_out_name $PWD/small.txt $PWD/out.txt" >> $test_file
				echo "    $PREC $test_out_name $PWD/medium.txt $PWD/out.txt" >> $test_file
				echo "    $PREC $test_out_name $PWD/gpl.txt $PWD/out.txt" >> $test_file
				echo "    set +x" >> $test_file
				echo "    rm -f $test_out_name" >> $test_file
				echo "}" >> $test_file
				echo "" >> $test_file
			fi
			done
		done
		func_name="it_make_clean_$(basename $root_lib_dir)"
		echo "$func_name() {" >> $test_file
		echo "    pushd $make_dir" >> $test_file
		#echo "    make clean" >> $test_file
		echo "    popd" >> $test_file
		echo "}" >> $test_file
		echo "" >> $test_file
	fi
done

$PWD/roundup $test_file
rm -f $PWD/file.txt
