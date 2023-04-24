include ../Makefile.param
RANLIB = true
ARFLAGS = rcv

ROOT_DIR := .
DEPFILE := dep

SRC_ROOT            :=   $(ROOT_DIR)/src
OBJ_ROOT            :=   $(ROOT_DIR)/obj

DEP_ROOT            :=   $(OBJ_ROOT)

OUT_LIB_DIR :=   $(LIBS_PATH)/$(CHIP_ARCH)/libhik

ifeq ($(dbg),1)
    CFLAGS      += -DDEBUG_ON
    CFLAGS      += -g3 
    OUT_LIB     := $(OUT_LIB_DIR)/libhik.so
else
    CFLAGS      += -Os
    OUT_LIB     := $(OUT_LIB_DIR)/libhik.so
endif           


CFLAGS+=-ffunction-sections -fdata-sections -DWITH_NOIDREF  -DHI_OS_LINUX -DHI_OS_TYPE=HI_OS_LINUX -DCONFIG_NETWORK_USE_SYSTEM -DHI_LINUX_SUPPORT_UCLIBC -DHI_OS_SUPPORT_UCLINUX -DHI_MEMORY_DEBUG


#****源代码路径********************
SRC_DIR             :=  $(SRC_ROOT)
#***不需要包含的源代码路径*********************
#SRC_EXP_DIR         :=  $(INIPARSER_PATH)      \
#                        $(VSCP_PATH)

SRCS                :=  $(shell find $(SRC_DIR) -name '*.c')
SRCS                :=  $(sort $(SRCS))

#过滤掉不希望包含的文件路径
ifneq ($(strip $(SRC_EXP_DIR)),)
    SRCS_EXP        :=  $(shell find $(SRC_EXP_DIR) -name '*.c')
    SRCS            :=  $(filter-out $(SRCS_EXP),$(SRCS))
endif

OBJS                :=  $(SRCS:$(ROOT_DIR)%.c=$(OBJ_ROOT)%.o)
OBJ_DIR             :=  $(sort $(dir $(OBJS)))

INCLUDE_DIR         :=  $(SRC_ROOT)
ifneq ($(strip $(INCLUDE_EXP_DIR)),)
    INCLUDE_DIR     :=  $(filter-out $(INCLUDE_EXP_DIR),$(INCLUDE_DIR))
endif

INCLUDES            :=  $(shell echo "find $(INCLUDE_DIR) -name '*.h'";find $(INCLUDE_DIR) -name '*.h')
INCLUDE_DIR         :=  $(sort $(dir $(INCLUDES)))
INCLUDE_SPATH       :=  $(foreach dir,$(INCLUDE_DIR), -I$(dir))

#*********
LIB_INCLUDE_PATH     := $(INCLUDE_SPATH)
LIB_INCLUDE_PATH     += -I./inc
LIB_INCLUDE_PATH     += -I./inc/encrypt
LIB_INCLUDE_PATH     += -I$(LIBINC_PATH)
LIB_INCLUDE_PATH     += -I$(IPC_PATH)/ProtocolService/src


LIB_SRC :=
LIB_SRC += $(SRC_ROOT)/hik_net.c
LIB_SRC += $(SRC_ROOT)/base64.c
LIB_SRC += $(SRC_ROOT)/d3des.c
LIB_SRC += $(SRC_ROOT)/md5.c
LIB_SRC += $(SRC_ROOT)/posixWrapper.c
LIB_SRC += $(SRC_ROOT)/hmac.c
LIB_SRC += $(SRC_ROOT)/hik_netfuns.c
LIB_SRC += $(SRC_ROOT)/hik_netlogin.c
LIB_SRC += $(SRC_ROOT)/hik_lst.c
LIB_SRC += $(SRC_ROOT)/hik_security.c
LIB_SRC += $(SRC_ROOT)/hik_netutil.c

LIB_OBJS :=
LIB_OBJS += $(OBJ_ROOT)/hik_net.o
LIB_OBJS += $(OBJ_ROOT)/base64.o
LIB_OBJS += $(OBJ_ROOT)/d3des.o
LIB_OBJS += $(OBJ_ROOT)/md5.o
LIB_OBJS += $(OBJ_ROOT)/posixWrapper.o
LIB_OBJS += $(OBJ_ROOT)/hmac.o
LIB_OBJS += $(OBJ_ROOT)/hik_netfuns.o
LIB_OBJS += $(OBJ_ROOT)/hik_netlogin.o
LIB_OBJS += $(OBJ_ROOT)/hik_lst.o
LIB_OBJS += $(OBJ_ROOT)/hik_security.o
LIB_OBJS += $(OBJ_ROOT)/hik_netutil.o

all:
	@echo "*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "* CREATE $(OUT_LIB_DIR)"
	$(if $(shell find $(OUT_LIB_DIR)), $(shell cd ./), $(shell mkdir $(OUT_LIB_DIR)))
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/hik_net.c -o $(OBJ_ROOT)/hik_net.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/base64.c -o $(OBJ_ROOT)/base64.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/d3des.c -o $(OBJ_ROOT)/d3des.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/md5.c -o $(OBJ_ROOT)/md5.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/posixWrapper.c -o $(OBJ_ROOT)/posixWrapper.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/hmac.c -o $(OBJ_ROOT)/hmac.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/hik_netfuns.c -o $(OBJ_ROOT)/hik_netfuns.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/hik_netlogin.c -o $(OBJ_ROOT)/hik_netlogin.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/hik_lst.c -o $(OBJ_ROOT)/hik_lst.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/hik_security.c -o $(OBJ_ROOT)/hik_security.o
	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -Wall -c $(SRC_ROOT)/hik_netutil.c -o $(OBJ_ROOT)/hik_netutil.o

	$(GCC) $(CFLAGS) $(LIB_INCLUDE_PATH) -fPIC -shared -fvisibility=hidden -Wl,--gc-sections -o $(OUT_LIB) $(LIB_OBJS)
	$(AR) -rcv $(OUT_LIB_DIR)/libhik.a $(LIB_OBJS)
	
-include $(DEPFILE)

.PHONY: cleanall clean 

cleanall: clean
	@echo "DEL depends files"
	@rm -f $(SRC_DEPS)
	@echo "--------------------------------------------"    	
clean:
	@echo "DEL OBJ files"
	@rm -f $(OBJS)
	@rm -f $(LIB_OBJS)
	@echo "DEL LIBS file $(OUT_LIB) $(OUT_LIB_DIR)/*.a $(LIB_OBJS)"
	@rm -f $(DEPFILE)  $(OUT_LIB_DIR)/*.a 
	@rm -f $(OUT_LIB)  $(OUT_LIB_DIR)/*.so 
	
	
