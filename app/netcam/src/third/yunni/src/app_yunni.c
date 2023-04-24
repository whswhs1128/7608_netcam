#include "app_yunni.h"
#include "eventalarm.h"
#include "common.h"
#include "flash_ctl.h"
#include "bellcall.h"
#include "cfg_all.h"
#include "sdk_sys.h"
#include "ipcsearch.h"
#include "flash_ctl.h"

//#define DEFAULT_YUNNI_SERVER     "SVTDEHAYLOSQTBHYPAARPCPFLKLQSTPNPDTAEIHUPLASEEPIPKAOSUPESXLXPHLUSQLVLSPALNLTLRLKLOLMLP-$$"
//#define DEFAULT_YUNNI_DEVICE_ID  "OBJ-099958-CADDC"

//extern DBD DoorBellData;
static void* g_yunni_p2p_handle = 0;
//static int g_yunni_init = 0;
//struct device_info_mtd device_info;

#define MAX_ALARM_INTERVAL 30

int yunni_p2p_notify_alarm_md()
{
	int ret;
	//printf("================> Enter yunni_p2p_notify_alarm_md\n");
	ret = PPMsgPush(1, NULL, 0);
	//printf("================> Leave yunni_p2p_notify_alarm_md\n");
	return ret;
}


int yunni_p2p_alarm_cb_func(int nChannel, int nAlarmType, int nAction, void* pParam)
{
    //PRINT_INFO("nAlarmType:%d, nAction:%d\n", nAlarmType, nAction);

	if(nAction == 0)
	{
		//PRINT_INFO("\n");
		return 0;
	}

    char eventType[20] = {0};
	char strAction[20] = {0};

    #if 0
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);
    char startTime[128] = {0};
    sprintf(startTime,"%d-%02d-%02d %02d:%02d:%02d",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);
    #endif

    switch(nAlarmType)
    {
        case GK_ALARM_TYPE_ALARMIN:     //0:ÐÅºÅÁ¿±¨¾¯¿ªÊ¼
        	sprintf(eventType ,"%s", "LocalIO");
			sprintf(strAction ,"%s", "Start");
			break;

        case GK_ALARM_TYPE_DISK_FULL:       //1:Ó²ÅÌÂú
			sprintf(eventType ,"%s", "StorrageLowSpace");
			break;

        case GK_ALARM_TYPE_VLOST:            //2:ÐÅºÅ¶ªÊ§
        	sprintf(eventType ,"%s", "VideoLoss");
			sprintf(strAction ,"%s", "Start");
			break;

        case GK_ALARM_TYPE_VMOTION:          //3:ÒÆ¶¯Õì²â
			sprintf(eventType ,"%s", "VideoMotion");
			sprintf(strAction ,"%s", "Start");
            yunni_p2p_notify_alarm_md();
            break;
        case GK_ALARM_TYPE_DISK_UNFORMAT:    //4:Ó²ÅÌÎ´¸ñÊ½»¯
			sprintf(eventType ,"%s", "StorageNotExist");
			break;

        case GK_ALARM_TYPE_DISK_RWERR:       //5:¶ÁÐ´Ó²ÅÌ³ö´í,
			sprintf(eventType ,"%s", "StorageFailure");
			break;

        case GK_ALARM_TYPE_VSHELTER:         //6:ÕÚµ²±¨¾¯
			sprintf(eventType ,"%s", "VideoBlind");
			break;

        case GK_ALARM_TYPE_ALARMIN_RESUME:         //9:ÐÅºÅÁ¿±¨¾¯»Ö¸´
			sprintf(eventType ,"%s", "LocalIO");
			sprintf(strAction ,"%s", "Stop");
			break;

        case GK_ALARM_TYPE_VLOST_RESUME:         //10:ÊÓÆµ¶ªÊ§±¨¾¯»Ö¸´
			sprintf(eventType ,"%s", "VideoLoss");
			sprintf(strAction ,"%s", "Stop");
			break;

        case GK_ALARM_TYPE_VMOTION_RESUME:         //11:ÊÓÆµÒÆ¶¯Õì²â±¨¾¯»Ö¸´
			sprintf(eventType ,"%s", "VideoMotion");
			sprintf(strAction ,"%s", "Stop");
			break;

        case GK_ALARM_TYPE_NET_BROKEN:       //12:ÍøÂç¶Ï¿ª
			sprintf(eventType ,"%s", "NetAbort");
			break;

        case GK_ALARM_TYPE_IP_CONFLICT:      //13:IP³åÍ»
			sprintf(eventType ,"%s", "IPConfict");
			break;

        default:
			PRINT_INFO("Alarm: Type = %d", nAlarmType);
        	break;
    }

    //PRINT_INFO("alarm_type:%s action:%s\n", eventType, strAction);

    return 0;
}

