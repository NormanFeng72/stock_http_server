#!/bin/bash

cd examples

rm -r -f coverage

gcov -b -c http_server_test.cpp

lcov -c -d . --rc lcov_branch_coverage=1 -o coverage.info

genhtml --rc genhtml_branch_coverage=1 -o coverage coverage.info

chmod -R a+wr coverage

cd ..
