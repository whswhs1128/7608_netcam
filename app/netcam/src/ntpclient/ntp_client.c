#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <resolv.h>
#include <pthread.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "cfg_system.h"
#include "cfg_record.h"

#include "ntp_client.h"
#include "sdk_sys.h"

#include "utility/util_sock.h"

//#include "linux_network.h"

enum
{
	NTP_OFF = 0,             //ntp客户端关闭
	NTP_AUTO_SYNC            //ntp客户端开启
};


/**
	macro for ntp packet.
*/
#define  LI           0                //head
#define  VN           3                //version
#define  MODE         3                //mode:this is client
#define  STRATUM      0
#define  POLL         4
#define  PREC        -6
#define  JAN_1970     0x83aa7e80        //3600s*24h*(365days*70years+17days)
#define  NTPFRAC(x)  (4294 * (x) + ((1981 * (x)) >> 11))
#define  USEC(x)     (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))

/**
	macro for common define.
*/
#define  DEF_NTP_PORT            123
#define  DEF_TIMEOUT             10
#define  DEF_SAVETIME            60
#define  SYNC_TIME               48 * 60 * 60
#define  MAX_SEND_COUNT          3
#define  NTP_PARA	             "/opt/custom/ntp_param"

/**
	macro for printf debug.
*/

#define PRI_DBG     1
#define ERR 	    2
#define DBG_MASK	(PRI_DBG|ERR)

//#define NTP_DEBUG
#ifdef NTP_DEBUG
#define PRI         PRI_DBG
#else
#define PRI         0
#endif

//打印日志的宏
#define PRINT_PREFIX(dest, prefix)	\
do{	\
	time_t iTime;	\
	struct tm *stTime;	\
	iTime = time(&iTime);	\
	stTime = localtime(&iTime);	\
	fprintf(dest, "[%s][%04d-%02d-%02d %02d:%02d:%02d][%s][%d] ", prefix, \
	stTime->tm_year+1900, stTime->tm_mon+1, stTime->tm_mday, stTime->tm_hour,\
	stTime->tm_min, stTime->tm_sec, __func__, __LINE__);	\
} while(0)


