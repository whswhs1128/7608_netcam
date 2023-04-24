################################################################################
################################################################################
##
## Makefile -- makefile for textinsert library
##
## (C) Copyright 2012-2015 by Goke Microelectronics Co.,Ltd
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
##
################################################################################
################################################################################


TOPDIR = ../..
SUBDIR = subsystem/osd

include $(TOPDIR)/env/make/Makefile.config

LIBRARY = libosd

HEADERS += include/*.h
HEADER_SUBDIR = subsystem/osd

SOURCES  += ./src/osd_text.c
SOURCES  += ./src/osd_functions.c
SOURCES  += ./src/vector_font.c
SOURCES  += ./src/lattice_font.c

CCFLAGS_USER += -O2
CCFLAGS_USER += -I $(OSSPREBUILD)/freetype/include/freetype2
CCFLAGS_USER += -I include
CCFLAGS_USER += -I $(SDK_TOP)/adi/include
CCFLAGS_USER += -I $(SDK_TOP)/linux/kernel/drv/include

################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-library clean-all

build:  build-objects build-library install-library

headers: install-headers

install: build install-library install-headers

all: clean build install

################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += $(SOURCES)
  RELEASE_FILES += $(wildcard include/*.h   )
  RELEASE_FILES += Makefile

release: doc release-files

doc:

include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep
