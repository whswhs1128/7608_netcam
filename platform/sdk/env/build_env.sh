#!/bin/sh

cd $(dirname ${BASH_SOURCE[0]})

#if [ ! -f ../Config.mak.$USER ]; then
#	echo -e "\033[31m Config.mak.$USER don't exist!\033[0m"
#	cd - > /dev/null
#	return 1
#fi

if [ -f ../Config.mak.$USER ]; then
SDK_TOP_TEMP=`awk -F = '/^ *SDK_TOP/{print $2}' ../Config.mak.$USER | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi
if [ -z $SDK_TOP_TEMP ]; then
SDK_TOP_TEMP=`awk -F = '/^ *SDK_TOP/{print $2}' ../Config.mak | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi
if [ -z $SDK_TOP_TEMP ]; then
    echo -e "\033[31m please set path of the SDK(SDK_TOP)!\033[0m"
    cd - > /dev/null
	return 1
fi

export SDK_TOP=`eval echo $SDK_TOP_TEMP`

if [ -f ../Config.mak.$USER ]; then
KERNEL_VER_TEMP=`awk -F = '/^ *KERNEL_VER/{print $2}' ../Config.mak.$USER | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi
if [ -z $KERNEL_VER_TEMP ]; then
KERNEL_VER_TEMP=`awk -F = '/^ *KERNEL_VER/{print $2}' ../Config.mak | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi

KERNEL_VER=`eval echo $KERNEL_VER_TEMP`

if [ -f ../Config.mak.$USER ]; then
GCC_VER_TEMP=`awk -F = '/^ *GCC_VER/{print $2}' ../Config.mak.$USER | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi
if [ -z $GCC_VER_TEMP ]; then
GCC_VER_TEMP=`awk -F = '/^ *GCC_VER/{print $2}' ../Config.mak | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi

GCC_VER=`eval echo $GCC_VER_TEMP`

if [ -f ../Config.mak.$USER ]; then
ROOTFS_PARENT_DIRECTORY_TEMP=`awk -F = '/^ *ROOTFS_PARENT_DIRECTORY/{print $2}' ../Config.mak.$USER | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi
if [ -z $ROOTFS_PARENT_DIRECTORY_TEMP ]; then
ROOTFS_PARENT_DIRECTORY_TEMP=`awk -F = '/^ *ROOTFS_PARENT_DIRECTORY/{print $2}' ../Config.mak | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi

ROOTFS_PARENT_DIRECTORY=`eval echo $ROOTFS_PARENT_DIRECTORY_TEMP`

if [ -f ../Config.mak.$USER ]; then
TOOLCHAIN_PARENT_DIRECTORY_TEMP=`awk -F = '/^ *TOOLCHAIN_PARENT_DIRECTORY/{print $2}' ../Config.mak.$USER | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi
if [ -z $TOOLCHAIN_PARENT_DIRECTORY_TEMP ]; then
TOOLCHAIN_PARENT_DIRECTORY_TEMP=`awk -F = '/^ *TOOLCHAIN_PARENT_DIRECTORY/{print $2}' ../Config.mak | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi

TOOLCHAIN_PARENT_DIRECTORY=`eval echo $TOOLCHAIN_PARENT_DIRECTORY_TEMP`

if [ -f ../Config.mak.$USER ]; then
KERNEL_TOP_TEMP=`awk -F = '/^ *KERNEL_TOP/{print $2}' ../Config.mak.$USER | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi
if [ -z $KERNEL_TOP_TEMP ]; then
KERNEL_TOP_TEMP=`awk -F = '/^ *KERNEL_TOP/{print $2}' ../Config.mak | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi

export KERNEL_TOP=`eval echo $KERNEL_TOP_TEMP`

if [ -f ../Config.mak.$USER ]; then
UBOOT_TOP_TEMP=`awk -F = '/^ *UBOOT_TOP/{print $2}' ../Config.mak.$USER | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi
if [ -z $UBOOT_TOP_TEMP ]; then
UBOOT_TOP_TEMP=`awk -F = '/^ *UBOOT_TOP/{print $2}' ../Config.mak | sed 's/\r//;s/ //g;s/(//g;s/)//g'`
fi

export UBOOT_TOP=`eval echo $UBOOT_TOP_TEMP`

if [ -f ../Config.mak.$USER ]; then
C_LIBRARY_TEMP=`awk -F = '/^ *C_LIBRARY/{print $2}' ../Config.mak.$USER | sed 's/\r//;s/ //g'`
fi
if [ -z $C_LIBRARY_TEMP ]; then
C_LIBRARY_TEMP=`awk -F = '/^ *C_LIBRARY/{print $2}' ../Config.mak | sed 's/\r//;s/ //g'`
fi

if [ -f ../Config.mak.$USER ]; then
CHIP_ID_TEMP=`awk -F = '/^ *DECODER/{print $2}' ../Config.mak.$USER | sed 's/\r//;s/ //g'`
fi
if [ -z $CHIP_ID_TEMP ]; then
CHIP_ID_TEMP=`awk -F = '/^ *DECODER/{print $2}' ../Config.mak | sed 's/\r//;s/ //g'`
fi

CHIP_ID=`eval echo $CHIP_ID_TEMP`

export C_LIBRARY=`eval echo $C_LIBRARY_TEMP`

if [ $C_LIBRARY == glibc ]; then
#export CROSS_COMPILE=arm-gcc6.3-linux-uclibceabi-
export CROSS_COMPILE=aarch64-mix210-linux-
else
#export CROSS_COMPILE=arm-gcc6.3-linux-uclibceabi-
export CROSS_COMPILE=aarch64-mix210-linux-
fi

ARM_LINUX_TOOLCHAIN_NAME=`echo $CROSS_COMPILE | sed 's/-$//'`

export TOOLCHAIN_TOP=$TOOLCHAIN_PARENT_DIRECTORY
if [ 0 -eq `echo $PATH | grep -c $TOOLCHAIN_PARENT_DIRECTORY/bin` ]; then
export PATH=$PATH:$TOOLCHAIN_PARENT_DIRECTORY/bin
fi
echo "source over"
make deploy-rootfs
export CHIP_ID
export ARCH=arm
export ROOTFS_TOP=$ROOTFS_PARENT_DIRECTORY/rootfs_$C_LIBRARY

cd - > /dev/null

echo "#############################################################"
echo SDK_TOP=$SDK_TOP
echo ROOTFS_TOP=$ROOTFS_TOP
echo TOOLCHAIN_TOP=$TOOLCHAIN_TOP
echo KERNEL_TOP=$KERNEL_TOP
echo UBOOT_TOP=$UBOOT_TOP
echo ""
echo LIBC: $C_LIBRARY
echo CHIP_ID: $CHIP_ID
echo GCC VERSION: $GCC_VER
echo CROSS_COMPILE: $CROSS_COMPILE
echo "#############################################################"

unset CHIP_ID_TEMP
unset SDK_TOP_TEMP
unset ROOTFS_PARENT_DIRECTORY_TEMP
unset KERNEL_TOP_TEMP
unset KERNEL_VER_TEMP
unset GCC_VER_TEMP
unset C_LIBRARY_TEMP
unset UBOOT_TOP_TEMP
unset ARM_LINUX_TOOLCHAIN_NAME
