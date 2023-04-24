################################################################################
################################################################################
##
## Makefile -- Makefile for snd-soc-gk_ipcam module
##
## (C) Copyright 2012-2017 by Goke Microelectronics Co.,Ltd
##
## Version: $Id: Makefile 13657 2018-05-14 02:54:03Z yulindeng $
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
  SUBDIR = linux/kernel/drv/snd/alsa
  PWDDIR = $(shell pwd)

include $(TOPDIR)/env/make/Makefile.config

DEVMODE = $(shell [ -f Makefile.dev ] && echo y)

ifeq ($(DEVMODE), y)
  include Makefile.dev


else
  include Makefile.rel


endif







include $(TOPDIR)/env/make/Makefile.rules


-include $(TOPDIR)/env/make/Makfile.dep


