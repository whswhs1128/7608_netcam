#!/bin/sh

CUR_DIR=$(pwd)
BIN_DIR=${CUR_DIR}/bin
IMAGE_DIR=${CUR_DIR}/../rootfs/bin

#echo ${CUR_DIR}
#echo ${BIN_DIR}
#echo ${IMAGE_DIR}

cp -rf ${IMAGE_DIR}/custom.jffs2      ${BIN_DIR}/
cp -rf ${IMAGE_DIR}/resource.squashfs ${BIN_DIR}/
cp -rf ${IMAGE_DIR}/rootfs.squashfs   ${BIN_DIR}/

cd ${BIN_DIR}

rm -f  ./gk_upgrade.bin
./pack_upgrade_tool -p flashmap_gk7101.ini gk_upgrade.bin
chmod 777 gk_upgrade.bin
echo ""
echo "===================================================="
ls -lth  gk_upgrade.bin
echo "===================================================="