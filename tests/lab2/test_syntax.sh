#!/bin/bash

CUR_DIR=$(dirname $(realpath $0))
TESTCASE_DIR=./testcase
OUTPUT_DIR=./token
BUILD_DIR=$CUR_DIR/../../build

mkdir -p $OUTPUT_DIR

for testcase in $TESTCASE_DIR/*.cminus
do
    $BUILD_DIR/parser < $testcase > $OUTPUT_DIR/$(basename $testcase).tokens
done
