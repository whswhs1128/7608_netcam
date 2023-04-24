/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/config_parse.c
**
**
** \brief       config file parse
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "config.h"
#include "./sys/xml.h"

XMLN *XMLTree = NULL;

static GK_BOOL presetNum[MAX_PRESET_NUM + 1];
static Node *head;
/*******************************************************************************/
static GK_S32 loadXmlFile(ONVIF_FILE_TYPE type);
static GK_S32 releaseXmlFile();
static const char *xml_write_ws_cb(XMLN *node, int where);

static GK_S32 requestConfigNum(GK_S8 *curElement);
static GK_S32 parseXmlVideoSource(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlAudioSource(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlVideoEncoder(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlAudioEncoder(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlRelayOutputs(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlMediaProfile(GK_S8 *rootElement, GK_S8 *curElement, GK_S32 Index);
static GK_S32 parseXmlVideoSourceOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlAudioSourceOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlVideoEncoderOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlAudioEncoderOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlPTZConfigurationOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlVideoAnalytics(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlPTZConfiguration(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index);
static GK_S32 parseXmlImageOption(GK_CHAR *curElement);
static GK_S32 parseXmlImageMoveOption(GK_CHAR *curElement);

static GK_S32 save_meida_VideoSourceXml(XMLN *xmlMedia);
static GK_S32 save_meida_AudioSourceXml(XMLN *xmlMedia);
static GK_S32 save_meida_VideoEncoderXml(XMLN *xmlMedia);
static GK_S32 save_meida_AudioEncoderXml(XMLN *xmlMedia);
static GK_S32 save_meida_PTZXml(XMLN *xmlMedia);
static GK_S32 save_media_RelayOutputXml(XMLN *xmlMedia);
static GK_S32 save_meida_ProfileXml(XMLN *xmlMedia);


/*******************************************************************************/
GK_S32 config_AddUser(const GONVIF_DEVMNG_User_S *user);
GK_S32 config_FindUser(const GK_CHAR *name, GONVIF_DEVMNG_User_S **User);
GK_S32 config_DeleteUser(const GK_CHAR *name);
GK_S32 config_DeleteAllUsers(void);
GK_S32 config_GetScope(const GONVIF_DEVMNG_Scopes_S *scope);

/*******************************************************************************/

GK_S32 config_Load_DeviceXml(ONVIF_FILE_TYPE type)
{
    XMLN *node = NULL;
    XMLN *temp = NULL;
    XMLN *usersNode = NULL;
    XMLN *rootNode = NULL;
    GK_S32 i = 0;
	int ret = 0;
	int len = XML_FILE_SIZE; 
	char *buffer = malloc(XML_FILE_SIZE);
	if (buffer == NULL)
	{
		ONVIF_ERR("malloc failed!\n");
		return -1;
	}
	ret = XmlFileHandle.OnvifGetFile(type,buffer,&len);
	if (ret != 0)
	{
		ONVIF_ERR("Get File failed!\n");
		return -1;
	}
	rootNode = xml_file_parse_plus(buffer,len);
	if (buffer != NULL)
	{
		free(buffer);
		buffer = NULL;
	}
	
	if(!rootNode)
	{
		ONVIF_ERR("Unable to read xml file!\n");
		return -1;
	}

    memset(&g_GkIpc_OnvifInf.devmgmt, 0, sizeof(Gkonvif_Devicemgmt));
    g_GkIpc_OnvifInf.devmgmt.User = (GONVIF_DEVMNG_User_S **)malloc(MAX_USER_NUM * sizeof(GONVIF_DEVMNG_User_S *));
    if (g_GkIpc_OnvifInf.devmgmt.User == NULL)
    {
		goto error;
    }
    memset(g_GkIpc_OnvifInf.devmgmt.User, 0, MAX_USER_NUM * sizeof(GONVIF_DEVMNG_User_S *));

	usersNode = xml_node_get_child(rootNode, "Users");
    if(usersNode == NULL)
    {
		ONVIF_ERR("Unable to find Users element in XML tree!\n");
		goto error;
	}
    for(node = xml_node_get_child(usersNode, "user"); node != NULL; node = xml_node_get_next(node, "user"))
    {
        GONVIF_DEVMNG_User_S temp_user;
        memset(&temp_user, 0, sizeof(GONVIF_DEVMNG_User_S));
        temp = xml_node_get_child(node, "name");
        if((temp != NULL) && (temp->data != NULL))
        {
            strncpy(temp_user.aszUsername, temp->data, MAX_USERNAME_LENGTH-1);
            temp = xml_node_get_child(node, "pwd");
            if(temp == NULL)
            {
            	ONVIF_ERR("pwd is NULL!\n");
                goto error;
            }
			if(temp->data)
            	strncpy(temp_user.aszPassword, temp->data, MAX_PASSWORD_LENGTH-1);
            temp = xml_node_get_child(node, "level");
            if((temp == NULL) || (temp->data == NULL))
            {
                goto error;
            }
            temp_user.enUserLevel = atoi(temp->data);
        }
        config_AddUser(&temp_user);
	}
	node = xml_node_get_child(rootNode, "DHCP");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find DHCP element in XML tree!\n");
		goto error;
	}
    g_GkIpc_OnvifInf.devmgmt.DHCP = atoi(node->data);

	node = xml_node_get_child(rootNode, "DateTimeType");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find DateTimeType element in XML tree!\n");
		goto error;
	}
    g_GkIpc_OnvifInf.devmgmt.enDateTimeType = atoi(node->data);

	node = xml_node_get_child(rootNode, "DaylightSavings");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find DaylightSavings element in XML tree!\n");
		goto error;
	}
    g_GkIpc_OnvifInf.devmgmt.bDaylightSavings = atoi(node->data);

	node = xml_node_get_child(rootNode, "time_zone");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find time_zone element in XML tree!\n");
		goto error;
	}
    g_GkIpc_OnvifInf.devmgmt.time_zone = atoi(node->data);

    node = xml_node_get_child(rootNode, "NIC_alive");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find NIC_alive element in XML tree!\n");
		goto error;
	}
    g_GkIpc_OnvifInf.devmgmt.NIC_alive = atoi(node->data);

    node = xml_node_get_child(rootNode, "NetworkProtocolsInf");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find NetworkProtocolsInf element in XML tree!\n");
		goto error;
	}
    g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.sizeNetworkProtocols = atoi(xml_attr_get_data(node, "num"));
    //GK_S8 *buf[4]={"HTTP", "HTTPS", "RTSP"};
    GK_CHAR *buf[3]={"HTTP", "RTSP"};
    for (i = 0; i < g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.sizeNetworkProtocols; i++)
    {
        temp = xml_node_get_child(node, buf[i]);
        if(temp == NULL)
        {
            ONVIF_ERR("Unable to find %s element in XML tree!\n", buf[i]);
            goto error;
        }
        if(strcmp(buf[i],"HTTP") == 0)
        {
            g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[i].enName = NetworkProtocolType_HTTP;
        }
        else if(strcmp(buf[i],"RTSP") == 0)
        {
            g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[i].enName = NetworkProtocolType_RTSP;
        }

        //if (node->value.element.num_attrs != 2)
        //{
            //ONVIF_ERR("%d\n", node->value.element.num_attrs);
            //return (-2);
        //}

        if(xml_attr_get_data(temp, "enable") != NULL)
        {
            g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[i].enEnabled = atoi(xml_attr_get_data(temp, "enable"));
        }
        if(xml_attr_get_data(temp, "Port") != NULL)
        {
            g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[i].sizePort = 1;
            g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[i].port[0] = atoi(xml_attr_get_data(temp, "Port"));
        }
    }

    node = xml_node_get_child(rootNode, "DeviceInfo");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find deviceinfo element in XML tree!\n");
		goto error;
	}
    /*Manufacturer*/
    temp = xml_node_get_child(node, "Manufacturer");
    if(temp == NULL)
    {
        ONVIF_ERR("Unable to find Manufacturer element in XML tree!\n");
		goto error;
    }
    if(temp->data != NULL)
    {
        strncpy(g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszManufacturer, temp->data, MAX_MANUFACTURER_LENGTH-1);
    }

    /*Model*/
    temp = xml_node_get_child(node, "Model");
    if(temp == NULL)
    {
        ONVIF_ERR("Unable to find Model element in XML tree!\n");
		goto error;
    }
    if(temp->data != NULL)
    {
        strncpy(g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszModel, temp->data, MAX_MODEL_LENGTH-1);
    }

    /*FirmwareVersion*/
    temp = xml_node_get_child(node, "FirmwareVersion");
    if(temp == NULL)
    {
        ONVIF_ERR("Unable to find FirmwareVersion element in XML tree!\n");
		goto error;
    }
    if(temp->data != NULL)
    {
        strncpy(g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszFirmwareVersion, temp->data, MAX_FIRMWARE_VERSION_LENGTH-1);
    }

    /*SerialNumber*/
    temp = xml_node_get_child(node, "SerialNumber");
    if(temp == NULL)
    {
        ONVIF_ERR("Unable to find SerialNumber element in XML tree!\n");
		goto error;
    }
    if(temp->data != NULL)
    {
        strncpy(g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszSerialNumber, temp->data, MAX_SERIAL_NUMBER_LENGTH-1);
    }

    /*HardwareId*/
    temp = xml_node_get_child(node, "HardwareId");
    if(temp == NULL)
    {
        ONVIF_ERR("Unable to find HardwareId element in XML tree!\n");
		goto error;
    }
    if(temp->data != NULL)
    {
        strncpy(g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszHardwareId, temp->data, MAX_HARDWARE_ID_LENGTH-1);
    }

    xml_node_del(rootNode);
    return 0;

error:
    xml_node_del(rootNode);
    return -1;

}

GK_S32 config_Load_DiscoverXml(ONVIF_FILE_TYPE type)
{
    XMLN *rootNode = NULL;
    XMLN *node = NULL;
    XMLN *scopesNode = NULL;

	int ret = 0;
	int len = XML_FILE_SIZE; 
	char *buffer = malloc(XML_FILE_SIZE);
	if (buffer == NULL)
	{
		ONVIF_ERR("malloc failed!\n");
		return -1;
	}
	ret  = XmlFileHandle.OnvifGetFile(type,buffer,&len);
	if (ret != 0)
	{
		ONVIF_ERR("Get File error!\n");
		return -1;	
	}
	rootNode = xml_file_parse_plus(buffer,len);
	if (buffer != NULL)
	{
		free(buffer);
		buffer = NULL;
	}
	
	if (!rootNode)
	{
		ONVIF_ERR("Unable to read file!\n");
		return -1;
	}

    memset(&g_GkIpc_OnvifInf.discov, 0, sizeof(GKonvif_Discovery));

    g_GkIpc_OnvifInf.discov.Scopes = (GONVIF_DEVMNG_Scopes_S **)malloc(MAX_SCOPE_NUM * sizeof(GONVIF_DEVMNG_Scopes_S *));
    if (g_GkIpc_OnvifInf.discov.Scopes == NULL)
    {
       goto error;
    }
    memset(g_GkIpc_OnvifInf.discov.Scopes, 0, MAX_SCOPE_NUM * sizeof(GONVIF_DEVMNG_Scopes_S *));

	scopesNode = xml_node_get_child(rootNode, "Scopes");
    if(scopesNode == NULL)
    {
		ONVIF_ERR("Unable to find Scopes element in XML tree!\n");
        goto error;
	}
    for (node = xml_node_get_child(scopesNode, "scope"); node != NULL; node = xml_node_get_next(node, "scope"))
    {
        GONVIF_DEVMNG_Scopes_S scope;
        if(node->data != NULL)
        {
            if(strncmp(xml_attr_get_data(node, "type"), "Fixed", 5) == 0)
                scope.enScopeDef = ScopeDefinition_Fixed;
            else if(strncmp(xml_attr_get_data(node, "type"), "Configurable", 12) == 0)
                scope.enScopeDef = ScopeDefinition_Configurable;

            strncpy(scope.aszScopeItem, node->data, LEN_SCOPE_ITEM-1);
        }
        config_GetScope(&scope);
	}

	node = xml_node_get_child(rootNode, "DiscoveryMode");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find DiscoveryMode element in XML tree!\n");
        goto error;
	}
    g_GkIpc_OnvifInf.discov.discoverymode = atoi(node->data);

	node = xml_node_get_child(rootNode, "Security");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find Security element in XML tree!\n");
        goto error;
	}
    g_GkIpc_OnvifInf.discov.Is_security = atoi(node->data);

	node = xml_node_get_child(rootNode, "Hostport");
    if(node == NULL)
    {
		ONVIF_ERR("Unable to find Hostport element in XML tree!\n");
        goto error;
	}
    if(node->data != NULL)
    {
        g_GkIpc_OnvifInf.discov.hostport = (unsigned int)atoi(node->data);
    }
    else
    {
        g_GkIpc_OnvifInf.discov.hostport = ONVIF_HTTP_LISTEN_PORT;
    }

    xml_node_del(rootNode);
    return 0;

error:
    xml_node_del(rootNode);
    return -1;

}

GK_S32 config_Load_MediadynamicXml(ONVIF_FILE_TYPE type)
{
	GK_S32 cfgNum;
	GK_S8 cfgBuf[32];

	GK_S32 retVal = loadXmlFile(type);
	if(retVal != 0)
	{
		ONVIF_ERR("Load file failed!\n");
		return -1;
	}

	/*parse the video source configuration*/
	GK_S32 VideoSourceNum = requestConfigNum(VIDEOSOURCE_TAG);
	if(VideoSourceNum <= 0)
	{
		ONVIF_ERR("request video source num failed!\n");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.VideoSourceNum = VideoSourceNum;
	g_GkIpc_OnvifInf.gokeIPCameraVideoSource = malloc(MAX_VIDEOSOURCE_NUM * sizeof(GONVIF_MEDIA_VideoSourceConfiguration_S));
	if(g_GkIpc_OnvifInf.gokeIPCameraVideoSource == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeIPCameraVideoSource, 0, MAX_VIDEOSOURCE_NUM * sizeof(GONVIF_MEDIA_VideoSourceConfiguration_S));

	for(cfgNum = 0; cfgNum < g_GkIpc_OnvifInf.VideoSourceNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlVideoSource(VIDEOSOURCE_TAG, cfgBuf, cfgNum) != 0)
		{
            goto error;
		}
	}

	/*parse the audio source configuration*/
	GK_S32 AudioSourceNum = requestConfigNum(AUDIOSOURCE_TAG);
	if(AudioSourceNum <= 0)
	{
		ONVIF_ERR("request audio source num failed!\n");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.AudioSourceNum = AudioSourceNum;
	g_GkIpc_OnvifInf.gokeIPCameraAudioSource = malloc(MAX_AUDIOSOURCE_NUM * sizeof(GONVIF_MEDIA_AudioSourceConfiguration_S));
	if(g_GkIpc_OnvifInf.gokeIPCameraAudioSource == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeIPCameraAudioSource, 0, MAX_AUDIOSOURCE_NUM * sizeof(GONVIF_MEDIA_AudioSourceConfiguration_S));
	for(cfgNum =0; cfgNum < g_GkIpc_OnvifInf.AudioSourceNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlAudioSource(AUDIOSOURCE_TAG, cfgBuf, cfgNum) != 0)
		{
            goto error;
		}
	}
	/*parse the video encoder configuration*/
	GK_S32 VideoEncoderNum = requestConfigNum(VIDEOENCODER_TAG);
	if(VideoEncoderNum <= 0)
	{
		ONVIF_ERR("request video encoder num failed!\n");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.VideoEncodeNum = VideoEncoderNum;
	g_GkIpc_OnvifInf.gokeIPCameraVideoEncode = malloc(MAX_VIDEOENCODE_NUM * sizeof(GONVIF_MEDIA_VideoEncoderConfiguration_S));
	if(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode, 0, MAX_VIDEOENCODE_NUM * sizeof(GONVIF_MEDIA_VideoEncoderConfiguration_S));
	for(cfgNum =0; cfgNum < g_GkIpc_OnvifInf.VideoEncodeNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf,sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlVideoEncoder(VIDEOENCODER_TAG, cfgBuf, cfgNum) != 0)
		{
            goto error;
		}
	}

	/*parse the audio encoder configuration*/
	GK_S32 AudioEncoderNum = requestConfigNum(AUDIOENCODER_TAG);
	if(AudioEncoderNum <= 0)
	{
		ONVIF_ERR("request audio encoder num failed!\n");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.AudioEncodeNum = AudioEncoderNum;
	g_GkIpc_OnvifInf.gokeIPCameraAudioEncode = malloc(MAX_AUDIOENCODE_NUM * sizeof(GONVIF_MEDIA_AudioEncoderConfiguration_S));
	if(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode, 0, MAX_AUDIOENCODE_NUM * sizeof(GONVIF_MEDIA_AudioEncoderConfiguration_S));
	for(cfgNum =0; cfgNum < g_GkIpc_OnvifInf.AudioEncodeNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlAudioEncoder(AUDIOENCODER_TAG, cfgBuf, cfgNum) != 0)
		{
            goto error;
		}
	}

	/*parse the PTZ configuration*/
	GK_S32 PTZConfigNum = requestConfigNum(PTZ_TAG);
	if(PTZConfigNum <= 0)
	{
		ONVIF_ERR("request PTZ configuration num failed!\n");
        goto error;
	}

	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.PTZConfigurationNum = PTZConfigNum;
	g_GkIpc_OnvifInf.gokePTZConfiguration = malloc(MAX_PTZ_CONFIGURATION_NUM * sizeof(GONVIF_PTZ_Configuration_S));
	if(g_GkIpc_OnvifInf.gokePTZConfiguration == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokePTZConfiguration, 0, MAX_PTZ_CONFIGURATION_NUM * sizeof(GONVIF_PTZ_Configuration_S));
	for(cfgNum = 0; cfgNum < g_GkIpc_OnvifInf.PTZConfigurationNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlPTZConfiguration(PTZ_TAG, cfgBuf, cfgNum) != 0)
		{
            goto error;
		}
	}

	/*parse the relay outputs configuration*/
	GK_S32 relayOutputsNum = requestConfigNum(RELAYOUTPUT_TAG);
	if(relayOutputsNum <= 0)
	{
		ONVIF_ERR("request relayOutput num failed!\n");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.relayOutputsNum = relayOutputsNum;
	g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs = malloc(MAX_RELAYOUTPUT_NUM * sizeof(GONVIF_DEVICEIO_RelayOutput_S));
	if(g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs, 0, MAX_RELAYOUTPUT_NUM * sizeof(GONVIF_DEVICEIO_RelayOutput_S));
	for(cfgNum =0; cfgNum < g_GkIpc_OnvifInf.relayOutputsNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlRelayOutputs(RELAYOUTPUT_TAG, cfgBuf, cfgNum) != 0)
		{
		    ONVIF_ERR("request relayOutput Configuration failed!\n");
            goto error;
		}
	}

	/*parse the media Profile configuration*/
	GK_S32 ProfileNum = requestConfigNum(MEDIAPROFILE_TAG);
	if(AudioEncoderNum <= 0)
	{
		ONVIF_ERR("request profile num failed!\n");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.ProfileNum = ProfileNum;
	g_GkIpc_OnvifInf.gokeIPCameraProfiles = malloc(MAX_PROFILE_NUM * sizeof(GONVIF_MEDIA_Profile_S));
	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeIPCameraProfiles, 0, MAX_PROFILE_NUM * sizeof(GONVIF_MEDIA_Profile_S));
	for(cfgNum =0; cfgNum < g_GkIpc_OnvifInf.ProfileNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlMediaProfile(MEDIAPROFILE_TAG, cfgBuf, cfgNum) != 0)
		{
            goto error;
		}
	}

	releaseXmlFile();
	return 0;

