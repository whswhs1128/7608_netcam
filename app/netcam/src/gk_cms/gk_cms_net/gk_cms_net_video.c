#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"

PAYLOAD_NAME m_payloadname[] = {
	{ PT_PCMU,"PCMU" },
	{ PT_1016,"1016" },
	{ PT_G721,"G721" },
	{ PT_GSM,"GSM" },
	{ PT_G723,"G723" },
	{ PT_DVI4_8K,"DVI4_8K" },
	{ PT_DVI4_16K,"DVI4_16K" },
	{ PT_LPC,"LPC" },
	{ PT_PCMA,"PCMA" },
	{ PT_G722,"G722" },
	{ PT_S16BE_STEREO,"S16BE_STEREO" },
	{ PT_S16BE_MONO,"S16BE_MONO" },
	{ PT_QCELP,"QCELP" },
	{ PT_CN,"CN" },
	{ PT_MPEGAUDIO,"MPEGAUDIO" },
	{ PT_G728,"G728" },
	{ PT_DVI4_3,"DVI4_3" },
	{ PT_DVI4_4,"DVI4_4" },
	{ PT_G729,"G729" },
	{ PT_G711A,"G711A" },
	{ PT_G711U,"G711U" },
	{ PT_G726,"G726" },
	{ PT_G729A,"G729A" },
	{ PT_LPCM,"LPCM" },
	{ PT_CelB,"CelB" },
	{ PT_JPEG,"JPEG" },
	{ PT_CUSM,"CUSM" },
	{ PT_NV,"NV" },
	{ PT_PICW,"PICW" },
	{ PT_CPV,"CPV" },
	{ PT_H261,"H261" },
	{ PT_MPEGVIDEO,"MPEGVIDEO" },
	{ PT_MPEG2TS,"MPEG2TS" },
	{ PT_H263,"H263" },
	{ PT_SPEG,"SPEG" },
	{ PT_MPEG2VIDEO,"MPEG2VIDEO" },
	{ PT_AAC,"AAC" },
	{ PT_WMA9STD,"WMA9STD" },
	{ PT_HEAAC,"HEAAC" },
	{ PT_PCM_VOICE,"PCM_VOICE" },
	{ PT_PCM_AUDIO,"PCM_AUDIO" },
	{ PT_MP3,"MP3" },
	{ PT_ADPCMA,"ADPCMA" },
	{ PT_AEC,"AEC" },
	{ PT_X_LD,"X_LD" },
	{ PT_H264,"H264" },
	{ PT_D_GSM_HR,"D_GSM_HR" },
	{ PT_D_GSM_EFR,"D_GSM_EFR" },
	{ PT_D_L8,"D_L8" },
	{ PT_D_RED,"D_RED" },
	{ PT_D_VDVI,"D_VDVI" },
	{ PT_D_BT656,"D_BT656" },
	{ PT_D_H263_1998,"D_H263_1998" },
	{ PT_D_MP1S,"D_MP1S" },
	{ PT_D_MP2P,"D_MP2P" },
	{ PT_D_BMPEG,"D_BMPEG" },
	{ PT_MP4VIDEO,"MP4VIDEO" },
	{ PT_MP4AUDIO,"MP4AUDIO" },
	{ PT_VC1,"VC1" },
	{ PT_JVC_ASF,"JVC_ASF" },
	{ PT_D_AVI,"D_AVI" },
	{ PT_MAX,"MAX" },
	{ PT_AMR,"AMR" },
	{ PT_MJPEG,"MJPEG" },
	{ PT_H264_HIGHPROFILE,"H264_HIGHPROFILE" },
	{ PT_H264_MAINPROFILE,"H264_MAINPROFILE" },
	{ PT_H264_BASELINE,"H264_BASELINE" },
	{ PT_H265, "H265"},
	{ -1," " },
};

DMS_VIDEO_FORMAT m_videoformat[] = {
	{DMS_VIDEO_FORMAT_CIF,352,288,DMS_PAL,"CIF"	},
	{DMS_VIDEO_FORMAT_CIF,352,240,DMS_NTSC,"CIF"	},
	{DMS_VIDEO_FORMAT_D1,704,576,DMS_PAL,"D1"	},
	{DMS_VIDEO_FORMAT_D1,704,480,DMS_NTSC,"D1"	},
	{DMS_VIDEO_FORMAT_D1,720,576,DMS_PAL,"D1"	},
	{DMS_VIDEO_FORMAT_D1,720,480,DMS_NTSC,"D1"	},
	{DMS_VIDEO_FORMAT_HD1,704,288,DMS_PAL,"HD1"	},
	{DMS_VIDEO_FORMAT_HD1,704,240,DMS_NTSC,"HD1"	},
	{DMS_VIDEO_FORMAT_HD1,720,288,DMS_PAL,"HD1"	},
	{DMS_VIDEO_FORMAT_HD1,720,240,DMS_NTSC,"HD1"	},
	{DMS_VIDEO_FORMAT_QCIF,176,144,DMS_PAL,"QCIF"	},
	{DMS_VIDEO_FORMAT_QCIF,176,120,DMS_NTSC,"QCIF"	},

	{DMS_VIDEO_FORMAT_VGA,640,480,-1,"VGA"		},
	{DMS_VIDEO_FORMAT_QVGA,320,240,-1,"QVGA"	},
	{DMS_VIDEO_FORMAT_720P,1280,720,-1,"720P"	},
	{DMS_VIDEO_FORMAT_130H,1280,1024,-1,"130H"	},
	{DMS_VIDEO_FORMAT_300H,2048,1536,-1,"300H"	},
	{DMS_VIDEO_FORMAT_QQVGA,160,112,-1,"QQVGA"	},
	{DMS_VIDEO_FORMAT_UXGA,1600,1200,-1,"UXGA"	},
	{DMS_VIDEO_FORMAT_SVGA,800,600,-1,"SVGA"	},
    {DMS_VIDEO_FORMAT_1080P,1920,1080,-1,"1080P"},
	{DMS_VIDEO_FORMAT_XGA,1024,768,-1,"XGA"},
	{DMS_VIDEO_FORMAT_960P,1280,960,-1,"960P"},
	{DMS_VIDEO_FORMAT_640,640,360,-1,"360P"},

	{DMS_VIDEO_FORMAT_5M,2592,1944,-1,"5M"},
	{DMS_VIDEO_FORMAT_4M,2592, 1520, -1, "4M"},
	{DMS_VIDEO_FORMAT_2048x1520,2048,1520, -1, "300H"},
	{-1, -1, -1, -1, NULL}
};

#define CMS_AUDIO_MAX_ITEM_MAP		10
#define CMS_VIDEO_MAX_STREAM_MAP	4
#define CMS_VIDEO_MAX_ITEM_MAP		30

GK_CFG_MAP video2Arr[CMS_VIDEO_MAX_STREAM_MAP][CMS_VIDEO_MAX_ITEM_MAP];
GK_CFG_MAP audioArr[CMS_AUDIO_MAX_ITEM_MAP];

int cms_audio_get_def_map(void)
{
    #if 0
	int ret = 0;
	memset(audioArr, 0, CMS_AUDIO_MAX_ITEM_MAP*sizeof(GK_CFG_MAP));
   	ret = netcam_audio_get_map(audioArr, CMS_AUDIO_MAX_ITEM_MAP);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get audio map.\n");
		return -1;
	}
    #endif
	return 0;
}

int cms_video_get_def_map(void)
{
	int ret = 0;
	memset(video2Arr, 0, CMS_VIDEO_MAX_STREAM_MAP*CMS_VIDEO_MAX_ITEM_MAP*sizeof(GK_CFG_MAP));
   	ret = netcam_video_get_map(video2Arr, CMS_VIDEO_MAX_STREAM_MAP, CMS_VIDEO_MAX_ITEM_MAP);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get video map.\n");
		return -1;
	}

	return 0;
}

GK_CFG_MAP *cms_get_element_from_map(GK_CFG_MAP *mapArr, char *strName)
{
	int i = 0;
	int sizeEle = 0;
	if(mapArr == NULL || strName == NULL)
	{
		PRINT_ERR("CMS: invalid parameters.");
		return NULL;
	}
	while(mapArr[i].stringName != NULL && i < CMS_VIDEO_MAX_ITEM_MAP)
	{
		sizeEle++;
		i++;
	}
	for(i = 0; i < sizeEle; i++)
	{
		//printf("mapArr[%d].stringName:%s\n", i, mapArr[i].stringName);
		if(mapArr[i].stringName != NULL)
		{
			if(strcmp(mapArr[i].stringName, strName) == 0)
				break;
		}
	}
	if(i >= sizeEle)
	{
		PRINT_ERR("CMS: fail to get element from map.");
		return NULL;
	}

	return &mapArr[i];
}

int cms_get_resolution(DMS_SIZE *optArr, int streamID, int maxOpt)
{
	int i = 0, j = 0, num = 0;
	char *des = NULL, *resp = NULL;
	char resStr[20][20];
	memset(resStr, 0, sizeof(resStr));
	if(optArr == NULL || maxOpt == 0 || (streamID < 0 || streamID > DMS_MAX_STREAMNUM))
	{
		PRINT_ERR("CMS: invalid paramters.");
		return -1;
	}
	for(i = 0; i < CMS_VIDEO_MAX_ITEM_MAP; i++)
	{
		if(video2Arr[streamID][i].stringName != NULL)
		{
			if(strcmp("resolution", video2Arr[streamID][i].stringName) == 0)
			{
				des = (char *)video2Arr[streamID][i].dataAddress;
				break;
			}
		}
	}
	if(i >= CMS_VIDEO_MAX_ITEM_MAP)
	{
		PRINT_ERR("CMS: no resolution options found.");
		return -1;
	}
	if(des != NULL)
	{
		cJSON *json = NULL;
		json = cJSON_Parse(des);
		if (!json)
		{
			PRINT_INFO("CMS: fail to parse Json.");
			return -1;
		}
	    cJSON *opt = NULL;
	    opt = cJSON_GetObjectItem(json, "opt");
	    if(opt == NULL)
	    {
	        PRINT_ERR("CMS: fail to get opt element.");
			cJSON_Delete(json);
	        return -1;
	    }

		int arraySize = cJSON_GetArraySize(opt);
		///PRINT_INFO("arraySize = %d\n", arraySize);

		cJSON *arrayItem = NULL;
		for(j = 0; j < arraySize; j++)
		{
			arrayItem = cJSON_GetArrayItem(opt, j);
			if(arrayItem == NULL)
			{
				PRINT_ERR("CMS: fail to get array item.");
				cJSON_Delete(json);
				return -1;
			}
			if(arrayItem->valuestring != NULL)
				strncpy(resStr[j], arrayItem->valuestring, 19);

			//PRINT_INFO("resStr: %s\n", resStr[j]);
		}

		cJSON_Delete(json);
		if(maxOpt > arraySize)
			num = arraySize;
		else
			num = maxOpt;

		for(j = 0; j < num; j++)
		{
			optArr[j].nWidth = atoi(resStr[j]);
			resp = strchr(resStr[j], 'x');
			if(resp == NULL)
			{
				PRINT_ERR("CMS: invalid resolution format.");
				optArr[j].nWidth = 0;
				return -1;
			}
			optArr[j].nHeight = atoi(resp+1);
			//PRINT_INFO("resolution: %d x %d\n", optArr[j].nHeight, optArr[j].nWidth);
		}
	}

	return 0;
}

int cms_resolution_to_format(int streamID, int width, int height, LPDMS_NET_COMPRESSION_INFO pstComInfo)
{
#if 0
	int i = 0;
	int formatNum = 0;
	while(m_videoformat[i].lpName != NULL)
	{
		formatNum++;
		i++;
	}

	for(i = 0; i < formatNum; i++)
	{
		if(m_videoformat[i].nWidth == width && m_videoformat[i].nHeight == height)
		{
			pstComInfo->dwStreamFormat = m_videoformat[i].nVideoFormat;
			//width and height should match to nVideoFormat;
			pstComInfo->wWidth		   = width;
			pstComInfo->wHeight		   = height;
			break;
		}
	}
	if(i >= formatNum)
	{
		PRINT_ERR("CMS: fail to translate resolution format.");
		//return -1;
	}
#endif
	int i = 0, ret = 0;
    DMS_SIZE stVideoSize[10];
    memset(stVideoSize, 0, 10*sizeof(DMS_SIZE));
	ret = cms_get_resolution(stVideoSize, streamID, 10);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get resolution.");
		return -1;
	}
	for(i = 0; i < 10; i++)
	{
		if(stVideoSize[i].nWidth == width && stVideoSize[i].nHeight == height)
		{
			//pstComInfo->dwStreamFormat = m_videoformat[i].nVideoFormat;
			pstComInfo->wWidth		   = width;
			pstComInfo->wHeight		   = height;
			break;
		}
	}
	if(i >= 10)
	{
		PRINT_ERR("CMS: unsupported resolution format.");
		return -1;
	}
	return 0;
}

