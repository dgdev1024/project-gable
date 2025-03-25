#!/bin/bash

# Check to see if the "./res" directory exists. If it doesn't, print an error message and exit.
if [ ! -d "./res" ]; then
    echo "GABUILD: The \"./res\" directory does not exist."
    exit 1
fi

# Set the default mode to "debug".
MODE="debug"

# If the first argument is "release", set the mode to "release".
if [ "$1" == "release" ]; then
    MODE="release"
fi

# Iterate over all the files in the "./res" directory, and all its subdirectories, and its
# subdirectories' subdirectories, etc. and print the path of each file.
for file in $(find ./res -type f); do

    # Check the extension of the file. If it's not ".asm", skip the file.
    if [ ${file: -4} != ".asm" ]; then
        continue
    fi

    echo "GABUILD: Building $file..."
    
    # Get the folder path of the file. Replace "./res" with "./assets" in the path.
    folder=$(dirname $file | sed 's/^\.\/res/\.\/assets/')

    # Get the file name of the file. Replace ".asm" with ".bin" in the name.
    filename=$(basename $file | sed 's/\.asm/\.bin/')

    # Create the folder in the "./assets" directory if it doesn't exist.
    mkdir -p $folder

    # Run GABUILD on the resource file.
    ./build/bin/gabuild/$MODE/gabuild -i $file -o $folder/$filename

    # Early out if the command returned non-zero.
    if [ $? -ne 0 ]; then
        exit 1
    fi

done
