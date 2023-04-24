#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
//
//#include "sdk_api.h"
#include "cfg_com.h"
//#include "common.h"
#include "netcam_api.h"
#include "ftplib.h"
#include "utility_api.h"
#include "sdk_sys.h"


static int ftp_run_flag = 0;

static int ftp_upload_flag = 0;
static pthread_mutex_t ftp_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  ftp_cond = PTHREAD_COND_INITIALIZER;


static void ftp_enter_lock(void)
{
    pthread_mutex_lock(&ftp_lock);
}

static void ftp_leave_lock(void)
{
    pthread_mutex_unlock(&ftp_lock);
}

int netcam_ftp_upload_get_status(void)
{
	int ret=0;
	ftp_enter_lock();
	ret = ftp_upload_flag;
	ftp_leave_lock();
	return ret;
}

int netcam_ftp_upload_status(void)
{
	int ret=0;
	ftp_enter_lock();
	while (!ftp_upload_flag)
		pthread_cond_wait(&ftp_cond, &ftp_lock);
	ret = ftp_upload_flag;
	ftp_leave_lock();
	return ret;
}


void netcam_ftp_upload_set_status(int flag)
{
	ftp_enter_lock();
	ftp_upload_flag = flag;

	if (ftp_upload_flag)
		pthread_cond_signal(&ftp_cond);
	ftp_leave_lock();
}

int netcam_ftp_check(char *ipaddr, int port, char *username, char * passdword)
{
	netbuf *conn = NULL;
	int ret=0;
	char ipaddress[64]={0};

	if(ipaddr == NULL ||
		port < 0 ||
		port > 0xFFFF ||
		username == NULL ||
		passdword == NULL )
	{

		return -1;
	}

	snprintf(ipaddress, sizeof(ipaddress), "%s:%d", ipaddr, port);

	FtpInit();                     //FTP 初始化
	ret = FtpConnect(ipaddress,&conn);//FTP 连接
	if(ret)
	{
		PRINT_INFO("connect is susseccful \r\n");
	}
	else
	{
		PRINT_INFO("connect is failed \r\nftp info :address:%s userName:%s password:%s \n",ipaddress, username, passdword);
		return -2;
	}

	ret = FtpLogin(username,passdword,conn);//FTP登陆
	if(ret)
	{
		PRINT_INFO("login is ok \r\n");
	}
	else
	{
		PRINT_ERR("login is error \r\n");
		return -3;
	}
	FtpClose(conn);
	return 0;

}


static int netcam_ftp_upload(char *ipaddr, int port, char *username, char * passdword, char *filename,char *uploadDir)
{
	netbuf *conn = NULL;
	int ret=0;
	char outfile[64]={0};
	char ipaddress[64]={0};

	struct tm tt;
	sys_get_tm(&tt);
	snprintf(outfile, sizeof(outfile), "md_%04d%02d%02d_%02d%02d%02d.jpg", tt.tm_year, tt.tm_mon, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec);

	if(ipaddr == NULL ||
		port < 0 ||
		port > 0xFFFF ||
		username == NULL ||
		passdword == NULL ||
		filename == NULL )
	{

		return -1;
	}


	snprintf(ipaddress, sizeof(ipaddress), "%s:%d", ipaddr, port);
	//sprintf(outfile, "%s%s", uploadDir, rindex(filename, '/'));

	FtpInit();                     //FTP 初始化
	//printf("ftp init \r\n");

	ret = FtpConnect(ipaddress,&conn);//FTP 连接
	if(ret)
	{
		PRINT_INFO("connect is susseccful \r\n");
	}
	else
	{
		PRINT_INFO("connect is failed \r\nftp info :address:%s userName:%s password:%s local:%s upload:%s\n",ipaddress, username, passdword, filename, outfile);
		//PRINT_ERR("connect is failed \r\n");
		return -1;
	}

	ret = FtpLogin(username,passdword,conn);//FTP登陆
	if(ret)
	{
		PRINT_INFO("login is ok \r\n");
	}
	else
	{
		PRINT_ERR("login is error \r\n");
		FtpClose(conn);
		return -2;
	}
	FtpMkdir(uploadDir, conn);
	FtpChdir(uploadDir, conn);//下载路径

	//printf("========set FtpOptions FTPLIB_PORT=========\r\n");
	FtpOptions(FTPLIB_CONNMODE,FTPLIB_PORT,conn);//设置FTP

	ret = FtpPut(filename,outfile,FTPLIB_IMAGE,conn);//FTP 上传文件
	if(ret)
	{
		PRINT_INFO("ftp upload \"%s\" is susseccful\r\n", outfile);
	}
	else
	{
		PRINT_ERR("ftp update  failed\r\n");
	}

	FtpClose(conn);
	return 0;

}

static void* netcam_ftp_upload_thread(void *arg)
{
	sdk_sys_thread_set_name("netcam_ftp_upload_thread");
    while(ftp_run_flag)
    {
        sleep(1);
        if(netcam_ftp_upload_status())
        {
            if(!access(FTP_JPG_FILE, R_OK))
				netcam_ftp_upload(runNetworkCfg.ftp.address,runNetworkCfg.ftp.port, runNetworkCfg.ftp.userName,runNetworkCfg.ftp.password,FTP_JPG_FILE,runNetworkCfg.ftp.datapath);
            netcam_ftp_upload_set_status(0);
        }
    }
    return 0;
}

int netcam_ftp_start()
{
	pthread_t ftp_tid=0;
	if(!ftp_run_flag)
	{
		ftp_run_flag = 1;
		pthread_create(&ftp_tid, NULL, netcam_ftp_upload_thread, NULL);
		if(0 == ftp_tid)
		{
			printf("ftp create thread failed!");
			return -1;
		}
		pthread_detach(ftp_tid);
		return 0;
	}

	return -1;
}

int netcam_ftp_stop()
{
	ftp_run_flag = 0;
	return 0;
}

int netcam_ftp_is_ok()
{
	int ret = 0;
	if( netcam_ftp_upload_get_status() == 0 &&
		runNetworkCfg.ftp.enableFTP != 0 &&
		strlen(runNetworkCfg.ftp.address) != 0 &&
		runNetworkCfg.ftp.port != 0 &&
		strlen(runNetworkCfg.ftp.userName)!=0 )
	{
		ret = 1;
	}
	
	return ret;

}
