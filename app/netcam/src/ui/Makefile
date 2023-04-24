################################################################################
################################################################################
##
## Makefile -- makefile for ui library
##
## (C) Copyright 2012-2016 by Goke Microelectronics Co.,Ltd
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
##
################################################################################
################################################################################
TOPDIR = ../../../..
SUBDIR += app/netcam/src/ui
APPDIR =  $(TOPDIR)/app/netcam

include $(TOPDIR)/env/make/Makefile.config
include $(APPDIR)/module_config


LIBRARY = libapp_ui

HEADERS +=  
HEADER_SUBDIR = 

CCFLAGS_USER += -DMODULE_SUPPORT_GUI
CCFLAGS_USER += -I inc
CCFLAGS_USER += -I src

##############src############################
SOURCES +=  src/lv_hal_porting.c
SOURCES +=  src/app_ui_main.c
#SOURCES +=  src/lv_tutorial_keyboard.c
SOURCES +=  src/app_ui_face_detect_menu.c

CCFLAGS_USER += -O2
CCFLAGS_USER += -I lib/inc
CCFLAGS_USER += -I ./$(APPDIR)/inc
CCFLAGS_USER += -I ./$(APPDIR)/inc/cfg

CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/lvgl-5.1.1
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/jpeg
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/json
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/subsystem/wireless

CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/adi
CCFLAGS_USER += -I$(INSTALL_HEADER_DIR)/platform/sdk
CCFLAGS_USER += -I$(APPDIR)/src/common
CCFLAGS_USER += -I$(APPDIR)/src/third/face6323
CCFLAGS_USER += -I$(APPDIR)/src/third/face6323/inc


################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-library clean-all

build:  build-objects build-library install-library pre-install

headers: install-headers

install: build install-library  

all: clean build install

pre-install:
	@echo ""
	@echo "Install ui lib ...."
	@echo ""
################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += Makefile.rel
  RELEASE_FILES += $(wildcard include/*.h   )
  RELEASE_FILES += Makefile
  RELEASE_FILES += lib/$(LIBRARY)$(LIBRARY_EXT)
  RELEASE_FILES += lib/$(LIBRARY)$(SHARED_EXT)

release: release-lib-prepare  doc release-files $(DIRECTORIES:%=subdir-release-%)

doc:

release-lib-prepare: clean build-library install-release-library

include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep
