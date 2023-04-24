#!/bin/sh

if ! [ -e bin ];then
    mkdir bin -p
fi

gcc -o mksensor mksensor.c -I../../adi/include -I../../image/image_lib/include
if [ $? -ne 0 ];then 
exit 1
fi
./mksensor
if [ $? -ne 0 ];then 
exit 1
fi
rm mksensor
#if ! [ -e ${ROOTFS_TOP}/usr/local/bin/sensors ];then
#    mkdir ${ROOTFS_TOP}/usr/local/bin/sensors -p
#fi
#cp bin/*.bin ${ROOTFS_TOP}/usr/local/bin/sensors/ -f
#rm *.bin
