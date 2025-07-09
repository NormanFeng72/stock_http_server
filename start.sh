#!/bin/bash

export ASAN_OPTIONS="log_path=./asan.log"
export UBSAN_OPTIONS="log_path=./ubsan.log"
export LSAN_OPTIONS="log_path=./lsan.log"
# export LD_PRELOAD=/usr/lib/gcc/x86_64-linux-gnu/10/libasan.so
./http_server_test $1 $2
