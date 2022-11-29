#!/bin/bash
cd ../..
./autogen.sh
./configure --disable-gui-tests --disable-tests --with-gui=qt5 CPPFLAGS="-I/usr/local/BerkeleyDB.4.8/include -O2" LDFLAGS="-L/usr/local/BerkeleyDB.4.8/lib"
make