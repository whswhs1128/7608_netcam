#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <csl_sem.h>
#include <cJSON.h>
#include <goolink_sesapi.h>
#include <goolink_apiv2.h>
#include "goolink_cc_switch.h"
#include "transparent.h"
#include "debug.h"
#include <HieServer_Cfg.h>

#include "netcam_api.h"


#define NET_LED_GPIO 13
int nTimeOutMS = TIMEOUT_IDLE; //太长了传输会很慢; 太短了吃cpu性能也不可取
STRU_CONN_INFO s_hOldConnID[MAX_CONN_NUM] = {0};
static int led_ctl_enable = 1;

void sysUsecTime(struct tm * ptm);
/*
1 通过串口跟CC1310通信
2 通过glnk上报CC接口信息
*/
CMutex s_hSensorDataMutex;

USERDEF_CMD struUserType[] = { //glnk的字符串和命令对应表, 这些是命令头部
    {USRDEF_CC_LIST,    KEY_CC_LIST},
    {USRDEF_CC_ADD,     KEY_CC_ADD},
    {USRDEF_CC_DEL,     KEY_CC_DEL},
    {USRDEF_CC_ALARM,   KEY_CC_ALARM},
    {USRDEF_CC_WAKEUP,  KEY_CC_WAKEUP},
    {USRDEF_CC_SETWIFI, KEY_CC_SETWIFI},
    {USRDEF_PIR,        KEY_PIR},
    {USRDEF_ALARM_ACTION, KEY_ALARM_ACTION},
    {USRDEF_HUMANTRACE, KEY_HUMANTRACE},
    {USRDEF_NETLED,     KEY_NETLED},
};
static char s_SesBufRecvIn[SES_BUF_SIZE]; //接收远程glnk过来的自定义命令
char s_SesBufSendOut[SES_BUF_SIZE]; //返回内容给远程app
SENSOR_NODE struSenseorNode[MAX_SENSOR_NODE]; //设备本地记录cc传感器详情

int MPU_SetAutoHumanTrace(int bEnable) //下发自动人形追踪开关
{
	runMdCfg.handle.bCheckedFigure = bEnable;
	MdCfgSave();
}

int MPU_GetAutoHumanTrace()
{
	return runMdCfg.handle.bCheckedFigure;
}

int MPU_GetNetLed()
{
	if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10"))
	{
		int ledEnableValue =0;
		if (access("/opt/custom/cfg/blueLed.conf", R_OK) != 0)
		{
			ledEnableValue =1;
			return ledEnableValue;
		}
		FILE *ledFp = NULL;

		ledFp = fopen("/opt/custom/cfg/blueLed.conf","r");
		if (NULL == ledFp)
		{
			ledEnableValue =1;
			return ledEnableValue;
		}
		fseek(ledFp,0L,SEEK_SET);

		char buf[6]={0};
		int readCnt = fread(buf,sizeof(buf),1,ledFp);  /* 返回值为0 */
		fclose(ledFp);

		if (strcmp(buf,"false") == 0)
			ledEnableValue = 0;
		else if (strcmp(buf,"true") == 0)
			ledEnableValue = 1;

		return ledEnableValue;

	}
	else
	{

		return glnk_led_control_state();

	}
}

int MPU_SetNetLed(int bshow)
{
	if (bshow)
	{
		led_ctl_enable = 1;
	}
	else
	{
		led_ctl_enable = 0;
	}

	if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10"))
		sdk_gpio_set_output_value(NET_LED_GPIO, bshow);
}

int glnk_led_control_state(void)
{
	return led_ctl_enable;
}

int parse_json_cmdtype(char * pbuf, USERDEF_CMD * pCmd, int nTotalCmd) //根据传过来的字符串找到对应的命令是什么
{
    JS_HEAP_OBJ_MGR heapobjmgr = {0};
    //这里不会直接比较头, 而是解析一下再比较
    int rettype = -1, i = 0;
    //取json字符串的头, 根据头分流命令
    cJSON* root  = cJSON_ParseEx(&heapobjmgr, pbuf);
    if (NULL == root){
        return -1;
    }
    printf("1.app_orignal_cmd:\n%s\n", JsPrintFmt(root));
    cJSON * jshead = cJSON_GetArrayItem(root, 0); //取下标第一个元素的内容
    if (NULL == jshead){
        return -1;
    }
    char *pszRootKey = jshead->string;
    char *pszRootValue = jshead->valuestring;
    printf("parse_cmdtype, pszRoot=%s, pszRootValue=%d \n", pszRootKey, pszRootValue);

    for (i = 0; i< nTotalCmd; i++){
        printf("parse_cmdtype %s  %s \n", pszRootKey, pCmd[i].szName);
        if (0 == strncmp(pszRootKey, pCmd[i].szName, strlen(pCmd[i].szName))){
            rettype = pCmd[i].iType;
            printf("parse_cmdtype, rettype=%d, protocol_name=%s \n", rettype, pCmd[i].szName);
            break;
        }
    }
    JsHeapObjFree(&heapobjmgr);
    return rettype;
}

int parse_find_item_idindex(char *pRecvIn)
{
    JS_HEAP_OBJ_MGR heapobjmgr = {0};
    int i = 0, ifinditem = 0;
    cJSON * root  = cJSON_ParseEx(&heapobjmgr, pRecvIn);
    //printf("1.orignal_app_cmd:\n%s \n",  cJSON_Print(root));
    cJSON * jshead = cJSON_GetArrayItem(root, 0); //取下标第一个元素的key对应的value内容
    cJSON * itemSensorId = cJSON_GetObjectItem(jshead, KEY_SENSORID);//从第一个元素中找到id
    //printf("find_item 1111 %s \n",  cJSON_Print(jshead));
    printf("find_item 2222 %s \n",  JsPrintFmt(itemSensorId));

    for ( i = 0; i<MAX_SENSOR_NODE; i++){ //遍历所有sensor节点, 将可用的sensor信息全部打印到sendbuf上
        if (0 == strncmp(itemSensorId->valuestring, struSenseorNode[i].id, strlen(struSenseorNode[i].id))){
            ifinditem = i;
            printf("find_item, ifinditem=%d \n", ifinditem);
            break;
        }
    }
    if (ifinditem >= MAX_SENSOR_NODE){ //溢出没找到
        ifinditem = -1;
        printf("glnk_delete_macid, could not find deleteid:%s \n", itemSensorId->valuestring);
    }
    //printf("finally, ifinditem=%d\n", ifinditem);
    JsHeapObjFree(&heapobjmgr);
    return ifinditem;
}

void reply_to_app_write(const char * szCmdType, int ifinditem, char * pBufOutput)
{
    JS_HEAP_OBJ_MGR heapobjmgr = {0};
    cJSON* VideoRecordroot = cJSON_CreateObjectEx(&heapobjmgr);
    if (0==strcmp(szCmdType, KEY_CC_ADD) || 0==strcmp(szCmdType, KEY_CC_DEL)){
        cJSON* JsSensorNode    = cJSON_CreateObjectEx(&heapobjmgr);
        if (ifinditem > -1){//id可以找到
            cJSON_AddStringToObject(JsSensorNode, KEY_SENSORID,     struSenseorNode[ifinditem].id);
            cJSON_AddStringToObject(JsSensorNode, KEY_SENSORLOC,    struSenseorNode[ifinditem].szLocation);
            cJSON_AddNumberToObject(JsSensorNode, KEY_ACCEPT,       struSenseorNode[ifinditem].bAccepted);
            cJSON_AddNumberToObject(JsSensorNode, KEY_HANDLEBYAPP,  struSenseorNode[ifinditem].bHandledByApp);
        }
        else{ //id不存在, 通知给app没有找到
            cJSON_AddStringToObject(JsSensorNode, KEY_SENSORID,     "null");
        }
        cJSON_AddItemReferenceToObject(VideoRecordroot, szCmdType, JsSensorNode);
    }
    else if (0==strcmp(szCmdType, KEY_CC_LIST)){
        cJSON* JsSensorArray   = cJSON_CreateArrayEx(&heapobjmgr);
        for (int i = 0; i< MAX_SENSOR_NODE; i++){
            cJSON* JsSensorNode    = cJSON_CreateObjectEx(&heapobjmgr);
            cJSON_AddStringToObject(JsSensorNode, KEY_SENSORID,     struSenseorNode[i].id);
            cJSON_AddStringToObject(JsSensorNode, KEY_SENSORLOC,    struSenseorNode[i].szLocation);
            cJSON_AddNumberToObject(JsSensorNode, KEY_ACCEPT,       struSenseorNode[i].bAccepted);
            cJSON_AddNumberToObject(JsSensorNode, KEY_HANDLEBYAPP,  struSenseorNode[i].bHandledByApp);
            cJSON_AddNumberToObject(JsSensorNode, KEY_ALARMING,     struSenseorNode[i].bAlarming);
            cJSON_AddStringToObject(JsSensorNode, KEY_TYPE,         struSenseorNode[i].sztype); //A是震动传感器, B是室外机
            cJSON_AddItemReferenceToArray(JsSensorArray, JsSensorNode);
        }
        cJSON_AddItemReferenceToObject(VideoRecordroot, KEY_CC_LIST, JsSensorArray);
    }
    else if (0==strcmp(szCmdType, KEY_HUMANTRACE)){
        printf("reply_to_app_write, KEY_HUMANTRACE ===================================== \n ");
        cJSON* JsNode   = cJSON_CreateObjectEx(&heapobjmgr);
        int bhuman_switch = MPU_GetAutoHumanTrace();
        cJSON_AddNumberToObject(JsNode, KEY_HT_SWITCH, bhuman_switch); //中间的字段
        cJSON_AddItemReferenceToObject(VideoRecordroot, KEY_HUMANTRACE, JsNode); //协议头
    }
    else if (0==strcmp(szCmdType, KEY_NETLED)){ //20190225
        printf("reply_to_app_write, KEY_NETSTATUSLED >>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n ");
        cJSON* JsNode   = cJSON_CreateObjectEx(&heapobjmgr);
        int bnetled_switch = MPU_GetNetLed(); //获取底层开关状态, 这里要纠正
        cJSON_AddNumberToObject(JsNode, KEY_HT_SWITCH, bnetled_switch); //中间的字段
        cJSON_AddItemReferenceToObject(VideoRecordroot, KEY_NETLED, JsNode); //协议头
        printf("reply_to_app_write, KEY_NETSTATUSLED %d<<<<<<<<<<<<<<<<<<<<<<<<<<< \n ",bnetled_switch);
    }
    else {
        printf("reply_to_app_write, protocol %s is not existed \n", szCmdType);
        return;
    }

    if(pBufOutput){
        JsToBuffer(VideoRecordroot, pBufOutput);
    }

    JsHeapObjFree(&heapobjmgr);
}


int glnk_humantrace()//这里只填充发送缓冲, 交给别人发送
{
    reply_to_app_write(KEY_HUMANTRACE, 0, s_SesBufSendOut);
}

