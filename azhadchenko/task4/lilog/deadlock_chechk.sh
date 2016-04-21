#!/bin/bash

gcc -std=gnu99 -pthread pthread_test.c
for run in {1..10000}
do
	./a.out
done
