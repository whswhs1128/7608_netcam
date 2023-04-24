/*
 * (C) Copyright 2012
 * Goke Micro Limited
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_CPU_GK7202
#define CONFIG_CMD_TFTPPUT

// note: add a new board
// 1. add a gpio map file at arch\arm\include\asm\arch-gkxxxx
// 2. add a board define at include\configs\gkxxxx.h
/*
 * GK7202 board
 */
//#define CONFIG_MACH_GK7202_FPGA
//#define CONFIG_MACH_GK7202_EVB_V1_1
//#define CONFIG_MACH_GK7205_EVB_V1_1
//#define CONFIG_MACH_7205_SC4236_V_1_0
//#define CONFIG_MACH_7202_SC2232_V_1_0
//#define CONFIG_MACH_7202_GC2053_V_1_0
//#define CONFIG_MACH_7202_GC2053_TB
//#define CONFIG_MACH_7202_GC2053_TB_38
//#define CONFIG_MACH_7205_GC2053_FACE
//#define CONFIG_MACH_7202_GC2053_TB_V2
#define CONFIG_MACH_7202_GC2053_TB_V3_IPC20
//#define CONFIG_MACH_7205_IPC20

/*
 * GK7202 board specific data
 */
#define BOARD_VERSION(a, b) (((a) << 16) + (b))
#define BOARD_MK_STR1(x)     #x
#define BOARD_MK_STR(x)     BOARD_MK_STR1(x)

/* TODO: board info configure */
#if defined(CONFIG_MACH_GK7202_FPGA)
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_PHY_USE_MII
#define CONFIG_CPU_GK7202
#define CONFIG_FPGA_BOARD
#define CONFIG_BOARD_TYPE               "fpga"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0
#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_A

#elif defined(CONFIG_MACH_GK7202_EVB_V1_1)
/*
 *  i2s GPIO config different between evb7202 and evb7205
 */
#define CONFIG_SYSTEM_USE_EXTERN_I2S
//#define CONFIG_SYSTEM_USE_EXTERN_I2S_FOR7205
#define CONFIG_SYSTEM_USE_SDCARD
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7202
#define CONFIG_BOARD_TYPE               "devkit-v1.1"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_B


#elif defined(CONFIG_MACH_GK7205_EVB_V1_1)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
#define CONFIG_SYSTEM_USE_SDCARD
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7205
#define CONFIG_BOARD_TYPE               "devkit-v1.1"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_B2
#define CONFIG_SYSTEM_VOUT              0   //0--Close VOUT(OK)  1--I80 8bit(OK)
                                            //2--RGB 8bit(OK)    3--RGB 24bit(OK)
                                            //4--RGB 16bit use to 24bit LCD data wire (OK)
                                            //5--RGB 8bit with spi0(undefined)
                                            //6--RGB 8bit with spi1(undefined)
#if CONFIG_SYSTEM_VOUT ==  0
#define CONFIG_STSTEM_DUL_VIN           0  //0--normal(DVP or DVP+DVP)   1--BT1120+DVP 
#endif

#elif defined(CONFIG_MACH_7205_SC4236_V_1_0)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7205
#define CONFIG_BOARD_TYPE               "SC4236_V_1_0"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_C

#elif defined(CONFIG_MACH_7202_SC2232_V_1_0)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7202
#define CONFIG_BOARD_TYPE               "SC2232_V_1_0"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_D
#elif defined(CONFIG_MACH_7202_GC2053_V_1_0)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
//#define CONFIG_SYSTEM_USE_SDCARD
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7202
#define CONFIG_BOARD_TYPE               "devkit-v1.1"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_E

#elif defined(CONFIG_MACH_7202_GC2053_TB)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
#define CONFIG_SYSTEM_USE_SDCARD
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7202
#define CONFIG_BOARD_TYPE               "gc2053-tb"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_F

#elif defined(CONFIG_MACH_7202_GC2053_TB_38)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
#define CONFIG_SYSTEM_USE_SDCARD
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7202
#define CONFIG_BOARD_TYPE               "gc2053-tb-38"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_H

#elif defined(CONFIG_MACH_7205_GC2053_FACE)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7205
#define CONFIG_BOARD_TYPE               "gc2053-face"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_G

#elif defined(CONFIG_MACH_7202_GC2053_TB_V2)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
#define CONFIG_SYSTEM_USE_SDCARD
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7202
#define CONFIG_BOARD_TYPE               "gc2053-tb-v2"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_I

#elif defined(CONFIG_MACH_7202_GC2053_TB_V3_IPC20)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
#define CONFIG_SYSTEM_USE_SDCARD
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7202
#define CONFIG_BOARD_TYPE               "gc2053-tb-v3_ipc20"
#define CONFIG_BOARD_VERSION_MAJOR      1       //v1.0
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_J

#elif defined(CONFIG_MACH_7205_IPC20)
#define CONFIG_SYSTEM_USE_EXTERN_I2S
#define CONFIG_SYSTEM_USE_SDCARD
#define CONFIG_PHY_USE_EXTERN_CLK
#define CONFIG_CPU_GK7205
#define CONFIG_BOARD_TYPE               "gk7205_ipc20"
#define CONFIG_BOARD_VERSION_MAJOR      1       
#define CONFIG_BOARD_VERSION_MINOR      0

#define CONFIG_MACH_GK_T26_R25
#define CONFIG_MACH_GK_T26_R25_K

#else
#error "Undefine gpio configure header file or no support. "
#endif


#define CONFIG_BOARD_VARSION_CODE       BOARD_VERSION(CONFIG_BOARD_VERSION_MAJOR, CONFIG_BOARD_VERSION_MINOR)

#define CONFIG_IDENT_STRING             " for GK7202 " CONFIG_BOARD_TYPE " (GOKE)"

/* Uboot Debug information */
#define CONFIG_BOARD_HARDWARE_INFO  1

/* Uboot fast startup option, only use in production. */
/* When startup, default used bootcmd parameters.  */
//#define CONFIG_UBOOT_FAST_STARTUP

/*Disable DCACHE */
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_ICACHE_OFF

#define CONFIG_SYS_USE_SPIFLASH

/* total memory available to uboot */
#define CONFIG_SYS_UBOOT_SIZE           (8*1024 * 1024)

#define CONFIG_BOARD_EARLY_INIT_F

/*
 *Physical Memory Map
 */

#define CONFIG_SYS_SDRAM_BASE           0xC0000000

#define CONFIG_USR_SDRAM_SIZE           0x10000    /* 64K */

#define CONFIG_SYS_TEXT_BASE            0xC0800000

#define CONFIG_SYS_MEMTEST_START       (CONFIG_SYS_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END         (CONFIG_SYS_SDRAM_BASE - CONFIG_SYS_UBOOT_SIZE)

#define CONFIG_NR_DRAM_BANKS            1   /* we have 1 bank of DDR RAM */

/*
 *Size of malloc() pool
 */
//#define CONFIG_SYS_MALLOC_LEN           (CONFIG_ENV_SIZE + 128 * 1024 * 5)
#define CONFIG_SYS_MALLOC_LEN           (1024 * 1024 * 4)

/*
 *env config
 */
/* bootstrap + u-boot + env + linux in spi flash */
//#define CONFIG_ENV_IS_IN_SPINAND
#define CONFIG_ENV_IS_IN_SPI_FLASH
//#define CONFIG_ENV_IS_IN_MMC
//#define CONFIG_SYS_MMC_ENV_DEV          0 //mmc dev number

#define CONFIG_ENV_OFFSET               0x40000
#define CONFIG_ENV_SIZE                 0x10000

/*
 *allow to overwrite serial and ethaddr
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_ENV_ADDR                 (CONFIG_ENV_OFFSET)
#define CONFIG_SYS_LONGHELP


#define CONFIG_SYS_PROMPT               "GK7202 # "
#define CONFIG_CHIP_NAME                "gk7202"

/*
 *Command interface options
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_SF
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_ELF
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE

/*
 *High Level Configuration Options
 */
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x100000 - GENERATED_GBL_DATA_SIZE)




/*
 *Buffers
 */
#define CONFIG_SYS_CBSIZE               1024      /* Console I/O Buffer Size */

/*
 *Print Buffer Size
 */
#define CONFIG_SYS_PBSIZE              (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/*
 *Boot Argument Buffer Size
 */
#define CONFIG_SYS_BARGSIZE            (CONFIG_SYS_CBSIZE)

/*-----------------------------------------------------------------------
 * Hardware drivers
 -----------------------------------------------------------------------*/
/*
 *GPIO
 */
