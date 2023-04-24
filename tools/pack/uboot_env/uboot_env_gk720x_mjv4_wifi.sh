#########################################################################
#运行方式如:./uboot_env.sh 0 41 2048 2048 7205S gk7205S_HZD_gc2053_v1.0  >> 0:内部phytype 1:外部phytype 41内存大小 #
# partinfo ------------ flash分区信息表                                 #
# bootargs_flash ------ 从flash启动的bootargs                           #
# bootcmd ------------- 从flash启动的bootcmd                            #
# bootargs_nfs -------- 以nfs方式启动的bootargs                         #
# bootcmd_nfs --------- 以nfs方式启动的bootcmd                          #
#注意：修改分区信息时要对应修改partinfo和bootcmd                        #
#########################################################################

create_uboot_env()
{
echo -e "arm_freq=${arm_freq}\r
baudrate=115200\r
bootcmd=run sfboot\r
bootdelay=0\r
bootfile=zImage\r
bsbsize=${bsbsize}KB\r
consoledev=ttySGK0\r
ethact=goke\r
ethaddr=3C:97:0E:22:E1:18\r
gatewayip=192.168.10.1\r
hostname=\"${chip}\"\r
ipaddr=192.168.10.25\r
loadaddr=0xC1000000\r
mem=${mem}M\r
netdev=eth0\r
netmask=255.255.255.0\r
nfsserver=192.168.10.31\r
nfsboot=setenv bootargs console=\$\{consoledev\},\$\{baudrate\} root=/dev/nfs rw nfsroot=\$\{nfsserver\}:\$\{rootpath\} ip=\$\{ipaddr\}::\$\{gatewayip\}:\$\{netmask\}:\$\{ethact\}:\$\{netdev\}:off init=/linuxrc mem=\$\{mem\} phytype=\$\{phytype\}, mtdparts=\$\{mtdparts\}; sf probe 0 0;sf read 0xC1000000 0x100000 0x1A0000;bootm\r
mtdparts=gk_flash:256k(Uboot),64k(Envs),704k(Config),1664k(Kernel),2944k(Rootfs),2496k(Resource),64k(User),8M@0(all)\r
phytype=${phytype}\r
ppmsize=${ppmsize}KB\r
rootfstype=ubi.mtd=3 rootfstype=ubifs root=ubi0:rootfs\r
rootpath=/opt/goke/rootfs_uClibc\r
serverip=192.168.10.31\r
sfboot=setenv bootargs console=\$\{consoledev\},\$\{baudrate\} initrd=0xC11A0000,0x2e0000 root=/dev/ram init=/linuxrc mem=\$\{mem\} phytype=\$\{phytype\}, mtdparts=\$\{mtdparts\}; sf probe 0 0;sf read 0xC1000000 0x100000 0x1A0000;sf read 0xC11A0000 0x2A0000 0x2e0000; bootm\r
sfkernel=0x40000\r
stderr=serial\r
stdin=serial\r
stdout=serial\r
board=${board_type}" > uboot_env_cfg.txt
}

phytype=$1
mem=$2
bsbsize=$3
ppmsize=$4
chip=$5
board_type=$6
arm_freq=0x00112032

echo "phytype:${phytype}"
echo "mem    :${mem}M"
echo "bsbsize:${bsbsize}KB"
echo "ppmsize:${ppmsize}KB"
echo "chip   :${chip}"
echo "board_type   :${board_type}"

if [ $# -lt 6 ];then
    echo -e "\033[31mpara error, usage:./uboot_env.sh phytype mem(M) bsbsize(KB) ppmsize(KB) chip board_type\033[0m"
    exit
fi
if [ ${chip}x == "gk7202"x ];then
    arm_freq=0x00112032
else
	arm_freq=0x01202D01
fi

echo -n "are you sure? [y/n]: "
read input
if [ 'y' = $input ];then
    create_uboot_env
    if [ 0 -eq $phytype ];then
		name=uboot_env_${mem}_in.bin
    else
		name=uboot_env_${mem}_ext.bin
    fi
    ../upgrade/bin/pack_upgrade_tool -b uboot_env_cfg.txt  $name 64K
    chmod 644 $name
    cp ./$name ../../release_package/uboot_env/ 
fi
