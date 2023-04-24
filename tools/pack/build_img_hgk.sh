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
hboard=amg
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
	"--osmem")
		if [ "$2"x != ""x ];then
			osmem=$2;
			echo "This osmem is :${osmem}M"
		fi
		shift
		;;
	"--pwd")
		if [ "$2"x != ""x ];then
			pwd=$2;
			echo "This pwd is :${pwd}"
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
tar -xf ${SDK_TOP}/linux/rootfs/rootfs_uclibc.tar.bz2 -C ${ROOTFS_DIR}
tar -jxf other/rootfs_uClibc_passwd.tar.bz2 -C ${ROOTFS_DIR}
tar -jxf other/plist/rootfs_uClibc_plist.tar.bz2 -C ${ROOTFS_DIR}
tar -jxf other/console_redirect/rootfs_uClibc_console_redirect.tar.bz2 -C ${ROOTFS_DIR}
#tar -jxf other/rootfs_uClibc_dosfstools.tar.bz2 -C ${ROOTFS_DIR}

mkdir -p ${ROOTFS_DIR}/usr/local/bin
mkdir -p ${ROOTFS_DIR}/usr/share/udhcpc

sudo rm -rf ${ROOTFS_DIR}/usr/share/udhcpc/default.script
cp script/default.script ${ROOTFS_DIR}/usr/share/udhcpc/default.script
cp script/S70mount ${ROOTFS_DIR}/etc/init.d/

sudo rm -rf ${ROOTFS_DIR}/etc/resolv.conf
ln -s /tmp/resolv.conf ${ROOTFS_DIR}/etc/resolv.conf
chmod 777 ${ROOTFS_DIR}/usr/share/udhcpc/default.script

sudo rm -rf ${ROOTFS_DIR}/run
ln -s /tmp ${ROOTFS_DIR}/run

sudo chmod 777 script -R
cp script/udhcpd.conf ${ROOTFS_DIR}/bin
cp script/network_tools.sh ${ROOTFS_DIR}/usr/local/bin/


#echo "ulimit -p 300">> ${ROOTFS_DIR}/etc/profile
echo "ulimit -s unlimited" >> ${ROOTFS_DIR}/etc/profile
echo "ulimit -p unlimited" >> ${ROOTFS_DIR}/etc/profile
echo "ulimit -l unlimited" >> ${ROOTFS_DIR}/etc/profile
echo "TZ=UTC-08:00"        >> ${ROOTFS_DIR}/etc/profile
echo "export TZ"           >> ${ROOTFS_DIR}/etc/profile

##***************************************************************************##



################### Step 2.  Build the netcam image ###########################
echo ""; echo "Start recompiling program ...";echo "";
cd ${NETCAM_DIR}
/bin/sh version.sh $DECODER $sensor 
cd ${APP_ROOT_DIR}
pwd
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
#cp -rf mqtt_demo  ${ROOTFS_DIR}/opt/custom/
#cp -rf mqtt_ota_demo  ${ROOTFS_DIR}/opt/custom
if [ $board = "gk7605v100rb" ] || [ $board = "gk7205v200rb" ];then
	hboard=demo
fi
sed -e "s/REPLOAD/load${DECODER}/g" -e "s/REPSENSOR/${sensor}/g" -e "s/REPMEM/${osmem}M/g" -e "s/REPBOARD/${hboard}/g" script/startup_hgk.sh >${ROOTFS_DIR}/usr/local/bin/startup.sh
cp -rf script/dmseg.sh ${ROOTFS_DIR}/usr/local/bin/dmseg.sh
cp -f script/mount.sh ${ROOTFS_DIR}/usr/local/bin/mount.sh
#cp -rf upgrade/bin/upgrade_flash ${ROOTFS_DIR}/usr/sbin/upgrade_flash
cp -f script/fsck.sh ${ROOTFS_DIR}/usr/local/bin/fsck.sh

if [ $board = "gk7205v300_232v5_scmj" ] || [ $board = "gk7205v200_232v5_scmj" ];then
    echo "add key ko"
    echo "insmod /gmp/key.ko" >> ${ROOTFS_DIR}/usr/local/bin/mount.sh
fi

sed -i  's#\("password":\).*#\1'\"${pwd}\"',#g' ${ROOTFS_DIR}/usr/local/defcfg/gk_user_cfg.cjson
#cat ${ROOTFS_DIR}/usr/local/defcfg/gk_user_cfg.cjson


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
	
    echo -e "tar xf wifi  ...\n"
    tar -jxf other/rootfs_uClibc_${wifi}_hgk.tar.bz2 -C ${ROOTFS_DIR}
    cp -f script/wifi.sh ${ROOTFS_DIR}/usr/local/bin/wifi.sh
	sudo chmod 777 ${ROOTFS_DIR}/usr/local/bin/wifi.sh
fi
##***************************************************************************##



##Step 8. Move /lib/firmware&/lib/modules and make Symbolic Link to resource ##

##***************************************************************************##
mkdir -p ${ROOTFS_DIR}/opt/resource
mv ${ROOTFS_DIR}/gmp ${ROOTFS_DIR}/opt/resource
sudo chmod 777 ${ROOTFS_DIR}/opt/resource/gmp/*
ln -sv /opt/resource/gmp ${ROOTFS_DIR}/gmp

################# Step 9.  Make rootfs.image,custom.jffs2 file ################
rm -rf ${DEST_DIR}
mkdir -p ${DEST_DIR}

sudo chmod 777 imgtools_hgk -R
./imgtools_hgk/mkfs.jffs2 -r ${ROOTFS_DIR}/opt/custom -o ${DEST_DIR}/custom.jffs2 -n -e 0x10000 
cp -rf ${ROOTFS_DIR}/opt/custom ./custom_tmp
rm -rf ${ROOTFS_DIR}/opt/custom/*

./imgtools_hgk/mksquashfs ${ROOTFS_DIR}/opt/resource ${DEST_DIR}/resource.squashfs -b ${packSize} -comp xz
cp -rf ${ROOTFS_DIR}/opt/resource ./resource_tmp
rm -rf ${ROOTFS_DIR}/opt/resource/*



mkdir ${ROOTFS_DIR}/usr/local/defcfg/ -p
cp ${DEST_DIR}/custom.jffs2 ${ROOTFS_DIR}/usr/local/defcfg/ -rf


./imgtools_hgk/mksquashfs ${ROOTFS_DIR} ${DEST_DIR}/rootfs.squashfs -b ${packSize} -comp xz
rm -rf ${ROOTFS_DIR}/etc/init.d/S70mount
cp -rf ./custom_tmp/* ${ROOTFS_DIR}/opt/custom
cp -rf ./resource_tmp/* ${ROOTFS_DIR}/opt/resource
rm -rf ./custom_tmp ./resource_tmp
##***************************************************************************##
tar -jxf other/rootfs_uClibc_non_passwd.tar.bz2 -C ${ROOTFS_DIR}
rm -rf ${ROOTFS_DIR}/gmp
mv ${ROOTFS_DIR}/opt/resource/gmp ${ROOTFS_DIR}/gmp

# for debug
cd ../../app/netcam
make  install
