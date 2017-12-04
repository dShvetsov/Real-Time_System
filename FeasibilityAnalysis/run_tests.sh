#!/usr/bin/env bash

if [ $# -lt 1 ] ; then
    echo "Give programm to test"
    exit 1
fi

yesno_single_test() {
    test_file=$1
    expected_answer=$2
    if [ -n "$($program $test_file | grep $expected_answer)"  ] ; then
        result="\033[32m OK \033[0m"
        ret=0
    else
        result="\033[31m FAILED \033[0m"
        ret=1
    fi
    echo -en "Test : $test_file : [ $result ] \n"
    return 0
}

yesno_test_dir() {
    directory=$1
    expected=$2
    for f in $(ls ${directory}/*.xml) ; do
        yesno_single_test $f $expected
        if [ $? -ne 0 ] ; then
            let failed=$failed+1
        fi
        let total=$total+1
    done
}

export program=$1
test_dir="testdata"
export total=0
export failed=0

echo "Test not feasible"
yesno_test_dir ${test_dir}/NO "NO"
echo "Test feasible"
yesno_test_dir ${test_dir}/YES "YES"

echo "------------------------------"
echo "Total : $total, Failed : $failed"
