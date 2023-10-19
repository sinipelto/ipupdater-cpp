# Dynamic IP Updater (C++ version)
C++ Implementation of the ip updater tool. Necessary for native implementation of updater as a executable binary (no script).

See the bash version for more details: https://github.com/Sinipelto/godaddy-ipupdater.

Uses curlpp as a wrapper for libcurl (http://www.curlpp.org/).

Curlpp uses libcurl (https://curl.haxx.se/libcurl/). Linked as static library. Either use lib(s) provided by your OS or add them in the lib/ dir.

## Build instructions ##

simply run `make` to build the app. Build files included in build dir and complete binary package and assets in bin/

For fresh build:
`make clean`
`make`

**Copyright (c) Sinipelto 2019**
