#!/bin/sh

SD_PATH="/mnt/sd_card/ding_ping_config"
killall dhcpcd
killall wpa_supplicant
killall dhcpd

killall hostapd

sleep 3
ifconfig wlan0 up;
sleep 2
                                          
$SD_PATH/rtwpriv wlan0 mp_start                                                                                               
$SD_PATH/rtwpriv wlan0 mp_channel 7                                   
$SD_PATH/rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0                      
$SD_PATH/rtwpriv wlan0 mp_ant_tx a                                          
$SD_PATH/rtwpriv wlan0 mp_txpower patha=44,pathb=44                     
$SD_PATH/rtwpriv wlan0 mp_rate 108                                       
$SD_PATH/rtwpriv wlan0 mp_ctx background      
$SD_PATH/rtwpriv wlan0 mp_channel 1
$SD_PATH/rtwpriv wlan0 mp_channel 2   
