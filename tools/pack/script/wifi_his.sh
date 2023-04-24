
echo "insmod wifi driver! " ;
RTL8188FU_VENDER_ID=0bda

lsusb > /tmp/usb_info
grep $RTL8188FU_VENDER_ID /tmp/usb_info
if [ $? -eq 0 ]; then
	echo  "  wifi driver already installed" 
else
		if [ -f "/lib/modules/3.4.43-gk/extra/8188fu.ko" ]
        then
            echo "insmod 8188fu.ko ..."
            /sbin/insmod  /lib/modules/3.4.43-gk/extra/8188fu.ko ;
        fi
fi
