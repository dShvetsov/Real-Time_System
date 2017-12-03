#!/usr/bin/env bash

if [ $# -lt 1 ] ; then
    echo "Give programm to test"
    exit 1
fi

yesno_single_test() {
    test_file=$1
    expected_answer=$2
    if [ -n $($program $test_file | grep $expected_answer)  ] ; then
        echo "OK"
    else
        echo "FAILED"
    fi
    return 0
}

yesno_test_dir() {
    directory=$1
    expected=$2
    for f in $(ls ${directory}/*.xml) ; do
        yesno_single_test $f $expected
    done
}

program=$1
test_dir="testdata"

yesno_test_dir ${test_dir}/NO "NO"
yesno_test_dir ${test_dir}/YES "YES"

