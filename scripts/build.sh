#!/bin/bash


project_root_dir=$(dirname $(dirname $(realpath $0)))

git submodule update --init --recursive

build_dir=$project_root_dir/build
if [ ! -d $build_dir ]; then
    mkdir $build_dir
fi



libnabo_build_dir=$project_root_dir/external/libnabo/build
if [ ! -d $libnabo_build_dir ]; then
    mkdir $libnabo_build_dir
fi

cd $libnabo_build_dir
cmake ..
cmake --build . --config Release



cpp_netlib_build_dir=$project_root_dir/external/cpp-netlib/build
if [ ! -d $cpp_netlib_build_dir ]; then
    mkdir $cpp_netlib_build_dir
fi

cd $cpp_netlib_build_dir
cmake ..
cmake --build . --config Release


cd $build_dir
cmake ..
cmake --build . 

