#!/bin/sh
#
# Start mount....
#
echo "mount /dev/mtdblock2 -> /opt/custom "
mount -t jffs2 /dev/mtdblock2  /opt/custom
if [ "$?" != "0" ]; then
    dd if=/usr/local/defcfg/custom.jffs2 of=/dev/mtdblock2
    mount -t jffs2 /dev/mtdblock2  /opt/custom
    echo "dd/dev/mtdblock2 and mount /dev/mtdblock2 -> /opt/custom "
fi

ifconfig eth0 up
/sbin/udhcpc -i eth0 &
mkfifo /tmp/uartfifo  
exec 3<>/tmp/uartfifo
#/usr/local/bin/readfifo &
#check sd
#/usr/local/bin/fsck.sh

echo "mount /dev/mtdblock5 -> /opt/resource "
mount  -t squashfs /dev/mtdblock5  /opt/resource
#cp /dev/mtdblock6 /tmp/resource.fs
#mount -t squashfs /tmp/resource.fs /opt/resource/

