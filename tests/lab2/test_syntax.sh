#!/bin/bash

# DO NOT MODIFY!
# If you need customized behavior, please create your own script.

CUR_DIR=$(dirname $(realpath $0))
TESTCASE=${1:-testcase}
TESTCASE_DIR=$CUR_DIR/$TESTCASE
OUTPUT_DIR=$CUR_DIR/syntree_$TESTCASE
BUILD_DIR=$CUR_DIR/../../build

# Make sure $OUTPUT_DIR exists.
mkdir -p $OUTPUT_DIR

# Generate a .syntax_tree file for all .cminus files in the testcase dir.
for testcase in $TESTCASE_DIR/*.cminus
do
    filename=$(basename $testcase)
    echo "[info] Analyzing $filename"
    $BUILD_DIR/parser < $testcase > $OUTPUT_DIR/${filename%.cminus}.syntax_tree
done

# Output a summary if the basic test suite is used.
if [[ ${2:-no} != "no" ]]; then
    echo "[info] Comparing..."
    if [[ ${2:-no} == "verbose" ]]; then
        DIFF_FLAGS=-qr
    fi
    diff ${DIFF_FLAGS} $OUTPUT_DIR ${OUTPUT_DIR}_std
    if [ $? ]; then
        echo "[info] No difference! Congratulations!"
    else
        echo "[info] Differences spotted."
    fi
fi
