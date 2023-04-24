################################################################################
################################################################################
##
## Makefile -- makefile for top level modules.
##
## (C) Copyright 2012-2013 by Goke Microelectronics Co.,Ltd
##
## Version: $Id: Makefile 8919 2016-07-08 10:24:49Z qiuqiwei $
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
## o DIRECTORIES = the list of sub directories to build
##
################################################################################
################################################################################

  TOPDIR = .
  SUBIDR = .

include $(TOPDIR)/env/make/Makefile.config

DIRECTORIES += platform
DIRECTORIES += subsystem
DIRECTORIES += app

################################################################################
# define user targets
################################################################################

default: pre_update headers build install
#default: pre_update

pre_update:
	#/bin/sh  ./pre_update.sh

clean: $(DIRECTORIES:%=subdir-clean-%) clean-install-dir

build: $(DIRECTORIES:%=subdir-build-%)

headers: $(DIRECTORIES:%=subdir-headers-%)

install: $(DIRECTORIES:%=subdir-install-%)

depend: $(DIRECTORIES:%=subdir-depend-%)

all: clean pre_update headers build install

################################################################################
# include internal definitions and rules
################################################################################


  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += Config.mak
  RELEASE_FILES += Makefile
  RELEASE_FILES += ReleaseInfo.txt

release: release-all

release-subdirs: $(DIRECTORIES:%=subdir-release-%)

info: force	@echo $(DIRECTORIES)


release-all:
#	@$(SVN) update
	@$(SUBMAKE) release-clean
	@$(SUBMAKE) release-info
	@$(SUBMAKE) release-files
	@$(SUBMAKE) release-subdirs
	@$(SUBMAKE) release-bz2
	@$(SUBMAKE) release-remove-dir

include $(TOPDIR)/env/make/Makefile.rules

################################################################################
# local directory specific rules
################################################################################

subdir-clean-%:
	@$(NEWLINE)
	@$(ECHO) $(ECHOPREFIX) Submaking clean [$(SUBDIR)/$(@:subdir-clean-%=%)] for target [$(TARGET)]
	@$(SUBMAKE) -C $(@:subdir-clean-%=%) clean TARGET=$(TARGET)

subdir-depend-%:
	@$(NEWLINE)
	@$(ECHO) $(ECHOPREFIX) Submaking depend [$(SUBDIR)/$(@:subdir-depend-%=%)] for target [$(TARGET)]
	@$(SUBMAKE) -C $(@:subdir-depend-%=%) depend TARGET=$(TARGET)

subdir-headers-%:
	@$(NEWLINE)
	@$(ECHO) $(ECHOPREFIX) Submaking headers [$(SUBDIR)/$(@:subdir-headers-%=%)] for target [$(TARGET)]
	@$(SUBMAKE) -C $(@:subdir-headers-%=%) headers TARGET=$(TARGET)

subdir-build-%:
	@$(NEWLINE)
	@$(ECHO) $(ECHOPREFIX) Submaking build [$(SUBDIR)/$(@:subdir-build-%=%)] for target [$(TARGET)]
	@$(SUBMAKE) -C $(@:subdir-build-%=%) build TARGET=$(TARGET)

subdir-install-%:
	@$(NEWLINE)
	@$(ECHO) $(ECHOPREFIX) Submaking install [$(SUBDIR)/$(@:subdir-install-%=%)] for target [$(TARGET)]
	@$(SUBMAKE) -C $(@:subdir-install-%=%) install TARGET=$(TARGET)

subdir-release-%:
	@$(NEWLINE)
	@$(ECHO) $(ECHOPREFIX) Submaking release [$(SUBDIR)/$(@:subdir-release-%=%)] for target [$(TARGET)]
	@$(SUBMAKE) -C $(@:subdir-release-%=%) release TARGET=$(TARGET)

subdir-doc-%:
	@$(NEWLINE)
	@$(ECHO) $(ECHOPREFIX) Submaking doc [$(SUBDIR)/$(@:subdir-doc-%=%)] for target [$(TARGET)]
	@$(SUBMAKE) -C $(@:subdir-doc-%=%) doc TARGET=$(TARGET)
