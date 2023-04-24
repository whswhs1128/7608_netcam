/*!
*****************************************************************************
** \file        subsystem/onvif-no-gsoap/src/onvif_ptz.c
**
** \version     $Id$
**
** \brief       onvif ptz
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "onvif_ptz.h"
#include "onvif_priv.h"


#define EPSILON 10e-5

/***************************************************************************
函数描述:  获取云台服务能力集
入口参数：pstPTZ_GetServiceCapabilitiesRes : 服务能力信息
返回值     ：ONVIF_OK: 成功
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_GetServiceCapabilities(GONVIF_PTZ_GetServiceCapabilities_Res_S *pstPTZ_GetServiceCapabilitiesRes)
{

	pstPTZ_GetServiceCapabilitiesRes->stCapabilities.enEFlip = Boolean_FALSE;
	pstPTZ_GetServiceCapabilitiesRes->stCapabilities.enReverse = Boolean_FALSE;
    pstPTZ_GetServiceCapabilitiesRes->stCapabilities.enGetCompatibleConfigurations = Boolean_FALSE;

    return ONVIF_OK;
}

/***************************************************************************
函数描述:  获取云台全部节点信息
入口参数：pstPTZ_GetNodesRes : 云台节点信息
返回值     ：ONVIF_OK: 成功
				ONVIF_ERR_PTZ_NOT_SUPPORTED : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_GetNodes(GONVIF_PTZ_GetNodes_Res_S *pstPTZ_GetNodesRes)
{
    GK_S32 i = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto PTZ_Not_Supported;
    }
    pstPTZ_GetNodesRes->sizePTZNode = g_GkIpc_OnvifInf.sizePTZnode;	// 1
    if(pstPTZ_GetNodesRes->sizePTZNode > MAX_PTZ_NODE_NUM)
    {
        pstPTZ_GetNodesRes->sizePTZNode = MAX_PTZ_NODE_NUM;
    }
    for(i = 0;i < pstPTZ_GetNodesRes->sizePTZNode; i++)
    {
        memcpy(&pstPTZ_GetNodesRes->stPTZNode[i], &g_GkIpc_OnvifInf.pstPTZnode[i], sizeof(GONVIF_PTZ_Node_S));
    }

    return ONVIF_OK;

PTZ_Not_Supported:
    return ONVIF_ERR_PTZ_NOT_SUPPORTED;
}


/***************************************************************************
函数描述:  获取云台全部节点信息
入口参数：pstPTZ_GetNode : 云台节点信息
				pstPTZ_GetNodeRes : 获取云台单个节点返回信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_GetNode(GONVIF_PTZ_GetNode_S *pstPTZ_GetNode, GONVIF_PTZ_GetNode_Res_S *pstPTZ_GetNodeRes)
{
    GK_S32 i = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto PTZ_Not_Supported;
    }
    //check token
    for(i = 0;i < g_GkIpc_OnvifInf.sizePTZnode; i++)
    {
        if(strcmp(pstPTZ_GetNode->aszNodeToken, g_GkIpc_OnvifInf.pstPTZnode[i].token) == 0)
        {
            memcpy(&pstPTZ_GetNodeRes->stPTZNode, &g_GkIpc_OnvifInf.pstPTZnode[i], sizeof(GONVIF_PTZ_Node_S));
            break;
        }
        if(i == (g_GkIpc_OnvifInf.sizePTZnode-1))
        {
            ONVIF_ERR("No such PTZNode on the device.");
            goto No_Such_Node;
        }
    }

    return ONVIF_OK;

PTZ_Not_Supported:
    return ONVIF_ERR_PTZ_NOT_SUPPORTED;
No_Such_Node:
    return ONVIF_ERR_PTZ_NOT_SUCH_NODE;
}


/***************************************************************************
函数描述:  获取云台全部节点信息
入口参数：pstGetConfigurationsRes : 云台全部配置信息
返回值     ：ONVIF_OK: 成功
				ONVIF_ERR_PTZ_NOT_SUPPORTED : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_GetConfigurations(GONVIF_PTZ_GetConfigutations_Res_S *pstGetConfigurationsRes)
{
    GK_S32 i = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto PTZ_Not_Supported;
    }
    pstGetConfigurationsRes->sizePTZConfiguration = g_GkIpc_OnvifInf.PTZConfigurationNum;
    if(pstGetConfigurationsRes->sizePTZConfiguration > MAX_PTZ_CONFIGURATION_NUM)
    {
        pstGetConfigurationsRes->sizePTZConfiguration = MAX_PTZ_CONFIGURATION_NUM;
    }

    for(i = 0; i < pstGetConfigurationsRes->sizePTZConfiguration; i++)
    {
        memcpy(&pstGetConfigurationsRes->stPTZConfiguration[i], &g_GkIpc_OnvifInf.gokePTZConfiguration[i], sizeof(GONVIF_PTZ_Configuration_S));
    }

	return ONVIF_OK;

PTZ_Not_Supported:
    return ONVIF_ERR_PTZ_NOT_SUPPORTED;
}


/***************************************************************************
函数描述:  获取云台单个配置信息
入口参数：pstPTZ_GetConfiguration : 获取云台单个配置请求信息
				pstPTZ_GetConfigurationRes : 获取云台单个配置返回信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_GetConfiguration(GONVIF_PTZ_GetConfiguration_S *pstPTZ_GetConfiguration, GONVIF_PTZ_GetConfiguration_Res_S *pstPTZ_GetConfigurationRes)
{
    GK_S32 i = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto PTZ_Not_Supported;
    }
    for(i = 0;i < g_GkIpc_OnvifInf.PTZConfigurationNum; i++)
    {
        if(strcmp(pstPTZ_GetConfiguration->aszPTZConfiguationToken, g_GkIpc_OnvifInf.gokePTZConfiguration[i].token) == 0)
        {
            memcpy(&pstPTZ_GetConfigurationRes->stPTZConfiguration, &g_GkIpc_OnvifInf.gokePTZConfiguration[i], sizeof(GONVIF_PTZ_Configuration_S));
            break;
        }
        if(i == (g_GkIpc_OnvifInf.PTZConfigurationNum-1))
        {
            ONVIF_ERR("The requested configuration does not exist.");
            goto No_Such_Config;
        }
    }

	return ONVIF_OK;

PTZ_Not_Supported:
    return ONVIF_ERR_PTZ_NOT_SUPPORTED;
No_Such_Config:
    return ONVIF_ERR_PTZ_NO_SUCH_CONFIG;

}

/***************************************************************************
函数描述:  获取云台配置条件信息
入口参数：pstPTZ_GetConfigurationOptions : 获取云台配置条件请求信息
				pstPTZ_GetConfigurationOptionsRes : 获取云台配置条件返回信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_GetConfigurationOptions(GONVIF_PTZ_GetConfigurationOptions_S *pstPTZ_GetConfigurationOptions, GONVIF_PTZ_GetConfigurationOptions_Res_S *pstPTZ_GetConfigurationOptionsRes)
{
    GK_S32 i = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto PTZ_Not_Supported;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.PTZConfigurationNum; i++)
    {
        if(strcmp(pstPTZ_GetConfigurationOptions->aszConfigurationToken, g_GkIpc_OnvifInf.gokePTZConfiguration[i].token) == 0)
        {
        	if(i >= g_GkIpc_OnvifInf.sizePTZConfigurationOptions)
        	{
				ONVIF_ERR("Have no corresponding PTZ configuration options.");
				goto Run_Error;
        	}
            memcpy(&pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions, &g_GkIpc_OnvifInf.pstPTZConfigurationOptions[i], sizeof(GONVIF_PTZ_ConfigurationOptions_S));
            break;
        }
        if(i == (g_GkIpc_OnvifInf.PTZConfigurationNum-1))
        {
            ONVIF_ERR("The requested configuration does not exist.");
            goto No_Such_Config;
        }
    }

    return ONVIF_OK;

PTZ_Not_Supported:
    return ONVIF_ERR_PTZ_NOT_SUPPORTED;
No_Such_Config:
    return ONVIF_ERR_PTZ_NO_SUCH_CONFIG;
Run_Error:
	return ONVIF_ERR_PTZ_SERVER_RUN_ERROR;
}


/***************************************************************************
函数描述:  设置云台配置信息
入口参数：pstPTZ_SetConfiguration : 云台配置信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_SetConfiguration(GONVIF_PTZ_SetConfiguration_S *pstPTZ_SetConfiguration)
{
    GK_S32 i = 0;

    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto PTZ_Not_Supported;
    }
    for(i = 0;i < g_GkIpc_OnvifInf.PTZConfigurationNum; i++)
    {
        if(strcmp(pstPTZ_SetConfiguration->stPTZConfiguration.token, g_GkIpc_OnvifInf.gokePTZConfiguration[i].token) == 0)
        {
        	memset(&g_GkIpc_OnvifInf.gokePTZConfiguration[i], 0, sizeof(GONVIF_PTZ_Configuration_S));
            memcpy(&g_GkIpc_OnvifInf.gokePTZConfiguration[i], &pstPTZ_SetConfiguration->stPTZConfiguration, sizeof(GONVIF_PTZ_Configuration_S));
            break;
        }
        if(i == (g_GkIpc_OnvifInf.PTZConfigurationNum-1))
        {
            ONVIF_ERR("The requested configuration does not exist.");
            goto No_Such_Config;
        }
    }
	if(pstPTZ_SetConfiguration->enForcePersistence == Boolean_TRUE)
	{
	    GK_NVT_SaveXmlConfig(XML_MEIDA);
	}
    return ONVIF_OK;

PTZ_Not_Supported:
    return ONVIF_ERR_PTZ_NOT_SUPPORTED;
No_Such_Config:
    return ONVIF_ERR_PTZ_NO_SUCH_CONFIG;
}

ONVIF_RET GK_NVT_PTZ_ContinuousMove(GONVIF_PTZ_ContinuousMove_S *pstPTZ_ContinuousMove)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
#if 0	//standard onvif
    GK_S32 k = 0;
    GK_S32 n = 0;
    GK_FLOAT panMin  = 0;
    GK_FLOAT panMax  = 0;
    GK_FLOAT tileMin = 0;
    GK_FLOAT tileMax = 0;
    GK_FLOAT zoomMin = 0;
    GK_FLOAT zoomMax = 0;
#endif
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto Not_Supported;
    }
    //check profile token to find which profile user want to set. i : which profile
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken != NULL)
    	{
	        if(strcmp(pstPTZ_ContinuousMove->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	            break;
		}
    }
    if(i >= g_GkIpc_OnvifInf.ProfileNum)
    {
        ONVIF_ERR("Profile token does not exist.");
        goto Invalid_Profile;
    }
    //if profile which is found has a PTZ configuraion, the check node token to find which node the configutaion belongs to. j : which PTZ node
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.sizePTZnode; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->nodeToken,
                g_GkIpc_OnvifInf.pstPTZnode[j].token) == 0)
                break;
        }
        if(j >= g_GkIpc_OnvifInf.sizePTZnode)
        {
            ONVIF_ERR("Configuration has no such token PTZ node.");
            goto Not_Supported;
        }
    }
    else
    {
        ONVIF_ERR("Profile token does not reference a PTZ configuration.");
        goto No_PTZ_Config;
    }
#if 0	//standard onvif
    //if the PTZ node is found, find space in corresponding PTZ node.
    //if the node has no corresponding space, use its default space  in PTZconfiguration to find corresponding coordinate systems.
    if(pstPTZ_ContinuousMove->stVelocity.stPanTilt.aszSpace[0] != '\0')
    {
        for(k = 0; k < g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.sizeContinuousPanTiltVelocitySpace; k++)
        {
            if(strcmp(g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[k].aszURI,
            	pstPTZ_ContinuousMove->stVelocity.stPanTilt.aszSpace) == 0)
            {
            	panMin  = g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[k].stXRange.min;
            	panMax  = g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[k].stXRange.max;
            	tileMin = g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[k].stYRange.min;
            	tileMax = g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[k].stYRange.max;
                break;
	        }
        }
        if(k >= g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.sizeContinuousPanTiltVelocitySpace)
        {
            ONVIF_INFO("Unsupported Continuous PanTilt space in node, use default space.");
			for(k = 0; k < g_GkIpc_OnvifInf.PTZConfigurationNum; k++)
			{
				if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->token, g_GkIpc_OnvifInf.gokePTZConfiguration[k].token) == 0)
				{
					for(n = 0; n < g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.sizeContinuousPanTiltVelocitySpace; n++)
					{
						if(strcmp(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.stContinuousPanTiltVelocitySpace[n].aszURI,
							pstPTZ_ContinuousMove->stVelocity.stPanTilt.aszSpace) == 0)
						{
							panMin	= g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.stContinuousPanTiltVelocitySpace[n].stXRange.min;
							panMax	= g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.stContinuousPanTiltVelocitySpace[n].stXRange.max;
							tileMin = g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.stContinuousPanTiltVelocitySpace[n].stYRange.min;
							tileMax = g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.stContinuousPanTiltVelocitySpace[n].stYRange.max;
							break;
						}
					}
					if(n >= g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.sizeContinuousPanTiltVelocitySpace)
					{
						ONVIF_INFO("Unsupported defualt Continuous PanTilt space in Configuration options.");
						goto Unsupported_Space;
					}
					break;
 				}
 			}
			if(k >= g_GkIpc_OnvifInf.PTZConfigurationNum)
			{
				ONVIF_ERR("Profile token does not reference a PTZ configuration.");
            	goto No_PTZ_Config;
            }
        }
    }
    if(pstPTZ_ContinuousMove->stVelocity.stZoom.aszSpace[0] != '\0')
    {
        for(k = 0; k < g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.sizeContinuousZoomVelocitySpace; k++)
        {
            if(strcmp(g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.stContinuousZoomVelocitySpace[k].aszURI,
            	pstPTZ_ContinuousMove->stVelocity.stZoom.aszSpace) == 0)
            {
            	zoomMin  = g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.stContinuousZoomVelocitySpace[k].stXRange.min;
            	zoomMax  = g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.stContinuousZoomVelocitySpace[k].stXRange.max;
                break;
	        }
        }
        if(k >= g_GkIpc_OnvifInf.pstPTZnode[j].stSupportedPTZSpaces.sizeContinuousZoomVelocitySpace)
        {
            ONVIF_INFO("Unsupported Continuous Zoom space in node, use default space.");
			for(k = 0; k < g_GkIpc_OnvifInf.PTZConfigurationNum; k++)
			{
				if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->token, g_GkIpc_OnvifInf.gokePTZConfiguration[k].token) == 0)
				{
					for(n = 0; n < g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.sizeContinuousZoomVelocitySpace; n++)
					{
						if(strcmp(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.stContinuousZoomVelocitySpace[n].aszURI,
							pstPTZ_ContinuousMove->stVelocity.stZoom.aszSpace) == 0)
						{
							zoomMin	= g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.stContinuousZoomVelocitySpace[n].stXRange.min;
							zoomMax	= g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.stContinuousZoomVelocitySpace[n].stXRange.max;
							break;
						}
					}
					if(n >= g_GkIpc_OnvifInf.pstPTZConfigurationOptions[k].stSpaces.sizeContinuousZoomVelocitySpace)
					{
						ONVIF_INFO("Unsupported defualt Continuous Zoom space in Configuration options.");
						goto Unsupported_Space;
					}
					break;
 				}
 			}
			if(k >= g_GkIpc_OnvifInf.PTZConfigurationNum)
			{
				ONVIF_ERR("Profile token does not reference a PTZ configuration.");
            	goto No_PTZ_Config;
            }
        }
    }

    //set
    ONVIF_INFO("Continuous\nspeed(%f, %f, %f)\n",
    	pstPTZ_ContinuousMove->stVelocity.stPanTilt.x,
    	pstPTZ_ContinuousMove->stVelocity.stPanTilt.y,
    	pstPTZ_ContinuousMove->stVelocity.stZoom.x);
    ONVIF_PTZContinuousMove_S stContinuousMove;
    memset(&stContinuousMove, 0, sizeof(ONVIF_PTZContinuousMove_S));
    stContinuousMove.panSpeed  = (GK_S32)pstPTZ_ContinuousMove->stVelocity.stPanTilt.x;
	stContinuousMove.tileSpeed = (GK_S32)pstPTZ_ContinuousMove->stVelocity.stPanTilt.y;
	stContinuousMove.zoomSpeed = (GK_S32)pstPTZ_ContinuousMove->stVelocity.stZoom.x;
    ONVIF_INFO("Continuous\nspeed(%d, %d, %d)\n",
    	stContinuousMove.panSpeed,
    	stContinuousMove.tileSpeed,
    	stContinuousMove.zoomSpeed);

    //check
	if(stContinuousMove.panSpeed < (int)panMin || stContinuousMove.panSpeed < (int)panMax ||
		stContinuousMove.tileSpeed < (int)tileMin || stContinuousMove.panSpeed < (int)tileMax ||
		stContinuousMove.zoomSpeed < (int)zoomMin || stContinuousMove.panSpeed < (int)zoomMax)
    {
        ONVIF_ERR("Invalid continuous speed.");
        goto Invalid_Speed;
    }
	//move
	/*
	** TODO: absolute moving
	*/
