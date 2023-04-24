################################################################################
################################################################################
##
## Makefile -- makefile for hemu library
##
## (C) Copyright 2012-2016 by Goke Microelectronics Co.,Ltd
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
##
################################################################################
################################################################################

TOPDIR = ../../../../..
SUBDIR += app/netcam/src/third/mojing
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config
include $(TOPDIR)/app/netcam/module_config

LIBRARY = libmojing

HEADERS +=  inc/*.h
HEADERS +=  src/pb/*.h
HEADER_SUBDIR = $(SUBDIR)

ifeq ($(SUPPORT_MOFANG),y)
CCFLAGS_USER += -DMODULE_SUPPORT_MOFANG
CCFLAGS_USER += -I $(APPDIR)/src/third/mofang/inc
endif

CCFLAGS_USER += -DMODULE_SUPPORT_MOJING
CCFLAGS_USER += -I$(TOPDIR)/$(SUBDIR)/inc
CCFLAGS_USER += -I$(TOPDIR)/$(SUBDIR)/src

SOURCES += src/mojing.c


ifneq ($(USE_NANOPB),y)
	SOURCES += src/mojingCMD.cpp
	LIBRARIES += protobuf-lite
else
	SOURCES += src/mojingCMD.pb.c
	SOURCES += src/pb/pb_encode.c
	SOURCES += src/pb/pb_decode.c
	SOURCES += src/pb/pb_common.c
	CCFLAGS_USER += -I$(TOPDIR)/app/netcam/src/third/mojing/src/pb
endif

#SOURCES += src/third/mojing/src/SSLHandle.cpp

#LIBRARIES += jsoncpp
#LIBRARIES += protobuf



LIBRARIES += md5
#CCFLAGS_USER += -I$(TOPDIR)/app/netcam/src/third/mojing/inc
CCFLAGS_USER += -I$(TOPDIR)/app/netcam/src/third/mojing/inc

CCFLAGS_USER += -O2
CCFLAGS_USER += -I ./lib/inc
#CCFLAGS_USER += -I ./lib/inc/openssl
CCFLAGS_USER += -I ./inc
CCFLAGS_USER += -I ./src/pb

CCFLAGS_USER += -I lib/inc
CCFLAGS_USER += -I ./$(APPDIR)/inc
CCFLAGS_USER += -I ./$(APPDIR)/inc/cfg

CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/utility
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/mediabuffer
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/json
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/workqueue
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/g711
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/adpcm
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/eventalarm
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/ntpclient
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/wireless
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/ftp
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/mail
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/md5
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/adi
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/platform/sdk
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/mbedtls
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/osd

CCFLAGS_USER += -I$(APPDIR)/src/ftp
CCFLAGS_USER += -I$(APPDIR)/src/email
CCFLAGS_USER += -I$(APPDIR)/src/update
CCFLAGS_USER += -I$(APPDIR)/src/common

CCFLAGS_USER += -I$(TOPDIR)/sdk/inc
CCFLAGS_USER += -I$(TOPDIR)/app/netcam/src/search/inc/

################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-library clean-all

build:  build-objects build-library install-library pre-install headers

headers: install-headers

install: build install-library  

all: clean build install

pre-install:
	@echo ""
	@echo "Install third lib ...."
	cp lib/lib/lib*.a   $(INSTALL_LIBRARY_DIR)
	@echo ""
################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += Makefile.rel
  RELEASE_FILES += $(wildcard include/*.h   )
  RELEASE_FILES += Makefile
  RELEASE_FILES += lib/$(LIBRARY)$(LIBRARY_EXT)
  RELEASE_FILES += lib/$(LIBRARY)$(SHARED_EXT)

release: release-lib-prepare  doc release-files $(DIRECTORIES:%=subdir-release-%)

doc:

release-lib-prepare: clean build-library install-release-library

include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep
