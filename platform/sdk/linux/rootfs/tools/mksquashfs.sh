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

echo "delete files of no use"

case $1 in
    --imx222 ) 
		#for imx222
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ar0330.ko ;
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ov2710.ko ;
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ov9710.ko ;

		rm -fr ../../../rootfs/etc/sensors/ar0330*.bin ;
		rm -fr ../../../rootfs/etc/sensors/ov2710*.bin ;
		rm -fr ../../../rootfs/etc/sensors/ov9710*.bin ;
		rm -fr ../../../rootfs/etc/sensors/ov9715*.bin ;
    	;;
    --ov2710 ) 
		#for ov2710
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ar0330.ko ;
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/imx222.ko ;
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ov9710.ko ;

		rm -fr ../../../rootfs/etc/sensors/ar0330*.bin ;
		rm -fr ../../../rootfs/etc/sensors/imx122*.bin ;
		rm -fr ../../../rootfs/etc/sensors/ov9710*.bin ;  
		rm -fr ../../../rootfs/etc/sensors/ov9715*.bin ; 
    	;;
    --ov9710 ) 
		#for ov9710
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ar0330.ko
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ov2710.ko
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/imx222.ko

		rm -fr ../../../rootfs/etc/sensors/ar0330*.bin
		rm -fr ../../../rootfs/etc/sensors/ov2710*.bin
		rm -fr ../../../rootfs/etc/sensors/imx122*.bin   
		rm -fr ../../../rootfs/etc/sensors/ov9715*.bin ;  
    	;;
    --ov9715 ) 
		#for ov9715
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ar0330.ko
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ov2710.ko
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/imx222.ko

		rm -fr ../../../rootfs/etc/sensors/ar0330*.bin
		rm -fr ../../../rootfs/etc/sensors/ov2710*.bin
		rm -fr ../../../rootfs/etc/sensors/imx122*.bin     
		rm -fr ../../../rootfs/etc/sensors/ov9710*.bin ;
    	;;
    --ar0330 ) 
    	#for ar0330
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ov9710.ko
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/ov2710.ko
		rm -fr ../../../rootfs/lib/modules/3.4.43-gk+/extra/imx222.ko

		rm -fr ../../../rootfs/etc/sensors/ov9710*.bin
		rm -fr ../../../rootfs/etc/sensors/ov2710*.bin
		rm -fr ../../../rootfs/etc/sensors/imx122*.bin  
		rm -fr ../../../rootfs/etc/sensors/ov9715*.bin ;
    	;;
    all ) 
    
    	;;
    *)         
    	echo "unkown sensor, exit!";
    	exit;
    	;;
esac    	

rm -fr ../../../rootfs/lib/firmware/default_binary.bin
rm -fr ../../../rootfs/lib/firmware/orccode.bin
rm -fr ../../../rootfs/lib/firmware/orcme.bin

rm -fr ../../../rootfs/usr/local/bin/factory
rm -fr ../../../rootfs/usr/local/bin/adidemo
rm -fr ../../../rootfs/usr/local/bin/logcapture
rm -fr ../../../rootfs/usr/local/bin/logparser

echo "make squashfs"
rm -fr ../../../gk_squashfs.img
./mksquashfs ../../../rootfs ../../../gk_squashfs.img -comp xz

