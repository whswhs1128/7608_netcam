#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/unistd.h>

#include "netcam_api.h"
#include "flash_ctl.h"

static int get_p2pid(const char* buf, int len)
{
	int cmd = 1;
	int sockfd;
	struct sockaddr_in des_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("socket error");
		return -1;
	}

	des_addr.sin_family = AF_INET;
	des_addr.sin_port = htons(36678);
	des_addr.sin_addr.s_addr = inet_addr("192.168.10.163");
	bzero(&(des_addr.sin_zero), 8);

	if (connect(sockfd, (struct sockaddr *)&des_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("connect failed");
		return -1;
	}

	cmd = htonl(cmd);
	if (send(sockfd, &cmd, sizeof(cmd), 0) < 0)
	{
		printf("send msg failed!");
		close(sockfd);
		return -1;
	}

	if (recv(sockfd, buf, len, 0) < 0)
	{
		printf("recv fail!\n");
		close(sockfd);
		return -1;
	}

	close(sockfd);
	return 0;
}

static void* get_p2pid_thread(void* arg)
{
	int i;
	struct device_info_mtd mtd_info;

    pthread_detach(pthread_self());

	memset(&mtd_info, 0, sizeof(struct device_info_mtd));

	for (i = 0; i < 5; i++)
	{
		if (get_p2pid(mtd_info.device_id, sizeof(mtd_info.device_id)))
		{
			sleep(3);
			continue;
		}

		break;
	}

	if (i == 5)
	{
		printf("p2p id server is not start!\n");
		return 0;
	}

	printf("p2pid=[%s]\n", mtd_info.device_id);
	if (mtd_info.device_id[0] == '#')
	{
		printf("no enough p2p id\n");
		return 0;
	}

	save_info_to_mtd_reserve(MTD_TUTK_P2P, &mtd_info, sizeof(struct device_info_mtd));
    printf("Get p2p2 id, system will auto reboot\n");
    sleep(1);

	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT

	return 0;
}

int netcam_p2pid_init()
{
	pthread_t threadid;
	struct device_info_mtd mtd_info;

	memset(&mtd_info, 0, sizeof(struct device_info_mtd));
	if (load_info_to_mtd_reserve(MTD_TUTK_P2P, &mtd_info, sizeof(struct device_info_mtd)))
		goto get_id;

	if (mtd_info.device_id[0] != '\0')
	{
		return 0;
	}

get_id:
	if (pthread_create(&threadid, NULL, get_p2pid_thread, NULL) < 0)
	{
		printf("Fail to create thread[thread_http_server_run]! errno[%d] errinfo[%s]",
			errno, strerror(errno));
		return -1;
	}

	return 0;
}
