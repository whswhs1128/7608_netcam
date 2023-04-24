#include "onvif_priv.h"
#include "onvif_analytics.h"

ONVIF_RET GK_NVT_Analytics_GetServiceCapabilities(GONVIF_ANALYTICS_GetServiceCapabilities_Res_S *pstAN_GetServiceCapabilitiesRes)
{
    pstAN_GetServiceCapabilitiesRes->stCapabilities.size = 0;
    pstAN_GetServiceCapabilitiesRes->stCapabilities.enRuleSupport = Boolean_TRUE;
    pstAN_GetServiceCapabilitiesRes->stCapabilities.enAnalyticsModulesSupport = Boolean_TRUE;
    pstAN_GetServiceCapabilitiesRes->stCapabilities.enCellBasedSceneDescriptionSupported = Boolean_FALSE;
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_Analytics_GetSupportedRules(GONVIF_ANALYTICS_GetSupportedRules_S *pstGetSupportedRulesReq, GONVIF_ANALYITICS_GetSupportedRulesRes_S *pstGetSupportedRulesRes)
{
	ONVIF_INFO("Token: %s\n", pstGetSupportedRulesReq->aszConfigurationToken);
	
	GK_S32 index;
	for(index = 0; index < g_GkIpc_OnvifInf.VideoAnalyticNum; index++)
	{
		if(strcmp(pstGetSupportedRulesReq->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszToken) == 0)
			break;
	}
	if(index >=g_GkIpc_OnvifInf.VideoAnalyticNum)
	{
		return ONVIF_ERR_ANALYTICS_INVALID_TOKEN;
	}
	pstGetSupportedRulesRes->stSupportedRules.sizeRuleContentSchemaLocation = 1;
	strncpy(pstGetSupportedRulesRes->stSupportedRules.pszRuleContentSchemaLocation[0], "http://www.w3.org/2001/XMLSchema", MAX_64_LENGTH-1);

	pstGetSupportedRulesRes->stSupportedRules.sizeRuleDescription = 1;
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].name, "tt:CellMotionDetector", MAX_NAME_LENGTH-1);
	
	pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stParameters.sizeSimpleItemDescription = 4;
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stParameters.stSimpleItemDescription[0].name, "MinCount", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stParameters.stSimpleItemDescription[0].type, "xsd:integer", MAX_TYPE_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stParameters.stSimpleItemDescription[1].name, "AlarmOnDelay", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stParameters.stSimpleItemDescription[1].type, "xsd:integer", MAX_TYPE_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stParameters.stSimpleItemDescription[2].name, "AlarmOffDelay", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stParameters.stSimpleItemDescription[2].type, "xsd:integer", MAX_TYPE_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stParameters.stSimpleItemDescription[3].name, "ActiveCells", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stParameters.stSimpleItemDescription[3].type, "xsd:base64Binary", MAX_TYPE_LENGTH-1);

	pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].sizeMessages = 1;
	pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stSource.sizeSimpleItemDescription = 3;
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[0].name, "VideoSourceConfigurationToken", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[0].type, "tt:ReferenceToken", MAX_TYPE_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[1].name, "VideoAnalyticsConfigurationToken", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[1].type, "tt:ReferenceToken", MAX_TYPE_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[2].name, "Rule", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[2].type, "xsd:string", MAX_TYPE_LENGTH-1);
	pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stData.sizeSimpleItemDescription = 1;
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stData.stSimpleItemDescription[0].name, "IsMotion", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].stData.stSimpleItemDescription[0].type, "xsd:boolean", MAX_TYPE_LENGTH-1);
	pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].enIsProperty = 1;
	strncpy(pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[0].stMessages[0].aszParentTopic, "tns1:RuleEngine/CellMotionDetector/Motion", MAX_64_LENGTH-1);

	return ONVIF_OK;
}
ONVIF_RET GK_NVT_Analytics_GetRules(GONVIF_ANALYTICS_GetRules_S *pstGetRulesReq, GONVIF_ANALYTICS_GetRulesRes_s *pstGetRulesRes)
{
	ONVIF_INFO("Token: %s\n", pstGetRulesReq->aszConfigurationToken);
	GK_S32 i, j, index;
	for(index = 0; index < g_GkIpc_OnvifInf.VideoAnalyticNum; index++)
	{
		if(strcmp(pstGetRulesReq->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszToken) == 0)
			break;
	}
	if(index >=g_GkIpc_OnvifInf.VideoAnalyticNum)
	{
		return ONVIF_ERR_ANALYTICS_INVALID_TOKEN;
	}

	pstGetRulesRes->sizeRule = g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.sizeRule;
	for(i = 0; i < pstGetRulesRes->sizeRule; i++)
	{
		strncpy(pstGetRulesRes->stRule[i].name, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].pszName, MAX_NAME_LENGTH-1);
		strncpy(pstGetRulesRes->stRule[i].type, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].pszType, MAX_TYPE_LENGTH-1);
		pstGetRulesRes->stRule[i].stParameters.sizeSimpleItem = g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.sizeSimpleItem;
		for(j = 0; j < pstGetRulesRes->stRule[i].stParameters.sizeSimpleItem; j++)
		{
			strncpy(pstGetRulesRes->stRule[i].stParameters.stSimpleItem[j].name, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem[j].pszName, MAX_NAME_LENGTH-1);
			strncpy(pstGetRulesRes->stRule[i].stParameters.stSimpleItem[j].value, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem[j].pszValue, MAX_256_LENGTH-1);
		}
		pstGetRulesRes->stRule[i].stParameters.sizeElementItem = g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.sizeElementItem	;
		for(j = 0; j < pstGetRulesRes->stRule[i].stParameters.sizeElementItem; j++)
		{
			strncpy(pstGetRulesRes->stRule[i].stParameters.stElementItem[j].name, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstElementItem[j].pszName, MAX_NAME_LENGTH-1);
		}
	}
	
	return ONVIF_OK;
}