static int yunni_p2p_device_info_load(struct device_info_mtd* pinfo)
{
    if(!pinfo)
    {
        PRINT_ERR("pinfo is NULL\n");
        return -1;
    }
    if(load_info_to_mtd_reserve(MTD_YUNNI_P2P, pinfo,sizeof(struct device_info_mtd)))
    {
        PRINT_ERR("no yunni device id in mtd1\n");
        return -1;
    }

	strncpy(runSystemCfg.deviceInfo.serialNumber, pinfo->device_id,
		sizeof(runSystemCfg.deviceInfo.serialNumber));
	SystemCfgSave();

//	g_yunni_init = 1;
	PRINT_INFO("load yunni device info success!%x,%x,%x\n", pinfo->device_id[0], pinfo->device_id[1], pinfo->device_id[2]);
    return 0;
}

static void *yunni_p2p_thread(void *param)
{
	//char server_id[] = "SVTDEHAYLOSQTBHYPAARPCPFLKLQSTPNPDTAEIHUPLASEEPIPKAOSUPESXLXPHLUSQLVLSPALNLTLRLKLOLMLP-$$";
	//char device_id[] = "OBJ-099954-EEACE";
/*
	FILE *fp;
	char buf[64] = {0};
	fp = fopen("/opt/custom/yunni_deviceid", "r");
	if (!fp)
		return NULL;

	fread(buf, sizeof(buf), 1, fp);
	fclose(fp);

	PRINT_INFO("yunni device ID: %s\n", buf);
*/
	struct device_info_mtd device_info;

	memset(&device_info, 0, sizeof(struct device_info_mtd));
	sdk_sys_thread_set_name("yunni_p2p_thread");

	if (yunni_p2p_device_info_load(&device_info))
		return NULL;
	PRINT_INFO("YunNi manufacturer ID:%s\n", device_info.manufacturer_id);
    PRINT_INFO("YunNi device ID:%s\n", device_info.device_id);
	add_p2p_info("YunNi", device_info.device_id);
    PPSvrInit(device_info.manufacturer_id, device_info.device_id, 0);
    return NULL;
}

int yunni_p2p_start()
{
	PRINT_INFO("yunni p2p server start...\n");
	//if (!g_yunni_init)
	//	return -1;

    g_yunni_p2p_handle = event_alarm_open(yunni_p2p_alarm_cb_func);

	//¿ªÆôÔÆÄÞP2P³õÊ¼»¯Ïß³Ì
    pthread_t thread_id;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    //pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//detached
    if(0 != pthread_create(&thread_id, &attr, yunni_p2p_thread, NULL))
    {
        pthread_attr_destroy(&attr);
        return -1;
    }
    pthread_attr_destroy(&attr);
    P2PName_add(MTD_YUNNI_P2P);
	return 0;
}

int yunni_p2p_stop()
{

	event_alarm_close(g_yunni_p2p_handle);

	PPSvrDestroy();
    usleep(200000);
    PRINT_INFO("yunni p2p server stop...\n");
    return 0;
}

/*
int yunni_p2p_get_device_info_load(struct device_info_mtd  *deviceInfo)
{

	memcpy(deviceInfo,&device_info,sizeof(struct device_info_mtd));
	return 0;
}
*/

