/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/onvif_event.c
**
** \brief       onvif event manage
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "onvif_event.h"
#include "onvif_priv.h"
#include "event_handle.h"

const char *md_msg_desc = 
    "<tns1:RuleEngine>"
        "<CellMotionDetector>"
            "<Motion wstop:topic=\"true\">"
                "<tt:MessageDescription IsProperty=\"true\">"                                                                             
                    "<tt:Source>"
                        //"<tt:SimpleItemDescription Name=\"VideoSourceConfiguration\" Type=\"tt:ReferenceToken\"/>"
                        //"<tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
                        //"<tt:SimpleItemDescription Name=\"Rule\" Type=\"xs:string\"/>"
                        "<tt:SimpleItemDescription Name=\"VideoAnalytics\" Type=\"xs:string\"/>"
                    "</tt:Source>"
                    "<tt:Data>"
                        "<tt:SimpleItemDescription Name=\"IsMotion\" Type=\"xs:boolean\"/>"
                    "</tt:Data>"
                "</tt:MessageDescription>"
            "</Motion>"
        "</CellMotionDetector>"
    "</tns1:RuleEngine>";
const char *vt_msg_desc = 
    "<tns1:VideoAnalytics>"
        "<VideoTamperDetector>"
            "<Tamper wstop:topic=\"true\">"
                "<tt:MessageDescription IsProperty=\"true\">"                                                                                    
                    "<tt:Source>"
                        "<tt:SimpleItemDescription Name=\"VideoAnalytics\" Type=\"xs:string\"/>"
                    "</tt:Source>"
                    "<tt:Data>"
                        "<tt:SimpleItemDescription Name=\"IsTamper\" Type=\"xs:boolean\"/>"
                    "</tt:Data>"
                "</tt:MessageDescription>"
            "</Tamper>"
        "</VideoTamperDetector>"
    "</tns1:VideoAnalytics>";



/***************************************************************************
函数描述:  获取事件服务能力集
入口参数：pstSoap : 
				pstEV_GetServiceCapabilitiesRes : 事件服务能力集信息
返回值     ：ONVIF_OK: 成功
***************************************************************************/
ONVIF_RET GK_NVT_Event_GetServiceCapabilities(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_GetServiceCapabilities_Res_S *pstEV_GetServiceCapabilitiesRes)
{
    pstSoap->stHeader.pszAction = "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetServiceCapabilitiesResponse";
    
    pstEV_GetServiceCapabilitiesRes->stCapabilities.enWSSubscriptionPolicySupport = Boolean_FALSE;
    pstEV_GetServiceCapabilitiesRes->stCapabilities.enWSPullPointSupport = Boolean_TRUE;
    pstEV_GetServiceCapabilitiesRes->stCapabilities.enWSPausableSubscriptionManagerInterfaceSupport = Boolean_FALSE;
    pstEV_GetServiceCapabilitiesRes->stCapabilities.maxNotificationProducers = 10; 
    pstEV_GetServiceCapabilitiesRes->stCapabilities.maxPullPoints = 5;  
    pstEV_GetServiceCapabilitiesRes->stCapabilities.enPersistentNotificationStorage = Boolean_FALSE;  
    
    return ONVIF_OK;
}

