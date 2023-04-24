/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif/src/EvtMngt_Start.h
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _EVENT_HANDLE_H_
#define _EVENT_HANDLE_H_

#include "onvif_std.h"
#include "onvif_priv.h"
#include "eventalarm.h"


//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************

#define EVENT_FILTER_LEN				128
#define EVENT_RECEIVE_LEN 			    (sizeof(EVENT_Receive_S)-sizeof(GK_S32))
#define EVENT_MSG_LEN                   1024

#define ONVIF_MD_MSG_FMT \
	"<wsnt:NotificationMessage>"\
		"<wsnt:SubscriptionReference>"\
			"<wsa:Address>%s</wsa:Address>"\
		"</wsnt:SubscriptionReference>"\
		"<wsnt:Topic Dialect=\"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet\">"\
			"tns1:RuleEngine/CellMotionDetector/Motion"\
		"</wsnt:Topic>"\
		"<wsnt:ProducerReference>"\
			"<wsa:Address>%s</wsa:Address>"\
		"</wsnt:ProducerReference>"\
		"<wsnt:Message>"\
			"<tt:Message UtcTime=\"%s\" PropertyOperation=\"Changed\">"\
			"<tt:Source>"\
			"<tt:SimpleItem Name=\"VideoSourceConfigurationToken\" Value=\"IPCameraSourceToken\"/>"\
			"<tt:SimpleItem Name=\"VideoAnalyticsConfigurationToken\" Value=\"IPCameraAnalyticsToken\"/>"\
			"<tt:SimpleItem Name=\"Rule\" Value=\"IPCameraSourceToken\"/>"\
			"</tt:Source>"\
			"<tt:Data>"\
			"<tt:SimpleItem Name=\"IsMotion\" Value=\"true\"/>"\
			"</tt:Data>"\
			"</tt:Message>"\
		"</wsnt:Message>"\
	"</wsnt:NotificationMessage>"

#define ONVIF_VT_MSG_FMT \
    "<wsnt:NotificationMessage>"\
        "<wsnt:SubscriptionReference>"\
            "<wsa:Address>%s</wsa:Address>"\
        "</wsnt:SubscriptionReference>"\
        "<wsnt:Topic Dialect=\"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet\">"\
            "tns1:VideoAnalytics/tnspanal:SignalTooDark/FigureChanged"\
        "</wsnt:Topic>"\
        "<wsnt:ProducerReference>"\
            "<wsa:Address>%s</wsa:Address>"\
        "</wsnt:ProducerReference>"\
        "<wsnt:Message>"\
            "<tt:Message UtcTime=\"%s\" PropertyOperation=\"Changed\">"\
            "<tt:Source>"\
            "<tt:SimpleItem Name=\"VideoAnalytics\" Value=\"VT\"/>"\
            "</tt:Source>"\
            "<tt:Data>"\
            "<tt:SimpleItem Name=\"IsTamper\" Value=\"true\"/>"\
            "</tt:Data>"\
            "</tt:Message>"\
        "</wsnt:Message>"\
    "</wsnt:NotificationMessage>"

typedef int EventMngt_Handle;

//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************

typedef enum
{
	EVEHND = 0,
	DEVHND = 1
}EVENT_HandleType_E;

typedef enum
{
    EVENT_MANAGE_OK,
    EVENT_MANAGE_SYS_ERROR,
    EVENT_MANAGE_HANDLE_OVERFLOW,
    EVENT_MANAGE_HANDLE_NOTEXIST,
    EVENT_MANAGE_HANDLE_FREE_FAILED,
    EVENT_MANAGE_INVALID_TOPICEXPRESSION,
}EVENT_Ret_E;

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef enum
{
	ONVIF_EVENT_MOTION_DETECTOR = 3,
	ONVIF_EVENT_VIDEO_TAMPER = 6,
    ONVIF_EVENT_FIELD_DETECTOR,
	ONVIF_EVENT_LINE_DETECTOR,
	ONVIF_EVENT_SIGNAL_LOST,
	ONVIF_EVENT_PTZ_PRESET_REACHED,
	
	ONVIF_EVENT_ALL,
}EVENT_Type_E;

typedef struct
{
	EVENT_Type_E enType;     
	time_t time;
}EVENT_Receive_S;

typedef struct
{
	GK_ALARM_TYPE_E enType;     
	time_t time;
}EVENT_Send_S;

typedef struct
{
    GK_CHAR aszTopicExpression[EVENT_FILTER_LEN];
    GK_CHAR aszMessageContent[EVENT_FILTER_LEN];
}EVENT_FilterExpression_S;

typedef struct notify
{
    time_t occurenceTime;
    EVENT_FilterExpression_S stFilterExpression;
}EVENT_Content_S;

typedef struct
{
	EVENT_Type_E enEventType;       		
	EVENT_Content_S stEventContent;
}EVENT_Message_S;

/*Event */
typedef struct
{
    time_t      startTime;
    time_t 		TerminationTime;
    GK_S8   	aszServerAddress[MAX_ADDR_LENGTH];
    GK_S8   	aszClientAddress[MAX_ADDR_LENGTH];
    GK_CHAR     aszTopicExpression[MAX_TOPICEXPRESSION_LEN];
    GK_CHAR     aszMessageContent[MAX_MESSAGECONTENT_LEN];
}EVENT_Handle_S;

typedef struct
{
    GK_CHAR aszEventOccurTime[MAX_TIME_LEN];
    GK_CHAR aszEvent[EVENT_MSG_LEN];
}EVENT_NotificationMessage_S;

typedef struct
{
    GK_S32 eventID;
    time_t terminationTime;
    GK_S32 messageLimit;
    GONVIF_Boolean_E enPullMsg;
}EVENT_PullMessages_S;

typedef struct
{
    GK_CHAR aszCurrentTime[MAX_TIME_LEN];
    GK_CHAR aszTerminationTime[MAX_TIME_LEN];
    GK_S32  sizeNotificationMessage;
    EVENT_NotificationMessage_S *pstNotificationMessage;
}EVENT_PullMessages_Res_S;

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

EVENT_Ret_E EventManagement_Init(void);
EVENT_Ret_E EventManagement_Subscribe(GONVIF_EVENT_Subscribe_S *pstEV_Subscribe, GONVIF_EVENT_Subscribe_Res_S *pstEV_SubscribeRes);//done
EVENT_Ret_E EventManagement_Unsubscribe(GK_S32 index);
EVENT_Ret_E EventManagement_Renew(GK_S32 index, GONVIF_EVENT_Renew_S *pstEV_Renew, GONVIF_EVENT_Renew_Res_S *pstEV_RenewRes);
EVENT_Ret_E EventManagement_CreatePullPointSubscription(GONVIF_EVENT_CreatePullPointSubscription_S *pstEV_CreatePullPointSubscription, GONVIF_EVENT_CreatePullPointSubscription_Res_S *pstEV_CreatePullPointSubscriptionRes);
EVENT_Ret_E EventManagement_PullMessages(GK_S32 index, GK_S32 timeout, GK_S32 messageLimit); //done, not test. 

#ifdef __cplusplus
}
#endif


#endif /* _EVENT_HANDLE_H_ */
