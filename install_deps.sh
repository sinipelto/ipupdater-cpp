#!/bin/sh

# Ensure any required libraries and tools are installed before building the binary
# Works on debian-based systems (aptitude and dpkg installed)

# Update repository metadata
sudo apt-get update

# Install required packages
sudo apt-get install -y curl
sudo apt-get install -y libcurl3-gnutls
sudo apt-get install -y libcurl4

#sudo apt-get install -y libcurl4-openssl-dev

#sudo apt-get install -y libcurlpp-dev
#sudo apt-get install -y libcurlpp0

echo "Dependencies installed."