error:
    releaseXmlFile();
    return -1;

}

GK_S32 config_Load_MediastaticXml(ONVIF_FILE_TYPE type)
{
	GK_S32 cfgNum;
	GK_S8 cfgBuf[32];

	GK_S32 retVal = loadXmlFile(type);
	if(retVal != 0)
	{
		ONVIF_ERR("Load Media Xml File Failed!\n");
		return -1;
	}

	/*parse the video source configuration option*/
	GK_S32 VSOptionNum = requestConfigNum(VIDEOSOURCE_OPTION_TAG);
	if(VSOptionNum <= 0)
	{
		ONVIF_ERR("Fail to request num of video source options\n");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.VideoSourceOptionNum = VSOptionNum;
	g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption = malloc(MAX_VIDEOSOURCEOPTION_NUM * sizeof(GONVIF_MEDIA_VideoSourceConfigurationOptions_S));
	if(g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption, 0, MAX_VIDEOSOURCEOPTION_NUM * sizeof(GONVIF_MEDIA_VideoSourceConfigurationOptions_S));
	for(cfgNum = 0; cfgNum < g_GkIpc_OnvifInf.VideoSourceOptionNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlVideoSourceOption(VIDEOSOURCE_OPTION_TAG, cfgBuf, cfgNum) != 0)
		{
			ONVIF_ERR("Fail to parse options of video source configurations.");
            goto error;
		}
	}

	/*parse the audio source configuration option*/
	GK_S32 ASOptionNum = requestConfigNum(AUDIOSOURCE_OPTION_TAG);
	if(ASOptionNum <= 0)
	{
		ONVIF_ERR("Fail to request num of audio source options.");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.AudioSourceOptionNum = ASOptionNum;
	g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption = malloc(MAX_AUDIOSOURCEOPTION_NUM * sizeof(GONVIF_MEDIA_AudioSourceConfigurationOptions_S));
	if(g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption, 0, MAX_AUDIOSOURCEOPTION_NUM * sizeof(GONVIF_MEDIA_AudioSourceConfigurationOptions_S));
	for(cfgNum =0; cfgNum < g_GkIpc_OnvifInf.AudioSourceOptionNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlAudioSourceOption(AUDIOSOURCE_OPTION_TAG, cfgBuf, cfgNum) != 0)
		{
			ONVIF_ERR("Fail to parse options of audio source configurations.");
            goto error;
		}
	}

	/*parse the video encoder configuration option*/
	GK_S32 VEOptionNum = requestConfigNum(VIDEOENCODER_OPTION_TAG);
	if(VEOptionNum <= 0)
	{
		ONVIF_ERR("Fail to request num of video encoder options.");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.VideoEncodeOptionNum = VEOptionNum;
	g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption = malloc(MAX_VIDEOENCODEOPTION_NUM * sizeof(GONVIF_MEDIA_VideoEncoderConfigurationOptions_S));
	if(g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption, 0, MAX_VIDEOENCODEOPTION_NUM * sizeof(GONVIF_MEDIA_VideoEncoderConfigurationOptions_S));
	// options of video encoder configuration maybe come from exterior of private XML.
	for(cfgNum =0; cfgNum < g_GkIpc_OnvifInf.VideoEncodeOptionNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlVideoEncoderOption(VIDEOENCODER_OPTION_TAG, cfgBuf, cfgNum) != 0)
		{
			ONVIF_ERR("Fail to parse options of video encoder configurations.");
            goto error;
		}
	}

	/*parse the audio encoder configuration option*/
	GK_S32 AEOptionNum = requestConfigNum(AUDIOENCODER_OPTION_TAG);
	if(AEOptionNum <= 0)
	{
		ONVIF_ERR("Fail to request num of audio encoder configurations.\n");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.AudioEncodeOptionNum = AEOptionNum;
	g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption = malloc(MAX_AUDIOENCODEOPTION_NUM * sizeof(GONVIF_MEDIA_AudioEncoderConfigurationOptions_S));
	if(g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption, 0, MAX_AUDIOENCODEOPTION_NUM * sizeof(GONVIF_MEDIA_AudioEncoderConfigurationOptions_S));
	//options of audio encoder configuration maybe come from exterior of private XML.
	for(cfgNum =0; cfgNum < g_GkIpc_OnvifInf.AudioEncodeOptionNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf,sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlAudioEncoderOption(AUDIOENCODER_OPTION_TAG,cfgBuf,cfgNum) != 0)
		{
			ONVIF_ERR("Fail to parse options of audio encoder configurations.");
            goto error;
		}
	}

	/*parse the PTZ configuration option*/
    if(parseXmlPTZConfigurationOption(PTZ_CONFIGURATION_OPTIONS_TAG, NULL, 0) != 0)
    {
		ONVIF_ERR("Fail to parse options of PTZ configurations.");
        goto error;
    }
	
	/*parse the video analytic configuration*/
	GK_S32 VideoAnalyticNum = requestConfigNum(VIDEOANALYTIC_TAG);
	if(VideoAnalyticNum <= 0)
	{
		ONVIF_ERR("Fail to request num of analytics configurations.\n");
        goto error;
	}
	memset(cfgBuf, 0, sizeof(cfgBuf));
	g_GkIpc_OnvifInf.VideoAnalyticNum = VideoAnalyticNum;
	g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic = malloc(MAX_VIDEOANALYTIC_NUM * sizeof(GONVIF_MEDIA_VideoAnalyticsConfiguration_S));
	if(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic == NULL)
	{
		goto error;
	}
	memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic, 0, MAX_VIDEOANALYTIC_NUM * sizeof(GONVIF_MEDIA_VideoAnalyticsConfiguration_S));
	for(cfgNum =0; cfgNum < g_GkIpc_OnvifInf.VideoAnalyticNum; cfgNum++)
	{
	    memset(cfgBuf, 0, sizeof(cfgBuf));
		snprintf(cfgBuf, sizeof(cfgBuf), "Configuration%d", cfgNum);
		if(parseXmlVideoAnalytics(VIDEOANALYTIC_TAG, cfgBuf, cfgNum) != 0)
		{
			ONVIF_ERR("Fail to parse options of analytics configurations.\n");
            goto error;
		}
	}

	releaseXmlFile();
	return 0;

error:
	releaseXmlFile();
	return -1;

}
GK_S32 config_Load_ImagingXml(ONVIF_FILE_TYPE type)
{
	GK_S32 retVal = loadXmlFile(type);
	if(retVal != 0)
	{
		ONVIF_ERR("Load Image Xml File Failed!\n");
		return -1;
	}

	if(parseXmlImageOption(IMAGEOPTION_TAG) != 0)
	{
		releaseXmlFile();
		return -1;
	}
	if(parseXmlImageMoveOption(IMAGEMOVEOPTION_TAG) != 0)
	{
		releaseXmlFile();
		return -1;
	}

	releaseXmlFile();

	return 0;
}


GK_S32 config_Load_PtzXml(ONVIF_FILE_TYPE type)
{
/*
 * TODO: Here these parameters of PTZ node should be got from XML. 
*/
    g_GkIpc_OnvifInf.sizePTZnode = 1;
    g_GkIpc_OnvifInf.pstPTZnode = (GONVIF_PTZ_Node_S *)malloc(g_GkIpc_OnvifInf.sizePTZnode * sizeof(GONVIF_PTZ_Node_S));
	if(g_GkIpc_OnvifInf.pstPTZnode == NULL)
	{
		return -1;
	}
//token
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].token, "PTZ-GPIO-Token", MAX_TOKEN_LENGTH-1);
//name
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].name, "PTZ-GPIO", MAX_NAME_LENGTH-1);
//absolute
    //PanTilt
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.sizeAbsolutePanTiltPositionSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[0].aszURI,
        "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[0].stXRange.max = 1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[0].stYRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[0].stYRange.max = 1;
    //Zoom
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.sizeAbsoluteZoomPositionSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stAbsoluteZoomPositionSpace[0].aszURI,
        "http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stAbsoluteZoomPositionSpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stAbsoluteZoomPositionSpace[0].stXRange.max = 1;
//relative 
    //PanTilt
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.sizeRelativePanTiltTranslationSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[0].aszURI,
        "http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[0].stXRange.max = 1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[0].stYRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[0].stYRange.max = 1;
    //Zoom
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.sizeRelativeZoomTranslationSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stRelativeZoomTranslationSpace[0].aszURI,
        "http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stRelativeZoomTranslationSpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stRelativeZoomTranslationSpace[0].stXRange.max = 1;
//continuous
    //PanTilt
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.sizeContinuousPanTiltVelocitySpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[0].aszURI,
        "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[0].stXRange.max = 1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[0].stYRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[0].stYRange.max = 1;
    //Zoom
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.sizeContinuousZoomVelocitySpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stContinuousZoomVelocitySpace[0].aszURI,
        "http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stContinuousZoomVelocitySpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stContinuousZoomVelocitySpace[0].stXRange.max = 1;
//Speed
    //PanTilt
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.sizePanTiltSpeedSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stPanTiltSpeedSpace[0].aszURI,
        "http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stPanTiltSpeedSpace[0].stXRange.min = 0;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stPanTiltSpeedSpace[0].stXRange.max = 1;
    //Zoom
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.sizeZoomSpeedSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stZoomSpeedSpace[0].aszURI,
        "http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stZoomSpeedSpace[0].stXRange.min = 0;
    g_GkIpc_OnvifInf.pstPTZnode[0].stSupportedPTZSpaces.stZoomSpeedSpace[0].stXRange.max = 1;
//HomePosition
    g_GkIpc_OnvifInf.pstPTZnode[0].enHomeSupported = Boolean_FALSE;
    g_GkIpc_OnvifInf.pstPTZnode[0].enFixedHomePosition = Boolean_FALSE;
//Preset    
    g_GkIpc_OnvifInf.pstPTZnode[0].maximumNumberOfPresets = 32;
    g_GkIpc_OnvifInf.pstPTZnode[0].stExtension.stSupportedPresetTour.sizePTZPresetTourOperation = 4;
    g_GkIpc_OnvifInf.pstPTZnode[0].stExtension.stSupportedPresetTour.enPTZPresetTourOperation[0] = PTZPresetTourOperation_Start;
    g_GkIpc_OnvifInf.pstPTZnode[0].stExtension.stSupportedPresetTour.enPTZPresetTourOperation[1] = PTZPresetTourOperation_Stop;
    g_GkIpc_OnvifInf.pstPTZnode[0].stExtension.stSupportedPresetTour.enPTZPresetTourOperation[2] = PTZPresetTourOperation_Pause;

    return 0;
}

GK_S32 config_Load_PtzPreset(GK_CHAR *filePath)
{
    FILE *fp = NULL;
    Node *p = NULL;
    Node *pt = NULL;

    head = (Node *)malloc(sizeof(Node));
    memset(head, 0, sizeof(Node));
    fp = fopen(filePath, "r");
    if(fp == NULL)
    {
        ONVIF_ERR("No ptz preset\n");
        return 0;
    }

    memset(presetNum, GK_FALSE, sizeof(presetNum));
    p = head;
    ONVIF_DBG("****************load ptz presets**************\n");
    while(!feof(fp))
    {
        pt = (Node *)malloc(sizeof(Node));
        if(fread(&pt->Preset, sizeof(Preset), 1, fp) == 0)
        {
            free(pt);
            break;
        }
        p->Next = pt;
        p = pt;
        p->Next = NULL;
        ONVIF_DBG("%d\t%s\t%s\n", p->Preset.Num ,p->Preset.PresetToken, p->Preset.PresetName);
        presetNum[p->Preset.Num] = GK_TRUE;
    }
    ONVIF_DBG("******************** end **********************\n");
    fclose(fp);
    return 0;
}

GK_S32 config_Save_DeviceXml(ONVIF_FILE_TYPE type)
{
    char xmlBuf[10];
    XMLN *node;
    XMLN *top;
    XMLN *node_user;
    XMLN *node_users;
    XMLN *node_net;
    XMLN *node_deviceinfo;
	FILE *fp ;
	char filename[64] = {0}; 
    GK_S32 i=0;
	int ret = 0;
    XMLN *xmlTitle = NULL;
    xmlTitle = xml_node_add_new(XML_NO_PARENT, "?xml version=\"1.0\" encoding=\"UTF-8\"?");
    top = xml_node_add_new(XML_NO_PARENT, "DeviceMgmt");

    node_users = xml_node_add_new(top, "Users");

    for(i = 0; i < g_GkIpc_OnvifInf.devmgmt.Usercount; i++)
    {
        node_user = xml_node_add_new(node_users, "user");

        node = xml_node_add_new(node_user, "name");
        xml_node_set_data(node, g_GkIpc_OnvifInf.devmgmt.User[i]->aszUsername);
        node = xml_node_add_new(node_user, "pwd");
        xml_node_set_data(node, g_GkIpc_OnvifInf.devmgmt.User[i]->aszPassword);
        node = xml_node_add_new(node_user, "level");
        snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.devmgmt.User[i]->enUserLevel);
        xml_node_set_data(node, xmlBuf);
	}

    memset(xmlBuf, 0, sizeof(xmlBuf));
    node = xml_node_add_new(top, "DHCP");
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.devmgmt.DHCP);
    xml_node_set_data(node, xmlBuf);

    memset(xmlBuf, 0, sizeof(xmlBuf));
    node = xml_node_add_new(top, "DateTimeType");
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.devmgmt.enDateTimeType);
    xml_node_set_data(node, xmlBuf);

    memset(xmlBuf, 0, sizeof(xmlBuf));
    node = xml_node_add_new(top, "DaylightSavings");
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.devmgmt.bDaylightSavings);
    xml_node_set_data(node, xmlBuf);

    memset(xmlBuf, 0, sizeof(xmlBuf));
    node = xml_node_add_new(top, "time_zone");
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.devmgmt.time_zone);
    xml_node_set_data(node, xmlBuf);

    memset(xmlBuf, 0, sizeof(xmlBuf));
    node = xml_node_add_new(top, "NIC_alive");
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.devmgmt.NIC_alive);
    xml_node_set_data(node, xmlBuf);

    node_net = xml_node_add_new(top, "NetworkProtocolsInf");
    xml_attr_add(node_net, "num", "2");

    memset(xmlBuf, 0, sizeof(xmlBuf));
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[0].port[0]);
    node = xml_node_add_new(node_net, "HTTP");
    xml_attr_add(node, "Port", xmlBuf);
    xml_attr_add(node, "enable", g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[0].enEnabled ? "1" : "0");
    xml_node_set_data(node, "");

    memset(xmlBuf, 0, sizeof(xmlBuf));
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[1].port[0]);
    node = xml_node_add_new(node_net, "RTSP");
    xml_attr_add(node, "Port", xmlBuf);
    xml_attr_add(node, "enable", g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[1].enEnabled ? "1" : "0");
    xml_node_set_data(node, "");

    node_deviceinfo = xml_node_add_new(top, "DeviceInfo");
    node = xml_node_add_new(node_deviceinfo, "Manufacturer");
    xml_node_set_data(node, g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszManufacturer);
    node = xml_node_add_new(node_deviceinfo, "Model");
    xml_node_set_data(node, g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszModel);
    node = xml_node_add_new(node_deviceinfo, "FirmwareVersion");
    xml_node_set_data(node, g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszFirmwareVersion);
    node = xml_node_add_new(node_deviceinfo, "SerialNumber");
    xml_node_set_data(node, g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszSerialNumber);
    node = xml_node_add_new(node_deviceinfo, "HardwareId");
    xml_node_set_data(node, g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszHardwareId);

   	ret = XmlFileHandle.OnvifSaveFile(type,filename);
    if(ret != 0){
        ONVIF_ERR("open device xml config file failed!\n");
        return -1;
	}
	fp= fopen(filename, "wb");
	if(fp == NULL)
	{
		ONVIF_ERR("open %s config file failed!\n", filename);
		return -1;
	}
    xml_file_save(xmlTitle, fp, xml_write_ws_cb);
    xml_file_save(top, fp, xml_write_ws_cb);
    fclose(fp);

    xml_node_del(top);
    xml_node_del(xmlTitle);

    return (0);
}
GK_S32 config_Save_DiscoverXml(ONVIF_FILE_TYPE type)
{
    XMLN *node;
    XMLN *top;
    XMLN *node_Scopes;
	FILE *fp ;
    GK_S32 i = 0;
	int ret =0;
	char filename[64] = {0};
    char xmlBuf[50];
    XMLN *xmlTitle;
    xmlTitle = xml_node_add_new(XML_NO_PARENT, "?xml version=\"1.0\" encoding=\"UTF-8\"?");

    top = xml_node_add_new(XML_NO_PARENT, "Discover");

    node_Scopes = xml_node_add_new(top, "Scopes");

	for (i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
	{
        node = xml_node_add_new(node_Scopes, "scope");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.discov.Scopes[i]->enScopeDef ? "Configurable":"Fixed");
        xml_attr_add(node, "type", xmlBuf);
        xml_node_set_data(node, g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem);
	}
 
    node = xml_node_add_new(top, "Address");
    xml_node_set_data(node, "onvif/device_service");
    node = xml_node_add_new(top, "DiscoveryMode");
    memset(xmlBuf, 0, sizeof(xmlBuf));
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.discov.discoverymode);
    xml_node_set_data(node, xmlBuf);

    node = xml_node_add_new(top, "Security");
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.discov.Is_security);
    xml_node_set_data(node, xmlBuf);

    node = xml_node_add_new(top, "Hostport");
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.discov.hostport);
    xml_node_set_data(node, xmlBuf);

    ret = XmlFileHandle.OnvifSaveFile(type,filename);
	if(ret != 0)
	{
		ONVIF_ERR("open discover xml config file failed!\n");
		return -1;
	}
	fp= fopen(filename, "wb");
	if(fp == NULL)
	{
		ONVIF_ERR("open %s.xml config file failed!\n", filename);
		return -1;
	}
    xml_file_save(xmlTitle, fp, xml_write_ws_cb);
    xml_file_save(top, fp, xml_write_ws_cb);
    fclose(fp);

    xml_node_del(top);
    xml_node_del(xmlTitle);

    return (0);
}

