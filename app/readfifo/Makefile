################################################################################
################################################################################
##
## Makefile -- makefile for sample venc
##
## (C) Copyright 2012-2014 by Goke Microelectronics Co.,Ltd
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
##
################################################################################
################################################################################

TOPDIR = ../..
SUBDIR = app/readfifo
NETCAMDIR = ${TOPDIR}/app/netcam/src/common

include $(TOPDIR)/env/make/Makefile.config

SUPPORT_UPGRADE = y
SUPPORT_UPGRADE_BACKUPS = y
SUPPORT_UPGRADE_FAST = y

################################################################################
# source files
################################################################################
SOURCES += readFifo.c
#SOURCES += upgrade/goke_upgrade.c
ifeq ($(SUPPORT_UPGRADE),y)
SOURCES += porting.c
SOURCES += $(wildcard update/*.c)
SOURCES += watchdog/netcam_watchdog.c
endif

ifeq ($(SUPPORT_UPGRADE_FAST),y)
SOURCES += readipcsearch.c
CCFLAGS_USER += -DMODULE_SUPPORT_UPGRADE_FAST
endif


################################################################################
# define define user cc flags
################################################################################
CCFLAGS_USER += -O3 -DDEBUG -g
CCFLAGS_USER += -I$(NETCAMDIR)
ifeq ($(SUPPORT_UPGRADE),y)
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/adi
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/image
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/utility
CCFLAGS_USER += -Iupdate
CCFLAGS_USER += -Iupgrade/inc
CCFLAGS_USER += -I$(TOPDIR)/app/netcam/inc
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/platform/sdk
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/app/netcam/src/cfg
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/json
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/wireless
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/md5
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/mediabuffer
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/mbedtls
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/
#CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/workqueue
endif



################################################################################
# define define user ld flags
################################################################################
ifeq ($(SUPPORT_UPGRADE),y)
CCFLAGS_USER += -DMODULE_UPGRADE_IN_FIFO
endif

ifeq ($(SUPPORT_UPGRADE_BACKUPS),y)
CCFLAGS_USER += -DMODULE_SUPPORT_UPGRADE_BACKUPS
endif

################################################################################
# define library used
################################################################################
#LIBRARIES += app_cfg
LIBRARIES += utilityApi
#LIBRARIES += app_update
LIBRARIES += md5
LIBRARIES += json
LIBRARIES += sdk
#LDFLAGS_USER += -lmbedtls



################################################################################
# define target name
################################################################################
PROGRAM = readfifo



################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-all

build:  build-objects build-program

install: build install-program install-bin 

headers:

all: clean build install

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += Makefile
  RELEASE_FILES += $(SOURCES)

release: release-files $(DIRECTORIES:%=subdir-release-%) info

doc: $(DIRECTORIES:%=subdir-doc-%)

info: force
	@echo $(DIRECTORIES)
install-program-priv:
	cp -rf build/arm11-gcc-uClibc-linux-GK720X/bin/readfifo $(ROOTFS_TOP)/usr/sbin


include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep

