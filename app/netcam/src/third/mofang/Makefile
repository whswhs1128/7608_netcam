################################################################################
################################################################################
##
## Makefile -- makefile for mofang library
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
SUBDIR += app/netcam/src/third/mofang
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config
include $(TOPDIR)/app/netcam/module_config

LIBRARY = libmofang

HEADERS +=  inc/*.h
HEADER_SUBDIR = $(SUBDIR)

SOURCES += $(wildcard src/*.c)

CCFLAGS_USER += -DMODULE_SUPPORT_MOFANG
CCFLAGS_USER += -O2
CCFLAGS_USER += -I inc
CCFLAGS_USER += -I src
CCFLAGS_USER += -I $(APPDIR)/src/third/mojing/lib/inc/

CCFLAGS_USER += -I ./$(APPDIR)/inc
CCFLAGS_USER += -I ./$(APPDIR)/inc/cfg
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/json
#CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/md5
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/platform/sdk
CCFLAGS_USER += -I$(TOPDIR)/sdk/inc

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
