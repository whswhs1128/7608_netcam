#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"
#include "utility_api.h"

// OSD 704*576 X,Y in CMS
#define   CMS_WIDTH   704
#define   CMS_HEIGHT  576



static void GetOsd(DMS_NET_CHANNEL_OSDINFO *ptrOsd)
{

    ST_GK_ENC_STREAM_H264_ATTR h264Attr;
    GK_NET_CHANNEL_INFO channelInfo;
    DMS_NET_CHANNEL_OSDINFO osdInfo;
    int dataOffset = (int)&osdInfo.bShowTime - (int)&osdInfo;

	ptrOsd->dwSize = sizeof(DMS_NET_CHANNEL_OSDINFO);
    ptrOsd->dwChannel = 0;
    netcam_video_get(0, 0 ,&h264Attr);
    netcam_osd_get_info(0,&channelInfo);
    //to do
    memset(&ptrOsd->bShowTime,0,sizeof(DMS_NET_CHANNEL_OSDINFO)-dataOffset);

    ptrOsd->bShowTime = channelInfo.osdDatetime.enable;
    ptrOsd->bDispWeek = channelInfo.osdDatetime.displayWeek;
    ptrOsd->dwTimeTopLeftX = gk_cms_float_rate_to_data(channelInfo.osdDatetime.x , CMS_WIDTH) ;
    ptrOsd->dwTimeTopLeftY = gk_cms_float_rate_to_data(channelInfo.osdDatetime.y , CMS_HEIGHT);

	ptrOsd->byShowChanName = channelInfo.osdChannelID.enable;
    strncpy(ptrOsd->csChannelName,channelInfo.osdChannelID.text,DMS_NAME_LEN);
    ptrOsd->dwShowNameTopLeftX = gk_cms_float_rate_to_data(channelInfo.osdChannelID.x , CMS_WIDTH) ;
    ptrOsd->dwShowNameTopLeftY = gk_cms_float_rate_to_data(channelInfo.osdChannelID.y , CMS_WIDTH) ;
	//printf("show time:%d,%d,%d,%d,%s,%d,%d\n",ptrOsd->bShowTime,ptrOsd->bDispWeek,ptrOsd->dwTimeTopLeftX,ptrOsd->dwTimeTopLeftY,
	//	ptrOsd->csChannelName,ptrOsd->dwShowNameTopLeftX,ptrOsd->dwShowNameTopLeftY);
    #if 0
    ptrOsd->stOsd[0].bOsdEnable = channelInfo.osdChannelID.enable;
    ptrOsd->stOsd[0].dwOsdContentType = 0; // not support
    ptrOsd->stOsd[0].dwLayer = 0; // not support
    ptrOsd->stOsd[0].dwAreaAlpha = 0; // not support
    ptrOsd->stOsd[0].dwFgColor = 0; // not support
    ptrOsd->stOsd[0].dwBgColor = 0; // not support


    ptrOsd->stOsd[0].nLeftX = ptrOsd->stOsd[0].nLeftX*255/100;// from x/100-> x/255
    ptrOsd->stOsd[0].nLeftY = ptrOsd->stOsd[0].nLeftY*255/100; // from x/100-> x/255

    ptrOsd->stOsd[0].nWidth = 0; /**< 区域宽，为绝对值,图片时有效*/
    ptrOsd->stOsd[0].nHeight = 0; /**< 区域高，为绝对值,图片时有效*/
    strncpy(ptrOsd->stOsd[0].csOsdCotent,channelInfo.osdChannelID.text,DMS_MAX_OSD_LEN);
    #endif
}

