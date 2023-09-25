#!/bin/bash

lib_path=./lib
export LD_LIBRARY_PATH=$lib_path:$LD_LIBRARY_PATH

resource_dir=./resource
threads_num=1
wavlist=$1
result_path=$2

./bin/asr_test --threads_num $threads_num --resource_dir $resource_dir --wavlist $wavlist --result_path $result_path
