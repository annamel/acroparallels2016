#!/bin/sh

if [ $# -gt 0 ]; then
	ROOT_LIB_DIR="../$1"
else
	ROOT_LIB_DIR=../enikitenko
fi

ROOT_TEST_DIR=../enikitenko

(cd ../../../tester && ROOT_TEST_DIR=$ROOT_TEST_DIR ROOT_LIB_DIR=$ROOT_LIB_DIR ./tester.sh)
