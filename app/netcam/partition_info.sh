#!/bin/bash
TOPDIR=../..
DEPLOY_PACK_NAME=deploy_pack.sh
FLASH_MAP_NAME=flashmap_cfg.ini
PLATFORM=
SDK_VER=
BOARD=
SDK_VER_DIR=
BOARD_DIR=

TARGET=inc/partition_info.h

readParam()
{
	file=$1;item=$2;separator=$3;field=$4;
    val=$(grep -e "${item}" ${file} | cut -d ${separator} -f ${field})
    #echo ${val}
	echo ${val} | sed 's/\r//'
}

readLineNum()
{
	file=$1;item=$2;
	val=$(grep -n ${item} ${file} | cut -d : -f 1)
	echo ${val}
}

readLineParam()
{
	file=$1;line=$2;separator=$3;field=$4;
    val=$(awk NR==${line}'{print}' ${file} | cut -d ${separator} -f ${field})
    #echo ${val}
	echo ${val} | sed 's/\r//'
}


parse_deploy_pack()
{
	DEPLOY_PACK_DIR=${TOPDIR}/${DEPLOY_PACK_NAME}
	BOARD=$(readParam ${DEPLOY_PACK_DIR} BOARD= = 2)
	#echo "BOARD  ${BOARD}"
	PLATFORM=$(readParam ${DEPLOY_PACK_DIR} PLATFORM= = 2)
	#echo "PLATFORM  ${PLATFORM}"
	SDK_VER=$(readParam ${DEPLOY_PACK_DIR} SDK_VER= = 2)
	#echo "SDK_VER  ${SDK_VER}"
	SDK_VER_DIR=${TOPDIR}/platform/${PLATFORM}/${SDK_VER}
	BOARD_DIR=${SDK_VER_DIR}/board
	#echo "BOARD_DIR  ${BOARD_DIR}"
}

mtd_name_backups_line=
mtd_size_backups_line=
mtd_size_backups=0
parse_flash_map()
{
	FLASH_MAP_DIR=${BOARD_DIR}/${BOARD}/${FLASH_MAP_NAME}
	echo "FLASH_MAP_DIR  ${FLASH_MAP_DIR}"
	mtd_name_backups_line=$(readLineNum ${FLASH_MAP_DIR} mtd_name=backups)
	if [ "$mtd_name_backups_line" = "" ]
	then
	echo "no backups"
	else
	echo "mtd_name_backups_line  ${mtd_name_backups_line}"
	let "mtd_part_backups_line=${mtd_name_backups_line}-1"
	mtd_part_backups=$(readLineParam ${FLASH_MAP_DIR} ${mtd_part_backups_line} = 2)
	echo "mtd_part_backups  ${mtd_part_backups}"
	
	let "mtd_size_backups_line=${mtd_name_backups_line}+2"
	echo "mtd_size_backups_line  ${mtd_size_backups_line}"
	mtd_size_backups=$(readLineParam ${FLASH_MAP_DIR} ${mtd_size_backups_line} = 2)
	echo "mtd_size_backups  ${mtd_size_backups}"
	fi
	mtd_name_user_line=$(readLineNum ${FLASH_MAP_DIR} mtd_name=user)
	let "mtd_part_user_line=${mtd_name_user_line}-1"
	mtd_part_user=$(readLineParam ${FLASH_MAP_DIR} ${mtd_part_user_line} = 2)
	
	echo "mtd_part_user  ${mtd_part_user}"
	( 
		echo /\* This file is auto generated\*/
		echo \#define MTD_PART_BACKUPS \"$mtd_part_backups\" 
		echo \#define MTD_SIZE_BACKUPS \($mtd_size_backups\)
		echo \#define MTD_PART_USER	\"$mtd_part_user\"  
	) > .tmppartition_info.sh
}

parse_deploy_pack
parse_flash_map
mv -f .tmppartition_info.sh $TARGET




