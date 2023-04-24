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
SUBDIR += app/netcam/src/gk_cms
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config
include $(APPDIR)/module_config

LIBRARY = libapp_gkcms

HEADERS +=
HEADER_SUBDIR =


CCFLAGS_USER += -DMODULE_SUPPORT_CMS
CCFLAGS_USER += -DMODULE_SUPPORT_LOCAL_REC

ifeq ($(SUPPORT_RF433_CONFIG),y)
CCFLAGS_USER += -DMODULE_SUPPORT_RF433
endif

CCFLAGS_USER += -I$(TOPDIR)/app/netcam/src/avi
CCFLAGS_USER += -I .
CCFLAGS_USER += -I gk_cms_net
#SOURCES += $(wildcard src/gk_cms/*.c)
#SOURCES += $(wildcard src/gk_cms/gk_cms_net/*.c)
SOURCES += gk_cms_session.c
SOURCES += gk_cms_utility.c
SOURCES += gk_cms_sock.c
SOURCES += gk_cms_protocol.c
SOURCES += gk_cms_media.c
SOURCES += gk_cms_pb.c
SOURCES += gk_cms_net/gk_cms_net_user.c
SOURCES += gk_cms_net/gk_cms_net_system.c
SOURCES += gk_cms_net/gk_cms_net_image.c
SOURCES += gk_cms_net/gk_cms_net_update.c
SOURCES += gk_cms_net/gk_cms_net_audio.c
SOURCES += gk_cms_net/gk_cms_net_sd.c
SOURCES += gk_cms_net/gk_cms_net_network.c
SOURCES += gk_cms_net/gk_cms_net_alarm.c
SOURCES += gk_cms_net/gk_cms_net_md.c
SOURCES += gk_cms_net/gk_cms_net_snap.c
SOURCES += gk_cms_net/gk_cms_net_video.c
SOURCES += gk_cms_net/gk_cms_net_record.c
SOURCES += gk_cms_net/gk_cms_net_channel.c
SOURCES += gk_cms_net/gk_cms_net_ptz.c



CCFLAGS_USER += -O2
CCFLAGS_USER += -I ./$(APPDIR)/inc
CCFLAGS_USER += -I ./$(APPDIR)/inc/cfg
CCFLAGS_USER += -I ./$(APPDIR)/src/search/inc	#xqq

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
CCFLAGS_USER += -I$(APPDIR)/src/ftp
CCFLAGS_USER += -I$(APPDIR)/src/email
CCFLAGS_USER += -I$(APPDIR)/src/update

CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/platform/sdk
CCFLAGS_USER += -I$(APPDIR)/src/rf433

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
