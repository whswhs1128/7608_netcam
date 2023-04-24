
CFLAGES = -I./inc 

OBJS_DIR = ./objs
SRCS_DIR = ./src
BIN_DIR = ../bin

IMAGE_DIR = ../../rootfs/bin
ifeq ($(CONFIG_FILE),)
CONFIG_FILE = flashmap_gk7101.ini
endif

ifeq ($(UPGRADE_FILE),)
UPGRADE_FILE = gk_upgrade.bin
endif


## IPC板端升级工具，需要使用到交叉编译工具连 : arm-goke-linux-gnueabi-gcc
UPGRADE = $(OBJS_DIR)/upgrade_
#UPGRADE_CC = ${CROSS_COMPILE}gcc
#UPGRADE_STRIP = ${CROSS_COMPILE}strip
UPGRADE_CC = aarch64-mix210-linux-gcc
UPGRADE_STRIP = aarch64-mix210-linux-strip
UPGRADE_SRCS = $(SRCS_DIR)/flash_ctl.c $(SRCS_DIR)/crc32.c $(SRCS_DIR)/parser_cfg.c $(SRCS_DIR)/upgrade_flash.c $(SRCS_DIR)/md5.c $(SRCS_DIR)/convert_ascii.c $(SRCS_DIR)/convert_big_little_endian.c
UPGRADE_OBJS = $(UPGRADE)flash_ctl.o $(UPGRADE)crc32.o $(UPGRADE)parser_cfg.o $(UPGRADE)upgrade_flash.o $(UPGRADE)md5.o $(UPGRADE)convert_ascii.o $(UPGRADE)convert_big_little_endian.o
UPGRADE_TARGET = upgrade_flash


## PC端升级打包工具，需要使用本地系统的GCC编译工具
PACK = $(OBJS_DIR)/pack_
PACK_CC = gcc
PACK_SRCS = $(SRCS_DIR)/pack_upgrade_tool.c $(SRCS_DIR)/flash_ctl.c $(SRCS_DIR)/crc32.c $(SRCS_DIR)/parser_cfg.c $(SRCS_DIR)/pack_upgrade_tool.c $(SRCS_DIR)/md5.c $(SRCS_DIR)/convert_ascii.c $(SRCS_DIR)/convert_big_little_endian.c
PACK_OBJS = $(PACK)pack_upgrade_tool.o $(PACK)flash_ctl.o $(PACK)crc32.o $(PACK)parser_cfg.o $(PACK)md5.o $(PACK)convert_ascii.o $(PACK)convert_big_little_endian.o
PACK_TARGET = pack_upgrade_tool

CP = cp -rf
MV = mv
RM = rm -rf

.PHONY:   all do_pack_upgrade do_unpack_upgrade


#all: clean MKDIR $(UPGRADE_TARGET) $(PACK_TARGET)
all: clean MKDIR $(UPGRADE_TARGET)

MKDIR:
	@mkdir $(OBJS_DIR) -p
	@mkdir $(BIN_DIR) -p
	
############ 编译生成IPC板端升级程序 #################
$(UPGRADE_TARGET):$(UPGRADE_OBJS)
	$(UPGRADE_CC) -o  $@  $^ $(CFLAGES)
	$(UPGRADE_STRIP) $@ 
	-$(MV) $(UPGRADE_TARGET)  $(BIN_DIR)

$(UPGRADE)flash_ctl.o: $(SRCS_DIR)/flash_ctl.c
	$(UPGRADE_CC) -c $<  $(CFLAGES) -o $@

$(UPGRADE)crc32.o: $(SRCS_DIR)/crc32.c
	$(UPGRADE_CC) -c $<  $(CFLAGES) -o $@

$(UPGRADE)parser_cfg.o: $(SRCS_DIR)/parser_cfg.c
	$(UPGRADE_CC) -c $<  $(CFLAGES) -o $@

$(UPGRADE)upgrade_flash.o: $(SRCS_DIR)/upgrade_flash.c
	$(UPGRADE_CC) -c $<  $(CFLAGES) -o $@
	
$(UPGRADE)md5.o: $(SRCS_DIR)/md5.c
	$(UPGRADE_CC) -c $<  $(CFLAGES) -o $@

$(UPGRADE)convert_ascii.o: $(SRCS_DIR)/convert_ascii.c
	$(UPGRADE_CC) -c $<  $(CFLAGES) -o $@	

$(UPGRADE)convert_big_little_endian.o: $(SRCS_DIR)/convert_big_little_endian.c
	$(UPGRADE_CC) -c $<  $(CFLAGES) -o $@		

############ 编译PC端升级打包程序 #####################
$(PACK_TARGET):$(PACK_OBJS)
	$(PACK_CC) -o  $@  $^ $(CFLAGES)
	-$(MV) $(PACK_TARGET) $(BIN_DIR)

$(PACK)pack_upgrade_tool.o: $(SRCS_DIR)/pack_upgrade_tool.c
	$(PACK_CC) -c $<  $(CFLAGES) -o $@

$(PACK)flash_ctl.o: $(SRCS_DIR)/flash_ctl.c
	$(PACK_CC) -c $<  $(CFLAGES) -o $@

$(PACK)crc32.o: $(SRCS_DIR)/crc32.c
	$(PACK_CC) -c $<  $(CFLAGES) -o $@

$(PACK)parser_cfg.o: $(SRCS_DIR)/parser_cfg.c
	$(PACK_CC) -c $<  $(CFLAGES) -o $@
	
$(PACK)md5.o: $(SRCS_DIR)/md5.c
	$(PACK_CC) -c $<  $(CFLAGES) -o $@
	
$(PACK)convert_ascii.o: $(SRCS_DIR)/convert_ascii.c
	$(PACK_CC) -c $<  $(CFLAGES) -o $@	

$(PACK)convert_big_little_endian.o: $(SRCS_DIR)/convert_big_little_endian.c
	$(PACK_CC) -c $<  $(CFLAGES) -o $@		

clean:
	-$(RM) $(UPGRADE_TARGET) $(UPGRADE_OBJS) $(PACK_TARGET) $(PACK_OBJS)
	-$(RM) $(OBJS_DIR)
	-$(RM) $(BIN_DIR)


##################  执行升级打包命令  ##############################
do_pack_upgrade:
	-$(CP) $(IMAGE_DIR)/custom.jffs2      $(BIN_DIR)/
	-$(CP) $(IMAGE_DIR)/resource.squashfs $(BIN_DIR)/
	-$(CP) $(IMAGE_DIR)/rootfs.squashfs   $(BIN_DIR)/
	-$(RM) $(BIN_DIR)/$(UPGRADE_FILE)
	$(BIN_DIR)/$(PACK_TARGET) -p  $(BIN_DIR)/$(CONFIG_FILE)  $(BIN_DIR)/$(UPGRADE_FILE)
	-chmod 777 $(BIN_DIR)/$(UPGRADE_FILE)

##################  执行升级打包命令  ##############################
do_unpack_upgrade:
	$(BIN_DIR)/$(PACK_TARGET) -u  $(BIN_DIR)/$(UPGRADE_FILE)


#$(TARGET):$(OBJS)
#	$(CC) -o  $@  $^ $(CFLAGES)
#	-$(CP) $(TARGET) /opt/goke/rootfs/root/
#	-$(CP) $(TARGET) /home/guixing/nfs/gk
#
#$(OBJS):%.o:%.c
#	$(CC) -c $<  $(CFLAGES) -o $@


