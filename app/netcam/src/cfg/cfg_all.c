
/*!
*****************************************************************************
** FileName     : cfg_all.c
**
** Description  : config api for all.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-1
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "cfg_all.h"

pthread_mutex_t g_cfg_wr_mutex;
static CFG_LIST_T *cfg_list = NULL;



static int is_sdcard_patch_exist()
{
    char sdk_cfg_patch_path[128] = {0};
    // 判断SDK_CFG_PATH是否存在
    snprintf(sdk_cfg_patch_path,sizeof(sdk_cfg_patch_path), "/mnt/sd_card/%s", SDK_CFG_CJSON_PATCH_FILE);
	if (access(sdk_cfg_patch_path, F_OK) == 0) {
	    return 1;
	}
	return 0;
}

int CfgSaveAll()
{
	CFG_LIST_T *node = NULL;
	
    SystemCfgSave();
    AudioCfgSave();
    VideoCfgSave();
    ImageCfgSave();
    ChannelCfgSave();
    NetworkCfgSave();
    MdCfgSave();
    SnapCfgSave();
    RecordCfgSave();
    AlarmCfgSave();
    PtzCfgSave();
    UserCfgSave();
    PresetCruiseCfgSave();
	pwmCfgSave();
	node = cfg_list;
	while(node != NULL)
	{
		if((node->privete_cfg.privete_cfg_save != NULL))
		{
			node->privete_cfg.privete_cfg_save();
		}
		node = node->next;
	}	
    return 0;
}

static void CheckCfgDir()
{
    if(access(CFG_DIR, R_OK))
    {
        if(mkdir(CFG_DIR, S_IRWXU | S_IRWXG| S_IROTH | S_IXOTH))
            PRINT_ERR("mkdir %s fail\n", CFG_DIR);
    }
}
int CfgLoadAll()
{
	CFG_LIST_T *node = NULL;

    CheckCfgDir();
    init_cfg_sdk();

    //add by bruce
    //StoreCfgLoad();

    SystemCfgLoad();
    #if 0
    //用户保存的store cfg，更新配置文件
    if (runStoreCfg.enable) {
        runSystemCfg.timezoneCfg.timezone = runStoreCfg.timezone;
        runSystemCfg.deviceInfo.languageType = runStoreCfg.languageType;
        PRINT_INFO("runStoreCfg, update timezone:%d, languageType:%d\n", runStoreCfg.timezone, runStoreCfg.languageType);
        SystemCfgSave();
    }
    #endif

    //sd 卡更新配置文件
    if (runSystemCfg.deviceInfo.languageType == -1 || is_sdcard_patch_exist()) {
        if (sdk_cfg.languageType != -1) {//FIXME(heyong): load sdk_cfg at first.
            runSystemCfg.deviceInfo.languageType = sdk_cfg.languageType;
            PRINT_INFO("update the language set by sdk_cfg %d.\n", sdk_cfg.languageType);
            SystemCfgSave();
        }
    }
    
    AudioCfgLoad();
    VideoCfgLoad();
    ImageCfgLoad();
    ChannelCfgLoad();
    NetworkCfgLoad();
    MdCfgLoad();
    SnapCfgLoad();
    RecordCfgLoad();
    AlarmCfgLoad();
    PtzCfgLoad();
    UserCfgLoad();
    PresetCruiseCfgLoad();
	pwmCfgLoad();	
	node = cfg_list;
	while(node != NULL)
	{
		if((node->privete_cfg.privete_cfg_Load != NULL))
		{
			node->privete_cfg.privete_cfg_Load();
		}
		node = node->next;
	}	
    return 0;
}



int CfgLoadDefValueAll()
{
	CFG_LIST_T *node = NULL;
	
    SystemCfgLoadDefValue();
    AudioCfgLoadDefValue();
    VideoCfgLoadDefValue();
    ImageCfgLoadDefValue();
    ChannelCfgLoadDefValue();
    NetworkCfgLoadDefValue();
    MdCfgLoadDefValue();
    SnapCfgLoadDefValue();
    RecordCfgLoadDefValue();
    AlarmCfgLoadDefValue();
    PtzCfgLoadDefValue();
    UserCfgLoadDefValue();
    PresetCruiseCfgDefault();
	pwmCfgLoadDefValue();
	node = cfg_list;
	while(node != NULL)
	{
		if((node->privete_cfg.privete_cfg_Load_def_value != NULL))
		{
			node->privete_cfg.privete_cfg_Load_def_value();
		}
		node = node->next;
	}

    return 0;
}

int CfgInit()
{
    pthread_mutex_init(&g_cfg_write_mutex,NULL);
    pthread_mutex_init(&g_cfg_wr_mutex,NULL);
    CfgLoadAll();

    return 0;
}

int CfgUnInit()
{
    CfgSaveAll();
    pthread_mutex_destroy(&g_cfg_wr_mutex);
    pthread_mutex_destroy(&g_cfg_write_mutex);

    return 0;
}


int get_param(int id, void *dest)
{
	CFG_LIST_T *node = NULL;
    pthread_mutex_lock(&g_cfg_wr_mutex);
    switch (id) {
        //GK_NET_SYSTEM_CFG runSystemCfg;
        case SYSTEM_PARAM_ID:
            memcpy(dest, &runSystemCfg, sizeof(GK_NET_SYSTEM_CFG));
            break;

        //GK_NET_AUDIO_CFG runAudioCfg;
        case AUDIO_PARAM_ID:
            memcpy(dest, &runAudioCfg, sizeof(GK_NET_AUDIO_CFG));
            break;

        //GK_NET_VIDEO_CFG runVideoCfg;
        case VIDEO_PARAM_ID:
            memcpy(dest, &runVideoCfg, sizeof(GK_NET_VIDEO_CFG));
            break;

        //GK_NET_IMAGE_CFG runImageCfg;
        case IMAGE_PARAM_ID:
            memcpy(dest, &runImageCfg, sizeof(GK_NET_IMAGE_CFG));
            break;

        //GK_NET_CHANNEL_CFG runChannelCfg;
        case CHANNEL_PARAM_ID:
            memcpy(dest, &runChannelCfg, sizeof(GK_NET_CHANNEL_CFG));
            break;

        //GK_NET_NETWORK_CFG runNetworkCfg;
        case NETWORK_PARAM_ID:
            memcpy(dest, &runNetworkCfg, sizeof(GK_NET_NETWORK_CFG));
            break;

        //GK_NET_MD_CFG runMdCfg;
        case MD_PARAM_ID:
            memcpy(dest, &runMdCfg, sizeof(GK_NET_MD_CFG));
            break;

        //GK_NET_SNAP_CFG runSnapCfg;
        case SNAP_PARAM_ID:
            memcpy(dest, &runSnapCfg, sizeof(GK_NET_SNAP_CFG));
            break;

        //GK_NET_RECORD_CFG runRecordCfg;
        case RECORD_PARAM_ID:
            memcpy(dest, &runRecordCfg, sizeof(GK_NET_RECORD_CFG));
            break;

        //GK_NET_ALARM_CFG runAlarmCfg;
        case ALARM_PARAM_ID:
            memcpy(dest, &runAlarmCfg, sizeof(GK_NET_ALARM_CFG));
            break;

        //GK_NET_PTZ_CFG runPtzCfg;
        case PTZ_PARAM_ID:
            memcpy(dest, &runPtzCfg, sizeof(GK_NET_PTZ_CFG));
            break;

        //GK_NET_USER_CFG runUserCfg;
        case USER_PARAM_ID:
            memcpy(dest, &runUserCfg, sizeof(GK_NET_USER_CFG));
            break;

        //GK_NET_PtzDec_CFG runRS232Cfg;
        case PTZ_DEC_PARAM_ID:
            memcpy(dest, &runPtzDecCfg, sizeof(GK_NET_DECODERCFG));
            break;

        //GK_NET_PtzPreset_CFG runPresetCfg;
        case PTZ_PRESET_PARAM_ID:
            memcpy(dest, &runPresetCfg, sizeof(GK_NET_PRESET_INFO));
            break;

        //GK_NET_PtzCruise_CFG runCruiseCfg;
        case PTZ_CRUISE_PARAM_ID:
            memcpy(dest, &runCruiseCfg, sizeof(GK_NET_CRUISE_CFG));
			break;
        default:
			node = cfg_list;
			while(node != NULL)
			{
				if((node->privete_cfg.CfgId == id) && (node->privete_cfg.privete_cfg_get_param != NULL))
				{
					node->privete_cfg.privete_cfg_get_param(dest);
				}
				node = node->next;
			}
            break;
    }

    pthread_mutex_unlock(&g_cfg_wr_mutex);
    return 0;
}

int set_param(int id, void *src)
{	
	CFG_LIST_T *node;
    pthread_mutex_lock(&g_cfg_wr_mutex);
    switch (id) {
        //GK_NET_SYSTEM_CFG runSystemCfg;
        case SYSTEM_PARAM_ID:
            memcpy(&runSystemCfg, src, sizeof(GK_NET_SYSTEM_CFG));
            break;

        //GK_NET_AUDIO_CFG runAudioCfg;
        case AUDIO_PARAM_ID:
            memcpy(&runAudioCfg, src, sizeof(GK_NET_AUDIO_CFG));
            break;

        //GK_NET_VIDEO_CFG runVideoCfg;
        case VIDEO_PARAM_ID:
            memcpy(&runVideoCfg, src, sizeof(GK_NET_VIDEO_CFG));
            break;

        //GK_NET_IMAGE_CFG runImageCfg;
        case IMAGE_PARAM_ID:
            memcpy(&runImageCfg, src, sizeof(GK_NET_IMAGE_CFG));
            break;

        //GK_NET_CHANNEL_CFG runChannelCfg;
        case CHANNEL_PARAM_ID:
            memcpy(&runChannelCfg, src, sizeof(GK_NET_CHANNEL_CFG));
            break;

        //GK_NET_NETWORK_CFG runNetworkCfg;
        case NETWORK_PARAM_ID:
            memcpy(&runNetworkCfg, src, sizeof(GK_NET_NETWORK_CFG));
            break;

        //GK_NET_MD_CFG runMdCfg;
        case MD_PARAM_ID:
            memcpy(&runMdCfg, src, sizeof(GK_NET_MD_CFG));
            break;

        //GK_NET_SNAP_CFG runSnapCfg;
        case SNAP_PARAM_ID:
            memcpy(&runSnapCfg, src, sizeof(GK_NET_SNAP_CFG));
            break;

        //GK_NET_RECORD_CFG runRecordCfg;
        case RECORD_PARAM_ID:
            memcpy(&runRecordCfg, src, sizeof(GK_NET_RECORD_CFG));
            break;

        //GK_NET_ALARM_CFG runAlarmCfg;
        case ALARM_PARAM_ID:
            memcpy(&runAlarmCfg, src, sizeof(GK_NET_ALARM_CFG));
            break;

        //GK_NET_PTZ_CFG runPtzCfg;
        case PTZ_PARAM_ID:
            memcpy(&runPtzCfg, src, sizeof(GK_NET_PTZ_CFG));
            break;

        //GK_NET_USER_CFG runUserCfg;
        case USER_PARAM_ID:
            memcpy(&runUserCfg, src, sizeof(GK_NET_USER_CFG));
            break;

        //GK_NET_PtzDec_CFG runRS232Cfg;
        case PTZ_DEC_PARAM_ID:
            memcpy(&runPtzDecCfg, src, sizeof(GK_NET_DECODERCFG));
            break;

        //GK_NET_PtzPreset_CFG runPresetCfg;
        case PTZ_PRESET_PARAM_ID:
            memcpy(&runPresetCfg, src, sizeof(GK_NET_PRESET_INFO));
            break;

        //GK_NET_PtzCruise_CFG runCruiseCfg;
        case PTZ_CRUISE_PARAM_ID:
            memcpy(&runCruiseCfg, src, sizeof(GK_NET_CRUISE_CFG));
            break;
        default:
			node = cfg_list;
			while(node != NULL)
			{
				if((node->privete_cfg.CfgId == id) && (node->privete_cfg.privete_cfg_get_param != NULL))
				{
					node->privete_cfg.privete_cfg_get_param(src);
				}
				node = node->next;
			}
            break;
    }

    pthread_mutex_unlock(&g_cfg_wr_mutex);
    return 0;
}

int schedule_time_to_slice(GK_SCHEDTIME  *s_time, SDK_U32 *s_slice)
{
    int i, j, k;
    int mask1, mask2;
    GK_SCHEDTIME  *stime = s_time;
    SDK_U32 *value = NULL;

    /* 对 slice 清0 */
    SDK_U32 *tmp = s_slice;
    for (i = 0; i < 7; i ++) {
        for (j = 0; j < 3; j ++) {
            *tmp = 0;
            tmp ++;
        }
    }
    
    for (i = 0; i < 7; i ++) {
        for (j = 0; j < 4; j ++) {
            value = s_slice + i * 3;
            mask1 = stime->startHour * 4 + (stime->startMin / 15);
            mask2 = stime->stopHour * 4 + (stime->stopMin / 15);

            //printf("\ni=%d j=%d, start:%d-%d stop:%d-%d\n", i, j, stime->startHour, stime->startMin, stime->stopHour, stime->stopMin);
            //printf("i=%d j=%d, mask1:%d mask2:%d\n\n", i, j, mask1, mask2);
            if ((mask1 == mask2) && (stime->startMin == stime->stopMin)) {
                //PRINT_INFO("jump.\n");
            } else if ((mask1 < mask2) || ((mask1 == mask2) && (stime->startMin < stime->stopMin))) {
                for (k = 0; k < 32; k ++) {
                    if ((k >= mask1) && (k <= mask2)) {
                        *value |= (1 << k);
                    }
                }
                for (k = 32; k < 64; k ++) {
                    if ((k >= mask1) && (k <= mask2)) {
                        *(value + 1) |= (1 << (k - 32));
                    }
                }
                for (k = 64; k < 96; k ++) {
                    if ((k >= mask1) && (k <= mask2)) {
                        *(value + 2) |= (1 << (k - 64));
                    }
                }
                //printf("1, scheduleSlice:%u\n", runRecordCfg.scheduleSlice[i][0]);
                //printf("2, scheduleSlice:%u\n", runRecordCfg.scheduleSlice[i][1]);
                //printf("3, scheduleSlice:%u\n\n", runRecordCfg.scheduleSlice[i][2]);
            } 
            else if ((mask1 > mask2) || ((mask1 == mask2) && (stime->startMin > stime->stopMin))) {
                for (k = 0; k <= mask2; k ++) {
                    if (k < 32) {
                        *value |= (1 << k);
                    } else if ((k >= 32) && (k < 64)) {
                        *(value + 1) |= (1 << (k - 32));
                    } else {
                        *(value + 2) |= (1 << (k - 64));
                    }
                }

                for (k = mask1; k < 96; k ++) {
                    if (k < 32) {
                        *value |= (1 << k);
                    } else if ((k >= 32) && (k < 64)) {
                        *(value + 1) |= (1 << (k - 32));
                    } else {
                        *(value + 2) |= (1 << (k - 64));
                    }
                }
                //printf("4, scheduleSlice:%u\n", runRecordCfg.scheduleSlice[i][0]);
                //printf("5, scheduleSlice:%u\n", runRecordCfg.scheduleSlice[i][1]);
                //printf("6, scheduleSlice:%u\n\n", runRecordCfg.scheduleSlice[i][2]);
            } else {
                PRINT_ERR("error!!! mask1:%d, mask2:%d, startMin:%d, stopMin:%d\n", mask1, mask2, stime->startMin, stime->stopMin);
            }
            /* 递增 */
            stime ++;
        }
    }


    return 0;
}
    

