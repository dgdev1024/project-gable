#!/bin/bash

# The first argument is the subfolder within the "./res" directory to build/copy files from.
# If it's not provided, print an error message and exit.
SUBFOLDER=$1
if [ -z "$SUBFOLDER" ]; then
    echo "GABUILD: No subfolder specified."
    exit 1
fi

# The second argument is the mode to build in, defaulting to "debug".
MODE="debug"

# If the second argument is "release", set the mode to "release".
if [ "$2" == "release" ]; then
    MODE="release"
fi

# Check to see if the "./res" directory, and the specified subfolder, exist.
# If they don't, print an error message and exit.
if [ ! -d "./res" ]; then
    echo "GABUILD: The folder \"./res\" does not exist. Skipping..."
    exit 0
elif [ ! -d "./res/$SUBFOLDER" ]; then
    echo "GABUILD: The folder \"./res/$SUBFOLDER\" does not exist. Skipping..."
    exit 0
fi

# Iterate over all the files in the "./res/$SUBFOLDER" directory, and all its subdirectories, and its
# subdirectories' subdirectories, etc. and print the path of each file.
for file in $(find ./res/$SUBFOLDER -type f); do

    # Check the extension of the file. If it's not ".inc", ".gbd", ".asm" or ".s", then assume that the file is a
    # binary file and copy it to the "./assets" directory.
    if [ ${file: -4} != ".asm" ] && [ ${file: -2} != ".s" ] && [ ${file: -4} != ".inc" ] && [ ${file: -4} != ".gbd" ]; then
        echo "GABUILD: Copying $file..."
        
        # Get the folder path of the file. Replace "./res" with "./assets" in the path.
        folder=$(dirname $file | sed 's/^\.\/res/\.\/assets/')

        # Create the folder in the "./assets" directory if it doesn't exist.
        mkdir -p $folder

        # Copy the file to the "./assets" directory.
        cp $file $folder

        # Early out if the command returned non-zero.
        if [ $? -ne 0 ]; then
            exit 1
        fi
    else
        echo "GABUILD: Building $file..."
        
        # Get the folder path of the file. Replace "./res" with "./assets" in the path.
        folder=$(dirname $file | sed 's/^\.\/res/\.\/assets/')

        # Get the file name of the file. Replace ".asm" with ".bin" in the name.
        filename=$(basename $file | sed 's/\.asm/\.bin/' | sed 's/\.s/\.bin/')

        # Create the folder in the "./assets" directory if it doesn't exist.
        mkdir -p $folder

        # Run GABUILD on the resource file.
        ./build/bin/gabuild/$MODE/gabuild -i $file -o $folder/$filename

        # Early out if the command returned non-zero.
        if [ $? -ne 0 ]; then
            exit 1
        fi
    fi

done
