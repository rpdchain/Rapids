#!/bin/bash
sudo apt-get install build-essential libtool bsdmainutils autotools-dev autoconf pkg-config automake python3
sudo apt-get install libgmp-dev libevent-dev libboost-all-dev libsodium-dev cargo
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:pivx/pivx
sudo apt-get update
cd

wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
tar -xzvf db-4.8.30.NC.tar.gz
cd db-4.8.30.NC/build_unix/
../dist/configure --enable-cxx
cd
sed -i 's/__atomic_compare_exchange/__atomic_compare_exchange_db/g' db-4.8.30.NC/dbinc/atomic.h
cd db-4.8.30.NC/build_unix/
make
make install
cd

sudo apt-get install libssl-dev
sudo apt-get install libqt5gui5 libqt5core5a libqt5dbus5 libqt5svg5-dev libqt5charts5-dev qttools5-dev qttools5-dev-tools libqrencode-dev
sudo apt install protobuf-compiler
cd Rapids/scripts/build
./compile-linux.sh