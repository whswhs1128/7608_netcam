

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <libgen.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include "fcntl.h"
#include "netcam_api.h"

#include <sys/mount.h>
#include "readipcsearch.h"
#include "mtd-abi.h"
#include "mtd-user.h"

#define RECV_BUF_SIZE       (15 * 1024)
#define SEND_BUF_SIZE       (4 * 1024)
#define CMD_SIZE             12

#define START_CODE          0x4844
#define CMD_GET_PC			0x0201
#define CMD_GET_RET         0x0801
#define CMD_SET_RET         0x0802
#define CMD_GET             0x0101
#define CMD_SET             0x0102
#define CMD_SET_TCP 	  	0x0102
#define CMD_UPGRADE 		0x0103
#define CMD_STATUS          0x0104
#define CMD_SET_P2PINFO     0x0105
#define CMD_SET_RESET       0x0106
#define CMD_SET_P2PINFO_pc 	0x0107
#define CMD_SET_REBOOT   	0x0108
#define CMD_PHONE_SEARCH    0x0109
#define CMD_SET_TELNET  	0x0202
#define CMD_UPGRADE_FAST  	0x0204
#define CMD_UPGRADE_BIN  	0x0205
#define CMD_UPGRADE_GET_NAME  	0x0206

#define CMD_MOJING_SEARCH           0x0301
#define CMD_MOJING_SEARCH_RET       0x0302
#define CMD_MOJING_SEARCH_RESET     0x0303
#define CMD_MOJING_GET_LIST_GET     0x0304
#define CMD_MOJING_GET_LIST_RET     0x0305

#define CMD_GET_DS_INFO     0x0113
#define CMD_SET_DS_INFO     0x0114
#define CMD_BATCH_SET_DS_INFO     0x0115//批量设置参数(设备id除外)，用广播方式，
#define CMD_GET_FAC_CFG     0x0116




#define SEARCH_BROADCAST_ADDR   "255.255.255.255"
#define SEARCH_BROADCAST_SEND_PORT   0x8888//20909
#define SEARCH_BROADCAST_RECV_PORT   0x9888//9095
#define SEARCH_TOOL_BROADCAST_RECV_PORT   9095
#define UPGRADE_PORT         16889
#define GK_CMS_TCP_LISTEN_PORT 0x9123
#define	SERVER_PACK_FLAG 0x03404324
#define GK_CMS_BUFLEN 4096
#define DMS_MAX_IP_LENGTH 		16
#define IPTOSBUFFERS    12

#define LAN_DEV     "eth0"
#define DANA_FILE "/tmp/dana/danale.conf"
#define ULU_CONF_DEVICEID_PATH "/opt/custom/ulucu"
#define UPGRADE_FAST_PATH "/tmp/up.bin"



static int g_search_thread_run    = 0;
int upStatus = 0;
int upTotal = 0;


pthread_mutex_t up_send_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Upgrade_FILE_HEAD
{
	int cmd;
	unsigned long nBufSize;		//??????????
}GK_UPGRADE_FILE_HEAD,*UPGRADE_FILE_HEAD;


typedef struct tagSEARCH_CMD
{
    unsigned long dwFlag; //0x4844
    unsigned long dwCmd;

}SEARCH_CMD;

typedef struct tagSEARCH_CMD_RET
{
    unsigned long dwFlag; //0x4844
    unsigned long dwCmd;
    unsigned long dwCmdFrom;
    unsigned long status;
}SEARCH_CMD_RET;

typedef enum {
	GK_MSG_UPGRADE =                    0x40000000,  //??????
	GK_MSG_CRCERROR =                   0x40000001,  //??CRC?
	GK_MSG_UPGRADEOK =          		0x40000002,  //????
} gkMsg;
typedef enum
{
    WF_READ,
    WF_WRITE
}WAIT_EVENT;

struct send_msg {
		char upgrade_state;
};


static int Upgrade_NotifyUpgrade(int sock, int write_size, int type)
{
	struct send_msg send_data;
	send_data.upgrade_state = write_size&0xff;

	int ret = send(sock,&send_data, sizeof(struct send_msg), 0);//Upgrade_SockSend(sock, &send_data, sizeof(struct send_msg), 100);
	if (ret != (sizeof(struct send_msg))) {
	    PRINT_ERR("send:%d size:%d", ret, sizeof(struct send_msg));
	    return -1;
	} else {
	    //PRINT_INFO("send %d data_size:%d, head_size:%d\n", ret, data_size, sizeof(JB_NET_PACK_HEAD) + sizeof(JB_SERVER_MSG));
	}

	return 0;
}
static int Upgrade_NotifyUpgradeDoing(int sock, int write_size)
{
	return Upgrade_NotifyUpgrade(sock, write_size, GK_MSG_UPGRADE);
}

static int Upgrade_NotifyUpgradeDone(int sock, int write_size)
{
	return Upgrade_NotifyUpgrade(sock, write_size, GK_MSG_UPGRADEOK);
}
static int UpgradeSetSockAttr(int fd)
{

	int opt = 1;
	int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret < 0) {
		PRINT_ERR("setsockopt SO_REUSEADDR failed, error:%s\n", strerror(errno));
		return -1;
	}

	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		PRINT_ERR("Fail to get old flags [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		PRINT_ERR("Fail to set flags to O_NONBLOCK [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}


	// set nodelay
	int on = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

	return 0;
}
static int Get_Upgrade_Status(int sock)
{
	int ret;
	int cnt = upStatus * 100 / upTotal;
    //printf("---->Get_Upgrade_Status:%d, %d,%d\n", cnt, upStatus, upTotal);
	if (cnt < 99)
	{
		ret = Upgrade_NotifyUpgradeDoing(sock, cnt);
		usleep(218 * 1000);
		PRINT_INFO("sock:%d cnt:%d  ret:%d \n", sock, cnt, ret);
	}
	else
	{
		ret = Upgrade_NotifyUpgradeDone(sock, 100);

		PRINT_INFO("Gk_CmsNotifyUpgradeDone! sock:%d  ret:%d\n", sock, ret);
		return 1;
	}

	return 0;
}
static int time_wait_search(int sock, WAIT_EVENT e, int sec, int usec, int times)
{
	int ret = 0;
	fd_set fset;
	struct timeval tv;
	while(times > 0)
	{
		FD_ZERO(&fset);
		FD_SET(sock,&fset);

		tv.tv_sec = sec;
		tv.tv_usec = usec;

		if(e ==WF_READ)
			ret = select(sock+1, &fset, NULL, NULL, &tv);
		else
			ret = select(sock+1, NULL, &fset, NULL, &tv);
		if(ret > 0)
			goto finish;
		else if(ret < 0){
			if(errno != EINTR && errno != EAGAIN)
				goto error;
			}
		times--;
	}
error:
	PRINT_ERR("select %s [time_wait] in IPCSearch!fd[%d] times[%d] errno[%d] errinfo[%s]\n",errno == 0?"timeout":"error",sock,times,errno,strerror(errno));
finish:
	return ret;
}
static int recv_data(int sock,char* buffer,int len)
{
	int n , ret;
	char* pbuf =buffer;
	int offset = 0;
	while(len > 0)
	{
		n = recv(sock,pbuf,len,0);
		if(n < 0)
		{
			if(errno == EINTR || errno == EAGAIN||errno == EWOULDBLOCK)
			{
				ret = time_wait_search(sock, WF_READ, 0, 200000, 25);
				if(ret > 0)
					continue;
				else if (ret == 0)
					break;
				else
					goto error;
			}
			else
			{
				ret = 1;
				goto error;
			}
		}
		if (n == 0)
			break;
		len -= n;
		offset += n;
		pbuf += n;
	}
	return offset;
error:
	PRINT_ERR("Failed to recv data[recv_data]! n[%d] fd[%d] ret[%d] errno[%d] errinfo[%s]\n",n, sock, ret, errno, strerror(errno));
	return -1;
}


