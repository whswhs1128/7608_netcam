/*******************************************************************************
 *
 * Copyright (c)
 *
 *  \file parser_cfg.h
 *
 *  \brief parse config file.
 *
 *   create version  v 1.0.0  wangguixing
 *
 ******************************************************************************/

#ifndef _PARSER_CFG_H_
#define _PARSER_CFG_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define MAGIC_NUM           "GOKE_IPCAM_FIRMWARE"    /* ���� GKIC */

#define MAX_MTD_NUM         16              /*  ���֧��16 ��MTD���� */
#define MAX_MTD_SIZE        0x2000000       /*  32MB . MTD ���֧�ֵ����� */
#define MIMI_MTD_SIZE       0x1000          /*  4KB.  MTD ��С֧�ֵ�����   */

#define MAX_NORFLASH_SIZE   0x2000000       /*  32MB . nor_flash���֧�ֵ����� */
#define MIMI_NORFLASH_SIZE  0x200000        /*  2MB.    nor_flash��С֧�ֵ�����  */

#define MTD_PATH            "/proc/mtd"
#define FIRMWARE_START      "[FIRMWARE]"
#define FIRMWARE_END        "[FIRMWARE_END]"

#define UPGRADE_NORMAL_MTD  1
#define UPGRADE_ALL_MTD     2

