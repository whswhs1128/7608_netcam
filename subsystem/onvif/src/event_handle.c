/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif/src/onvif_handle.c
**
**
** \brief       according the handle value,judge the handle type
**              0-100 eve_handle，1000-1100 dev_handle 1300-1500 other_handle.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <sys/prctl.h>
#include "http.h"
#include "soap_packet.h"
#include "event_handle.h"
//#include "sdk_sys.h"

extern GK_CHAR xml_hdr[];
extern GK_CHAR onvif_xmlns[];
extern GK_CHAR soap_head[];
extern GK_CHAR soap_body[];
extern GK_CHAR soap_tailer[];

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

#define BYTE_WIDE   (8)			/*1 byte width*/

const char *g_Evt_Topic[] =
{
	"tns1:RuleEngine/CellMotionDetector/Motion",
    "tns1:VideoAnalytics/tnspanal:SignalTooDark/FigureChanged",
	"tns1:RuleEngine/FieldDetector/ObctsInside",
	"tns1:RuleEngine/LineDetector/Crossed",
	"tns1:VideoSource/SignalLoss",
	"tns1:PTZController/PTZpreset/Reached",
};

const GK_CHAR *g_Evt_Property[] =
{
    "Initialized",
    "Changed",
    "Deleted"
};

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************


EVENT_PullMessages_S     g_stEventPullMessage;
EVENT_PullMessages_Res_S g_stEventPullMessageRes;

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

static GK_U8   mark[MAX_SUB_ID/8+1]={0x0};    //100 in total, remaining is unused
static GK_BOOL locked = GK_FALSE;
static GK_S32  eventMsgID;
static void*  g_eventHandle = NULL;

pthread_mutex_t EventMutex;

EVENT_Handle_S *eventtHandleAarry[MAX_SUB_ID];  //dispatched by order

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static GK_S32 EvtMngt_szTime_To_Int(char *szTime);
static GK_S32 EvtMngt_Dispatch_Handle(EVENT_HandleType_E typeHandle);
static GK_S32 EvtMngt_Get_HandleNum(EVENT_HandleType_E typeHandle);
static GK_S32 EvtMngt_Free_Handle(GK_S32 index);
static GK_S32 EvtMngt_Handle_InUsed(GK_S32 index);
static GK_S32 EvtMngt_Check_Filter(GK_S32 index, void *filterExpression);
static GK_S32 EvtMngt_Parse_Addr(const GK_CHAR* pdata, GK_CHAR *host, GK_CHAR *url, GK_U32 *port);
static GK_S32 EvtMngt_Package_Message(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header, const void *msg);
static void  *EvtMngt_Process(void *argv);
static void   EvtMngt_Start(void);
static GK_S32 EvtMngt_Alarm_CbFunc(GK_S32 nChannel, GK_S32 nAlarmType, GK_S32 nAction, void *pParam);

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

/***************************************************************************
函数描述: 初始化事件服务管理
入口参数：无
返回值     ：EVENT_MANAGE_OK: 成功
         		      非零: 失败
***************************************************************************/

EVENT_Ret_E EventManagement_Init(void)
{

	pthread_t eventThreadID;
    key_t key;
    memset(eventtHandleAarry, 0, MAX_SUB_ID * sizeof(EVENT_Handle_S *));

	if((key = ftok(".", 'H')) == -1)
	{
	    goto system_error;
	}
	if((eventMsgID = msgget(1234, 0666 | IPC_CREAT)) == -1)
	{
	    goto system_error;
	}

	if(pthread_mutex_init(&EventMutex, NULL) < 0)
	{
	    goto system_error;
	}

	GK_S32 err = pthread_create(&eventThreadID, NULL, EvtMngt_Process, NULL);
	if(err != 0)
	{
	    goto system_error;
	}

    g_eventHandle = event_alarm_open(EvtMngt_Alarm_CbFunc);


    return EVENT_MANAGE_OK;
system_error:
    return EVENT_MANAGE_SYS_ERROR;
}

