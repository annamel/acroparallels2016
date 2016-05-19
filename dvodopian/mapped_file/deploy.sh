#!/usr/bin/env bash

DIRS="chunk_manager key_set sorted_list"
DESTINATION="$HOME/Programming/Parallels/acroparallels2016/dvodopian/mapped_file/"

shopt -s extglob

cp -f ./out/Release/* ./out/
cp -Rf ./!(tests) "$DESTINATION"