#!/bin/bash

gcc -std=gnu99 -pthread pthread_test.c
./a.out info
cat info | more
