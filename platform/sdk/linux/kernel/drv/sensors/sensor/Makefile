################################################################################
################################################################################
##
## Makefile -- makefile for sensor module
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
  SUBDIR = linux/kernel/drv/sensors/sensor
  PWDDIR = $(shell pwd)

include $(TOPDIR)/env/make/Makefile.config

################################################################################
# project specific settings
################################################################################
PRIVATE_CFLAGS += -I$(TOPDIR)/image/image_lib/include


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

headers: install-headers

install: force
	@$(MAKE) install-kernel-modules

depend:

all: clean headers build install

################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += $(wildcard Makefile)
  RELEASE_FILES += sensor.ko

release: clean build  
	@make release-files

doc:

include $(TOPDIR)/env/make/Makefile.rules

################################################################################
# local directory specific rules
################################################################################

include $(TOPDIR)/linux/kernel/drv/Makefile.drv


