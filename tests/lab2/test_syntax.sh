#!/bin/bash

CUR_DIR=$(dirname $(realpath $0))
TESTCASE_DIR=$CUR_DIR/testcase
OUTPUT_DIR=$CUR_DIR/token
BUILD_DIR=$CUR_DIR/../../build

mkdir -p $OUTPUT_DIR

for testcase in $TESTCASE_DIR/*.cminus
do
    filename=$(basename $testcase)
    $BUILD_DIR/parser < $testcase > $OUTPUT_DIR/${filename%.cminus}.syntax_tree
done