/********************************************************************************
函数描述: 创建订阅, 仅用于Basic Notification方式
入口参数：GONVIF_EVENT_Subscribe_S *pstEV_Subscribe : 订阅请求信息
				GONVIF_EVENT_Subscribe_Res_S *pstEV_SubscribeRes : 订阅回复信息	
返回值     ：EVENT_MANAGE_OK: 订阅成功
         		      EVENT_MANAGE_HANDLE_OVERFLOW: 事件管理已满
         		      EVENT_MANAGE_SYS_ERROR  : 订阅条件无效
         		      
*********************************************************************************/
EVENT_Ret_E EventManagement_Subscribe(GONVIF_EVENT_Subscribe_S *pstEV_Subscribe, GONVIF_EVENT_Subscribe_Res_S *pstEV_SubscribeRes)
{
    GK_S32 index = 0;
    //Can I change this to char ** to mange the handle?
    index = EvtMngt_Dispatch_Handle(EVEHND);//dispatch management flag.
    if(index == -1)
    {
        return EVENT_MANAGE_HANDLE_OVERFLOW;
    }

    eventtHandleAarry[index] = (EVENT_Handle_S *)malloc(sizeof(EVENT_Handle_S));//dispatch corresponding managment information
	if(eventtHandleAarry[index] == NULL)
	{
		ONVIF_ERR("Fail to alloc memory.");
		return EVENT_MANAGE_SYS_ERROR;
	}
    memset(eventtHandleAarry[index], 0, sizeof(EVENT_Handle_S));

    EVENT_Handle_S *tempHandle = eventtHandleAarry[index];

    snprintf(pstEV_SubscribeRes->stSubscriptionReference.aszAddress, MAX_ADDR_LENGTH,
        "http://%s:%d/Subcription?Idx=%d", g_GkIpc_OnvifInf.discov.hostip, g_GkIpc_OnvifInf.discov.hostport, index);
#if 0
    struct tm *subtime;
#endif
    GK_CHAR cur_time[100], term_time[100];
    memset(cur_time, 0, sizeof(cur_time));
    memset(term_time, 0, sizeof(term_time));

    time(&pstEV_SubscribeRes->currentTime);
#if 0
    subtime = localtime(&pstEV_SubscribeRes->currentTime);
    sprintf(cur_time, "%04d-%02d-%02dT%02d:%02d:%02dZ",
    	subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
    	subtime->tm_hour, subtime->tm_min, subtime->tm_sec);
    ONVIF_DBG("[EventManagement_Subscribe]cur_time:%s\n", cur_time);
#endif

    GK_S32 tempTime = EvtMngt_szTime_To_Int(pstEV_Subscribe->aszInitialTerminationTime);
    if(tempTime == 0)
    {
        pstEV_SubscribeRes->terminationTime = pstEV_SubscribeRes->currentTime + 3600;   //default termination time.
    }
    else
    {
        pstEV_SubscribeRes->terminationTime = pstEV_SubscribeRes->currentTime + tempTime;
    }
#if 0
    subtime = localtime(&pstEV_SubscribeRes->terminationTime);
    sprintf(term_time, "%04d-%02d-%02dT%02d:%02d:%02dZ",
    	subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
    	subtime->tm_hour, subtime->tm_min, subtime->tm_sec);
    ONVIF_DBG("[EventManagement_Subscribe]term_time:%s\n", term_time);
#endif
    //save corresponding subcription information to event managment handle.
    tempHandle->startTime = pstEV_SubscribeRes->currentTime;
    tempHandle->TerminationTime = pstEV_SubscribeRes->terminationTime;
    strncpy(tempHandle->aszClientAddress, pstEV_Subscribe->stConsumerReference.aszAddress, MAX_ADDR_LENGTH - 1);
    strncpy(tempHandle->aszServerAddress, pstEV_SubscribeRes->stSubscriptionReference.aszAddress, MAX_ADDR_LENGTH - 1);
    if(pstEV_Subscribe->stFilter.aszTopicExpression[0] != '\0')
    {
        strncpy(tempHandle->aszTopicExpression,
            pstEV_Subscribe->stFilter.aszTopicExpression, MAX_TOPICEXPRESSION_LEN - 1);
    }

    if(pstEV_Subscribe->stFilter.aszMessageContent[0] != '\0')
    {
        strncpy(tempHandle->aszMessageContent,
            pstEV_Subscribe->stFilter.aszMessageContent, MAX_MESSAGECONTENT_LEN - 1);
    }

    return EVENT_MANAGE_OK;

}

/***************************************************************************
函数描述: 取消订阅
入口参数：index  : 订阅ID 号
返回值     ：EVENT_MANAGE_OK: 取消订阅成功
         		      EVENT_MANAGE_HANDLE_FREE_FAILED : 取消订阅失败
				EVENT_MANAGE_HANDLE_NOTEXIST : 订阅号不存在
***************************************************************************/
EVENT_Ret_E EventManagement_Unsubscribe(GK_S32 index)
{
    GK_S32 ret;
    ret = EvtMngt_Handle_InUsed(index);
    if(ret == 1)
    {
        if(-1 == EvtMngt_Free_Handle(index))
        {
            return EVENT_MANAGE_HANDLE_FREE_FAILED;
        }
    }
    else
    {
        return EVENT_MANAGE_HANDLE_NOTEXIST;
    }

    return EVENT_MANAGE_OK;
}