#ifndef _GET_ERRNO
#define _GET_ERRNO() errno
#endif
static int wr_timedwait(int sock, int read_opt, int sec, int usec, int times)
{
	int ret = 0;
	fd_set fset;
	struct timeval tv;

	while (times > 0) {
		FD_ZERO(&fset);
		FD_SET(sock, &fset);
		tv.tv_sec = sec;
		tv.tv_usec = usec;

		if (read_opt) {
			ret = select(sock + 1, &fset, NULL, NULL, &tv);
		} else {
			ret = select(sock + 1, NULL, &fset, NULL, &tv);
		}
 		if ((ret > 0) || ((ret < 0) &&
 		        (_GET_ERRNO() != EINTR) && (_GET_ERRNO() !=EAGAIN))) {
            break;
		}
		times--;
        printf("wr_timedwait: %s sock[%d] select timeout, times[%d], ret = %d, errno = %d\n",read_opt?"read":"write", sock, times,ret, errno);
	}
	return ret;
}

static int sock_wrn_bin(int sock, int size, int timeout_sec, int
read_opt)
{
    int i = 0;
    int ret = 0;
	int len = 0;
    int left = 0;
    int oneRcvSize = 0;
    int oneLeftSize = 0;
    int oneTmpSize = 0;
    int oneMaxSize = 512 * 1024;
    int tmpSize = 512 * 1024;
    int  dev_fd;
    int tmpEraseSize = 0;
    int blockTotal = 0;
    int curBlock = 0;
    char devMtdName[32] = {0};
    unsigned char *nullData = NULL;
    erase_info_t erase;
    int flashRet = 0;
    struct mtd_info_user mtd_info;

    char *buffer = (char*)malloc(oneMaxSize);    
    if (buffer == NULL)
    {
        printf("sock_wrn_fast malloc error:%d\n", tmpSize);
        return 0;
    }

    readfifo_sys_shell_result("ls -l /dev/mtd* | sort -nr | head -n 1 | awk '{print $10}'", devMtdName, 32);
    if (strlen(devMtdName) - strlen(strstr(devMtdName, "mtd")) > 1)
    {
        devMtdName[strlen(strstr(devMtdName, "mtd")) + 3] = 0;
    }
    printf("mtd:%s\n", devMtdName);
    //strcpy(devMtdName, "/dev/mtd2");
    if ((dev_fd = open(devMtdName, O_SYNC | O_RDWR)) < 0)
    {
        printf("open %s error! %s\n", devMtdName, strerror(errno));
        return -1;
    }

    #if 1
    if ((ret = ioctl(dev_fd, MEMGETINFO, &mtd_info)) != 0)
    {
        printf("%s:  unable to get MTD device info\n", devMtdName);
        close(dev_fd);
        return ret;
    }
    #else
    mtd_info.erasesize = 64*1024;
    #endif
    printf("mtd_info.erasesize:%d\n", mtd_info.erasesize);

    erase.length = mtd_info.erasesize;
    blockTotal = (size + mtd_info.erasesize - 1) / mtd_info.erasesize;
    nullData = (unsigned char *)malloc(mtd_info.erasesize);
    if (nullData == NULL)
    {
        printf("malloc %d error\n", mtd_info.erasesize);
        return -1;
    }
    memset(nullData, 0xff, mtd_info.erasesize);
    
    /* read data until size is reached, or tired of timeout */
    left = size;
    oneLeftSize = oneMaxSize;
    erase.start = 0;
    while (len < size) {
        tmpSize = oneLeftSize - oneRcvSize;
        if (read_opt) {
            ret = recv(sock, buffer + oneRcvSize, tmpSize, 0);
        } else {
            ret = send(sock, buffer + oneRcvSize, tmpSize, 0);
        }
		if (ret <= 0) {
		    if ((_GET_ERRNO() == EINTR) || (_GET_ERRNO() == EAGAIN)) {
    			ret = wr_timedwait(sock, read_opt, 1, 0, timeout_sec);
    			if (ret > 0) {
    				continue;//read again
    			} else {
                    goto ERROR;
    			}
			} else {
                goto ERROR;
            }
		}
        oneRcvSize += ret;
        left -= ret;
        len += ret;
        //printf("oneRcvSize:%d, max:%d, tmpSize:%d, ret:%d, len:%d\n", oneRcvSize, oneMaxSize, tmpSize, ret, len);
        if (oneRcvSize >= oneMaxSize || left == 0)
        {
            tmpEraseSize = oneRcvSize / mtd_info.erasesize;
            printf("cur tmpEraseSize:%d, %d, %d\n", tmpEraseSize, oneRcvSize, mtd_info.erasesize);
            for (i = 0; i < tmpEraseSize; i++)
            {
                printf("\rErasing blocks: %2d/%2d, addr:0x%x\n", curBlock, blockTotal, curBlock * mtd_info.erasesize);
#if 1
                if (memcmp(nullData, buffer + i * mtd_info.erasesize, mtd_info.erasesize) != 0)
                {
                    if (ioctl (dev_fd, MEMERASE, &erase) < 0)
                    {
                        printf("\n");
                        printf("While erasing blocks 0x%.8x-0x%.8x on %s:\n",
                                                                (unsigned int)erase.start,
                                                                (unsigned int)(erase.start + erase.length),
                                                                devMtdName);
                        close(dev_fd);
                        return -1;
                    }

                    flashRet = write(dev_fd, buffer + i * mtd_info.erasesize, mtd_info.erasesize);
                    if (flashRet < 0 || flashRet != mtd_info.erasesize)
                    {
                        printf("wirte flash error! dev_fd:%d, pos:0x%x, count:%d  %s, ret:%d\n",dev_fd, curBlock * mtd_info.erasesize, mtd_info.erasesize, strerror(errno), flashRet);
                        close(dev_fd);
                        return -1;
                    }


                }
                else
                {
                    if (ioctl (dev_fd, MEMERASE, &erase) < 0)
                    {
                        printf("\n");
                        printf("While erasing blocks 0x%.8x-0x%.8x on %s:\n",
                                                                (unsigned int)erase.start,
                                                                (unsigned int)(erase.start + erase.length),
                                                                devMtdName);
                        close(dev_fd);
                        return -1;
                    }
                    
                    if (lseek(dev_fd, (curBlock + 1) * mtd_info.erasesize, SEEK_SET) < 0)
                    {
                        printf("lseek dev_fd:%d error! \n", dev_fd);
                        close(dev_fd);
                        return -1;
                    }
                    printf("block %d not need write.\n", curBlock);
                }
#endif
                erase.start += erase.length;
                curBlock += 1;
            } 
            usleep(20000);
            oneRcvSize = 0;
            oneLeftSize = left > oneMaxSize ? oneMaxSize:left;
        }
    }
    if (buffer)
        free(buffer);
    if (nullData)
        free(nullData);
    return len;
ERROR:
    printf("sock_wr_n: %s-> sock[%d] ret[%d] errno[%d] [%d - %d = %d]\n",
	    read_opt?"read":"write", sock, ret, _GET_ERRNO(), size, len, (size - len)
);
    
    if (buffer)
        free(buffer);
    if (nullData)
        free(nullData);
    return ret;
}

static int sock_wrn_fast(int sock, int size, int timeout_sec, int
read_opt)
{
    int ret = 0;
	int len = 0;
    int left = 0;
    int tmpSize = 64 * 1024;
    FILE *upFp = NULL;

    char *buffer = (char*)malloc(tmpSize);
    
    if (buffer == NULL)
    {
        printf("sock_wrn_fast malloc error:%d\n", tmpSize);
        return 0;
    }
    upFp = fopen(UPGRADE_FAST_PATH, "wb");
    if (upFp == NULL)
    {
        return -1;
    }

    /* read data until size is reached, or tired of timeout */
    left = size;
    while (len < size) {
        tmpSize = left > 65536?65536:left;
        if (read_opt) {
            ret = recv(sock, buffer, tmpSize, 0);
        } else {
            ret = send(sock, buffer, tmpSize, 0);
        }
		if (ret <= 0) {
		    if ((_GET_ERRNO() == EINTR) || (_GET_ERRNO() == EAGAIN)) {
    			ret = wr_timedwait(sock, read_opt, 1, 0, timeout_sec);
    			if (ret > 0) {
    				continue;//read again
    			} else {
                    goto ERROR;
    			}
			} else {
                goto ERROR;
            }
		}
        left -= ret;
        fwrite(buffer, 1, ret, upFp);
        len += ret;
    }
    fclose(upFp);
    if (buffer)
        free(buffer);
    return len;
ERROR:
    printf("sock_wr_n: %s-> sock[%d] ret[%d] errno[%d] [%d - %d = %d]\n",
	    read_opt?"read":"write", sock, ret, _GET_ERRNO(), size, len, (size - len)
);
    
    if (buffer)
        free(buffer);
    return ret;
}



static int Upgrade_CmdUpgradeDataFast(int sock, int ext_len)
{
	int   ret       = 0;
    char cmds[512] = {0};
	//runSystemCfg.deviceInfo.upgradestatus = netcam_get_update_status();
	if(netcam_get_update_status() < 0)
	{
	    PRINT_ERR("is updating..............\n");
	    Upgrade_NotifyUpgradeDoing(sock,0);
	    return 0;
	}

	PRINT_INFO("sock:%d  ext_len:%d\n", sock, ext_len);

    strcpy(cmds, "touch /tmp/feeddog;killall -9 netcam;echo 3 > /proc/sys/vm/drop_caches");
    new_system_call(cmds);
	//netcam_update_relase_system_resource();

    sleep(1);
    ret = sock_wrn_fast(sock, ext_len, 30, 1);
    if (ret >= ext_len)
    {
        sprintf(cmds, "echo \"/usr/local/bin/upgrade_flash %s >> /tmp/uartfifo;sleep 2;reboot -f\" > /tmp/upsh \n chmod +x /tmp/upsh\n/tmp/upsh &", UPGRADE_FAST_PATH);
        new_system_call(cmds);
    }
    else
    {
        goto UP_UPGRADE_EXIT;
    }

	return 0;

UP_UPGRADE_EXIT:
	usleep(500 * 1000);
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
	return 0;
}


static int Upgrade_CmdUpgradeDataBin(int sock, int ext_len)
{
	int   ret       = 0;
    char cmds[512] = {0};
	//runSystemCfg.deviceInfo.upgradestatus = netcam_get_update_status();
	if(netcam_get_update_status() < 0)
	{
	    PRINT_ERR("is updating..............\n");
	    Upgrade_NotifyUpgradeDoing(sock,0);
	    return 0;
	}

	PRINT_INFO("sock:%d  ext_len:%d\n", sock, ext_len);

    strcpy(cmds, "touch /tmp/feeddog;killall -9 netcam;echo 3 > /proc/sys/vm/drop_caches");
    new_system_call(cmds);
	//netcam_update_relase_system_resource();

    sleep(1);
    printf("start to get data..\n");
    ret = sock_wrn_bin(sock, ext_len, 30, 1);
    printf("sock_wrn_fast %d,%d\n", ret, ext_len);
    if (ret >= ext_len)
    {
    }
    else
    {
        goto UP_UPGRADE_EXIT;
    }

	return 0;

UP_UPGRADE_EXIT:
	usleep(500 * 1000);
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
	return 0;
}



