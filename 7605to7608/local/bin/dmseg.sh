#!/bin/sh

dmesg -c > /dev/null
while true
do
    sleep 1
    dmesg -c >> /tmp/uartfifo
done