GK_S32 config_Save_MediaXml(ONVIF_FILE_TYPE type)
{
    XMLN *xmlTitle;
    XMLN *meidaNode;
	int ret = 0;
	char file_name[64] = {0};
	FILE *fp = NULL;
    xmlTitle = xml_node_add_new(XML_NO_PARENT, "?xml version=\"1.0\" encoding=\"UTF-8\"?");
    meidaNode = xml_node_add_new(XML_NO_PARENT, "Media");

	save_meida_VideoSourceXml(meidaNode);

	save_meida_AudioSourceXml(meidaNode);

	save_meida_VideoEncoderXml(meidaNode);

    save_meida_AudioEncoderXml(meidaNode);

    save_meida_PTZXml(meidaNode);

    save_media_RelayOutputXml(meidaNode);

    save_meida_ProfileXml(meidaNode);

	ret = XmlFileHandle.OnvifSaveFile(type,file_name);
	if(ret != 0)
	{
		ONVIF_ERR("open media xml config file failed!\n");
		return -1;
	}
	fp = fopen(file_name,"wb");
	if(fp == NULL)
	{
		ONVIF_ERR("open %s.xml config file failed!\n", file_name);
		return -1;
	}
    xml_file_save(xmlTitle, fp, xml_write_ws_cb);
    xml_file_save(meidaNode, fp, xml_write_ws_cb);

    fclose(fp);

    xml_node_del(meidaNode);
    xml_node_del(xmlTitle);

    return (0);
}

const GK_CHAR *config_GetPassword(const GK_CHAR *username)
{
    GK_U8 i = 0;
    for(i = 0; i < g_GkIpc_OnvifInf.devmgmt.Usercount; i++)
    {
        if (0 == strcmp(g_GkIpc_OnvifInf.devmgmt.User[i]->aszUsername, username))
        {
            return (const GK_CHAR *)g_GkIpc_OnvifInf.devmgmt.User[i]->aszPassword; ;
        }
    }
    return NULL;
}

GK_S32 config_AddUser(const GONVIF_DEVMNG_User_S *user)
{
    const GONVIF_DEVMNG_User_S *tempUser = user;
    if(g_GkIpc_OnvifInf.devmgmt.Usercount >= MAX_USER_NUM)
    {
    	ONVIF_ERR("Numbers of users have overflow.");
    	return -1;
	}
    g_GkIpc_OnvifInf.devmgmt.User[g_GkIpc_OnvifInf.devmgmt.Usercount] = (GONVIF_DEVMNG_User_S *)malloc(sizeof(GONVIF_DEVMNG_User_S));
    if(g_GkIpc_OnvifInf.devmgmt.User[g_GkIpc_OnvifInf.devmgmt.Usercount] == NULL)
    {
    	ONVIF_ERR("Fail to alloc memory.");
    	return -1;
    }
    strncpy(g_GkIpc_OnvifInf.devmgmt.User[g_GkIpc_OnvifInf.devmgmt.Usercount]->aszUsername, tempUser->aszUsername, MAX_USERNAME_LENGTH-1);
    strncpy(g_GkIpc_OnvifInf.devmgmt.User[g_GkIpc_OnvifInf.devmgmt.Usercount]->aszPassword, tempUser->aszPassword, MAX_USERNAME_LENGTH-1);
    g_GkIpc_OnvifInf.devmgmt.User[g_GkIpc_OnvifInf.devmgmt.Usercount]->enUserLevel = tempUser->enUserLevel;
    g_GkIpc_OnvifInf.devmgmt.Usercount++;

    return 0;
}

GK_S32 config_FindUser(const GK_CHAR *name, GONVIF_DEVMNG_User_S **User)
{
    const GK_CHAR *username = name;
    GK_U8 count = g_GkIpc_OnvifInf.devmgmt.Usercount;
    GK_U8 index=0;

    if (NULL == name || (NULL == User))
    {
        return (-2);
    }
    while(index<count)
    {
       if (0 == strcmp(User[index]->aszUsername, username))
       {
           return index;
       }
       index++;
    }
    if (index == count)
        return (-1);
        
    return (-3);
}

GK_S32 config_DeleteUser(const GK_CHAR *name)
{
	GK_S32 i = 0;
	GK_S32 index = 0;
	index = config_FindUser(name, g_GkIpc_OnvifInf.devmgmt.User);
	if(index >= 0)
	{
		if(g_GkIpc_OnvifInf.devmgmt.User[index])
		{
			free(g_GkIpc_OnvifInf.devmgmt.User[index]);
			g_GkIpc_OnvifInf.devmgmt.User[index] = NULL;
		}
		for(i = index;i < g_GkIpc_OnvifInf.devmgmt.Usercount; i++)
		{
			g_GkIpc_OnvifInf.devmgmt.User[i] = g_GkIpc_OnvifInf.devmgmt.User[i+1];
		}
		g_GkIpc_OnvifInf.devmgmt.Usercount--;
	}
	else
	{
		ONVIF_ERR("No such user.\n");
		return -1;
	}

	return 0;
}

GK_S32 config_DeleteAllUsers()
{
	GK_U8 i = 0;
	for(i = 0; i < g_GkIpc_OnvifInf.devmgmt.Usercount; i++)
	{
		if(g_GkIpc_OnvifInf.devmgmt.User[i])
		{
			if(g_GkIpc_OnvifInf.devmgmt.User[i])
			{
				free(g_GkIpc_OnvifInf.devmgmt.User[i]);
				g_GkIpc_OnvifInf.devmgmt.User[i] = NULL;
			}
		}
	}
	g_GkIpc_OnvifInf.devmgmt.Usercount = 0;
	
	return 0;
}
GK_S32 config_GetScope(const GONVIF_DEVMNG_Scopes_S *scope)
{
    const GONVIF_DEVMNG_Scopes_S *Scope = scope;
    if(Scope->aszScopeItem[0] != '\0')
    {
        GK_U8 count = g_GkIpc_OnvifInf.discov.Scopecount;
        g_GkIpc_OnvifInf.discov.Scopes[count] = (GONVIF_DEVMNG_Scopes_S *)malloc(sizeof(GONVIF_DEVMNG_Scopes_S));
        strcpy(g_GkIpc_OnvifInf.discov.Scopes[count]->aszScopeItem, Scope->aszScopeItem);
        g_GkIpc_OnvifInf.discov.Scopes[count]->enScopeDef = Scope->enScopeDef;
        g_GkIpc_OnvifInf.discov.Scopecount++;
    }

    return 0;
}

static GK_S32 loadXmlFile(ONVIF_FILE_TYPE type)
{
	if(XMLTree != NULL)
    {
        xml_node_del(XMLTree);
        XMLTree = NULL;
    }
	int ret = 0;
	int len = XML_FILE_SIZE; 
	char *buffer = malloc(XML_FILE_SIZE);
	if (buffer == NULL)
	{
		ONVIF_ERR("malloc failed!\n");
		return -1;
	}
	ret = XmlFileHandle.OnvifGetFile(type,buffer,&len);
	if (ret != 0)
	{
		ONVIF_ERR("Get File failed\n");
		return -1;
	}
	XMLTree = xml_file_parse_plus(buffer,len);
	if (!XMLTree)
	{
		ONVIF_ERR("Unable to read XML file!\n");
		return -1;
	}
	if (buffer != NULL)
	{
		free(buffer);
		buffer = NULL;
	}
	return 0;
}

static GK_S32 releaseXmlFile()
{

	if(!XMLTree)
	{
		ONVIF_ERR("xml file not load!\n");
		return -1;
	}

	xml_node_del(XMLTree);

    XMLTree = NULL;

	return 0;
}

static const char *xml_write_ws_cb(XMLN *node, int where)
{
    XMLN *parent = NULL;
    const char *name = NULL;
    int level;
    static const char *tabs = "\t\t\t\t\t\t\t\t";

    name = node->name;

    if (!strncmp(name, "?xml", 4))
    {
        if (where == XML_WS_AFTER_OPEN)
            return ("\n");
        else
            return (NULL);
    }
    else if (where == XML_WS_BEFORE_OPEN)
    {
        for (level = 0, parent = node->parent;parent;level++, parent = parent->parent);
        if ( level > 8)
            level = 8;
        return (tabs + 8 - level);
    }
	else if(where == XML_WS_BEFORE_CLOSE && (node->f_child != NULL && node->f_child->type == NTYPE_TAG))
	{
        for (level = 0, parent = node->parent;parent;level++, parent = parent->parent);
        if ( level > 8)
            level = 8;
        return (tabs + 8 - level);
	}
    else if (where == XML_WS_AFTER_CLOSE)
    {
        return ("\n");
    }
    else if (where == XML_WS_AFTER_OPEN && (node->f_child != NULL && node->f_child->type == NTYPE_TAG))
    {
        return ("\n");
    }

    return (NULL);
}

static GK_S32 requestConfigNum(GK_S8 *curElement)
{
	GK_S32 configNum = 0;
	XMLN *configNode = NULL;

	configNode = xml_node_get_child(XMLTree, curElement);
	if(!configNode)
	{
		ONVIF_ERR("Unable to find media element in XML tree!\n");
		return -1;
	}

	const GK_CHAR *pszNum = xml_attr_get_data(configNode, "Num");
	if(pszNum)
	{
		sscanf(pszNum, "%d", &configNum);
	}
	else
	{
		ONVIF_ERR("request config num failed or xml file content error!\n");
		return -1;
	}

	return configNum;
}

static GK_S32 parseXmlVideoSource(GK_CHAR *rootElement, GK_S8 *curElement, GK_S32 index)
{
	XMLN *videoSourceNode = NULL;
	XMLN *configNode = NULL;
	XMLN *valueNode = NULL;

	videoSourceNode = xml_node_get_child(XMLTree, rootElement);
	if(videoSourceNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	configNode = xml_node_get_child(videoSourceNode, curElement);
	if(configNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
		return -1;
	}
	const GK_S8 *pszToken = xml_attr_get_data(configNode, "Token");
	if(pszToken)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].pszToken = malloc(MAX_TOKEN_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].pszToken, pszToken, MAX_TOKEN_LENGTH-1);

	}
	const GK_S8 *pszName = xml_attr_get_data(configNode, "Name");
	if(pszName)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].pszName = malloc(MAX_NAME_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].pszName, pszName, MAX_NAME_LENGTH-1);
	}
	valueNode = xml_node_get_child(configNode, "UseCount");
	if(valueNode == NULL)
	{
		return -1;
	}
	g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].useCount = atoi(valueNode->data);

	valueNode = xml_node_get_child(configNode, "SourceToken");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].pszSourceToken= malloc(MAX_TOKEN_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].pszSourceToken, valueNode->data, MAX_TOKEN_LENGTH-1);
	}
	valueNode = xml_node_get_child(configNode, "Bounds");
	if(valueNode == NULL)
	{
		return -1;
	}
	const GK_S8 *pszHeight = xml_attr_get_data(valueNode, "height");
	if(pszHeight)
	{
		sscanf(pszHeight, "%d", &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].stBounds.height);
	}
	const GK_S8 *pszWidth = xml_attr_get_data(valueNode, "width");
	if(pszWidth)
	{
		sscanf(pszWidth, "%d", &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].stBounds.width);
	}
	const GK_S8 *pszY = xml_attr_get_data(valueNode, "y");
	if(pszY)
	{
		sscanf(pszY, "%d", &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].stBounds.y);
	}
	const GK_S8 *pszX = xml_attr_get_data(valueNode, "x");
	if(pszX)
	{
		sscanf(pszX, "%d", &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[index].stBounds.x);
	}

	return 0;
}

static GK_S32 parseXmlAudioSource(GK_CHAR *rootElement, GK_S8 *curElement, GK_S32 index)
{
	XMLN *audioSourceNode = NULL;
	XMLN *configNode = NULL;
	XMLN *valueNode = NULL;

	audioSourceNode = xml_node_get_child(XMLTree, rootElement);
	if(audioSourceNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	configNode = xml_node_get_child(audioSourceNode, curElement);
	if(configNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
		return -1;
	}

	const GK_S8 *pszToken = xml_attr_get_data(configNode, "Token");
	if(pszToken)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioSource[index].pszToken = malloc(MAX_TOKEN_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraAudioSource[index].pszToken, pszToken,MAX_TOKEN_LENGTH-1);

	}

	const GK_S8 *pszName = xml_attr_get_data(configNode, "Name");
	if(pszName)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioSource[index].pszName = malloc(MAX_NAME_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraAudioSource[index].pszName, pszName,MAX_NAME_LENGTH-1);
	}

	valueNode = xml_node_get_child(configNode, "UseCount");
	if(valueNode == NULL)
	{
		return -1;
	}
	g_GkIpc_OnvifInf.gokeIPCameraAudioSource[index].useCount = atoi(valueNode->data);

	valueNode = xml_node_get_child(configNode, "SourceToken");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioSource[index].pszSourceToken = malloc(MAX_TOKEN_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraAudioSource[index].pszSourceToken, valueNode->data, MAX_TOKEN_LENGTH-1);
	}

	return 0;

}

static GK_S32 parseXmlVideoEncoder(GK_CHAR *rootElement, GK_S8 *curElement, GK_S32 index)
{
	XMLN *VideoEncoderNode = NULL;
	XMLN *configNode = NULL;
	XMLN *multiNode = NULL;
	XMLN *addrNode = NULL;
	XMLN *valueNode = NULL;
	VideoEncoderNode = xml_node_get_child(XMLTree, rootElement);
	if(VideoEncoderNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	configNode = xml_node_get_child(VideoEncoderNode, curElement);
	if(configNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
		return -1;
	}

	const GK_S8 *pszToken = xml_attr_get_data(configNode, "Token");
	if(pszToken)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].pszToken = malloc(MAX_TOKEN_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].pszToken, pszToken, MAX_TOKEN_LENGTH-1);

	}

	const GK_S8 *pszName = xml_attr_get_data(configNode, "Name");
	if(pszName)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].pszName = malloc(MAX_NAME_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].pszName, pszName, MAX_NAME_LENGTH-1);
	}

	valueNode = xml_node_get_child(configNode, "UseCount");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].useCount = atoi(valueNode->data);
	}

	multiNode = xml_node_get_child(configNode, "Multicast");
	if(multiNode == NULL)
	{
		return -1;
	}

	addrNode = xml_node_get_child(multiNode, "Address");
	if(addrNode == NULL)
	{
		return -1;
	}
	
	valueNode = xml_node_get_child(addrNode, "Type");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.stIPAddress.enType = atoi(valueNode->data);
	}

	valueNode = xml_node_get_child(addrNode, "IPv4Address");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.stIPAddress.pszIPv4Address = (GK_CHAR *)malloc(IPV4_STR_LENGTH);
		memset(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.stIPAddress.pszIPv4Address, 0, IPV4_STR_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.stIPAddress.pszIPv4Address, valueNode->data, IPV4_STR_LENGTH-1);
	}

	valueNode = xml_node_get_child(addrNode, "IPv6Address");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.stIPAddress.pszIPv6Address = (GK_CHAR *)malloc(IPV6_STR_LENGTH);
		memset(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.stIPAddress.pszIPv6Address, 0, IPV6_STR_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.stIPAddress.pszIPv6Address, valueNode->data, IPV6_STR_LENGTH-1);
	}

	valueNode = xml_node_get_child(multiNode, "Port");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.port = atoi(valueNode->data);
	}

	valueNode = xml_node_get_child(multiNode, "TTL");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.ttl = atoi(valueNode->data);
	}

	valueNode = xml_node_get_child(multiNode, "AutoStart");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].stMulticast.enAutoStart = atoi(valueNode->data);
	}

	valueNode = xml_node_get_child(configNode, "SessionTimeout");
	if(valueNode == NULL)
	{
		return -1;
	}
	if(valueNode->data != NULL)
	{
		sscanf(valueNode->data, "%lld", &g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[index].sessionTimeout);
	}

	return 0;
}

static GK_S32 parseXmlAudioEncoder(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index)
{
	XMLN *AudioEncoderNode = NULL;
	XMLN *ConfigNode = NULL;
	XMLN *multiNode = NULL;
	XMLN *addrNode = NULL;
	XMLN *ValueNode = NULL;
	AudioEncoderNode = xml_node_get_child(XMLTree, rootElement);
	if(AudioEncoderNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	ConfigNode = xml_node_get_child(AudioEncoderNode, curElement);
	if(ConfigNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
		return -1;
	}

	const GK_S8 *pszToken = xml_attr_get_data(ConfigNode, "Token");
	if(pszToken)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].pszToken = malloc(MAX_TOKEN_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].pszToken, pszToken, MAX_TOKEN_LENGTH-1);

	}

	const GK_S8 *pszName = xml_attr_get_data(ConfigNode, "Name");
	if(pszName)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].pszName= malloc(MAX_NAME_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].pszName, pszName, MAX_NAME_LENGTH-1);
	}

	ValueNode = xml_node_get_child(ConfigNode, "UseCount");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].useCount = atoi(ValueNode->data);
	}
#if 0
	/*Encoding*/
	ValueNode = xml_node_get_child(ConfigNode, "Encoding");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].enEncoding = atoi(ValueNode->data);
	}

	/*Bitrate*/
	ValueNode = xml_node_get_child(ConfigNode, "Bitrate");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].bitRate = atoi(ValueNode->data);
	}
	/*SampleRate*/
	ValueNode = xml_node_get_child(ConfigNode, "SampleRate");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].sampleRate = atoi(ValueNode->data);
	}
#endif
	
	multiNode = xml_node_get_child(ConfigNode, "Multicast");
	if(multiNode == NULL)
	{
		return -1;
	}

	addrNode = xml_node_get_child(multiNode, "Address");
	if(addrNode == NULL)
	{
		return -1;
	}

	ValueNode = xml_node_get_child(addrNode, "Type");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.stIPAddress.enType = atoi(ValueNode->data);
	}
	
	ValueNode = xml_node_get_child(addrNode, "IPv4Address");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.stIPAddress.pszIPv4Address = (GK_S8 *)malloc(IPV4_STR_LENGTH);
		memset(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.stIPAddress.pszIPv4Address, 0, IPV4_STR_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.stIPAddress.pszIPv4Address, ValueNode->data, IPV4_STR_LENGTH-1);
	}
	
	ValueNode = xml_node_get_child(addrNode, "IPv6Address");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.stIPAddress.pszIPv6Address = (GK_S8 *)malloc(IPV6_STR_LENGTH);
		memset(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.stIPAddress.pszIPv6Address, 0, IPV6_STR_LENGTH);
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.stIPAddress.pszIPv6Address, ValueNode->data, IPV6_STR_LENGTH-1);
	}

	ValueNode = xml_node_get_child(multiNode, "Port");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.port = atoi(ValueNode->data);
	}

	ValueNode = xml_node_get_child(multiNode, "TTL");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.ttl = atoi(ValueNode->data);
	}

	ValueNode = xml_node_get_child(multiNode, "AutoStart");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].stMulticast.enAutoStart = atoi(ValueNode->data);
	}

	ValueNode = xml_node_get_child(ConfigNode, "SessionTimeout");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		sscanf(ValueNode->data, "%lld", &g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[index].sessionTimeout);
	}

	return 0;

}

static GK_S32 parseXmlRelayOutputs(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index)
{
	XMLN *relayOutputNode = NULL;
	XMLN *configNode = NULL;
	XMLN *valueNode = NULL;
	relayOutputNode = xml_node_get_child(XMLTree, rootElement);
	if(relayOutputNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", rootElement);
		return -1;
	}
	configNode = xml_node_get_child(relayOutputNode, curElement);
	if(configNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", curElement);
		return -1;
	}

	const GK_S8 *pszToken = xml_attr_get_data(configNode, "Token");
	if(pszToken)
	{
    	strncpy(g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[index].aszToken, pszToken, MAX_TOKEN_LENGTH-1);
	}

	valueNode = xml_node_get_child(configNode, "Mode");
	if(valueNode == NULL)
	{
        ONVIF_ERR();
		return -1;
	}
	if(valueNode->data != NULL)
	{
	    g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[index].stProperties.enMode = atoi(valueNode->data);
    }

	valueNode = xml_node_get_child(configNode, "DelayTime");
	if(valueNode == NULL)
	{
        ONVIF_ERR();
		return -1;
	}
	if(valueNode->data != NULL)
	{
        g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[index].stProperties.delayTime = atoll(valueNode->data);
	}

	valueNode = xml_node_get_child(configNode, "IdleState");
	if(valueNode == NULL)
	{
        ONVIF_ERR();
		return -1;
	}
	if(valueNode->data != NULL)
	{
        g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[index].stProperties.enIdleState = atoi(valueNode->data);
	}

	return 0;
}


static GK_S32 parseXmlMediaProfile(GK_S8 *rootElement, GK_S8 *curElement, GK_S32 Index)
{
    GK_S32 i = 0;
	XMLN *MediaProfileNode;
	XMLN *ConfigNode;
	XMLN *ValueNode;
	MediaProfileNode = xml_node_get_child(XMLTree, rootElement);
	if(MediaProfileNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	ConfigNode = xml_node_get_child(MediaProfileNode, curElement);
	if(ConfigNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
		return -1;
	}

	const GK_S8 *pszFixed = xml_attr_get_data(ConfigNode, "Fixed");
	if(pszFixed)
	{
		GK_S32 Fixed = 0;
		sscanf(pszFixed, "%d", &Fixed);
		g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].enFixed = (GONVIF_Boolean_E)Fixed;
	}

	const GK_S8 *pszToken = xml_attr_get_data(ConfigNode, "Token");
	if(pszToken)
	{
		g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pszToken= malloc(MAX_TOKEN_LENGTH);
    	strcpy(g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pszToken, pszToken);

	}

	const GK_S8 *pszName = xml_attr_get_data(ConfigNode, "Name");
	if(pszName)
	{
		g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pszName = malloc(MAX_NAME_LENGTH);
    	strcpy(g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pszName, pszName);
	}

	ValueNode = xml_node_get_child(ConfigNode, "VideoSourceConfiguration");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		GK_S8 *pszVSToken = ValueNode->data;
		for(i = 0; i < g_GkIpc_OnvifInf.VideoSourceNum; i++)
		{
        	if(strcmp(pszVSToken,g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszToken) == 0)
        	{
				g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pstVideoSourceConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i];
			}

		}
	}

	ValueNode = xml_node_get_child(ConfigNode, "AudioSourceConfiguration");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		GK_S8 *pszASToken = ValueNode->data;
		for(i = 0; i < g_GkIpc_OnvifInf.AudioSourceNum; i++)
		{
        	if(strcmp(pszASToken,g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszToken) == 0)
        	{
				g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pstAudioSourceConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i];
			}

		}
	}

	ValueNode = xml_node_get_child(ConfigNode, "VideoEncoderConfiguration");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		GK_S8 *pszVEToken = ValueNode->data;
		for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
		{
        	if(strcmp(pszVEToken,g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].pszToken) == 0)
        	{
				g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pstVideoEncoderConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i];
			}

		}
	}

	ValueNode = xml_node_get_child(ConfigNode, "AudioEncoderConfiguration");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		GK_S8 *pszAEToken = ValueNode->data;
		for(i = 0; i < g_GkIpc_OnvifInf.AudioEncodeNum; i++)
		{
        	if(strcmp(pszAEToken,g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].pszToken) == 0)
        	{
				g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pstAudioEncoderConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i];
			}
		}
	}

	ValueNode = xml_node_get_child(ConfigNode, "VideoAnalyticConfiguration");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		GK_S8 *pszVAToken = ValueNode->data;
		for(i = 0; i < g_GkIpc_OnvifInf.VideoAnalyticNum; i++)
		{
        	if(strcmp(pszVAToken,g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].pszToken) == 0)
        	{
				g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pstVideoAnalyticsConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i];
			}
		}
	}

	ValueNode = xml_node_get_child(ConfigNode, "PTZConfiguration");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		GK_S8 *PTZToken = ValueNode->data;
		for(i = 0; i < g_GkIpc_OnvifInf.PTZConfigurationNum; i++)
		{
        	if(strcmp(PTZToken, g_GkIpc_OnvifInf.gokePTZConfiguration[i].token) == 0)
        	{
				g_GkIpc_OnvifInf.gokeIPCameraProfiles[Index].pstPTZConfiguration = &g_GkIpc_OnvifInf.gokePTZConfiguration[i];
			}
		}
	}
	return 0;
}

static GK_S32 parseXmlVideoSourceOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index)
{
	XMLN *VideoSourceOptionNode = NULL;
	XMLN *ConfigNode = NULL;
	XMLN *boundsRange = NULL;
	XMLN *ValueNode = NULL;
	VideoSourceOptionNode = xml_node_get_child(XMLTree, rootElement);
	if(VideoSourceOptionNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	ConfigNode = xml_node_get_child(VideoSourceOptionNode, curElement);
	if(ConfigNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
		return -1;
	}

	boundsRange = xml_node_get_child(ConfigNode, "BoundsRange");
	if(boundsRange == NULL)
	{
		return -1;
	}

	ValueNode = xml_node_get_child(boundsRange, "XRange");
	if(ValueNode == NULL)
	{
		return -1;
	}
	const GK_S8 *pszXMin = xml_attr_get_data(ValueNode, "Min");
	if(pszXMin)
	{
		sscanf(pszXMin, "%d", &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].stBoundsRange.stXRange.min);
	}
	const GK_S8 *pszXMax = xml_attr_get_data(ValueNode, "Max");
	if(pszXMax)
	{
		sscanf(pszXMax, "%d", &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].stBoundsRange.stXRange.max);
	}

	ValueNode = xml_node_get_child(boundsRange, "YRange");
	if(ValueNode == NULL)
	{
		return -1;
	}
	const GK_S8 *pszYMin = xml_attr_get_data(ValueNode, "Min");
	if(pszYMin)
	{
		sscanf(pszYMin, "%d", &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].stBoundsRange.stYRange.min);
	}
	const GK_S8 *pszYMax = xml_attr_get_data(ValueNode, "Max");
	if(pszYMax)
	{
		sscanf(pszYMax, "%d", &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].stBoundsRange.stYRange.max);
	}

	ValueNode = xml_node_get_child(boundsRange, "WidthRange");
	if(ValueNode == NULL)
	{
		return -1;
	}
	const GK_S8 *pszWidthMin = xml_attr_get_data(ValueNode, "Min");
	if(pszWidthMin)
	{
		sscanf(pszWidthMin, "%d", &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].stBoundsRange.stWidthRange.min);
	}
	const GK_S8 *pszWidthMax = xml_attr_get_data(ValueNode, "Max");
	if(pszWidthMax)
	{
		sscanf(pszWidthMax, "%d", &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].stBoundsRange.stWidthRange.max);
	}

	ValueNode = xml_node_get_child(boundsRange, "HeightRange");
	if(ValueNode == NULL)
	{
		return -1;
	}
	const GK_S8 *pszHeightMin = xml_attr_get_data(ValueNode, "Min");
	if(pszHeightMin)
	{
		sscanf(pszHeightMin, "%d", &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].stBoundsRange.stHeightRange.min);
	}
	const GK_S8 *pszHeightMax = xml_attr_get_data(ValueNode, "Max");
	if(pszHeightMax)
	{
		sscanf(pszHeightMax, "%d", &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].stBoundsRange.stHeightRange.max);
	}

	ValueNode = xml_node_get_child(ConfigNode, "sizeVideoSourceTokensAvailable");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].sizeVideoSourceTokensAvailable = atoi(ValueNode->data);
	}

	ValueNode = xml_node_get_child(ConfigNode, "VideoSourceTokensAvailable");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
   		g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].pszVideoSourceTokensAvailable= (GK_CHAR **)malloc(sizeof(GK_CHAR *));
		*g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].pszVideoSourceTokensAvailable = (GK_CHAR *)malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
		if(*g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].pszVideoSourceTokensAvailable == NULL)
		{
			return -1;
		}
	    memset(*g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].pszVideoSourceTokensAvailable, 0, sizeof(GK_S8)*MAX_TOKEN_LENGTH);
    	strncpy(*g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[index].pszVideoSourceTokensAvailable, ValueNode->data, MAX_TOKEN_LENGTH-1);
	}

	return 0;

}

static GK_S32 parseXmlAudioSourceOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index)
{
	XMLN *AudioSourceOptionNode = NULL;
	XMLN *ConfigNode = NULL;
	XMLN *ValueNode = NULL;
	AudioSourceOptionNode = xml_node_get_child(XMLTree, rootElement);
	if(AudioSourceOptionNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	ConfigNode = xml_node_get_child(AudioSourceOptionNode, curElement);
	if(ConfigNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
		return -1;
	}

	ValueNode = xml_node_get_child(ConfigNode, "sizeInputTokensAvailable");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[index].sizeInputTokensAvailable = atoi(ValueNode->data);
	}

	ValueNode = xml_node_get_child(ConfigNode, "InputTokensAvailable");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
   		g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[index].pszInputTokensAvailable = (GK_CHAR **)malloc(sizeof(GK_CHAR *));
		*g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[index].pszInputTokensAvailable = (GK_CHAR *)malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
		if(*g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[index].pszInputTokensAvailable == NULL)
		{
			return -1;
		}
	    memset(*g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[index].pszInputTokensAvailable, 0, sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
    	strncpy(*g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[index].pszInputTokensAvailable, ValueNode->data, MAX_TOKEN_LENGTH-1);
	}
	return 0;
}

static GK_S32 parseXmlVideoEncoderOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index)
{
	XMLN *VideoEncoderOptionNode = NULL;
	XMLN *ConfigNode = NULL;
	XMLN *EncodeNode = NULL;
	XMLN *ValueNode = NULL;
	GK_S32 sizeResolutions = 0;
	const GK_S8 *pszFRMin = NULL;
	const GK_S8 *pszFRMax = NULL;
	const GK_S8 *pszEIMin = NULL;
	const GK_S8 *pszEIMax = NULL;
	const GK_S8 *pszBMin = NULL;
	const GK_S8 *pszBMax = NULL;
	VideoEncoderOptionNode = xml_node_get_child(XMLTree, rootElement);
	if(VideoEncoderOptionNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	ConfigNode = xml_node_get_child(VideoEncoderOptionNode, curElement);
	if(ConfigNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", curElement);
		return -1;
	}

	ValueNode = xml_node_get_child(ConfigNode, "QualityRange");
	if(ValueNode == NULL)
	{		
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "QualityRange");
		return -1;
	}
	const GK_S8 *pszQRMin = xml_attr_get_data(ValueNode, "Min");
	if(pszQRMin)
	{
		sscanf(pszQRMin, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stQualityRange.min);
	}
	const GK_S8 *pszQRMax = xml_attr_get_data(ValueNode, "Max");
	if(pszQRMax)
	{
		sscanf(pszQRMax, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stQualityRange.max);
	}
#if 0
	/*JPEG*/
	EncodeNode = xml_node_get_child(ConfigNode, "JPEG");
	if(EncodeNode == NULL)
	{		
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "JPEG");
		return -1;
	}
	/*sizeResolutionsAvailable*/
	ValueNode = xml_node_get_child(EncodeNode, "sizeResolutionsAvailable");
	if(ValueNode == NULL)
	{		
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "sizeResolutionsAvailable");
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.sizeResolutionsAvailable = atoi(ValueNode->data);
	}

	/*JPEG ResolutionsAvailable*/
	sizeResolutions = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.sizeResolutionsAvailable;
	if(sizeResolutions != 0)
	{
		g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.pstResolutionsAvailable = (GONVIF_MEDIA_VideoResolution_S *)malloc(sizeof(GONVIF_MEDIA_VideoResolution_S)*sizeResolutions);
		if(g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.pstResolutionsAvailable == NULL)
		{
			ONVIF_ERR("Fail to allow memory.");
			return -1;
		}
		memset(g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.pstResolutionsAvailable, 0, sizeof(GONVIF_MEDIA_VideoResolution_S)*sizeResolutions);
		GK_S8 AvailableBuf[32] = {0};
		memset(AvailableBuf, 0, sizeof(AvailableBuf));
		GK_S32 i = 0;
		for(i = 0; i < sizeResolutions; i++)
		{
			snprintf(AvailableBuf, sizeof(AvailableBuf), "ResolutionsAvailable%d", i);
			ValueNode = xml_node_get_child(EncodeNode, AvailableBuf);
			if(ValueNode == NULL)
			{
				ONVIF_ERR("Unable to find %s element in XML tree!\n", AvailableBuf);
				return -1;
			}
			const GK_S8 *pszWidth = xml_attr_get_data(ValueNode, "Width");
			if(pszWidth)
			{
				sscanf(pszWidth, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.pstResolutionsAvailable[i].width);
			}
			const GK_S8 *pszHeight = xml_attr_get_data(ValueNode, "Height");
			if(pszHeight)
			{
				sscanf(pszHeight, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.pstResolutionsAvailable[i].height);
			}
		}
	}
	/*JPEG FrameRateRange*/
	ValueNode = xml_node_get_child(EncodeNode, "FrameRateRange");
	if(ValueNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "FrameRateRange");
		return -1;
	}
	pszFRMin = xml_attr_get_data(ValueNode, "Min");
	if(pszFRMin)
	{
		sscanf(pszFRMin, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.stFrameRateRange.min);
	}
	pszFRMax = xml_attr_get_data(ValueNode, "Max");
	if(pszFRMax)
	{
		sscanf(pszFRMax, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.stFrameRateRange.max);
	}
	/*JPEG EncodingIntervalRange*/
	ValueNode = xml_node_get_child(EncodeNode, "EncodingIntervalRange");
	if(ValueNode == NULL)
	{		
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "EncodingIntervalRange");
		return -1;
	}
	pszEIMin = xml_attr_get_data(ValueNode, "Min");
	if(pszEIMin)
	{
		sscanf(pszEIMin, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.stEncodingIntervalRange.min);
	}
	pszEIMax = xml_attr_get_data(ValueNode, "Max");
	if(pszEIMax)
	{
		sscanf(pszEIMax, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stJPEG.stEncodingIntervalRange.max);
	}
	/*JPEG BitrateRange*/
	ValueNode = xml_node_get_child(EncodeNode, "BitrateRange");
	if(ValueNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "BitrateRange");
		return -1;
	}
	pszBMin = xml_attr_get_data(ValueNode, "Min");
	if(pszBMin)
	{
		sscanf(pszBMin, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stExtension.stJPEG.stBitrateRange.min);
	}
	const GK_S8 *pszBMax = xml_attr_get_data(ValueNode, "Max");
	if(pszBMax)
	{
		sscanf(pszBMax, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stExtension.stJPEG.stBitrateRange.max);
	}
