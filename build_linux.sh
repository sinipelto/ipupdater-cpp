#!/bin/sh

# Ensure bin/ exists
mkdir bin || true

# Clean bin/ of any old stuff
rm -r bin/*
rm -r bin/.*

# Copy stuff from skeleton for the program
cp -rT skeleton/ bin/

# Compile the program with optimization using g++
g++ -Wall -Wextra -Wpedantic -o bin/updater -I . -I include -Ofast "$(find . -iname '*.cpp')" libcurl.a
