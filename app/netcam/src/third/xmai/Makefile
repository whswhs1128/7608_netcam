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
SUBDIR += applications/netcam/src/xmai
APPDIR =  $(TOPDIR)/applications/netcam/

include $(TOPDIR)/env/make/Makefile.config


LIBRARY = libapp_xmai

HEADERS +=
HEADER_SUBDIR =


CCFLAGS_USER += -DMODULE_SUPPORT_XMAI

SOURCES += xmai_message.c
SOURCES += xmai_session.c

CCFLAGS_USER += -O2
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
CCFLAGS_USER += -I$(APPDIR)/src/xmai
CCFLAGS_USER += -I$(APPDIR)/src/gk_cms
CCFLAGS_USER += -I$(TOPDIR)/sdk/inc
CCFLAGS_USER += -I$(TOPDIR)/applications/netcam/src/rf433/

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
