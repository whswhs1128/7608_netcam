################################################################################
################################################################################
##
## Makefile -- makefile for encrypt library
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
SUBDIR += app/netcam/src/third/encrypt
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config


LIBRARY = libapp_encrypt

HEADERS +=  inc/*.h
HEADER_SUBDIR = $(SUBDIR)

CCFLAGS_USER += -DMODULE_SUPPORT_ENCRYPT_AUTHOR
CCFLAGS_USER += -Iinc
CCFLAGS_USER += -Isrc

SOURCES += src/encrypt_dx.c

CCFLAGS_USER += -O2

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
	cp lib/libdx8api_armgcc.a $(INSTALL_LIBRARY_DIR)
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