int UpgradeDealCmdDispatch(int sock)
{
	int ret = 0;
	fd_set fdr;
	char recv_buf[RECV_BUF_SIZE] = {0};
	char cmd[3] = {0};
	char data[8] = {0};
	char send_buf[4];
	unsigned short start_code = 0;
	unsigned short cmd_code   = 0;
    GK_UPGRADE_FILE_HEAD cmd_head = {0};

	ret = UpgradeSetSockAttr(sock);
	if (ret < 0) {
		printf("UpgradeSetSockAttr error\n");
		close(sock);
		return -1;
	}
	int opt = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret < 0) {
		printf("setsockopt SO_REUSEADDR failed, error:%s\n", strerror(errno));
		return -1;
	}
	memset(send_buf,0x0,sizeof(send_buf));
	while (1) {
		memset(&cmd_head, 0x0, sizeof(GK_UPGRADE_FILE_HEAD));
		memset(recv_buf,0x0,sizeof(recv_buf));
		ret = recv_data(sock,recv_buf,12);
		if(ret <= 0)
		{
			printf("Recv cmd error ret:%d\n", ret);
		}
		start_code = ((recv_buf[1] << 8) | recv_buf[0]);
		cmd_code   = ((recv_buf[3] << 8) | recv_buf[2]);
		//PRINT_INFO("===start_code is %x==\n==cmd_code is %x========\n",start_code,cmd_code);
		//start_code = ((recv_buf[1] << 8) | recv_buf[0]);
		if (START_CODE!= start_code)
		{
			printf("start_code:0x%x is err!\n", start_code);
			close(sock);
			return -1;
		}

    	memcpy(cmd,recv_buf+2,2);
		cmd_head.cmd = atoi(cmd);
		switch (cmd_code ) {
            case CMD_UPGRADE_FAST:  
                strncpy(data,recv_buf+4,8);
				cmd_head.nBufSize = atoi(data);
				PRINT_INFO("Ready upgrade fast  cmd_head.nBufSize is %d\n ",(int)cmd_head.nBufSize);
                upTotal = cmd_head.nBufSize/65536;
				Upgrade_CmdUpgradeDataFast(sock,cmd_head.nBufSize);
                printf("Upgrade_CmdUpgradeDataFast exit\n");
				break;
			case CMD_STATUS:
				ret = Get_Upgrade_Status(sock);
				if(ret)
				{
					close(sock);
				}
				break;
            case CMD_UPGRADE_BIN: 
                strncpy(data,recv_buf+4,8);
				cmd_head.nBufSize = atoi(data);
				PRINT_INFO("Ready upgrade bin  cmd_head.nBufSize is %d\n ",(int)cmd_head.nBufSize);
                upTotal = cmd_head.nBufSize/65536;
				Upgrade_CmdUpgradeDataBin(sock,cmd_head.nBufSize);
                upStatus = upTotal * 100;
                if(sock > 0)
                {
                    close(sock);
                }
                sleep(2);
                netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT);
                printf("Upgrade_CmdUpgradeDataBin exit\n");

            case CMD_UPGRADE_GET_NAME:
            {
                char devType[32] = {0};
                readfifo_sys_shell_result("cat /root/sys_info | grep device_type | awk '{print $2}' | awk -F\\\" '{print $2}'", devType, 32);
                printf("device type:%s\n", devType);
                int ret = send(sock, devType, 32, 0);//Upgrade_SockSend(sock, &send_data, sizeof(struct send_msg), 100);
                if (ret != 32) {
                    PRINT_ERR("send:%d size:32", ret);
                    return -1;
                } else {
                    //PRINT_INFO("send %d data_size:%d, head_size:%d\n", ret, data_size, sizeof(JB_NET_PACK_HEAD) + sizeof(JB_SERVER_MSG));
                }
                printf("send name over\n");
                
                break; 
            }
                
		    default:
				PRINT_INFO("cmd:0x%x unknow\n ", cmd_code);
	        	break;
		}
	}
	if(sock > 0)
	{
		FD_CLR(sock, &fdr);
		close(sock);
	}
	return 0;
}

