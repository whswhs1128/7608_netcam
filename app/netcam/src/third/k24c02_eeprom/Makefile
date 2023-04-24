################################################################################
################################################################################
##
## Makefile -- makefile for k24c02_eeprom library
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
SUBDIR += app/netcam/src/third/k24c02_eeprom
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config


LIBRARY = libk24c02_eeprom

HEADERS +=  inc/*.h
HEADER_SUBDIR = $(SUBDIR)

CCFLAGS_USER += -DMODULE_SUPPORT_K24C02_EEPROM
CCFLAGS_USER += -Iinc
CCFLAGS_USER += -Isrc

SOURCES += src/k24c02_eeprom.c

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
  RELEASE_FILES += $(wildcard include/*.h   )
  RELEASE_FILES += Makefile
  RELEASE_FILES += lib/$(LIBRARY)$(LIBRARY_EXT)
  RELEASE_FILES += lib/$(LIBRARY)$(SHARED_EXT)

release: release-lib-prepare  doc release-files $(DIRECTORIES:%=subdir-release-%)

doc:

release-lib-prepare: clean build-library install-release-library

include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep
