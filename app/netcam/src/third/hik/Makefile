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
SUBDIR += app/netcam/src/third/hik
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config

LIBRARY = libapp_hik

HEADERS +=  
HEADER_SUBDIR = 

SOURCES += ./hik/src/hik_base64.c
SOURCES += ./hik/src/d3des.c
SOURCES += ./hik/src/hik_lst.c
SOURCES += ./hik/src/hik_net.c
SOURCES += ./hik/src/hik_netfuns.c
SOURCES += ./hik/src/hik_netlogin.c
SOURCES += ./hik/src/hik_netutil.c
SOURCES += ./hik/src/hik_security.c
SOURCES += ./hik/src/hmac.c
SOURCES += ./hik/src/hik_md5.c
SOURCES += ./hik/src/posixWrapper.c

SOURCES += ./hik_protocol/hik_search_protocol.c
SOURCES += ./hik_protocol/hik_server_protocol.c
SOURCES += ./hik_protocol/hik_session.c

CCFLAGS_USER += -DMODULE_SUPPORT_HIK
CCFLAGS_USER += -O2
CCFLAGS_USER += -I hik/inc
CCFLAGS_USER += -I hik/inc/encrypt
CCFLAGS_USER += -I hik_protocol
CCFLAGS_USER += -I$(APPDIR)/inc
CCFLAGS_USER += -I$(APPDIR)/inc/cfg
CCFLAGS_USER += -I$(APPDIR)/src/ftp
CCFLAGS_USER += -I$(APPDIR)/src/email
CCFLAGS_USER += -I$(APPDIR)/src/update
CCFLAGS_USER += -I$(APPDIR)/src/common
CCFLAGS_USER += -I$(APPDIR)/src/search/inc
CCFLAGS_USER += -I$(APPDIR)/src/rf433/*.h
CCFLAGS_USER += -I$(APPDIR)/src/gk_cms

CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/eventalarm
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/json
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/wireless
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/platform/sdk
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
	#cp $(BUILD_LIBRARY_SUBDIR)/libapp_hik.a   $(APPDIR)/lib
	#cp $(BUILD_LIBRARY_SUBDIR)/libapp_hik.a   $(INSTALL_LIBRARY_DIR)
	#rm -rf $(INSTALL_LIBRARY_DIR)/libapp_hik.so
	#rm -rf build/
	#cp build/arm11-gcc-uClibc-linux-GK7102/lib/libapp_hik.so /opt/goke/rootfs_uClibc/usr/lib/
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
