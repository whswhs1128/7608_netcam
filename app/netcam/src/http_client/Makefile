################################################################################
################################################################################
##
## Makefile -- Makefile for http_clent library
##
## (C) Copyright 2020-2023 by Goke Microelectronics Co.,Ltd
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
##
################################################################################
################################################################################
TOPDIR = ../../../..
APPDIR =  $(TOPDIR)/app/netcam
SUBDIR += app/netcam/src/http_client
include $(TOPDIR)/env/make/Makefile.config
include $(TOPDIR)/app/netcam/module_config
include $(TOPDIR)/Config.mak
DEVMODE ?= $(shell [ -f Makefile.dev ] && echo y)

ifeq ($(DEVMODE), y)
  include Makefile.dev
else
  include Makefile.rel
endif

include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep
