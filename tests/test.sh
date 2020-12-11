#!/bin/bash -x

./tests/tests st 1
./tests/tests st 2
./tests/tests mt 1 100
./tests/tests mt 2 10
