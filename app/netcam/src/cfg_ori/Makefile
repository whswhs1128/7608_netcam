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
SUBDIR += app/netcam/src/cfg
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config
include $(APPDIR)/module_config

LIBRARY = libapp_cfg

HEADERS +=  $(APPDIR)/inc/cfg/*.h
HEADER_SUBDIR = $(SUBDIR)

CCFLAGS_USER += -I $(APPDIR)/inc
CCFLAGS_USER += -I $(APPDIR)/inc/cfg
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/platform/sdk
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/json
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/wireless
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/utility
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/app/netcam/src/update

SOURCES += ./cfg_alarm.c
SOURCES += ./cfg_all.c
SOURCES += ./cfg_audio.c
SOURCES += ./cfg_channel.c
SOURCES += ./cfg_common.c
SOURCES += ./cfg_gb28181.c
SOURCES += ./cfg_image.c
SOURCES += ./cfg_md.c
SOURCES += ./cfg_network.c
SOURCES += ./cfg_p2p.c
SOURCES += ./cfg_ptz.c
SOURCES += ./cfg_pwm.c
SOURCES += ./cfg_record.c
SOURCES += ./cfg_sdk.c
SOURCES += ./cfg_snap.c
SOURCES += ./cfg_store.c
SOURCES += ./cfg_system.c
SOURCES += ./cfg_test.c
SOURCES += ./cfg_user.c
SOURCES += ./cfg_video.c

CCFLAGS_USER += -O2
################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-library clean-all

build:  build-objects build-library install-library headers

headers: install-headers

install: build install-library 

all: clean build install

################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += Makefile.rel
  RELEASE_FILES += $(wildcard $(APPDIR)/inc/cfg/*.h   )
  RELEASE_FILES += Makefile
  RELEASE_FILES += lib/$(LIBRARY)$(LIBRARY_EXT)
  RELEASE_FILES += lib/$(LIBRARY)$(SHARED_EXT)

release: release-lib-prepare  doc release-files $(DIRECTORIES:%=subdir-release-%)

doc:

release-lib-prepare: clean build-library install-release-library

include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep
