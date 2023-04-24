################################################################################
################################################################################
##
## Makefile -- makefile for subsystem modules.
##
## (C) Copyright 2012-2016 by Goke Microelectronics Co.,Ltd
##
## Version: $Id: Makefile 1 2018-07-12 07:09:45Z qiuqiwei $
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
## o DIRECTORIES = the list of sub directories to build
##
################################################################################
################################################################################

  TOPDIR = ..
  SUBDIR = subsystem

include $(TOPDIR)/env/make/Makefile.config

################################################################################
# project specific settings
################################################################################

DIRECTORIES += zlib

ifeq ($(release-software-midware-http) , y)
  DIRECTORIES += http
endif
DIRECTORIES += http
DIRECTORIES += json
DIRECTORIES += md5
DIRECTORIES += mediabuffer
DIRECTORIES += minirtsp
DIRECTORIES += onvif
DIRECTORIES += g711
DIRECTORIES += adpcm
DIRECTORIES += ad_pcm
#DIRECTORIES += ntpclient
DIRECTORIES += eventalarm
DIRECTORIES += log
DIRECTORIES += utility
DIRECTORIES += workqueue
DIRECTORIES += zbar
DIRECTORIES += wireless
#DIRECTORIES += aec
DIRECTORIES += osd
DIRECTORIES += ftp
DIRECTORIES += mail
DIRECTORIES += upnp
DIRECTORIES += yudp
DIRECTORIES += xml
DIRECTORIES += mbedtls-2.6.0
DIRECTORIES += wavechannel
DIRECTORIES += lvgl-5.1.1
DIRECTORIES += jpeg
DIRECTORIES += serverkey
################################################################################
# include library specific targets ( if available )
################################################################################


################################################################################
# define user targets
################################################################################

default: headers build install

clean: $(DIRECTORIES:%=subdir-clean-%) clean-install-dir

build: $(DIRECTORIES:%=subdir-build-%)

headers: $(DIRECTORIES:%=subdir-headers-%)

install: $(DIRECTORIES:%=subdir-install-%)

depend: $(DIRECTORIES:%=subdir-depend-%)

all: clean headers install

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