/***************************************************************************
函数描述: 取消订阅
入口参数：index  : 更新订阅ID 号
				GONVIF_EVENT_Renew_S *pstEV_Renew : 订阅请求
				GONVIF_EVENT_Renew_Res_S *pstEV_RenewRes : 订阅回复 
返回值     ：EVENT_MANAGE_OK: 更新订阅成功
				EVENT_MANAGE_HANDLE_NOTEXIST : 订阅号不存在
***************************************************************************/
EVENT_Ret_E EventManagement_Renew(GK_S32 index, GONVIF_EVENT_Renew_S *pstEV_Renew, GONVIF_EVENT_Renew_Res_S *pstEV_RenewRes)
{
    GK_S32 ret;
    time(&pstEV_RenewRes->currentTime);
    GK_S32 tempTime = EvtMngt_szTime_To_Int(pstEV_Renew->aszTerminationTime);
    pstEV_RenewRes->terminationTime = pstEV_RenewRes->currentTime + tempTime;

    ret = EvtMngt_Handle_InUsed(index);
    if(ret == 1)
    {
        eventtHandleAarry[index]->TerminationTime = pstEV_RenewRes->terminationTime;
    }
    else
    {
        return EVENT_MANAGE_HANDLE_NOTEXIST;
    }

    return EVENT_MANAGE_OK;
}

/***************************************************************************
函数描述:  创建拉取点
入口参数：pstEV_CreatePullPointSubscription  : 创建拉取点请求信息
				pstEV_CreatePullPointSubscriptionRes : 创建拉取点回复信息 
返回值     ：EVENT_MANAGE_OK: 创建拉取点成功
				EVENT_MANAGE_HANDLE_OVERFLOW : 事件管理已满
***************************************************************************/
EVENT_Ret_E EventManagement_CreatePullPointSubscription(GONVIF_EVENT_CreatePullPointSubscription_S *pstEV_CreatePullPointSubscription, GONVIF_EVENT_CreatePullPointSubscription_Res_S *pstEV_CreatePullPointSubscriptionRes)
{
    GK_S32 index = 0;
    index = EvtMngt_Dispatch_Handle(EVEHND);
    if(-1 == index)
    {
        return EVENT_MANAGE_HANDLE_OVERFLOW;
    }

    eventtHandleAarry[index] = (EVENT_Handle_S *)malloc(sizeof(EVENT_Handle_S));
    EVENT_Handle_S *tempHandle = eventtHandleAarry[index];
    memset(tempHandle, 0, sizeof(EVENT_Handle_S));

    sprintf(pstEV_CreatePullPointSubscriptionRes->stSubscriptionReference.aszAddress,
        "http://%s:%d/Subscription?Idx=%d", g_GkIpc_OnvifInf.discov.hostip, g_GkIpc_OnvifInf.discov.hostport, index);
    time(&pstEV_CreatePullPointSubscriptionRes->aszCurrentTime);

    GK_S32 tempTime = EvtMngt_szTime_To_Int(pstEV_CreatePullPointSubscription->aszInitialTerminationTime);
    if(tempTime == 0)
    {
        pstEV_CreatePullPointSubscriptionRes->aszTerminationTime = pstEV_CreatePullPointSubscriptionRes->aszCurrentTime + 120;
    }
    else
    {
        pstEV_CreatePullPointSubscriptionRes->aszTerminationTime = pstEV_CreatePullPointSubscriptionRes->aszCurrentTime + tempTime;
    }
    //eventtHandleAarry
    tempHandle->startTime = pstEV_CreatePullPointSubscriptionRes->aszCurrentTime;
    tempHandle->TerminationTime =  pstEV_CreatePullPointSubscriptionRes->aszTerminationTime;
    strncpy(tempHandle->aszServerAddress, pstEV_CreatePullPointSubscriptionRes->stSubscriptionReference.aszAddress, MAX_ADDR_LENGTH - 1);

    if(pstEV_CreatePullPointSubscription->stFilter.aszTopicExpression[0] != '\0')
    {
        strncpy(tempHandle->aszTopicExpression, pstEV_CreatePullPointSubscription->stFilter.aszTopicExpression, MAX_TOPICEXPRESSION_LEN - 1);
    }

    if(pstEV_CreatePullPointSubscription->stFilter.aszMessageContent[0] != '\0')
    {
        strncpy(tempHandle->aszMessageContent, pstEV_CreatePullPointSubscription->stFilter.aszMessageContent, MAX_MESSAGECONTENT_LEN - 1);
    }

    return EVENT_MANAGE_OK;

}

