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

  TOPDIR = ../..
  SUBDIR = linux/rootfs

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
  RELEASE_FILES += rootfs.mak
  RELEASE_FILES += $(wildcard configs/*)
  RELEASE_FILES += $(wildcard arm-gk720x-linux-gnueabi/*.tar.bz2)
  RELEASE_FILES += $(wildcard arm-gk720x-linux-gnueabi/resource/*)
  RELEASE_FILES += $(wildcard arm-gk720x-linux-uclibcgnueabi/*.tar.bz2)
  RELEASE_FILES += $(wildcard arm-gk720x-linux-uclibcgnueabi/resource/*)
  RELEASE_FILES += $(wildcard tools/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-glibc/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-glibc/*/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-glibc/*/*/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-glibc/*/*/*/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-glibc/*/*/*/*/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-glibc/*/*/*/*/*/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-uClibc/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-uClibc/*/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-uClibc/*/*/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-uClibc/*/*/*/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-uClibc/*/*/*/*/*)
  RELEASE_FILES += $(wildcard oss-prebuild-gk720x-uClibc/*/*/*/*/*/*)


release: release-files $(DIRECTORIES:%=subdir-release-%)


include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep

