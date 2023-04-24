#########################################################################
#运行方式如:./uboot_env.sh 0 41  0:内部phytype 1:外部phytype 41内存大小 #
# partinfo ------------ flash分区信息表                                 #
# bootargs_flash ------ 从flash启动的bootargs                           #
# bootcmd ------------- 从flash启动的bootcmd                            #
# bootargs_nfs -------- 以nfs方式启动的bootargs                         #
# bootcmd_nfs --------- 以nfs方式启动的bootcmd                          #
#注意：修改分区信息时要对应修改partinfo和bootcmd                        #
#########################################################################

MEM="32 35 36 38 40 41 42 50"

create_uboot_env()
{
    echo "arm_freq=0x01203201" > uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "baudrate=115200" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    #echo "bootargs=console=ttySGK0,115200 noinitrd mem=${mem}M rw root=/dev/mtdblock5 rootfstype=squashfs init=linuxrc phytype=${phytype}" >> uboot_env_cfg.txt
    #echo "bootargs=console=ttySGK0,115200 initrd=0xC11a0000,0x200000  root=/dev/ram init=/linuxrc mem=${mem}M phytype=${phytype}" >> uboot_env_cfg.txt
    #echo "bootargs_flash=noinitrd console=ttySGK0 initrd=0xC11A0000,0x240000 root=/dev/mtdblock4 init=/linuxrc mem=${mem}M" >> uboot_env_cfg.txt
    echo "bootargs_flash=console=ttySGK0,115200 initrd=0xC11A0000,0x240000  root=/dev/ram init=/linuxrc mem=${mem}M phytype=${phytype}" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "bootargs_nfs=console=ttySGK0,115200  root=/dev/nfs rw nfsroot=192.168.10.5:/opt/goke/rootfs_uClibc ip=192.168.10.35::192.168.10.1:255.255.255.0:goke:eth0:off init=/linuxrc mem=${mem}M phytype=${phytype}" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "partinfo=mtdparts=gk7101_flash:256k(Uboot),64k(Envs),384k(Config),1664k(Kernel),2304k(Rootfs),3456k(Resource),64k(User),8M@0(all)" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    #echo "bootcmd=sf probe;sf read c1000000 e0000 1a0000;bootm c1000000" >> uboot_env_cfg.txt
    #echo "bootcmd=sf probe 0 0;sf read 0xC1000000 0x0000000c0000 0x1a0000;sf read 0xC11a0000 0x260000 0x200000; bootm" >> uboot_env_cfg.txt
    echo "bootcmd=setenv bootargs \$\{bootargs_flash\}, \$\{partinfo\}; sf probe 0 0;sf read 0xC1000000 0x0B0000 0x1A0000;sf read 0xC11A0000 0x250000 0x240000; bootm" >>uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "bootcmd_nfs=setenv bootargs \$\{bootargs_nfs\}, \$\{partinfo\}; sf probe 0 0;sf read 0xC1000000 0x0B0000 0x1A0000;bootm">> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "bootdelay=1" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "consoledev=ttySGK0" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "ethact=gk7101" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "ethaddr=3C:97:0E:22:E1:18" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "gatewayip=192.168.10.1" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "hostname=\"gk7101\"" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "ipaddr=192.168.10.162" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "loadaddr=0xC1000000" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "netdev=eth0" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "netmask=255.255.255.0" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "nfsserver=192.168.10.159" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "phytype=${phytype}" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "rootpath=/opt/goke/rootfs_uClibc" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "serverip=192.168.10.159" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "sfkernel=0x40000" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "stderr=serial" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "stdin=serial" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "stdout=serial" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "mem=${mem}M" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "bsbsize=${bsbsize}M" >> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
    echo "soctype=${soctype}">> uboot_env_cfg.txt
    echo -e "\r" >> uboot_env_cfg.txt
}

phytype=$1
mem=$2
case $mem in
    32)
    mem=32
    bsbsize=2
    soctype=1
    ;;
    35)
    mem=35
    bsbsize=2
    soctype=1
    ;;
    36)
    mem=36
    bsbsize=2
    soctype=1
    ;;
    38)
    mem=38
    bsbsize=1
    soctype=1
    ;;
    40)
    mem=40
    bsbsize=2
    soctype=1
    ;;
    41)
    mem=41
    bsbsize=2
    soctype=1
    ;;
    42)
    mem=42
    bsbsize=1
    soctype=1
    ;;

    46)
    mem=46
    bsbsize=4
    soctype=0
    ;;
    50)
    mem=50
    bsbsize=4
    soctype=0
    ;;
   *)
    echo "mem not find"
    echo "./uboot_env.sh phytype mem "
    exit
    ;;
    esac

echo "phytype:$phytype"
echo "mem:$mem"
echo "bsbsize:$bsbsize"
echo "soctype:$soctype"
echo "num:$?"
if [ $# -eq 0 ]
then
    echo "./uboot_env.sh phytype mem "
    exit
fi
echo "are you sure? [y/n]"
read input
if [ 'y' = $input ]
then
    echo "input y"
    create_uboot_env
    if [ 0 -eq $phytype ]
    then
    name=uboot_env_${mem}_in.bin
    else
    name=uboot_env_${mem}_ext.bin
    fi
    ../bin/pack_upgrade_tool -b uboot_env_cfg.txt  $name 64K
    chmod 644 $name
    cp ./$name ../../release_package/uboot_env/ 
else
    echo "input not y"
fi

