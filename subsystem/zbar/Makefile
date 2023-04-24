################################################################################LIBRARIES += ipcdrv_audio
################################################################################
##
## Makefile -- makefile for hardware driver library
##
## (C) Copyright 2012-2013 by Guoke Microelectronics Co.,Ltd
##
## Version: $Id: makefile 2 2014-08-07 07:42:50Z huangjunlei $
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
##
################################################################################
################################################################################

TOPDIR = ../..
SUBDIR += subsystem/zbar


include $(TOPDIR)/env/make/Makefile.config


HEADERS += ./include/*.h
HEADER_SUBDIR = ${SUBDIR}

################################################################################
# define target name
################################################################################

LIBRARY     = libzbar

################################################################################
# source files
################################################################################


CCFLAGS_USER += -I ./include
CCFLAGS_USER += -I ./src
CCFLAGS_USER += -D __DEBUG__ -w

SOURCES  	+= src/decoder.c
SOURCES 	+= src/img_scanner.c
SOURCES 	+= src/scanner.c
SOURCES 	+= src/image.c
SOURCES 	+= src/refcnt.c
SOURCES 	+= src/error.c 
SOURCES 	+= src/symbol.c
SOURCES		+= src/decoder/qr_finder.c
SOURCES 	+= src/qrcode/bch15_5.c
SOURCES 	+= src/qrcode/binarize.c
SOURCES 	+= src/qrcode/isaac.c
SOURCES 	+= src/qrcode/qrdec.c
SOURCES 	+= src/qrcode/qrdectxt.c
SOURCES 	+= src/qrcode/rs.c
SOURCES 	+= src/qrcode/util.c


################################################################################
# define user targets
################################################################################

default: build

clean: clean-objects clean-library 

build: build-objects build-library install-headers  install-library

headers:

install: build


all: clean build


	
################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += $(SOURCES)
  RELEASE_FILES += $(wildcard firmware/*.bin)
  RELEASE_FILES += $(wildcard inc/*)
  RELEASE_FILES += $(wildcard ispBin/sensors/*.bin)
  RELEASE_FILES += $(wildcard sensors/*)
  RELEASE_FILES += makefile
  CCFLAGS_USER += -I$(TOPDIR)/$(SUBDIR)/inc -I./inc
  LINTFLAGS_USER += -e923
  LINTFLAGS_USER += -e970

release: release-files

doc: $(DIRECTORIES:%=subdir-doc-%)

include $(TOPDIR)/env/make/Makefile.rules

################################################################################
# include optional dependencies
################################################################################

-include $(DEPENDENCY_FILENAME)
