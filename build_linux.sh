#!bin/sh

# Ensure bin/ exists
mkdir bin

# Clean bin/ of any old stuff
rm -r bin/*

# Copy stuff from skeleton for the program
cp -R skeleton/* bin/

# Compile the program with optimization using g++
g++ -lcurl -o bin/updater -I . -Ofast *.cpp internal/*.cpp
