################################################################################
################################################################################
##
## Makefile -- Makefile for isp3a library
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
SUBDIR += subsystem/wireless

include $(TOPDIR)/env/make/Makefile.config
include $(TOPDIR)/app/netcam/module_config


DEVMODE = $(shell [ -f Makefile.dev ] && echo y)

ifeq ($(DEVMODE), y)
  include Makefile.dev
else
  include Makefile.rel
endif
#echo CCFLAGS_USERxx=$(CCFLAGS_USER)
include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep
