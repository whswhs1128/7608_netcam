################################################################################
################################################################################
##
## Makefile -- makefile for env
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

  TOPDIR = ..
  SUBDIR = env

include $(TOPDIR)/env/make/Makefile.config


################################################################################
# define user targets
################################################################################

default: 

clean: 

build: 

install: 

headers:

all: 

################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += Makefile
  RELEASE_FILES += $(wildcard make/*)
  RELEASE_FILES += $(wildcard configs/*)
  RELEASE_FILES += build_env.sh

release: release-files $(DIRECTORIES:%=subdir-release-%) 


include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep

