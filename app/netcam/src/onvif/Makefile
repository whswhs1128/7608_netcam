################################################################################
################################################################################
##
## Makefile -- makefile for tuning library
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

TOPDIR = ../../../..
SUBDIR += app/netcam/src/onvif
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config

LIBRARY = libapp_onvif

HEADERS +=  
HEADER_SUBDIR = 

CCFLAGS_USER += -DMODULE_SUPPORT_ONVIF
#SOURCES += $(wildcard src/avi/*.c)
SOURCES =  onvif_adapter.c
SOURCES += onvif_http.c
SOURCES += ttl.c
SOURCES += sdk_af.c
SOURCES += httpClient.c
SOURCES += net_time.c

CCFLAGS_USER += -O2
CCFLAGS_USER += -I ./$(APPDIR)/inc
CCFLAGS_USER += -I ./$(APPDIR)/inc/cfg

CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/utility
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/mediabuffer
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/json

CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/workqueue
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/g711
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/adpcm
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/onvif
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/http
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/ftp
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/mail
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/wireless/
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/platform/sdk
CCFLAGS_USER += -I /home/xieqianqian/7608_netcam/platform/sdk/subsystem/vision/include
CCFLAGS_USER += -I $(INSTALL_HEADER_DIR)/subsystem/vision
CCFLAGS_USER += -I $(INSTALL_HEADER_DIR)/gmp

################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-library clean-all

build:  build-objects build-library install-library

headers: install-headers

install: build install-library 

all: clean build install

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
