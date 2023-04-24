################################################################################
################################################################################
##
## Makefile -- makefile for application adidemo
##
## (C) Copyright 2012-2014 by Goke Microelectronics Co.,Ltd
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
##
################################################################################
################################################################################


CC=gcc
CSRCS += xml.c
CSRCS += main.c

OBJS1+=$(CSRCS:%.c=%.o)
OBJS    += $(addprefix $(OUT_DIR),$(notdir $(OBJS1)))

PRO=xml_test

all: $(PRO) $(OBJS)

$(PRO):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OUT_DIR)%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<
	


clean:
	$(RM) $(OBJS)