static void GetShelter(DMS_NET_CHANNEL_SHELTER *ptrShelter )
{
    GK_NET_SHELTER_RECT cover;
    DMS_NET_CHANNEL_SHELTER shelter;
    int i,enable = 0;
    int dataOffset = (int)&shelter.bEnable - (int)&shelter;

    memset(&ptrShelter->bEnable, 0, sizeof(DMS_NET_CHANNEL_SHELTER)-dataOffset);
	ptrShelter->dwSize = sizeof(DMS_NET_CHANNEL_SHELTER);
	ptrShelter->dwChannel = 0;
    //todo
    for( i = 0; i < 4; i++)
    {
        netcam_pm_get_cover(0,i,&cover);

        enable |= cover.enable;
        ptrShelter->strcShelter[i].nType = 1;
		ptrShelter->strcShelter[i].wLeft = gk_cms_float_rate_to_data(cover.x , CMS_WIDTH);
        ptrShelter->strcShelter[i].wTop  = gk_cms_float_rate_to_data(cover.y , CMS_HEIGHT);
        ptrShelter->strcShelter[i].wWidth = gk_cms_float_rate_to_data(cover.width, CMS_WIDTH);
        ptrShelter->strcShelter[i].wHeight = gk_cms_float_rate_to_data(cover.height, CMS_HEIGHT);
        /*ptrShelter->strcShelter[i].wLeft = gk_cms_rate_to_data(cover.x , CMS_WIDTH);
        ptrShelter->strcShelter[i].wTop  = gk_cms_rate_to_data(cover.y , CMS_HEIGHT);
        ptrShelter->strcShelter[i].wWidth = gk_cms_rate_to_data(cover.width, CMS_WIDTH);
        ptrShelter->strcShelter[i].wHeight = gk_cms_rate_to_data(cover.height, CMS_HEIGHT);*/
        ptrShelter->strcShelter[i].dwColor = cover.color;
    }

    if(enable != 0)
    {
        ptrShelter->bEnable = 1;
    }
    else
    {
        ptrShelter->bEnable = 0;
    }


}

int Gk_CmsGetOsd(int sock, int stream_no)
{
    char tmp[DMS_NAME_LEN] = {0};
    DMS_NET_CHANNEL_OSDINFO osd;
    DMS_NET_CHANNEL_OSDINFO *ptrOsd = &osd;


    PRINT_INFO("enter,channel id:%ld",ptrOsd->dwChannel);
    GetOsd( ptrOsd);
    utility_gbk_to_utf8(ptrOsd->csChannelName, tmp, DMS_NAME_LEN);
    memset(ptrOsd->csChannelName, 0, DMS_NAME_LEN);
    memcpy(ptrOsd->csChannelName, tmp, DMS_NAME_LEN);

    return GkCmsCmdResqByIndex(sock, &osd, sizeof(DMS_NET_CHANNEL_OSDINFO), DMS_NET_GET_OSDCFG, stream_no);
}

int Gk_CmsGetShelter(int sock, int stream_no)
{
    DMS_NET_CHANNEL_SHELTER shelter;
    //DMS_NET_CHANNEL_SHELTER *ptrShelter = &shelter;

    //PRINT_INFO("enter,channel id:%ld",ptrShelter->dwChannel);
    //todo
    GetShelter(&shelter);


    return GkCmsCmdResqByIndex(sock, &shelter, sizeof(DMS_NET_CHANNEL_SHELTER), DMS_NET_GET_SHELTERCFG, stream_no);
}


int Gk_CmsSetOsd(int sock, int stream_no)
{
    DMS_NET_CHANNEL_OSDINFO osd;
    DMS_NET_CHANNEL_OSDINFO *ptrOsd = &osd;
    int ret;
    int i;

    PRINT_INFO();

    ret = GkSockRecv(sock, (char *)&osd, sizeof(DMS_NET_CHANNEL_OSDINFO));
    if (ret != sizeof(DMS_NET_CHANNEL_OSDINFO)) {
        PRINT_ERR();
        return -1;
    }
    GK_NET_CHANNEL_INFO channelInfo;
    ST_GK_ENC_STREAM_H264_ATTR h264Attr;

    PRINT_INFO("dw channel:%ld",ptrOsd->dwChannel);

    ptrOsd->dwChannel = 0;
    netcam_video_get(0, ptrOsd->dwChannel ,&h264Attr);

    netcam_osd_get_info(0,&channelInfo);


    channelInfo.osdDatetime.enable = ptrOsd->bShowTime;
    channelInfo.osdDatetime.displayWeek = ptrOsd->bDispWeek;
    channelInfo.osdDatetime.x = gk_cms_data_to_float_rate(ptrOsd->dwTimeTopLeftX , CMS_WIDTH) ;
    channelInfo.osdDatetime.y = gk_cms_data_to_float_rate(ptrOsd->dwTimeTopLeftY, CMS_HEIGHT) ;
    //channelInfo.osdDatetime.dateSprtr = 1;
    //channelInfo.osdDatetime.timeFmt = runChannelCfg.channelInfo[0].osdDatetime.timeFmt;
    //channelInfo.osdDatetime.dateFormat = 0;

    channelInfo.osdChannelID.x = gk_cms_data_to_float_rate(ptrOsd->dwShowNameTopLeftX , CMS_WIDTH);
    channelInfo.osdChannelID.y = gk_cms_data_to_float_rate(ptrOsd->dwShowNameTopLeftY , CMS_HEIGHT);
    
    utility_utf8_to_gbk(ptrOsd->csChannelName,channelInfo.osdChannelID.text,DMS_NAME_LEN);
    //strncpy(channelInfo.osdChannelID.text,ptrOsd->csChannelName,DMS_NAME_LEN);
    channelInfo.osdChannelID.enable = ptrOsd->byShowChanName;
    #if 0 // no osd channelID for IPC
    channelInfo.osdChannelID.enable = ptrOsd->stOsd[0].bOsdEnable;
    channelInfo.osdChannelID.x = ptrOsd->stOsd[0].nLeftX*100/255 ;
    channelInfo.osdChannelID.y = ptrOsd->stOsd[0].nLeftY*100/255;
    strncpy(channelInfo.osdChannelID.text,ptrOsd->stOsd[0].csOsdCotent,MAX_STR_LEN_128);
    #endif
	for(i = 0; i < netcam_video_get_channel_number(); i++)
	{
		memcpy(&runChannelCfg.channelInfo[i].osdDatetime, &channelInfo.osdDatetime, sizeof(GK_NET_OSD_DATETIME));
		memcpy(&runChannelCfg.channelInfo[i].osdChannelID, &channelInfo.osdChannelID, sizeof(GK_NET_OSD_CHANNEL_ID));
	}
    netcam_osd_update_id();
    //todo
    netcam_timer_add_task(netcam_osd_pm_save, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);	// 8*250ms = 2s
    return 0;
}

int Gk_CmsSetShelter(int sock, int stream_no)
{
    DMS_NET_CHANNEL_SHELTER shelter;
    int i;
    GK_NET_SHELTER_RECT cover;

    PRINT_INFO();
    int ret = GkSockRecv(sock, (char *)&shelter, sizeof(DMS_NET_CHANNEL_SHELTER));
    if (ret != sizeof(DMS_NET_CHANNEL_SHELTER)) {
        PRINT_ERR();
        return -1;
    }
    else
    {
        PRINT_INFO("dw channel:%d",(int)shelter.dwChannel);
        if(shelter.dwChannel == 0)
        {
            PRINT_INFO("bEnable:%d",(int)shelter.bEnable);
            cover.enable = shelter.bEnable;
            for(i = 0; i < 4; i++)
            {
				cover.x = gk_cms_data_to_float_rate(shelter.strcShelter[i].wLeft , CMS_WIDTH);
                cover.y = gk_cms_data_to_float_rate(shelter.strcShelter[i].wTop, CMS_HEIGHT);
                cover.width = gk_cms_data_to_float_rate(shelter.strcShelter[i].wWidth, CMS_WIDTH);
                cover.height = gk_cms_data_to_float_rate(shelter.strcShelter[i].wHeight, CMS_HEIGHT);
                cover.color = shelter.strcShelter[i].dwColor;
                netcam_pm_set_cover(0,i,cover);
            }
            netcam_timer_add_task(netcam_osd_pm_save, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);
        }

    }
    //todo

    return 0;
}