int glnk_netstatusled()//这里只填充发送缓冲, 交给别人发送
{
    reply_to_app_write(KEY_NETLED, 0, s_SesBufSendOut);
}

//=====================================alarm_action========================================
//20181015 新款报警联动开关
#define ACTION_TIME_SEGNUM 4
typedef struct STRU_ACTIONFILTER_TIMESEG{ //在该时间段中是否勾选
    int StartTime;
    int EndTime;
    int ActionMask; //通过mask来指定联动的动作, 好扩展, 否则以后可能都不兼容了
}STRU_ACTIONFILTER_TIMESEG;

typedef struct STRU_ALARM_ACTION_INFO {
    //报警源
    int bCheckedPIR;
    int bCheckedMotion;
    int nMotionsenstive; //移动侦测灵敏度

    int bCheckedFigure; //3 是否开启人形检测过滤
    int StayDelayTime; //刚开始逗留不报警的时间, 超过这个时间就要开始报警了

    //联动动作
    STRU_ACTIONFILTER_TIMESEG struTimeSeg[ACTION_TIME_SEGNUM]; //默认给4个时间段 //20181015修改成带时间段的

    //自定义选择何种语音报警
    int AlarmAudioType; //1: 默认语音1; 2: 默认语音2; 10: 自定义;
}STRU_ALARM_ACTION_INFO;

/////////////////////////////////////////////////////////////////////////////////
int MPU_Get_Alarm_Action(STRU_ALARM_ACTION_INFO * pStruAlarmSrcAction)
{
	printf("MPU_Get_Alarm_Action\n");

	int sensi;
	int retSensi = 0;

	sensi = runMdCfg.sensitive;
	if(sensi <= 25)
	{
		retSensi = 1;
	}
	else if(sensi <= 50)
	{
		retSensi = 2;
	}
	else if(sensi <= 75)
	{
		retSensi = 3;
	}
	else if(sensi < 100)
	{
		retSensi = 4;
	}
	else
	{
		retSensi = 4;
	}

	pStruAlarmSrcAction->bCheckedMotion = runMdCfg.enable;

	pStruAlarmSrcAction->nMotionsenstive = retSensi;
	pStruAlarmSrcAction->StayDelayTime = runMdCfg.handle.intervalTime;
	pStruAlarmSrcAction->AlarmAudioType = runP2PCfg.alarmAudioType;//runMdCfg.handle.alarmAudioType;
	pStruAlarmSrcAction->bCheckedFigure = runMdCfg.handle.bCheckedFigure;

	for (int i = 0; i<ACTION_TIME_SEGNUM; i++)
	{
#if 0
		if(	   (runMdCfg.scheduleTime[0][i].startHour != runMdCfg.scheduleTime[0][i].stopHour)
			|| (runMdCfg.scheduleTime[0][i].startMin != runMdCfg.scheduleTime[0][i].stopMin))
#endif
		{
    	    pStruAlarmSrcAction->struTimeSeg[i].StartTime = runMdCfg.scheduleTime[0][i].startHour*60 + runMdCfg.scheduleTime[0][i].startMin;
			pStruAlarmSrcAction->struTimeSeg[i].EndTime = runMdCfg.scheduleTime[0][i].stopHour*60 + runMdCfg.scheduleTime[0][i].stopMin;
			printf("pStruAlarmSrcAction->struTimeSeg[%d].StartTime = %d\n", i, pStruAlarmSrcAction->struTimeSeg[i].StartTime);
			printf("pStruAlarmSrcAction->struTimeSeg[%d].EndTime = %d\n", i, pStruAlarmSrcAction->struTimeSeg[i].EndTime);
			if ((i>0) && (pStruAlarmSrcAction->struTimeSeg[i].StartTime == pStruAlarmSrcAction->struTimeSeg[0].StartTime))
			{
				pStruAlarmSrcAction->struTimeSeg[i].StartTime = 1440;
				pStruAlarmSrcAction->struTimeSeg[i].EndTime = 1440;
			}
			//runMdCfg.handle.is_beep[i]?(pStruAlarmSrcAction->struTimeSeg[i].ActionMask |= DVR_ALARM_TRIGGER_AUDIOOUT):(pStruAlarmSrcAction->struTimeSeg[i].ActionMask &= ~DVR_ALARM_TRIGGER_AUDIOOUT);
			runP2PCfg.is_beep[i]?(pStruAlarmSrcAction->struTimeSeg[i].ActionMask |= DVR_ALARM_TRIGGER_AUDIOOUT):(pStruAlarmSrcAction->struTimeSeg[i].ActionMask &= ~DVR_ALARM_TRIGGER_AUDIOOUT);
			runMdCfg.handle.is_rec?(pStruAlarmSrcAction->struTimeSeg[i].ActionMask |= DVR_ALARM_TRIGGER_RECORD):(pStruAlarmSrcAction->struTimeSeg[i].ActionMask &= ~DVR_ALARM_TRIGGER_RECORD);
			runMdCfg.handle.is_light[i]?(pStruAlarmSrcAction->struTimeSeg[i].ActionMask |= DVR_ALARM_TRIGGER_LIGHT):(pStruAlarmSrcAction->struTimeSeg[i].ActionMask &= ~DVR_ALARM_TRIGGER_LIGHT);
			runMdCfg.handle.is_pushMsg?(pStruAlarmSrcAction->struTimeSeg[i].ActionMask |= DVR_ALARM_TRIGGER_NOTIFYAMS):(pStruAlarmSrcAction->struTimeSeg[i].ActionMask &= ~DVR_ALARM_TRIGGER_NOTIFYAMS);
		}
	}
}

