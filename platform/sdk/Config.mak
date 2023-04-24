################################################################################
################################################################################
##
## Config.mak -- user specific development environment settings
##
##
## (C) Copyright 2012-2015  by Goke Microelectronics Co.,Ltd
##
## Copy this file intto Config.mak.<username> to protect your private settings
## from being overwritten the next time you get an update.
## The make environment will first check for file "Config.mak.<username>",
## it will use the file "Config.mak" only if the user specific configuration
## file does not exist. <username> should be the string reported by
## "echo %USERNAME%" if you are using the standard windows shell, Cygwin
## users should call "echo $USERNAME" instead.
##
################################################################################
################################################################################


################################################################################
#
# target/host/decoder/system settings
#
# mandatory setting
#
# none of these setting should be modified.
#
# Available values for DECODER:
#
# GK720X
#
################################################################################

  HOST    ?= linux

  TARGET  ?= arm11-gcc

  DECODER ?= GK720X

  SYSTEM  ?= linux

################################################################################
#
# linux specific [kernel/toolchain/clib] settings
#
# mandatory setting
#
# Available values for KERNEL_VER:
#
#  3.4.43    
#
# Available values for GCC_VER:
#
#  4.8.5
#
# Available values for C_LIBRARY:
#
#  glibc
#  uClibc
#
################################################################################

  KERNEL_VER ?= 3.4.43

  GCC_VER    ?= 4.8.5

  C_LIBRARY  ?= uClibc

################################################################################
#
# path settings
#
# mandatory setting
#
# SDK_TOP:
#  - Description: specify the absolute path directory where this file reside in.
#                 you must specify the value of this configuration before any
#                 'make' operation.
#  - Values     : MUST be current path directory.
#
# KERNEL_TOP:
#  - Description: specify the abolute path directory where kernel reside in.
#                 generally you don't need change this manually.
#  - Values     : decided by the $(KERNEL_VER). Don't change.
#
# UBOOT_TOP:
#  - Description: specify the abolute path directory where u-boot reside in.
#                 generally you don't need change this manually.
#  - Values     : decided by the $(SDK_TOP). Don't change.
#
################################################################################

  SDK_TOP    ?=

  KERNEL_TOP ?= $(SDK_TOP)/linux/kernel/$(KERNEL_VER)

  UBOOT_TOP  ?= $(SDK_TOP)/boot/u-boot

################################################################################
#
# toolchain parent directroy settings (optional)
#
# This macro can be used to override the default directory where the toolchain
# reside in.
#
# By default the toolchain will be installed into /opt/goke/
# Note:
#   This config CAN NOT be assigned with $(PWD), you can use $(SDK_TOP) instead.
#
################################################################################

TOOLCHAIN_PARENT_DIRECTORY ?= /opt/goke


################################################################################
#
# rootfs parent directroy settings (optional)
#
# This macro can be used to override the default directory where the rootfs
# reside in.
#
# By default the rootfs will be installed into /opt/goke/
# Note:
#   This config CAN NOT be assigned with $(PWD), you can use $(SDK_TOP) instead.
#
################################################################################

ROOTFS_PARENT_DIRECTORY ?= /opt/goke

################################################################################
#
# Install and Release Configuration (optional)
#
# Here you can specific your private installation and release directories.
# The macros $(RELEASE_MAJOR) and $(RELEASE_MINOR) can be used to create
# release specific sub directories.
#
################################################################################

  INSTALL_DIR   ?= $(TOPDIR)/install


################################################################################
#
# Global make arguments (optional)
#
# Here you can specific your private make arguments to be passed to every
# make call, such as:
#   REPORT      = 1           : to enable command line reporting
#
################################################################################

  REPORT      ?= 0


