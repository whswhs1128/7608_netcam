#!/bin/sh

# Do not expand names
board=$1
upgrade_board=$2
set -f

TARGET=src/common/user_config.h
CC=`echo $CROSS_COMPILE | sed 's/-$/-gcc/'`
TIMESTAMP=`date +'%Y-%m-%d %H:%M:%S'`
SVNREVISION=$(svn info | awk -F : '/Revision/{print $2}')

# Generate a temporary compile.h

( echo /\* This file is auto generated\*/

  echo \#define APPSVNREVISION \"`echo $SVNREVISION`\"

  echo \#define APPTIMESTAMP \"$TIMESTAMP\"

  echo \#define LIBC \"$C_LIBRARY\"

  echo \#define COMPILER \"`$CC -v 2>&1 | tail -n 1`\"
  echo \#define DEVICE_BOARD \"$board\"
  echo \#define UPGRADE_DEVICE_BOARD \"$upgrade_board\"
) > .tmpversion.sh


#echo "  generate  $TARGET"
mv -f .tmpversion.sh $TARGET

