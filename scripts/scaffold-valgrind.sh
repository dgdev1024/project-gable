#!/bin/bash

# Run the application scaffolding specified.
valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./build/bin/scaffold-$1/debug/scaffold-$1 $@