#endif
    //set
    ONVIF_INFO("Continuous speed(%f, %f, %f)\n",
    	pstPTZ_ContinuousMove->stVelocity.stPanTilt.x,
    	pstPTZ_ContinuousMove->stVelocity.stPanTilt.y,
    	pstPTZ_ContinuousMove->stVelocity.stZoom.x);
    ONVIF_PTZContinuousMove_S stContinuousMove;
    memset(&stContinuousMove, 0, sizeof(ONVIF_PTZContinuousMove_S));
    stContinuousMove.panSpeed  = (GK_S32)((pstPTZ_ContinuousMove->stVelocity.stPanTilt.x)*5);
	stContinuousMove.tileSpeed = (GK_S32)((pstPTZ_ContinuousMove->stVelocity.stPanTilt.y)*5);
	stContinuousMove.zoomSpeed = (GK_S32)((pstPTZ_ContinuousMove->stVelocity.stZoom.x)*5);
    ONVIF_INFO("Continuous speed(%d, %d, %d)\n",
    	stContinuousMove.panSpeed,
    	stContinuousMove.tileSpeed,
    	stContinuousMove.zoomSpeed);

    //check
	if(stContinuousMove.panSpeed < -5 || stContinuousMove.panSpeed > 5 ||
		stContinuousMove.tileSpeed < -5 || stContinuousMove.panSpeed > 5 ||
		stContinuousMove.zoomSpeed < -5 || stContinuousMove.panSpeed > 5)
    {
        ONVIF_ERR("Invalid continuous speed.");
        goto Invalid_Speed;
    }
	//move
	GK_S32 ret = 0;
	ret = g_stPTZAdapter.ContinuousMove(stContinuousMove);
	if(ret != 0)
	{
		ONVIF_ERR("Continuous moving error");
		goto run_error;
	}
    return ONVIF_OK;

