#!/bin/sh

do_help ()
{
    SELF=`basename $0`
    echo "eg: $SELF --nor"
    echo "eg: $SELF --nand"
    echo "note : first ensure vol_size in config, second ensourc -c paramter"
    echo "       eg : for nor  flash, vol_size=12M, peb=64k,   -c paramter is 12*1024/64  =192."
    echo "       eg : for nand flash, vol_size=30M, peb=128k,  -c paramter is 30*1024/128 =240."
    echo "note : vol_size less than this mtd size, eg : mtdx is 13M, vol_size is 12M/11M, less than 13M."
    exit 
}
if [ $# -eq 0 ]
then
    do_help
fi

if [ -f ../../../Config.mak.$USER ]; then
C_LIBRARY=`awk -F = '/^ *C_LIBRARY/{print $2}' ../../../Config.mak.$USER | sed 's/\r//;s/ //g'`
fi
if [ -z $C_LIBRARY ]; then
C_LIBRARY=`awk -F = '/^ *C_LIBRARY/{print $2}' ../../../Config.mak | sed 's/\r//;s/ //g'`
fi

rm -fr rootfs_$C_LIBRARY
rm -fr rootfs_$C_LIBRARY.tar.xz
cd ../../../
rm -fr rootfs_$C_LIBRARY.tar.xz
tar Jcfp rootfs_$C_LIBRARY.tar.xz rootfs_$C_LIBRARY
cp -fr rootfs_$C_LIBRARY.tar.xz linux/rootfs/tools/
rm -fr rootfs_$C_LIBRARY.tar.xz
cd linux/rootfs/tools/
tar xfp rootfs_$C_LIBRARY.tar.xz
  
rm -fr rootfs_$C_LIBRARY/lib/firmware/default_binary.bin
rm -fr rootfs_$C_LIBRARY/lib/firmware/orccode.bin
rm -fr rootfs_$C_LIBRARY/lib/firmware/orcme.bin

rm -fr rootfs_$C_LIBRARY/usr/local/bin/factory
rm -fr rootfs_$C_LIBRARY/usr/local/bin/adidemo
rm -fr rootfs_$C_LIBRARY/usr/local/bin/logcapture
rm -fr rootfs_$C_LIBRARY/usr/local/bin/logparser

#echo -e "\nauto eth0\niface eth0 inet static\naddress 192.168.1.120\nnetwork 192.168.0.0\nnetmask 255.255.255.0\nbroadcast 192.168.1.255\ngateway 192.168.1.1" >> rootfs/etc/network/interfaces


echo "make ubifs"
echo "note : ubifs.cfg vol_size and mkfs.ubifs -c parameter !!"

case $1 in
    --nor) 
		#for nor flash
		rm -fr ../../../image/gk_ubi_nor.img;
		rm -fr ../../../gk_ubifs_nor.img;
		./mkfs.ubifs -q -r rootfs_$C_LIBRARY -m 1 -e 65408 -c 208 -o ../../../gk_ubi_nor.img;
		./ubinize -p 64KiB -m 1 ubifs_nor.cfg -o ../../../gk_ubifs_nor.img;
		echo "write gk_ubifs_nor.img to uboot";
    	;;
    --nand ) 
		#for nand flash
		rm -fr ../../../gk_ubi_nand.img;
		rm -fr ../../../gk_ubifs_nand.img;
		./mkfs.ubifs -q -r rootfs_$C_LIBRARY -m 2048 -e 126976 -c 240 -o ../../../gk_ubi_nand.img;
		./ubinize -p 128KiB -m 2048 ubifs_nand.cfg -o ../../../gk_ubifs_nand.img -O 2048;
		echo "write gk_ubifs_nand.img to uboot";
    	;;
    *)         
    	echo "unkown flash type, exit!";
    	rm -fr rootfs_$C_LIBRARY;
    	rm -fr rootfs_$C_LIBRARY.tar.xz;
    	exit;
    	;;
esac

rm -fr rootfs_$C_LIBRARY
rm -fr rootfs_$C_LIBRARY.tar.xz