/***************************************************************************
函数描述:  拉取消息
入口参数：index  : 订阅ID 号
				timeout  : 超时
				messageLimit : 获取Notify 最大数量
返回值     ：EVENT_MANAGE_OK: 拉取成功
				EVENT_MANAGE_HANDLE_NOTEXIST : 订阅号不存在
***************************************************************************/
EVENT_Ret_E EventManagement_PullMessages(GK_S32 index, GK_S32 timeout, GK_S32 messageLimit)
{
    GK_S32 ret;
    ret = EvtMngt_Handle_InUsed(index);
    if(ret != 1)
    {
        return EVENT_MANAGE_HANDLE_NOTEXIST;
    }

    time_t curTime;
    time(&curTime);
    memset(&g_stEventPullMessage, 0, sizeof(g_stEventPullMessage));
    g_stEventPullMessage.messageLimit = messageLimit;
    g_stEventPullMessage.terminationTime = curTime + timeout;
    g_stEventPullMessage.eventID = index;
    g_stEventPullMessage.enPullMsg = Boolean_TRUE;

    memset(&g_stEventPullMessageRes, 0, sizeof(g_stEventPullMessageRes));
    g_stEventPullMessageRes.pstNotificationMessage = (EVENT_NotificationMessage_S *)malloc(sizeof(EVENT_NotificationMessage_S) * messageLimit);
    memset(g_stEventPullMessageRes.pstNotificationMessage, 0, sizeof(EVENT_NotificationMessage_S) * messageLimit);

    sleep(2);
    pthread_mutex_lock(&EventMutex);//wait for get full message(messageLimit)or timeout and then send all messages or part of messages..

    memset(&g_stEventPullMessage, 0, sizeof(g_stEventPullMessage));
    time(&curTime);
    struct tm *subtime;
    subtime = localtime(&curTime);
	sprintf(g_stEventPullMessageRes.aszCurrentTime, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
		subtime->tm_hour, subtime->tm_min, subtime->tm_sec);

    subtime = localtime(&(eventtHandleAarry[index]->TerminationTime));
    sprintf(g_stEventPullMessageRes.aszTerminationTime, "%04d-%02d-%02dT%02d:%02d:%02dZ",
        subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
        subtime->tm_hour, subtime->tm_min, subtime->tm_sec);

    pthread_mutex_unlock(&EventMutex);

    return EVENT_MANAGE_OK;

}

EVENT_Ret_E EventManagement_Exit(void)
{
    if(g_eventHandle)
    {
	    event_alarm_close(g_eventHandle);
        g_eventHandle = 0;
    }
    return EVENT_MANAGE_OK;
}
/*******************************************************************************/
static GK_S32 EvtMngt_szTime_To_Int(char *szTime)
{
	/*
	* PT[%dH][%dM][%dS].[....]
	*/
	GK_CHAR *ptr = szTime;
	GK_CHAR tmp[32];
	GK_S32 i, ret;
	GK_S32 h = 0, m = 0, s = 0;
	if (szTime[0] != 'P' || szTime[1] != 'T')
	{
		return -1;
	}
	ptr += 2;
	for(;;)
	{
		i = 0;
		while(isdigit(*ptr) && *ptr != '\0'  && i < 10)
		    tmp[i++] = *ptr++;
		if(i >= 10)
		{
			break;
        }
		else if(i > 0)
		{
			tmp[i] = '\0';
			if(*ptr == 'H')
				sscanf(tmp, "%d", &h);
			else if(*ptr == 'M')
				sscanf(tmp, "%d", &m);
			else if(*ptr == 'S')
				sscanf(tmp, "%d", &s);
			else if(*ptr == '\0')
				break;
		}
		else
		{
			break;
		}
		while((!isdigit(*ptr)) && (*ptr != '\0'))
		{
			ptr++;
		}
		if (*ptr == '\0')
			break;
	}

	ret = 3600*h + 60*m + s;

	return ret;
}


static GK_S32 EvtMngt_Get_HandleNum(EVENT_HandleType_E typeHandle)
{

    EventMngt_Handle max_sd = 0;
	GK_U8 *checkPos = NULL;
    GK_U8 tmp = 0;
    GK_U8 *pmark = NULL;
    GK_S32 i = 0;
    GK_S32 j = 0;

    (void)typeHandle;

    pmark = mark;
    checkPos = &pmark[MAX_SUB_ID/8];

    while(i <= (MAX_SUB_ID/8))
    {
        if(0 != checkPos[0-i])
        {
            tmp = checkPos[0-i];
            for(j = 0; j < BYTE_WIDE; j++)
            {
                tmp >>= 1;
                if( 0x0 == tmp)
                {
                    return (max_sd = (MAX_SUB_ID + (8 - MAX_SUB_ID%8)%8 - i*8 - (BYTE_WIDE - j)) + 1);
                }
            }
        }
        i++;
    }

    return max_sd;

}

