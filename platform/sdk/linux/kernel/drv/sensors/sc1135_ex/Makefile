################################################################################
################################################################################
##
## Makefile -- makefile for sc1135_ex sensor  module
##
## (C) Copyright 2012-2015 by Goke Microelectronics Co.,Ltd
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
## o DIRECTORIES = the list of sub directories to build
##
################################################################################
################################################################################

  TOPDIR = ../../../../..
  SUBDIR = linux/kernel/drv/sensors/sc1135_ex
  PWDDIR = $(shell pwd)

include $(TOPDIR)/env/make/Makefile.config

################################################################################
# project specific settings
################################################################################


################################################################################
# include library specific targets ( if available )
################################################################################

################################################################################
# define user targets
################################################################################

default: build

clean: force
	@$(MAKE) clean-kernel-modules

build: force
	@$(MAKE) build-kernel-modules

headers: $(DIRECTORIES:%=subdir-headers-%)

install: force
	@$(MAKE) install-kernel-modules

depend: $(DIRECTORIES:%=subdir-depend-%)

all: clean headers build install

################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += ../../include/gk_sensor.h
  RELEASE_FILES += Makefile
  RELEASE_FILES += Kbuild
  RELEASE_FILES += sc1135_ex.c
  RELEASE_FILES += sc1135_pri.h
  RELEASE_FILES += sc1135_ex.ko

release: build $(DIRECTORIES:%=subdir-release-%)
	@make release-files

doc: $(DIRECTORIES:%=subdir-doc-%)

include $(TOPDIR)/env/make/Makefile.rules

################################################################################
# local directory specific rules
################################################################################

include $(TOPDIR)/linux/kernel/drv/Makefile.drv