static CFG_LIST_T* cfg_list_create(void)
{
	CFG_LIST_T *head = NULL;

	head = malloc(sizeof(CFG_LIST_T));
	if(head)
	{
		memset(head, 0, sizeof(CFG_LIST_T));		
	}
	return head;
}

static int cfg_list_insert_tail(CFG_LIST_T *list, PRIVETE_CFG_T *privete_cfg)
{
	CFG_LIST_T *node = list, *in, *ptail_node;

	if(!list || !privete_cfg)
	{
		PRINT_ERR("list:%p, privete_cfg:%p\n", list, privete_cfg);
		return -1;
	}
	while(node != NULL)
	{
		ptail_node = node;
		node = node->next;
	}
	
	in = (CFG_LIST_T*)malloc(sizeof(CFG_LIST_T));

	if(in)
	{
		memcpy(&(in->privete_cfg), privete_cfg, sizeof(PRIVETE_CFG_T));
	    in->next = ptail_node->next;
	    ptail_node->next = in;
	}
	else
		PRINT_ERR("cfg_list_insert_tail malloc failed!\n");
	
	return 0;
}

void reg_privete_cfg_api(PRIVETE_CFG_T *privete_cfg)
{
	if(cfg_list == NULL)
	{
		cfg_list = cfg_list_create();
		if(cfg_list)
			memcpy(&(cfg_list->privete_cfg), privete_cfg, sizeof(PRIVETE_CFG_T));	
		else
			PRINT_ERR("reg_privete_cfg_api creat failed!\n");
	}
	else
	{
		cfg_list_insert_tail(cfg_list, privete_cfg);	
	}

}



