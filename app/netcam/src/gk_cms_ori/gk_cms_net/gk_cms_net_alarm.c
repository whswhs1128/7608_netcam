#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"


int Gk_CmsGetAlarmout(int sock)
{
    DMS_NET_ALARMOUTCFG alarmout;
	memset(&alarmout, 0, sizeof(DMS_NET_ALARMOUTCFG));
	alarmout.dwSize = sizeof(DMS_NET_ALARMOUTCFG);
    //todo

    GkCmsCmdResq(sock, (char *)&alarmout, sizeof(DMS_NET_ALARMOUTCFG), DMS_NET_GET_ALARMOUTCFG);
    return 0;
}

int Gk_CmsGetAlarmoutState(int sock)
{
    DMS_NET_SENSOR_STATE state;
	memset(&state, 0, sizeof(DMS_NET_SENSOR_STATE));
	state.dwSize = sizeof(DMS_NET_SENSOR_STATE);
    //todo


    GkCmsCmdResq(sock, (char *)&state, sizeof(DMS_NET_SENSOR_STATE), DMS_NET_GET_ALARMOUT_STATE);
    return 0;
}

int Gk_CmsGetAlarmin(int sock)
{
    DMS_NET_ALARMINCFG alarmin;
    memset(&alarmin, 0, sizeof(DMS_NET_ALARMINCFG));
    
    alarmin.dwSize = sizeof(DMS_NET_ALARMINCFG);
	strncpy(alarmin.csAlarmInName, runAlarmCfg.alarmIn.alarmInName, DMS_NAME_LEN-1);
	//PRINT_INFO("alarmin.csAlarmInName: %s\n", alarmin.csAlarmInName);
	//alarm type: manual switch
	alarmin.byAlarmType = 0;
	//1: need alarm or 0:not need alarm
	if(runAlarmCfg.alarmIn.handle.is_snap || runAlarmCfg.alarmIn.handle.is_beep)
		alarmin.byAlarmInHandle = 1;
	else
		alarmin.byAlarmInHandle = 0;
	//capabilities
    alarmin.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOSNAP;
    alarmin.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOBEEP;    
	//interval time: wDelayTime==wDuration==wRecTime==wSnapNum==wBuzzerTime, UI show wDuration.
	//PRINT_INFO("runAlarmCfg.alarmIn.handle.intervalTime: %d\n", runAlarmCfg.alarmIn.handle.intervalTime);
    alarmin.stHandle.wDuration = runAlarmCfg.alarmIn.handle.intervalTime;
	//need snap
	//PRINT_INFO("runAlarmCfg.alarmIn.handle.is_snap: %d\n", runAlarmCfg.alarmIn.handle.is_snap);
	if(runAlarmCfg.alarmIn.handle.is_snap == 1)
	{
		alarmin.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOSNAP;
		alarmin.stHandle.bySnap[0] = 0x01;
        alarmin.stHandle.wSnapNum = runAlarmCfg.alarmIn.handle.snapNum;

	}
	//need beep
	//PRINT_INFO("runAlarmCfg.alarmIn.handle.is_beep: %d\n", runAlarmCfg.alarmIn.handle.is_beep);
	if(runAlarmCfg.alarmIn.handle.is_beep == 1)
	{
        alarmin.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOBEEP;
        alarmin.stHandle.wBuzzerTime = runAlarmCfg.alarmIn.handle.beepTime;
	}
	//Schedule Time
    int i = 0;
    int j = 0;
    for(i = 0; i < 7; i++) 
    {
        for(j = 0; j < 4; j++)
        {
            alarmin.stScheduleTime[i][j].byStartHour = runAlarmCfg.alarmIn.scheduleTime[i][j].startHour;
            alarmin.stScheduleTime[i][j].byStartMin  = runAlarmCfg.alarmIn.scheduleTime[i][j].startMin;
            alarmin.stScheduleTime[i][j].byStopHour  = runAlarmCfg.alarmIn.scheduleTime[i][j].stopHour;
            alarmin.stScheduleTime[i][j].byStopMin   = runAlarmCfg.alarmIn.scheduleTime[i][j].stopMin;
        }
    }
  
    GkCmsCmdResq(sock, (char *)&alarmin, sizeof(DMS_NET_ALARMINCFG), DMS_NET_GET_ALARMINCFG);
    return 0;
}

int Gk_CmsSetAlarmout(int sock)
{
    DMS_NET_ALARMOUTCFG alarmout;
    int ret = GkSockRecv(sock, (char *)&alarmout, sizeof(DMS_NET_ALARMOUTCFG));
    if (ret != sizeof(DMS_NET_ALARMOUTCFG)) 
    {
        PRINT_ERR();
        return -1;
    }

    //todo

    return 0;
}

int Gk_CmsAlarmoutControl(int sock)
{
    DMS_NET_ALARMOUT_CONTROL alarmout_control;
    int ret = GkSockRecv(sock, (char *)&alarmout_control, sizeof(DMS_NET_ALARMOUT_CONTROL));
    if (ret != sizeof(DMS_NET_ALARMOUT_CONTROL)) 
    {
        PRINT_ERR();
        return -1;
    }

    //todo

    return 0;
}


int Gk_CmsSetAlarmin(int sock)
{
    DMS_NET_ALARMINCFG alarmin;
    int ret = GkSockRecv(sock, (char *)&alarmin, sizeof(DMS_NET_ALARMINCFG));
    if(ret != sizeof(DMS_NET_ALARMINCFG))
    {
        PRINT_ERR();
        return -1;
    }
	//PRINT_INFO("csAlarmInName: %s\nbyAlarmType: %d\nbyAlarmInHandle: %d\n", 
		//alarmin.csAlarmInName, alarmin.byAlarmType, alarmin.byAlarmInHandle);
	if(alarmin.byAlarmInHandle)
	{
		//snap
	    if((alarmin.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOSNAP) == GK_ALARM_EXCEPTION_TOSNAP) 
	    {
	        runAlarmCfg.alarmIn.handle.is_snap = 1;
	        runAlarmCfg.alarmIn.handle.snapNum = alarmin.stHandle.wSnapNum;
	    } 
	    else
	    {   
	        runAlarmCfg.alarmIn.handle.is_snap = 0;
	    }
		//beep
	    if((alarmin.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOBEEP) == GK_ALARM_EXCEPTION_TOBEEP)
	    {
	        runAlarmCfg.alarmIn.handle.is_beep  = 1;
	        runAlarmCfg.alarmIn.handle.beepTime = alarmin.stHandle.wBuzzerTime;
	    } 
	    else 
	    {
	        runAlarmCfg.alarmIn.handle.is_beep  = 0;
	    }
		//interval time: wDelayTime==wDuration==wRecTime==wSnapNum==wBuzzerTime, UI show wDuration.
		//PRINT_INFO("alarmin.stHandle.wDelayTime: %d\n", alarmin.stHandle.wDelayTime);
		runAlarmCfg.alarmIn.handle.intervalTime = alarmin.stHandle.wDelayTime;
	    //Schedule time: schedule time had not implemented at this moment, it should be implemented.
	    int i = 0;
	    int j = 0;
	    for(i = 0; i < 7; i++) 
	    {
	        for(j = 0; j < 4; j++)
	        {
	            runAlarmCfg.alarmIn.scheduleTime[i][j].startHour = (SDK_S32)alarmin.stScheduleTime[i][j].byStartHour;
	            runAlarmCfg.alarmIn.scheduleTime[i][j].startMin  = (SDK_S32)alarmin.stScheduleTime[i][j].byStartMin;
	            runAlarmCfg.alarmIn.scheduleTime[i][j].stopHour  = (SDK_S32)alarmin.stScheduleTime[i][j].byStopHour;
	            runAlarmCfg.alarmIn.scheduleTime[i][j].stopMin   = (SDK_S32)alarmin.stScheduleTime[i][j].byStopMin;
				//PRINT_INFO("%d:%d:%d-%d:%d", i, 
					//(SDK_S32)alarmin.stScheduleTime[i][j].byStartHour, (SDK_S32)alarmin.stScheduleTime[i][j].byStartMin,
					//(SDK_S32)alarmin.stScheduleTime[i][j].byStopHour, (SDK_S32)alarmin.stScheduleTime[i][j].byStopMin);
	        }
	    }
    }
	else
	{
		runAlarmCfg.alarmIn.handle.is_snap = 0;
		runAlarmCfg.alarmIn.handle.is_beep = 0;
	}
    return 0;
}

#ifdef MODULE_SUPPORT_RF433
#include "rf433.h"
int gk_cms_rf433_resp_sock;
int Gk_Cms_433dev_modifyname(int sock)
{
	GkCmsSet433Req streq;
	GkCmsSet433Resp resp = {-1,0};
	int ret = GkSockRecv(sock, (char *)&streq, sizeof(GkCmsSet433Req));
    if (ret == -1) {
        PRINT_ERR();
        return -1;
    }
	
	PRINT_INFO("modify 433 code,id=%d;name=%s\r\n",streq.id,streq.name);
	resp.result = rf433_modify_code_name(streq.id,streq.name);
	GkCmsCmdResq(sock, (char *)&resp, sizeof(GkCmsSet433Resp), DMS_RF433_DEV_MODIFY_NAME);
	return 0;
}


