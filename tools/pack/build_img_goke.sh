#/bin/sh

ROOTFS_DIR=${ROOTFS_TOP}
DEST_DIR=./bin

CUR_DIR=$(pwd)
UPGRADE_DIR=${CUR_DIR}/upgrade
APP_ROOT_DIR=${CUR_DIR}/../..

echo "Rootfs directory: $ROOTFS_DIR"

sensor=""
sensordetect=0
DECODER=""
wifi=""
board=""
packSize=128K

do_help ()
{
    SELF=`basename $0`
    echo "usage:"
    echo "$SELF --sensor [sensor] -d GK7101 "
    echo "eg: $SELF --sensor imx222 -d GK7101"
    exit 
}

if [ $# -eq 0 ]
then
    do_help
fi

until [ $# -eq 0 ]
do
    argv=$1
	case $argv in
	"--sensor")
		sensor=$2;
		shift
		;;
	"-d")
		DECODER=$2;
		shift
		;;
	"--wifi")
		if [ "$2"x != ""x ];then
			wifi=$2;
			echo "This board support wifi:${wifi}"
		fi
        shift
		;;
    "--board")
		if [ "$2"x != ""x ];then
			board=$2;
			echo "This board is :${board}"
		fi
		shift
		;;
	*)
		sensordetect=1;
		echo "unknow command $argv"
		;;
	esac
    shift
done


if [ $board = "gk7202_gc2053_tb_v3" ];then
    packSize=64K
fi

echo "packSize ${packSize}"

if [ -z $sensor ]
then
    do_help
fi

if [ -z $DECODER ]
then
    do_help
fi

echo ""
echo "Build $sensor -- $DECODER rootfs image"
echo ""



##################### Step 1.  ReBuild the rootfs  ############################
echo "Delete ${ROOTFS_DIR}"
sudo rm -rf ${ROOTFS_DIR}

# produce mini rootfs 
mkdir -p ${ROOTFS_DIR}
tar -jxf ${SDK_TOP}/linux/rootfs/arm-gk720x-linux-uclibcgnueabi/minirootfs_uClibc.tar.bz2 -C ${ROOTFS_DIR}
tar -jxf other/rootfs_uClibc_passwd.tar.bz2 -C ${ROOTFS_DIR}
#tar -jxf other/plist/rootfs_uClibc_plist.tar.bz2 -C ${ROOTFS_DIR}
#tar -jxf other/console_redirect/rootfs_uClibc_console_redirect.tar.bz2 -C ${ROOTFS_DIR}
#tar -jxf other/rootfs_uClibc_dosfstools.tar.bz2 -C ${ROOTFS_DIR}
sudo chmod 777 script -R

sudo rm -rf ${ROOTFS_DIR}/usr/share/udhcpc/default.script
cp script/default.script ${ROOTFS_DIR}/usr/share/udhcpc/default.script
cp script/S70mount ${ROOTFS_DIR}/etc/init.d/

sudo rm -rf ${ROOTFS_DIR}/etc/resolv.conf
ln -s /tmp/resolv.conf ${ROOTFS_DIR}/etc/resolv.conf
chmod 777 ${ROOTFS_DIR}/usr/share/udhcpc/default.script

sudo rm -rf ${ROOTFS_DIR}/run
ln -s /tmp ${ROOTFS_DIR}/run

sudo chmod 777 script -R
cp script/mkfs.vfat ${ROOTFS_DIR}/sbin
cp script/mkdosfs ${ROOTFS_DIR}/sbin
cp script/fdisk ${ROOTFS_DIR}/sbin
cp script/busybox ${ROOTFS_DIR}/bin
cp script/udhcpd.conf ${ROOTFS_DIR}/bin
ln -s /bin/busybox ${ROOTFS_DIR}/usr/sbin/udhcpd
cp script/network_tools.sh ${ROOTFS_DIR}/usr/local/bin/


#echo "ulimit -p 300">> ${ROOTFS_DIR}/etc/profile
echo "ulimit -s unlimited" >> ${ROOTFS_DIR}/etc/profile
echo "ulimit -p unlimited" >> ${ROOTFS_DIR}/etc/profile
echo "ulimit -l unlimited" >> ${ROOTFS_DIR}/etc/profile