Not_Supported:
	return ONVIF_ERR_PTZ_NOT_SUPPORTED;
Invalid_Profile:
	return ONVIF_ERR_PTZ_NOT_SUCH_PROFILE_EXIST;
No_PTZ_Config:
	return ONVIF_ERR_PTZ_NOT_REFERRNCE_PTZCONFIG;
//Unsupported_Space:
	//return ONVIF_ERR_PTZ_NOT_UNSUPPORTED_SPACE;
Invalid_Speed:
	return ONVIF_ERR_PTZ_INVALID_SPEED;
run_error:
	return ONVIF_ERR_PTZ_SERVER_RUN_ERROR;
}

ONVIF_RET GK_NVT_PTZ_Stop(GONVIF_PTZ_Stop_S *pstPTZ_Stop)
{
	GK_S32 i = 0;
	GK_S32 j = 0;

    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto Not_Supported;
    }
    //check profile token to find which profile user want to set. i : which profile
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken != NULL)
    	{
	        if(strcmp(pstPTZ_Stop->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	            break;
		}
    }
    if(i >= g_GkIpc_OnvifInf.ProfileNum)
    {
        ONVIF_ERR("Profile token does not exist.");
        goto Invalid_Profile;
    }
    //if profile which is found has a PTZ configuraion, the check node token to find which node the configutaion belongs to. j : which PTZ node
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.sizePTZnode; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->nodeToken,
                g_GkIpc_OnvifInf.pstPTZnode[j].token) == 0)
                break;
        }
        if(j >= g_GkIpc_OnvifInf.sizePTZnode)
        {
            ONVIF_ERR("Configuration has no such token PTZ node.");
            goto Not_Supported;
        }
    }
    else
    {
        ONVIF_ERR("Profile token does not reference a PTZ configuration.");
        goto No_PTZ_Config;
    }
    GK_S32 ret = 0;
	ONVIF_PTZStop_S stStop;
	memset(&stStop, 0, sizeof(ONVIF_PTZStop_S));
	if(pstPTZ_Stop->enPanTilt == Boolean_TRUE)
		stStop.stopPanTilt = GK_TRUE;
	else
		stStop.stopPanTilt = GK_FALSE;
	if(pstPTZ_Stop->enZoom == Boolean_TRUE)
		stStop.stopZoom = GK_TRUE;
	else
		stStop.stopZoom = GK_FALSE;
	ret = g_stPTZAdapter.Stop(stStop);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to stop PTZ.");
		goto run_error;
	}

	return ONVIF_OK;

