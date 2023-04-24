#include <string.h>
#include "Ssmtp.h"
#define MAX_STR_LEN_16 16
#define MAX_STR_LEN_20 20
#define MAX_STR_LEN_32 32
#define MAX_STR_LEN_64 64
#define MAIL_MAX_STR_LEN_64 64
#define MAX_STR_LEN_128 128
#define MAX_STR_LEN_256 256
typedef struct {
    int   enableEmail;      //是否启用
    int    attachPicture;    //是否带附件
    unsigned char    smtpServerVerify; //发送服务器要求身份验证
    unsigned char    mailInterval;     //最少2s钟(1-2秒；2-3秒；3-4秒；4-5秒)

    char    eMailUser[MAX_STR_LEN_64];    //账号
    char    eMailPass[MAX_STR_LEN_64];    //密码
    int   encryptionType;   /**< 加密类型 ssl*/
    char    smtpServer[MAX_STR_LEN_128];   //smtp服务器  //用于发送邮件    
    int   smtpPort;      /**< 服务器端口,一般为25，用户根据具体服务器设置 */
    char    pop3Server[MAX_STR_LEN_128];   //pop3服务器  //用于接收邮件,和IMAP性质类似   
    int   pop3Port;      /**< 服务器端口,一般为25，用户根据具体服务器设置 */
    char    fromAddr[MAX_STR_LEN_64]; 	/**< 发送人地址 */
    char    toAddrList0[MAX_STR_LEN_64];  /**< 收件人地址 1 */    
    char    toAddrList1[MAX_STR_LEN_64];  /**< 收件人地址 2 */    
    char    toAddrList2[MAX_STR_LEN_64];  /**< 收件人地址 3 */
    char    toAddrList3[MAX_STR_LEN_64];  /**< 收件人地址 4 */
    char    ccAddrList0[MAX_STR_LEN_64];  /**< 收件人地址 0 */
    char    ccAddrList1[MAX_STR_LEN_64];  /**< 收件人地址 1 */
    char    ccAddrList2[MAX_STR_LEN_64];  /**< 收件人地址 2 */
    char    ccAddrList3[MAX_STR_LEN_64];  /**< 收件人地址 3 */    
    char    bccAddrList0[MAX_STR_LEN_64]; /**< 密送人地址 0 */
    char    bccAddrList1[MAX_STR_LEN_64]; /**< 密送人地址 1 */
	char	  bccAddrList2[MAX_STR_LEN_64]; /**< 密送人地址 2 */
	char	  bccAddrList3[MAX_STR_LEN_64]; /**< 密送人地址 3 */
} GK_NET_EMAIL_PARAM, *LPGK_NET_EMAIL_PARAM;


int netcam_mail_send(char *jpegfilename,GK_NET_EMAIL_PARAM mail)
{
	char errmsg[SMTP_ERROR_SIZE];
	SMTP_CFG cfg;

    memset(&cfg, 0, sizeof(cfg));
	cfg.attachPicture = mail.attachPicture;
	cfg.Usetls = mail.encryptionType;
	cfg.SmtpPort = mail.smtpPort;
	cfg.PopPort = mail.pop3Port;
	strncpy(cfg.Pop3Server, mail.pop3Server, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.SmtpServer, mail.smtpServer, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.FromAddr,mail.fromAddr, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.UserName,mail.eMailUser, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.PassWord, mail.eMailPass, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr0, mail.toAddrList0, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr1, mail.toAddrList1, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr2, mail.toAddrList2, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr3, mail.toAddrList3, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr0, mail.ccAddrList0, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr1, mail.ccAddrList1, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr2, mail.ccAddrList2, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr3, mail.ccAddrList3, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr0, mail.bccAddrList0, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr1, mail.bccAddrList1, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr2, mail.bccAddrList2, MAIL_MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr3, mail.bccAddrList3, MAIL_MAX_STR_LEN_64-1);

	if (jpegfilename == NULL || 0 == mail.attachPicture)
	{
        if (ssmtp_transmit(&cfg, errmsg))
            printf("ssmtp_transmit error! msg:[%s]\n", errmsg);
    }
    else
	{
		FILE *f = NULL;
		struct list_head head={NULL, NULL};
		struct list_head *pos;

		char outfile[64]={0};
		//struct tm tt;
		//sys_get_tm(&tt);
		//snprintf(outfile, sizeof(outfile), "md_%04d%02d%02d_%02d%02d%02d.jpg", tt.tm_year, tt.tm_mon, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec);
		//snprintf(outfile, sizeof(outfile), "md_%04d%02d%02d_%02d%02d%02d.jpg",2017,9,5,12,0,0);
		strcpy(outfile,jpegfilename);
		INIT_LIST_HEAD(&head);
        //JPEG_PICTURE *jpeg = (JPEG_PICTURE *)malloc(sizeof(JPEG_PICTURE));
        JPEG_PICTURE jpeg_base, *jpeg = &jpeg_base;
        memset(jpeg,0,sizeof(JPEG_PICTURE));
        f=fopen(jpegfilename,"rb");
        if (!f)
        {
            printf("fail to open file: %s\n", jpegfilename);
            return -1;
        }
        fseek(f,0,SEEK_END);
        jpeg->length=ftell(f)+1;
        fseek(f,0,SEEK_SET);
        fread(jpeg->buf,1,jpeg->length,f);
        fclose(f);
        strncpy(jpeg->file_name, outfile, sizeof(jpeg->file_name));

        //INIT_LIST_HEAD(&head);
        //list_add_tail(&(jpeg->jpeg_list),&head);
        list_add_head(&(jpeg->jpeg_list),&head);
        int ret_ssmtp = 0; 
		ret_ssmtp = ssmtp(&cfg,&head,errmsg);
		if (-1 == ret_ssmtp){
			printf("ssmtp failed!\n");
				list_for_each(pos,&head)
		    {
		        JPEG_PICTURE *jpeg_tmp;
		        jpeg_tmp = list_entry(pos,JPEG_PICTURE,jpeg_list);
		        if(jpeg_tmp)
		        {
		            printf(" ^^^^^^^^^free mail,%d\n", __LINE__);
		            //list_del(&jpeg_tmp->jpeg_list);
		        }
		    }
			return -1;
		}
		list_for_each(pos,&head)
	    {
	        JPEG_PICTURE *jpeg_tmp;
	        jpeg_tmp = list_entry(pos,JPEG_PICTURE,jpeg_list);
	        if(jpeg_tmp)
	        {
	            printf(" ^^^^^^^^^free mail,%d\n", __LINE__);
	            //list_del(&jpeg_tmp->jpeg_list);
	        }
	    }

    }

    return 0;
}