int cms_get_video(LPDMS_NET_COMPRESSION_INFO pstComInfo, GK_NET_VIDEO_CFG stConfigs, int index)
{
	int ret= 0;
	//compression type
	if(stConfigs.vencStream[index].enctype== 1)
		pstComInfo->dwCompressionType = PT_H264;
	else if(stConfigs.vencStream[index].enctype == 2)
		pstComInfo->dwCompressionType = PT_JPEG;
	else if(stConfigs.vencStream[index].enctype == 3)
		pstComInfo->dwCompressionType = PT_H265;
	else
		PRINT_ERR("CMS: invalid compression type.");

	//framte rate
	pstComInfo->dwFrameRate = (DWORD)stConfigs.vencStream[index].h264Conf.fps;
	//resolution format
	//printf("width = %d, height = %d\n", stConfigs.vencStream[index].streamFormat.width,
		//stConfigs.vencStream[index].streamFormat.height);
	ret = cms_resolution_to_format(index, (int)stConfigs.vencStream[index].h264Conf.width,
		(int)stConfigs.vencStream[index].h264Conf.height, pstComInfo);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to translate resolution.");
		return -1;
	}
	//brc mode and bitrate
	if(stConfigs.vencStream[index].h264Conf.rc_mode == 0 || stConfigs.vencStream[index].h264Conf.rc_mode == 2)
	{
		pstComInfo->dwRateType = 0;
		pstComInfo->dwBitRate = (DWORD)(stConfigs.vencStream[index].h264Conf.bps*1000);
	}
	else if(stConfigs.vencStream[index].h264Conf.rc_mode == 1 || stConfigs.vencStream[index].h264Conf.rc_mode == 3)
	{
		pstComInfo->dwRateType = 1;
		//pstComInfo->dwBitRate = (DWORD)((stConfigs.vencStream[index].h264Conf.vbrMinbps+stConfigs.vencStream[index].h264Conf.vbrMaxbps)*1000/2);
		pstComInfo->dwBitRate = (DWORD)(stConfigs.vencStream[index].h264Conf.bps*1000);
	}
	else
	{
		PRINT_ERR("CMS: invalid rate type.");
		return -1;
	}
	//quality
	if(stConfigs.vencStream[index].h264Conf.quality == 0)
		pstComInfo->dwImageQuality = 2;	// common, not good and worst show common
	else if(stConfigs.vencStream[index].h264Conf.quality == 1)
		pstComInfo->dwImageQuality = 1;
	else if(stConfigs.vencStream[index].h264Conf.quality == 2)
		pstComInfo->dwImageQuality = 0;
	//Interval
	pstComInfo->dwMaxKeyInterval = (DWORD)stConfigs.vencStream[index].h264Conf.gop;
	//wEncodeVideo
	//wEncodeAudio
	if(stConfigs.vencStream[index].avStream == 0)
	{
		pstComInfo->wEncodeVideo = 1;
		pstComInfo->wEncodeAudio = 1;
	}
	else if(stConfigs.vencStream[index].avStream == 1)
	{
		pstComInfo->wEncodeVideo = 1;
		pstComInfo->wEncodeAudio = 0;
	}
	else
	{
		pstComInfo->wEncodeVideo = 0;
		pstComInfo->wEncodeAudio = 0;
	}
	return 0;
}

int cms_get_audio(LPDMS_NET_COMPRESSION_INFO pstComInfo, int enable, ST_GK_ENC_STREAM_AUDIO_ATTR stAudioAttr)
{
	//audio: enable, format type, sample

	pstComInfo->wBitsPerSample = stAudioAttr.sample_width;
	pstComInfo->wFormatTag = PT_G711A;

	return 0;
}

int cms_get_stream_attr(LPDMS_NET_COMPRESSION_INFO pstComInfo,  GK_NET_VIDEO_CFG stVideoAttr, int index, int enableAudio, ST_GK_ENC_STREAM_AUDIO_ATTR stAudioAttr)
{
	int ret = 0;
	ret = cms_get_video(pstComInfo, stVideoAttr, index);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get video paramters.");
		return -1;
	}
	ret = cms_get_audio(pstComInfo, enableAudio, stAudioAttr);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get audio paramters.");
		return -1;
	}

	return 0;
}

