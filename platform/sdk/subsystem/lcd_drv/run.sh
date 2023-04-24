#!/bin/sh

if ! [ -e bin ];then
    mkdir bin -p
fi

gcc -o mklcd mklcd.c -I../../adi/include -I../../image/image_lib/include
./mklcd
rm mklcd
if ! [ -e ${ROOTFS_TOP}/usr/local/bin/display ];then
    mkdir ${ROOTFS_TOP}/usr/local/bin/display -p
fi
cp bin/*.bin ${ROOTFS_TOP}/usr/local/bin/display/ -f
#rm *.bin