int MPU_Set_Alarm_Action(STRU_ALARM_ACTION_INFO * pStruAlarmSrcAction)
{
	printf("MPU_Set_Alarm_Action:%d\n", pStruAlarmSrcAction->nMotionsenstive);

	int pirSensi = 0;

	STRU_ALARM_ACTION_INFO *p = (STRU_ALARM_ACTION_INFO *)pStruAlarmSrcAction;
	STRU_ALARM_ACTION_INFO resp = {0} ;
	memcpy(&resp,p,sizeof(STRU_ALARM_ACTION_INFO));

	runMdCfg.enable = p->bCheckedMotion;

	pirSensi = SYS_MD_SENSITIVITY_HIGHEST + 1 - p->nMotionsenstive;
	netcam_md_set_sensitivity(0,(SYS_MD_SENSITIVITY_t)pirSensi);

	runMdCfg.handle.intervalTime = p->StayDelayTime;
	runP2PCfg.alarmAudioType = p->AlarmAudioType;

	runMdCfg.handle.recStreamNo = 1;
	runMdCfg.handle.recTime = 1;

	runMdCfg.sensitive = p->nMotionsenstive*20;

	runMdCfg.handle.is_rec = (p->struTimeSeg[0].ActionMask & DVR_ALARM_TRIGGER_RECORD)? 1:0;
	runMdCfg.handle.is_pushMsg = (p->struTimeSeg[0].ActionMask & DVR_ALARM_TRIGGER_NOTIFYAMS)? 1:0;

	int i = 0,j = 0;
	//for (i = 0; i < 7; i ++)
	{
		if(p->struTimeSeg[0].StartTime > p->struTimeSeg[0].EndTime)
		{
			runMdCfg.scheduleTime[i][0].startHour = 0;
			runMdCfg.scheduleTime[i][0].startMin = 0;
			runMdCfg.scheduleTime[i][0].stopHour = p->struTimeSeg[0].EndTime/60;
			runMdCfg.scheduleTime[i][0].stopMin = p->struTimeSeg[0].EndTime%60;
			//runMdCfg.handle.is_beep[0] = (p->struTimeSeg[0].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
			runP2PCfg.is_beep[0] = (p->struTimeSeg[0].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
			runMdCfg.handle.is_light[0] = (p->struTimeSeg[0].ActionMask & DVR_ALARM_TRIGGER_LIGHT)? 1:0;

			runMdCfg.scheduleTime[i][1].startHour = p->struTimeSeg[0].StartTime/60;
			runMdCfg.scheduleTime[i][1].startMin = p->struTimeSeg[0].StartTime%60;
			runMdCfg.scheduleTime[i][1].stopHour = 24;
			runMdCfg.scheduleTime[i][1].stopMin = 0;
			//runMdCfg.handle.is_beep[1] = (p->struTimeSeg[1].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
			runP2PCfg.is_beep[1] = (p->struTimeSeg[1].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
			runMdCfg.handle.is_light[1] = (p->struTimeSeg[1].ActionMask & DVR_ALARM_TRIGGER_LIGHT)? 1:0;

			runMdCfg.scheduleTime[i][2].startHour = 24;
			runMdCfg.scheduleTime[i][2].startMin = 0;
			runMdCfg.scheduleTime[i][2].stopHour = 24;
			runMdCfg.scheduleTime[i][2].stopMin = 0;
			//runMdCfg.handle.is_beep[2] = (p->struTimeSeg[2].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
			runP2PCfg.is_beep[2] = (p->struTimeSeg[2].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
			runMdCfg.handle.is_light[2] = (p->struTimeSeg[2].ActionMask & DVR_ALARM_TRIGGER_LIGHT)? 1:0;

			runMdCfg.scheduleTime[i][3].startHour = 24;
			runMdCfg.scheduleTime[i][3].startMin = 0;
			runMdCfg.scheduleTime[i][3].stopHour = 24;
			runMdCfg.scheduleTime[i][3].stopMin = 0;
			//runMdCfg.handle.is_beep[3] = (p->struTimeSeg[3].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
			runP2PCfg.is_beep[3]  = (p->struTimeSeg[3].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
			runMdCfg.handle.is_light[3] = (p->struTimeSeg[3].ActionMask & DVR_ALARM_TRIGGER_LIGHT)? 1:0;
		}
		else
		{
			for (j = 0; j < 4; j ++)
			{
				runMdCfg.scheduleTime[i][j].startHour = p->struTimeSeg[j].StartTime/60;
				runMdCfg.scheduleTime[i][j].startMin = p->struTimeSeg[j].StartTime%60;
				runMdCfg.scheduleTime[i][j].stopHour = p->struTimeSeg[j].EndTime/60;
				runMdCfg.scheduleTime[i][j].stopMin = p->struTimeSeg[j].EndTime%60;

				//runMdCfg.handle.is_beep[j] = (p->struTimeSeg[j].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
				runP2PCfg.is_beep[j] = (p->struTimeSeg[j].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0;
				runMdCfg.handle.is_light[j] = (p->struTimeSeg[j].ActionMask & DVR_ALARM_TRIGGER_LIGHT)? 1:0;
			}
		}
	}

	MdCfgSave();
	P2PCfgSave();
}


int glnk_alarm_action(char *pRecvIn)
{
    JS_HEAP_OBJ_MGR heapobjmgr = {0};

    STRU_ALARM_ACTION_INFO struAAinfo;
    memset(&struAAinfo, 0, sizeof(STRU_ALARM_ACTION_INFO));
    cJSON * root  = cJSON_ParseEx(&heapobjmgr, pRecvIn);
    if (NULL == root){
        printf("glnk_alarm_action, 1111  return \n");
        return -1;
    }
    char *pszStr = JsPrintFmt(root);
    if (NULL == pszStr){
        printf("glnk_alarm_action, 11112222  return \n");
        return -1;
    }
    GDEBUG("1.orignal_app_cmd:\n%s \n", pszStr);
    cJSON * jshead = cJSON_GetArrayItem(root, 0); //取下标第一个元素的key对应的value的所有内容
    if (NULL == jshead){
        printf("glnk_alarm_action, 2222 return \n");
        return -1;
    }
    jshead = cJSON_GetArrayItem(jshead, 0); //取下一级的全部内容
    //GDEBUG("find_item 1111:\n%s \n jshead->string =%s \n",  cJSON_Print(jshead), jshead->string);
    if (0 == strncmp(jshead->string, KEY_OP_SET, strlen(KEY_OP_SET))){ //此命令是set
        /*
        app发送:
        {"alarm_action":{"set" : { "PirEnable":1, "MotionEnable":1, "MotionSensitive": 2, "FigureFilterEnable":1, "ActionRecord":1, "ActionSound":1, "ActionPushup":1}}}
        dev返回:
        {"alarm_action":  "result": 1}
        */
        //接下来取下标
        cJSON * param1 = NULL;
        param1 = cJSON_GetObjectItem(jshead, KEY_AA_PIRENABLE);//从第一个元素中找到id
        if (param1){
            GDEBUG("filed1: %s, value=%s, %d \n",  JsPrintFmt(param1), param1->string, param1->valueint);
            struAAinfo.bCheckedPIR = param1->valueint;
        }

        //移动侦测
        param1 = cJSON_GetObjectItem(jshead, KEY_AA_MOTIONENABLE);//从第一个元素中找到id
        if (param1){
            GDEBUG("filed1: %s, value=%s, %d \n",  JsPrintFmt(param1), param1->string, param1->valueint);
            struAAinfo.bCheckedMotion  = param1->valueint;
        }

        //移动侦测灵敏度
        param1 = cJSON_GetObjectItem(jshead, KEY_AA_MOTIONSENSITIVE);
        if (param1){
            GDEBUG("filed1: %s, value=%s, %d \n",  JsPrintFmt(param1), param1->string, param1->valueint);
            struAAinfo.nMotionsenstive = param1->valueint;
        }

        //人形检测
        param1 = cJSON_GetObjectItem(jshead, KEY_AA_FIGUREPILTERENABLE);
        if (param1){
            GDEBUG("filed1: %s, value=%s, %d \n",  JsPrintFmt(param1), param1->string, param1->valueint);
             struAAinfo.bCheckedFigure = param1->valueint;
        }

        //逗留时间
        param1 = cJSON_GetObjectItem(jshead, KEY_AA_STAYDEAYTIME);
        if (param1){
            GDEBUG("filed1: %s, value=%s, %d \n",  JsPrintFmt(param1), param1->string, param1->valueint);
            struAAinfo.StayDelayTime = param1->valueint;
        }

        //自定义声音报警类型
        param1 = cJSON_GetObjectItem(jshead, KEY_AA_ALARMAUDIOTYPE);
        if (param1){
            GDEBUG("filed1: %s, value=%s, %d \n",  JsPrintFmt(param1), param1->string, param1->valueint);
            struAAinfo.AlarmAudioType = param1->valueint;
        }

        //联动动作现在从数组总解析
        param1 = cJSON_GetObjectItem(jshead, KEY_AA_ACTION);
        for (int i = 0; i<ACTION_TIME_SEGNUM; i++){
            cJSON * iJSArrTime = cJSON_GetArrayItem(param1, i);
            if (iJSArrTime){ //可能有些数组不满4个

                cJSON * JSitemArr;
                GDEBUG("i=%d \n", i);

                //先获取时间
                JSitemArr = cJSON_GetObjectItem(iJSArrTime, KEY_AA_ACTION_START); //继续从数组的第一个元素中取
                if (JSitemArr){
                    GDEBUG("%s, value=%s, %d \n", JsPrintFmt(JSitemArr), JSitemArr->string, JSitemArr->valueint);
                    struAAinfo.struTimeSeg[i].StartTime = JSitemArr->valueint;
                }

                JSitemArr = cJSON_GetObjectItem(iJSArrTime, KEY_AA_ACTION_END); //继续从数组的第一个元素中取
                if (JSitemArr){
                    GDEBUG("%s, value=%s, %d \n",  JsPrintFmt(JSitemArr), JSitemArr->string, JSitemArr->valueint);
                    struAAinfo.struTimeSeg[i].EndTime = JSitemArr->valueint;
                }

                //由于SD默认录像, 无需暴露出来了
                JSitemArr = cJSON_GetObjectItem(iJSArrTime, KEY_AA_ACTIONRECORD); //继续从数组的第一个元素中取
                if (JSitemArr){
                    GDEBUG("%s, value=%s, %d \n",  JsPrintFmt(JSitemArr), JSitemArr->string, JSitemArr->valueint);
                    struAAinfo.struTimeSeg[i].ActionMask |= JSitemArr->valueint?DVR_ALARM_TRIGGER_RECORD:0;
                }

                JSitemArr = cJSON_GetObjectItem(iJSArrTime, KEY_AA_ACTIONSOUND); //声
                if (JSitemArr){
                    GDEBUG("%s, value=%s, %d \n",  JsPrintFmt(JSitemArr), JSitemArr->string, JSitemArr->valueint);
                    struAAinfo.struTimeSeg[i].ActionMask |= JSitemArr->valueint?DVR_ALARM_TRIGGER_AUDIOOUT:0;
                }

                JSitemArr = cJSON_GetObjectItem(iJSArrTime, KEY_AA_ACTIONLIGHT); //光
                if (JSitemArr){
                    GDEBUG("%s, value=%s, %d \n",  JsPrintFmt(JSitemArr), JSitemArr->string, JSitemArr->valueint);
                    struAAinfo.struTimeSeg[i].ActionMask |= JSitemArr->valueint?DVR_ALARM_TRIGGER_LIGHT:0;
                }

                JSitemArr = cJSON_GetObjectItem(iJSArrTime, KEY_AA_ACTIONPUSHUP); //上报app
                if (JSitemArr){
                    GDEBUG("%s, value=%s, %d \n",  JsPrintFmt(JSitemArr), JSitemArr->string, JSitemArr->valueint);
                    struAAinfo.struTimeSeg[i].ActionMask |= JSitemArr->valueint?DVR_ALARM_TRIGGER_NOTIFYGUI:0;
                }
            }
            else{
                break;
            }
        }

        MPU_Set_Alarm_Action(&struAAinfo);
		if ((runMdCfg.enable == 0) || ((runMdCfg.handle.is_light[0] == 0) && (runMdCfg.handle.is_light[1] == 0)
			                            &&(runMdCfg.handle.is_light[2] == 0) && (runMdCfg.handle.is_light[3] == 0)))
		{
			if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10"))
				gk_isp_led_off();
		}

        //返回给app
        cJSON* JsSensorNode    = cJSON_CreateObjectEx(&heapobjmgr);
        cJSON_AddNumberToObject(JsSensorNode, "result",   1);

        cJSON* VideoRecordroot = cJSON_CreateObjectEx(&heapobjmgr);
        cJSON_AddItemReferenceToObject(VideoRecordroot, KEY_ALARM_ACTION, JsSensorNode);

        JsToBuffer(VideoRecordroot, s_SesBufSendOut);
    }
    else if (0 == strncmp(jshead->string, KEY_OP_GET, strlen(KEY_OP_GET))){ //此命令是get
        /*
        app发送:
        {"alarm_action":{"get" : null}}
        dev返回:
        {"alarm_action":{"get" :
            {
                "PirEnable":1,
                "MotionEnable":1,
                "MotionSensitive": 2,

                //下面跟人形检测一样, 都在公共路径上:
                "FigureFilterEnable":1,  //在公共源路径上
                "StayDelayTime": 3,  //前3秒不报警
                "Action":[
                        {"StartTime":0,    "EndTime":1440, "ActionRecord":1, "ActionSound":1, "ActionPushup":1, "ActionLight":1, "Action12V":1},
                        {"StartTime":1440, "EndTime":1440, "ActionRecord":1, "ActionSound":0, "ActionPushup":1, "ActionLight":1, "Action12V":1},
                        {"StartTime":1440, "EndTime":1440, "ActionRecord":1, "ActionSound":0, "ActionPushup":1, "ActionLight":1, "Action12V":1}
                        {"StartTime":1440, "EndTime":1440, "ActionRecord":1, "ActionSound":0, "ActionPushup":1, "ActionLight":1, "Action12V":1}
                        ]
                "AlarmAudioType":1,  //播放自定义声音的选择
            }
        }}
        */
        MPU_Get_Alarm_Action(&struAAinfo);
        cJSON* JsSensorNode    = cJSON_CreateObjectEx(&heapobjmgr);

        cJSON_AddNumberToObject(JsSensorNode, KEY_AA_MOTIONENABLE,    struAAinfo.bCheckedMotion);
        cJSON_AddNumberToObject(JsSensorNode, KEY_AA_MOTIONSENSITIVE, struAAinfo.nMotionsenstive);

        //cJSON_AddNumberToObject(JsSensorNode, KEY_AA_FIGUREPILTERENABLE, struAAinfo.bCheckedFigure);// comment it in goke solution

        cJSON_AddNumberToObject(JsSensorNode, KEY_AA_STAYDEAYTIME, struAAinfo.StayDelayTime);

        cJSON* JSAction[ACTION_TIME_SEGNUM]={NULL};
        cJSON *JsArr = cJSON_CreateArrayEx(&heapobjmgr);
        for (int i = 0; i<ACTION_TIME_SEGNUM; i++){
            JSAction[i] = cJSON_CreateObjectEx(&heapobjmgr);
            cJSON_AddNumberToObject(JSAction[i], KEY_AA_ACTION_START,  struAAinfo.struTimeSeg[i].StartTime);
            cJSON_AddNumberToObject(JSAction[i], KEY_AA_ACTION_END,    struAAinfo.struTimeSeg[i].EndTime);

            //SD存储是默认强制, 故不再暴露出来
            cJSON_AddNumberToObject(JSAction[i], KEY_AA_ACTIONRECORD, (struAAinfo.struTimeSeg[i].ActionMask & DVR_ALARM_TRIGGER_RECORD)? 1:0);
            cJSON_AddNumberToObject(JSAction[i], KEY_AA_ACTIONSOUND,  (struAAinfo.struTimeSeg[i].ActionMask & DVR_ALARM_TRIGGER_AUDIOOUT)? 1:0);
            cJSON_AddNumberToObject(JSAction[i], KEY_AA_ACTIONLIGHT,  (struAAinfo.struTimeSeg[i].ActionMask & DVR_ALARM_TRIGGER_LIGHT)? 1:0);
            cJSON_AddNumberToObject(JSAction[i], KEY_AA_ACTIONPUSHUP, (struAAinfo.struTimeSeg[i].ActionMask & DVR_ALARM_TRIGGER_NOTIFYAMS)? 1:0);

            //20181116 因为跟烟弹机开关冲突, 不再设置12V联动项
            //cJSON_AddNumberToObject(JSAction, KEY_AA_ACTIONV12,    (struAAinfo.struTimeSeg[i].ActionMask & DVR_ALARM_TRIGGER_V12)? 1:0);

            cJSON_AddItemReferenceToObject(JsArr, KEY_AA_ACTION, JSAction[i]); //加入到数组中
        }
        cJSON_AddItemReferenceToObject(JsSensorNode, KEY_AA_ACTION, JsArr);

        cJSON_AddNumberToObject(JsSensorNode, KEY_AA_ALARMAUDIOTYPE, struAAinfo.AlarmAudioType);

        ///////////////////////////////////////////////////////////////////////////////////////////////
        //2 封装get
        cJSON* getmiddle = cJSON_CreateObjectEx(&heapobjmgr);
        cJSON_AddItemReferenceToObject(getmiddle, KEY_OP_GET, JsSensorNode);

        //3 封装root
        cJSON* VideoRecordroot = cJSON_CreateObjectEx(&heapobjmgr);
        cJSON_AddItemReferenceToObject(VideoRecordroot, KEY_ALARM_ACTION, getmiddle);

        //4 形成字符串
        JsToBuffer(VideoRecordroot, s_SesBufSendOut);
    }

    JsHeapObjFree(&heapobjmgr);

    return 0;
}

int my_ses_recv(int item)
{
    int ret = 0, ncmdtype = -1;
    ret = glnk_ses_read(s_hOldConnID[item].hconnid, s_SesBufRecvIn, SES_BUF_SIZE, nTimeOutMS);
    if (ret > 0){ //可以读到数据
        int bHex = 0, len = 0;
        //printf("Fnx_SesProcess ret>0, s_SesBufRecvIn:\n%s \n", s_SesBufRecvIn);
        GDEBUG("Fnx_SesProcess ret>0");
        memset(s_SesBufSendOut, 0, SES_BUF_SIZE);

        if(0 == strncmp(s_SesBufRecvIn, DOWNLOADFILE,  TOKENSIZE)){ //传输文件, 这里是自定义二进制流
            bHex = 1;
            s_hOldConnID[item].iType = USRDEF_DOWNLOAD;
            GDEBUG("Fnx_SesProcess, enter HEX DOWNLOADFILE\n");
            //在此直接用透传写文件
            len = download_file(s_SesBufRecvIn); //返回发送长度

            //20180930, 处于性能考虑, 传送完了赶紧关闭连接
            TLV_V_UploadAlarmResponse *phead = (TLV_V_UploadAlarmResponse *)s_SesBufRecvIn;
            if (phead->type == 3){
                s_hOldConnID[item].nconnStatus = CONN_FINISH;
            }

            ret = glnk_ses_write(s_hOldConnID[item].hconnid, s_SesBufSendOut, len, nTimeOutMS);
            GDEBUG("len=%d, ret=%d \n", len, ret);
        }
        else{ //之前的json字符串协议
            ncmdtype = parse_json_cmdtype(s_SesBufRecvIn, struUserType, sizeof(struUserType)/sizeof(USERDEF_CMD));
            s_hOldConnID[item].iType = ncmdtype;
            switch(ncmdtype)
            {
				case USRDEF_ALARM_ACTION:
	                glnk_alarm_action(s_SesBufRecvIn); //20180820 加入pir的控制
	                break;
	            case USRDEF_HUMANTRACE:
	                glnk_humantrace();
	                break;
	            case USRDEF_NETLED:
	                glnk_netstatusled();
	                break;
	            default:
	                break;
            }
            printf("Fnx_SesProcess, s_SesBufSendOut:\n%s \n", s_SesBufSendOut);
            len = strlen(s_SesBufSendOut); //非字符串不能用这个发送了
            if (len > 0)glnk_ses_write(s_hOldConnID[item].hconnid, s_SesBufSendOut, len, nTimeOutMS);
        }
    }
    else if (0 == ret){ //返回给上面后要关闭连接
        GDEBUG("Fnx_SesProcess ret=0, hConnectionID=%d \n", s_hOldConnID[item].hconnid);
    }
    else{//超时, 不关闭连接
        //printf("Fnx_SesProcess ret<0, timeout, hConnectionID=%d \n", hConnectionID);
    }
    //printf("ret=%d", ret);
    return ret;
}


static void *Fnx_GlnkSesProcess(void *lParam)
{
    int ret             = 0;
	int ncmdtype        = -1;
	int i               = 0;
    int hConnectionID   = 0;
    static int bPollSes = 1;

	pthread_detach(pthread_self());
	sdk_sys_thread_set_name("GlnkSesProcessThread");
    printf("Fnx_SesProcess >>>>>>>>>>>>>>>> \n");

    while(bPollSes){
        hConnectionID = glnk_ses_poll(nTimeOutMS); //拿过一次就没有了
        //printf("Fnx_SesProcess, hConnectionID=%d \n", hConnectionID);
        if(hConnectionID){//有新连接过来
            for (i = 0; i<MAX_CONN_NUM; i++){
                if (0 == s_hOldConnID[i].hconnid){ //当没有别人占用, 可以保存, 之后要用数组来保存connid
                    s_hOldConnID[i].hconnid = hConnectionID; //找到了空挡, 直接投入保存起来
                    s_hOldConnID[i].iType = 0; //暂时不填type
                    break;
                }
            }

            if (i == MAX_CONN_NUM){ //找了一圈没有找到空槽slot, 主动拒绝掉.
                GDEBUG("over max connect, reject, hConnectionID=%d \n", hConnectionID); //始终没有进来
                glnk_ses_close(hConnectionID);
            }
        }

        int bExistConn = 0;
        nTimeOutMS = TIMEOUT_IDLE; //从空闲开始数
        for (i = 0; i<MAX_CONN_NUM; i++){//检查老连接, 是否有数据可读, 或者可以关闭了
            if (s_hOldConnID[i].hconnid >0){
                //当有连接在list中时, 重新确定timeout
                if (USRDEF_DOWNLOAD == s_hOldConnID[i].iType){ //发现有上传就进一步拉低
                    nTimeOutMS = (nTimeOutMS < TIMEOUT_DOWNLOAD)?nTimeOutMS:TIMEOUT_DOWNLOAD; //1 有连接还在此时要拉高cpu, 加快读取速度
                }
                nTimeOutMS = (nTimeOutMS < TIMEOUT_JSON)?nTimeOutMS:TIMEOUT_JSON; //1 有连接还在此时要拉高cpu, 加快读取速度

                //先确定最小超时时间
                //printf("check old_connid=%d, poll improve frequency, CPU overload !! nTimeOutMS=%d ", s_hOldConnID[i].hconnid , nTimeOutMS);
            }
        }

        //下面根据超时时间收取
        for (i = 0; i<MAX_CONN_NUM; i++){//检查老连接, 是否有数据可读, 或者可以关闭了
            if (s_hOldConnID[i].hconnid >0){
                bExistConn = 1; //list中还有连接
                ret = my_ses_recv(i);
                if (0 == ret || (CONN_FINISH == s_hOldConnID[i].nconnStatus)){ //有些连接可以主动关闭, 那么也可以关闭
                    GDEBUG("read finish, close old_id=%d", s_hOldConnID[i].hconnid);
                    glnk_ses_close(s_hOldConnID[i].hconnid);
                    memset(& s_hOldConnID[i], 0, sizeof(STRU_CONN_INFO));
                }
            }
        }

        if (0 == bExistConn){ //2 空槽,放慢poll速度
            nTimeOutMS = TIMEOUT_IDLE;
            //printf("change back poll slowly nTimeOutMS=%d", nTimeOutMS);
        }
    }
}

void glnk_cc_switch_init()
{
    int ret = 0;
    pthread_t tThreadCC,  tThreadSes, tThreadAlarm;

    memset(struSenseorNode, 0, sizeof(SENSOR_NODE) * MAX_SENSOR_NODE);

    //glnk_cc_test();

    ret = pthread_create(&tThreadSes, NULL, Fnx_GlnkSesProcess, NULL); //透传接口
    if (ret != 0){
        printf("glnk_cc_switch_init, create Fnx_GlnkSesProcess failed !!!!! \n");
    }
    return;
}

//////////////////////////////////下面是测试函数///////////////////////////////////////
void test_glnk()
{
    char *pbuf = NULL;
//     pbuf = "{\"pir_alarm\":{\"set\":{\"on-off\": true, \"time\":20}}}";
//     glnk_pir(pbuf);

    pbuf = (char *) "{\"alarm_action\":                       \
                   {\"set\":                         \
                         {                           \
                         \"PirEnable\": 1,           \
                         \"MotionEnable\": 0,        \
                         \"MotionSensitive\": 2,     \
                         \"FigureFilterEnable\":1,   \
                         \"Action\":[                \
                                     { \"StartTime\":0,    \"EndTime\":1440,  \"ActionSound\":  1, \"ActionRecord\": 1, \"ActionPushup\": 1}, \
                                     { \"StartTime\":0,    \"EndTime\":1440,  \"ActionSound\":  1, \"ActionRecord\": 1, \"ActionPushup\": 1}, \
                                     { \"StartTime\":0,    \"EndTime\":1440,  \"ActionSound\":  1, \"ActionRecord\": 1, \"ActionPushup\": 1}, \
                                     { \"StartTime\":0,    \"EndTime\":1440,  \"ActionSound\":  1, \"ActionRecord\": 1, \"ActionPushup\": 1} \
                                    ]\
                          } \
                    } \
            } ";
    //glnk_alarm_action(pbuf);

    pbuf = (char *) "{\"alarm_action\":{\"get\":null}}";
    glnk_alarm_action(pbuf);

    return;

    for (int i = 0; i<MAX_SENSOR_NODE; i++){ //遍历所有sensor节点, 将可用的sensor信息全部打印到sendbuf上
        sprintf(struSenseorNode[i].id, "M%d", 1000 + i);
        sprintf(struSenseorNode[i].szLocation, "location_%d", i);
        struSenseorNode[i].bAccepted = 0;
        struSenseorNode[i].bHandledByApp = 0;
    }

    printf("\n\n");

#if 0
    parse_json_cmdtype(s_SesBufSendOut, struUserType, sizeof(struUserType)/sizeof(USERDEF_CMD));
#endif
}


void glnk_cc_test()
{
    test_glnk();
}