/***************************************************************************
函数描述:  获取事件属性信息
入口参数：pstSoap : 
				pstEV_GetEventPropertiesRes : 事件属性信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_Event_GetEventProperties(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_GetEventProperties_Res_S *pstEV_GetEventPropertiesRes)
{
    pstSoap->stHeader.pszAction = "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesResponse";

    pstEV_GetEventPropertiesRes->sizeTopicNamespaceLocation = 1;
    sprintf(pstEV_GetEventPropertiesRes->aszTopicNamespaceLocation[0], "http://www.onvif.org/ver10/topics/topicns.xml");

    pstEV_GetEventPropertiesRes->enFixedTopicSet = Boolean_TRUE;

    pstEV_GetEventPropertiesRes->sizeTopicExpressionDialect = 2;
    sprintf(pstEV_GetEventPropertiesRes->aszTopicExpressionDialect[0], "http://docs.oasis-open.org/wsn/t-1/TopicExpression/Concrete");
    sprintf(pstEV_GetEventPropertiesRes->aszTopicExpressionDialect[1], "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");

    pstEV_GetEventPropertiesRes->sizeMessageContentFilterDialect = 2;
    sprintf(pstEV_GetEventPropertiesRes->aszMessageContentFilterDialect[0], "http://www.onvif.org/ver10/tev/MessageContentFilterDialect/ItemFilter");
    sprintf(pstEV_GetEventPropertiesRes->aszMessageContentFilterDialect[1], "http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter");

    pstEV_GetEventPropertiesRes->sizeMessageContentSchemaLocation = 1;
    sprintf(pstEV_GetEventPropertiesRes->aszMessageContentSchemaLocation[0], "http://www.onvif.org/ver10/schema/onvif.xsd");

    memset(pstEV_GetEventPropertiesRes->stTopicSet.any, 0, MAX_ANY_LEN*MAX_ANY_NUM);
    GK_S32 i = 0;
    const GK_CHAR *meg_desc[] = {md_msg_desc, vt_msg_desc};
    for(i = 0; i < sizeof(meg_desc)/sizeof(meg_desc[0]); i++)
    {
        if(i >= MAX_ANY_NUM)
            break;
            
        snprintf(pstEV_GetEventPropertiesRes->stTopicSet.any[i], MAX_ANY_LEN, meg_desc[i]);
    }
    if(i >= MAX_ANY_NUM)
    {
        pstEV_GetEventPropertiesRes->stTopicSet.size = MAX_ANY_NUM;
    }
    else
    {
        pstEV_GetEventPropertiesRes->stTopicSet.size = sizeof(meg_desc)/sizeof(meg_desc[0]);
    }
    
    return ONVIF_OK;

}

ONVIF_RET GK_NVT_Event_Subscribe(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_Subscribe_S *pstEV_Subscribe, GONVIF_EVENT_Subscribe_Res_S *pstEV_SubscribeRes)
{
    ONVIF_DBG("[Subscribe]pstEV_Subscribe->stConsumerReference.aszAddress: %s\n"
        "[Subscribe]pstEV_Subscribe->aszInitialTerminationTime: %s\n"
        "[Subscribe]pstEV_Subscribe->stFilter.aszTopicExpression: %s\n"
        "[Subscribe]pstEV_Subscribe->stFilter.aszMessageContent: %s\n",
        pstEV_Subscribe->stConsumerReference.aszAddress,
        pstEV_Subscribe->aszInitialTerminationTime,
        pstEV_Subscribe->stFilter.aszTopicExpression,
        pstEV_Subscribe->stFilter.aszMessageContent);
        
    EVENT_Ret_E ret = EVENT_MANAGE_OK;
    ret = EventManagement_Subscribe(pstEV_Subscribe, pstEV_SubscribeRes);
    if(ret == EVENT_MANAGE_HANDLE_OVERFLOW)
    {
        ONVIF_ERR("[Subscribe]Subscribe overflow.");
        pstSoap->stHeader.pszAction = "http://www.w3.org/2005/08/addressing/soap/fault";
        goto subscribe_overflow;
    }
    
    pstSoap->stHeader.pszAction = "http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/SubscribeResponse";

    return ONVIF_OK;

subscribe_overflow:
    return ONVIF_ERR_EVENT_SUBSCRIBE_OVERFLOW;
}

/***************************************************************************
函数描述:  取消订阅事件
入口参数：pstSoap : 
				pstEV_Unsubscribe : 取消订阅请求信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_Event_Unsubscribe(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_Unsubscribe_S *pstEV_Unsubscribe)
{   
    (void)pstEV_Unsubscribe;
    
    ONVIF_DBG("\n[Unsubscribe]\npstSoap->stHeader.aszTo: %s\n", pstSoap->stHeader.aszTo);

    
    GK_S32 index = 0;
    sscanf(pstSoap->stHeader.aszTo, "%*[^=]=%i", &index);
    EVENT_Ret_E ret = EVENT_MANAGE_OK;
    ret = EventManagement_Unsubscribe(index);
    if(ret == EVENT_MANAGE_HANDLE_NOTEXIST)
    {
        ONVIF_ERR("[Unsubscribe]Subscription not exist.");
        
        pstSoap->stHeader.pszAction = "http://www.w3.org/2005/08/addressing/soap/fault";
        goto event_notexist;
    }
    else if(ret == EVENT_MANAGE_HANDLE_FREE_FAILED)
    {
        ONVIF_ERR("[Unsubscribe]Unsubscribe failed.");
        pstSoap->stHeader.pszAction = "http://www.w3.org/2005/08/addressing/soap/fault";
        goto run_error;
    }
    
    pstSoap->stHeader.pszAction = "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeResponse";
    return ONVIF_OK;
    
event_notexist:
    return ONVIF_ERR_EVENT_SUBSCRIPTION_NOTEXIST;
run_error:
    return ONVIF_ERR_EVENT_SERVER_RUN_ERROR;
}

/***************************************************************************
函数描述:  更新事件
入口参数：pstSoap : 
				pstEV_Renew : 更新事件请求信息
				pstEV_RenewRes : 更新事件回复信息
返回值     ：ONVIF_OK: 成功
				ONVIF_ERR_EVENT_SUBSCRIPTION_NOTEXIST : 失败
***************************************************************************/
ONVIF_RET GK_NVT_Event_Renew(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_Renew_S *pstEV_Renew, GONVIF_EVENT_Renew_Res_S *pstEV_RenewRes)
{
    ONVIF_DBG("[Renew]\npstSoap->stHeader.aszTo: %s\n"
        "pstEV_Renew->aszTerminationTime: %s\n",
        pstSoap->stHeader.aszTo,
        pstEV_Renew->aszTerminationTime);

        
    GK_S32 index = 0;    
    sscanf(pstSoap->stHeader.aszTo, "%*[^=]=%i", &index);
    EVENT_Ret_E ret = EVENT_MANAGE_OK; 
    ret = EventManagement_Renew(index, pstEV_Renew, pstEV_RenewRes);
    if(ret == EVENT_MANAGE_HANDLE_NOTEXIST)
    {
        ONVIF_ERR("[Renew]Subscription not exist.");
        pstSoap->stHeader.pszAction = "http://www.w3.org/2005/08/addressing/soap/fault";
        goto event_notexist;
    }
    
    pstSoap->stHeader.pszAction = "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewResponse";
    return ONVIF_OK;
    
event_notexist:
    return ONVIF_ERR_EVENT_SUBSCRIPTION_NOTEXIST;
}