int cms_get_def_val(GK_CFG_MAP *array, int arrSize, char *strName, int *val)
{
	int i = 0;
	for(i = 0; i < arrSize; i++)
	{
		if(array[i].stringName != NULL && strcmp(array[i].stringName, strName) == 0)
		{
			if(array[i].defaultValue != NULL)
			{
				*val = atoi(array[i].defaultValue);
				break;
			}
		}
	}
	if(i > arrSize)
	{
		PRINT_ERR("CMS: fail to get default value.");
		return -1;
	}

	return 0;
}

int cms_get_stream_def_attr(LPDMS_NET_COMPRESSION_INFO pstComInfo, GK_CFG_MAP *video2Array, int streamID, GK_CFG_MAP *audioArray)
{
	int ret= 0;
	int defValue = 0;
	//
	ret = cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "enctype",  &defValue);//format_encodeType
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get default width.");
		return -1;
	}
	if(defValue == 1)
		pstComInfo->dwCompressionType = PT_H264;
	else if(defValue == 2)
		pstComInfo->dwCompressionType = PT_JPEG;
	else
		PRINT_ERR("CMS: invalid compression type.");
	//resolution
	int width = 0, height = 0;
	ret = cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "width",  &width);//format_width
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get default width.");
		return -1;
	}
	pstComInfo->wWidth = (WORD)width;
	ret = cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "height", &height);//format_height
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get default height.");
		return -1;
	}
	pstComInfo->wHeight = (WORD)height;
	//video format
	ret = cms_resolution_to_format(streamID, width, height, pstComInfo);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to translate resolution.");
		return -1;
	}

	//framte rate
	ret = cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "fps", &defValue);//format_fps
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get fps.");
		return -1;
	}
	pstComInfo->dwFrameRate = (DWORD)defValue;

	//brc mode and bitrate
	ret = cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "rc_mode", &defValue);//h264_brcMode
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get brc.");
		return -1;
	}
	if(defValue == 0 || defValue== 2)
	{
		pstComInfo->dwRateType = 0;
		cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "bps", &defValue);//h264_cbrAvgBps
		pstComInfo->dwBitRate = (DWORD)(defValue*1000);
	}
	else if(defValue == 1 || defValue == 3)
	{
		int mixBps = 0, maxBps = 0;
		pstComInfo->dwRateType = 1;
		cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "h264_vbrMinbps", &mixBps);
		cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "h264_vbrMaxbps", &maxBps);
		pstComInfo->dwBitRate = (DWORD)((mixBps+maxBps)*1000/2);
	}
	else
	{
		PRINT_ERR("CMS: invalid rate type.");
		return -1;
	}

	//quality
	ret = cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "quality", &defValue);//h264_quality
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get quality.");
		return -1;
	}
	pstComInfo->dwImageQuality = (DWORD)defValue;
	//Interval
	ret = cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "gop", &defValue);//h264_gopN
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get interval.");
		return -1;
	}
	pstComInfo->dwMaxKeyInterval = (DWORD)defValue;
	//wEncodeVideo
	ret = cms_get_def_val(video2Array, CMS_VIDEO_MAX_ITEM_MAP, "enable", &defValue);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get video encoder mode.");
		return -1;
	}
	pstComInfo->wEncodeVideo = (WORD)defValue;
	//audio: enable, format type, sample
	ret = cms_get_def_val(audioArray, CMS_AUDIO_MAX_ITEM_MAP, "mode", &defValue);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get audio encoder mode.");
		return -1;
	}
	if(defValue == 0)
		pstComInfo->wEncodeAudio = 0;
	else
		pstComInfo->wEncodeAudio = 1;
	ret = cms_get_def_val(audioArray, CMS_AUDIO_MAX_ITEM_MAP, "type", &defValue);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get audio format.");
		return -1;
	}
	if(defValue == 0)
		pstComInfo->wFormatTag = (WORD)PT_G711A;
	else if(defValue == 1)
		pstComInfo->wFormatTag = (WORD)PT_G711U;
	else if(defValue == 2)
		pstComInfo->wFormatTag = (WORD)PT_LPCM;

	ret = cms_get_def_val(audioArray, CMS_AUDIO_MAX_ITEM_MAP, "sampleBitWidth", &defValue);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get audio encoder mode.");
		return -1;
	}

	pstComInfo->wBitsPerSample = (WORD)defValue;

	return 0;
}

int cms_set_stream_attr(DMS_NET_COMPRESSION_INFO stComInfo, int streamID)
{
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, streamID, &stH264Config);
    if(ret != 0)
    {
        PRINT_ERR("CMS: fail to get video parameterss.");
        return -1;
    }
