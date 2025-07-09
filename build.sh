#!/bin/bash

# apt update
# apt install openssl -y
# apt install libssl-dev -y
find ./ -name "*.gc*" -exec rm -f {} \;

make clean

# ./configure --with-openssl
./configure --without-openssl 
./configure CFLAGS="-fsanitize=address" CXXFLAGS="-fsanitize=address"
# --without-openssl --enable_asan
# make CC=abf-gcc CXX=abf-g++
make CC=gcc CXX=g++

#export GCOV_PREFIX=/home/norman/code/libhv-master/bin
#export GCOV_PREFIX=${PWD} 
#export GCOV_PREFIX_STRIP=3

cp bin/http_server_test ./http_server_test


export ASAN_OPTIONS="log_path=./asan.log"
export UBSAN_OPTIONS="log_path=./ubsan.log"
export LSAN_OPTIONS="log_path=./lsan.log"
# export LD_PRELOAD=/usr/lib/gcc/x86_64-linux-gnu/10/libasan.so