/***************************************************************************
函数描述:  创建拉取点事件
入口参数：pstSoap : 
				pstEV_CreatePullPointSubscription : 创建拉取点事件请求信息
				pstEV_CreatePullPointSubscriptionRes : 创建拉取点事件回复信息
返回值     ：ONVIF_OK: 成功
				非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Event_CreatePullPointSubscription(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_CreatePullPointSubscription_S *pstEV_CreatePullPointSubscription, GONVIF_EVENT_CreatePullPointSubscription_Res_S *pstEV_CreatePullPointSubscriptionRes)
{
    ONVIF_DBG("[Pull]pstEV_Subscribe->initialTerminationTime: %s\n"
        "[Pull]pstEV_Subscribe->stFilter.aszTopicExpression: %s\n"
        "[Pull]pstEV_Subscribe->stFilter.aszMessageContent: %s\n",
        pstEV_CreatePullPointSubscription->aszInitialTerminationTime,
        pstEV_CreatePullPointSubscription->stFilter.aszTopicExpression,
        pstEV_CreatePullPointSubscription->stFilter.aszMessageContent);
        
    EVENT_Ret_E ret = EVENT_MANAGE_OK;    
    ret = EventManagement_CreatePullPointSubscription(pstEV_CreatePullPointSubscription, pstEV_CreatePullPointSubscriptionRes);
    if(ret == EVENT_MANAGE_HANDLE_OVERFLOW)
    {
        ONVIF_ERR("[Subscribe]Event management overflow.");
        pstSoap->stHeader.pszAction = "http://www.w3.org/2005/08/addressing/soap/fault";
        goto event_overflow;
    }
    else if(ret == EVENT_MANAGE_INVALID_TOPICEXPRESSION)
    {
        ONVIF_ERR("[CreatePullPointSubscription]Invaild topic expression.");
        pstSoap->stHeader.pszAction = "http://www.w3.org/2005/08/addressing/soap/fault";
        goto InvalidTopicExpression;
    }
    
    pstSoap->stHeader.pszAction = "http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionResponse";
    return ONVIF_OK;

event_overflow:
    return ONVIF_ERR_EVENT_SUBSCRIBE_OVERFLOW;
InvalidTopicExpression:
    return ONVIF_ERR_EVENT_INVALID_TOPICEXPRESSION;

}


/***************************************************************************
函数描述:  拉取消息
入口参数：pstSoap : 
				pstEV_PullMessages : 创建拉取点事件请求信息
				pstEV_PullMessagesRes : 创建拉取点事件回复信息
返回值     ：ONVIF_OK: 成功
				非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Event_PullMessages(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_PullMessages_S *pstEV_PullMessages, GONVIF_EVENT_PullMessages_Res_S *pstEV_PullMessagesRes)
{
    GK_S32 index = 0;
    if(pstSoap->stHeader.aszTo[0] == '\0' || pstEV_PullMessages->messageLimit == 0)
    {
        goto invalid_addr;
    }

    sscanf(pstSoap->stHeader.aszTo, "%*[^=]=%i", &index);
    EVENT_Ret_E ret;
    ret = EventManagement_PullMessages(index, pstEV_PullMessages->timeout, pstEV_PullMessages->messageLimit);
    if(ret == EVENT_MANAGE_HANDLE_NOTEXIST)
    {
        ONVIF_ERR("[PullMessages]Subscription not exist.");        
        pstSoap->stHeader.pszAction = "http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionResponse";
        goto event_notexist;
    }
    
    pstSoap->stHeader.pszAction = "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesResponse";
    return ONVIF_OK;
    
invalid_addr:
    return ONVIF_ERR_EVENT_INVALID_SUBSCRIPTION_ADDR;
event_notexist:
    return ONVIF_ERR_EVENT_SUBSCRIPTION_NOTEXIST;

}


/***************************************************************************
函数描述:  设置事件同步点
入口参数：pstSoap : 
				pstSetSynchronizationPointReq : 设置事件同步点请求信息
返回值     ：ONVIF_OK: 成功
***************************************************************************/
ONVIF_RET GK_NVT_Event_SetSynchronizationPoint(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_SetSynchronizationPoint_S *pstSetSynchronizationPointReq)
{
    pstSoap->stHeader.pszAction = "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/SetSynchronizationPointResponse";
    
    return ONVIF_OK;
}