mkdir -p ${ROOTFS_DIR}/usr/local/bin
##***************************************************************************##



################### Step 2.  Build the netcam image ###########################
echo ""; echo "Start recompiling program ...";echo "";
cd ${APP_ROOT_DIR}

make 

if [ $? -ne 0 ]
then
	echo "";echo ""
	echo "Compiling failed, Please use svn up to use latest version"
	echo "";echo ""
	exit
fi

${CROSS_COMPILE}strip  ${ROOTFS_DIR}/usr/local/bin/netcam
rm -f ${ROOTFS_DIR}/usr/local/bin/netcam_d


cd ${CUR_DIR}
##***************************************************************************##



############## Step 3.  make and copy upgrade_flash in rootfs  ################
cd ${UPGRADE_DIR}/code
make 
cd ${CUR_DIR}  
cp -rf ${UPGRADE_DIR}/bin/upgrade_flash  ${ROOTFS_DIR}/usr/local/bin

cd ${CUR_DIR}
echo -e "make readfifo  ...\n"
echo -e ${CUR_DIR}
cd ../../app/readfifo
ln -s ../netcam/src/third/upgrade upgrade
ln -s ../netcam/src/update update
ln -s ../netcam/src/watchdog watchdog
make all
${CROSS_COMPILE}strip  ${ROOTFS_DIR}/usr/local/bin/readfifo

##***************************************************************************##



##################### Step 4. copy the sdk file to rootfs  ####################

##***************************************************************************##



##################### Step 5. copy script file to rootfs  #####################
cd ${CUR_DIR}
cp -rf sys_info  ${ROOTFS_DIR}/root/sys_info
if  [ $sensordetect -eq 0 ]; then
	sed -e "s/REPSENSOR/${sensor}/g" -e "s/REPBOARD/${board}/g" script/startup.sh >${ROOTFS_DIR}/usr/local/bin/startup.sh
else
	sed -e "s/REPSENSOR/ /g" -e "s/REPBOARD/ /g" script/startup.sh >${ROOTFS_DIR}/usr/local/bin/startup.sh
	#sed -i "/init.sh/a\\/usr/local/bin/sensordetect.sh" ${ROOTFS_DIR}/usr/local/bin/startup.sh
	echo "/usr/local/bin/sensordetect.sh" >> ${ROOTFS_DIR}/usr/local/bin/init.sh
	cp -rf script/sensordetect.sh ${ROOTFS_DIR}/usr/local/bin/sensordetect.sh
fi


if [ $board = "gk7202_gc2053_tb_38_big_v4" ] || [ $board = "gk7202_gc2053_tb_38_big_v2_v4" ];then
    echo "modify osd default memory value:$board"
	sed -i "s/OSD=512 vi0=1 vi1=0/OSD=1824 vi0=1 vi1=0 OSDS0=58 OSDS1=42 OSDS2=0 OSDS3=0 OSDS4=0 OSDS5=0/g" ${ROOTFS_DIR}/usr/local/bin/init.sh
fi

if [ $board = "gk7205s_gc4653_ipc20" ];then
    echo "modify osd default memory value:$board"
	sed -i "s/OSD=512 vi0=1 vi1=0/OSD=3024 vi0=1 vi1=0 OSDS0=58 OSDS1=42 OSDS2=0 OSDS3=0 OSDS4=0 OSDS5=0/g" ${ROOTFS_DIR}/usr/local/bin/init.sh
    cat ${ROOTFS_DIR}/usr/local/bin/init.sh
fi

if [ $board = "gk7202_gc2053_tb_v3" ];then
    echo "modify osd default memory value:$board"
	sed -i "s/OSD=512 vi0=1 vi1=0/OSD=1824 vi0=1 vi1=0 OSDS0=58 OSDS1=42 OSDS2=0 OSDS3=0 OSDS4=0 OSDS5=0/g" ${ROOTFS_DIR}/usr/local/bin/init.sh
    cat ${ROOTFS_DIR}/usr/local/bin/init.sh
fi

cp -rf script/dmseg.sh ${ROOTFS_DIR}/usr/local/bin/dmseg.sh
cp -f script/mount.sh ${ROOTFS_DIR}/usr/local/bin/mount.sh
#cp -rf upgrade/bin/upgrade_flash ${ROOTFS_DIR}/usr/sbin/upgrade_flash
cp -f script/fsck.sh ${ROOTFS_DIR}/usr/local/bin/fsck.sh