Not_Supported:
	return ONVIF_ERR_PTZ_NOT_SUPPORTED;
Invalid_Profile:
	return ONVIF_ERR_PTZ_NOT_SUCH_PROFILE_EXIST;
No_PTZ_Config:
	return ONVIF_ERR_PTZ_NOT_REFERRNCE_PTZCONFIG;
run_error:
	return ONVIF_ERR_PTZ_SERVER_RUN_ERROR;
}

ONVIF_RET GK_NVT_PTZ_GetPresets(GONVIF_PTZ_GetPresets_S *pstPTZ_GetPresets, GONVIF_PTZ_GetPresets_Res_S *pstPTZ_GetPresetsRes)
{
	GK_S32 i = 0;
	GK_S32 j = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto Not_Supported;
    }
    //check profile token to find which profile user want to set. i : which profile
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken != NULL)
    	{
	        if(strcmp(pstPTZ_GetPresets->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	            break;
		}
    }
    if(i >= g_GkIpc_OnvifInf.ProfileNum)
    {
        ONVIF_ERR("The requested profile token ProfileToken does not exist.");
        goto Invalid_Profile;
    }
    //if profile which is found has a PTZ configuraion, the check node token to find which node the configutaion belongs to. j : which PTZ node
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.sizePTZnode; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->nodeToken,
                g_GkIpc_OnvifInf.pstPTZnode[j].token) == 0)
                break;
        }
        if(j >= g_GkIpc_OnvifInf.sizePTZnode)
        {
            ONVIF_ERR("PTZ is not supported.");
            goto Not_Supported;
        }
    }
    else
    {
        ONVIF_ERR("The requested profile token does not reference a PTZ configuration.");
        goto No_PTZ_Config;
    }
	GK_S32 ret = 0;
	ONVIF_PTZ_GetPresets_S stPresets;
	memset(&stPresets, 0, sizeof(ONVIF_PTZ_GetPresets_S));
	ret = g_stPTZAdapter.GetPresets(&stPresets);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to get all presets");
		goto run_error;
	}
	GK_S32 k = 0;
	pstPTZ_GetPresetsRes->sizePreset = stPresets.sizePreset;
	for(k = 0; k < pstPTZ_GetPresetsRes->sizePreset; k++)
	{
		snprintf(pstPTZ_GetPresetsRes->stPreset[k].aszToken, MAX_TOKEN_LENGTH,
			"preset_token_%d", stPresets.stPreset[k].presetNum);
		if(stPresets.stPreset[k].presetName[0] != '\0')	//maybe other client like cms set preset which has no name, if true, we give default name.
			strncpy(pstPTZ_GetPresetsRes->stPreset[k].aszName, stPresets.stPreset[k].presetName, MAX_NAME_LENGTH-1);
		else
			snprintf(pstPTZ_GetPresetsRes->stPreset[k].aszName, MAX_NAME_LENGTH,
				"preset%d", stPresets.stPreset[k].presetNum);
	}

    return ONVIF_OK;

Not_Supported:
	return ONVIF_ERR_PTZ_NOT_SUPPORTED;
Invalid_Profile:
	return ONVIF_ERR_PTZ_NOT_SUCH_PROFILE_EXIST;
No_PTZ_Config:
	return ONVIF_ERR_PTZ_NOT_REFERRNCE_PTZCONFIG;
run_error:
	return ONVIF_ERR_PTZ_SERVER_RUN_ERROR;
}