#define CONFIG_GK7202_GPIO
/*
 *Timer
 */
#define CONFIG_SYS_HZ                   1000

/*
 *UART & serial
 */
#define CONFIG_GOKE_SERIAL
#define CONFIG_CONS_INDEX               0
#define CONFIG_BAUDRATE                 115200
#define CONFIG_SYS_BAUDRATE_TABLE       { 4800, 9600, 19200, 38400, 57600, 115200 }

/*
 *Ethernet
 */
#define CONFIG_DRIVER_GOKE_ETH

/*
 *mmc driver  SUPORT SD CARD
 */
#define CONFIG_CMD_MMC_UPDATE
#ifdef CONFIG_CMD_MMC_UPDATE
#define CONFIG_GENERIC_MMC
#define CONFIG_SPL_LIBDISK_SUPPORT

#define CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_MMC_GK
#define CONFIG_MMC_WIDE
#define CONFIG_MMC_FAT
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT
#define CONFIG_SUPPORT_VFAT
#define CONFIG_FS_FAT
#define CONFIG_FAT_WRITE

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_SYSTEM_USE_SDCARD
#endif
/*
*spi drvier support
*/
//#define CONFIG_GOKE_SPI
//#define CONFIG_CMD_SPI_EEPROM

/*
*if support sd or spi dma,need open irq
*/
#ifdef CONFIG_CMD_MMC_UPDATE
#define CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ            (4 * 1024)   /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ            (4 * 1024)   /* FIQ stack */
#endif


#define CONFIG_UBOOT_HEADER_MAGIC      0x474b3732
/*
 *SPI NANDFLASH
 */
#define CONFIG_GOKE_SPINAND
#define CONFIG_CMD_NAND
#define CONFIG_CMD_SPINAND
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_NAND_SFLASH_1X_R_1X_W_MODE
//#define CONFIG_NAND_SFLASH_4X_R_1X_W_MODE
//#define CONFIG_NAND_SFLASH_4X_R_4X_W_MODE
#define CONFIG_MTD_SPINAND_INTERECC   /*if use ecc in uboot, please set it in kernel*/

/*
 *SPI FLASH
 */
#define CONFIG_GOKE_SFLASH
#define CONFIG_ENV_SECT_SIZE            (0x00010000)
#define CONFIG_SYS_MAX_FLASH_SECT       (128)
#define CONFIG_SYS_MAX_FLASH_BANKS      1
#define CONFIG_SYS_FLASH_BASE           0x00000000
#define CONFIG_SYS_FLASH_EMPTY_INFO     /* flinfo indicates empty blocks */
#define CONFIG_SFLASH_1X_R_1X_W_MODE
//#define CONFIG_SFLASH_4X_R_1X_W_MODE
//#define CONFIG_SFLASH_4X_R_4X_W_MODE
//#define CONFIG_GOKE_SPI
//#define CONFIG_CMD_SPI_EEPROM

/*
 * USB
 */
#if 0
#define CONFIG_CMD_USB
#define CONFIG_MUSB_HCD
#define CONFIG_USB_GOKE
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION        1
#define CONFIG_MUSB_TIMEOUT 2000   // the timer isn't correct(udelay failed), so we set timeout value from 100000 (100ms)to 2000;
#endif

/*
 *MTD Support
 */
#undef CONFIG_MTD_DEVICE
#undef CONFIG_MTD_PARTITIONS
#undef CONFIG_CMD_MTDPARTS
//#define MTDIDS_DEFAULT                    "spinand0=GK7101-spinand"
//#define MTDPARTS_DEFAULT ""

/*
 *UBIFS Support
 */
//#define CONFIG_CMD_UBI                1
//#define CONFIG_CMD_MTDPARTS           1
//#define CONFIG_RBTREE                 1
//#define CONFIG_CMD_UBIFS              1
//#define CONFIG_LZO                    1

/*
 *Display(LCD) Support
 *Show logo in UBoot
 */
//#define CONFIG_CMD_VO_LOGO

/*
 *AUDIO & I2S Support
 */
//#define CONFIG_CMD_AUDIO
//#define CONFIG_GOKE_AUDIO

//#define CONFIG_CMD_I2S
//#define CONFIG_GOKE_I2S


/*
 *Boot zImage and Image
 */
#define CONFIG_GK_IPC
#define CONFIG_GK7202
#define CONFIG_CMD_BOOTZ
#define CONFIG_UBOOT_TO_KERNEL          1


//#define CONFIG_GK_HAL_ADDR              0xC0012000

#define CONFIG_U2K_PHY_TYPE             0xC0000000
#define CONFIG_U2K_ARM_FREQ             0xC0000004
//#define CONFIG_U2K_HAL_ADDR             0xC0000008

#define CONFIG_U2K_SOC_ADDR             0xC0000010
#define CONFIG_U2K_MEM_ADDR             0xC0000014
#define CONFIG_U2K_BSB_ADDR             0xC0000018
#define CONFIG_U2K_DSP_ADDR             0xC000001C
#define CONFIG_U2K_ENABLE               0xC0000020

#define CONFIG_U2K_USR_ADDR             0xC0000024

#define CONFIG_U2K_TOTAL_MEM            0xC0000028

#define CONFIG_U2K_PPM_ADDR             0xC000002C
#define CONFIG_U2K_BOOT_LOGO_ADDR       0xC0000030
#define CONFIG_U2K_BOOT_LOGO_PITCH      0xC0000034
#define CONFIG_U2K_BOOT_LOGO_SIZE       0xC0000038

/*-----------------------------------------------------------------------
 * Boot parameters
 -----------------------------------------------------------------------*/
#define CONFIG_HOSTNAME                 "gk7202"

/* definition SOC types */
#define CONFIG_SOC_GK7202               5

#define CONFIG_ETHADDR                  3C:97:0E:22:E1:20
#define CONFIG_IPADDR                   11.1.5.20
#define CONFIG_GATEWAYIP                11.1.5.1
#define CONFIG_NETMASK                  255.255.255.0
#define CONFIG_SERVERIP                 11.1.5.19
#define CONFIG_ROOTPATH                 "/opt/work"
#define CONFIG_BOOTFILE                 "zImage"            /* file to load */
#define CONFIG_LOADADDR                 0xC1000000          /* default load address */
#define CONFIG_SYS_LOAD_ADDR            CONFIG_LOADADDR     /* default load address */
#define CONFIG_CMDLINE_TAG              1                   /* enable passing cmdline */
#define CONFIG_SETUP_MEMORY_TAGS        1
#define CONFIG_SYS_MAXARGS              64                  /* max number of command args */
#define CONFIG_BOOTDELAY                5
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_USE_EXT_PHY              0

#define CONFIG_SOC_TYPE                 CONFIG_SOC_GK7202
#define CONFIG_KERNEL_MEM_SIZE          36     //MB
#define CONFIG_PPM_MEM_SIZE             2048   //KB
#define CONFIG_BSB_MEM_SIZE             2048   //KB
#define CONFIG_USR_MEM_SIZE             0      //KB

#undef  CONFIG_BOOTARGS                                     /* the boot command will set bootargs */


#define CONFIG_EXTRA_ENV_SETTINGS           \
    "arm_freq=0x01202D01\0"                 \
    "mem=" BOARD_MK_STR(CONFIG_KERNEL_MEM_SIZE) "M\0"        \
    "ppmsize=" BOARD_MK_STR(CONFIG_PPM_MEM_SIZE) "KB\0"      \
    "bsbsize=" BOARD_MK_STR(CONFIG_BSB_MEM_SIZE) "KB\0"      \
    "netdev=eth0\0"                         \
    "consoledev=ttySGK0\0"                  \
    "rootfstype=ubi.mtd=3 rootfstype=ubifs root=ubi0:rootfs\0"  \
    "sfkernel=0x50000\0"                     \
    "sfboot=setenv bootargs console=${consoledev},${baudrate} noinitrd mem=${mem} rw ${rootfstype} init=linuxrc ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}:${netdev} mac=${ethaddr} phytype=${phytype};sf probe 0 0;sf read ${loadaddr} ${sfkernel} ${filesize}; bootm\0"    \
    "nfsserver=11.1.5.19\0"                        \
    "tftpboot=setenv bootargs root=/dev/nfs nfsroot=${nfsserver}:${rootpath},proto=tcp,nfsvers=3,nolock ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}:${netdev} mac=${ethaddr} phytype=${phytype} console=${consoledev},${baudrate} mem=${mem};tftpboot ${bootfile};bootm\0"     \
    "phytype=0\0"

#define CONFIG_BOOTCOMMAND          \
    "run tftpboot"