ONVIF_RET GK_NVT_Analytics_GetSupportedAnalyticsModules(GONVIF_ANALYTICS_GetSupportedAnalyticsModules_S *pstGetSupportedAnalyticsModulesReq, GONVIF_ANALYTICS_GetSupportedAnalyticsModulesRes_S *pstGetSupportedAnalyticsModulesRes)
{
	ONVIF_INFO("Token: %s\n", pstGetSupportedAnalyticsModulesReq->aszConfigurationToken);
	GK_S32 index;

	for(index = 0; index < g_GkIpc_OnvifInf.VideoAnalyticNum; index++)
	{
		if(strcmp(pstGetSupportedAnalyticsModulesReq->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszToken) == 0)
			break;
	}
	if(index >=g_GkIpc_OnvifInf.VideoAnalyticNum)
	{
		return ONVIF_ERR_ANALYTICS_INVALID_TOKEN;
	}

	pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.sizeAnalyticsModuleContentSchemaLocation = 1;
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.pszAnalyticsModuleContentSchemaLocation[0] , "http://www.w3.org/2001/XMLSchema", MAX_64_LENGTH-1);

	pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.sizeAnalyticsModuleDescription = 1;
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].name, "CellMotionEngine", MAX_NAME_LENGTH-1);
	pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stParameters.sizeSimpleItemDescription = 1;
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stParameters.stSimpleItemDescription[0].name, "Sensitivity", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stParameters.stSimpleItemDescription[0].type, "xsd:integer", MAX_TYPE_LENGTH-1);
	pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stParameters.sizeElementItemDescription = 1;
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stParameters.stElementItemDescription[0].name, "Layout", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stParameters.stElementItemDescription[0].type, "tt:CellLayout", MAX_TYPE_LENGTH-1);

	pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].sizeMessages = 1;
	pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stSource.sizeSimpleItemDescription = 3;
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[0].name, "VideoSourceConfigurationToken", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[0].type, "tt:ReferenceToken", MAX_TYPE_LENGTH-1);
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[1].name, "VideoAnalyticsConfigurationToken", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[1].type, "tt:ReferenceToken", MAX_TYPE_LENGTH-1);
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[2].name, "Rule", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stSource.stSimpleItemDescription[2].type, "xsd:string", MAX_TYPE_LENGTH-1);
	pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stData.sizeSimpleItemDescription = 1;
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stData.stSimpleItemDescription[0].name, "IsMotion", MAX_NAME_LENGTH-1);
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].stData.stSimpleItemDescription[0].type, "xsd:boolean", MAX_TYPE_LENGTH-1);
	pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].enIsProperty = 1;
	strncpy(pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[0].stMessages[0].aszParentTopic, "tns1:RuleEngine/CellMotionDetector/Motion", MAX_64_LENGTH-1);

	return ONVIF_OK;

}
ONVIF_RET GK_NVT_Analytics_GetAnalyticsModules(GONVIF_ANALYTICS_GetAnalyticsModules_S *pstGetAnalyticsModulesReq, GONVIF_ANALYTICS_GetAnalyticsModulesRes_S *pstpstGetAnalyticsModulesRes)
{
	ONVIF_INFO("Token: %s\n", pstGetAnalyticsModulesReq->aszConfigurationToken);
	GK_S32 i, j, index;

	for(index = 0; index < g_GkIpc_OnvifInf.VideoAnalyticNum; index++)
	{
		if(strcmp(pstGetAnalyticsModulesReq->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszToken) == 0)
			break;
	}
	if(index >=g_GkIpc_OnvifInf.VideoAnalyticNum)
	{
		return ONVIF_ERR_ANALYTICS_INVALID_TOKEN;
	}

	pstpstGetAnalyticsModulesRes->sizeAnalyticsModule = g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.sizeAnalyticsModule;
	for(i = 0; i < pstpstGetAnalyticsModulesRes->sizeAnalyticsModule; i++)
	{
		strncpy(pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].type, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].pszType, MAX_TYPE_LENGTH-1);
		strncpy(pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].name, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].pszName, MAX_NAME_LENGTH-1);
		pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.sizeSimpleItem = g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.sizeSimpleItem;
		for(j = 0; j < pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.sizeSimpleItem; j++)
		{
			strncpy(pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.stSimpleItem[j].name, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem[j].pszName, MAX_NAME_LENGTH-1);
			strncpy(pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.stSimpleItem[j].value, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem[j].pszValue, MAX_256_LENGTH-1);
		}
		pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.sizeElementItem = g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.sizeElementItem;
		for(j = 0; j < pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.sizeElementItem; j++)
		{
			strncpy(pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.stElementItem[j].name, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstElementItem[j].pszName, MAX_NAME_LENGTH-1);
			/* !!note: where is the content of any from? */
			strncpy(pstpstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.stElementItem[j].any, "<tt:CellLayout Columns=\"22\" Row=\"15\">"
																										  "<tt:Transformation>"
																										  "<tt:Translate x=\"-1.000000\" y=\"-1.000000\"/>"
																										  "<tt:Scale x=\"0.001042\" y=\"0.001852\"/>"
																										  "</tt:Transformation>"
																										  "</tt:CellLayout>", MAX_256_LENGTH-1);
		}
	}

	return ONVIF_OK;

}


