
echo "insmod wifi driver! " ;

wifiCheck=/opt/custom/cfg/nowifi
usbCnt=`lsusb | wc -l`
if [[ $usbCnt == 3 ]];then
    if [ -e $wifiCheck ]; then
        echo "not install wifi driver" 
        touch /tmp/nowifi
    else
        if [ -f "/lib/modules/3.4.43-gk/extra/mt7601Usta.ko" ]
        then
            echo "insmod mt7601Usta.ko ..."
            /sbin/insmod  /lib/modules/3.4.43-gk/extra/mt7601Usta.ko ;
        fi

        if [ -f "/lib/modules/3.4.43-gk/extra/8188eu.ko" ]
        then
            echo "insmod 8188eu.ko ..."
            /sbin/insmod  /lib/modules/3.4.43-gk/extra/8188eu.ko ;
        fi

        if [ -f "/lib/modules/3.4.43-gk/extra/8188fu.ko" ]
        then
            echo "insmod 8188fu.ko ..."
            /sbin/insmod  /lib/modules/3.4.43-gk/extra/8188fu.ko ;
        fi
    fi
else
    touch /tmp/nowifi
fi
