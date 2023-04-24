#!/bin/sh

kernel_ver=$(uname -r)

insmod /lib/modules/$kernel_ver/extra/hal.ko
insmod /lib/modules/$kernel_ver/extra/hw_crypto.ko
insmod /lib/modules/$kernel_ver/extra/media.ko BSB0=1024 BSB1=1024 OSD=256 vi0=1 vi1=1
insmod /lib/modules/$kernel_ver/extra/audio.ko
insmod /lib/modules/$kernel_ver/extra/sensor.ko 
insmod /lib/modules/$kernel_ver/extra/mipi.ko

rm /tmp/sensor_hw_ch0.bin -f
rm /tmp/sensor_ex_ch0.ko -f
rm /tmp/sensor_hw_ch1.bin -f
rm /tmp/sensor_ex_ch1.ko -f

if [ $# -eq 0 ] ;then
    echo "sensordetect"
    /usr/local/bin/sensordetect -c 2
else
    if [ -e /usr/local/bin/sensors/$1"_hw.bin" ];then
        ln -s /usr/local/bin/sensors/$1"_hw.bin" /tmp/sensor_hw_ch0.bin
    else
        echo "\"/usr/local/bin/sensors/${1}_hw.bin\" is no exist."
        exit 0
    fi

    if [ -e /lib/modules/$kernel_ver/extra/$1"_ex.ko" ];then
        ln -s /lib/modules/$kernel_ver/extra/$1"_ex.ko" /tmp/sensor_ex_ch0.ko
    else
        echo "\"/lib/modules/$kernel_ver/extra/${1}_ex_ch0.ko\" is no exist."
    fi

    if [ -e /usr/local/bin/sensors/$2"_hw.bin" ];then
    	ln -s /usr/local/bin/sensors/$2"_hw.bin" /tmp/sensor_hw_ch1.bin
    else
        echo "\"/usr/local/bin/sensors/${2}_hw.bin\" is no exist."
        exit 0
    fi
    if [ -e /lib/modules/$kernel_ver/extra/$2"_ex.ko" ];then
        ln -s /lib/modules/$kernel_ver/extra/$2"_ex.ko" /tmp/sensor_ex_ch1.ko
    else
        echo "\"/lib/modules/$kernel_ver/extra/${2}_ex.ko\" is no exist."
    fi
fi

if [ -e /tmp/sensor_ex_ch0.ko ];then
    insmod /tmp/sensor_ex_ch0.ko
fi

if [ -e /tmp/sensor_ex_ch1.ko ];then
    insmod /tmp/sensor_ex_ch1.ko
fi

if [ -e /usr/local/bin/display/lcd_hw.bin ];then
    if [ -e /tmp/lcd_hw.bin ]; then
        rm /tmp/lcd_hw.bin -f
    fi
    ln -s /usr/local/bin/display/lcd_hw.bin /tmp/lcd_hw.bin
fi

