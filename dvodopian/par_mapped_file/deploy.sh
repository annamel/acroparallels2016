#!/usr/bin/env bash


DESTINATION="$HOME/Programming/Parallels/acroparallels2016/dvodopian/par_mapped_file"

shopt -s extglob

cp -Rf ./!(tests|out) "$DESTINATION/"