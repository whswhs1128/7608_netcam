/*!
*****************************************************************************
** FileName     : gk_cms_media.c
**
** Description  : media api of cms.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "common.h"
#include "cfg_all.h"

int GkSockSendToAll(char *szbuf, int len, int type)
{
    int i = 0;
    int sock = 0;
    int ret = 0;
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        sock = GkGetLvSockByIndex(i, type);
        if (sock > 0) {
            ret = GkSockSendFrame(sock, szbuf, len);
            #if 1
            if (ret < 0) {
                //GkCloseLvSockByIndex(i, type);
                GkCloseAllSockByIndex(i);
            }
            #endif
        }
    }
    return 0;
}

int GkSendFrameUnit(void *data, int size, GK_NET_FRAME_HEADER *header, int is_first, int stream_type)
{
    NET_DATA_PACKET data_packet;

    data_packet.stPackHead.nFlag = JBNV_FLAG;
    data_packet.stPackHead.nSize = sizeof(DATA_PACKET) - PACK_SIZE + size;

    data_packet.stPackData.wIsSampleHead = is_first;
    data_packet.stPackData.wBufSize = size;
    data_packet.stPackData.stFrameHeader.wMotionDetect = 0;
    data_packet.stPackData.stFrameHeader.wFrameIndex = header->frame_no;

    if (header->frame_type == GK_NET_FRAME_TYPE_A) {
        #if 1
        data_packet.stPackData.stFrameHeader.dwVideoSize = 0;
        data_packet.stPackData.stFrameHeader.wAudioSize = header->frame_size;
        data_packet.stPackData.stFrameHeader.byKeyFrame = 1;
        #else
        return 0;
        #endif
    } else if (header->frame_type == GK_NET_FRAME_TYPE_I) {
        data_packet.stPackData.stFrameHeader.dwVideoSize = header->frame_size;
        data_packet.stPackData.stFrameHeader.wAudioSize = 0;
        data_packet.stPackData.stFrameHeader.byKeyFrame = 1;
    } else {
        data_packet.stPackData.stFrameHeader.dwVideoSize = header->frame_size;
        data_packet.stPackData.stFrameHeader.wAudioSize = 0;
        data_packet.stPackData.stFrameHeader.byKeyFrame = 0;
    }

    data_packet.stPackData.stFrameHeader.dwTimeTick = (DWORD)((header->sec * 1000) + (header->usec / 1000));
    data_packet.stPackData.stFrameHeader.byReserved = (BYTE)header->video_reso;

    memcpy(data_packet.stPackData.byPackData, data, size);

    GkSockSendToAll((char *)&data_packet, sizeof(NET_DATA_PACKET) - PACK_SIZE + size, stream_type);

    return 0;
}

static int GkSockSendSliceToAll(char *szbuf, int len, int type)
{
    int one_send_len = 50 * 1024;
    char *data = szbuf;
    int offset = 0;
    int left_len = len;
    while(left_len > 0) {
        int send_len = 0;
        if (left_len < one_send_len)
            send_len = left_len;
        else
            send_len = one_send_len;

        GkSockSendToAll(data + offset, send_len, type);

        offset += send_len;
        left_len -= send_len;
    }
    return 0;
}

int GkSendOneFrame(void *data, int size, GK_NET_FRAME_HEADER *pHeader, int stream_type)
{
    GK_NET_CMS_FRAME_HEADER cms_frame_header;
    memset(&cms_frame_header, 0, sizeof(GK_NET_CMS_FRAME_HEADER));

    cms_frame_header.dwSize = sizeof(GK_NET_CMS_FRAME_HEADER);
    cms_frame_header.dwFrameIndex = pHeader->frame_no;
    cms_frame_header.dwTimeTick = (DWORD)((pHeader->sec * 1000) + (pHeader->usec / 1000));
	cms_frame_header.byVideoCode = pHeader->media_codec_type ? PT_H265 : GK_CMS_PT_H264;
    cms_frame_header.byAudioCode = GK_CMS_PT_G711A;
    cms_frame_header.byReserved1 = (BYTE)pHeader->video_reso;

    if (pHeader->frame_type == GK_NET_FRAME_TYPE_A) {
        cms_frame_header.dwVideoSize = 0;
        cms_frame_header.wAudioSize = size;
        //printf("stream%d A no:%lu size:%u\n", stream_type, cms_frame_header.dwFrameIndex, cms_frame_header.wAudioSize);
    } else {
        cms_frame_header.dwVideoSize = size;
        cms_frame_header.wAudioSize = 0;

        if (pHeader->frame_type == GK_NET_FRAME_TYPE_I) {
            cms_frame_header.byFrameType = 0;
            cms_frame_header.byReserved2 = 1;
            //if (stream_type == 0)
                //printf("stream%d I no:%lu size:%lu\n", stream_type, cms_frame_header.dwFrameIndex, cms_frame_header.dwVideoSize);
        } else if (pHeader->frame_type == GK_NET_FRAME_TYPE_P) {
            cms_frame_header.byFrameType = 1;
            cms_frame_header.byReserved2 = 2;
            //if (stream_type == 0)
                //printf("stream%d P no:%lu size:%lu\n", stream_type, cms_frame_header.dwFrameIndex, cms_frame_header.dwVideoSize);
        }
        cms_frame_header.byReserved1 = stream_type;
        

        if (stream_type == 0) {
            cms_frame_header.wVideoWidth = runVideoCfg.vencStream[0].h264Conf.width;
            cms_frame_header.wVideoHeight = runVideoCfg.vencStream[0].h264Conf.height;
        } else if (stream_type == 1) {
            cms_frame_header.wVideoWidth = runVideoCfg.vencStream[1].h264Conf.width;
            cms_frame_header.wVideoHeight = runVideoCfg.vencStream[1].h264Conf.height;
        } else {
            PRINT_ERR("cms not support stream%d", stream_type);
            return -1;
        }
        //printf("stream%d V width:%u height:%u\n", stream_type, cms_frame_header.wVideoWidth, cms_frame_header.wVideoHeight);
    }

    GkSockSendToAll((char *)&cms_frame_header, sizeof(GK_NET_CMS_FRAME_HEADER), stream_type);

    #if 1
    GkSockSendSliceToAll(data, size, stream_type);
    #else
    GkSockSendToAll(data, size, stream_type);
    #endif
    return 0;
}

int GkSendFrame(void *data, int size, GK_NET_FRAME_HEADER *header, int stream_type)
{
#if 0
    int ret;
    int nOnePacketSize = PACK_SIZE;
    int nOffSet = 0;
    int frame_len = size;
    if (frame_len > nOnePacketSize) {
        ret = GkSendFrameUnit(data + nOffSet, nOnePacketSize, header, 1, stream_type);
        nOffSet += nOnePacketSize;
        frame_len -= nOnePacketSize;
    } else {
        ret = GkSendFrameUnit(data, frame_len, header, 1, stream_type);
        return frame_len;
    }

    while (frame_len >= nOnePacketSize) {
        ret = GkSendFrameUnit(data + nOffSet, nOnePacketSize, header, 0, stream_type);

        nOffSet += nOnePacketSize;
        frame_len -= nOnePacketSize;
    }

    if (frame_len > 0) {
        ret = GkSendFrameUnit(data + nOffSet, frame_len, header, 0, stream_type);

        nOffSet += frame_len;
        frame_len -= frame_len;
    }
    ret = ret;
    return nOffSet;
#else
    return GkSendOneFrame(data, size, header, stream_type);
#endif
}


