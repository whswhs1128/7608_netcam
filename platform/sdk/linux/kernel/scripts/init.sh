#!/bin/sh

kernel_ver=$(uname -r)

insmod /lib/modules/$kernel_ver/extra/hal.ko
insmod /lib/modules/$kernel_ver/extra/hw_crypto.ko
insmod /lib/modules/$kernel_ver/extra/media.ko BSB0=1024 BSB1=1024 OSD=512 vi0=1 vi1=0
#change osd config for gkvision rectangle painting
#insmod /lib/modules/$kernel_ver/extra/media.ko BSB0=1024 BSB1=1024 OSD=4096 vi0=1 vi1=0 OSDS0=15 OSDS1=55
insmod /lib/modules/$kernel_ver/extra/audio.ko
insmod /lib/modules/$kernel_ver/extra/sensor.ko 
insmod /lib/modules/$kernel_ver/extra/mipi.ko
#insmod /lib/modules/$kernel_ver/extra/ptz_drv.ko gpio_par=28,29,33,34,30,1,32,31

rm /tmp/sensor_hw_ch0.bin -f
rm /tmp/sensor_ex_ch0.ko -f

if [ $# -eq 0 ] ;then
    echo "sensordetect"
    /usr/local/bin/sensordetect 0
else
    if [ -e /usr/local/bin/sensors/$1"_hw.bin" ];then
        ln -s /usr/local/bin/sensors/$1"_hw.bin" /tmp/sensor_hw_ch0.bin
    else
        echo "\"/usr/local/bin/sensors/${1}_hw.bin\" is no exist."
        exit 0
    fi
    
    ex_file_name=$1
    if ( echo ${1} |grep -q "_dvp" );then
        ex_file_name=${1%"_dvp"}
    fi

    if ( echo ${1} |grep -q "_mipi" );then
        ex_file_name=${1%"_mipi"}
    fi
    
    if [ -e /lib/modules/$kernel_ver/extra/$ex_file_name"_ex.ko" ];then
        ln -s /lib/modules/$kernel_ver/extra/$ex_file_name"_ex.ko" /tmp/sensor_ex_ch0.ko
    else
        echo "\"/lib/modules/$kernel_ver/extra/${ex_file_name}_ex.ko\" is no exist."
    fi
fi

if [ -e /tmp/sensor_ex_ch0.ko ];then
    insmod /tmp/sensor_ex_ch0.ko 
fi

if [ -e /usr/local/bin/display/lcd_hw.bin ];then
    if [ -e /tmp/lcd_hw.bin ]; then
        rm /tmp/lcd_hw.bin -f
    fi
    ln -s /usr/local/bin/display/lcd_hw.bin /tmp/lcd_hw.bin
fi