/*-----------------------------------------------------------------------
 * FLASH CONFIG parameters
 -----------------------------------------------------------------------*/
//#include <asm/types.h>
//#include <asm/arch/gpio.h>

/**********************************************************************************
*
*	THE NAND CONFIG ON-OFF
*   ============================= GIGA START ================================
*   0.----->GD5F1GQ4UA-------------------->  #define PRI_SF_NAND_GD5F1GQ4UA
*	1.----->GD5F2GQ4UA-------------------->  #define PRI_SF_NAND_GD5F2GQ4UA  
*	2.----->GD5F4GQ4UA-------------------->  #define PRI_SF_NAND_GD5F4GQ4UA
*	3.----->GD5F1GQ4UC-------------------->  #define PRI_SF_NAND_GD5F1GQ4UC
*	4.----->GD5F2GQ4UC-------------------->  #define PRI_SF_NAND_GD5F2GQ4UC  
*	5.----->GD5F4GQ4UC-------------------->  #define PRI_SF_NAND_GD5F4GQ4UC
*	6.----->GD5F1GQ4UE-------------------->  #define PRI_SF_NAND_GD5F1GQ4UE
*	============================= GIGA END ==================================
*
*   ============================= ESMT START ================================
*	7.----->F50L512M41A------------------->  #define PRI_SF_NAND_F50L512M41A 
*	============================= ESMT END ==================================
*
*   ============================= ATO START =================================
*	8.----->ATO25D1GA--------------------->  #define PRI_SF_NAND_ATO25D1GA
*	============================= ATO END ===================================
*
*   ============================= WINBOND START =============================
*	9.---->W25N01GV---------------------->  #define PRI_SF_NAND_W25N01GV  
*	10.---->W25M161AV-W25N01GV------------>  #define PRI_SF_NAND_W25M161AV_W25N01GV
*	============================= WINBOND END ===============================
*
*   ============================= MXIC START ================================
*	11.---->MX35LF1GE4AB------------------>  #define PRI_SF_NAND_MX35LF1GE4AB
*	============================= MXIC END ==================================
*
*   ============================= DOSILICON START ===========================
*	12.---->DS35Q1GA---------------------->  #define PRI_SF_NAND_DS35Q1GA
*	============================= DOSILICON END =============================
*   ============================= XTX START ===========================
*	13.---->XT26G01A---------------------->  #define PRI_SF_NAND_XT26G01A
*	============================= XTX END =============================
*	
***********************************************************************************/


/**********************************************************************************
*
*	THE NOR CONFIG ON-OFF
*   ============================= Spansion START ===========================
*   0.----->S25FL004A--------------------->  #define PRI_SF_NOR_S25FL004A
*	1.----->S25FL008A--------------------->  #define PRI_SF_NOR_S25FL008A 
*	2.----->S25FL016A--------------------->  #define PRI_SF_NOR_S25FL016A
*	3.----->S25FL032P--------------------->  #define PRI_SF_NOR_S25FL032P
*	4.----->S25FL064A--------------------->  #define PRI_SF_NOR_S25FL064A  
*	5.----->FL128PIFL--------------------->  #define PRI_SF_NOR_FL128PIFL
*	============================= Spansion END =============================
*
*   ============================= MX START =================================
*   6.----->MX25L3206E-------------------->  #define PRI_SF_NOR_MX25L3206E
*	7.----->MX25L6465E-------------------->  #define PRI_SF_NOR_MX25L6465E 
*	8.----->MX25L12845-------------------->  #define PRI_SF_NOR_MX25L12845
*	9.----->MX25L1605D-------------------->  #define PRI_SF_NOR_MX25L1605D
*	10.----->MX25L6455E-------------------->  #define PRI_SF_NOR_MX25L6455E 
*	11.----->MX253235D--------------------->  #define PRI_SF_NOR_MX253235D
*	12.----->MX25L3255D-------------------->  #define PRI_SF_NOR_MX25L3255D
*	============================= MX END ===================================
*	
*   ============================= Winbond START ============================
*   13.----->W25Q16BV---------------------->  #define PRI_SF_NOR_W25Q16BV
*	14.----->W25Q32FV---------------------->  #define PRI_SF_NOR_W25Q32FV
*	15.----->W25Q64FV---------------------->  #define PRI_SF_NOR_W25Q64FV
*	16.----->W25Q128FV--------------------->  #define PRI_SF_NOR_W25Q128FV
*	17.----->W25Q256FV--------------------->  #define PRI_SF_NOR_W25Q256FV
*	============================= Winbond END ==============================
*
*   ============================= SST START ================================
*   18.----->SST26VF016-------------------->  #define PRI_SF_NOR_SST26VF016
*	============================= SST END ==================================
*
*   ============================= numonyx START ============================
*   19.----->N25Q128----------------------->  #define PRI_SF_NOR_N25Q128
*	============================= numonyx END ==============================
*
*   ============================= giga START ===============================
*   20.----->GD25Q32C---------------------->  #define PRI_SF_NOR_GD25Q32C
*	21.----->GD25Q64C---------------------->  #define PRI_SF_NOR_GD25Q64C
*	22.----->GD25Q128C--------------------->  #define PRI_SF_NOR_GD25Q128C
*	23.----->GD25Q256D--------------------->  #define PRI_SF_NOR_GD25Q256D
*	============================= giga END =================================
*
*   ============================= EON START ================================
*   24.----->EN25Q64----------------------->  #define PRI_SF_NOR_EN25Q64
*	25.----->EN25Q80C---------------------->  #define PRI_SF_NOR_EN25Q80C
*	26.----->ENQH64A----------------------->  #define PRI_SF_NOR_ENQH64A
*	27.----->EN25Q128---------------------->  #define PRI_SF_NOR_EN25Q128
*	============================= EON END ==================================
*
*   ============================= XMC START ================================
*   28.----->XM25QH32B--------------------->  #define PRI_SF_NOR_XM25QH32B
*	29.----->XM25QH64A--------------------->  #define PRI_SF_NOR_XM25QH64A
*	30.----->XM25QH128A-------------------->  #define PRI_SF_NOR_XM25QH128A
*	============================= XMC END ==================================
*
*   ============================= ISSI START ===============================
*   31.----->IS25LP032--------------------->  #define PRI_SF_NOR_IS25LP032
*	32.----->IS25LP064--------------------->  #define PRI_SF_NOR_IS25LP064
*	33.----->IS25LP128--------------------->  #define PRI_SF_NOR_IS25LP128
*	34.----->P25LQ032---------------------->  #define PRI_SF_NOR_P25LQ032
*	============================= ISSI END =================================
*
*   ============================= Fidelix START ============================
*   35.----->FM25Q64A---------------------->  #define PRI_SF_NOR_FM25Q64A
*	============================= Fidelix END ==============================
*
*   ============================= Boya START ===============================
*   36.----->BY25D80----------------------->  #define PRI_SF_NOR_BY25D80
*	37.----->BY25Q64AS--------------------->  #define PRI_SF_NOR_BY25Q64AS
*	============================= Boya END =================================
*
*   ============================= Puya START ===============================
*   38.----->P25Q80H----------------------->  #define PRI_SF_NOR_P25Q80H
*	============================= Puya END =================================
*
*   ============================= XTX START ================================
*   39.----->XT25F08B---------------------->  #define PRI_SF_NOR_XT25F08B
*   40.----->XT25F32B---------------------->  #define PRI_SF_NOR_XT25F32B
*	41.----->XT25F64B---------------------->  #define PRI_SF_NOR_XT25F64B
*	42.----->XT25F128B--------------------->  #define PRI_SF_NOR_XT25F128B
*	43.----->XT25F256B--------------------->  #define PRI_SF_NOR_XT25F256B
*	============================= XTX END ==================================
*
*   ============================= BoHong START =============================
*   44.----->BH25Q64BS--------------------->  #define PRI_SF_NOR_BH25Q64BS
*	45.----->BH25Q128AS-------------------->  #define PRI_SF_NOR_BH25Q128AS
*	============================= BoHong END ===============================
*
***********************************************************************************/



//extern gpio_cfg_t gk_all_gpio_cfg;


#define SFLASH_NAD_NOR_ALL_OPEN  //open support the sflash check all flash 


#ifndef SFLASH_NAD_NOR_ALL_OPEN

#define PRI_SF_NOR_MX25L6465E 
#define PRI_SF_NOR_S25FL032P
#define PRI_SF_NAND_GD5F2GQ4UC
#define PRI_SF_NAND_DS35Q1GA

#endif 




#endif   /* __CONFIG_H */