#if 0
	printf("stH264Config.width: %d\nstH264Config.height: %d\n"
			"stH264Config.fps: %d\nstH264Config.gop: %d\n"
			"stH264Config.rc_mode: %d\nstH264Config.bps:%d\n"
			"stH264Config.quality:%d\nstH264Config.stream_switch: %d\n",
		stH264Config.width,
		stH264Config.height,
		stH264Config.fps,
		stH264Config.gop,
		stH264Config.rc_mode,
		stH264Config.bps,
		stH264Config.quality,
		stH264Config.stream_switch);
#endif
#if 0
	printf("dwCompressionType: %d\ndwFrameRatet: %d\n"
			"dwStreamFormat: %d\nwHeight: %d\nwWidth: %d\n"
			"dwRateType: %d\ndwBitRate: %d\n"
			"dwImageQuality: %d\ndwMaxKeyInterval: %d\n"
			"wEncodeAudio: %d\nwEncodeVideo: %d\n"
			"wFormatTag: %d\nwBitsPerSample: %d\n",
		(int)stComInfo.dwCompressionType, (int)stComInfo.dwFrameRate,
		(int)stComInfo.dwStreamFormat, (int)stComInfo.wHeight, (int)stComInfo.wWidth,
		(int)stComInfo.dwRateType, (int)stComInfo.dwBitRate,
		(int)stComInfo.dwImageQuality, (int)stComInfo.dwMaxKeyInterval,
		(int)stComInfo.wEncodeAudio, (int)stComInfo.wEncodeVideo,
		(int)stComInfo.wFormatTag, (int)stComInfo.wBitsPerSample);
#endif
	if(stComInfo.wEncodeVideo == 1)
	{
		if(stComInfo.wEncodeAudio == 1)
		{
            runVideoCfg.vencStream[streamID].avStream = 0;
		}
		else if(stComInfo.wEncodeAudio == 0)
			runVideoCfg.vencStream[streamID].avStream = 1;
	}
	else if(stComInfo.wEncodeVideo == 0)
	{
		runVideoCfg.vencStream[streamID].avStream = 2;
	}
	GK_CFG_MAP *pstCfg = NULL;
    //encode type
    if(stComInfo.dwCompressionType == PT_H264)
    {
        stH264Config.enctype = 1;
    }
    else if(stComInfo.dwCompressionType == PT_H265)
    {
        stH264Config.enctype = 3;
    }
    else
    {
        PRINT_ERR("CMS: dwCompressionType error[%d].\n", (int)stComInfo.dwCompressionType);
    }
    
	//resolution
    stH264Config.width	 = stComInfo.wWidth;
    stH264Config.height  = stComInfo.wHeight;
    //frame
    pstCfg = cms_get_element_from_map(video2Arr[streamID], "fps");//format_fps
	if(pstCfg == NULL)
	{
		PRINT_ERR("CMS: fail to get element of format_fps");
		return -1;
	}
    if(stComInfo.dwFrameRate < pstCfg->min)
		stH264Config.fps = atoi(pstCfg->defaultValue);
	else if(stComInfo.dwFrameRate > pstCfg->max)
		stH264Config.fps = pstCfg->max;
	else
		stH264Config.fps = stComInfo.dwFrameRate;
	//gop
    pstCfg = cms_get_element_from_map(video2Arr[streamID], "gop");//h264_gopN
	if(pstCfg == NULL)
	{
		PRINT_ERR("CMS: fail to get element of h264_gopN");
		return -1;
	}
    if(stComInfo.dwFrameRate < pstCfg->min)
		stH264Config.gop = atoi(pstCfg->defaultValue);
	else if(stComInfo.dwFrameRate > pstCfg->max)
		stH264Config.gop = pstCfg->max;
	else
		stH264Config.gop = stComInfo.dwMaxKeyInterval;
	//encode mode
	stH264Config.rc_mode = stComInfo.dwRateType;	//keep bps: 0, keep quality: 1
	//bps
	stH264Config.bps 	 = stComInfo.dwBitRate/1000;
	//qulity
	pstCfg = cms_get_element_from_map(video2Arr[streamID], "quality");//h264_quality
	if(pstCfg == NULL)
	{
		PRINT_ERR("CMS: fail to get element of h264_quality");
		return -1;
	}
	if(stComInfo.dwImageQuality == 4 || stComInfo.dwImageQuality == 3 || stComInfo.dwImageQuality == 2)//0->5: best->worst
		stH264Config.quality = pstCfg->min;
	else if(stComInfo.dwImageQuality == 1)
		stH264Config.quality = 1;
	else if(stComInfo.dwImageQuality == 0)
		stH264Config.quality = pstCfg->max;
