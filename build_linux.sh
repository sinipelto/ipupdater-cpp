#!/bin/sh

# Ensure PATH is set up correctly
# ENSURE g++  IN PATH!

# Remove old bin
rm -r bin/

# Ensure bin/ exists
mkdir bin

# Copy stuff from skeleton for the program
cp -rT skeleton/ bin/

# Compile the program with optimization using g++
g++ -Wall -Wextra -Wpedantic -o bin/updater -I . -I include -Ofast $(find . -iname '*.cpp') lib/libcurl.a