static GK_S32 EvtMngt_Dispatch_Handle(EVENT_HandleType_E typeHandle)
{
	GK_U8 *checkPos = mark;
	GK_U8 i, j;
	GK_S32 index = -1;
	GK_U8 tempMove;

    (void)typeHandle;

	for(i = 0; i < MAX_SUB_ID/8 + 1; i++, checkPos++)
	{
		tempMove = *checkPos;
		for(j = 0; j < 8; j++)
		{
			if(0 == ((tempMove >> j) & (0x01)))
			{
				*checkPos |= 1<<j;
				index = ((checkPos - mark) * BYTE_WIDE + j);
				if(index <= (MAX_SUB_ID-1))
				    return index;
			}
		}
	}

	return -1;

}

static GK_S32 EvtMngt_Free_Handle(GK_S32 index)
{
    GK_U8 *pmark = mark;
    GK_S32 _2pos = (index%BYTE_WIDE);
    GK_S32 _1pos = (index/BYTE_WIDE);
    /*reset mark bit */
    if (0 != (pmark[_1pos] & (0x1 << _2pos)) )
    {
        pmark[_1pos] &= ~(0x1 << _2pos);
    }
    else
    {
        return -1;
    }
    /*free EVE handle */
    if(eventtHandleAarry[index])
    {
        memset(eventtHandleAarry[index], 0, sizeof(EVENT_Handle_S));
        free(eventtHandleAarry[index]);
        eventtHandleAarry[index] = NULL;
    }
    else
    {
        return -1;
    }

    return 0;
}

static GK_S32 EvtMngt_Handle_InUsed(GK_S32 index)
{
    if(index >= MAX_SUB_ID)
    {
        return -1;
    }

    return (eventtHandleAarry[index] ? 1 : 0);
}


static GK_S32 EvtMngt_Check_Filter(GK_S32 index, void *filterExpression)
{
    GK_S32 ret = 0;
    GK_S32 topicExpressionLen = 0, messageContentLen = 0;
    EVENT_FilterExpression_S *tempFilter = (EVENT_FilterExpression_S *)filterExpression;

    ret = EvtMngt_Handle_InUsed(index);
    if(ret != 1)
    {
        return EVENT_MANAGE_HANDLE_NOTEXIST;
    }

    topicExpressionLen = strlen(tempFilter->aszTopicExpression);
    messageContentLen = strlen(tempFilter->aszMessageContent);
    ONVIF_DBG("[EvtMngt_Check_Filter]topicExpressionLen: %d\n"
        "[EvtMngt_Check_Filter]contentExpressionLen: %d\n",
        topicExpressionLen, messageContentLen);
    EVENT_Handle_S *tempHandle = eventtHandleAarry[index];
    /*TopicExpression Compare*/
    if(tempHandle->aszTopicExpression[0] != '\0')
    {
        if(0 == strncmp(tempFilter->aszTopicExpression, tempHandle->aszTopicExpression, topicExpressionLen))
        {
            ONVIF_DBG("[EvtMngt_Check_Filter]TopicExpression match!\n");
        }
        else
        {
            return -2;//topicset not match
        }

    }
    /*MessageContent Compare*/
    if(tempHandle->aszMessageContent[0] != '\0')
    {
        if(0 == strncmp(tempFilter->aszMessageContent, tempHandle->aszMessageContent, messageContentLen))
        {
            ONVIF_DBG("[EvtMngt_Check_Filter]MessageContent match!\n");
        }
        else
        {
            return -3;//message content not match
        }
    }

    return 0;
}

static GK_S32 EvtMngt_Parse_Addr(const GK_CHAR* pdata, GK_CHAR *host, GK_CHAR *url, GK_U32 *port)
{
    const GK_CHAR *p1, *p2;
    GK_S32 len = strlen(pdata);

    *port = 80;
    //http://192.168.103.244:8085/subscription-5
    if (len > 7) // skip "http://"
    {
        p1 = strchr(pdata+7, ':');
        if (p1)
        {
            strncpy(host, pdata+7, p1-pdata-7);

            char buff[100];
            memset(buff, 0, 100);

            p2 = strchr(p1, '/');
            if (p2)
            {
                strncpy(url, p2, len - (p2 - pdata));

                len = p2 - p1 - 1;
                strncpy(buff, p1+1, len);
            }
            else
            {
                len = len - (p1 - pdata);
                strncpy(buff, p1+1, len);
            }

            *port = atoi(buff);
        }
        else
        {
            p2 = strchr(pdata+7, '/');
            if (p2)
            {
                strncpy(url, p2, len - (p2 - pdata));

                len = p2 - pdata - 7;
                strncpy(host, pdata+7, len);
            }
            else
            {
                len = len - 7;
                strncpy(host, pdata+7, len);
            }
        }
    }

    return 1;
}


