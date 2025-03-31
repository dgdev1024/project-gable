#!/bin/bash

# Assume the default install path for the executable is "/usr/local/bin", and the default install
# path for its libraries is "/usr/local/lib". You can change these paths as needed.
INC_DIR="/usr/local/include"
BIN_DIR="/usr/local/bin"
LIB_DIR="/usr/local/lib"

# Check if the user has provided a custom install path for the include files (--inc <path>)
if [[ $1 == "--inc" ]]; then
    # Expects the second argument to be the custom path
    if [[ -z $2 ]]; then
        echo "Error: No path provided for --inc option."
        exit 1
    fi

    INC_DIR=$2
    shift 2
fi

# Check if the user has provided a custom install path for the libraries (--lib <path>)
if [[ $1 == "--lib" ]]; then
    # Expects the second argument to be the custom path
    if [[ -z $2 ]]; then
        echo "Error: No path provided for --lib option."
        exit 1
    fi

    LIB_DIR=$2
    shift 2
fi

# Check if the user has provided a custom install path for the executable (--bin <path>)
if [[ $1 == "--bin" ]]; then
    # Expects the second argument to be the custom path
    if [[ -z $2 ]]; then
        echo "Error: No path provided for --bin option."
        exit 1
    fi

    BIN_DIR=$2
    shift 2
fi

# Install the executable
echo "Installing GABUILD to $BIN_DIR..."
cp ./build/bin/gabuild/release/gabuild $BIN_DIR
if [[ $? -ne 0 ]]; then
    echo "Error: Failed to copy gabuild to $BIN_DIR."
    exit 1
fi

# Install the libraries
echo "Installing GABLE library to $LIB_DIR..."
cp ./build/bin/gable/release/libgable.so $LIB_DIR
if [[ $? -ne 0 ]]; then
    echo "Error: Failed to copy libgable.so to $LIB_DIR."
    exit 1
fi

# Install the includes
echo "Installing GABLE include files to $INC_DIR..."

cp -R ./projects/gable/include/GABLE $INC_DIR

# Run ldconfig to update the library cache
echo "Updating library cache..."
ldconfig

echo "Installation OK."