#if 0
	printf("stH264Config.width: %d\nstH264Config.height: %d\n"
			"stH264Config.fps: %d\nstH264Config.gop: %d\n"
			"stH264Config.rc_mode: %d\nstH264Config.bps:%d\n"
			"stH264Config.quality:%d\n",
		stH264Config.width,
		stH264Config.height,
		stH264Config.fps,
		stH264Config.gop,
		stH264Config.rc_mode,
		stH264Config.bps,
		stH264Config.quality);
#endif

    ret = netcam_video_set(0, streamID, &stH264Config);
    if(ret != 0)
    {
        PRINT_ERR("CMS: fail to set  video parameters.");
        return -1;
    }

	return 0;
}

int Gk_CmsGetStreamSupport(int sock)
{
	int ret = 0;
	int i = 0, j = 0;
    DMS_NET_SUPPORT_STREAM_FMT stream_info;
    memset(&stream_info, 0, sizeof(DMS_NET_SUPPORT_STREAM_FMT));

	ret = cms_video_get_def_map();
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get video default map.");
		return -1;
	}
	ret = cms_audio_get_def_map();
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get audio default map.");
		return -1;
	}

    int stream_num = netcam_video_get_channel_number();
    PRINT_INFO("stream_num:%d\n", stream_num);

    stream_info.dwSize = sizeof(DMS_NET_SUPPORT_STREAM_FMT);
    stream_info.dwChannel = 0;
    //video fmt
    stream_info.dwVideoSupportFmt[0][0] = PT_H264;
	stream_info.dwVideoSupportFmt[0][1] = PT_H265;
    stream_info.dwVideoSupportFmt[1][0] = PT_H264;
	stream_info.dwVideoSupportFmt[1][1] = PT_H265;
    stream_info.dwVideoSupportFmt[2][0] = PT_H264;
	PRINT_INFO("=========================>Get Bps value!\n");
 	//fps range in each stream
	for(i = 0; i < stream_num; i++)
	{
		for(j = 0; j < CMS_VIDEO_MAX_ITEM_MAP; j++)
		{
			if((video2Arr[i][j].stringName != NULL) && (strcmp(video2Arr[i][j].stringName, "bps") == 0))
			{
				//PRINT_ERR("video2Arr[i][j].min: %d, video2Arr[i][j].max: %d\n",
					//video2Arr[i][j].min, video2Arr[i][j].max);
				stream_info.stVideoBitRate[i].nMin = video2Arr[i][j].min*1000;
				stream_info.stVideoBitRate[i].nMax = video2Arr[i][j].max*1000;
				break;
			}
		}
	}
	//Video Size
	for(i = 0; i < stream_num; i++)
	{
		ret = cms_get_resolution(stream_info.stVideoSize[i], i, 10);
		if(ret != 0)
		{
			PRINT_ERR("CMS: fail to get resolution options.");
			return -1;
		}
	}

	//audio fmt
	stream_info.dwAudioFmt[0] = PT_G711A;
	stream_info.dwAudioFmt[1] = PT_PCM_VOICE;
	stream_info.dwAudioFmt[2] = PT_PCM_AUDIO;
	stream_info.dwAudioFmt[3] = PT_ADPCMA;
	//audio sampleRate
	stream_info.dwAudioSampleRate[0] = 8000;
	stream_info.dwAudioSampleRate[1] = 8000;
	stream_info.dwAudioSampleRate[2] = 8000;
	//stream count
	stream_info.byStreamCount = stream_num;
    PRINT_INFO("stream_info.byStreamCount:%d\n", stream_info.byStreamCount);
    
    GkCmsCmdResq(sock, &stream_info, sizeof(DMS_NET_SUPPORT_STREAM_FMT), DMS_NET_GET_SUPPORT_STREAM_FMT);
    return 0;
}

