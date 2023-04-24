#include "parser_cfg.h"
#include "flash_ctl.h"
#include "netcam_api.h"
#include <sys/mount.h>
#include "sdk_sys.h"
#include "md5.h"
#include "media_fifo.h"
#ifdef MODULE_SUPPORT_UPGRADE_OUT
#include <sys/shm.h>
#endif



static int startUpdate = 0;
static char * buff_upgrade = NULL;
static int  upgrade_len = 0;
static cbFunc upgrade_cb = NULL;
static cbFunc down_load_cb 	   = NULL;
static void*  down_load_cb_arg = NULL;
static pthread_mutex_t cb_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t upTid ;
static int isSaveMd5 = 0;
static int isPrepareUpgrade = 0;

#ifdef MODULE_SUPPORT_UPGRADE_OUT
typedef struct
{
    int upgradeStart;
    int upgradeProcess;
    int fileSize;
    int isFeedDog;
    int isSaveMd5;
    char sdkName[32];
    char curVer[32];
    int memOffset;
}UPGRADE_SHARE_INFO;

UPGRADE_SHARE_INFO *shareInfo = NULL;
int shmid = -1;

void* shmFile = NULL;
int shmidFile = -1;
int shmidFileSize = 0;
#endif

static void upgrade_save_md5(unsigned char *inEncrypt,int len)
{
	unsigned char md5_str[64]={0};
	unsigned char md5_val[16]={0};
	MD5_CTX md5;

	MD5Init(&md5);
	MD5Update(&md5,inEncrypt,len);
	MD5Final(&md5,md5_val);

	snprintf(md5_str,64,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	md5_val[0],md5_val[1],md5_val[2],md5_val[3],
	md5_val[4],md5_val[5],md5_val[6],md5_val[7],
	md5_val[8],md5_val[9],md5_val[10],md5_val[11],
	md5_val[12],md5_val[13],md5_val[14],md5_val[15]);

	if(save_info_to_mtd_reserve(MTD_MD5_INFO, md5_val, sizeof(md5_val)) != 0)
	{
		PRINT_ERR("upgrade_save_md5 %s failed!\n",md5_str);
	}
	else
	{
		PRINT_INFO("upgrade_save_md5 %s success!\n",md5_str);
	}
}

static void * upgrade_thread(void *param)
{
    int  ret = 0;
    struct timespec times = {0, 0};
    static unsigned long last_timestamp = 0;

	sdk_sys_thread_set_name("upgrade_thread");
    PRINT_INFO("Start upgrade_thread ......\n");
	pthread_detach(pthread_self());
	while(1)
	{
		pthread_mutex_lock(&cb_lock);

		if(down_load_cb)
        {
			down_load_cb(down_load_cb_arg);
			down_load_cb = NULL;

			pthread_mutex_unlock(&cb_lock);

			continue;
		}

		pthread_mutex_unlock(&cb_lock);

		if(isPrepareUpgrade)
		{
			clock_gettime(CLOCK_MONOTONIC, &times);

			if(!last_timestamp)
				last_timestamp = times.tv_sec;
			else if((times.tv_sec - last_timestamp) >= 30*60) //释放资源后超过30分钟还没有升级成功直接重启
			{
				PRINT_ERR("\n###########Update Release resource exceed 30 min! REBOOT SYSTEM!\n");
				last_timestamp = times.tv_sec;
                startUpdate = 0;
				netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);
				continue;
			}
		}

		if(startUpdate == 0 || buff_upgrade == NULL || upgrade_len == 0)
		{
			usleep(500000);
			continue;
		}

        if (NULL == buff_upgrade || 0 == upgrade_len )
        {
            PRINT_ERR("This upgrade input param error!\n");
            goto Upgrade_error;
        }

        LOG_NOTICE("Start write updating package(g_pbuf_upgrade:%p, len:%d)...\n", buff_upgrade, upgrade_len);
        if(upgrade_cb)
            upgrade_cb(UPDATE_START);
        ret = upgrade_flash_by_buf(buff_upgrade, upgrade_len);
        if (ret < 0)
        {
            PRINT_ERR("Write updating package:error ret:%d\n", ret);
            goto Upgrade_error;
        }

        if (isSaveMd5)
        {
		    upgrade_save_md5(buff_upgrade,upgrade_len);
        }
        LOG_NOTICE("Write updating all package success...\n");
        if(upgrade_cb)
            upgrade_cb(UPDATE_SUCCESS);
		
		/* 升级进度完成 */
		set_upgrade_end();
    	sleep(5);
        startUpdate = 0;

        //netcam_exit(90);
        //new_system_call("reboot -f");//force REBOOT
        netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);

    Upgrade_error:

        if(upgrade_cb)
            upgrade_cb(UPDATE_FAIL);
        buff_upgrade  = NULL;
        upgrade_len   = 0;
        startUpdate = 0;
        isPrepareUpgrade = 0;
	}
    return NULL;
}