int main(int argc, char* argv[])
{
	char errmsg[SMTP_ERROR_SIZE];
	SMTP_CFG cfg;

	cfg.Usetls = 0;
	cfg.SmtpPort = 25;
	#if 1
	strncpy(cfg.SmtpServer, "smtp.163.com", MAX_SERVER_ADDR_LEN);
	strncpy(cfg.FromAddr, "gdouchenxx@163.com", MAX_SERVER_ADDR_LEN);
	strncpy(cfg.UserName, "gdouchenxx", LEN_128_SIZE);
	strncpy(cfg.PassWord, "a6691555", LEN_128_SIZE);
	strncpy(cfg.ToAddr0, "sanwenyu88@qq.com", MAX_SERVER_ADDR_LEN);
	#else
	strncpy(cfg.SmtpServer, "pop.qq.com", LEN_64_SIZE-1);
	strncpy(cfg.FromAddr, "1019170644@qq.com", LEN_64_SIZE-1);
	strncpy(cfg.UserName, "1019170644@qq.com", LEN_128_SIZE);
	strncpy(cfg.PassWord, "ffseqobpzujakuwf", LEN_128_SIZE);
	strncpy(cfg.ToAddr0, "xiangdong@gokemicro.com", LEN_64_SIZE-1);	
	strncpy(cfg.ToAddr1, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	strncpy(cfg.ToAddr2, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	strncpy(cfg.ToAddr3, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	strncpy(cfg.CCToAddr0, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	strncpy(cfg.CCToAddr1, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	strncpy(cfg.CCToAddr2, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	strncpy(cfg.BCCToAddr0, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	strncpy(cfg.BCCToAddr1, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	strncpy(cfg.BCCToAddr2, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	strncpy(cfg.BCCToAddr3, "xiangdong@gokemicro.com", LEN_64_SIZE-1);
	#endif
	#if 1
	GK_NET_EMAIL_PARAM mail;
	mail.enableEmail = 1;      //是否启用
    mail.attachPicture = 1;    //是否带附件
    mail.smtpServerVerify = 0; //发送服务器要求身份验证
    mail.mailInterval = 2;     //最少2s钟(1-2秒；2-3秒；3-4秒；4-5秒)

    strncpy(mail.eMailUser,"gdouchenxx@163.com",MAX_SERVER_ADDR_LEN);    //账号
    strncpy(mail.eMailPass,"a6691555", LEN_128_SIZE);    //密码
    mail.encryptionType = 0;   /**< 加密类型 ssl*/
    strncpy(mail.smtpServer,"smtp.163.com", MAX_SERVER_ADDR_LEN);   //smtp服务器  //用于发送邮件    
    mail.smtpPort = 25;      /**< 服务器端口,一般为25，用户根据具体服务器设置 */
	strncpy(mail.fromAddr,"gdouchenxx@163.com", MAX_SERVER_ADDR_LEN); 	/**< 发送人地址 */
    strncpy(mail.toAddrList0,"sanwenyu88@qq.com", MAX_SERVER_ADDR_LEN);
	netcam_mail_send("/home/2.txt",mail);
	#else
	
	if (ssmtp_transmit(&cfg, errmsg))
		printf("ssmtp_transmit error! msg:[%s]\n", errmsg);
	#endif
	return 0;
}

