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

TOPDIR = ../../../
SUBDIR += midware/zbar/test


include $(TOPDIR)/env/make/Makefile.config
#COMPILE_LEVEL = -O0
################################################################################
# define target name
################################################################################

PROGRAM     = zbar_test

################################################################################
# source files
################################################################################

# include $(SENSOR_DRV_DIR)/makefile.sensors
# CCFLAGS_USER += -I $(TOPDIR)/apps/board/$(BOARD)
CCFLAGS_USER += -I ./include
CCFLAGS_USER += -D __DEBUG__ -w
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/midware

SOURCES  	+= src/main.c

################################################################################
# define library used
################################################################################
LIBRARIES += zbar


################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-program

build: build-objects build-program  

headers:

install: build install-program install-bin

all: clean build install 


	
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