#define NETCAM_SDCARD_UPDATE_INI "/mnt/sd_card/update.ini"
int netcam_update_for_sdcard()
{
    int fileSize = 0;
    int offset = 0;
    int len = 0;
    int cnt = 0;
    int ret = 0;
    char *data = NULL;
    struct gk_upgrade_header p_upgrade_header;

    char path[256] = {0};
    char filename[256] = {0};

    memset(&p_upgrade_header, 0, sizeof(p_upgrade_header));

    //读取升级文件名
    FILE *fp_ini = NULL;
    fp_ini = fopen(NETCAM_SDCARD_UPDATE_INI, "rb");
    if (fp_ini == NULL) {
        LOG_WARNING(NETCAM_SDCARD_UPDATE_INI" not exist!");
        return -1;
    }
    fscanf(fp_ini,"%s",filename);//读1行
    fclose(fp_ini);

    //打开升级文件
    sprintf(path, "%s%s", "/mnt/sd_card/", filename);
    PRINT_INFO("find update file: [%s]\n", path);

    FILE *fp = NULL;
    fp = fopen(path, "rb");
    if (fp == NULL) {
        PRINT_ERR("fopen error %s not exist!", path);
        return -1;
    }

    //检测升级文件是否匹配
    if (fread((char *)&p_upgrade_header, 1, sizeof(struct gk_upgrade_header), fp)
                != (sizeof(struct gk_upgrade_header))) {
            PRINT_ERR("fread error!");
            fclose(fp);
            return -1;
    }
    ret = check_upgrade_header(&p_upgrade_header);
    if(ret < 0)
    {
        PRINT_ERR("update file error!");
        fclose(fp);
        return -1;
    }

    //获取文件大小
    if (0 != fseek(fp, 0, SEEK_END)) {
        fclose(fp);
        return -1;
    }
    fileSize = ftell(fp);

    data = netcam_update_malloc(fileSize);
    if(!data) {
        PRINT_ERR();
        fclose(fp);
        return -1;
    }
    memset(data, 0, fileSize);

    if(0 != fseek(fp, 0, SEEK_SET)) {
        PRINT_ERR();
        free(data);
        fclose(fp);
        return -1;
    }

    //读取升级文件
    len = fileSize;
    while (len > 0) {
        ret = fread(data + offset, 1, len, fp);
        if (ret >= 0)
        {
            offset += ret;
            len -= ret;
        }
        else
        {
            PRINT_INFO("offset:%d  len:%d  ret:%d\n", offset, len, ret);
            //break;
        }
    }
    fclose(fp);

    //开始升级
    ret = netcam_update(data, fileSize, NULL);
    if (ret < 0)
    {
        PRINT_ERR("Check updating package, failed: %d \n", ret);
        free(data);
        return -1;
    }

    cnt = 0;
    while (cnt < 99)
    {
        cnt = netcam_update_get_process();
        len = (fileSize * cnt)/100/2;
        usleep(218 * 1000);
        PRINT_INFO("len:%d  cnt:%d  ret:%d \n", len, cnt, ret);
    }

    usleep(500 * 1000);
    //netcam_exit(90);
    //new_system_call("reboot -f");//force REBOOT
	netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);
    return 0;
}

int netcam_update_init(cbFunc updateCb)
{
#ifdef MODULE_SUPPORT_UPGRADE_OUT
    if (shmid != -1)
    {
        return 0;
    }
    void* shm = NULL;
    shmid = shmget((key_t)8891,sizeof(UPGRADE_SHARE_INFO),0666|IPC_CREAT);   //创建共享内存
    if(shmid == -1)
    {
        printf("shmget error\n");
    }
    else
    {
        printf("shmid = %d\n",shmid);
    }

    shm = shmat(shmid,(void*)0,0);                        //映射共享内存
    if(shm == (void*)(-1))
    {
        printf("shmat error\n");
        shareInfo = NULL;
    }
    else
    {
        shareInfo = (UPGRADE_SHARE_INFO*)shm;
		memset(shareInfo, 0, sizeof(UPGRADE_SHARE_INFO));
        shareInfo->upgradeProcess = 0;
        shareInfo->isFeedDog = 1;
        shareInfo->memOffset = 0;
    }
    return 0;
#else
	int ret;

    if (upgrade_cb != NULL)
    {
        return 0;
    }

	upgrade_cb = updateCb;
	ret = pthread_create(&upTid, NULL, upgrade_thread,NULL);
	if(ret != 0)
	{
		PRINT_ERR("Create update thread failed,Please try again:%s",strerror(errno));
	}
	else
	{
		 PRINT_INFO("Create update thread success");
	}

    //检测SD卡升级
    //FIXME(heyong): 暂时关闭SD卡升级程序
    //netcam_update_for_sdcard();
	return ret;
#endif
}
int netcam_get_update_status()
{
	if(startUpdate == 1)
	{
		//PRINT_ERR("system is updating, don't call update again... \n" );
		return -1;
	}
    return 0;
}

int netcam_update_feed_dog(void)
{
#ifdef MODULE_SUPPORT_UPGRADE_OUT
    if (shareInfo != NULL)
    {
        shareInfo->isFeedDog++;
    }
#else
#endif
    return 0;
}

void *netcam_update_malloc(int size)
{
#ifdef MODULE_SUPPORT_UPGRADE_OUT
    if (shmFile != NULL)
    {
        if (size == shmidFileSize)
        {
            return shmFile;
        }
        else
        {
            //升级分配内存大小不一致时返回null，重启后再重新申请
            return NULL;
        }
    }

    shmidFile = shmget((key_t)8892, size, 0666|IPC_CREAT);   //创建共享内存
    if(shmidFile == -1)
    {
        printf("shmget error\n");
    }
    else
    {
        printf("shmid = %d\n",shmidFile);
    }

    shmFile = shmat(shmidFile,(void*)0,0);                        //映射共享内存
    if(shmFile == (void*)(-1))
    {
        printf("shmat error\n");
        shmFile = NULL;
    }
    shmidFileSize = size;
    return shmFile;
#else
    return malloc(size);
#endif
}