#define DBG(flag, fmt, args...)	\
do{	\
	switch (flag&DBG_MASK) \
	{	\
		case ERR: \
			PRINT_PREFIX(stdout, "DBG_ERR");		\
			printf(fmt, ##args);			\
			break;\
		case PRI_DBG: \
			PRINT_PREFIX(stdout, "DBG_PRI");		\
			printf(fmt, ##args); \
			break;\
	default: \
	break;	\
	}	\
} while(0)

typedef struct __NtpTime
{
	unsigned	int  integer;
	unsigned	int  fraction;

} NtpTime;

typedef struct __NtpServResp
{
	struct  timeval dlytime;
	struct  timeval offtime;
	struct  timeval newtime;
} NtpServResp;

//ntp client的结构
typedef struct __st_ntp
{
	volatile int flag;
	int zone;
	time_t sync_time;
	struct timeval time;
	char server[64];
	int is_overlap;
	int port;
	int index;
}STNTP;

//ntp server列表
const char* ntp_server[] =
{
    "cn.ntp.org.cn",
    "edu.ntp.org.cn",
	"time.windows.com",        
	"s2c.time.edu.cn",
	"s2d.time.edu.cn",
	"s2e.time.edu.cn",
	"s2f.time.edu.cn",
	"s2g.time.edu.cn",
	"s2h.time.edu.cn",
	"s2j.time.edu.cn",
	"s2k.time.edu.cn",
	"s2m.time.edu.cn",	
	"ntp.sjtu.edu.cn",
	"s1a.time.edu.cn",
	"s1b.time.edu.cn",
	"s1c.time.edu.cn",
	"s1d.time.edu.cn",
	"s1e.time.edu.cn",
	"s2a.time.edu.cn",
	"s2b.time.edu.cn",
	"pool.ntp.org",
	"time-a.nist.gov",
	"time-b.nist.gov",
	"time-a.timefreq.bldrdoc.gov",
	"time-b.timefreq.bldrdoc.gov",
	"time-c.timefreq.bldrdoc.gov",
	"utcnist.colorado.edu",
	"time.nist.gov",
	"time-nw.nist.gov",
	"nist1.symmetricom.com",
	"nist1-dc.glassey.com",
	"nist1-ny.glassey.com",
	"nist1-sj.glassey.com",
	"nist1.aol-ca.truetime.com",
	"nist1.aol-va.truetime.com"
};

static STNTP ntp_data;
int ntp_done = 0;
//发送数据包给ntp server
static int send_packet(int fd)
{
	int ret;
	unsigned int data[12];
	struct timeval now;

	if(sizeof(data) != 48)
	{
		DBG(ERR, "data size error!\n");
		return -1;
	}

	memset((char *)data, 0, sizeof (data));
	data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24)
		| (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
	data[1] = htonl(1 << 16);
	data[2] = htonl(1 << 16);
	//get local time value
	gettimeofday(&now, NULL);
	data[10] = htonl(now.tv_sec + JAN_1970);
	data[11] = htonl(NTPFRAC(now.tv_usec));

	ret = send(fd, data, sizeof(data), 0);
	DBG(PRI,"send packet to ntp server. ret[%d]\n", ret);
	return ret;
}

//获取ntpserver返回的数据并解析时间
static int get_server_time(int sock, NtpServResp *resp)
{
	unsigned int data[12];
	long long orius, recus, traus, desus, offus, dlyus;
	socklen_t sockAddrLen;
	NtpTime oritime, rectime, tratime, destime;
	struct timeval now;
	struct sockaddr_in addr;

	sockAddrLen = sizeof(struct sockaddr_in);
	memset(data, 0, sizeof(data));

	if (recvfrom(sock, data, sizeof(data), 0, (struct sockaddr *)&addr, &sockAddrLen) <= 0)
	{
		DBG(ERR, "ntp recvfrom time server was failed! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}

	gettimeofday(&now, NULL);
	destime.integer  = now.tv_sec + JAN_1970;
	destime.fraction = NTPFRAC(now.tv_usec);

#define  DATA(i) ntohl(((unsigned int*)data)[i])
	oritime.integer  = DATA(6);
	oritime.fraction = DATA(7);
	rectime.integer  = DATA(8);
	rectime.fraction = DATA(9);
	tratime.integer  = DATA(10);
	tratime.fraction = DATA(11);
#undef  DATA

#define  MKSEC(ntpt)   ((ntpt).integer - JAN_1970)
#define  MKUSEC(ntpt)  (USEC((ntpt).fraction))
#define  TTLUSEC(sec,usec)   (( long long )(sec)*1000000 + (usec))
#define  GETSEC(us)    ((us)/1000000)
#define  GETUSEC(us)   ((us)%1000000)

	orius = TTLUSEC(MKSEC(oritime), MKUSEC(oritime));
	recus = TTLUSEC(MKSEC(rectime), MKUSEC(rectime));
	traus = TTLUSEC(MKSEC(tratime), MKUSEC(tratime));
	desus = TTLUSEC(now.tv_sec, now.tv_usec);
	offus = ((recus - orius) + (traus - desus))/2;
	dlyus = (recus - orius) + (desus - traus);
	resp->offtime.tv_sec  = GETSEC(offus);
	resp->offtime.tv_usec = GETUSEC(offus);
	resp->dlytime.tv_sec  = GETSEC(dlyus);
	resp->dlytime.tv_usec = GETUSEC(dlyus);

	resp->newtime.tv_sec = destime.integer - JAN_1970 +
		resp->offtime.tv_sec;
	resp->newtime.tv_usec = 0;

	return 0;
}

//修改系统时间
static void mod_localtime(struct timeval newtime)
{
	int ret;
	ret = settimeofday(&newtime, NULL);
	if(ret == -1)
	{
		DBG(ERR,"ntp set time fail! errno[%d] errinfo[%s]\n", errno, strerror(errno));
		return;
	}

	DBG(PRI_DBG,"ntp set time success!\n");
	return;
}

static int is_ipaddress(const char* s)
{
	int i;
	int len = strlen(s);

	for (i = 0; i < len; i++)
	{
		if (s[i] >= '0' && s[i] <= '9')
			continue;
		if (s[i] == '.')
			continue;

		return 0;
	}

	return 1;
}

//连接ntp server
static int connect_ntp_server(const char *servname, int port)
{
	int sock = -1;
	int addr_len = sizeof(struct sockaddr_in);
	static struct sockaddr_in addr_dst;
	static char host_old[32] = {0};
	
	DBG(PRI, "connect to ntp server %s : %d\n", servname, port);

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		DBG(ERR,"create sock error! errno[%d] errinfo[%s]\n", errno, strerror(errno));
		return -1;
	}

	//memset(&addr_dst, 0, addr_len);
	addr_dst.sin_family = AF_INET;
	addr_dst.sin_port = htons(port);
	if (!is_ipaddress(servname))
	{
		//res_init();
		if((addr_dst.sin_addr.s_addr <= 0)||(strncmp(host_old, servname, sizeof(host_old))))
		{
			struct in_addr server;
			int ret;
			ret = utility_net_resolve_host(&server,servname);  
			if (ret)
			{
				DBG(PRI,"gethostbyname error! errno[%d] errinfo[%s]\n", 
					errno, strerror(errno)); 
				close(sock);
				return -1;
			}
			memcpy(&(addr_dst.sin_addr), &server, sizeof(server));
			strncpy(host_old, servname, sizeof(host_old));
		}
	}
	else
		addr_dst.sin_addr.s_addr = inet_addr(servname);
		
	if (connect(sock, ( struct  sockaddr *) &addr_dst, addr_len)) 
	{ 
		DBG(ERR,"connect server error! errno[%d] errinfo[%s]\n", errno, strerror(errno));
		close(sock);
		return -1;
	}

	DBG(PRI_DBG, "connect to ntp server %s(%s:%d) success\n" ,
		servname, inet_ntoa(addr_dst.sin_addr), port);
	return sock;
}


static int conn_default_ntp_server(STNTP *ntp)
{
	int ret = -1;

	if (ntp->index >= sizeof(ntp_server) / sizeof(char*))
		ntp->index = 0;

	ret = connect_ntp_server(ntp_server[ntp->index], DEF_NTP_PORT);
	if (ret > 0)
		strncpy(ntp->server, ntp_server[ntp->index], sizeof(ntp->server));

	//sleep(5);
	return ret;
}

static int ntp_conn_server(STNTP *ntp)
{
	int sock = -1;
	//if user set ntp server
	if (ntp->is_overlap)
		sock = connect_ntp_server(ntp->server, ntp->port);
	else
	{
		//使用默认的ntp server
		sock = conn_default_ntp_server(ntp);
		ntp->index++;
	}
	
	return sock;
}

static int init_ntp_param(STNTP* ntp)
{
	//struct timeval tv;
	//struct timezone tz;

	//if(0 != access(NTP_PARA, 0))
	//{
//		memset(ntp, 0, sizeof(ntp_param));
		ntp->flag= NTP_AUTO_SYNC;
		//ntp->zone= 0;
		#ifdef MODULE_SUPPORT_ZK_WAPI
		ntp->sync_time = runSystemCfg.ntpCfg.sync_time; // SYNC_TIME;
		ntp->is_overlap = 1;
		#else
		ntp->sync_time = SYNC_TIME;
		#endif
		//gettimeofday(&tv, &tz);
		//ntp->time = tv;
		//ntp->index = 0;
		ntp->port = DEF_NTP_PORT;
		//ntp->is_overlap = 0;
		strncpy(ntp->server, ntp_server[0], strlen(ntp_server[0]));
		
		//save_param(ntp);
		DBG(PRI,"ntp set default param\n");
	//}
	/*
	else
	{
		read_param(ntp);
		DBG(PRI,"ntp param: flag[%d],zone[%d],sync_time[%d],"
			"time[%u],index[%d],server[%s],port[%d],overlap[%d]\n", 
			ntp->flag, (int)ntp->zone, (int)ntp->sync_time, 
			(unsigned int)ntp->time.tv_sec, ntp->index, ntp->server, 
			ntp->port, ntp->is_overlap);
		mod_localtime(ntp->time);
	}
	*/
	return 0;
}

static void* ntp_gettime_thread(void* arg)
{
    sdk_sys_thread_set_name("ntp_gettime_thread");
	int ret;
	int ntp_sock= -1;
	int send_count = 0;
	time_t curr = 0, sub_time;

	fd_set fds_read;
	NtpServResp response;
	//struct timezone tz;
	struct timeval timeout;

	DBG(PRI_DBG, "ntp client thread start!\n");
	init_ntp_param(&ntp_data);

	//sleep(1);
	while (1)
	{
		sub_time = time(NULL) - curr;
		//read_param(&ntp_data);

		DBG(PRI, "ntp param: flag[%d],zone[%d],sync_time[%d],"
			"time[%u],index[%d],server[%s],port[%d],overlap[%d],sub_time[%d]\n", 
			ntp_data.flag, (int)ntp_data.zone, (int)ntp_data.sync_time, 
			(unsigned int)ntp_data.time.tv_sec, ntp_data.index, ntp_data.server, 
			ntp_data.port, ntp_data.is_overlap, (int)sub_time);

		if (ntp_data.flag == NTP_OFF)           //ntp off
		{
			//save time
			/*if (sub_time > DEF_SAVETIME)
			{
				gettimeofday(&ntp_data.time, &tz);
				save_param(&ntp_data);
				curr = time(NULL);
			}*/
            DBG(PRI_DBG, "ntp off\n");
			sleep(DEF_TIMEOUT);
			continue;
		}

		//ntp on
		if (curr && sub_time < ntp_data.sync_time)
		{
			sleep(DEF_TIMEOUT);
            DBG(PRI_DBG, "ntp is in sync interval time\n");
			/*if (sub_time > DEF_SAVETIME)
			{
				gettimeofday(&ntp_data.time, &tz);
				save_param(&ntp_data);
			}
			*/
			continue;
		}

		//connect ntp server
		if (ntp_sock < 0 && (ntp_sock = ntp_conn_server(&ntp_data)) < 0)
		{
			//ntp_data.index++;
			//save_param(&ntp_data);
            //DBG(ERR, "ntp_conn_server fialed\n");
			sleep(DEF_TIMEOUT);
			continue;
		}

		//send ntp packet
		send_packet(ntp_sock);

		FD_ZERO(&fds_read);
		FD_SET(ntp_sock, &fds_read);
		timeout.tv_sec = DEF_TIMEOUT;
		timeout.tv_usec = 0;

		ret = select(ntp_sock + 1, &fds_read, NULL, NULL, &timeout);
		if (ret == -1)
		{
			DBG(ERR, "ntp select error! errno[%d] errinfo[%s]\n", errno, strerror(errno));
			close(ntp_sock);
			ntp_sock = -1;
			//ntp_data.index++;
			//save_param(&ntp_data);
			continue;
		}

		//发送三次没有回应，则选择下一个ntpserver
		if  (ret == 0 || !FD_ISSET (ntp_sock, &fds_read))
		{
			if (send_count < MAX_SEND_COUNT)
			{
				send_count++;
			}

			if (send_count >= MAX_SEND_COUNT)
			{
				close(ntp_sock);
				ntp_sock = -1;
				//ntp_data.index++;
				send_count = 0;
			}
            DBG(ERR, "ntp read date failed\n");
			//save_param(&ntp_data);
			continue;
		}

		send_count = 0;
		if(get_server_time(ntp_sock, &response))
		{
			close(ntp_sock);
			ntp_sock = -1;
			//ntp_data.index++;
			//save_param(&ntp_data);
            DBG(ERR, "ntp get_server_time failed\n");
			sleep(DEF_TIMEOUT);
			continue;
		}
        #if 0
        //保障第一次设置时间是正确的,step1
        if(response.newtime.tv_sec < 10*365*24*60*60)
        {
            printf("get_server_time :%ld is  invalid\n",response.newtime.tv_sec);
            continue;
        }
        #endif
        
        DBG(PRI_DBG, "ntp set time,get tv_sec; %ld\n", response.newtime.tv_sec);
		//修改系统时间
		mod_localtime(response.newtime);

        
        //保障第一次设置时间是正确的,step1
        struct timeval tv;   
        gettimeofday (&tv, NULL);
        DBG(PRI_DBG, "after ntp set time,get tv_sec; %ld\n", tv.tv_sec);
        
#if 1   //开机时间
        struct timespec times = {0, 0};
        
        struct tm timeNow;
        char logTime[64];
        long startTime = 0;
        clock_gettime(CLOCK_MONOTONIC, &times);
                
        startTime = tv.tv_sec - times.tv_sec;
        memset(logTime, 0, sizeof(logTime));
        localtime_r(&startTime, &timeNow);
        sprintf(logTime, "%04d-%02d-%02d %02d:%02d:%02d.%03d",\
            timeNow.tm_year + 1900, timeNow.tm_mon + 1, \
            timeNow.tm_mday,timeNow.tm_hour, timeNow.tm_min,\
            timeNow.tm_sec, tv.tv_usec/1000);
        DBG(PRI_DBG, "uptime=%d,logTime:%s\n", times.tv_sec,logTime);
        //ite_eXosip_startup_log_save(logTime);		
#endif
        #if 0
        if(tv.tv_sec < 10*365*24*60*60)
        {
            sleep(DEF_TIMEOUT);
            printf("get system time:%ld is err \n",tv.tv_sec);
            continue ;
        }
        #endif
        
		ntp_data.time = response.newtime;
		//save_param(&ntp_data);

		//设置ntpserver
		#ifndef MODULE_SUPPORT_ZK_WAPI
		memset(runSystemCfg.ntpCfg.serverDomain, 0, MAX_URL_STR_SIZE-1);
		strncpy(runSystemCfg.ntpCfg.serverDomain, ntp_data.server, MAX_URL_STR_SIZE-1);
		SystemCfgSave();
		#endif
        ntp_done = 1;
		//关闭socket
		close(ntp_sock);
		ntp_sock = -1;
        //NTP做一次，每天0点重启开机都会运行一次
		#ifndef MODULE_SUPPORT_ZK_WAPI
        DBG(PRI_DBG, "exit ntp thread\n");
        break;
		#endif
        
		curr = time(NULL);
		sleep(DEF_TIMEOUT);
	}

	return 0;
}

int start_ntp_client()
{
	pthread_t ntp_p;

	if (pthread_create(&ntp_p, NULL, ntp_gettime_thread, NULL))
	{
		DBG(ERR, "create ntp client thread error! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}

	return 0;
}

void ntpc_enable()
{
	ntp_data.flag = NTP_AUTO_SYNC;
	runSystemCfg.ntpCfg.enable = 1;
	SystemCfgSave();
}

void ntpc_disable()
{
	ntp_data.flag = NTP_OFF;
	runSystemCfg.ntpCfg.enable = 0;
	SystemCfgSave();
}

void ntpc_set_addr(char *ntpserver, int serLen, int port)
{
	if (!ntpserver || !serLen)
		return;

	if (port > 0)
		ntp_data.port = port;
	
	memset(ntp_data.server, 0, sizeof(ntp_data.server));
	strncpy(ntp_data.server, ntpserver, sizeof(ntp_data.server));
	memset(runSystemCfg.ntpCfg.serverDomain, 0, MAX_URL_STR_SIZE-1);
	strncpy(runSystemCfg.ntpCfg.serverDomain, ntpserver, MAX_URL_STR_SIZE-1);
	ntp_data.is_overlap = 1;

	SystemCfgSave();
}

void ntpc_set_synctime(int synctime)
{
	ntp_data.sync_time = synctime;
	runSystemCfg.ntpCfg.sync_time = synctime;

	SystemCfgSave();
}


