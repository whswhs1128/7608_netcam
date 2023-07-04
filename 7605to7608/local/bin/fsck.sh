#!/bin/sh
#
# Start mmc fsck....
#

MMC=mmcblk0p1
fsckFile=/opt/custom/cfg/fsck
#/tmp/fsckfifo
infoFile=/tmp/uartfifo

#if exist and length is not 0
if [ -e $fsckFile ]; then
    echo "fs check error last" >> $infoFile
else
    if [ -b /dev/$MMC ]; then
        touch $fsckFile
        #/etc/hotplug/remove.sh $MMC
        echo 3 > /proc/sys/vm/drop_caches
        /usr/bin/free >> $infoFile 2>&1
        echo "/usr/bin/time /sbin/fsck.fat -av /dev/$MMC" >> $infoFile 2>&1
        /usr/bin/time /sbin/fsck.fat -av /dev/$MMC >> $infoFile 2>&1
        echo 3 > /proc/sys/vm/drop_caches
        /usr/bin/free >> $infoFile 2>&1
        #/etc/hotplug/insert.sh $MMC
        rm -f $fsckFile
    else    
        echo "no sd card" >> $infoFile
    fi
fi

