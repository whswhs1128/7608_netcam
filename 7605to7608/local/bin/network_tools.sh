#!/bin/sh


kill_wifi_tools()
{
    while [ $i -le $j ]
    do
        PIDS=`ps -ef |grep $1 |grep -v grep`
        if [ "$PIDS" == "" ]; then
            echo "kill end !"
            return 0                   
        else                           
            echo "myprocess is runing:$PIDS, tools:$1!"
            killall $1 
        fi 
    done
}     

AP_MODE="AP_MODE"
STA_MODE="STA_MODE"
wifi_tools_start()
{
    echo "wifi_tools_start:$1"
    ifconfig wlan0 up
    ifconfig wlan1 up

    if [ "$1" == $STA_MODE ];then
        echo "start STA !"
        ifconfig wlan0 0.0.0.0
        ifconfig wlan1 0.0.0.0
        /usr/local/bin/wpa_supplicant -d -Dwext -iwlan0 -c /tmp/passwd.conf >/dev/null&
    fi

    if [ $1 == $AP_MODE ];then
        echo "start ap !"
        ifconfig wlan0 192.168.252.1
        ifconfig wlan1 0.0.0.0
        route add -net 192.168.252.0 netmask 255.255.255.0 gw 192.168.252.1 wlan0
        /usr/local/bin/hostapd -d /opt/custom/cfg/hostapd.conf -B
        /usr/sbin/udhcpd /opt/custom/cfg/udhcpd.conf &
    fi
}

start_dhcp()
{
    if [ "$1" == "LAN_DHCP" ]; then
        /sbin/udhcpc -i eth0 -b
        /sbin/udhcpc -i eth1 -b
    fi
    if [ "$1" == "WIFI_DHCP" ]; then
        /sbin/udhcpc -i wlan0 -b
        /sbin/udhcpc -i wlan1 -b
    fi
}

kill_dhcpc()
{
    while [ $i -le $j ]
    do
            PIDS=`ps -ef |grep /sbin/udhcpc|grep -v grep | awk '{print $3}'`
            if [ "$PIDS" == "" ]; then
                return 0
            else
                echo "myprocess is runing:$PIDS!"
                killall udhcpc
            fi
    done
}
sleep 4

while [ 1 ]
do                                     
    sleep 1
    if [ ! -e "/tmp/network_tools_ctrl" ]; then
        echo "/tmp/network_tools_ctrl not exist" > /dev/null
        continue
    fi
    #read wifi cfg                     
    cat /tmp/network_tools_ctrl | while read line
    do                             
        echo "wifi cmd :$line"
        #check wifi cmd
        if [ "" == "$line" ]
        then
            continue
        fi

        echo "xxxx network_tools cmd:$line"
        #restart wifi tools
        if [ "$line" == "STA_MODE" ] || [ "$line" == "AP_MODE" ]; then
            kill_dhcpc 
            kill_wifi_tools wpa_supplicant
            kill_wifi_tools udhcpd
            kill_wifi_tools hostapd
            wifi_tools_start $line
        fi


        #dhcp ctrl
        if [ "$line" == "LAN_DHCP" ] || [ "$line" == "WIFI_DHCP" ]; then
            kill_dhcpc 
            start_dhcp $line
        fi
       
    done
    echo "network tools loop"
    
done  
exit 0


