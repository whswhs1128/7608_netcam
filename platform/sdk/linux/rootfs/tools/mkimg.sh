#!/bin/sh

#make squashfs for nor flash

do_help ()
{
    SELF=`basename $0`
    echo "eg: $SELF --imx222"
    echo "eg: $SELF --ov2710"
    echo "eg: $SELF --ov9710"
    echo "eg: $SELF --ov9715"
    echo "eg: $SELF --ar0330"
    echo "eg: $SELF all"
    exit 
}
if [ $# -eq 0 ]
then
    do_help
fi

rm -fr rootfs
rm -fr rootfs.tar.xz
cd ../../../
rm -fr rootfs.tar.xz
tar Jcfp rootfs.tar.xz rootfs
cp -fr rootfs.tar.xz linux/rootfs/tools/
rm -fr rootfs.tar.xz
cd linux/rootfs/tools/
tar xfp rootfs.tar.xz

echo "delete files of no use"

case $1 in
    --imx222 ) 
		#for imx222
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ar0330.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ov2710.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ov9710.ko ;

		rm -fr rootfs/etc/sensors/ar0330*.bin ;
		rm -fr rootfs/etc/sensors/ov2710*.bin ;
		rm -fr rootfs/etc/sensors/ov9710*.bin ;
		rm -fr rootfs/etc/sensors/ov9715*.bin ;
    	;;
    --ov2710 ) 
		#for ov2710
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ar0330.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/imx222.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ov9710.ko ;

		rm -fr rootfs/etc/sensors/ar0330*.bin ;
		rm -fr rootfs/etc/sensors/imx122*.bin ;
		rm -fr rootfs/etc/sensors/ov9710*.bin ;  
		rm -fr rootfs/etc/sensors/ov9715*.bin ; 
    	;;
    --ov9710 ) 
		#for ov9710
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ar0330.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ov2710.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/imx222.ko ;

		rm -fr rootfs/etc/sensors/ar0330*.bin ;
		rm -fr rootfs/etc/sensors/ov2710*.bin ;
		rm -fr rootfs/etc/sensors/imx122*.bin ;  
		rm -fr rootfs/etc/sensors/ov9715*.bin ;  
    	;;
    --ov9715 ) 
		#for ov9715
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ar0330.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ov2710.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/imx222.ko ;

		rm -fr rootfs/etc/sensors/ar0330*.bin ;
		rm -fr rootfs/etc/sensors/ov2710*.bin ;
		rm -fr rootfs/etc/sensors/imx122*.bin ;
		rm -fr rootfs/etc/sensors/ov9710*.bin ;
    	;;
    --ar0330 ) 
    	#for ar0330
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ov9710.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/ov2710.ko ;
		rm -fr rootfs/lib/modules/3.4.43-gk+/extra/imx222.ko ;

		rm -fr rootfs/etc/sensors/ov9710*.bin ;
		rm -fr rootfs/etc/sensors/ov2710*.bin ;
		rm -fr rootfs/etc/sensors/imx122*.bin ; 
		rm -fr rootfs/etc/sensors/ov9715*.bin ;
    	;;
    all ) 
    
    	;;
    *)         
    	echo "unkown sensor, exit!";
    	rm -fr rootfs;
    	rm -fr rootfs.tar.xz;
    	exit;
    	;;
esac    	

rm -fr rootfs/lib/firmware/default_binary.bin
rm -fr rootfs/lib/firmware/orccode.bin
rm -fr rootfs/lib/firmware/orcme.bin

rm -fr rootfs/usr/local/bin/factory
rm -fr rootfs/usr/local/bin/adidemo
rm -fr rootfs/usr/local/bin/logcapture
rm -fr rootfs/usr/local/bin/logparser

mkdir -p ../../../image

mkdir -p rootfs/mnt/config
mkdir -p rootfs/mnt/resource
mkdir -p rootfs/mnt/nfs

echo "make config_jffs2"
rm -fr config_jffs2
mkdir config_jffs2
cp -fr rootfs/usr/local/bin/video.xml config_jffs2

rm -fr rootfs/usr/local/bin/video.xml

cd rootfs/usr/local/bin/
ln -s /mnt/config/video.xml  video.xml
cd ../../../../

./mkfs.jffs2 -r config_jffs2 -o ../../../image/config_jffs2.img -n -e 0x10000 -p --pad=0x80000

rm -fr config_jffs2


echo "make rootfs_squashfs_resource"
rm -fr rootfs_squashfs_resource
mkdir rootfs_squashfs_resource

cp -fr rootfs/webSvr/   rootfs_squashfs_resource/

cd rootfs
rm -fr webSvr
ln -s /mnt/resource/webSvr webSvr
cd ../

rm -fr ../../../image/rootfs_squashfs_resource.img
./mksquashfs  rootfs_squashfs_resource ../../../image/rootfs_squashfs_resource.img -comp xz

rm -fr rootfs_squashfs_resource

echo "make rootfs_squashfs"

rm -fr S30mount
echo -e "#! /bin/sh\n\nmount -t jffs2 /dev/mtdblock2 /mnt/config \nmount -t squashfs /dev/mtdblock5 /mnt/resource" > S30mount
chmod +x S30mount
cp -fr S30mount rootfs/etc/init.d/

rm -fr S30mount

echo -e "\nauto eth0\niface eth0 inet static\naddress 192.168.1.120\nnetwork 192.168.0.0\nnetmask 255.255.255.0\nbroadcast 192.168.1.255\ngateway 192.168.1.1" >> rootfs/etc/network/interfaces

rm -fr ../../../image/rootfs_squashfs.img
./mksquashfs rootfs ../../../image/rootfs_squashfs.img -comp xz

rm -fr rootfs
rm -fr rootfs.tar.xz



