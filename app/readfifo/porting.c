#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/vfs.h>
#include <errno.h>
#include <netinet/tcp.h>

#include "netcam_api.h"

extern int new_system_call(const char *cmdline);

void sdk_sys_thread_set_name(char *name)
{
    prctl(PR_SET_NAME, (unsigned long)name, 0,0,0); \
    pid_t tid;\
    tid = syscall(SYS_gettid);\
    LOG_INFO("set pthread name:%d, %s, %s pid:%d tid:%d\n", __LINE__, __func__, name, getpid(), tid);\

}

int copy_file(char *src_name, char *des_name)
{
	int fd_old, fd_new, ret;
	char bufer[1024];
	fd_old = open(src_name, O_RDONLY);
	if(fd_old < 0)
	{
		LOG_ERR("open %s fail", src_name);
		return -1;
	}
	fd_new = open(des_name, O_WRONLY|O_TRUNC|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
	if(fd_new < 0)
	{
		LOG_ERR("open %s fail", des_name);
		close(fd_old);
		return -1;
	}
	while((ret = read(fd_old, bufer, sizeof(bufer))) > 0)
	{
		if(write(fd_new, bufer, ret) <= 0)
		{
			close(fd_old);
			close(fd_new);
			remove(des_name);
			return -1;
		}
	}
	close(fd_old);
	close(fd_new);
	return 0;
}

int readfifo_sys_shell_result(char *cmd, char *buf, int buflen)
{
    FILE *fp = NULL;
	int len = 0;

	if(!cmd || !buf)
		return -1;

	if (NULL == (fp = popen(cmd, "r")))
	{
		goto err;
	}

	if (NULL == fgets(buf, buflen, fp))
	{
		goto err;
	}

	len = strlen(buf);
	if(len > 0)
	{
		if(buf[len-1] == '\n')
			buf[len-1] = '\0';
	}
	else
		goto err;

	pclose(fp);

	return 0;

err:
	if(fp)
		pclose(fp);

	return -1;
}


int gb28181_close(void)
{
}

void GLNK_UpdateRelaseResource(void)
{
}

void netcam_md_enable(int enable)
{
}

void netcam_timer_destroy()
{
}

void local_record_uninit()
{
}

int mmc_sdcard_status_check_exit(void)
{
    return 0;
}


int netcam_osd_deinit()
{
    return 0;
}

int mediabuf_uninit(int fifo_id)
{
    return 0;
}


void netcam_sys_operation(void *fps, void *operation)
{
    NETCAM_SYS_OPERATION oper = (NETCAM_SYS_OPERATION)operation;
    switch(oper)
    {
        case SYSTEM_OPERATION_REBOOT:
        {
            char cmd[32] = {0};
            snprintf(cmd, sizeof(cmd),"%s -f -d %d ", "/sbin/reboot", 0);
            new_system_call(cmd);
			sleep(2);

            break;
        }
        default:
            printf("netcam_sys_operation %d not handle\n", oper);
    }
}