int Gk_Cms_433dev_getlist(int sock)
{
	GkCmsGet433Resp resp = {0};
	resp.num = rf433_get_dev_num();
	PRINT_INFO("433 code num=%d\r\n",resp.num);
	int ii;
	for(ii = 0;ii < resp.num;ii++)
	{
		rf433_get_dev_corresponding_info(ii,&(resp.dev[ii].id),&(resp.dev[ii].type),resp.dev[ii].name);
		PRINT_INFO("seq:%d,id:%d,type:%d,name:%s",ii,resp.dev[ii].id,resp.dev[ii].type,resp.dev[ii].name);
	}
	GkCmsCmdResq(sock, (char *)&resp, sizeof(GkCmsGet433Resp), DMS_RF433_DEV_GET_DEV_LIST);
	return 0;
}
int Gk_Cms_433dev_learncode(int sock)
{
	GkCmsCfg433Req streq;
	GkCmsCfg433Resp resp = {-1,-1};
	int ret = GkSockRecv(sock, (char *)&streq, sizeof(GkCmsCfg433Req));
    if (ret == -1) {
        PRINT_ERR();
		gk_cms_rf433_resp_sock = -1;
        return -1;
    }
	gk_cms_rf433_resp_sock = sock;
	PRINT_INFO("learn code;type=%d;name=%s\r\n",streq.type,streq.name);
	if(streq.name)
	{
		resp.result = AVIOCTRL_CFG_433_WAITING;
		rf433_learn_code(streq.type,streq.name);
	}
	else
	{
		resp.result = AVIOCTRL_CFG_433_ERROR;
	}
	//GkCmsCmdResq(sock, (char *)&resp, sizeof(GkCmsCfg433Resp), DMS_RF433_DEV_LEARN_CODE);
	return 0;
}
int Gk_Cms_433dev_deletecode(int sock)
{
	GkCmsDel433Req streq;
	GkCmsDel433Resp resp = {-1,-1};
	int ret = GkSockRecv(sock, (char *)&streq, sizeof(GkCmsDel433Req));
    if (ret == -1) {
        PRINT_ERR();
        return -1;
    }
	resp.result = 0;
	PRINT_INFO("delete code;id=%d\r\n",streq.id);
	rf433_delete_code(streq.id);
	GkCmsCmdResq(sock, (char *)&resp, sizeof(GkCmsDel433Resp), DMS_RF433_DEV_DELETE_DEV);
	return 0;
}
int Gk_Cms_433dev_exit_learncode(int sock)
{
	GkCmsCfg433ExitResp resp = {-1,-1};
	resp.result = 0;
	PRINT_INFO("exit learn code\r\n");
	rf433_exit_learn();
	GkCmsCmdResq(sock, (char *)&resp, sizeof(GkCmsCfg433ExitResp), DMS_RF433_DEV_EXIT_LEARN_CODE);
	return 0;
}
void gk_cms_rf433_learn_handle(int flag)
{
	GkCmsCfg433Resp resp = {-1,-1};
	PRINT_INFO("rf433_learn_handle:flag=%d\n",flag);
	resp.result = flag;
	if(flag == AVIOCTRL_CFG_433_WAITING) return;
	if(gk_cms_rf433_resp_sock > 0)
	{
		GkCmsCmdResq(gk_cms_rf433_resp_sock, (char *)&resp, sizeof(GkCmsCfg433Resp), DMS_RF433_DEV_LEARN_CODE);
		gk_cms_rf433_resp_sock = -1;
	}
}
void gk_cms_rf433_event_handle(int areaid,int type,int code)
{
	PRINT_INFO("areaid=%d,type=%d,code=0x%x\n",areaid,type,code);
	PRINT_INFO("rf433 event.\n");
	if(RF433_DEV_TYPE_CONTROL == rf433_get_dev_type(areaid))
	{
		if(code & RF433_KEY_SOS)
		{
			goto ENTER_ALARM_HANDLE;
		}
		else if(code & RF433_KEY_DEFENCE)
		{
			runMdCfg.enable = 1;
			return;
		}
		else if(code & RF433_KEY_UNDEFENCE)
		{
			runMdCfg.enable = 0;
			return;
		}
		else
		{
			return;
		}
	}
	else if(RF433_DEV_TYPE_ALARM == rf433_get_dev_type(areaid))
	{
		
	}
	else
	{
		return;
	}
	if(!runMdCfg.enable) return ;
ENTER_ALARM_HANDLE:
	cms_notify_to_all_client(JB_MSG_RF433_DEV_ALARM_BASENUM + areaid);
}
#endif