static GK_S32 EvtMngt_Package_Message(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header, const void *msg)
{
	EVENT_Handle_S  *pstEventHandle = (EVENT_Handle_S *)argv;
	GONVIF_Soap_S   *pstSoapHeader  = (GONVIF_Soap_S *)header;
    EVENT_Message_S *pstEventMsg    = (EVENT_Message_S *)msg;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstSoapHeader->stHeader.pszAction);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	GK_CHAR aszTime[100];
    memset(aszTime, 0, sizeof(aszTime));
	struct tm *eventOcurredtime;
	eventOcurredtime = localtime(&pstEventMsg->stEventContent.occurenceTime);
	snprintf(aszTime, sizeof(aszTime), "%04d-%02d-%02dT%02d:%02d:%02dZ",
		eventOcurredtime->tm_year+1900, eventOcurredtime->tm_mon+1, eventOcurredtime->tm_mday,
		eventOcurredtime->tm_hour, eventOcurredtime->tm_min, eventOcurredtime->tm_sec);

    if(pstEventMsg->enEventType == ONVIF_EVENT_MOTION_DETECTOR)
    {
    	offset += snprintf(p_buf+offset, mlen-offset,
    		"<wsnt:Notify>"ONVIF_MD_MSG_FMT"</wsnt:Notify>",
    		pstEventHandle->aszClientAddress, pstEventHandle->aszServerAddress, aszTime);
    }
    else if(pstEventMsg->enEventType == ONVIF_EVENT_VIDEO_TAMPER)
    {
    	offset += snprintf(p_buf+offset, mlen-offset,
    		"<wsnt:Notify>"ONVIF_VT_MSG_FMT"</wsnt:Notify>",
    	pstEventHandle->aszClientAddress, pstEventHandle->aszServerAddress, aszTime);
    }
    else if(pstEventMsg->enEventType == ONVIF_EVENT_FIELD_DETECTOR)
    {
    	//offset += snprintf(p_buf+offset, mlen-offset,
    		//"<wsnt:Notify>"ONVIF_MD_MSG_FMT"</wsnt:Notify>",
    		//pstEventHandle->aszClientAddress, pstEventHandle->aszServerAddress, aszTime);
    }
    else if(pstEventMsg->enEventType == ONVIF_EVENT_LINE_DETECTOR)
    {
    	//offset += snprintf(p_buf+offset, mlen-offset,
    		//"<wsnt:Notify>"ONVIF_MD_MSG_FMT"</wsnt:Notify>",
    		//pstEventHandle->aszClientAddress, pstEventHandle->aszServerAddress, aszTime);
    }
    else if(pstEventMsg->enEventType == ONVIF_EVENT_PTZ_PRESET_REACHED)
    {
    	//offset += snprintf(p_buf+offset, mlen-offset,
    		//"<wsnt:Notify>"ONVIF_MD_MSG_FMT"</wsnt:Notify>",
    		//pstEventHandle->aszClientAddress, pstEventHandle->aszServerAddress, aszTime);
    }
    else
    {
        ONVIF_ERR("unrecognizable event type.");
        return -1;
    }

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

static GK_S32 EvtMngt_Alarm_CbFunc(GK_S32 nChannel, GK_S32 nAlarmType, GK_S32 nAction, void *pParam)
{
    GK_S32 ret = 0;
    EVENT_Send_S stEventMsg;
    memset(&stEventMsg, 0, sizeof(EVENT_Send_S));
    time_t now;
    time(&now);
    stEventMsg.time = now;
    stEventMsg.enType = nAlarmType;
    ret = msgsnd(eventMsgID, (void *)&stEventMsg, sizeof(EVENT_Send_S)-4, 0);
    if(ret == -1)
    {
        ONVIF_ERR("send event message faild.");
        return -1;
    }

    return 0;
}

static void *EvtMngt_Process(void *argv)
{
    struct timeval tv;

    sdk_sys_thread_set_name("EvtMngt_Process");
    while(1)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 800000;
        select(1, NULL, NULL, NULL, &tv);
        EvtMngt_Start();
    }

    return NULL;
}

