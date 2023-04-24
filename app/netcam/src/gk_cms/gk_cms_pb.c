/*!
*****************************************************************************
** FileName     : gk_cms_pb.c
**
** Description  : playback api of cms.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-10-12, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "gk_cms_common.h"
#include "gk_cms_utility.h"
#include "gk_cms_protocol.h"
#include "gk_cms_sock.h"

#ifdef MODULE_SUPPORT_LOCAL_REC
#include "avi_utility.h"
#include "avi_search_api.h"
#include "gk_cms_pb.h"

extern int avi_flag_pause;
int is_step = 0;
static sem_t  step_sem;
static int is_stop = 0;
static AviPBHandle *pPBHandle = NULL;
pthread_mutex_t pb_getframe_x = PTHREAD_MUTEX_INITIALIZER;
static int is_pb_loop_running = 0;

static AviPBHandle *Gk_Cms_PlayBackByName(char *csFileName)
{
    PRINT_INFO("call Gk_Cms_PlayBackByName\n");

    AviPBHandle *pPBHandle = NULL;
    pPBHandle = (AviPBHandle *)create_pb_handle();
    if (NULL == pPBHandle) {
        PRINT_ERR();
        return NULL;
    }

    pthread_mutex_init(&pb_getframe_x, NULL);
    int ret = avi_pb_open(csFileName, pPBHandle);
    if (ret < 0) {
        PRINT_ERR("avi_pb_open %s error.\n", csFileName);
        //avi_pb_close(pPBHandle);
        return NULL;
    }

    print_pb_handle(pPBHandle);

    return pPBHandle;
}

static AviPBHandle *Gk_Cms_PlayBackByTime(AVI_DMS_TIME *lpStartTime, AVI_DMS_TIME *lpStopTime)
{
    PRINT_INFO("call Gk_Cms_PlayBackByTime\n");

    u32t ch_num = 0;
    u64t start = time_to_u64t(lpStartTime);
    u64t stop = time_to_u64t(lpStopTime);

    AviPBHandle *pPBHandle = NULL;
    pPBHandle = (AviPBHandle *)create_pb_handle();
    if (NULL == pPBHandle) {
        PRINT_ERR();
        return NULL;
    }

    pPBHandle->list = search_file_by_time(0, ch_num, start, stop);  //0， 所有的情况
    if (NULL == pPBHandle->list) {
        PRINT_ERR("list is null.\n");
        //avi_pb_close(pPBHandle);
        return NULL;
    }

    print_pb_handle(pPBHandle);

    return pPBHandle;
}


static int Gk_Cms_PlayBackStop(AviPBHandle *pPBHandle)
{
    PRINT_INFO("call Gk_Cms_PlayBackStop\n");
    avi_pb_close(pPBHandle);
    pthread_mutex_destroy(&pb_getframe_x);
    return 0;
}

#if 0
int Gk_Cms_PlayBackControl(AviPBHandle *pPBHandle, DWORD dwControlCode, char    *lpInBuffer, DWORD    dwInLen,  char     *lpOutBuffer,  DWORD    *lpOutLen)
{
    PRINT_INFO("call Gk_Cms_PlayBackControl\n");

    switch (dwControlCode) {
        case DMS_NET_PLAYSTART:
            printf("in PlayBackControl, cmd = start\n");
            avi_pb_start();
            break;
        case DMS_NET_PLAYPAUSE:
            printf("in PlayBackControl, cmd = pause\n");
            avi_pb_pause();
            break;
        case DMS_NET_PLAYRESTART:
            printf("in PlayBackControl, cmd = restart\n");
            avi_pb_restart(pPBHandle);
            break;
        case DMS_NET_FASTFORWARD2X:
            printf("in PlayBackControl, cmd = fastforward2\n");
            avi_pb_set_speed(2);
            break;
        case DMS_NET_FASTFORWARD4X:
            printf("in PlayBackControl, cmd = fastforward4\n");
            avi_pb_set_speed(4);
            break;
        case DMS_NET_FASTFORWARD8X:
            printf("in PlayBackControl, cmd = fastforward8\n");
            avi_pb_set_speed(8);
            break;
        case DMS_NET_FASTFORWARD16X:
            printf("in PlayBackControl, cmd = fastforward16\n");
            avi_pb_set_speed(16);
            break;
        case DMS_NET_FASTFORWARD32X:
            printf("in PlayBackControl, cmd = fastforward32\n");
            avi_pb_set_speed(32);
            break;
        case DMS_NET_SLOWPLAY2X:
            printf("in PlayBackControl, cmd = slowplay2\n");
            break;
        case DMS_NET_SLOWPLAY4X:
            printf("in PlayBackControl, cmd = slowplay4\n");
            break;
        case DMS_NET_SLOWPLAY8X:
            printf("in PlayBackControl, cmd = slowplay8\n");
            break;
        case DMS_NET_SLOWPLAY16X:
            printf("in PlayBackControl, cmd = slowplay16\n");
            break;
        case DMS_NET_SLOWPLAY32X:
            printf("in PlayBackControl, cmd = slowplay32\n");
            break;
        case DMS_NET_PLAYNORMAL:
            printf("in PlayBackControl, cmd = playnormal\n");
            avi_pb_set_speed(1);
            break;
        case DMS_NET_PLAYFRAME:
            printf("in PlayBackControl, cmd = playframe\n");
            break;
        case DMS_NET_PLAYSETPOS:
            printf("in PlayBackControl, cmd = set pos\n");
            pthread_mutex_lock(&pb_getframe_x);
            avi_pb_set_pos(pPBHandle, (int *)lpInBuffer);
            pthread_mutex_unlock(&pb_getframe_x);
            break;
        case DMS_NET_PLAYGETPOS:
            printf("in PlayBackControl, cmd = get pos\n");
            *lpOutLen = avi_pb_get_pos(pPBHandle);
            break;
        case DMS_NET_PLAYGETTIME:
            printf("in PlayBackControl, cmd = get time\n");
            struct tm now_time;
            avi_pb_get_time(pPBHandle, &now_time);
            break;
        case DMS_NET_PLAYGETFRAME:
            printf("in PlayBackControl, cmd = get frame\n");
            avi_pb_get_frame(pPBHandle, lpOutBuffer, (int *)lpOutLen);
            break;
        case DMS_NET_GETTOTALFRAMES:
            printf("in PlayBackControl, cmd = get total frames\n");
            break;
        case DMS_NET_GETTOTALTIME:
            printf("in PlayBackControl, cmd = get total time\n");
            break;
        case DMS_NET_THROWBFRAME:
            printf("in PlayBackControl, cmd = throw bframe\n");
            break;
        default:
            printf("in PlayBackControl, cmd = other\n");
            break;
    }

    return 0;
}
#endif

static int GkSockPbSend(int sock, AviPBHandle *pPBHandle, char *pBuff, int size)
{
    int ret;
    GK_NET_CMS_FRAME_HEADER cms_frame_header;
    memset(&cms_frame_header, 0, sizeof(GK_NET_CMS_FRAME_HEADER));

    cms_frame_header.dwSize = sizeof(GK_NET_CMS_FRAME_HEADER);
    cms_frame_header.dwFrameIndex = pPBHandle->no;
    cms_frame_header.dwTimeTick = pPBHandle->node.timetick;
    cms_frame_header.byVideoCode = GK_CMS_PT_H264;
    cms_frame_header.byAudioCode = GK_CMS_PT_G711A;
    //cms_frame_header.byReserved1 = (BYTE)pHeader->video_reso;

    if (pPBHandle->node.frame_type == 0x00) {  // audio
        cms_frame_header.dwVideoSize = 0;
        cms_frame_header.wAudioSize = pPBHandle->node.size;
        //printf("stream%d A no:%lu size:%u\n", stream_type, cms_frame_header.dwFrameIndex, cms_frame_header.wAudioSize);
    } else {
        cms_frame_header.dwVideoSize = pPBHandle->node.size;
        cms_frame_header.wAudioSize = 0;

        if (pPBHandle->node.frame_type == 0x11) { // I
            cms_frame_header.byFrameType = 1;
            //if (stream_type == 0)
                //printf("stream%d I no:%lu size:%lu\n", stream_type, cms_frame_header.dwFrameIndex, cms_frame_header.dwVideoSize);
        } else if (pPBHandle->node.frame_type == 0x10) { // P
            cms_frame_header.byFrameType = 0;
            //if (stream_type == 0)
                //printf("stream%d P no:%lu size:%lu\n", stream_type, cms_frame_header.dwFrameIndex, cms_frame_header.dwVideoSize);
        }

        cms_frame_header.wVideoWidth = pPBHandle->video_width;
        cms_frame_header.wVideoHeight = pPBHandle->video_height;
        //printf("V width:%u height:%u\n", cms_frame_header.wVideoWidth, cms_frame_header.wVideoHeight);
   }

    ret = GkSockSendFrame(sock, (char *)&cms_frame_header, sizeof(GK_NET_CMS_FRAME_HEADER));
    if (ret < 0) {
        PRINT_ERR("Playback, GkSockSendFrame header error.\n");
        return -1;
    }

    ret = GkSockSendFrame(sock, pPBHandle->node.one_frame_buf, size);
    if (ret < 0) {
        PRINT_ERR("Playback, GkSockSendFrame header error.\n");
        return -1;
    }

    return 0;
}

int Gk_CmsPbControl(int sock)
{
    VODACTION_REQ pb_cmd;
    memset(&pb_cmd, 0, sizeof(VODACTION_REQ));
    int ret = GkSockRecv(sock, (char *)&pb_cmd, sizeof(VODACTION_REQ));
    if (ret != sizeof(VODACTION_REQ)) {
        PRINT_ERR("recv %d, struct size: %d\n", ret, sizeof(VODACTION_REQ));
        return -1;
    }

    switch (pb_cmd.dwAction) {
        case JB_PLAY_CMD_PLAY:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_PLAY\n", sock);
            is_stop = 0;
            is_step = 0;
            avi_flag_pause = 0;
            break;
        case JB_PLAY_CMD_STOP:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_STOP\n", sock);
            is_stop = 1;
            avi_flag_pause = 0;
            break;
        case JB_PLAY_CMD_PAUSE:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_PAUSE\n", sock);
            is_step = 0;
            avi_flag_pause = 1;
            break;
        case JB_PLAY_CMD_RESUME:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_RESUME\n", sock);
            is_step = 0;
            avi_flag_pause = 0;
            avi_pb_pause();
            break;
        case JB_PLAY_CMD_FASTPLAY:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_FASTPLAY, speed = %lu\n", sock, pb_cmd.dwData);
            is_step = 0;
            avi_pb_set_speed(pb_cmd.dwData);
            break;
        case JB_PLAY_CMD_STEPIN:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_STEPIN\n", sock);
            is_step = 1;
            avi_flag_pause = 0;
            sem_post(&step_sem);
            break;
        case JB_PLAY_CMD_FIRST:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_FIRST\n", sock);
            break;
        case JB_PLAY_CMD_LAST:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_LAST\n", sock);
            break;
        case JB_PLAY_CMD_SEEK:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_SEEK, seek = %lu\n", sock, pb_cmd.dwData);
            pthread_mutex_lock(&pb_getframe_x);
            avi_pb_seek(pPBHandle, pb_cmd.dwData);
            pthread_mutex_unlock(&pb_getframe_x);
            break;
        case JB_PLAY_CMD_SOUND:
            PRINT_INFO("pb cmd sock:%d, action = JB_PLAY_CMD_SOUND\n", sock);
            break;
            
        default:
            PRINT_INFO("pb cmd sock:%d, in PlayBackControl, unknown cmd = %lu\n", sock, pb_cmd.dwAction);
            break;
    }

    return 0;

}


static int Gk_Pb_SendFrameLoop(int sock, AviPBHandle *pPBHandle)
{
    int ret;


    int size = 0;
    FILE_NODE node;

    is_stop = 0;
    avi_flag_pause = 0;
    is_step = 0;

    while (1) {

        while(avi_flag_pause)
            sleep(2);

        if (is_stop) {
            PRINT_INFO("to stop.\n");
            break;
        }

        if (is_step) {
            PRINT_INFO("to step in.\n");
            sem_wait(&step_sem);
        }

        // PRINT_INFO("no = %d, count = %d \n", p_frame->no, p_frame->index_count);
        //播放结束时，如果是单文件(链表为NULL)返回200002
        //如果不是，则取下一个文件
        if (pPBHandle->no >= pPBHandle->index_count) {
            PRINT_INFO("playback %s end. no = %d\n", pPBHandle->file_path, pPBHandle->no);

            //单文件
            if (pPBHandle->list == NULL) {
                PRINT_INFO("playback end");
                break;
            }

            //多文件
            memset(&node, 0, sizeof(FILE_NODE));
            ret = get_file_node(pPBHandle->list, &node);
            if (ret != DMS_NET_FILE_SUCCESS) {
                PRINT_ERR("get_file_node failed, ret = %d\n", ret);
                break;
            }
            print_node(&node);
            ret = avi_pb_open(node.path, pPBHandle);
            if (ret < 0) {
                PRINT_ERR("avi_pb_open %s error.\n", node.path);
                break;
            }
        }

        pthread_mutex_lock(&pb_getframe_x);
        ret = avi_pb_get_frame(pPBHandle, NULL, &size);
        pthread_mutex_unlock(&pb_getframe_x);
        if (ret < 0) {
            PRINT_ERR("call avi_pb_get_frame failed.\n");
            break;
        }
        if (ret == GK_NET_NOMOREFILE) {
            PRINT_INFO("playback file end.\n");
            break;
        }

        ret = GkSockPbSend(sock, pPBHandle, NULL, size);
        if (ret < 0) {
            PRINT_ERR("call GkSockSendFrame failed.\n");
            break;
        }

        //PRINT_INFO("no:%d\n", pPBHandle->no);

    }

    PRINT_INFO("Gk_Pb_SendFrameLoop end.\n");
    is_pb_loop_running = 0;
    return 0;
}

static int cal_pb_gop(AviPBHandle *pPBHandle)
{
    int gop = 0;
    int no = 0;

    while (1)
    {
        if ((pPBHandle->idx_array[4*no] == MAKE_FOURCC('0','0','d','c'))
            && (pPBHandle->idx_array[4*no+1] == 0x11))
        {
            gop = 0;
            break;
        }
        no ++;
    }
    //printf("first I :%d\n", no);

    no ++;
    while (1)
    {
        if (pPBHandle->idx_array[4*no] == MAKE_FOURCC('0','0','d','c'))
        {
            gop ++;
            if (pPBHandle->idx_array[4*no+1] == 0x11)
            {
                break;
            }
        }
        no ++;
    }
    //printf("gop :%d, no: %d\n", gop, no);  
    pPBHandle->gop = gop;

    return 0;
}

int Gk_Playback(int sock)
{
    /* 上个pb线程结束才会进行下一个 */
    while (is_pb_loop_running)
        sleep(2);
    is_pb_loop_running = 1;

    OPEN_FILE_EX pb_file;
    memset(&pb_file, 0, sizeof(OPEN_FILE_EX));
    int ret = GkSockRecv(sock, (char *)&pb_file, sizeof(OPEN_FILE_EX));
    if (ret != sizeof(OPEN_FILE_EX)) {
        PRINT_ERR("recv %d, struct size: %d\n", ret, sizeof(OPEN_FILE_EX));
        return -1;
    }

    PRINT_INFO("flag2:%lu id:%lu fileyype:%lu filename:%s start time:%lu-%lu-%lu %lu:%lu:%lu, stop time:%lu-%lu-%lu %lu:%lu:%lu\n",
        pb_file.nFlag2, pb_file.nID, pb_file.nFileType, pb_file.csFileName,
        pb_file.struStartTime.dwYear, pb_file.struStartTime.dwMonth, pb_file.struStartTime.dwDay,
        pb_file.struStartTime.dwHour, pb_file.struStartTime.dwMinute, pb_file.struStartTime.dwSecond,
        pb_file.struStopTime.dwYear, pb_file.struStopTime.dwMonth, pb_file.struStopTime.dwDay,
        pb_file.struStopTime.dwHour, pb_file.struStopTime.dwMinute, pb_file.struStopTime.dwSecond);

    sem_init(&step_sem, 0, 0);


    ret = GkAddPbSock(sock, (int)pb_file.nID);
    if (ret == 0)
        PRINT_INFO("add pb sock success.\n");
    else
        PRINT_INFO("add pb sock failed.\n");

    FilePlay_INFO pb_info;
    memset(&pb_info, 0, sizeof(FilePlay_INFO));

    if (pb_file.nFileType == 1) { // pb by filename
        PRINT_INFO("pb by name. nFileType = %d\n", (int)pb_file.nFileType);
        pPBHandle = Gk_Cms_PlayBackByName(pb_file.csFileName);
    } else if (pb_file.nFileType == 2) {
        PRINT_INFO("pb by time. nFileType = %d\n", (int)pb_file.nFileType);
        pPBHandle = Gk_Cms_PlayBackByTime((AVI_DMS_TIME *)&(pb_file.struStartTime), (AVI_DMS_TIME *)&(pb_file.struStopTime));
    }

	pb_info.dwSize = sizeof(FilePlay_INFO);
    pb_info.dwStream1Height = pPBHandle ? pPBHandle->video_height : 0;
    pb_info.dwStream1Width = pPBHandle ? pPBHandle->video_width : 0;
    pb_info.dwStream1CodecID = 4;
    pb_info.dwAudioChannels = 1;
    pb_info.dwAudioBits = 16;
    pb_info.dwAudioSamples = 8000;
    pb_info.dwWaveFormatTag = 1;
    strcpy(pb_info.csChannelName, "file_name_test");
    pb_info.dwFileSize = get_file_size(pb_file.csFileName);
    pb_info.dwTotalTime = pPBHandle ? cal_pb_all_time(pPBHandle) * 1000 : 0;
    pb_info.dwPlayStamp = pPBHandle ? pPBHandle->index_count : 0;;
    PRINT_INFO("file size:%lu, timelong:%lu, all count :%lu\n", pb_info.dwFileSize, pb_info.dwTotalTime, pb_info.dwPlayStamp);
	
    GkSockSend(sock, (char *)&pb_info, sizeof(FilePlay_INFO), 100);
	if (pPBHandle)
	{
	    cal_pb_gop(pPBHandle);
    	Gk_Pb_SendFrameLoop(sock, pPBHandle);
    	Gk_Cms_PlayBackStop(pPBHandle);
	}

    sem_destroy(&step_sem);
    return 0;
}

#endif