#endif	
	EncodeNode = xml_node_get_child(ConfigNode, "H264");
	if(EncodeNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "H264");
		return -1;
	}

	ValueNode = xml_node_get_child(EncodeNode, "sizeResolutionsAvailable");
	if(ValueNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "sizeResolutionsAvailable");
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.sizeResolutionsAvailable = atoi(ValueNode->data);
	}

	sizeResolutions = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.sizeResolutionsAvailable;
	if(sizeResolutions != 0)
	{
		g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.pstResolutionsAvailable  = (GONVIF_MEDIA_VideoResolution_S *)malloc(sizeof(GONVIF_MEDIA_VideoResolution_S)*sizeResolutions);
		if(g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.pstResolutionsAvailable == NULL)
		{			
			ONVIF_ERR("Fail to allow memory.");
			return -1;
		}
		memset(g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.pstResolutionsAvailable, 0, sizeof(GONVIF_MEDIA_VideoResolution_S)*sizeResolutions);
		GK_S8 AvailableBuf[32] = {0};
		memset(AvailableBuf, 0, sizeof(AvailableBuf));
		GK_S32 i = 0;
		for(i = 0; i < sizeResolutions; i++)
		{
			snprintf(AvailableBuf, sizeof(AvailableBuf), "ResolutionsAvailable%d", i);
			ValueNode = xml_node_get_child(EncodeNode, AvailableBuf);
			if(ValueNode == NULL)
			{
				ONVIF_ERR("Unable to find %s element in XML tree!\n", AvailableBuf);
				return -1;
			}
			const GK_S8 *pszWidth = xml_attr_get_data(ValueNode, "Width");
			if(pszWidth)
			{
				sscanf(pszWidth, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.pstResolutionsAvailable[i].width);
			}
			const GK_S8 *pszHeight = xml_attr_get_data(ValueNode, "Height");
			if(pszHeight)
			{
				sscanf(pszHeight, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.pstResolutionsAvailable[i].height);
			}
		}
	}

	ValueNode = xml_node_get_child(EncodeNode, "GovLengthRange");
	if(ValueNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "GovLengthRange");
		return -1;
	}
	const GK_S8 *pszGLMin = xml_attr_get_data(ValueNode, "Min");
	if(pszGLMin)
	{
		sscanf(pszGLMin, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.stGovLengthRange.min);
	}
	const GK_S8 *pszGLMax = xml_attr_get_data(ValueNode, "Max");
	if(pszGLMax)
	{
		sscanf(pszGLMax, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.stGovLengthRange.max);
	}

	ValueNode = xml_node_get_child(EncodeNode, "FrameRateRange");
	if(ValueNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "FrameRateRange");
		return -1;
	}
	pszFRMin = xml_attr_get_data(ValueNode, "Min");
	if(pszFRMin)
	{
		sscanf(pszFRMin, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.stFrameRateRange.min);
	}
	pszFRMax = xml_attr_get_data(ValueNode, "Max");
	if(pszFRMax)
	{
		sscanf(pszFRMax, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.stFrameRateRange.max);
	}

	ValueNode = xml_node_get_child(EncodeNode, "EncodingIntervalRange");
	if(ValueNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "EncodingIntervalRange");
		return -1;
	}
	pszEIMin = xml_attr_get_data(ValueNode, "Min");
	if(pszEIMin)
	{
		sscanf(pszEIMin, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.stEncodingIntervalRange.min);
	}
	pszEIMax = xml_attr_get_data(ValueNode, "Max");
	if(pszEIMax)
	{
		sscanf(pszEIMax, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.stEncodingIntervalRange.max);
	}

	ValueNode = xml_node_get_child(EncodeNode, "sizeH264ProfilesSupported");
	if(ValueNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "sizeH264ProfilesSupported");
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.sizeH264ProfilesSupported = atoi(ValueNode->data);
	}

	ValueNode = xml_node_get_child(EncodeNode, "H264ProfilesSupported");
	if(ValueNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "H264ProfilesSupported");
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.penH264ProfilesSupported = (GONVIF_MEDIA_H264Profile_E *)malloc(sizeof(GONVIF_MEDIA_H264Profile_E));
		if(g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.penH264ProfilesSupported == NULL)
		{
			ONVIF_ERR("Fail to allow memory.");
			return -1;
		}
		memset(g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.penH264ProfilesSupported, 0, sizeof(GONVIF_MEDIA_H264Profile_E));
		*g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stH264.penH264ProfilesSupported = atoi(ValueNode->data);
	}

	ValueNode = xml_node_get_child(EncodeNode, "BitrateRange");
	if(ValueNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", "BitrateRange");
		return -1;
	}
	pszBMin = xml_attr_get_data(ValueNode, "Min");
	if(pszBMin)
	{
		sscanf(pszBMin, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stExtension.stH264.stBitrateRange.min);
	}
	pszBMax = xml_attr_get_data(ValueNode, "Max");
	if(pszBMax)
	{
		sscanf(pszBMax, "%d", &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[index].stExtension.stH264.stBitrateRange.max);
	}
	
	return 0;
}

static GK_S32 parseXmlAudioEncoderOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index)
{
	XMLN *AudioEncoderOptionNode = NULL;
	XMLN *ConfigNode = NULL;
	XMLN *optionNode = NULL;
	XMLN *bitrateListNode = NULL;
	XMLN *sampleRateListNode = NULL;
	XMLN *ValueNode = NULL;
	AudioEncoderOptionNode = xml_node_get_child(XMLTree, rootElement);
	if(AudioEncoderOptionNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	ConfigNode = xml_node_get_child(AudioEncoderOptionNode, curElement);
	if(ConfigNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
		return -1;
	}

	ValueNode = xml_node_get_child(ConfigNode,  "sizeOptions");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].sizeOptions = atoi(ValueNode->data);
	}
	g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions = (GONVIF_MEDIA_AudioEncoderConfigurationOption_S *)malloc(
                     sizeof(GONVIF_MEDIA_AudioEncoderConfigurationOption_S)*g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].sizeOptions);
	if(g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions == NULL)
	{
		return -1;
	}
	memset(g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions, 0, sizeof(GONVIF_MEDIA_AudioEncoderConfigurationOption_S)*
                    g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].sizeOptions);

    GK_S32 i = 0;    
	GK_S32 j = 0;
    GK_CHAR optionBuf[32];
    for(i = 0; i < g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].sizeOptions; i++)
    {  
        memset(optionBuf, 0, sizeof(optionBuf));
        snprintf(optionBuf, sizeof(optionBuf), "Option%d", i);
        optionNode = xml_node_get_child(ConfigNode, optionBuf);
        if(optionNode == NULL)
        {
            return -1;
        }

    	ValueNode = xml_node_get_child(optionNode, "Encoding");
    	if(ValueNode == NULL)
    	{
    		return -1;
    	}
    	if(ValueNode->data != NULL)
    	{
    		g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].enEncoding = atoi(ValueNode->data);
    	}

    	bitrateListNode = xml_node_get_child(optionNode, "BitrateList");
    	if(bitrateListNode == NULL)
    	{
    		return -1;
    	}
    	const GK_S8 *pszBLSizeItems = xml_attr_get_data(bitrateListNode, "sizeItems");
    	if(pszBLSizeItems)
    	{
    		sscanf(pszBLSizeItems, "%d", &g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stBitrateList.sizeItems);
    	}
    	if(g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stBitrateList.sizeItems != 0)
    	{
    		GK_S32 BLItemNum = g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stBitrateList.sizeItems;

    		g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stBitrateList.pItems = (GK_S32 *)malloc(BLItemNum * sizeof(GK_S32));
            if(g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stBitrateList.pItems == NULL)
            {
                return -1;
            }
    		GK_S8 ItemBuf[32] = {0};
    		memset(ItemBuf, 0, sizeof(ItemBuf));
    		for(j = 0; j < BLItemNum; j++)
    		{
    			snprintf(ItemBuf, sizeof(ItemBuf), "BLItems%d", j);
    			ValueNode = xml_node_get_child(bitrateListNode, ItemBuf);
    			if(ValueNode == NULL)
    			{
    				return -1;
    			}
    			g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stBitrateList.pItems[j] = atoi(ValueNode->data);    			
    		}
    	}

    	sampleRateListNode = xml_node_get_child(optionNode, "SampleRateList");
    	if(sampleRateListNode == NULL)
    	{
    		return -1;
    	}
    	const GK_S8 *pszSRSizeItems = xml_attr_get_data(sampleRateListNode, "sizeItems");
    	if(pszSRSizeItems)
    	{
    		sscanf(pszSRSizeItems, "%d", &g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stSampleRateList.sizeItems);
    	}
    	if(g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stSampleRateList.sizeItems != 0)
    	{
    		GK_S32 SRItemNum = g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stSampleRateList.sizeItems;
    		g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stSampleRateList.pItems = (GK_S32 *)malloc(SRItemNum * sizeof(GK_S32));
            if(g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stSampleRateList.pItems == NULL)
            {
                return -1;
            }
    		GK_S8 ItemBuf[32] = {0};
    		memset(ItemBuf, 0, sizeof(ItemBuf));
    		for(j = 0; j < SRItemNum; j++)
    		{
    			snprintf(ItemBuf, sizeof(ItemBuf), "SRItems%d", j);
    			ValueNode = xml_node_get_child(sampleRateListNode, ItemBuf);
    			if(ValueNode == NULL)
    			{
    				return -1;
    			}
    			g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[index].pstOptions[i].stSampleRateList.pItems[j] = atoi(ValueNode->data);
    		}
    	}
    }
	return 0;
}

static GK_S32 parseXmlPTZConfigurationOption(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index)
{
    g_GkIpc_OnvifInf.sizePTZConfigurationOptions = 1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions = (GONVIF_PTZ_ConfigurationOptions_S *)malloc(g_GkIpc_OnvifInf.sizePTZConfigurationOptions * sizeof(GONVIF_PTZ_ConfigurationOptions_S));

    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.sizeAbsolutePanTiltPositionSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stAbsolutePanTiltPositionSpace[0].aszURI, 
        "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stAbsolutePanTiltPositionSpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stAbsolutePanTiltPositionSpace[0].stXRange.max = 1; 
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stAbsolutePanTiltPositionSpace[0].stYRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stAbsolutePanTiltPositionSpace[0].stYRange.max = 1;

    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.sizeAbsoluteZoomPositionSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stAbsoluteZoomPositionSpace[0].aszURI, 
        "http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stAbsoluteZoomPositionSpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stAbsoluteZoomPositionSpace[0].stXRange.max = 1;

    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.sizeRelativePanTiltTranslationSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stRelativePanTiltTranslationSpace[0].aszURI, 
        "http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stRelativePanTiltTranslationSpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stRelativePanTiltTranslationSpace[0].stXRange.max = 1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stRelativePanTiltTranslationSpace[0].stYRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stRelativePanTiltTranslationSpace[0].stYRange.max = 1;

    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.sizeRelativeZoomTranslationSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stRelativeZoomTranslationSpace[0].aszURI, 
        "http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stRelativeZoomTranslationSpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stRelativeZoomTranslationSpace[0].stXRange.max = 1;

    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.sizeContinuousPanTiltVelocitySpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stContinuousPanTiltVelocitySpace[0].aszURI, 
        "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stContinuousPanTiltVelocitySpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stContinuousPanTiltVelocitySpace[0].stXRange.max = 1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stContinuousPanTiltVelocitySpace[0].stYRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stContinuousPanTiltVelocitySpace[0].stYRange.max = 1;

    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.sizeContinuousZoomVelocitySpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stContinuousZoomVelocitySpace[0].aszURI, 
        "http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stContinuousZoomVelocitySpace[0].stXRange.min = -1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stContinuousZoomVelocitySpace[0].stXRange.max = 1;

    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.sizePanTiltSpeedSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stPanTiltSpeedSpace[0].aszURI,
        "http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stPanTiltSpeedSpace[0].stXRange.min = 0;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stPanTiltSpeedSpace[0].stXRange.max = 1;

    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.sizeZoomSpeedSpace = 1;
    strncpy(g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stZoomSpeedSpace[0].aszURI, 
        "http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace", MAX_URI_LENGTH-1);
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stZoomSpeedSpace[0].stXRange.min = 0;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stSpaces.stZoomSpeedSpace[0].stXRange.max = 1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stPTZTimeout.min = 1;
    g_GkIpc_OnvifInf.pstPTZConfigurationOptions[0].stPTZTimeout.max = 250;
    
    return 0;
}

