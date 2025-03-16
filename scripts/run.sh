#!/bin/bash

# Default mode flag (debug)
MODE="debug"

# Change mode flag if provided in an environment variable (eg. MODE=release ./scripts/run.sh ...)
if [ -n "$MODE" ]; then
  MODE=$MODE
fi

# Run the application, passing in all arguments except the first.
./build/bin/$1/$MODE/$1 "${@:2}"
