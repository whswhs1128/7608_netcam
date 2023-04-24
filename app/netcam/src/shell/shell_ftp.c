#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "shell.h"
#include "ftplib.h"
#include "cfg_all.h"

static const char *shortOptions = "hi:p:u:w:f:d:s:";

static struct option longOptions[] =
{
    {"help",       0, NULL, 'h'},
    {"ip",         1, NULL, 'i'},
    {"port",       1, NULL, 'p'},
    {"usr",        1, NULL, 'u'},
    {"pwd",        1, NULL, 'w'},
    {"file",       1, NULL, 'f'},
    {"dir",        1, NULL, 'd'},
    {"s",          1, NULL, 's'},
    {0,            0, NULL, 0}
};
extern int ite_eXosip_log_save(unsigned int logType, char *logContent);
static int shell_ftp_upload(char *ipaddr, int port, char *username, char * passdword, char *filename,char *uploadDir);
static int handle_ftp_command(int argc, char* argv[]);

static void Usage(void)
{
    printf("Usage: ftp [option]\n");
    printf("    -h help.\n");
    printf("    -i ipaddr\n");
    printf("    -p port\n");
    printf("    -u usrname\n");
    printf("    -w pwd\n");
    printf("    -f filename\n");
    printf("    -d dir\n");
    printf("    -s save\n");
}

int ftp_register_testcase(void)
{
    int   retVal =  0;
    (void)shell_registercommand (
        "ftp",
        handle_ftp_command,
        "ftp command",
        "---------------------------------------------------------------------\n"
        "ftp -i \n"
        "   brief : ftp ipaddr\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ftp -p \n"
        "   brief : ftp port\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ftp -u \n"
        "   brief : ftp usrname\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ftp -w \n"
        "   brief : ftp pwd\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ftp -f \n"
        "   brief : ftp filename\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ftp -d \n"
        "   brief : ftp dir\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ftp -s \n"
        "   brief : save type\n"
        "\n"
        /******************************************************************/
    );

    return retVal;
}

static int handle_ftp_command(int argc, char* argv[])
{
	int ret = 0;
    int option_index;//ch;
    char ch;
    char ipaddr[32];
    int port = 0;
    char username[32];
    char passdword[32];
    char filename[128];
    char uploadDir[128];
    int logType = 0;
    int type = 0;
    pthread_t pthid;
    memset(ipaddr, 0, 32);
    memset(username, 0, 32);
    memset(passdword, 0, 32);
    memset(filename, 0, 128);
    memset(uploadDir, 0, 128);
    /*change parameters when giving input options.*/
    optind = 1;
    while (1)
    {
        option_index = 0;
        ch = getopt_long(argc, argv, shortOptions, longOptions, &option_index);
		
        if (ch == 255)
    	{
            break;
    	}
        switch (ch)
        {
            case 'h':
            case '?':
                Usage();
                break;
            case 'i':
                strncpy(ipaddr, optarg, sizeof(ipaddr));
                printf("ipaddr %s\n", ipaddr);
                break;
            case 'p':
                port = atoi(optarg);
                printf("port %d\n", port);
                break;
                
            case 'u':
                strncpy(username, optarg, sizeof(username));
                printf("username %s\n", username);
                break;
            case 'w':
                strncpy(passdword, optarg, sizeof(passdword));
                printf("passdword %s\n", passdword);
                break;
            case 'f':
                strncpy(filename, optarg, sizeof(filename));
                printf("filename %s\n", filename);
                break;
                
            case 'd':
                strncpy(uploadDir, optarg, sizeof(uploadDir));
                printf("uploadDir %s\n", uploadDir);
                break;
            case 's':
                printf("=====logType %d\n", logType);
                logType = atoi(optarg);
                printf("logType %d\n", logType);
                type = 1;
                break;

            default:
                printf("ftp: bad params\n");
                break;
        }
    }
    if(type == 1)
        ite_eXosip_log_save(logType, NULL);
    else
        shell_ftp_upload(ipaddr,port,username,passdword,filename,uploadDir);
exit:    
    optind = 1;
    type = 0;
    return 0;
}
#define FILE_ALARM  "alarm"
#define FILE_LOG    "/opt/custom/cfg/test.log"
static int shell_ftp_upload(char *ipaddr, int port, char *username, char * passdword, char *filename,char *uploadDir)
{
	netbuf *conn = NULL;
	int ret=0;
	char outfile[64]={0};
	char ipaddress[64]={0};

	struct tm tt;
	sys_get_tm(&tt);
	snprintf(outfile, sizeof(outfile), "log_%s.log", FILE_ALARM);

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
	printf("ftp init \r\n");

	ret = FtpConnect(ipaddress,&conn);//FTP 连接
	if(ret)
	{
		printf("connect is susseccful \r\n");
	}
	else
	{
		printf("connect is failed \r\nftp info :address:%s userName:%s password:%s local:%s upload:%s\n",ipaddress, username, passdword, filename, outfile);
		return -1;
	}

	ret = FtpLogin(username,passdword,conn);//FTP登陆
	if(ret)
	{
		printf("login is ok \r\n");
	}
	else
	{
		printf("login is error \r\n");
		FtpClose(conn);
		return -2;
	}
	FtpMkdir(uploadDir, conn);
	FtpChdir(uploadDir, conn);//下载路径

	FtpOptions(FTPLIB_CONNMODE,FTPLIB_PASSIVE,conn);//设置FTP
    
	sprintf(outfile, "test.log");
    //读符合时间的数据到文件
    //上传
    //删除行
	ret = FtpPut(filename,outfile,FTPLIB_ASCII,conn);//FTP 上传文件
	if(ret)
	{
		printf("ftp upload \"%s\" is susseccful\r\n", outfile);
	}
	else
	{
		printf("ftp update  failed\r\n");
	}

	FtpClose(conn);
	return 0;

}