int netcam_update(char *binData, int length,cbFunc updateCb)
{
	int ret;
#ifdef MODULE_SUPPORT_GOKE_UPGRADE
    char str[64] = {0};
    sprintf(str, "cur:%d, len:%d", startUpdate, length);
    goke_upgrade_udp_info(3, str);
#endif
#ifdef MODULE_SUPPORT_UPGRADE_OUT
	if(startUpdate == 1)
	{
		PRINT_ERR("system is updating, don't call update again... \n" );
		return 0;
	}
	startUpdate = 1;
    
	PRINT_INFO("Check updating package... \n" );
	ret = verify_upgrade_buff(binData, length);
	if (ret < 0)
	{
		PRINT_ERR("Check updating package, failed: %d \n", ret);
		goto update_exit;
	}
	else
	{
		PRINT_INFO("Check updating package, success \n");
        if (shareInfo != NULL)
        {
            shareInfo->upgradeProcess = 0;
            shareInfo->upgradeStart = 1;
            shareInfo->fileSize = length;
            shareInfo->isSaveMd5 = isSaveMd5;
            strcpy(shareInfo->sdkName, sdk_cfg.name);
            strcpy(shareInfo->curVer, runSystemCfg.deviceInfo.upgradeVersion);
        }
	}
    return 0;
#else
	if(startUpdate == 1)
	{
		PRINT_ERR("system is updating, don't call update again... \n" );
		return 0;
	}
	startUpdate = 1;

	PRINT_INFO("Check updating package... \n" );
	ret = verify_upgrade_buff(binData, length);
	if (ret < 0)
	{
		PRINT_ERR("Check updating package, failed: %d \n", ret);
		goto update_exit;
	}
	else
	{
		PRINT_INFO("Check updating package, success \n");
	}

	buff_upgrade = binData;
	upgrade_len = length;

	return 0;
#endif
update_exit:
    if(upgrade_cb)
        upgrade_cb(UPDATE_FAIL);
	buff_upgrade = 0;
	upgrade_len = 0;
	startUpdate = 0;
    isPrepareUpgrade = 0;
	return -1;
}

int netcam_update_mail_style(char *binData, int length,cbFunc updateCb)
{
	char boundary[256];
	char *findData;
	char *startPtr;
	char *endPtr;
	char retData[256];
	char appLen[]="Content-Type: application/octet-stream\r\n\r\n";

	char *mailUpdateBuf = NULL;
	int mailUpdateLen = 0;
	int i;
	int ret;

	memset(retData, 0, 256);


	if(startUpdate == 1)
	{
		PRINT_ERR("system is updating, don't call update again... \n");
		return  -1;

	}

	//startUpdate = 1;

	memset(boundary,0,sizeof(boundary));
	findData=strstr(binData,"\r\n");
	if(findData == NULL)
	{
		PRINT_ERR("update package is not mail sytle format \n");
		goto update_mail_exit;
	}
	memcpy(boundary,binData,findData-binData);
	PRINT_INFO("boundary location: %s\n",boundary);
	startPtr=strstr(binData,appLen);
	if(startPtr == NULL)
	{
	 	PRINT_ERR("No found start boundary info, error http update package");
		goto update_mail_exit;

	}


	startPtr += strlen(appLen);
	PRINT_INFO("location:0x%x\n",(int)startPtr);
	endPtr = binData+length-1;
	i = 0;
	while(*endPtr!='\0'  && i < 1024)
	{
	    endPtr--;
	    i++;
	}

	PRINT_INFO("find end:  i=%d \n",i);
	endPtr++;
	endPtr =strstr(endPtr,boundary);
	 if(endPtr != NULL)
	 {
	    mailUpdateLen=endPtr-startPtr-2;
	    PRINT_INFO("len:%d\n", mailUpdateLen);
	 }
	 else
	 {
	 	PRINT_ERR("No found end boundary info, error http update package");
		goto update_mail_exit;

	 }

	 mailUpdateBuf = startPtr;

#ifdef MODULE_SUPPORT_GOKE_UPGRADE
     char str[64] = {0};
     sprintf(str, "cur:%d, len:%d", startUpdate, length);
     goke_upgrade_udp_info(3, str);
#endif

#ifdef MODULE_SUPPORT_UPGRADE_OUT
     if (shareInfo != NULL)
        shareInfo->memOffset = mailUpdateBuf - binData;

     netcam_update(mailUpdateBuf, mailUpdateLen, NULL);
#else
	 ret = verify_upgrade_buff(mailUpdateBuf, mailUpdateLen);
	 if (ret < 0)
	 {
		 PRINT_ERR("Check updating package, failed: %d \n", ret);
		 goto update_mail_exit;
	 }
	 else
	 {
		 PRINT_ERR("Check updating package, success \n");
	 }

	 buff_upgrade = mailUpdateBuf;
	 upgrade_len = mailUpdateLen;
#endif

	return 0;
update_mail_exit:
    if(upgrade_cb)
    {
        upgrade_cb(UPDATE_FAIL);
    }	//xqq
	buff_upgrade = 0;
	upgrade_len = 0;
	startUpdate = 0;
	return -1;
}



