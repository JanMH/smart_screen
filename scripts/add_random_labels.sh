#!/bin/bash

numbers_start=0

project_root_dir=$(dirname $(dirname $(realpath $0)))

if (( $# == 1 )); then
numbers_start=$1
fi

$project_root_dir/build/nofare_app/nofare_app_client add-label $numbers_start 1
$project_root_dir/build/nofare_app/nofare_app_client add-label $(($numbers_start+1)) 1
$project_root_dir/build/nofare_app/nofare_app_client add-label $(($numbers_start+2))  2
$project_root_dir/build/nofare_app/nofare_app_client add-label $(($numbers_start+3))  2
$project_root_dir/build/nofare_app/nofare_app_client add-label $(($numbers_start+4))  2
$project_root_dir/build/nofare_app/nofare_app_client add-label $(($numbers_start+5))  2
$project_root_dir/build/nofare_app/nofare_app_client add-label $(($numbers_start+6))  1
