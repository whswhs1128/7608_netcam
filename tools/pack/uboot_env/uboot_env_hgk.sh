#########################################################################
#运行方式如:./uboot_env.sh 41 gk7205S_HZD_gc2053_v1.0 gk7605v100 >> 0:41内存大小 #1 board_type
# mtdparts ------------ flash分区信息表                              #
# bootcmd ------------- run sfboot/nfsboot                                  #
# sfboot -------------- 从flash启动的bootargs                        #
# nfsboot ------------- 以nfs方式启动的bootargs                        #
#注意：修改分区信息时要对应修改mtdparts和sfboot/nfsboot 中kernel加载的地址及大小 #
#########################################################################

create_uboot_env()
{
echo -e "arch=arm\r
baudrate=115200\r
board=${board_type}\r
board_name=${chip}\r
bootcmd=run sfboot\r
bootdelay=0\r
bootfile=uImage\r
consoledev=ttyAMA0\r
cpu=armv7\r
ethact=eth0\r
ethaddr=7a:4a:f2:af:82:8c\r
gatewayip=192.168.10.1\r
ipaddr=192.168.10.25\r
loadaddr=0x41000000\r
mem=${mem}M\r
netmask=255.255.255.0\r
nfsserver=192.168.10.222\r
nfsboot=setenv bootargs console=\$\{consoledev\},\$\{baudrate\} root=/dev/nfs rw nfsroot=\$\{nfsserver\}:\$\{rootpath\},nfsvers=3 ip=\$\{ipaddr\}:\$\{gatewayip\}:\$\{netmask\}:\$\{board_name\}:\$\{ethact\} init=/linuxrc mem=\$\{mem\}, mtdparts=\$\{mtdparts\}; sf probe 0 0;sf read \$\{loadaddr\} 0x100000 0x250000;bootm\r
mtdparts=sfc:256k(Uboot),64k(Envs),704k(Config),2560k(Kernel),3840k(Rootfs),3840k(Resource),5056k(Backups),64k(User),16M@0(all)\r
rootfstype=initrd=0x41280000,0x3c0000 root=/dev/ram init=/linuxrc\r
rootpath=/home/nfs/rootfs_uClibc\r
serverip=192.168.10.221\r
sfboot=setenv bootargs console=null,115200 \$\{rootfstype\} mem=\$\{mem\}, mtdparts=\$\{mtdparts\}; sf probe 0 0;sf read \$\{loadaddr\} 0x100000 0x280000; sf read 0x41280000 0x380000 0x3c0000;bootm\r
soc=goke\r
stderr=serial\r
stdin=serial\r
stdout=serial\r
vendor=${chip}\r
verify=n" > uboot_env_hgk.txt
}

mem=$1
board_type=$2
chip=$3

echo "mem    :${mem}M"
echo "board_type   :${board_type}"

if [ $# -lt 3 ];then
    echo -e "\033[31mpara error, usage:./uboot_env.sh mem(M) board_type chip\033[0m"
    exit
fi

echo -n "are you sure? [y/n]: "
read input
if [ 'y' = $input ];then
    create_uboot_env
    name=uboot_env_${mem}.bin
    ../upgrade/bin/pack_upgrade_tool -b uboot_env_hgk.txt  $name 64K
    chmod 644 $name
fi