#cp -f ${APP_ROOT_DIR}/install/arm11-gcc-uClibc-linux-GK720X/lib/libmbedtls.so ${ROOTFS_DIR}/lib/libmbedtls.so
sudo chmod 777 ${ROOTFS_DIR}/usr/sbin/*
sudo chmod 777 ${ROOTFS_DIR}/usr/local/bin/*
##***************************************************************************##



###################### Step 6.  Support sd card. ##############################
echo -e "add hotplug  ...\n"

tar -jxf other/rootfs_uClibc_hotplug.tar.bz2 -C ${ROOTFS_DIR}
mkdir -p ${ROOTFS_DIR}/mnt/sd_card
##***************************************************************************##



#################### Step 7.  Support wifi. Add wifi tools ####################
if [ "$wifi"x != ""x ];then
	echo -e "add wifi  ...\n"

	#wifi device support
	tar -jxf other/rootfs_uClibc_${wifi}.tar.bz2 -C ${ROOTFS_DIR}
	cp -f script/wifi.sh ${ROOTFS_DIR}/usr/local/bin/wifi.sh
	sudo chmod 777 ${ROOTFS_DIR}/usr/local/bin/wifi.sh
fi
##***************************************************************************##



##Step 8. Move /lib/firmware&/lib/modules and make Symbolic Link to resource ##

##***************************************************************************##
mkdir -p ${ROOTFS_DIR}/opt/resource/lib
mv ${ROOTFS_DIR}/lib/firmware ${ROOTFS_DIR}/opt/resource/lib/firmware
mv ${ROOTFS_DIR}/lib/modules  ${ROOTFS_DIR}/opt/resource/lib/modules
ln -sv /opt/resource/lib/firmware ${ROOTFS_DIR}/lib/firmware
ln -sv /opt/resource/lib/modules  ${ROOTFS_DIR}/lib/modules

################# Step 9.  Make rootfs.image,custom.jffs2 file ################
rm -rf ${DEST_DIR}
mkdir -p ${DEST_DIR}

sudo chmod 777 imgtools -R
./imgtools/mkfs.jffs2 -r ${ROOTFS_DIR}/opt/custom -o ${DEST_DIR}/custom.jffs2 -n -e 0x10000 
cp -rf ${ROOTFS_DIR}/opt/custom ./custom_tmp
rm -rf ${ROOTFS_DIR}/opt/custom/*

./imgtools/mksquashfs ${ROOTFS_DIR}/opt/resource ${DEST_DIR}/resource.squashfs -b ${packSize} -comp xz
cp -rf ${ROOTFS_DIR}/opt/resource ./resource_tmp
rm -rf ${ROOTFS_DIR}/opt/resource/*

mkdir ${ROOTFS_DIR}/usr/local/defcfg/ -p
mkdir ${ROOTFS_DIR}/usr/local/bin/lib -p
cp ${DEST_DIR}/custom.jffs2 ${ROOTFS_DIR}/usr/local/defcfg/ -rf
./imgtools/mksquashfs ${ROOTFS_DIR} ${DEST_DIR}/rootfs.squashfs -b ${packSize} -comp xz

rm -rf ${ROOTFS_DIR}/etc/init.d/S70mount
rm -rf ${ROOTFS_DIR}/usr/local/bin/startup.sh
cp -rf ./custom_tmp/* ${ROOTFS_DIR}/opt/custom
cp -rf ./resource_tmp/* ${ROOTFS_DIR}/opt/resource
rm -rf ./custom_tmp ./resource_tmp
##***************************************************************************##
tar -jxf other/rootfs_uClibc_non_passwd.tar.bz2 -C ${ROOTFS_DIR}
rm -rf ${ROOTFS_DIR}/lib/firmware
rm -rf ${ROOTFS_DIR}/lib/modules
mv ${ROOTFS_DIR}/opt/resource/lib/firmware ${ROOTFS_DIR}/lib/firmware
mv ${ROOTFS_DIR}/opt/resource/lib/modules ${ROOTFS_DIR}/lib/modules

# for debug
cd ../../app/netcam
make  install
