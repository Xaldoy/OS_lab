#!/bin/bash

if [ $# -eq 0 ]; then
	echo "Usage: $0 <filename.cpp>"
	exit 1
fi

filename=$(basename -- "$1")
filename_no_ext="${filename%.*}"

g++ -o "$filename_no_ext" "$filename"

if [ $? -eq 0 ]; then
	echo "Compilation succesful. Running the program..."
	./"$filename_no_ext"
else 
	echo "Compilation failed."
fi
