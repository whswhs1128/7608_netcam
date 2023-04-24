#include <string.h>
#include "Ssmtp.h"
#include "cfg_network.h"

#include <pthread.h>
#include "netcam_api.h"
#include "utility_api.h"
#include "sdk_sys.h"



static int mail_run_flag = 0;

static int mail_send_flag = 0;
static pthread_mutex_t mail_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  mail_cond = PTHREAD_COND_INITIALIZER;

static void mail_enter_lock(void)
{
    pthread_mutex_lock(&mail_lock);
}

static void mail_leave_lock(void)
{
    pthread_mutex_unlock(&mail_lock);
}

int netcam_mail_send_get_status(void)
{
	int ret=0;
	mail_enter_lock();
	ret = mail_send_flag;
	mail_leave_lock();
	return ret;
}

static int netcam_mail_send_status(void)
{
	int ret=0;
	mail_enter_lock();
	while (!mail_send_flag)
		pthread_cond_wait(&mail_cond, &mail_lock);
	ret = mail_send_flag;
	mail_leave_lock();
	return ret;
}


void netcam_mail_send_set_status(int flag)
{
	mail_enter_lock();
	mail_send_flag = flag;

	if (mail_send_flag)
		pthread_cond_signal(&mail_cond);
	mail_leave_lock();
}



static int netcam_mail_send(char *jpegfilename)
{
	char errmsg[SMTP_ERROR_SIZE];
	SMTP_CFG cfg;

    memset(&cfg, 0, sizeof(cfg));
	cfg.attachPicture = runNetworkCfg.email.attachPicture;
	cfg.Usetls = runNetworkCfg.email.encryptionType;
	cfg.SmtpPort = runNetworkCfg.email.smtpPort;
	cfg.PopPort = runNetworkCfg.email.pop3Port;
	strncpy(cfg.Pop3Server, runNetworkCfg.email.pop3Server, MAX_STR_LEN_64-1);
	strncpy(cfg.SmtpServer, runNetworkCfg.email.smtpServer, MAX_STR_LEN_64-1);
	strncpy(cfg.FromAddr,runNetworkCfg.email.fromAddr, MAX_STR_LEN_64-1);
	strncpy(cfg.UserName,runNetworkCfg.email.eMailUser, MAX_STR_LEN_64-1);
	strncpy(cfg.PassWord, runNetworkCfg.email.eMailPass, MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr0, runNetworkCfg.email.toAddrList0, MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr1, runNetworkCfg.email.toAddrList1, MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr2, runNetworkCfg.email.toAddrList2, MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr3, runNetworkCfg.email.toAddrList3, MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr0, runNetworkCfg.email.ccAddrList0, MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr1, runNetworkCfg.email.ccAddrList1, MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr2, runNetworkCfg.email.ccAddrList2, MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr3, runNetworkCfg.email.ccAddrList3, MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr0, runNetworkCfg.email.bccAddrList0, MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr1, runNetworkCfg.email.bccAddrList1, MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr2, runNetworkCfg.email.bccAddrList2, MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr3, runNetworkCfg.email.bccAddrList3, MAX_STR_LEN_64-1);

	if (jpegfilename == NULL || 0 == runNetworkCfg.email.attachPicture)
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
		struct tm tt;
		sys_get_tm(&tt);
		snprintf(outfile, sizeof(outfile), "md_%04d%02d%02d_%02d%02d%02d.jpg", tt.tm_year, tt.tm_mon, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec);

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

static void* netcam_mail_send_thread(void *arg)
{
	sdk_sys_thread_set_name("netcam_mail_send_thread");
	while(mail_run_flag)
    {
        sleep(1);
        if(netcam_mail_send_status())
        {
            if(!access(MAIL_JPG_FILE, R_OK)){
				netcam_mail_send(MAIL_JPG_FILE);
			}else{
                netcam_mail_send(NULL);
			}
            netcam_mail_send_set_status(0);
        }
    }
    return NULL;
}


int netcam_mail_test_send(LPGK_NET_EMAIL_PARAM pEmail)
{
	char errmsg[SMTP_ERROR_SIZE];
	SMTP_CFG cfg;

    memset(&cfg, 0, sizeof(cfg));
	cfg.attachPicture   = 0;    /* ²»´ø¸½¼þ */
	cfg.Usetls          = pEmail->encryptionType;
	cfg.SmtpPort        = pEmail->smtpPort;
	cfg.PopPort         = pEmail->pop3Port;
	strncpy(cfg.Pop3Server, pEmail->pop3Server,   MAX_STR_LEN_64-1);
	strncpy(cfg.SmtpServer, pEmail->smtpServer,   MAX_STR_LEN_64-1);
	strncpy(cfg.FromAddr,   pEmail->fromAddr,     MAX_STR_LEN_64-1);
	strncpy(cfg.UserName,   pEmail->eMailUser,    MAX_STR_LEN_64-1);
	strncpy(cfg.PassWord,   pEmail->eMailPass,    MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr0,    pEmail->toAddrList0,  MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr1,    pEmail->toAddrList1,  MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr2,    pEmail->toAddrList2,  MAX_STR_LEN_64-1);
	strncpy(cfg.ToAddr3,    pEmail->toAddrList3,  MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr0,  pEmail->ccAddrList0,  MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr1,  pEmail->ccAddrList1,  MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr2,  pEmail->ccAddrList2,  MAX_STR_LEN_64-1);
	strncpy(cfg.CCToAddr3,  pEmail->ccAddrList3,  MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr0, pEmail->bccAddrList0, MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr1, pEmail->bccAddrList1, MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr2, pEmail->bccAddrList2, MAX_STR_LEN_64-1);
	strncpy(cfg.BCCToAddr3, pEmail->bccAddrList3, MAX_STR_LEN_64-1);

    strcpy(cfg.Subject,  "Network camera test Email");
    strcpy(cfg.TextBody, "This is testing Email!");

    if (ssmtp_transmit(&cfg, errmsg))
    {
        printf("ssmtp_transmit error! msg:[%s]\n", errmsg);
		printf("Network is too bad!\n");
        return -1;
    }

    return 0;
}


int netcam_mail_start()
{
	pthread_t mail_tid=0;
	if(!mail_run_flag)
	{
		mail_run_flag = 1;
		pthread_create(&mail_tid, NULL, netcam_mail_send_thread, NULL);
		if(0 == mail_tid)
		{
			printf("mail create thread failed!");
			return -1;
		}
		pthread_detach(mail_tid);
		return 0;
	}

	return -1;
}

int netcam_mail_stop()
{
	mail_run_flag = 0;
	return 0;
}

int netcam_mail_setting_is_ok()
{
	int ret = 0;
	if( netcam_mail_send_get_status() == 0 &&
		runNetworkCfg.email.enableEmail == 1 &&
		strlen(runNetworkCfg.email.eMailUser) != 0 &&
		strlen(runNetworkCfg.email.eMailPass) != 0 &&
		strlen(runNetworkCfg.email.toAddrList0) != 0 &&
		strlen(runNetworkCfg.email.smtpServer) != 0)
	{
		ret = 1;
	}

	return ret;

}