#define PCFG_DEBUG 0
#if PCFG_DEBUG
#define DEBUG_PRT(fmt, args...) \
do{ \
    printf("[FUN]%s [LINE]%d  "fmt, __FUNCTION__, __LINE__, ##args); \
}while(0)
#else
#define DEBUG_PRT(fmt, args...)
#endif

#define ERROR_PRT(fmt, args...) \
do{ \
    printf("\033[0;32;31m ERROR! [FUN]%s [LINE]%d  "fmt"\033[0m", __FUNCTION__, __LINE__, ##args); \
}while(0)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/*
  * �ýṹ��Ҫ��4�ֽڶ���, ����д�������ļ���ͷ��
  * ע��ýṹ����д���������ļ��еģ�Ϊ�˱������
  * �汾ǰ���������,  ���Ըýṹ��һ��ȷ��������Ҫ�޸�
  * Ϊ��, �Ѿ�Ԥ������64 ���ֽڵı���λ, �����Ժ���չ. 4�ֽڶ���
*/
struct gk_upgrade_header
{
    char  magic_num[32];        /* ����                         */
    char  image_ver[32];        /* ����汾��       */
    char  Platform[32];         /* ƽ̨����             */
    char  machine_type[32];     /* ��Ʒ����             */

    unsigned int  upgrade_flag; /* ������־, 0:��ʾ������; 1:��ʾ����  */
    unsigned int  flash_size;   /* ����flash ������С */
    unsigned int  file_type;    /* �����ļ����� */
    unsigned int  file_size;    /* �����ļ���С������ͷ(gk_file_header) */
    unsigned int  file_crc32;   /* �����ļ�crc32У���      */

    unsigned int  mtd_addr;     /* ��дƫ��λ�� (�������flash������: 8MB or 16MB)    */
    unsigned int  mtd_offset;   /* Ҫ���������ڷ����ڵı��˵�ַ, ��������flash��ƫ�Ƶ�ַ */
    unsigned int  mtd_size;     /* �����ļ�ָ��flash �����Ĵ�С */
    unsigned int  mtd_num;      /* �����ļ�ָ��flash ������ */
    char          mtd_part[32]; /* ָ��Ҫ������MTD �����豸��: /dev/mtd3  */
    char          mtd_name[32]; /* ָ��Ҫ������MTD ����������: rootfs , uboot */
    char          pack_md5[16];   /* �����ļ�md5*/

    char          reserve[48];
};
typedef struct gk_upgrade_header GK_UPGRADE_HEADER_S;


/* �ṹ�����ڽ���linux ��/proc/mtd ��mtd ��Ϣ */
struct proc_mtd_info
{
    unsigned int mtd_size;      /* �� MTD �����Ĵ�С */
    unsigned int mtd_addr;      /* �� MTD �������������FLASH �ռ�ĵ�ַ */
    char mtd_part[32];          /* �� MTD �ķ����� ��: /dev/mtd0,  /dev/mtd1  ...  */
    char mtd_name[32];          /* �� MTD �ķ����� ��: uboot, kernel, rootfs ... */
};


/*
  * �ṹ����������upgrade.bin ��һ������ģ�����Ϣ,
  * ����Ϣʹ���ڰ�˵�������
  */
struct upgrade_file_info
{
    struct gk_upgrade_header upgrade_header; /* ����ԭ�ļ���ͷ��Ϣ  */
    unsigned int file_addr;     /* ����ԭ�ļ��ڴ���ļ���λ�� (������ͷ)  */
    unsigned int file_size;     /* ����ԭ�ļ��Ĵ�С (������ͷ)  */
};


/*
  * �ṹ����������IPCģ����Ϣ,
  * ����Ϣʹ���ڰ�˵������������ж������ļ���IPC�Ƿ�ƥ�䡣
  */
struct gk_platform_info
{
    char  Platform[32];         /* ƽ̨����             */
    char  machine_type[32];     /* ��Ʒ����             */
    char  image_ver[32];        /* ����汾��       */
};

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/******************************************************************************
 * ��������:  ��ȡ��ǰ���Ȱٷֱ�, �ú�����Ҫ�ڵ���
 *                            upgrade_flash_by_buf ������, ���ܿ�ʼʹ�û�ȡ��ȷ��ֵ.
 * �������:  ��
 *                           
 * �������:  0--100��ֵ. ����ʾ 0%--100%
 * ����ֵ      : 
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
unsigned int get_upgrade_rate(void);


/******************************************************************************
 * ��������: ͨ��linuxϵͳ��/proc/mtd ��Ϣ��ȡ��ǰ��MTD ������Ϣ
 * �������: ��
 *
 * �������: pmtd_info: ���õ�MTD��Ϣ�ṹ������,
 *                                             ע������鳤��ΪMAX_MTD_NUM(16)
 *                            pmtd_num: ������Ч��MTD��Ϣ�ṹ�����
 *
 * ����ֵ      : =0: �ɹ�
 *                            <0: ʧ��
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_mtdinfo_by_proc_mtd(struct proc_mtd_info *pmtd_info, int *pmtd_num);


/******************************************************************************
 * ��������: ͨ��linux /proc/mtd ��ȡ������ΪALL ��MTD ������Ϣ
 * �������: ��
 *
 * �������: pmtd_info: ���õ�MTD��Ϣ�ṹ��,�ýṹ����1������ָ��
 *                                              ��������.
 *
 * ����ֵ      : =0: �ɹ�
 *                            <0: ʧ��
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_all_mtd_info_by_proc(struct proc_mtd_info *pall_mtd);


/******************************************************************************
 * ��������:
 * �������:
 *
 * �������:
 * ����ֵ      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int pack_product_file(char *config_file, char *upgrade_bin);


/******************************************************************************
 * ��������: ͨ����������ļ�������, �������������ļ�
 * �������: config_file: ��������ļ����ļ���
 *                           upgrade_bin: Ҫ�������������ļ����ļ���
 *
 * �������: ��
 *
 * ����ֵ      : =0: �ɹ�
 *                            <0: ʧ��
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int pack_upgrade_file(char *config_file, char *upgrade_bin);

int unpack_upgrade_file(char *upgrade_bin);

int verify_upgrade_file(char *upgrade_bin);

int verify_upgrade_buff(char *upgrade_buf, unsigned int buf_size);

int get_upgradeinfo_by_file(char *upgrade_bin, struct upgrade_file_info *pinfo, int * mod_num);

int get_upgradeinfo_by_buff(char *upgrade_buf,
                                    unsigned int buf_size,
                                    struct upgrade_file_info *pinfo,
                                    int * mod_num);

int judge_upgrade_by_info(struct proc_mtd_info *pmtd_info,
                                  int mtd_num,
                                  struct upgrade_file_info *pupgrade_info);

int upgrade_flash_by_buf(char *upgrade_buf, unsigned int buf_size);

int upgrade_flash_by_file(char *upgrade_file);

/*�������ͷ��Ϣ���������е�����
magic_num����Ϊ"GK7101_IPCAM_FIRMWARE"
��image_verΪ"FFFFFFFF"����ǿ�����������
��image_ver��ͬ����machine_type�뱾����ͬ��������
����ͨ������0��ʧ�ܷ���-1
*/
int check_upgrade_header(struct gk_upgrade_header *pheader);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _PARSER_CFG_H_ */

