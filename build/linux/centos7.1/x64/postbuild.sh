#!/bin/bash
base_path=$1
project=$2
version=$3
target=$4
release=$5

if [[ -z "${release}" ]]; then
    dst_path=${base_path}/${project}/v${version}/linux/centos7.1/x64/
else
    dst_path=${base_path}/${project}/v${version}/linux/centos7.1/x64_release/
fi

rm -rf ${dst_path}
mkdir -p ${dst_path}include
mkdir -p ${dst_path}lib

\cp -r ../../../../include/* ${dst_path}include/
\cp -r ${target} ${dst_path}lib/