/***************************************************************************
函数描述:  设置云台预置点
入口参数：pstPTZ_SetPreset : 设置云台预置点请求信息
				pstPTZ_SetPresetRes : 设置云台预置点返回信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_SetPreset(GONVIF_PTZ_SetPreset_S *pstPTZ_SetPreset, GONVIF_PTZ_SetPreset_Res_S *pstPTZ_SetPresetRes)
{
	ONVIF_INFO("\nProfile token: %s\nPreset token: %s\nPreset name: %s\n",
		pstPTZ_SetPreset->aszProfileToken, pstPTZ_SetPreset->aszPresetToken, pstPTZ_SetPreset->aszPresetName);
	GK_S32 i = 0;
	GK_S32 j = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto Not_Supported;
    }
    //check profile token to find which profile user want to set. i : which profile
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken != NULL)
    	{
	        if(strcmp(pstPTZ_SetPreset->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	            break;
		}
    }
    if(i >= g_GkIpc_OnvifInf.ProfileNum)
    {
        ONVIF_ERR("The requested profile token ProfileToken does not exist.");
        goto Invalid_Profile;
    }
    //if profile which is found has a PTZ configuraion, the check node token to find which node the configutaion belongs to. j : which PTZ node
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.sizePTZnode; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->nodeToken,
                g_GkIpc_OnvifInf.pstPTZnode[j].token) == 0)
                break;
        }
        if(j >= g_GkIpc_OnvifInf.sizePTZnode)
        {
            ONVIF_ERR("Configuration has no such token PTZ node.");
            goto Not_Supported;
        }
    }
    else
    {
        ONVIF_ERR("The requested profile token does not reference a PTZ configuration.");
        goto No_PTZ_Config;
    }
    if(strlen(pstPTZ_SetPreset->aszPresetName) > 64 || strchr(pstPTZ_SetPreset->aszPresetName, ' '))
    {
		ONVIF_ERR("The PresetName is either too long or contains invalid characters.");
		goto Invalid_Preset_Name;
    }
	GK_S32 ret = 0;
	ONVIF_PTZ_GetPresets_S stPresets;
	memset(&stPresets, 0, sizeof(ONVIF_PTZ_GetPresets_S));
	ret = g_stPTZAdapter.GetPresets(&stPresets);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to get all presets");
		goto run_error;
	}
	if(stPresets.sizePreset >= MAX_PRESET_NUM)
	{
		ONVIF_ERR("Maximum number of Presets reached.");
		goto Too_Many_Presets;
	}
	GK_S32 n = 0;
	for(n = 0; n < stPresets.sizePreset; n++)
	{
		if(stPresets.stPreset[n].presetName[0] != '\0')
		{
			if(strcmp(stPresets.stPreset[n].presetName, pstPTZ_SetPreset->aszPresetName) == 0)
			{
				ONVIF_ERR("The requested name already exist for another preset.");
				goto Preset_Exist;
			}
		}
	}
	GK_S32 m = 0;
	for(n = 0; n < MAX_PRESET_NUM; n++)
	{
		for(m = 0; m < stPresets.sizePreset; m++)
		{
			if(n == stPresets.stPreset[m].presetNum)
				break;
		}
		if(m >= stPresets.sizePreset)
		{
			ONVIF_INFO("Success to find a unused order number(%d) for new preset in preset order", n);
			break;
		}
	}
	ONVIF_PTZ_Preset_S stPreset;
	memset(&stPreset, 0, sizeof(ONVIF_PTZ_Preset_S));
	stPreset.presetNum = n;
	if(pstPTZ_SetPreset->aszPresetName[0] != '\0')
		strncpy(stPreset.presetName, pstPTZ_SetPreset->aszPresetName, MAX_NAME_LENGTH-1);
	else
		snprintf(stPreset.presetName, MAX_NAME_LENGTH, "preset%d", n);
	ret = g_stPTZAdapter.SetPreset(stPreset);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to set preset");
		goto run_error;
	}
	snprintf(pstPTZ_SetPresetRes->aszPresetToken, MAX_TOKEN_LENGTH, "preset_token_%d", n);

	return ONVIF_OK;

Not_Supported:
	return ONVIF_ERR_PTZ_NOT_SUPPORTED;
Invalid_Profile:
	return ONVIF_ERR_PTZ_NOT_SUCH_PROFILE_EXIST;
No_PTZ_Config:
	return ONVIF_ERR_PTZ_NOT_REFERRNCE_PTZCONFIG;
Too_Many_Presets:
	return ONVIF_ERR_PTZ_TOO_MANY_PRESETS;
Preset_Exist:
	return ONVIF_ERR_PTZ_PRESET_EXIST;
Invalid_Preset_Name:
	return ONVIF_ERR_PTZ_INVALID_PRESET_NAME;
//PTZ_Is_Moving:
	//return ONVIF_ERR_PTZ_IS_MOVING;
//No_Such_Token:
	//return ONVIF_ERR_PTZ_NO_SUCH_PRESET_TOKEN;
run_error:
	return ONVIF_ERR_PTZ_SERVER_RUN_ERROR;
}

/***************************************************************************
函数描述:  调用云台预置点
入口参数：pstPTZ_GotoPreset : 调用云台预置点请求信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_GotoPreset(GONVIF_PTZ_GotoPreset_S *pstPTZ_GotoPreset)
{
	GK_S32 i = 0;
	GK_S32 j = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto Not_Supported;
    }
    //check profile token to find which profile user want to set. i : which profile
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken != NULL)
    	{
	        if(strcmp(pstPTZ_GotoPreset->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	            break;
		}
    }
    if(i >= g_GkIpc_OnvifInf.ProfileNum)
    {
        ONVIF_ERR("The requested profile token ProfileToken does not exist.");
        goto Invalid_Profile;
    }
    //if profile which is found has a PTZ configuraion, the check node token to find which node the configutaion belongs to. j : which PTZ node
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.sizePTZnode; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->nodeToken,
                g_GkIpc_OnvifInf.pstPTZnode[j].token) == 0)
                break;
        }
        if(j >= g_GkIpc_OnvifInf.sizePTZnode)
        {
            ONVIF_ERR("Configuration has no such token PTZ node.");
            goto Not_Supported;
        }
    }
    else
    {
        ONVIF_ERR("The requested profile token does not reference a PTZ configuration.");
        goto No_PTZ_Config;
    }
    //check preset token
	GK_S32 ret = 0;
	ONVIF_PTZ_GetPresets_S stPresets;
	memset(&stPresets, 0, sizeof(ONVIF_PTZ_GetPresets_S));
	ret = g_stPTZAdapter.GetPresets(&stPresets);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to get all presets");
		goto run_error;
	}
	GK_S32 k = 0;
	GK_S32 presetNum = 0;
	while(!isdigit(pstPTZ_GotoPreset->aszPresetToken[k]) && k < MAX_TOKEN_LENGTH)
	{
		k++;
	}
	if(k >= MAX_TOKEN_LENGTH)
	{
		ONVIF_ERR("The requested preset token does not exist.");
		goto No_Such_Preset_Token;
	}
	presetNum = atoi(&pstPTZ_GotoPreset->aszPresetToken[k]);
	ONVIF_INFO("Set preset number: %d", presetNum);
	for(k = 0; k <= stPresets.sizePreset; k++)
	{
		if(presetNum == stPresets.stPreset[k].presetNum)
			break;
	}
	if(k >= stPresets.sizePreset)
	{
		ONVIF_ERR("The requested preset token does not exist.");
		goto No_Such_Preset_Token;
	}
	ret = g_stPTZAdapter.GotoPreset(presetNum);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to goto preset.");
		goto run_error;
	}

	return ONVIF_OK;

Not_Supported:
	return ONVIF_ERR_PTZ_NOT_SUPPORTED;
Invalid_Profile:
	return ONVIF_ERR_PTZ_NOT_SUCH_PROFILE_EXIST;
No_PTZ_Config:
	return ONVIF_ERR_PTZ_NOT_REFERRNCE_PTZCONFIG;
No_Such_Preset_Token:
	return ONVIF_ERR_PTZ_NO_SUCH_PRESET_TOKEN;
//Invalid_Speed:
	//return ONVIF_ERR_PTZ_INVALID_SPEED;
//Unsupported_Space:
	//return ONVIF_ERR_PTZ_NOT_UNSUPPORTED_SPACE;
run_error:
	return ONVIF_ERR_PTZ_SERVER_RUN_ERROR;
}


/***************************************************************************
函数描述:  删除云台预置点
入口参数：pstPTZ_RemovePreset : 删除云台预置点请求信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_RemovePreset(GONVIF_PTZ_RemovePreset_S *pstPTZ_RemovePreset)
{
	GK_S32 i = 0;
	GK_S32 j = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZ is not supported.");
        goto Not_Supported;
    }
    //check profile token to find which profile user want to set. i : which profile
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken != NULL)
    	{
	        if(strcmp(pstPTZ_RemovePreset->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	            break;
		}
    }
    if(i >= g_GkIpc_OnvifInf.ProfileNum)
    {
        ONVIF_ERR("The requested profile token ProfileToken does not exist.");
        goto Invalid_Profile;
    }
    //if profile which is found has a PTZ configuraion, the check node token to find which node the configutaion belongs to. j : which PTZ node
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.sizePTZnode; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->nodeToken,
                g_GkIpc_OnvifInf.pstPTZnode[j].token) == 0)
                break;
        }
        if(j >= g_GkIpc_OnvifInf.sizePTZnode)
        {
            ONVIF_ERR("Configuration has no such token PTZ node.");
            goto Not_Supported;
        }
    }
    else
    {
        ONVIF_ERR("The requested profile token does not reference a PTZ configuration.");
        goto No_PTZ_Config;
    }
	//check preset token
	GK_S32 ret = 0;
	ONVIF_PTZ_GetPresets_S stPresets;
	memset(&stPresets, 0, sizeof(ONVIF_PTZ_GetPresets_S));
	ret = g_stPTZAdapter.GetPresets(&stPresets);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to get all presets");
		goto run_error;
	}
	GK_S32 k = 0;
	GK_S32 presetNum = 0;
	while(!isdigit(pstPTZ_RemovePreset->aszPresetToken[k]) && k < MAX_TOKEN_LENGTH)
	{
		k++;
	}
	if(k >= MAX_TOKEN_LENGTH)
	{
		ONVIF_ERR("The requested preset token does not exist.");
		goto No_Such_Preset_Token;
	}
	presetNum = atoi(&pstPTZ_RemovePreset->aszPresetToken[k]);
	ONVIF_INFO("remove preset number: %d", presetNum);
	ret = g_stPTZAdapter.RemovePreset(presetNum);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to remove preset.");
		goto run_error;
	}

	return ONVIF_OK;

Not_Supported:
	return ONVIF_ERR_PTZ_NOT_SUPPORTED;
Invalid_Profile:
	return ONVIF_ERR_PTZ_NOT_SUCH_PROFILE_EXIST;
No_PTZ_Config:
	return ONVIF_ERR_PTZ_NOT_REFERRNCE_PTZCONFIG;
No_Such_Preset_Token:
	return ONVIF_ERR_PTZ_NO_SUCH_PRESET_TOKEN;
run_error:
	return ONVIF_ERR_PTZ_SERVER_RUN_ERROR;
}


/***************************************************************************
函数描述:  云台绝对运动
入口参数：pstPTZ_AbsoluteMove : 云台绝对运动信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_AbsoluteMove(GONVIF_PTZ_AbsoluteMove_S *pstPTZ_AbsoluteMove)
{
#if 1
    GK_S32 i = 0, j = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZNotSupported.");
        goto Not_Supported;
    }
    //check token
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken != NULL)
    	{
	        if(strcmp(pstPTZ_AbsoluteMove->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	            break;
		}
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        ONVIF_ERR("Profile token does not exist.");
        goto Invalid_Profile;
    }
    //check node
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.sizePTZnode; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->nodeToken,
                g_GkIpc_OnvifInf.pstPTZnode[j].token) == 0)
                break;
        }
        if(j >= g_GkIpc_OnvifInf.sizePTZnode)
        {
            ONVIF_ERR("Configuration has no such token PTZ node.");
            goto Not_Supported;
        }
    }
    else
    {
        ONVIF_ERR("Profile token does not reference a PTZ configuration.");
        goto No_PTZ_Config;
    }
    //set
    ONVIF_PTZAbsoluteMove_S stAbsoluteMove;
    memset(&stAbsoluteMove, 0, sizeof(ONVIF_PTZAbsoluteMove_S));
    stAbsoluteMove.panPosition  = pstPTZ_AbsoluteMove->stPosition.stPanTilt.x;
    stAbsoluteMove.tiltPosition = pstPTZ_AbsoluteMove->stPosition.stPanTilt.y;
    stAbsoluteMove.zoomPosition = pstPTZ_AbsoluteMove->stPosition.stZoom.x;
    stAbsoluteMove.panSpeed     = pstPTZ_AbsoluteMove->stSpeed.stPanTilt.x;
    stAbsoluteMove.tiltSpeed    = pstPTZ_AbsoluteMove->stSpeed.stPanTilt.y;
    stAbsoluteMove.zoomSpeed    = pstPTZ_AbsoluteMove->stSpeed.stZoom.x;
    ONVIF_INFO("Absolute\n(%.6g, %.6g, %.6g)\n(%.6g, %.6g, %.6g)\n",
        stAbsoluteMove.panPosition, stAbsoluteMove.tiltPosition, stAbsoluteMove.zoomPosition,
        stAbsoluteMove.panSpeed, stAbsoluteMove.tiltSpeed, stAbsoluteMove.zoomSpeed);
    //check
	if(stAbsoluteMove.panPosition < -100 || stAbsoluteMove.panPosition > 100 ||
		stAbsoluteMove.tiltPosition < -100 || stAbsoluteMove.tiltPosition > 100 ||
		stAbsoluteMove.zoomPosition < -100 || stAbsoluteMove.zoomPosition > 100)
    {
        ONVIF_ERR("Invalid position.");
        goto Invalid_position;
    }
	if(stAbsoluteMove.panSpeed < 0 || stAbsoluteMove.panSpeed > 100 ||
	    stAbsoluteMove.tiltSpeed < 0 || stAbsoluteMove.tiltSpeed > 100 ||
		stAbsoluteMove.zoomSpeed < 0 || stAbsoluteMove.zoomSpeed > 100)
    {
        ONVIF_ERR("Invalid speed.");
        goto Invalid_Speed;
    }
	//move
	/*
	** TODO: absolute moving
	*/
	if(0 != g_stPTZAdapter.AbsoluteMove(stAbsoluteMove))
        {
        ONVIF_ERR("Absolute moving error");
        goto run_error;
        }

	return ONVIF_OK;

