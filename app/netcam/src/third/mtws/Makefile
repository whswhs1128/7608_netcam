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
SUBDIR += applications/netcam/src/third/mtws
APPDIR =  $(TOPDIR)/applications/netcam

include $(TOPDIR)/env/make/Makefile.config


LIBRARY = libapp_mtws

HEADERS +=  
HEADER_SUBDIR = 



CCFLAGS_USER += -DMODULE_SUPPORT_MTWS_P2P
CCFLAGS_USER += -I$(TOPDIR)/$(SUBDIR)/inc
CCFLAGS_USER += -I$(TOPDIR)/$(SUBDIR)/src

SOURCES += ./src/phone_mtws.c
SOURCES += ./src/mtws_base64.c
#SOURCES += ./src/doorbell_func.c


CCFLAGS_USER += -O2
CCFLAGS_USER += -I ./lib/inc
CCFLAGS_USER += -I ./$(APPDIR)/inc
CCFLAGS_USER += -I ./$(APPDIR)/inc/cfg

CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/utility
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/mediabuffer
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/json

CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/workqueue
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/g711
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/adpcm
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/eventalarm
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/ntpclient
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/wireless
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/ftp
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware/mail
CCFLAGS_USER += -I$(APPDIR)/src/ftp
CCFLAGS_USER += -I$(APPDIR)/src/email
CCFLAGS_USER += -I$(APPDIR)/src/update
CCFLAGS_USER += -I$(APPDIR)/src/common

CCFLAGS_USER += -I$(TOPDIR)/sdk/inc
CCFLAGS_USER += -I$(TOPDIR)/applications/netcam/src/search/inc/

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
	cp lib/libp2p.a   $(INSTALL_LIBRARY_DIR)
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
