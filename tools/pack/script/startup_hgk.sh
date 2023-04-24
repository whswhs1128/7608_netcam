#!/bin/sh
#/usr/local/bin/mount.sh
export LD_LIBRARY_PATH=/usr/local/bin/lib:$LD_LIBRARY_PATH
/usr/local/bin/wifi.sh

if [ -f "/opt/custom/cfg/telnet" ]; then
    /usr/sbin/telnetd
fi


ulimit -s unlimited
ulimit -p unlimited
ulimit -l unlimited
echo 1000 > /proc/sys/vm/min_free_kbytes

#for goke __sdcard_ipc.sh.
if [ -f "/mnt/sd_card/__sdcard_ipc.sh" ]; then
	cp /mnt/sd_card/__sdcard_ipc.sh /tmp/__sdcard_ipc.sh
	chmod 777 /tmp/__sdcard_ipc.sh
	echo "run: /tmp/__sdcard_ipc.sh"
	/tmp/__sdcard_ipc.sh
fi

osmem=REPMEM
for ipinfo in `cat /proc/cmdline`
do
	case "$ipinfo" in
	mem=*)
		osmem=`echo "$ipinfo" | cut -d = -f 2 | cut -d M -f 1`M
		;;
	esac
done
echo "osmem:$osmem"
cd /gmp
/gmp/REPLOAD -i -sensor REPSENSOR -osmem $osmem -demo REPBOARD
cd ..

/usr/local/bin/readfifo &
#cp -rf /usr/local/bin/netcam /tmp/netcam
echo 3 > /proc/sys/vm/drop_caches
/usr/local/bin/network_tools.sh&

if [ -f "/opt/custom/cfg/telnet" ]; then
    echo "Do You Want to Run Application ? (y/n)"
    read -t 1 -n 1 char
else
    char = y
fi

#if [ -f "/mnt/sd_card/factory.txt" ]; then
#	rm -rf /opt/custom/cfg/*.cjson
#fi

killall udhcpc
if [ "$char" == "n" ]; then
        echo "Application Cancel!"
else
    /usr/local/bin/dmseg.sh &
    
    #/bin/cat /tmp/fsckfifo >> /tmp/uartfifo
    #/bin/rm /tmp/fsckfifo
    
    echo "...Start netcam application ..."
    /usr/local/bin/netcam &
    #/usr/local/bin/netcam >> /tmp/uartfifo 2>&1
    var=$?
    echo "sync data:$var"
    sync
    if [ "$var" -eq 111 ]; then
        echo "..... exit netcam application...exit code: $var" >> /tmp/uartfifo
        touch /tmp/netcamexit
        sleep 1 #wait dmesg.sh update log each eleep 1
        echo ""
    elif  [ "$var" -eq 252 ]; then
        echo "..... exit netcam application...exit code: $var" >> /tmp/uartfifo
        sleep 2 #wait dmesg.sh update log each eleep 1
        reboot -f
        killall readfifo
    elif  [ "$var" -eq 253 ]; then
        echo "..... exit netcam application...start to upgreade.." >> /tmp/uartfifo
        sleep 1
    else
        echo "..... reboot...exit code: $var" >> /tmp/uartfifo
        touch /tmp/netcamexit
        sleep 1 #wait dmesg.sh update log each eleep 1
    fi
    #mute
    sed -i  's#\("rebootMute":\).*#\1'1',#g' /opt/custom/cfg/gk_audio_cfg.cjson
    #exit except. the network may be error. so dhcp once
    #/sbin/udhcpc -i eth0 &
fi