int Gk_CmsGetVideo(int sock)
{
	int ret = 0;
    DMS_NET_CHANNEL_PIC_INFO video;
    memset(&video, 0, sizeof(DMS_NET_CHANNEL_PIC_INFO));
	//video
    GK_NET_VIDEO_CFG stVideoEncodeAttr;
    memset(&stVideoEncodeAttr, 0, sizeof(GK_NET_VIDEO_CFG));
    ret = netcam_video_get_all_channel_par(&stVideoEncodeAttr);
    if(ret != 0)
    {
        PRINT_ERR("CMS: fail to get all video channel parameters.");
        return -1;
    }
    //audio
    int enAudio = -1;
    
    ST_GK_ENC_STREAM_AUDIO_ATTR stAudioEncodeAttr;
    memset(&stAudioEncodeAttr, 0, sizeof(ST_GK_ENC_STREAM_AUDIO_ATTR));
    #if 0
	ret = netcam_audio_get(&enAudio, &stAudioEncodeAttr);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get all audio parameters.");
		//return -1;
	}
    #endif
	///////////////////////////////////////////////////////////
	video.dwSize = sizeof(DMS_NET_CHANNEL_PIC_INFO);
    video.dwChannel = 0;
	strncpy(video.csChannelName, "stream", DMS_NAME_LEN-1);
	//stRecordPara
	ret = cms_get_stream_attr(&video.stRecordPara, stVideoEncodeAttr, 0, enAudio, stAudioEncodeAttr);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get video paramters to RecordPara.");
		return -1;
	}
	//stNetPara
	ret = cms_get_stream_attr(&video.stNetPara, stVideoEncodeAttr, 1, enAudio, stAudioEncodeAttr);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get video paramters to RecordPara.");
		return -1;
	}
	//stPhonePara
	ret = cms_get_stream_attr(&video.stPhonePara, stVideoEncodeAttr, 2, enAudio, stAudioEncodeAttr);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get video paramters to RecordPara.");
		return -1;
	}

    GkCmsCmdResq(sock, &video, sizeof(DMS_NET_CHANNEL_PIC_INFO), DMS_NET_GET_PICCFG);
    return 0;
}


int Gk_CmsGetDefVideo(int sock)
{
	int ret = 0;
    DMS_NET_CHANNEL_PIC_INFO video;
	memset(&video, 0, sizeof(DMS_NET_CHANNEL_PIC_INFO));
    //to do
    //wFormatTag ²ÎÕÕ DMS_FORMAT
    // dwStreamFormat ²ÎÕÕ DMS_VIDEO_FORMAT
    video.dwSize =  sizeof(DMS_NET_CHANNEL_PIC_INFO);
    video.dwChannel = 0;
    strncpy(video.csChannelName, "stream", DMS_NAME_LEN-1);

    ret = cms_video_get_def_map();
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get video default map.");
		return -1;
	}
#if 0
	ret = cms_audio_get_def_map();
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get audio default map.");
		return -1;
	}
#endif
	//stRecordPara
	ret = cms_get_stream_def_attr(&video.stRecordPara, video2Arr[0], 0, audioArr);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail get default stream attr.");
		return -1;
	}
	//stNetPara
	ret = cms_get_stream_def_attr(&video.stNetPara, video2Arr[1], 1, audioArr);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail get default stream attr.");
		return -1;
	}
	//stPhonePara
	ret = cms_get_stream_def_attr(&video.stPhonePara, video2Arr[2], 2, audioArr);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail get default stream attr.");
		return -1;
	}

    GkCmsCmdResq(sock, &video, sizeof(DMS_NET_CHANNEL_PIC_INFO), DMS_NET_GET_DEF_PICCFG);
    return 0;
}


int Gk_CmsSetVideo(int sock)
{
	int ret = 0;
    DMS_NET_CHANNEL_PIC_INFO video;
    ret = GkSockRecv(sock, (char *)&video, sizeof(DMS_NET_CHANNEL_PIC_INFO));
    if (ret != sizeof(DMS_NET_CHANNEL_PIC_INFO))
    {
        PRINT_ERR();
        return -1;
    }
	//printf("video.dwChannel: %d\n", (int)video.dwChannel);
	//printf("video.csChannelName: %s\n", video.csChannelName);
	ret = cms_set_stream_attr(video.stRecordPara, 0);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to set main stream.");
		return -1;
	}
	ret = cms_set_stream_attr(video.stNetPara, 1);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to set second stream.");
		return -1;
	}
	ret = cms_set_stream_attr(video.stPhonePara, 2);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to ser third stream.");
		return -1;
	}

    return 0;
}