Not_Supported:
    return ONVIF_ERR_PTZ_NOT_SUPPORTED;
Invalid_Profile:
    return ONVIF_ERR_PTZ_NOT_SUCH_PROFILE_EXIST;
No_PTZ_Config:
    return ONVIF_ERR_PTZ_NOT_REFERRNCE_PTZCONFIG;
//Unsupported_Space:
    //return ONVIF_ERR_PTZ_NOT_UNSUPPORTED_SPACE;
Invalid_position:
    return ONVIF_ERR_PTZ_INVALID_POSITION;
Invalid_Speed:
    return ONVIF_ERR_PTZ_INVALID_SPEED;
run_error:
    return ONVIF_ERR_PTZ_SERVER_RUN_ERROR;
#else


	return ONVIF_OK;
#endif
}


/***************************************************************************
函数描述:  云台相对运动
入口参数：pstPTZ_RelativeMove : 云台相对运动信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_RelativeMove(GONVIF_PTZ_RelativeMove_S *pstPTZ_RelativeMove)
{
#if 1
    GK_S32 i = 0;
    GK_S32 j = 0;
    if(g_GkIpc_OnvifInf.sizePTZnode <= 0)
    {
        ONVIF_ERR("PTZNotSupported.");
        goto Not_Supported;
    }
    //check profile
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken != NULL)
    	{
	        if(strcmp(pstPTZ_RelativeMove->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        ONVIF_ERR("Profile token does not exist.");
        goto Invalid_Profile;
    }
    //check node
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.sizePTZnode; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->nodeToken,
                g_GkIpc_OnvifInf.pstPTZnode[j].token) == 0)
                break;
        }
        if(j >= g_GkIpc_OnvifInf.sizePTZnode)
        {
            ONVIF_ERR("Configuration has no such token PTZ node.");
            goto Not_Supported;
        }
    }
    else
    {
        ONVIF_ERR("Profile token does not reference a PTZ configuration.");
        goto No_PTZ_Config;
    }
    //check space
    if(pstPTZ_RelativeMove->stTranslation.stPanTilt.aszSpace[0] != '\0')
    {
        if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->defaultRelativePanTiltTranslationSpace,
            pstPTZ_RelativeMove->stTranslation.stPanTilt.aszSpace) != 0)
        {
            ONVIF_ERR("Translation PanTilt space unsupported");
            goto Unsupported_Space;
        }
    }
    if(pstPTZ_RelativeMove->stTranslation.stZoom.aszSpace[0] != '\0')
    {
        if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->defaultRelativeZoomTranslationSpace,
            pstPTZ_RelativeMove->stTranslation.stZoom.aszSpace) != 0)
        {
            ONVIF_ERR("Translation Zoom space unsupported.");
            goto Unsupported_Space;
        }
    }
    if(pstPTZ_RelativeMove->stSpeed.stPanTilt.aszSpace[0] != '\0')
    {
        if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->stDefaultPTZSpeed.stPanTilt.aszSpace,
            pstPTZ_RelativeMove->stSpeed.stPanTilt.aszSpace) != 0)
        {
            ONVIF_ERR("Zoom speed space unsupported.");
            goto Unsupported_Space;
        }
    }
    if(pstPTZ_RelativeMove->stSpeed.stZoom.aszSpace[0] != '\0')
    {
        if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->stDefaultPTZSpeed.stZoom.aszSpace,
            pstPTZ_RelativeMove->stSpeed.stZoom.aszSpace) != 0)
        {
            ONVIF_ERR("Zoom speed space unsupported.");
            goto Unsupported_Space;
        }
    }
    //Set
    ONVIF_PTZRelativeMove_S stRelativeMove;
    memset(&stRelativeMove, 0, sizeof(ONVIF_PTZRelativeMove_S));
    stRelativeMove.panTranslation  = pstPTZ_RelativeMove->stTranslation.stPanTilt.x;
    stRelativeMove.tiltTranslation = pstPTZ_RelativeMove->stTranslation.stPanTilt.y;
    stRelativeMove.zoomTranslation = pstPTZ_RelativeMove->stTranslation.stZoom.x;
    stRelativeMove.panSpeed        = pstPTZ_RelativeMove->stSpeed.stPanTilt.x;
    stRelativeMove.tiltSpeed       = pstPTZ_RelativeMove->stSpeed.stPanTilt.y;
    stRelativeMove.zoomSpeed       = pstPTZ_RelativeMove->stSpeed.stZoom.x;
	//check
    ONVIF_INFO("Translation\n(%.6g, %.6g, %.6g)\n(%.6g, %.6g, %.6g)\n",
        pstPTZ_RelativeMove->stTranslation.stPanTilt.x, pstPTZ_RelativeMove->stTranslation.stPanTilt.y, pstPTZ_RelativeMove->stTranslation.stZoom.x,
        pstPTZ_RelativeMove->stSpeed.stPanTilt.x, pstPTZ_RelativeMove->stSpeed.stPanTilt.y, pstPTZ_RelativeMove->stSpeed.stZoom.x);
	if(stRelativeMove.panTranslation < -100 || stRelativeMove.panTranslation > 100 ||
		stRelativeMove.tiltTranslation < -100 || stRelativeMove.tiltTranslation > 100 ||
		stRelativeMove.zoomTranslation < -100 || stRelativeMove.zoomTranslation > 100)
    {
        ONVIF_ERR("Invalid translation.");
        goto Invalid_Trans;
    }
	if(stRelativeMove.panSpeed < 0 || stRelativeMove.panSpeed > 100 ||
	    stRelativeMove.tiltSpeed < 0 || stRelativeMove.tiltSpeed > 100 ||
		stRelativeMove.zoomSpeed < 0 || stRelativeMove.zoomSpeed > 100)

    {
        ONVIF_ERR("Invalid speed.");
        goto Invalid_Speed;
    }
    //Stop
    ONVIF_PTZStop_S stStop;
    memset(&stStop, 0, sizeof(ONVIF_PTZStop_S));
    if((stRelativeMove.panSpeed > -EPSILON && stRelativeMove.panSpeed < EPSILON) ||
        (stRelativeMove.tiltSpeed> -EPSILON && stRelativeMove.tiltSpeed < EPSILON))
    {
        stStop.stopPanTilt = GK_TRUE;
        if(0 != g_stPTZAdapter.Stop(stStop))
        {
            ONVIF_ERR("Fail to stop PanTile.");
            goto run_error;
        }
    }
    memset(&stStop, 0, sizeof(ONVIF_PTZStop_S));
    if(stRelativeMove.zoomSpeed > -EPSILON && stRelativeMove.zoomSpeed < EPSILON)
    {
        stStop.stopZoom= GK_TRUE;
        if(0 != g_stPTZAdapter.Stop(stStop))
        {
            ONVIF_ERR("Fail to stop Zoom.");
            goto run_error;
        }
    }
    //Move
	if(0 != g_stPTZAdapter.RelativeMove(stRelativeMove))
	{
        ONVIF_ERR("Relative moving error");
        goto run_error;
	}

    return ONVIF_OK;

Not_Supported:
    return ONVIF_ERR_PTZ_NOT_SUPPORTED;
Invalid_Profile:
    return ONVIF_ERR_PTZ_NOT_SUCH_PROFILE_EXIST;
No_PTZ_Config:
    return ONVIF_ERR_PTZ_NOT_REFERRNCE_PTZCONFIG;
Unsupported_Space:
    return ONVIF_ERR_PTZ_NOT_UNSUPPORTED_SPACE;
Invalid_Trans:
    return ONVIF_ERR_PTZ_INVALID_TRANSLATION;
Invalid_Speed:
    return ONVIF_ERR_PTZ_INVALID_SPEED;
run_error:
    return ONVIF_ERR_PTZ_SERVER_RUN_ERROR;
#else




	return ONVIF_OK;
#endif
}



/***************************************************************************
函数描述:  获取云台状态
入口参数：pstGetStatusReq : 获取云台状态请求信息
				pstGetStatusRes : 获取云台状态返回信息
返回值     ：ONVIF_OK: 成功
				非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_PTZ_GetStatus(GONVIF_PTZ_GetStatus_Token_S *pstGetStatusReq, GONVIF_PTZ_GetStatus_S *pstGetStatusRes)
{
    GK_S32 i = 0;

	if (pstGetStatusReq->aszProfileToken[0] == '\0')
    {
        goto InvalArgValNull;
	}

    if(g_GkIpc_OnvifInf.PTZConfigurationNum == 0)
    {
        goto NoPTZSupport;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstGetStatusReq->aszProfileToken,g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto InvalArgValNoProrileExist;
    }

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration == NULL)
    {
        goto InvalArgValNoConfigExist;
    }

	time_t nowtime;
	struct tm *ptztime;
	time(&nowtime);
	ptztime = localtime(&nowtime);

	snprintf(pstGetStatusRes->aszUtcTime, MAX_TIME_LEN, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		ptztime->tm_year+1900, ptztime->tm_mon+1, ptztime->tm_mday,
		ptztime->tm_hour, ptztime->tm_min, ptztime->tm_sec);

    pstGetStatusRes->stPosition.stPanTilt.x = 0;
    pstGetStatusRes->stPosition.stPanTilt.y = 0;
    strcpy(pstGetStatusRes->stPosition.stPanTilt.aszSpace,"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace");

    pstGetStatusRes->stPosition.stZoom.x = 0;
    strcpy(pstGetStatusRes->stPosition.stZoom.aszSpace,"http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace");

    pstGetStatusRes->stMoveStatus.enPanTilt = PTZ_MoveStatus_IDLE;
    pstGetStatusRes->stMoveStatus.enZoom = PTZ_MoveStatus_IDLE;

    return ONVIF_OK;

InvalArgValNull:
    return ONVIF_ERR_MISSINGATTR;
NoPTZSupport:
    return ONVIF_ERR_MEDIA_NO_PTZ_SUPPORTED;
InvalArgValNoProrileExist:
    return ONVIF_ERR_NO_PROFILE;
InvalArgValNoConfigExist:
    return ONVIF_ERR_NO_PTZCFG;
}

