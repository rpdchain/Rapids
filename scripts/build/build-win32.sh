cd ..
./autogen.sh
./configure LDFLAGS="-L`pwd`/db4/lib/" CPPFLAGS="-I`pwd`/db4/include/" --prefix=`pwd`/depends/i686-w64-mingw32 --disable-online-rust --disable-tests
make
cd src
cd qt
strip rapids-qt.exe