static GK_S32 parseXmlVideoAnalytics(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index)
{
	XMLN *VideoAnalyticNode = NULL;
	XMLN *ConfigNode = NULL;
	XMLN *ModuleNode = NULL;
	XMLN *RuleNode = NULL;
	XMLN *ItemListNode = NULL;
    XMLN *SimpItemNode = NULL;
    XMLN *ElementItemNode = NULL;
    XMLN *ItemNode = NULL;
	XMLN *ValueNode = NULL;
    XMLN *LastNode = NULL;
    GK_S32 SimpItemNum = 0;
    GK_S32 ElementItemNum = 0;
	VideoAnalyticNode = xml_node_get_child(XMLTree, rootElement);
	if(VideoAnalyticNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	ConfigNode = xml_node_get_child(VideoAnalyticNode, curElement);
	if(ConfigNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
		return -1;
	}
	
	ValueNode = xml_node_get_child(ConfigNode, "Token");
	if(ValueNode != NULL && ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszToken = malloc(MAX_NAME_LENGTH);
		if(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszToken == NULL)
		{
			ONVIF_ERR("malloc failed\n");
			return -1;
		}
		memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszToken, 0, MAX_NAME_LENGTH);
		strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszToken, ValueNode->data, MAX_NAME_LENGTH);
	}
	else
	{
		ONVIF_ERR("Unable to find Name element in XML tree!\n");
		return -1;
	}

	ValueNode = xml_node_get_child(ConfigNode, "Name");
	if(ValueNode != NULL && ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszName = malloc(MAX_NAME_LENGTH);
		if(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszName == NULL)
		{
			ONVIF_ERR("malloc failed\n");
			return -1;
		}
		memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszName, 0, MAX_NAME_LENGTH);
		strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].pszName, ValueNode->data, MAX_NAME_LENGTH-1);
	}
	else
	{
		ONVIF_ERR("Unable to find Name element in XML tree!\n");
		return -1;
	}

	ValueNode = xml_node_get_child(ConfigNode, "UseCount");
	if(ValueNode == NULL)
	{
		return -1;
	}
	if(ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].useCount = atoi(ValueNode->data);
	}

	ValueNode = xml_node_get_child(ConfigNode, "AnalyticsModule");
	if(ValueNode == NULL)
	{
		return -1;
	}
    GK_S32 moduleNum = 0;
	const GK_S8 *pszNum = xml_attr_get_data(ValueNode, "Num");
	if(pszNum)
	{
		sscanf(xml_attr_get_data(ValueNode, "Num"), "%d", &moduleNum);
	}
	else
	{
		ONVIF_ERR("request config num failed or xml file content error!\n");
		return -1;
	}
    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.sizeAnalyticsModule = moduleNum;
    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule = malloc(sizeof(GONVIF_MEDIA_Config_S)*moduleNum);
    memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule, 0, sizeof(sizeof(GONVIF_MEDIA_Config_S)*moduleNum));
    GK_S8 ModuleBuf[32] = {0};
    memset(ModuleBuf, 0, sizeof(ModuleBuf));
    GK_S32 i = 0;
    for(i = 0; i < moduleNum; i++)
    {
        snprintf(ModuleBuf, sizeof(ModuleBuf), "Module%d", i);
        ModuleNode = xml_node_get_child(ValueNode, ModuleBuf);
        if(ModuleNode == NULL)
        {
            return -1;
        }

        LastNode = xml_node_get_child(ModuleNode, "Name");
        if(LastNode != NULL && LastNode->data != NULL)
        {
            g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].pszName = malloc(MAX_NAME_LENGTH);
            memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].pszName, 0, MAX_NAME_LENGTH);
            strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].pszName, LastNode->data, MAX_NAME_LENGTH-1);
        }
        else
        {
            ONVIF_ERR("Unable to find Name element in XML tree!\n");
            return -1;
        }

        LastNode = xml_node_get_child(ModuleNode, "Type");
        if(LastNode != NULL && LastNode->data != NULL)
        {
            g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].pszType = malloc(MAX_NAME_LENGTH);
            memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].pszType, 0, MAX_NAME_LENGTH);
            strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].pszType, LastNode->data, MAX_NAME_LENGTH-1);
        }
        else
        {
            ONVIF_ERR("Unable to find Name element in XML tree!\n");
            return -1;
        }

        ItemListNode = xml_node_get_child(ModuleNode, "ItemList");
        if(ItemListNode == NULL)
        {
            return -1;
        }
        SimpItemNode = xml_node_get_child(ItemListNode, "SimpItem");
        if(SimpItemNode == NULL)
        {
            return -1;
        }
        pszNum = xml_attr_get_data(SimpItemNode, "Num");
        if(pszNum)
        {
            sscanf(xml_attr_get_data(SimpItemNode, "Num"), "%d", &SimpItemNum);
        }
        else
        {
            ONVIF_ERR("request config num failed or xml file content error!\n");
            return -1;
        }
        g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.sizeSimpleItem = SimpItemNum;
        g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem = malloc(sizeof(GONVIF_MEDIA_ItemList_SimpleItem_S)*SimpItemNum);
        memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem,0,sizeof(GONVIF_MEDIA_ItemList_SimpleItem_S)*SimpItemNum);
        GK_S8 ItemBuf[32] = {0};
        memset(ItemBuf, 0, sizeof(ItemBuf));
        GK_S32 j = 0;
        for(j = 0; j < SimpItemNum; j++)
        {
            snprintf(ItemBuf, sizeof(ItemBuf), "Item%d", i);
            ItemNode = xml_node_get_child(SimpItemNode, ItemBuf);
            if(ItemNode == NULL)
            {
                return -1;
            }

            LastNode = xml_node_get_child(ItemNode, "Name");
            if(LastNode != NULL && LastNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem[j].pszName = malloc(MAX_NAME_LENGTH);
                memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem[j].pszName, 0, MAX_NAME_LENGTH);
                strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem[j].pszName, LastNode->data, MAX_NAME_LENGTH-1);
            }
            else
            {
                ONVIF_ERR("Unable to find Name element in XML tree!\n");
                return -1;
            }

            LastNode = xml_node_get_child(ItemNode, "Value");
            if(LastNode != NULL && LastNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem[j].pszValue= malloc(MAX_NAME_LENGTH);
                memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem[j].pszValue, 0, MAX_NAME_LENGTH);
                strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstSimpleItem[j].pszValue, LastNode->data, MAX_NAME_LENGTH-1);
            }
            else
            {
                ONVIF_ERR("Unable to find Name element in XML tree!\n");
                return -1;
            }
        }

        ElementItemNode = xml_node_get_child(ItemListNode, "ElementItem");
        pszNum = xml_attr_get_data(ElementItemNode, "Num");
        if(pszNum)
        {
            sscanf(xml_attr_get_data(ElementItemNode, "Num"), "%d", &ElementItemNum);
        }
        else
        {
            ONVIF_ERR("request config num failed or xml file content error!\n");
            return -1;
        }
        g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.sizeElementItem = ElementItemNum;
        g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstElementItem= malloc(sizeof(GONVIF_MEDIA_ItemList_ElementItem_S)*ElementItemNum);
        memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstElementItem,0,sizeof(GONVIF_MEDIA_ItemList_ElementItem_S)*ElementItemNum);
        memset(ItemBuf, 0, sizeof(ItemBuf));
        for(j = 0; j < ElementItemNum; j++)
        {
            snprintf(ItemBuf, sizeof(ItemBuf), "Item%d", i);
            ItemNode = xml_node_get_child(ElementItemNode, ItemBuf);
            if(ItemNode == NULL)
            {
                return -1;
            }

            LastNode = xml_node_get_child(ItemNode, "Name");
            if(LastNode != NULL && LastNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstElementItem[j].pszName = malloc(MAX_NAME_LENGTH);
                memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstElementItem[j].pszName, 0, MAX_NAME_LENGTH);
                strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stAnalyticsEngineConfiguration.pstAnalyticsModule[i].stParameters.pstElementItem[j].pszName, LastNode->data, MAX_NAME_LENGTH-1);
            }
            else
            {
                ONVIF_ERR("Unable to find Name element in XML tree!\n");
                return -1;
            }
        }
    }

	ValueNode = xml_node_get_child(ConfigNode, "AnalyticsRule");
	if(ValueNode == NULL)
	{
		return -1;
	}
    GK_S32 ruleNum = 0;
	pszNum = xml_attr_get_data(ValueNode, "Num");
	if(pszNum)
	{
		sscanf(xml_attr_get_data(ValueNode, "Num"), "%d", &ruleNum);
	}
	else
	{
		ONVIF_ERR("request config num failed or xml file content error!\n");
		return -1;
	}
    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.sizeRule = ruleNum;
    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule = malloc(sizeof(GONVIF_MEDIA_Config_S)*ruleNum);
    memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule, 0, sizeof(sizeof(GONVIF_MEDIA_Config_S)*ruleNum));
    GK_S8 RuleBuf[32] = {0};
    memset(RuleBuf, 0, sizeof(RuleBuf));
    for(i = 0; i < ruleNum; i++)
    {
        snprintf(RuleBuf, sizeof(RuleBuf), "Rule%d", i);
        RuleNode = xml_node_get_child(ValueNode, RuleBuf);
        if(RuleNode == NULL)
        {
            return -1;
        }
		
        LastNode = xml_node_get_child(RuleNode, "Name");
        if(LastNode != NULL && LastNode->data != NULL)
        {
            g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].pszName = malloc(MAX_NAME_LENGTH);
            memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].pszName, 0, MAX_NAME_LENGTH);
            strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].pszName, LastNode->data, MAX_NAME_LENGTH-1);
        }
        else
        {
            ONVIF_ERR("Unable to find Name element in XML tree!\n");
            return -1;
        }

        LastNode = xml_node_get_child(RuleNode, "Type");
        if(LastNode != NULL && LastNode->data != NULL)
        {
            g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].pszType = malloc(MAX_NAME_LENGTH);
            memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].pszType, 0, MAX_NAME_LENGTH);
            strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].pszType, LastNode->data, MAX_NAME_LENGTH-1);
        }
        else
        {
            ONVIF_ERR("Unable to find Name element in XML tree!\n");
            return -1;
        }
		
        ItemListNode = xml_node_get_child(RuleNode, "ItemList");
        if(ItemListNode == NULL)
        {
            return -1;
        }
        SimpItemNode = xml_node_get_child(ItemListNode, "SimpItem");
        if(SimpItemNode == NULL)
        {
            return -1;
        }
        pszNum = xml_attr_get_data(SimpItemNode, "Num");
        if(pszNum)
        {
            sscanf(xml_attr_get_data(SimpItemNode, "Num"), "%d", &SimpItemNum);
        }
        else
        {
            ONVIF_ERR("request config num failed or xml file content error!\n");
            return -1;
        }

        g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.sizeSimpleItem = SimpItemNum;
        g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem = malloc(sizeof(GONVIF_MEDIA_ItemList_SimpleItem_S)*SimpItemNum);
        memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem,0,sizeof(GONVIF_MEDIA_ItemList_SimpleItem_S)*SimpItemNum);
        GK_S8 ItemBuf[32] = {0};
        memset(ItemBuf, 0, sizeof(ItemBuf));
        GK_S32 j = 0;
        for(j = 0; j < SimpItemNum; j++)
        {
            snprintf(ItemBuf, sizeof(ItemBuf), "Item%d", j);
            ItemNode = xml_node_get_child(SimpItemNode, ItemBuf);
            if(ItemNode == NULL)
            {
                return -1;
            }
            LastNode = xml_node_get_child(ItemNode, "Name");
            if(LastNode != NULL && LastNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem[j].pszName = malloc(MAX_NAME_LENGTH);
                memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem[j].pszName, 0, MAX_NAME_LENGTH);
                strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem[j].pszName, LastNode->data, MAX_NAME_LENGTH-1);
            }
            else
            {
                ONVIF_ERR("Unable to find Name element in XML tree!\n");
                return -1;
            }
            LastNode = xml_node_get_child(ItemNode, "Value");
            if(LastNode != NULL && LastNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem[j].pszValue= malloc(MAX_NAME_LENGTH);
                memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem[j].pszValue, 0, MAX_NAME_LENGTH);
                strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstSimpleItem[j].pszValue, LastNode->data, MAX_NAME_LENGTH-1);
            }
            else
            {
                ONVIF_ERR("Unable to find Name element in XML tree!\n");
                return -1;
            }
        }

        ElementItemNode = xml_node_get_child(ItemListNode, "ElementItem");
        pszNum = xml_attr_get_data(ElementItemNode, "Num");
        if(pszNum)
        {
            sscanf(xml_attr_get_data(ElementItemNode, "Num"), "%d", &ElementItemNum);
        }
        else
        {
            ONVIF_ERR("request config num failed or xml file content error!\n");
            return -1;
        }

        g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.sizeElementItem = ElementItemNum;
        g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstElementItem = malloc(sizeof(GONVIF_MEDIA_ItemList_ElementItem_S)*ElementItemNum);
        memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstElementItem,0,sizeof(GONVIF_MEDIA_ItemList_ElementItem_S)*ElementItemNum);
        memset(ItemBuf, 0, sizeof(ItemBuf));
        for(j = 0; j < ElementItemNum; j++)
        {
            snprintf(ItemBuf, sizeof(ItemBuf), "Item%d", j);
            ItemNode = xml_node_get_child(ElementItemNode, ItemBuf);
            if(ItemNode == NULL)
            {
                return -1;
            }

            LastNode = xml_node_get_child(ItemNode, "Name");
            if(LastNode != NULL && LastNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstElementItem[j].pszName = malloc(MAX_NAME_LENGTH);
                memset(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstElementItem[j].pszName, 0, MAX_NAME_LENGTH);
                strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[index].stRuleEngineConfiguration.pstRule[i].stParameters.pstElementItem[j].pszName, LastNode->data, MAX_NAME_LENGTH-1);
            }
            else
            {
                ONVIF_ERR("Unable to find Name element in XML tree!\n");
                return -1;
            }
        }
    }

	return 0;
}

static GK_S32 parseXmlPTZConfiguration(GK_CHAR *rootElement, GK_CHAR *curElement, GK_S32 index)
{
	XMLN *PTZConfigurationNode = NULL;
	XMLN *ConfigNode = NULL;
	XMLN *DefaultPTZSpeedNode = NULL;
	XMLN *PanTiltLimitsNode = NULL;
	XMLN *ZoomLimitsNode = NULL;
	XMLN *range = NULL;
	XMLN *ValueNode = NULL;
	const GK_S8 *str = NULL;

	PTZConfigurationNode = xml_node_get_child(XMLTree, rootElement);
	if(PTZConfigurationNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n",rootElement);
		return -1;
	}

	ConfigNode = xml_node_get_child(PTZConfigurationNode, curElement);
	if(ConfigNode == NULL)
	{
		ONVIF_ERR("Unable to find %s element in XML tree!\n", curElement);
		return -1;
	}
	
	str = xml_attr_get_data(ConfigNode, "Token");
	if(str != NULL)
	{
		strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].token, str, MAX_TOKEN_LENGTH-1);		
	}
	else
	{
		ONVIF_ERR("Unable to find token element in XML tree!\n");
		return -1;
	}

	str = xml_attr_get_data(ConfigNode, "Name");
	if(str != NULL)
	{
		strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].name, str, MAX_NAME_LENGTH-1);
	}
	else
	{
		ONVIF_ERR("Unable to find Name attr in XML tree!\n");
		return -1;
	}

	ValueNode = xml_node_get_child(ConfigNode, "UseCount");
	if(ValueNode == NULL || ValueNode->data == NULL)
	{
		ONVIF_ERR("Unable to find UseCount element in XML tree!\n");
		return -1;
	}
	else
	{
		g_GkIpc_OnvifInf.gokePTZConfiguration[index].useCount = atoi(ValueNode->data);
	}

	ValueNode = xml_node_get_child(ConfigNode, "NodeToken");
	if(ValueNode != NULL && ValueNode->data != NULL)
	{
		strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].nodeToken, ValueNode->data, MAX_TOKEN_LENGTH-1);
	}
	else
	{
		ONVIF_ERR("Unable to find NodeToken element in XML tree!\n");
		return -1;
	}

	ValueNode = xml_node_get_child(ConfigNode, "DefaultAbsolutePantTiltPositionSpace");
	if(ValueNode != NULL && ValueNode->data != NULL)
	{
		strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].defaultAbsolutePantTiltPositionSpace, ValueNode->data, MAX_URI_LENGTH-1);
	}

	ValueNode = xml_node_get_child(ConfigNode, "DefaultAbsoluteZoomPositionSpace");
	if(ValueNode != NULL && ValueNode->data != NULL)
	{
		strcpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].defaultAbsoluteZoomPositionSpace, ValueNode->data);
	}

	ValueNode = xml_node_get_child(ConfigNode, "DefaultRelativePanTiltTranslationSpace");
	if(ValueNode != NULL && ValueNode->data != NULL)
	{
		strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].defaultRelativePanTiltTranslationSpace, ValueNode->data, MAX_URI_LENGTH-1);
	}

	ValueNode = xml_node_get_child(ConfigNode, "DefaultRelativeZoomTranslationSpace");
	if(ValueNode != NULL && ValueNode->data!= NULL)
	{
		strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].defaultRelativeZoomTranslationSpace, ValueNode->data, MAX_URI_LENGTH-1);
	}

	ValueNode = xml_node_get_child(ConfigNode, "DefaultContinuousPanTiltVelocitySpace");
	if(ValueNode != NULL && ValueNode->data != NULL)
	{
		strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].defaultContinuousPanTiltVelocitySpace, ValueNode->data, MAX_URI_LENGTH-1);
	}

	ValueNode = xml_node_get_child(ConfigNode, "DefaultContinuousZoomVelocitySpace");
	if(ValueNode != NULL && ValueNode->data != NULL)
	{
		strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].defaultContinuousZoomVelocitySpace, ValueNode->data, MAX_URI_LENGTH-1);
	}

	DefaultPTZSpeedNode = xml_node_get_child(ConfigNode, "DefaultPTZSpeed");
	if(DefaultPTZSpeedNode != NULL)
	{
		ValueNode = xml_node_get_child(DefaultPTZSpeedNode, "PanTilt");
		if(ValueNode != NULL)
		{
			str = xml_attr_get_data(ValueNode, "space");
			if(str)
			{
				strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].stDefaultPTZSpeed.stPanTilt.aszSpace, str, MAX_SPACE_LENGTH-1);
			}

			str = xml_attr_get_data(ValueNode, "x");
			if(str)
			{
				sscanf(str, "%f", &g_GkIpc_OnvifInf.gokePTZConfiguration[index].stDefaultPTZSpeed.stPanTilt.x);
			}
			else
			{
				ONVIF_ERR("Unable to find x element in XML tree!\n");
				return -1;
			}

			str = xml_attr_get_data(ValueNode, "y");
			if(str)
			{
				sscanf(str, "%f", &g_GkIpc_OnvifInf.gokePTZConfiguration[index].stDefaultPTZSpeed.stPanTilt.y);
			}
			else
			{
				ONVIF_ERR("Unable to find y element in XML tree!\n");
				return -1;
			}
		}

		ValueNode = xml_node_get_child(DefaultPTZSpeedNode, "Zoom");
		if(ValueNode != NULL)
		{
			str = xml_attr_get_data(ValueNode, "space");
			if(str)
			{
				strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].stDefaultPTZSpeed.stZoom.aszSpace, str, MAX_SPACE_LENGTH-1);
			}

			str = xml_attr_get_data(ValueNode, "x");
			if(str)
			{
				sscanf(str, "%f", &g_GkIpc_OnvifInf.gokePTZConfiguration[index].stDefaultPTZSpeed.stZoom.x);
			}
			else
			{
				ONVIF_ERR("Unable to find x element in XML tree!\n");
				return -1;
			}
		}
	}

	ValueNode = xml_node_get_child(ConfigNode, "DefaultPTZTimeout");
	if(ValueNode != NULL && ValueNode->data != NULL)
	{
		g_GkIpc_OnvifInf.gokePTZConfiguration[index].defaultPTZTimeout = atoll(ValueNode->data);
	}
	else
	{
		ONVIF_ERR("Unable to find UseCount element in XML tree!\n");
		return -1;
	}
	
	PanTiltLimitsNode = xml_node_get_child(ConfigNode, "PanTiltLimits");
	if(PanTiltLimitsNode != NULL)
	{
		range = xml_node_get_child(PanTiltLimitsNode, "Range");
		if(range != NULL)
		{
            ValueNode = xml_node_get_child(range, "URI");
            if(ValueNode != NULL && ValueNode->data != NULL)
            {
                strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].stPanTiltLimits.stRange.aszURI, ValueNode->data, MAX_URI_LENGTH-1);
            }
            else
            {
                ONVIF_ERR("Unable to find URI element in XML tree!\n");
                return -1;
            }

            ValueNode = xml_node_get_child(range, "XRange");
            if(ValueNode != NULL)
            {
                str = xml_attr_get_data(ValueNode, "Min");
                if(str)
                {
                    sscanf(str, "%f", &g_GkIpc_OnvifInf.gokePTZConfiguration[index].stPanTiltLimits.stRange.stXRange.min);
                }
                else
                {
                    ONVIF_ERR("Unable to find Min element in XML tree!\n");
                    return -1;
                }
                str = xml_attr_get_data(ValueNode, "Max");
                if(str)
                {
                    sscanf(str, "%f", &g_GkIpc_OnvifInf.gokePTZConfiguration[index].stPanTiltLimits.stRange.stXRange.max);
                }
                else
                {
                    ONVIF_ERR("Unable to find Max element in XML tree!\n");
                    return -1;
                }
            }
            else
            {
                ONVIF_ERR("Unable to find XRange element in XML tree!\n");
                return -1;
            }

            ValueNode = xml_node_get_child(range, "YRange");
            if(ValueNode != NULL)
            {
                str = xml_attr_get_data(ValueNode, "Min");
                if(str)
                {
                    sscanf(str, "%f", &g_GkIpc_OnvifInf.gokePTZConfiguration[index].stPanTiltLimits.stRange.stYRange.min);
                }
                else
                {
                    ONVIF_ERR("Unable to find Min element in XML tree!\n");
                    return -1;
                }
                str = xml_attr_get_data(ValueNode, "Max");
                if(str)
                {
                    sscanf(str, "%f", &g_GkIpc_OnvifInf.gokePTZConfiguration[index].stPanTiltLimits.stRange.stYRange.max);
                }
                else
                {
                    ONVIF_ERR("Unable to find Max element in XML tree!\n");
                    return -1;
                }
            }
            else
            {
                ONVIF_ERR("Unable to find YRange element in XML tree!\n");
                return -1;
            }
		}
		else
		{
			ONVIF_ERR("Unable to find Range element in XML tree!\n");
			return -1;
		}
	}

	ZoomLimitsNode = xml_node_get_child(ConfigNode, "ZoomLimits");
	if(ZoomLimitsNode != NULL)
	{
		range = xml_node_get_child(ZoomLimitsNode, "Range");
		if(range != NULL)
		{
            ValueNode = xml_node_get_child(range, "URI");
            if(ValueNode != NULL && ValueNode->data != NULL)
            {
                strncpy(g_GkIpc_OnvifInf.gokePTZConfiguration[index].stZoomLimits.stRange.aszURI, ValueNode->data, MAX_URI_LENGTH-1);
            }
            else
            {
                ONVIF_ERR("Unable to find URI element in XML tree!\n");
                return -1;
            }

            ValueNode = xml_node_get_child(range, "XRange");
            if(ValueNode != NULL)
            {
                str = xml_attr_get_data(ValueNode, "Min");
                if(str)
                {
                    sscanf(str, "%f", &g_GkIpc_OnvifInf.gokePTZConfiguration[index].stZoomLimits.stRange.stXRange.min);
                }
                else
                {
                    ONVIF_ERR("Unable to find Min element in XML tree!\n");
                    return -1;
                }
                str = xml_attr_get_data(ValueNode, "Max");
                if(str)
                {
                    sscanf(str, "%f", &g_GkIpc_OnvifInf.gokePTZConfiguration[index].stZoomLimits.stRange.stXRange.max);
                }
                else
                {
                    ONVIF_ERR("Unable to find Max element in XML tree!\n");
                    return -1;
                }
            }
            else
            {
                ONVIF_ERR("Unable to find XRange element in XML tree!\n");
                return -1;
            }
		}
		else
		{
			ONVIF_ERR("Unable to find Range element in XML tree!\n");
			return -1;
		}
	}

	return 0;
}

