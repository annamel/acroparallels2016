#!/usr/bin/env bash


DESTINATION="$HOME/Programming/Parallels/acroparallels2016/dvodopian/mapped_file"

shopt -s extglob

cp -Rf ./!(tests|out) "$DESTINATION/"