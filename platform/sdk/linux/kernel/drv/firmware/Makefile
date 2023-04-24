################################################################################
################################################################################
##
## Makefile -- makefile for firmware
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
  SUBDIR = linux/kernel/drv/firmware
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

default: install

clean:

build:

headers:

install: force
	@mkdir $(ROOTFS_TOP)/lib/firmware -p
	@echo "install firmware for $(CHIP_ID) family --> $(ROOTFS_TOP)/lib/firmware"
ifeq ($(CHIP_ID),GK720X)
	@echo "install firmware for GK720X"
	@cp ./gk_fw_720x.bin $(ROOTFS_TOP)/lib/firmware/gk_fw.bin
endif

depend: $(DIRECTORIES:%=subdir-depend-%)

all: clean headers build install

################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += Makefile
  RELEASE_FILES += gk_fw_720x.bin

release: release-files $(DIRECTORIES:%=subdir-release-%)

doc: $(DIRECTORIES:%=subdir-doc-%)

include $(TOPDIR)/env/make/Makefile.rules
force:

################################################################################
# local directory specific rules
################################################################################



