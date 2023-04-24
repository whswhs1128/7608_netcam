#!/bin/sh


rm -fr rootfs
rm -fr rootfs.tar.xz
cd ../../../
rm -fr rootfs.tar.xz
tar Jcfp rootfs.tar.xz rootfs
cp -fr rootfs.tar.xz linux/rootfs/tools/
rm -fr rootfs.tar.xz
cd linux/rootfs/tools/
tar xfp rootfs.tar.xz


rm -fr  rootfs/lib/firmware/default_binary.bin
rm -fr  rootfs/lib/firmware/orccode.bin
rm -fr  rootfs/lib/firmware/orcme.bin

rm -fr  rootfs/usr/local/bin/factory
rm -fr  rootfs/usr/local/bin/adidemo
rm -fr  rootfs/usr/local/bin/logcapture
rm -fr  rootfs/usr/local/bin/logparser

#echo -e "\nauto eth0\niface eth0 inet static\naddress 192.168.1.120\nnetwork 192.168.0.0\nnetmask 255.255.255.0\nbroadcast 192.168.1.255\ngateway 192.168.1.1" >> rootfs/etc/network/interfaces

echo "make jffs2 for nor flash"
echo "note : --pad parameter !!"

./mkfs.jffs2 -r rootfs -o ../../../gk_rootfs_jffs2.img -n -e 0x10000 -p --pad=0xDB0000

rm -fr rootfs
rm -fr rootfs.tar.xz

