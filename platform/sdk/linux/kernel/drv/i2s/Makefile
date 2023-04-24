################################################################################
################################################################################
##
## Makefile -- makefile for vi module
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

  TOPDIR = ../../../..
  SUBDIR = linux/kernel/drv/i2s
  PWDDIR = $(shell pwd)

include $(TOPDIR)/env/make/Makefile.config

include $(TOPDIR)/linux/kernel/drv/Makefile.drv

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
  RELEASE_FILES += $(wildcard Makefile)
  RELEASE_FILES += i2s.ko

release: clean build  $(DIRECTORIES:%=subdir-release-%)
	@make release-files


doc: $(DIRECTORIES:%=subdir-doc-%)

include $(TOPDIR)/env/make/Makefile.rules

################################################################################
# local directory specific rules
################################################################################



