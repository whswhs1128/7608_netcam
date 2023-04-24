################################################################################
################################################################################
##
## Makefile -- makefile for ADI test application
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


  TOPDIR = ../../../../
  SUBDIR = subsystem/acodec/amr/demo

include $(TOPDIR)/env/make/Makefile.config

################################################################################
# source files
################################################################################

SOURCES += amr_fmtConvert.c

################################################################################
# define define user cc flags
################################################################################

CCFLAGS_USER += -O3 -DDEBUG -g
CCFLAGS_USER += -I $(TOPDIR)/subsystem/acodec/amr/libamr/include
CCFLAGS_USER += -D USE_WAV_HEADER

################################################################################
# define define user ld flags
################################################################################

#LDFLAGS_USER += -lm

################################################################################
# define library used
################################################################################

LIBRARIES  = amr

################################################################################
# define target name
################################################################################

PROGRAM = amrConvert

################################################################################
# define local file to be installed
################################################################################

################################################################################
# define local file install location
# if this was commented out, the target will be installed into the default
# location:
#   $(ROOTFS_TOP)/usr/local/bin/
################################################################################


################################################################################
# define target install location
# if this was commented out, the target will be installed into the default
# location:
#   $(ROOTFS_TOP)/usr/local/bin/
################################################################################

# INSTALL_BIN_DIR =

################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-all

build:  build-objects build-program

install: build install-program install-bin

headers:

all: clean build install

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += Makefile
  RELEASE_FILES += $(SOURCES)

release: release-files $(DIRECTORIES:%=subdir-release-%) info

doc: $(DIRECTORIES:%=subdir-doc-%)

info: force
	@echo $(DIRECTORIES)


include $(TOPDIR)/env/make/Makefile.rules
-include $(TOPDIR)/env/make/Makefile.dep
