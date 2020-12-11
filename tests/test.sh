#!/bin/bash -x

./tests/tests_tc st 1
./tests/tests_tc st 2
./tests/tests_tc mt 1 100
./tests/tests_tc mt 2 10
