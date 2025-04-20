#!/bin/bash

set -e

# 如果没有build目录，则创建一个
if [ ! -d `pwd`/build ];then
    mkdir -p `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

cd ..

if [ ! -d /usr/include/testmymuduo ];then
    mkdir -p /usr/include/testmymuduo
fi

for header in `ls *.h`;do
    cp $header /usr/include/testmymuduo
done

cp `pwd`/lib/libMyMuduo.so /usr/lib

ldconfig