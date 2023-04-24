#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"
#include "parser_cfg.h"

static int Gk_CmsNotifyUpgrade(int sock, int write_size, int type)
{
    char *data = NULL;
    int data_size = 0;
    char *buf = malloc(data_size + sizeof(JB_NET_PACK_HEAD) + sizeof(JB_SERVER_MSG));
    if (buf == NULL) {
        PRINT_ERR("Gk_CmsNotify malloc error\n");
        return -1;
    }

    JB_NET_PACK_HEAD *cmd_head = (JB_NET_PACK_HEAD *)buf;
    cmd_head->nFlag = 9000;
    cmd_head->nCommand = GK_NET_CMD_NOTIFY;
    cmd_head->nChannel = 0;
    cmd_head->nErrorCode = 0;
    cmd_head->nBufSize = sizeof(JB_SERVER_MSG) + data_size;

    JB_SERVER_MSG *msg_head = (JB_SERVER_MSG *)(buf + sizeof(JB_NET_PACK_HEAD));
    msg_head->dwMsg = type;
    msg_head->dwChannel = 0;
    msg_head->cbSize = write_size;

    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);
    struct timeval time_tmp;
    gettimeofday(&time_tmp, NULL);

    msg_head->st.wYear = pTm->tm_year + 1900;
    msg_head->st.wMonth = pTm->tm_mon + 1;
    msg_head->st.wDay = pTm->tm_mday;
    msg_head->st.wHour = pTm->tm_hour;
    msg_head->st.wMinute = pTm->tm_min;
    msg_head->st.wSecond = pTm->tm_sec;
    msg_head->st.wDayOfWeek = pTm->tm_wday;
    msg_head->st.wMilliseconds = time_tmp.tv_usec/1000;

    char *msg = (char *)(buf  + sizeof(JB_NET_PACK_HEAD) + sizeof(JB_SERVER_MSG));
    if ((data != NULL) && (data_size != 0))
        memcpy(msg, data, data_size);

    int ret = GkSockSend(sock, buf, sizeof(JB_NET_PACK_HEAD) + sizeof(JB_SERVER_MSG) + data_size, 100);
    if (ret != (sizeof(JB_NET_PACK_HEAD) + sizeof(JB_SERVER_MSG) + data_size)) {
        PRINT_ERR("send:%d size:%d", ret, sizeof(JB_NET_PACK_HEAD) + sizeof(JB_SERVER_MSG) + data_size);
        free(buf);
        return -1;
    } else {
        //PRINT_INFO("send %d data_size:%d, head_size:%d\n", ret, data_size, sizeof(JB_NET_PACK_HEAD) + sizeof(JB_SERVER_MSG));
    }

    free(buf);
    return 0;
}


static int Gk_CmsNotifyUpgradeDoing(int sock, int write_size)
{
    return Gk_CmsNotifyUpgrade(sock, write_size, JB_MSG_UPGRADE);
}

static int Gk_CmsNotifyUpgradeDone(int sock, int write_size)
{
    return Gk_CmsNotifyUpgrade(sock, write_size, JB_MSG_UPGRADEOK);
}

static int Gk_CmsNotifyUpgradeError(int sock, int write_size)
{
    return Gk_CmsNotify(sock, JBERR_OPEN_FILE, NULL, 0);
}

static int Gk_wr_timedwait(int sock, int read_opt, int sec, int usec, int times)
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
 		        (errno != EINTR) && (errno !=EAGAIN))) {
            break;
		}
		times--;
        printf("wr_timedwait: %s sock[%d] select timeout, times[%d], ret = %d, errno = %d\n",read_opt?"read":"write", sock, times,ret, errno);
	}
	return ret;
}

int Gk_CmdUpgradeData(int sock, int ext_len)
{
    int   ret       = 0;
    int   cnt       = 0;
    int   len       = 0;
    int   offset    = 0;
    char *pRecv_buf = NULL;

    if(netcam_get_update_status() < 0)
    {
	    PRINT_ERR("is updating..............\n");
        Gk_CmsNotifyUpgradeError(sock, ext_len);
        return 0;
    }
    PRINT_INFO("sock:%d  ext_len:%d\n", sock, ext_len);

    netcam_update_relase_system_resource();
    //netcam_video_exit();

    pRecv_buf = netcam_update_malloc(ext_len + 2);
    if (NULL == pRecv_buf)
    {
        PRINT_ERR("malloc upgrade_size:%u ERROR! %s\n", ext_len + 2, strerror(errno));
        goto CMS_UPGRADE_EXIT;
    }

    len = ext_len;
    while (len > 0) {
        ret = recv(sock, pRecv_buf + offset, len, 0);
        if (ret > 0)
        {
            offset += ret;
            len -= ret;
        }
        else
        {
    		if ((errno == EINTR) || (errno == EAGAIN))
            {
    			ret = Gk_wr_timedwait(sock, 1, 1, 0, 30);
    			if (ret > 0)
                {
    				continue;//read again
    			}
                else
                {
                    goto CMS_UPGRADE_EXIT;
    			}
    		}
            else
            {
                goto CMS_UPGRADE_EXIT;
            }
            PRINT_INFO("offset:%d  len:%d  ret:%d\n", offset, len, ret);
            //break;
        }
    }
    PRINT_INFO("========== Get file ok. offset:%d  len:%d  ret:%d  =========\n", offset, len, ret);

    ret = netcam_update(pRecv_buf, ext_len, NULL);
    if (ret < 0)
	{
		PRINT_ERR("Check updating package, failed: %d \n", ret);
        Gk_CmsNotifyUpgradeError(sock, ext_len);
        goto CMS_UPGRADE_EXIT;
	}

    cnt = 0;
    while (cnt < 99)
    {
        cnt = netcam_update_get_process();
        len = (ext_len * cnt)/100/2;
        ret = Gk_CmsNotifyUpgradeDoing(sock, len);
        usleep(218 * 1000);
        PRINT_INFO("sock:%d  len:%d  cnt:%d  ret:%d \n", sock, len, cnt, ret);
    }

    ret = Gk_CmsNotifyUpgradeDone(sock, ext_len);

    PRINT_INFO("Gk_CmsNotifyUpgradeDone! sock:%d  len:%d  ret:%d\n", sock, len, ret);

	//如果校验成功，则内存地址将会被升级线程所使用，这里不能释放 !!
    //free(pRecv_buf);

CMS_UPGRADE_EXIT:
    usleep(500 * 1000);
    #if 0
    netcam_exit(90);
    new_system_call("reboot -f");//force REBOOT
	#endif
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
    return 0;
}


int Gk_CmdUpgradeReq(int sock, int ext_len)
{
    char recv_buf[GK_CMS_BUFLEN];
    RecvExtData(sock, recv_buf, GK_CMS_BUFLEN, ext_len);

    return 0;
}

int Gk_CmdUpgradeResp(int sock, int ext_len)
{
    char recv_buf[GK_CMS_BUFLEN];
    RecvExtData(sock, recv_buf, GK_CMS_BUFLEN, ext_len);

    return 0;
}