static GK_S32 parseXmlImageOption(GK_CHAR *curElement)
{
    GK_S32 i;
    XMLN *ImagingOptionNode = NULL;
    XMLN *ValueNode = NULL;
    XMLN *SecondValueNode = NULL;

    ImagingOptionNode = xml_node_get_child(XMLTree, curElement);
    if(ImagingOptionNode == NULL)
    {
        ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
        return -1;
    }
    g_GkIpc_OnvifInf.gokeImagingOptions = (GONVIF_IMAGE_Options_S *)malloc(sizeof(GONVIF_IMAGE_Options_S));
    if(g_GkIpc_OnvifInf.gokeImagingOptions == NULL)
    {
        return -1;
    }
    memset(g_GkIpc_OnvifInf.gokeImagingOptions, 0, sizeof(GONVIF_IMAGE_Options_S));

    ValueNode = xml_node_get_child(ImagingOptionNode, "BacklightCompensation");
    if(ValueNode == NULL)
    {
        return -1;
    }

    SecondValueNode = xml_node_get_child(ValueNode, "sizeMode");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    if(SecondValueNode->data != NULL)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.sizeMode = atoi(SecondValueNode->data);
    }

    GK_S32 BCModeNum = g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.sizeMode;
    if(BCModeNum != 0)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.penMode = (GONVIF_IMAGE_BackLightCompensation_Mode_E *)malloc(sizeof(GONVIF_IMAGE_BackLightCompensation_Mode_E) * BCModeNum);
        GK_S8 ModeBuf[32] = {0};
        for(i = 0; i < BCModeNum; i++)
        {
            memset(ModeBuf, 0, sizeof(ModeBuf));
            snprintf(ModeBuf, sizeof(ModeBuf), "Mode%d", i);
            SecondValueNode = xml_node_get_child(ValueNode, ModeBuf);
            if(SecondValueNode == NULL)
            {
                return -1;
            }
            if(SecondValueNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.penMode[i] = atoi(SecondValueNode->data);
            }
        }
    }

    SecondValueNode = xml_node_get_child(ValueNode, "Level");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszLevelMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszLevelMin)
    {
        sscanf(pszLevelMin, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.min);
    }
    const GK_S8 *pszLevelMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszLevelMax)
    {
        sscanf(pszLevelMax, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.max);
    }

    ValueNode = xml_node_get_child(ImagingOptionNode, "Brightness");
    if(ValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszBRMin = xml_attr_get_data(ValueNode, "Min");
    if(pszBRMin)
    {
        sscanf(pszBRMin, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.min);
    }
    const GK_S8 *pszBRMax = xml_attr_get_data(ValueNode, "Max");
    if(pszBRMax)
    {
        sscanf(pszBRMax, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.max);
    }

    ValueNode = xml_node_get_child(ImagingOptionNode, "ColorSaturation");
    if(ValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszCSMin = xml_attr_get_data(ValueNode, "Min");
    if(pszCSMin)
    {
        sscanf(pszCSMin, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.min);
    }
    const GK_S8 *pszCSMax = xml_attr_get_data(ValueNode, "Max");
    if(pszCSMax)
    {
        sscanf(pszCSMax, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.max);
    }

    ValueNode = xml_node_get_child(ImagingOptionNode, "Contrast");
    if(ValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszCRMin = xml_attr_get_data(ValueNode, "Min");
    if(pszCRMin)
    {
        sscanf(pszCRMin, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.min);
    }
    const GK_S8 *pszCRMax = xml_attr_get_data(ValueNode, "Max");
    if(pszCRMax)
    {
        sscanf(pszCRMax, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.max);
    }

    ValueNode = xml_node_get_child(ImagingOptionNode, "Exposure");
    if(ValueNode == NULL)
    {
        return -1;
    }

    SecondValueNode = xml_node_get_child(ValueNode, "sizeMode");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    if(SecondValueNode->data != NULL)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.sizeMode = atoi(SecondValueNode->data);
    }

    GK_S32 ESModeNum = g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.sizeMode;
    if(ESModeNum != 0)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.penMode = (GONVIF_IMAGE_Exposure_Mode_E *)malloc(sizeof(GONVIF_IMAGE_Exposure_Mode_E)*ESModeNum);
        GK_S8 ModeBuf[32] = {0};
        for(i = 0; i < ESModeNum; i++)
        {
            memset(ModeBuf, 0, sizeof(ModeBuf));
            snprintf(ModeBuf, sizeof(ModeBuf), "Mode%d",i);
            SecondValueNode = xml_node_get_child(ValueNode, ModeBuf);
            if(SecondValueNode == NULL)
            {
                return -1;
            }
            if(SecondValueNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.penMode[i] = atoi(SecondValueNode->data);
            }
        }
    }

    SecondValueNode = xml_node_get_child(ValueNode, "MinExposureTime");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinETMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMinETMin)
    {
        sscanf(pszMinETMin, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.min);
    }
    const GK_S8 *pszMinETMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMinETMax)
    {
        sscanf(pszMinETMax, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.max);
    }

    SecondValueNode = xml_node_get_child(ValueNode, "MaxExposureTime");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMaxETMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMaxETMin)
    {
        sscanf(pszMaxETMin, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.min);
    }
    const GK_S8 *pszMaxETMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxETMax)
    {
        sscanf(pszMaxETMax, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.max);
    }

    SecondValueNode = xml_node_get_child(ValueNode, "MinGain");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinGainMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMinGainMin)
    {
        sscanf(pszMinGainMin, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.min);
    }
    const GK_S8 *pszMinGainMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMinGainMax)
    {
        sscanf(pszMinGainMax, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.max);
    }

    SecondValueNode = xml_node_get_child(ValueNode, "MaxGain");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMaxGainMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMaxGainMin)
    {
        sscanf(pszMaxGainMin, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.min);
    }
    const GK_S8 *pszMaxGainMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxGainMax)
    {
        sscanf(pszMaxGainMax, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.max);
    }
   
    SecondValueNode = xml_node_get_child(ValueNode, "ExposureTime");
    if(SecondValueNode == NULL){
        return -1;
    }
    const GK_S8 *pszMinET = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMinET)
    {
        sscanf(pszMinET, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.min);
    }
    const GK_S8 *pszMaxET = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxET)
    {
        sscanf(pszMaxET, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.max);
    }

    SecondValueNode = xml_node_get_child(ValueNode, "Gain");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinGain = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMinGain)
    {
        sscanf(pszMinGain, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.min);
    }
    const GK_S8 *pszMaxGain = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxGain)
    {
        sscanf(pszMaxGain, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.max);
    }
	
    ValueNode = xml_node_get_child(ImagingOptionNode, "Focus");
    if(ValueNode == NULL)
    {
        return -1;
    }

    SecondValueNode = xml_node_get_child(ValueNode, "sizeAutoFocusModes");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    if(SecondValueNode->data != NULL)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.sizeAutoFocusModes = atoi(SecondValueNode->data);
    }
    GK_S32 AFModeNum = g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.sizeAutoFocusModes;

    if(AFModeNum != 0)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.penAutoFocusModes = (GONVIF_IMAGE_Focus_Mode_E *)malloc(sizeof(GONVIF_IMAGE_Focus_Mode_E)*AFModeNum);
        GK_S8 ModeBuf[32] = {0};
        for(i = 0; i < AFModeNum; i++)
        {
            memset(ModeBuf, 0, sizeof(ModeBuf));
            snprintf(ModeBuf, sizeof(ModeBuf), "AutoFocusModes%d", i);
            SecondValueNode = xml_node_get_child(ValueNode, ModeBuf);
            if(SecondValueNode == NULL)
            {
                return -1;
            }
            if(SecondValueNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.penAutoFocusModes[i] = atoi(SecondValueNode->data);
            }
        }
    }

    SecondValueNode = xml_node_get_child(ValueNode, "DefaultSpeed");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinDS = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMinDS)
    {
        sscanf(pszMinDS, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.min);
    }
    const GK_S8 *pszMaxDS = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxDS)
    {
        sscanf(pszMaxDS, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.max);
    }

    SecondValueNode = xml_node_get_child(ValueNode, "NearLimit");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinNL = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMinNL)
    {
        sscanf(pszMinNL, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.min);
    }
    const GK_S8 *pszMaxNL = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxNL)
    {
        sscanf(pszMaxNL, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.max);
    }
	
    SecondValueNode = xml_node_get_child(ValueNode, "FarLimit");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinFL = xml_attr_get_data(SecondValueNode, "min");
    if(pszMinFL)
    {
        sscanf(pszMinFL, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.min);
    }
    const GK_S8 *pszMaxFL = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxFL)
    {
        sscanf(pszMaxFL, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.max);
    }

    ValueNode = xml_node_get_child(ImagingOptionNode, "sizeIrCutFilterModes");
    if(ValueNode == NULL){
        return -1;
    }
    if(ValueNode->data != NULL){
        g_GkIpc_OnvifInf.gokeImagingOptions->sizeIrCutFilterModes = atoi(ValueNode->data);
    }
    GK_S32 FilterModeNum = g_GkIpc_OnvifInf.gokeImagingOptions->sizeIrCutFilterModes;

    if(FilterModeNum != 0)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes = (GONVIF_IMAGE_IrCutFilter_Mode_E *)malloc(sizeof(GONVIF_IMAGE_IrCutFilter_Mode_E)*FilterModeNum);
        GK_S8 ModeBuf[32] = {0};
        for(i = 0; i < FilterModeNum; i++)
        {
            memset(ModeBuf, 0, sizeof(ModeBuf));
            snprintf(ModeBuf, sizeof(ModeBuf), "IrCutFilterModes%d", i);
            ValueNode = xml_node_get_child(ImagingOptionNode, ModeBuf);
            if(ValueNode == NULL)
            {
                return -1;
            }
            if(ValueNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[i] = atoi(ValueNode->data);
            }
        }
    }

    ValueNode = xml_node_get_child(ImagingOptionNode, "Sharpness");
    if(ValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinSP = xml_attr_get_data(ValueNode, "Min");
    if(pszMinSP)
    {
        sscanf(pszMinSP, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.min);
    }
    const GK_S8 *pszMaxSP = xml_attr_get_data(ValueNode, "Max");
    if(pszMaxSP)
    {
        sscanf(pszMaxSP, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.max);
    }

    ValueNode = xml_node_get_child(ImagingOptionNode, "WideDynamicRange");
    if(ValueNode == NULL)
    {
        return -1;
    }

    SecondValueNode = xml_node_get_child(ValueNode, "sizeMode");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    if(SecondValueNode->data != NULL)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.sizeMode = atoi(SecondValueNode->data);
    }
    GK_S32 WDModeNum = g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.sizeMode;
    if(WDModeNum != 0)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.penMode = (GONVIF_IMAGE_WideDynamicRange_Mode_E *)malloc(sizeof(GONVIF_IMAGE_WideDynamicRange_Mode_E)*WDModeNum);
        GK_S8 ModeBuf[32] = {0};
        for(i = 0; i < WDModeNum; i++)
        {
            memset(ModeBuf, 0, sizeof(ModeBuf));
            snprintf(ModeBuf, sizeof(ModeBuf), "Mode%d", i);
            SecondValueNode = xml_node_get_child(ValueNode, ModeBuf);
            if(SecondValueNode == NULL)
            {
                return -1;
            }
            if(SecondValueNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.penMode[i] = atoi(SecondValueNode->data);
            }
        }
    }
    SecondValueNode = xml_node_get_child(ValueNode, "Level");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinWD = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMinWD)
    {
        sscanf(pszMinWD, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.min);
    }
    const GK_S8 *pszMaxWD = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxWD)
    {
        sscanf(pszMaxWD, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.max);
    }

    ValueNode = xml_node_get_child(ImagingOptionNode, "WhiteBalance");
    if(ValueNode == NULL)
    {
        return -1;
    }

    SecondValueNode = xml_node_get_child(ValueNode, "sizeMode");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    if(SecondValueNode->data != NULL){
        g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.sizeMode = atoi(SecondValueNode->data);
    }
    GK_S32 WBModeNum = g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.sizeMode;
    if(WBModeNum != 0)
    {
        g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.penMode = (GONVIF_IMAGE_WhiteBalance_Mode_E *)malloc(sizeof(GONVIF_IMAGE_WhiteBalance_Mode_E)*WBModeNum);
        GK_S8 ModeBuf[32] = {0};
        for(i = 0; i < WBModeNum; i++)
        {
            memset(ModeBuf, 0, sizeof(ModeBuf));
            snprintf(ModeBuf,sizeof(ModeBuf), "Mode%d",i);
            SecondValueNode = xml_node_get_child(ValueNode, ModeBuf);
            if(SecondValueNode == NULL)
            {
                return -1;
            }
            if(SecondValueNode->data != NULL)
            {
                g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.penMode[i] = atoi(SecondValueNode->data);
            }
        }
    }
    SecondValueNode = xml_node_get_child(ValueNode, "YrGain");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinYr = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMinYr)
    {
        sscanf(pszMinYr, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.min);
    }
    const GK_S8 *pszMaxYr = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxYr)
    {
        sscanf(pszMaxYr, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.max);
    }
    SecondValueNode = xml_node_get_child(ValueNode, "YbGain");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszMinYb = xml_attr_get_data(SecondValueNode, "Min");
    if(pszMinYb)
    {
        sscanf(pszMinYb, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.min);
    }
    const GK_S8 *pszMaxYb = xml_attr_get_data(SecondValueNode, "Max");
    if(pszMaxYb)
    {
        sscanf(pszMaxYb, "%f", &g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.max);
    }

    return 0;
}