static void EvtMngt_Start(void)
{
    GK_S32 sizeHandle = 0;
    GK_S32 index = 0;
    time_t now;
    time(&now);
    EVENT_Send_S stRecEvent;
    memset(&stRecEvent, 0, sizeof(EVENT_Send_S));
    if(g_stEventPullMessage.enPullMsg == Boolean_TRUE && locked == GK_FALSE)
    {
        locked = GK_TRUE;
        pthread_mutex_lock(&EventMutex);
    }

    memset(&stRecEvent, 0, sizeof(EVENT_Send_S));
    if(msgrcv(eventMsgID, &stRecEvent, EVENT_RECEIVE_LEN, 0, IPC_NOWAIT) == -1)
    {
        //TODO: no event occured.
    }
    else
    {
    	//ONVIF_INFO("Event accured!");
        EVENT_Message_S stEventMsg;
        memset(&stEventMsg, 0, sizeof(EVENT_Message_S));
        stEventMsg.enEventType = (EVENT_Type_E)stRecEvent.enType;
        stEventMsg.stEventContent.occurenceTime = stRecEvent.time;
        if(stEventMsg.enEventType == ONVIF_EVENT_MOTION_DETECTOR)
        {
            strncpy(stEventMsg.stEventContent.stFilterExpression.aszTopicExpression,
                "tns1:RuleEngine/CellMotionDetector/Motion", EVENT_FILTER_LEN-1);
        }
        else if(stEventMsg.enEventType == ONVIF_EVENT_VIDEO_TAMPER)
        {
            strncpy(stEventMsg.stEventContent.stFilterExpression.aszTopicExpression,
                "tns1:VideoAnalytics/tnspanal:SignalTooDark/FigureChanged", EVENT_FILTER_LEN-1);
        }
        sizeHandle = EvtMngt_Get_HandleNum(EVEHND);
        for(index = 0; index < sizeHandle; index++)// if this event satisfy whichever subscription, the corresponding message will be send.
        {
            if(EvtMngt_Check_Filter(index, (void *)&stEventMsg.stEventContent.stFilterExpression) != 0)
            {
                //filter not match
                ONVIF_DBG("[EvtMngt_Start]filter not match! handle: %d\n", index);
            }
            else
            {
                EVENT_Handle_S *tempHandle1 = eventtHandleAarry[index];
                if(tempHandle1 != NULL)
                {
                    //Just points given in the CreatePullSubscriptionResponse can be pulled.
                    if((g_stEventPullMessage.enPullMsg == Boolean_TRUE) && (g_stEventPullMessage.eventID == index /*pull point*/))
                    {
                        #if 0
                        ONVIF_DBG("[EvtMngt_Start]eventMsg.notify.occurenceTime: %ld\n"
                                  "[EvtMngt_Start]startTime: %ld\n"
                                  "[EvtMngt_Start]TerminationTime: %ld\n",
                                   eventMsg.stEventContent.occurenceTime,
                                   tempHandle1->startTime,
                                   tempHandle1->TerminationTime);
                        #endif
                        //compare occurenceTime time
                        if(stEventMsg.stEventContent.occurenceTime >= tempHandle1->startTime &&
                            stEventMsg.stEventContent.occurenceTime <= tempHandle1->TerminationTime)
                        {
                            struct tm *subtime;
                            subtime = localtime(&stEventMsg.stEventContent.occurenceTime);
                            sprintf(g_stEventPullMessageRes.pstNotificationMessage[g_stEventPullMessageRes.sizeNotificationMessage].aszEventOccurTime,
                                "%04d-%02d-%02dT%02d:%02d:%02dZ",subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
                                                                 subtime->tm_hour, subtime->tm_min, subtime->tm_sec);

                            snprintf(g_stEventPullMessageRes.pstNotificationMessage[g_stEventPullMessageRes.sizeNotificationMessage].aszEvent, EVENT_MSG_LEN,
                                        "<wsnt:Topic Dialect=\"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet\">"
                                            "tns1:RuleEngine/CellMotionDetector/Motion"                //what happended? Step one: categorize the Event.
                                        "</wsnt:Topic>"
                                        //"<wsnt:ProducerRefences>"
                                            //"%s"-->EndpointReferenceType                                                  //who produced the event? part one: identifies the device or a service within the device where the notification has been produced.
                                       // "</wsnt:ProducerRefences>"
                                        "<wsnt:Message>" //Source Key Data
                                            "<tt:Message UtcTime=\"%s\" PropertyOperation=\"Changed\">"//when did it happend?
                                            "<tt:Source>"
                                            "<tt:SimpleItem Name=\"VideoSourceConfigurationToken\" Value=\"IPCameraSourceToken\"/>"         //who produced the event? part two: identification of
                                            "<tt:SimpleItem Name=\"VideoAnalyticsConfigurationToken\" Value=\"IPCameraAnalyticsToken\"/>"   //the component within the WS-Endpoint, which is
                                            "<tt:SimpleItem Name=\"Rule\" Value=\"IPCameraSourceToken\"/>"                                  //responsible for the production of the notification.
                                            "</tt:Source>"
                                            "<tt:Data>"
                                            "<tt:SimpleItem Name=\"IsMotion\" Value=\"true\"/>"         //what happended? Step two: describe the details of the Event.
                                            "</tt:Data>"
                                            "</tt:Message>"
                                        "</wsnt:Message>",
                                        g_stEventPullMessageRes.pstNotificationMessage[g_stEventPullMessageRes.sizeNotificationMessage].aszEventOccurTime);

                            g_stEventPullMessageRes.sizeNotificationMessage++;
                            //when corresponding event ocurred, device should send the message to client until numbers of messages
                            // beyong Limit or timeout is reached.
                            if(g_stEventPullMessageRes.sizeNotificationMessage >= g_stEventPullMessage.messageLimit)
                            {
                                locked = GK_FALSE;
                                pthread_mutex_unlock(&EventMutex);
                            }
                        }
                    }
                    //subscription
                    else
                    {
                        //compare occurenceTime time
                        if(stEventMsg.stEventContent.occurenceTime >= tempHandle1->startTime &&
                            stEventMsg.stEventContent.occurenceTime <= tempHandle1->TerminationTime)
                        {

                            ONVIF_DBG("[EvtMngt_Start]eventMsg.notify.occurenceTime: %ld\n"
                                   "[EvtMngt_Start]startTime: %ld\n"
                                   "[EvtMngt_Start]TerminationTime: %ld\n",
                                   stEventMsg.stEventContent.occurenceTime,
                                   tempHandle1->startTime,
                                   tempHandle1->TerminationTime);
                            //send message context to client

                            HTTPREQ stHttpReq;
                            GK_CHAR aszDataBuf[4*1024];
                            GONVIF_Soap_S stSoapHeader;
                            memset(&aszDataBuf, 0, 4*1024);
                            memset(&stHttpReq, 0, sizeof(HTTPREQ));
                            memset(&stSoapHeader, 0, sizeof(GONVIF_Soap_S));
                            stSoapHeader.stHeader.pszAction = "http://docs.oasis-open.org/wsn/bw-2/NotificationConsumer/Notify";
                            strncpy(stHttpReq.action, stSoapHeader.stHeader.pszAction, 254);
                            EvtMngt_Parse_Addr(tempHandle1->aszClientAddress, stHttpReq.host, stHttpReq.url, &stHttpReq.port);//pull message has not EndpointReferenceType, so aszClientAddress is empty.
                            GK_S32 len = EvtMngt_Package_Message(aszDataBuf, sizeof(aszDataBuf), (const void *)tempHandle1, (const void *)&stSoapHeader, (const void *)&stEventMsg);
                            if(len != -1)
                            {
                                http_client_socket_request(&stHttpReq, 5000, aszDataBuf, len);
                            }
                        }
                    }
                }
            }
        }
    }

    sizeHandle = EvtMngt_Get_HandleNum(EVEHND);
    for(index = 0; index < sizeHandle; index++)
    {
        if(EvtMngt_Handle_InUsed(index) == 1)
        {
            EVENT_Handle_S *tempHandle2 = eventtHandleAarry[index];
            #if 0
            struct tm *subtime;
            GK_CHAR cur_time[100], term_time[100];
            memset(cur_time, 0, sizeof(cur_time));
            memset(term_time, 0, sizeof(term_time));

            subtime = localtime(&now);
            sprintf(cur_time, "%04d-%02d-%02d %02d:%02d:%02d",
                subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
                subtime->tm_hour, subtime->tm_min, subtime->tm_sec);
            ONVIF_DBG("[EvtMngt_Start]curtime:%s\n",cur_time);

            subtime = localtime(&tempHandle->TerminationTime);
            sprintf(term_time, "%04d-%02d-%02d %02d:%02d:%02d",
                subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
                subtime->tm_hour, subtime->tm_min, subtime->tm_sec);
            ONVIF_DBG("[EvtMngt_Start]termtime:%s\n", term_time);
            #endif

            if((g_stEventPullMessage.eventID == index) && g_stEventPullMessage.enPullMsg == Boolean_TRUE)
            {
                if(tempHandle2->TerminationTime <= now)
                {
                    if(EvtMngt_Free_Handle(index) == -1)
                    {
                        ONVIF_DBG("[EvtMngt_Start]timer free handle failed\n");
                    }
                    locked = GK_FALSE;
                    pthread_mutex_unlock(&EventMutex);
                    ONVIF_DBG("[event_timer]timer free handle succeed!\n");
                }
                else if(g_stEventPullMessage.terminationTime <= now)
                {
                    ONVIF_DBG("pullmessage timeout!!!!\n");
                    locked = GK_FALSE;
                    pthread_mutex_unlock(&EventMutex);
                }
            }
            else
            {
                if(tempHandle2->TerminationTime <= now)
                {
                    if(EvtMngt_Free_Handle(index) == -1)
                    {
                        ONVIF_DBG("[EvtMngt_Start]timer free handle failed\n");
                    }
                }
            }
        }
    }
}