int netcam_update_get_process(void)
{
#ifdef MODULE_SUPPORT_UPGRADE_OUT
    if (shareInfo != NULL)
    {
        return shareInfo->upgradeProcess;
    }
#else
	return get_upgrade_rate();
#endif
}

int netcam_is_prepare_update(void)
{
	return isPrepareUpgrade;
}

int netcam_update_relase_system_resource(void)
{
	int ret;
    int retryCnt = 3;
	int i;
    PRINT_INFO("Update Release resource ...\n");

    if(isPrepareUpgrade)
    {
        return 0;
    }		//xqq
	isPrepareUpgrade = 1;
#ifdef MODULE_SUPPORT_GB28181
	gb28181_close();
	PRINT_INFO("gb28181 Release resource ...\n");
#endif

    #ifdef MODULE_SUPPORT_GKVISION
    printf("exit gkvision\n");
    gk_vision_exit();
    #endif

	#ifdef MODULE_SUPPORT_GOOLINK
	GLNK_UpdateRelaseResource();
	#endif
    //清掉缓存数据
	//sdk1.1 not release md here
    //netcam_md_enable(0);
    //PRINT_INFO("Release MD resource end\n");
    netcam_timer_destroy();
    PRINT_INFO("Release Timer resource end\n");
    //stop_default_workqueue();

    local_record_uninit();
    mmc_sdcard_status_check_exit();
    PRINT_INFO("Release local record resource end\n");
    netcam_osd_deinit();
    PRINT_INFO("Release osd resource end\n");

#if 1
	for(i = 0; i < VENC_MAX_STREAM_NUM; i ++)
		mediabuf_uninit(i);
#endif

    new_system_call("sync &");
    new_system_call("echo 3 > /proc/sys/vm/drop_caches");
    while (retryCnt-- > 0)
    {
    	ret = umount2("/opt/resource",MNT_FORCE);
    	if(ret != 0)
    	{
    		PRINT_ERR("unmount2, /opt/resource error: %s",strerror(errno));
            new_system_call("lsof | grep /opt/resource");
            sleep(1);
        }
        else
        {
            break;
        }
    }
    retryCnt = 3;
    while (retryCnt-- > 0)
    {
        ret = umount2("/opt/custom",MNT_FORCE);
    	if(ret != 0)
    	{
    		PRINT_ERR("unmount2, /opt/custom error: %s",strerror(errno));
            new_system_call("lsof | grep /opt/custom");
            sleep(1);
        }
        else
        {
            break;
        }
    }

	usleep(50*1000);
    new_system_call("sync &");
    new_system_call("echo 3 > /proc/sys/vm/drop_caches");
    new_system_call("free");
	return 0;

}

int netcam_update_set_down_load_cb(cbFunc down_load,void *arg)
{
	int ret = -1;

	pthread_mutex_lock(&cb_lock);

	if(down_load_cb == NULL)
	{
		down_load_cb_arg = arg;
		down_load_cb = down_load;
		ret = 0;
	}
	pthread_mutex_unlock(&cb_lock);

	return ret;
}

void netcam_update_is_save_md5(int isSet)
{
    isSaveMd5 = isSet;
}


