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

LDFLAGS=$LDFLAGS\ "-lmappedfile -lm -lrt -lpthread"
UNAME=$(uname)
if [ $UNAME == "Darwin" ]; then
	SAVEIFS=$IFS
	IFS=$(echo -en "\n\b")
	CFLAGS=$CFLAGS\ -DNORT
else
	LDFLAGS=$LDFLAGS\ -lrt
fi

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
test_file=$(mktemp /tmp/tmp.XXXXXXXXXX)
echo $test_file
echo "#!$PWD/roundup" > $test_file
echo "" >> $test_file
echo 'SAVEIFS=$IFS' >> $test_file
echo 'IFS=$(echo -en "\n\b")' >> $test_file
echo "describe \"Test Mapped File\"" >> $test_file

LIB_SOURCE_DIR=$PWD/../**
if [ -z "$ROOT_LIB_DIR" ]; then
	ROOT_LIB_DIR=$(ls -d -1 $LIB_SOURCE_DIR)
fi
TEST_SOURCE_DIR=$PWD/../**
if [ -z "$ROOT_TEST_DIR" ]; then
	ROOT_TEST_DIR=$(ls -d -1 $TEST_SOURCE_DIR)
fi
if [ -z "PREC" ]; then
	PREC=""
fi
INCLUDE_DIR="$PWD/../include"



MF_SUFFIX="mapped_file"
TEST_SUFFIX="test"
LIBMAKE_SUFFIX="."
LIBOUT_SUFFIX="out"
TEST_BUILD=""

for root_lib_dir  in $ROOT_LIB_DIR  ; do
	make_dir="$root_lib_dir/$MF_SUFFIX/$LIBMAKE_SUFFIX/"
	lib_dir="$root_lib_dir/$MF_SUFFIX/$LIBOUT_SUFFIX/"
	if [ -d $make_dir ]; then
		out_dir="$root_lib_dir/$MF_SUFFIX/$LIBOUT_SUFFIX"
		echo "pushd '$make_dir'" >> $test_file
		if [ -f $make_dir/CMakeLists.txt ]; then
			echo "cmake ." >> $test_file
			echo ""
		fi
		echo "make" >> $test_file
		echo "popd" >> $test_file
		echo "" >> $test_file
		for root_test_dir in $ROOT_TEST_DIR ; do
			for test in $root_test_dir/$MF_SUFFIX/$TEST_SUFFIX/*.c ; do
			if [ -f $test ]; then
				func_name="it_check_$(basename $root_lib_dir)_by_$(basename $root_test_dir)_$(basename $test .c)"
				test_out_name="$out_dir/$(basename $test .c)"
				test_object_name="$test_out_name.o"

				TEST_BUILD=$TEST_BUILD\ $test_out_name\ $test_object_name
				echo "$func_name() {" >> $test_file
				echo "    gcc $CFLAGS -I'$PWD/../include' -c -o '$test_object_name' $LDFLAGS '$test'" >> $test_file
				echo "    g++ $CFLAGS -g -o '$test_out_name' '$test_object_name' $LDFLAGS -L'$out_dir'" >> $test_file
				echo "    set -x" >> $test_file
				echo "    $PREC '$test_out_name' '$PWD/small.txt' '$PWD/out.txt'" >> $test_file
				echo "    $PREC '$test_out_name' '$PWD/medium.txt' '$PWD/out.txt'" >> $test_file
				echo "    $PREC '$test_out_name' '$PWD/gpl.txt' '$PWD/out.txt'" >> $test_file
				echo "    set +x" >> $test_file
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
				echo "    g++ $CFLAGS -std=c++14 $CFLAGS -I'$PWD/../include' -c -o '$test_object_name' $LDFLAGS '$test'" >> $test_file
				echo "    g++ $CFLAGS -g -o '$test_out_name' '$test_object_name' $LDFLAGS  -L'$out_dir'" >> $test_file
				echo "    set -x" >> $test_file
				echo "    $PREC '$test_out_name' '$PWD/small.txt' '$PWD/out.txt'" >> $test_file
				echo "    $PREC '$test_out_name' '$PWD/medium.txt' '$PWD/out.txt'" >> $test_file
				echo "    $PREC '$test_out_name' '$PWD/gpl.txt' '$PWD/out.txt'" >> $test_file
				echo "    set +x" >> $test_file
				echo "}" >> $test_file
				echo "" >> $test_file

				echo "rm -rf '$PWD/out.txt'" >> $test_file
				echo "rm -rf '$test_out_name'" >> $test_file
				echo "rm -rf '$test_object_name'" >> $test_file
				echo "" >> $test_file
			fi
			done
		done
	fi
done

$PWD/roundup $test_file

for root_lib_dir in $ROOT_LIB_DIR  ; do
	make_dir="$root_lib_dir/$MF_SUFFIX/$LIBMAKE_SUFFIX/"
	pushd $make_dir > /dev/null
	make clean > /dev/null
	popd > /dev/null
done
for clean_file in $TEST_BUILD; do
	rm -f $clean_file
done

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
rm -f $PWD/out.txt

if [ $UNAME == "Darwin" ]; then
	IFS=$SAVEIFS
fi
