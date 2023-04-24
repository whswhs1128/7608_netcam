################################################################################
################################################################################
##
## Makefile -- makefile for tutk library
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
SUBDIR += app/netcam/src/third/goolink
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config
include $(APPDIR)/module_config


LIBRARY = libapp_goolink

HEADERS +=  
HEADER_SUBDIR = 



#CCFLAGS_USER += -DMODULE_SUPPORT_GOOLINK
CCFLAGS_USER += -I inc
CCFLAGS_USER += -I src

SOURCES += src/audioRecorder.c
SOURCES += src/goolink_server.c


CCFLAGS_USER += -O2
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
CCFLAGS_USER += -I$(APPDIR)/src/ftp
CCFLAGS_USER += -I$(APPDIR)/src/email
CCFLAGS_USER += -I$(APPDIR)/src/update
CCFLAGS_USER += -I$(APPDIR)/src/common

################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-library clean-all

build:  build-objects build-library install-library pre-install

headers: install-headers

install: build install-library  

all: clean build install

pre-install:
	@echo ""
	@echo "Install third lib ...."
	cp lib/libterminal.a   $(INSTALL_LIBRARY_DIR)
	cp lib/libvoice_transfer.so $(ROOTFS_TOP)/lib
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