static int UpCreateTcpSock(char *ipaddr, int port)
{
	int ret;
	int sockfd;
	in_addr_t s_addr;
	int reuseaddr = 1;
	struct sockaddr_in localaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return -1;

	if (ipaddr && ipaddr[0] && inet_pton(AF_INET, ipaddr, &s_addr) != 1) {
		goto fail;
	} else {
		s_addr = htonl(INADDR_ANY);
	}

	memset(&localaddr, 0, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(port);
	localaddr.sin_addr.s_addr = s_addr;

	/* Create socket for listening connections. */
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
	if (ret < 0)
		goto fail;

	ret = bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr));
	if (ret < 0)
		goto fail;

	ret = listen(sockfd, 20);
	if (ret < 0)
		goto fail;

	return sockfd;

fail:
	close(sockfd);
	return -1;
}

void * ipcsearch_thread(void *param)
{
	int ret           = 0;
	int listenfd = 0 , connectfd ;
	int fd_max = 0;
	fd_set readfd;

	struct timeval timeout;
	struct sockaddr_in addr;
	int sin_size;

	printf("in tool_discovery_thread\n");
    pthread_detach(pthread_self());
	//sdk_SetThreadName("ipcsearch_thread");
	sdk_sys_thread_set_name("ipcsearch_thread");
	g_search_thread_run = 1;

	listenfd = UpCreateTcpSock(NULL, UPGRADE_PORT);
	if (listenfd < 0) {
		PRINT_ERR("create listen socket fail");
		return NULL;
	}
	//SOCK_SET_NONBLOCKING(listenfd);
	int flags = fcntl(listenfd,F_GETFL,0);
	fcntl(listenfd,F_SETFL,flags|O_NONBLOCK);
	while (1)
	{
	    sleep(1);
		if(0 == g_search_thread_run)
		{
		    break;
		}

		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		FD_ZERO(&readfd);
		FD_SET(listenfd, &readfd);
		if(listenfd > fd_max)
			fd_max = listenfd;
		ret = select(fd_max + 1, &readfd, NULL, NULL, &timeout);

		if(0 == ret)
			continue;
		else if(ret < 0){
			break;
		}
		sin_size = sizeof(struct sockaddr_in);		
		if(FD_ISSET(listenfd, &readfd))
		{
			if ((connectfd = accept(listenfd, (struct sockaddr *)&addr, (socklen_t *) & sin_size)) == -1) {
				PRINT_ERR("Server: accept failed%s\n", strerror(errno));
				continue;
			}
			else
			{
				PRINT_INFO("accept session_socket:%d, remote ip:%s, remote port:%d\n",
			            connectfd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
				UpgradeDealCmdDispatch(connectfd);
				//CreateDetachThread(UpgradeSessionThread, (void *)connectfd, NULL);
			}
		}

	}
	if(listenfd > 0)
	{
		close(listenfd);
	}

	PRINT_INFO("ipc_discovery_thread end\n");
	return NULL;
}



int IPCsearch_init(void)
{
	int ret = 0;
	pthread_t thread_id;

	PRINT_INFO("Search Tool init\n");

	ret = pthread_create(&thread_id, NULL, ipcsearch_thread, NULL);
	if(ret < 0)
	{
		PRINT_ERR("create tool_discovery_thread failed! %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int IPCsearch_exit()
{
	g_search_thread_run = 0;
	usleep(200000);
	PRINT_INFO("descovery_deinit...\n");
	return 0;
}

int dwip_to_str(DWORD ip, char ip_str[32])
{
    ///char a0, a1, a2, a3;
    int  i0, i1, i2, i3;

    if (NULL == ip_str)
        return -1;
    i0 = ip & 0xff;
    i1 = (ip>>8)  & 0xff;
    i2 = (ip>>16) & 0xff;
    i3 = (ip>>24) & 0xff;

    sprintf(ip_str, "%d.%d.%d.%d",i0, i1, i2, i3);
    printf("xbb: %s", ip_str);

    return 0;
}