static GK_S32 parseXmlImageMoveOption(GK_CHAR *curElement)
{
    XMLN *ImagingMoveOptionNode;
    XMLN *ValueNode;
    XMLN *SecondValueNode;

    ImagingMoveOptionNode = xml_node_get_child(XMLTree, curElement);
    if(ImagingMoveOptionNode == NULL)
    {
        ONVIF_ERR("Unable to find %s element in XML tree!\n",curElement);
        return -1;
    }
    g_GkIpc_OnvifInf.gokeMoveOptions = (GONVIF_IMAGE_MoveOptions_S *)malloc(sizeof(GONVIF_IMAGE_MoveOptions_S));
    if(g_GkIpc_OnvifInf.gokeMoveOptions == NULL)
    {
        return -1;
    }
    memset(g_GkIpc_OnvifInf.gokeMoveOptions, 0, sizeof(GONVIF_IMAGE_MoveOptions_S));

    ValueNode = xml_node_get_child(ImagingMoveOptionNode, "Absolute");
    if(ValueNode == NULL)
    {
        return -1;
    }
	
    SecondValueNode = xml_node_get_child(ValueNode, "Position");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszABPMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszABPMin)
    {
        sscanf(pszABPMin, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stPosition.min);
    }
    const GK_S8 *pszABPMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszABPMax)
    {
        sscanf(pszABPMax, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stPosition.max);
    }

    SecondValueNode = xml_node_get_child(ValueNode, "Speed");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszABSMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszABSMin)
    {
        sscanf(pszABSMin, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stSpeed.min);
    }
    const GK_S8 *pszABSMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszABSMax)
    {
        sscanf(pszABSMax, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stSpeed.max);
    }

    ValueNode = xml_node_get_child(ImagingMoveOptionNode, "Relative");
    if(ValueNode == NULL)
    {
        return -1;
    }

    SecondValueNode = xml_node_get_child(ValueNode, "Distance");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszRTDMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszRTDMin)
    {
        sscanf(pszRTDMin, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stDistance.min);
    }
    const GK_S8 *pszRTDMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszRTDMax)
    {
        sscanf(pszRTDMax, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stDistance.max);
    }

    SecondValueNode = xml_node_get_child(ValueNode, "Speed");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszRTSMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszRTSMin)
    {
        sscanf(pszRTSMin, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stSpeed.min);
    }
    const GK_S8 *pszRTSMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszRTSMax)
    {
        sscanf(pszRTSMax, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stSpeed.max);
    }
	
    ValueNode = xml_node_get_child(ImagingMoveOptionNode, "Continuous");
    if(ValueNode == NULL)
    {
        return -1;
    }

    SecondValueNode = xml_node_get_child(ValueNode, "Speed");
    if(SecondValueNode == NULL)
    {
        return -1;
    }
    const GK_S8 *pszCTSMin = xml_attr_get_data(SecondValueNode, "Min");
    if(pszCTSMin)
    {
        sscanf(pszCTSMin, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stContinuous.stSpeed.min);
    }
    const GK_S8 *pszCTSMax = xml_attr_get_data(SecondValueNode, "Max");
    if(pszCTSMax)
    {
        sscanf(pszCTSMax, "%f", &g_GkIpc_OnvifInf.gokeMoveOptions->stContinuous.stSpeed.max);
    }

    return 0;
}

static GK_S32 save_meida_VideoSourceXml(XMLN *xmlMedia)
{
	GK_S32 i;
    XMLN *node;
    XMLN *node_videosource;
    XMLN *node_config;

	node_videosource = xml_node_add_new(xmlMedia, "VideoSource");

    GK_CHAR xmlBuf[32];
    memset(xmlBuf, 0, sizeof(xmlBuf));
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.VideoSourceNum);
    xml_attr_add(node_videosource, "Num", xmlBuf);
	for(i = 0; i < g_GkIpc_OnvifInf.VideoSourceNum; i++)
	{
        memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "Configuration%d", i);

        node_config = xml_node_add_new(node_videosource, xmlBuf);
    	xml_attr_add(node_config, "Token", g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszToken);
    	xml_attr_add(node_config, "Name", g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszName);

        node = xml_node_add_new(node_config, "UseCount");
        memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].useCount);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_config, "SourceToken");
        xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszSourceToken);

		node = xml_node_add_new(node_config, "Bounds");
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.height);
    	xml_attr_add(node, "height", xmlBuf);
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.width);
    	xml_attr_add(node, "width", xmlBuf);
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.y);
    	xml_attr_add(node, "y", xmlBuf);
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.x);
    	xml_attr_add(node, "x", xmlBuf);
      	xml_node_set_data(node, "");

	}
	return 0;
}

static GK_S32 save_meida_AudioSourceXml(XMLN *xmlMedia)
{
	GK_S32 i;
    XMLN *node;
    XMLN *node_audiosource;
    XMLN *node_config;

	node_audiosource = xml_node_add_new(xmlMedia, "AudioSource");
    GK_S8 xmlBuf[32];
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.AudioSourceNum);
    xml_attr_add(node_audiosource, "Num", xmlBuf);

	for(i = 0; i < g_GkIpc_OnvifInf.AudioSourceNum; i++)
	{
    	snprintf(xmlBuf, sizeof(xmlBuf), "Configuration%d", i);
        node_config = xml_node_add_new(node_audiosource, xmlBuf);
    	xml_attr_add(node_config, "Token", g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszToken);
    	xml_attr_add(node_config, "Name", g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszName);

        node = xml_node_add_new(node_config, "UseCount");
    	memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].useCount);
        xml_node_set_data(node, xmlBuf);


		node = xml_node_add_new(node_config, "SourceToken");
        xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszSourceToken);
	}
	return 0;
}

static GK_S32 save_meida_VideoEncoderXml(XMLN *xmlMedia)
{
	GK_S32 i;
    XMLN *node;
    XMLN *node_multi;
    XMLN *node_addr;
    XMLN *node_videoencoder;
    XMLN *node_config;

	node_videoencoder = xml_node_add_new(xmlMedia, "VideoEncoder");
    GK_S8 xmlBuf[32];
    memset(xmlBuf, 0, sizeof(xmlBuf));
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.VideoEncodeNum);
    xml_attr_add(node_videoencoder, "Num", xmlBuf);

	for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
	{
        memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "Configuration%d", i);
        node_config = xml_node_add_new(node_videoencoder, xmlBuf);
    	xml_attr_add(node_config, "Token", g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].pszToken);
    	xml_attr_add(node_config, "Name", g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].pszName);

        node = xml_node_add_new(node_config, "UseCount");
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].useCount);
        xml_node_set_data(node, xmlBuf);

		node_multi = xml_node_add_new(node_config, "Multicast");

		node_addr = xml_node_add_new(node_multi, "Address");
		node = xml_node_add_new(node_addr, "Type");
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.stIPAddress.enType);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_addr, "IPv4Address");
		if(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.stIPAddress.pszIPv4Address != NULL)
		{
        	xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.stIPAddress.pszIPv4Address);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

		node = xml_node_add_new(node_addr, "IPv6Address");
		if(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.stIPAddress.pszIPv6Address!= NULL)
		{
        	xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.stIPAddress.pszIPv6Address);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

		node = xml_node_add_new(node_multi, "Port");
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.port);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_multi, "TTL");
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.ttl);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_multi, "AutoStart");
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.enAutoStart);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_config, "SessionTimeout");
		snprintf(xmlBuf, sizeof(xmlBuf), "%lld", g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].sessionTimeout);
        xml_node_set_data(node, xmlBuf);
	}

	return 0;
}

static GK_S32 save_meida_AudioEncoderXml(XMLN *xmlMedia)
{
	GK_S32 i;
    XMLN *node;
    XMLN *node_multi;
    XMLN *node_addr;
    XMLN *node_audioencoder;
    XMLN *node_config;

	node_audioencoder = xml_node_add_new(xmlMedia, "AudioEncoder");
    GK_S8 xmlBuf[32];
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.AudioEncodeNum);
    xml_attr_add(node_audioencoder, "Num", xmlBuf);

	for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
	{
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "Configuration%d", i);
        node_config = xml_node_add_new(node_audioencoder, xmlBuf);
    	xml_attr_add(node_config, "Token", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].pszToken);
    	xml_attr_add(node_config, "Name", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].pszName);

        node = xml_node_add_new(node_config, "UseCount");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].useCount);
        xml_node_set_data(node, xmlBuf);
#if 0
        node = xml_node_add_new(node_config, "Encoding");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].enEncoding);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_config, "Bitrate");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].bitRate);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_config, "SampleRate");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].sampleRate);
        xml_node_set_data(node, xmlBuf);
#endif
		node_multi = xml_node_add_new(node_config, "Multicast");

		node_addr = xml_node_add_new(node_multi, "Address");

		node = xml_node_add_new(node_addr, "Type");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.stIPAddress.enType);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_addr, "IPv4Address");
		if(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.stIPAddress.pszIPv4Address != NULL)
		{
        	xml_node_set_data(node,g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.stIPAddress.pszIPv4Address);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

		node = xml_node_add_new(node_addr, "IPv6Address");
		if(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.stIPAddress.pszIPv6Address != NULL)
		{
        	xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.stIPAddress.pszIPv6Address);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

		node = xml_node_add_new(node_multi, "Port");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.port);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_multi, "TTL");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.ttl);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_multi, "AutoStart");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.enAutoStart);
        xml_node_set_data(node, xmlBuf);

		node = xml_node_add_new(node_config, "SessionTimeout");
		snprintf(xmlBuf, sizeof(xmlBuf), "%lld", g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].sessionTimeout);
        xml_node_set_data(node, xmlBuf);
	}

	return 0;
}

static GK_S32 save_meida_PTZXml(XMLN *xmlMedia)
{
	GK_S32 i;
	
    XMLN *node_PTZ;    
    XMLN *node_config;
    XMLN *node_speed;
    XMLN *node_PanTiltLimits;
    XMLN *node_ZoomLimits;
    XMLN *node_Range;
    XMLN *node;

	node_PTZ = xml_node_add_new(xmlMedia, "PTZ");
    GK_S8 xmlBuf[100];
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.PTZConfigurationNum);
    xml_attr_add(node_PTZ, "Num", xmlBuf);

	for(i = 0; i < g_GkIpc_OnvifInf.PTZConfigurationNum; i++)
	{
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "Configuration%d", i);
        node_config = xml_node_add_new(node_PTZ, xmlBuf);
        
        xml_attr_add(node_config, "Token", g_GkIpc_OnvifInf.gokePTZConfiguration[i].token);
        xml_attr_add(node_config, "Name", g_GkIpc_OnvifInf.gokePTZConfiguration[i].name);
        
        node = xml_node_add_new(node_config, "UseCount");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokePTZConfiguration[i].useCount);
        xml_node_set_data(node, xmlBuf);
        
		node = xml_node_add_new(node_config, "NodeToken");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].nodeToken);
        xml_node_set_data(node, xmlBuf);
       
		node = xml_node_add_new(node_config, "DefaultAbsolutePantTiltPositionSpace");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultAbsolutePantTiltPositionSpace);
        xml_node_set_data(node, xmlBuf);
       
		node = xml_node_add_new(node_config, "DefaultAbsoluteZoomPositionSpace");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultAbsoluteZoomPositionSpace);
        xml_node_set_data(node, xmlBuf);
       
		node = xml_node_add_new(node_config, "DefaultRelativePanTiltTranslationSpace");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultRelativePanTiltTranslationSpace);
        xml_node_set_data(node, xmlBuf);
       
		node = xml_node_add_new(node_config, "DefaultRelativeZoomTranslationSpace");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultRelativeZoomTranslationSpace);
        xml_node_set_data(node, xmlBuf);
       
		node = xml_node_add_new(node_config, "DefaultContinuousPanTiltVelocitySpace");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultContinuousPanTiltVelocitySpace);
        xml_node_set_data(node, xmlBuf);
       
		node = xml_node_add_new(node_config, "DefaultContinuousZoomVelocitySpace");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultContinuousZoomVelocitySpace);
        xml_node_set_data(node, xmlBuf);
      
		node_speed = xml_node_add_new(node_config, "DefaultPTZSpeed");
		node = xml_node_add_new(node_speed, "PanTilt");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stPanTilt.aszSpace);
        xml_attr_add(node, "space", xmlBuf);
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%f", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stPanTilt.x);
        xml_attr_add(node, "x", xmlBuf);
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%f", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stPanTilt.y);
        xml_attr_add(node, "y", xmlBuf);
        xml_node_set_data(node, "");
		node = xml_node_add_new(node_speed, "Zoom");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stZoom.aszSpace);
        xml_attr_add(node, "space", xmlBuf);
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%f", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stZoom.x);
        xml_attr_add(node, "x", xmlBuf);
        xml_node_set_data(node, "");
       
		node = xml_node_add_new(node_config, "DefaultPTZTimeout");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%lld", g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultPTZTimeout);
        xml_node_set_data(node, xmlBuf);
       
		node_PanTiltLimits = xml_node_add_new(node_config, "PanTiltLimits");
		node_Range = xml_node_add_new(node_PanTiltLimits, "Range");
		node = xml_node_add_new(node_Range, "URI");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.aszURI);
        xml_node_set_data(node, xmlBuf);
		node = xml_node_add_new(node_Range, "XRange");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%f", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.stXRange.min);
        xml_attr_add(node, "Min", xmlBuf);
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%f", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.stXRange.max);
        xml_attr_add(node, "Max", xmlBuf);
        xml_node_set_data(node, "");
		node = xml_node_add_new(node_Range, "YRange");
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "%f", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.stYRange.min);
        xml_attr_add(node, "Min", xmlBuf);
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%f", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.stYRange.max);
        xml_attr_add(node, "Max", xmlBuf);
        xml_node_set_data(node, "");
        
        node_ZoomLimits = xml_node_add_new(node_config, "ZoomLimits");
        node_Range = xml_node_add_new(node_ZoomLimits, "Range");
        node = xml_node_add_new(node_Range, "URI");
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%s", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stZoomLimits.stRange.aszURI);
        xml_node_set_data(node, xmlBuf);
        node = xml_node_add_new(node_Range, "XRange");
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%f", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stZoomLimits.stRange.stXRange.min);
        xml_attr_add(node, "Min", xmlBuf);
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%f", g_GkIpc_OnvifInf.gokePTZConfiguration[i].stZoomLimits.stRange.stXRange.max);
        xml_attr_add(node, "Max", xmlBuf);
        xml_node_set_data(node, "");
	}

	return 0;
}

static GK_S32 save_media_RelayOutputXml(XMLN *xmlMedia)
{
    GK_S32 i;
    XMLN *node;
    XMLN *node_relayoutput;
    XMLN *node_config;

    node_relayoutput = xml_node_add_new(xmlMedia, "RelayOutput");

    GK_CHAR xmlBuf[32];
    memset(xmlBuf, 0, sizeof(xmlBuf));
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.relayOutputsNum);
    xml_attr_add(node_relayoutput, "Num", xmlBuf);
    for(i = 0; i < g_GkIpc_OnvifInf.relayOutputsNum; i++)
    {
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "Configuration%d", i);

        node_config = xml_node_add_new(node_relayoutput, xmlBuf);
        xml_attr_add(node_config, "Token", g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].aszToken);

        node = xml_node_add_new(node_config, "Mode");
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.enMode);
        xml_node_set_data(node, xmlBuf);

        node = xml_node_add_new(node_config, "DelayTime");
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%lld", g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.delayTime);
        xml_node_set_data(node, xmlBuf);

        node = xml_node_add_new(node_config, "IdleState");
        memset(xmlBuf, 0, sizeof(xmlBuf));
        snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.enIdleState);
        xml_node_set_data(node, xmlBuf);
    }
    return 0;
}

static GK_S32 save_meida_ProfileXml(XMLN *xmlMedia)
{
	GK_S32 i;
    XMLN *node;
    XMLN *node_profile;
    XMLN *node_config;

	node_profile = xml_node_add_new(xmlMedia, "Profile");
    GK_S8 xmlBuf[32];
    snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.ProfileNum);
    xml_attr_add(node_profile, "Num", xmlBuf);

	for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
	{
	    memset(xmlBuf, 0, sizeof(xmlBuf));
    	snprintf(xmlBuf, sizeof(xmlBuf), "Configuration%d", i);
        node_config = xml_node_add_new(node_profile, xmlBuf);
    	snprintf(xmlBuf, sizeof(xmlBuf), "%d", g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].enFixed);
    	xml_attr_add(node_config, "Fixed", xmlBuf);
    	xml_attr_add(node_config, "Token", g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken);
    	xml_attr_add(node_config, "Name", g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName);

		node = xml_node_add_new(node_config, "VideoSourceConfiguration");
		if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration != NULL)
		{
        	xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration->pszToken);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

		node = xml_node_add_new(node_config, "AudioSourceConfiguration");
		if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration != NULL)
		{
        	xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration->pszToken);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

		node = xml_node_add_new(node_config, "VideoEncoderConfiguration");
		if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration != NULL)
		{
        	xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration->pszToken);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

		node = xml_node_add_new(node_config, "AudioEncoderConfiguration");
		if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration != NULL)
		{
        	xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration->pszToken);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

		node = xml_node_add_new(node_config, "VideoAnalyticConfiguration");
		if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoAnalyticsConfiguration!= NULL)
		{
        	xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoAnalyticsConfiguration->pszToken);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

		node = xml_node_add_new(node_config, "PTZConfiguration");
		if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
		{
        	xml_node_set_data(node, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->token);
		}
		else
		{
        	xml_node_set_data(node, "");
		}

	}

	return 0;
}

