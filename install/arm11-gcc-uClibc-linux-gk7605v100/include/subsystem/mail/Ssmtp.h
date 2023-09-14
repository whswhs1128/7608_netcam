/****************************************************************************
* Copyright 2012 - 2015, IPCamera Tech. Co., Ltd.
* ALL rights reserved
* 文件名: ssmtp.h
* 文件描述: 系统升级烧写flash接口
*
* 当前版本:	1.1
* 作者:scott
* 创建日期: 2012-01-01
*****************************************************************************/
#ifndef __SSMTP_H__
#define __SSMTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdio.h>
#include "list.h"

#define SMTP_BUF_SIZE					4096	/* A pretty large buffer, but not outrageous */
#define SMTP_ERROR_SIZE					128

#define SMTP_CONNECT_TIMEOUT			10			/*连接10秒钟 超时*/
#define ARPADATE_LENGTH					32			/* Current date in RFC format */

#define SMTP_SUBJECT					"Network camera"
										/*At 9:56 p.m. on November 12, 2008 to at 10:01 p.m. on November 12, 2008*/
#define SMTP_TEXTBODY					"Your camera alarm setting is triggered."
//#define SMTP_TEST_TEXTBODY				"Your SMTP settings testing Successfully."

#define MAX_JPEG_FILE_NAME_LEN			256
#define SNAP_JPEG_DATA_SIZE				(1024*1024)

#define LEN_16_SIZE					16
#define LEN_24_SIZE					24
#define LEN_32_SIZE					32
#define LEN_48_SIZE					48
#define LEN_64_SIZE					64
#define LEN_128_SIZE				128
#define LEN_256_SIZE     			256
#define LEN_512_SIZE				512
#define LEN_1024_SIZE				1024
#define LEN_1536_SIZE				1536
#define LEN_1920_SIZE				1920
#define LEN_2048_SIZE				2048
#define LEN_4096_SIZE				4096
#define LEN_8192_SIZE				8192


#define MAX_SERVER_ADDR_LEN			256


typedef struct hi_JPEG_PICTURE
{
    struct	list_head jpeg_list;		/*用于维护链表*/
	int		chno;						/*通道号*/	
	unsigned int	length;						/*图片大小*/
	char	buf[SNAP_JPEG_DATA_SIZE];			/*图片缓冲区*/
	char	file_name[MAX_JPEG_FILE_NAME_LEN];
}JPEG_PICTURE;



typedef struct smtp_cfg
{											
	unsigned int	Enable;
	unsigned int attachPicture;
	unsigned int	Usetls;						/*使用SSL安全传输 0-无 1-SSL 2-TLS*/
	char Pop3Server[LEN_64_SIZE];
	unsigned short  PopPort;
	unsigned short  SmtpPort;
	char SmtpServer[LEN_64_SIZE];
	char FromAddr[MAX_SERVER_ADDR_LEN];
	char UserName[LEN_128_SIZE];
	char PassWord[LEN_128_SIZE];
	char ToAddr0[MAX_SERVER_ADDR_LEN];			
	char ToAddr1[MAX_SERVER_ADDR_LEN];			
	char ToAddr2[MAX_SERVER_ADDR_LEN];
	char ToAddr3[MAX_SERVER_ADDR_LEN];
	char CCToAddr0[MAX_SERVER_ADDR_LEN];			
	char CCToAddr1[MAX_SERVER_ADDR_LEN];			
	char CCToAddr2[MAX_SERVER_ADDR_LEN];
	char CCToAddr3[MAX_SERVER_ADDR_LEN];
	char BCCToAddr0[MAX_SERVER_ADDR_LEN];			
	char BCCToAddr1[MAX_SERVER_ADDR_LEN];
	char BCCToAddr2[MAX_SERVER_ADDR_LEN];
	char BCCToAddr3[MAX_SERVER_ADDR_LEN];

    char Subject[LEN_128_SIZE];
    char TextBody[LEN_256_SIZE];
}SMTP_CFG;
//DNS header structure
struct DNS_HEADER {
	unsigned short id; // identification number

	unsigned char rd :1; // recursion desired
	unsigned char tc :1; // truncated message
	unsigned char aa :1; // authoritive answer
	unsigned char opcode :4; // purpose of message
	unsigned char qr :1; // query/response flag

	unsigned char rcode :4; // response code
	unsigned char cd :1; // checking disabled
	unsigned char ad :1; // authenticated data
	unsigned char z :1; // its z! reserved
	unsigned char ra :1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure
struct QUESTION {
	unsigned short qtype;
	unsigned short qclass;
};

//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA {
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD {
	unsigned char *name;
	struct R_DATA *resource;
	unsigned char *rdata;
};
//return 
struct DNS_IPADDR {
	unsigned long ipaddr;
};


//Structure of a Query
typedef struct {
	unsigned char *name;
	struct QUESTION *ques;
} QUERY;

//Function Prototypes
void gethostbyname_mail(struct DNS_IPADDR *dns_addr,char*host, int query_type);
void ChangetoDnsNameFormat(unsigned char*dns, unsigned char*host);
unsigned char* ReadName(unsigned char*reader, unsigned char*buffer, int*count);
//void get_dns_servers();
int get_dns_servers(char *dns1, char *dns2);


/* arpadate.c */
void get_arpadate(char *);


/*发送电子邮件 支持SSL 支持多附件发送*/
int ssmtp_transmit(SMTP_CFG *ecfg, char *err_msg);

/*报警联动email发送图片*/
int ssmtp(SMTP_CFG *ecfg, struct list_head *pic_list, char *err_msg);

#ifdef __cplusplus
}
#endif
 
#endif
