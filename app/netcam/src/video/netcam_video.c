
#include "cfg_com.h"
#include "media_fifo.h"
#include "common.h"
#include "netcam_api.h"
#include "sdk_enc.h"
#include "sdk_pq_bin.h"
#include "flash_ctl.h"
//#define TEST_TIMER
//#define DEBUG_FPS

#ifdef MODULE_SUPPORT_AF
typedef enum {
    /** normal-mode */
    VIDEO_AF_MODE_NORMAL,
    /** factory-mode */
    VIDEO_AF_MODE_FACTORY,
}VIDEO_AF_ModeEnumT;
#endif

static pthread_rwlock_t video_lock;
static NETCAM_VIDEO_STREAM_Porpety videoPro[4] = {0,0,0,0};
static MEDIABUF_HANDLE writerid[4] = {0,0,0,0};

static int video_frame_index[4] = {0,0,0,0};

static char g_snapshot_filePath[128];


static inline int video_enter_lock(void)
{
	return pthread_rwlock_wrlock(&video_lock);
}

static int video_leave_lock(void)
{
	return pthread_rwlock_unlock(&video_lock);
}

static void video_read_stream_callback(int stream, PS_GK_ENC_BUF_ATTR frameBuf)
{
    GK_NET_FRAME_HEADER header = {0};

	#ifdef DEBUG_FPS
    static int fps[4] = {0,};
    static int video_res[4] ={0,};
    #endif

    header.magic = MAGIC_TEST;
    header.device_type = 0;
    header.frame_size = frameBuf->data_sz;
    header.pts = frameBuf->time_us;

    /* ����ʱ�䣬CMS��Ҫʱ�� */
    struct timeval tv = {0};
    struct timezone tz = {0};
    gettimeofday(&tv, &tz);
    header.sec = tv.tv_sec - tz.tz_minuteswest*60;
    header.usec = tv.tv_usec;

    if(frameBuf->video.keyframe)
        header.frame_type = GK_NET_FRAME_TYPE_I;
    else
        header.frame_type = GK_NET_FRAME_TYPE_P;
    
    if(frameBuf->type == GK_ENC_DATA_H264)// h264			
    {
        header.media_codec_type = 0;// CODEC_ID_H264;
    }           
    else if(frameBuf->type == GK_ENC_DATA_H265)//h265
    {
        header.media_codec_type = 6;// CODEC_ID_H265;
    }

    #ifdef DEBUG_FPS
    if(fps[stream] != frameBuf->video.fps || (video_res[stream] != ((frameBuf->video.width << 16) + frameBuf->video.height)))
    {
        LOG_INFO("FPS change,    from :%d -> %d", fps[stream],frameBuf->video.fps);
        LOG_INFO("Width change,  from :%d -> %d", video_res[stream]>>16,frameBuf->video.width);
        LOG_INFO("Height change, from :%d -> %d", video_res[stream]&0xffff,frameBuf->video.height);
        fps[stream] = frameBuf->video.fps;
        video_res[stream] = ((frameBuf->video.width << 16) + frameBuf->video.height);
    }
    #endif
    header.frame_rate = frameBuf->video.fps;
    header.video_reso = ((frameBuf->video.width << 16) + frameBuf->video.height);
    header.frame_no = (++video_frame_index[stream]);

    mediabuf_write_frame(writerid[stream], frameBuf->data, frameBuf->data_sz, &header);
}

static void video_snapshot_callback(char *data, int size)
{
    FILE *fp = NULL;
    if(data == NULL || size == 0)
    {
        LOG_ERR("parameter error.\n");
        return;
    }

	fp = fopen(g_snapshot_filePath, "w+b");
	if(fp == NULL)
	{
        LOG_ERR("open file error.\n");
        return;
	}
    fwrite(data, 1, size, fp);
    fflush(fp);
    fclose(fp);
}

#ifdef MODULE_SUPPORT_AF
static VIDEO_AF_ModeEnumT video_check_af_mode(PST_GK_AF_OFFSET_PARAM lenParam)
{
    int i = 0;
    ST_GK_AF_OFFSET_PARAM stAFoffset;
    VIDEO_AF_ModeEnumT afMode;

    memset(&stAFoffset, 0, sizeof(stAFoffset));
    int ret = load_info_to_mtd_reserve(MTD_AF_OFFSET, &stAFoffset, sizeof(stAFoffset));
    if ((ret != 0) || (stAFoffset.enable))
    {
        LOG_INFO("[auto focus] enter factory mode.\n");
        afMode = VIDEO_AF_MODE_FACTORY;
    }
    else
    {
        LOG_INFO("[auto focus] enter normal mode.\n");
        memcpy(lenParam, &stAFoffset, sizeof(stAFoffset));
        afMode = VIDEO_AF_MODE_NORMAL;
    }

    return afMode;
}

static void video_get_len_param(s32* pLensParams, u16 length)
{
    int i = 0;
    for(i = 0; i<length; i++)
    {
        printf("LensParams[%d]=%d\n", i, pLensParams[i]);
    }

    ST_GK_AF_OFFSET_PARAM stAFoffset;

    memset(&stAFoffset, 0, sizeof(stAFoffset));
    memcpy(&stAFoffset, pLensParams, sizeof(stAFoffset.offset));
    stAFoffset.length = length;

    int ret = save_info_to_mtd_reserve(MTD_AF_OFFSET, &stAFoffset, sizeof(ST_GK_AF_OFFSET_PARAM));
    if(ret != 0)
    {
        PRINT_ERR("[auto focus] set len auto focus params error.\n");
    }
    else
    {
        PRINT_INFO("[auto focus] set len auto focus params success.\n");
    }

    netcam_sys_operation(NULL, SYSTEM_OPERATION_REBOOT);
}

static void video_set_af_offset_param(PST_GK_AF_OFFSET_PARAM pLensParams)
{
    gk_isp_set_af_offset(pLensParams);
}
#endif

#define PQ_BIN_FILE_PATH    "/opt/resource/pq_bin/pq_bin.bin"
static void video_set_pq_bin(void)
{
    //sdk_pq_bin_operate(PQ_BIN_IMPORT,PQ_BIN_FILE_PATH);	xqq add pq_bin later
}

int video_syn_cfg(int vin_id, int streamID, PS_GK_ENC_STREAM_H264_ATTR pstH264Atrr);

int netcam_video_get_channel_number(void)
{
    int i ;
    int number = 0;
    int max_number = 6;

    for(i = 0; i < max_number; i++)
    {
        if(runVideoCfg.vencStream[i].enable == 1)
        {
            number++;
        }
    }
    return number;
}

void netcam_video_init(void)
{
	int i = 0, ii = 0;
    int vencStream_cnt =  netcam_video_get_channel_number();
    EM_ENC_MAX_RES_MODE encRes;
    #ifdef MODULE_SUPPORT_AF
    VIDEO_AF_ModeEnumT af_mode;
    ST_GK_AF_OFFSET_PARAM af_param;
    #endif
    pthread_rwlock_init(&video_lock, NULL);

	//sdk_sys_init(vencStream_cnt);

    //sdk_isp_init();

    #ifdef MODULE_SUPPORT_AF
    memset(&af_param, 0, sizeof(ST_GK_AF_OFFSET_PARAM));
    af_mode = video_check_af_mode(&af_param);
    if (VIDEO_AF_MODE_FACTORY == af_mode)
    {
        gk_isp_get_af_offset(1, video_get_len_param);
    }
    else
    {
        video_set_af_offset_param(&af_param);
    }
    #endif
	sdk_enc_init();
    printf("vi :%dx%d\n", runVideoCfg.vi[0].width, runVideoCfg.vi[0].height);
    sdk_vi_set_params(&runVideoCfg.vi[0]);
    sdk_vout_set_params(&runVideoCfg.vout[0]);

    //bruce bug
    #if 0
    if(0 == runAudioCfg.rebootMute)
    {   
        LOG_INFO("sdk_ircu_detect, runAudioCfg.rebootMute = %d\n",runAudioCfg.rebootMute);
        sdk_ircu_detect();
    }
    #endif
    
    /* ���ûص��������ڻ�ȡ��Ƶ�� */
    sdk_set_get_video_stream_cb(video_read_stream_callback);

	//MEDIABUF_init();
	encRes = sdk_enc_max_resolution_mode();
	switch(encRes)  //46MB memory for 1920*1080P 8Mbps
	{
        case SENSOR_1920_1080P:
            LOG_INFO("SENSOR_1920_1080P\n");
			#ifdef MODULE_SUPPORT_MOJING
			mediabuf_init(0, 1500 * 1024);
            mediabuf_init(1, 900 * 1024);
			#else
			mediabuf_init(0, 1500 * 1024);
            mediabuf_init(1, 200 * 1024);
			#endif
            if (runVideoCfg.vencStream[2].enable)
                mediabuf_init(2, 112 * 1024);
            break;
        case SENSOR_1280_720P:
            LOG_INFO("SENSOR_1280_720P\n");
        case SENSOR_1280_960P:
            LOG_INFO("SENSOR_1280_960P\n");
            mediabuf_init(0, 650 * 1024);
            mediabuf_init(1, 300 * 1024);
            //mediabuf_init(2, 150 * 1024);
            break;
        default:
            LOG_ERR("Not support this resolution :%d",encRes);
    }

    //netcam_audio_add_writers();	xqq

    //VideoCfgPrint();

	// video input
	for(ii = 0; ii < vencStream_cnt; ++ii)
    {
		// video h264 encode
        writerid[ii] = mediabuf_add_writer(ii);
        if(writerid[ii] == NULL)
        {
            LOG_ERR("Create media writer error\n");
        }
        
        videoPro[ii].bufId = ii;
        videoPro[ii].streamId = ii;
        videoPro[ii].maxStreamNum = 3;

		// success to new a mediabuf
		ST_GK_ENC_STREAM_H264_ATTR vencStream_attr={0};
		//int venc_id = 0;
		//int vencStream_rc_mode = GK_ENC_H264_RC_MODE_CBR;
		
		sprintf(vencStream_attr.name, "stream%d", ii+1);

		// start video vencStream
        vencStream_attr.enctype =  runVideoCfg.vencStream[ii].enctype;
		vencStream_attr.width =  runVideoCfg.vencStream[ii].h264Conf.width;
		vencStream_attr.height = runVideoCfg.vencStream[ii].h264Conf.height;
		vencStream_attr.fps = runVideoCfg.vencStream[ii].h264Conf.fps;
		vencStream_attr.gop = runVideoCfg.vencStream[ii].h264Conf.gop;
		// FIXME:
		if(vencStream_attr.gop > 100){
			vencStream_attr.gop = 100;
		}
		vencStream_attr.gop = 2*runVideoCfg.vencStream[ii].h264Conf.fps;
		//vencStream_attr.gop = runVideoCfg.vencStream[ii].h264Conf.gop;

		vencStream_attr.profile = runVideoCfg.vencStream[ii].h264Conf.profile;
		vencStream_attr.rc_mode = runVideoCfg.vencStream[ii].h264Conf.rc_mode;
		vencStream_attr.bps = runVideoCfg.vencStream[ii].h264Conf.bps;
		vencStream_attr.quality = runVideoCfg.vencStream[ii].h264Conf.quality;
        vencStream_attr.re_rec_mode = runVideoCfg.vencStream[ii].h264Conf.re_rec_mode;
            
        ST_VENC_H264QP_CONF qpConf = {0};
        qpConf.qpMinOnI = runVideoCfg.vencStream[ii].h264QpConf.qpMinOnI;
        qpConf.qpMaxOnI = runVideoCfg.vencStream[ii].h264QpConf.qpMaxOnI;
        qpConf.qpMinOnP = runVideoCfg.vencStream[ii].h264QpConf.qpMinOnP;
        qpConf.qpMaxOnP = runVideoCfg.vencStream[ii].h264QpConf.qpMaxOnP;
        qpConf.qpIWeight = runVideoCfg.vencStream[ii].h264QpConf.qpIWeight;
        qpConf.qpPWeight = runVideoCfg.vencStream[ii].h264QpConf.qpPWeight;
        qpConf.adaptQp = runVideoCfg.vencStream[ii].h264QpConf.adaptQp;        
        //PRINT_INFO("cfg stream%d re_rec_mode:%d\n", ii, vencStream_h264_attr.re_rec_mode);
		PRINT_INFO("%s, resolution: %dx%d, fps:%d\n", vencStream_attr.name, vencStream_attr.width, vencStream_attr.height, vencStream_attr.fps);

		sdk_enc_create_stream_h264(i, ii, &vencStream_attr, &qpConf);
		sdk_enc_enable_stream_h264(i, ii, true);
    }
    //netcam_timer_add_task(video_set_pq_bin, NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_TRUE);   xqq pq_bin
    /* �����̣߳���ȡ�����������Ƶ������ */
	//sdk_video_enc_start();  xqq
    //printf("sdk_video_enc_start finished.\n");

    if (runVideoCfg.vencStream[0].h264Conf.width >= 1440)
    {
        LOG_INFO("enable h26x\n");
        //sdk_set_h26x_smart(1, 4);
    }
#if 0
    if (runAudioCfg.mode > 0) // 0 disable; 1 input; 2- input&output
    {
        #if 0
        if((runAudioCfg.type > 0) && (runAudioCfg.type <= 2))
            sdk_enc_create_stream_audio(0,  runAudioCfg.type);
        else
            sdk_enc_create_stream_audio(0,  0);
        #endif
    	//������Ƶ��ȡ�߳�
        sdk_audio_enc_start();
    }
#endif  //xqq
	netcam_video_web_stream_init(videoPro, vencStream_cnt);

}

void netcam_video_convert_channel(int id, int *vin_id, int *stream_id)
{
	if(id < 100){
		*vin_id = 0;
	}else{
		*vin_id = id / 100 - 1;
	}
	*stream_id = id % 100 - 1;
}

int netcam_video_get_map(GK_CFG_MAP video2Array[][MAX_ITEM_NUM], int maxStreamSize, int maxItemSize)
{
	int i = 0, j = 0;
	int streamNum = 0;
	int itemNum = 0;
	if(video2Array == NULL)
	{
		PRINT_ERR("Invalid paramters.\n");
		return -1;
	}
    video_enter_lock();
	if(maxStreamSize > MAX_VENC_STREAM_NUM)
		streamNum = MAX_VENC_STREAM_NUM;
	else
		streamNum = maxStreamSize;

	for(i = 0; i < streamNum; i++)
	{
		j = 0;
		while(videoMap[i][j].stringName != NULL && j < MAX_VENC_ITEM_NUM)j++;
		if(maxItemSize > j)
			itemNum = j;
		else
			itemNum = maxItemSize;
		memcpy(video2Array[i], videoMap[i], itemNum*sizeof(GK_CFG_MAP));
	}

    video_leave_lock();
	return 0;

}

int netcam_video_get(int vin_id, int stream_id ,PS_GK_ENC_STREAM_H264_ATTR h264Attr)
{
    int ret = 0;
    video_enter_lock();
    if(h264Attr == NULL)
    {
		PRINT_ERR("Invalid H264 attr.");
		goto out;
    }

    ret = video_syn_cfg(0, stream_id, h264Attr);
    if(ret != 0)
        goto out;

    video_leave_lock();
    return 0;

out:
    video_leave_lock();
    return ret;
}

int netcam_video_set(int vin_id, int stream_id ,PS_GK_ENC_STREAM_H264_ATTR h264Attr)
{
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR curAttr;
    video_enter_lock();

	if((h264Attr->bps > 8192) || (h264Attr->bps < 5))
	{
        PRINT_ERR("invalid bitrate.\n");
        goto out;
	}
    #if 0 
	/*gop��СΪfps��2�������Ϊ50*/
	if(h264Attr->gop > 50 || h264Attr->gop < 5 || h264Attr->gop < 2*h264Attr->fps)
		h264Attr->gop = 2*h264Attr->fps;
	if(h264Attr->gop > 50)
		h264Attr->gop = 50;
	else if(h264Attr->gop < 5)
		h264Attr->gop = 5;
	#else
	h264Attr->gop = 2*h264Attr->fps;
    #endif

    if((stream_id < 0 && stream_id >= netcam_video_get_channel_number()) || vin_id != 0)
    {
        PRINT_ERR("stream id error :%d,vin_id:%d\n", stream_id, vin_id);
        goto out;
    }

   // ret = sdk_enc_get_stream_h264(vin_id, stream_id, &curAttr);
   // if(ret != 0)
   // {
   //     PRINT_ERR("fail to get H264 attr.\n");
   //     goto out;
   // }
   // if(memcmp(&curAttr,h264Attr,sizeof(ST_GK_ENC_STREAM_H264_ATTR)) == 0)
   // {
   //     video_leave_lock();
   //     return 0;
   // }
   // ret = sdk_enc_set_stream_h264(vin_id,stream_id,h264Attr);
   // if(ret != 0)
   // {
   //     PRINT_ERR("fail to set H264 attr.\n");
   //     goto out;
   // }
	
    //ret = video_syn_cfg(0, stream_id, NULL);
    ret = video_syn_cfg_x(stream_id, h264Attr);
    if(ret != 0)
	{
		PRINT_ERR("fail to syn video paramters.\n");
		goto out;
	}
    //reload osd, the stream_width and stream_height maybe update.
    
#ifdef MODULE_SUPPORT_MOJING_V4
    osd_reload_all();
    netcam_osd_update_title();
    mojing_osd_compositions_show();
#else
    osd_reload_all();
#endif

    video_leave_lock();
    return 0;

out:
    video_leave_lock();
    return -1;
}

int netcam_video_snapshot(int width, int height, char *filePath, EM_GK_ENC_SNAPSHOT_QUALITY quality)
{
    video_enter_lock();
    int ret = 0;

	if(filePath == NULL || filePath[0] == '\0')
	{
        PRINT_ERR("snapshot: file path not exit.\n");
        goto error_out;
	}

    strcpy(g_snapshot_filePath, filePath);
    ret = sdk_enc_snapshot(0, quality, width, height, video_snapshot_callback);
    if(ret != 0)
    {
        PRINT_ERR("snapshot: faild.\n");
    }

    video_leave_lock();
    return 0;

error_out:
    video_leave_lock();
    return ret;
}


int netcam_video_get_all_channel_par(GK_NET_VIDEO_CFG *pstConfigs)
{
    int ret = 0;
    video_enter_lock();
    memcpy(pstConfigs, &runVideoCfg, sizeof(GK_NET_VIDEO_CFG));
    video_leave_lock();
    return ret;
}

void netcam_video_cfg_save(void)
{
	if(0 == video_enter_lock())
    {
		LOG_INFO("netcam_cfg_video_save!!");
		VideoCfgSave();
		video_leave_lock();
	}
}

void netcam_video_exit()
{
    static int testexit = 0;
    if(testexit == 1)
    {
        return;
    }
    LOG_WARNING("========= exit netcam\n");
    netcam_timer_destroy();
	netcam_net_exit();
    LOG_WARNING("=========  exit_SDK_video\n");
    LOG_WARNING("=========  exit_SDK_audio\n");
	netcam_osd_deinit();
    sdk_vin_destroy();
	netcam_md_destroy();
    sdk_enc_destroy();
    #ifdef MODULE_SUPPORT_WATCHDOG
    LOG_WARNING("=========  exit_SDK_watchdog\n");
    netcam_watchdog_exit();
    LOG_WARNING("=========  exit_SDK_watchdog end\n");
    #endif
}

char *netcam_video_get_cfg_json_string(int stream_id)
{
    char *buf;
    video_enter_lock();
    buf = videoCfgLoadStreamJson(stream_id);
    video_leave_lock();
    return buf;
}

int netcam_video_get_actual_fps(int stream_id)
{
    int ret = 0;
    ret = sdk_enc_get_stream_fps(0, stream_id);
    return ret;
}

//���һ��ʱ��󣬵��øú����������Ƿ���������
int netcam_video_check_alive()
{
    static int last_frame_index = 0;

    if (last_frame_index == video_frame_index[0])
        return -1;//֡��δ�ı䣬�����쳣

    last_frame_index = video_frame_index[0];
    return 0;//֡���ı䣬��������
}

int video_syn_cfg(int vin_id, int streamID, PS_GK_ENC_STREAM_H264_ATTR pstH264Atrr)
{
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Attr;
    memset(&stH264Attr, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));

    ret = sdk_enc_get_stream_h264(vin_id, streamID, &stH264Attr);
    if(ret != 0)
    {
        return -1;
    }
    #if 0
    printf("\n\n\n\n\n\n%d\n%d\n%d\n%d\n%d\n%d\n\n\n\n\n",
        stH264Attr.width,
        stH264Attr.height,
        stH264Attr.fps,
        stH264Attr.bps,
        stH264Attr.quality,
        stH264Attr.gop);
    #endif
    runVideoCfg.vencStream[streamID].h264Conf.width   = stH264Attr.width;
    runVideoCfg.vencStream[streamID].h264Conf.height  = stH264Attr.height;
    runVideoCfg.vencStream[streamID].h264Conf.fps     = stH264Attr.fps;
    runVideoCfg.vencStream[streamID].h264Conf.bps     = stH264Attr.bps;
    runVideoCfg.vencStream[streamID].h264Conf.profile = stH264Attr.profile;
    runVideoCfg.vencStream[streamID].h264Conf.quality = stH264Attr.quality;
    runVideoCfg.vencStream[streamID].h264Conf.gop     = stH264Attr.gop;
    runVideoCfg.vencStream[streamID].h264Conf.rc_mode = stH264Attr.rc_mode;
    runVideoCfg.vencStream[streamID].enctype          = stH264Attr.enctype;
    //runVideoCfg.vencStream[streamID].h264Conf.idrInterval =
	if(pstH264Atrr != NULL)
		memcpy(pstH264Atrr, &stH264Attr, sizeof(ST_GK_ENC_STREAM_H264_ATTR));

    return ret;
}

int video_syn_cfg_x(int streamID,  ST_GK_ENC_STREAM_H264_ATTR *h264Attr)
{
    int ret = 0;

    runVideoCfg.vencStream[streamID].h264Conf.width   = h264Attr->width;
    runVideoCfg.vencStream[streamID].h264Conf.height  = h264Attr->height;
    runVideoCfg.vencStream[streamID].h264Conf.fps     = h264Attr->fps;
    runVideoCfg.vencStream[streamID].h264Conf.bps     = h264Attr->bps;
    runVideoCfg.vencStream[streamID].h264Conf.profile = h264Attr->profile;
    runVideoCfg.vencStream[streamID].h264Conf.quality = h264Attr->quality;
    runVideoCfg.vencStream[streamID].h264Conf.gop     = h264Attr->gop;
    runVideoCfg.vencStream[streamID].h264Conf.rc_mode = h264Attr->rc_mode;
    runVideoCfg.vencStream[streamID].enctype          = h264Attr->enctype;

    return ret;

}

int netcam_video_set_stream_name(int streamID, char *name)
{
    if(NULL == name)
        return -1;
	netcam_osd_text_copy(runVideoCfg.vencStream[streamID].h264Conf.name, name, sizeof(runVideoCfg.vencStream[streamID].h264Conf.name));
    netcam_osd_set_title(streamID, runVideoCfg.vencStream[streamID].h264Conf.name);
    //strncpy(runVideoCfg.vencStream[streamID].h264Conf.name, name, sizeof(runVideoCfg.vencStream[streamID].h264Conf.name)-1);
    return 0;
}

int netcam_adjust_bps(int stream_id, int bps)
{
    return gk_adjust_bps(stream_id, bps);
}

int netcam_adjust_fps(int stream_id, int fps)
{
    return gk_adjust_fps(stream_id, fps);
}

int netcam_venc_set_h264_qp_config(int stream_id)
{
    ST_VENC_H264QP_CONF stH264QpConfig = {0};

    if ((stream_id == 0) || (stream_id == 1)) {
    	stH264QpConfig.streamId = stream_id;
    	//stH264QpConfig.qpMinOnI = runVideoCfg.vencStream[stream_id].h264QpConf.qpMinOnI;
    	//stH264QpConfig.qpMaxOnI = runVideoCfg.vencStream[stream_id].h264QpConf.qpMaxOnI;
    	//stH264QpConfig.qpMinOnP = runVideoCfg.vencStream[stream_id].h264QpConf.qpMinOnP;
    	//stH264QpConfig.qpMaxOnP = runVideoCfg.vencStream[stream_id].h264QpConf.qpMaxOnP;
    	stH264QpConfig.qpIWeight = runVideoCfg.vencStream[stream_id].h264QpConf.qpIWeight;
    	stH264QpConfig.qpPWeight = runVideoCfg.vencStream[stream_id].h264QpConf.qpPWeight;
    	stH264QpConfig.adaptQp= runVideoCfg.vencStream[stream_id].h264QpConf.adaptQp;
    } else {
        LOG_ERR("param error. stream_id = %d\n", stream_id);
        return -1;
    }

    return gk_venc_set_h264_qp_config(&stH264QpConfig);
}

int netcam_video_force_i_frame(int stream_id)
{
	return sdk_enc_request_stream_h264_keyframe(0,stream_id);
}

PIC_SIZE_E netcam_video_get_pic_format(int width, int height)
{
    PIC_SIZE_E picFormat;

    if ((width==352) && (height==288))
        picFormat = PIC_CIF;   /* 352 * 288 */
    else if ((width==640) && (height==360))
        picFormat = PIC_360P;   /* 640 * 360 */
    else if ((width==640) && (height==480))
        picFormat = PIC_VGA;   /* 640 * 480 */
    else if ((width==640) && (height==360))
        picFormat = PIC_640x360;   /* 640 * 360 */
    else if ((width==720) && (height==576))
        picFormat = PIC_D1_PAL;  /* 720 * 576 */
    else if ((width==720) && (height==480))
        picFormat = PIC_D1_NTSC;   /* 720 * 480 */
    else if ((width==1280) && (height==720))
        picFormat = PIC_720P;   /* 1280 * 720 */
    else if ((width==1920) && (height==1080))
        picFormat = PIC_1080P;  /* 1920 * 1080 */
    else if ((width==2304) && (height==1296))
        picFormat = PIC_2304x1296;  /* 2304 * 1296 */
    else if ((width==2560) && (height==1440))
        picFormat = PIC_2560x1440;
    else if ((width==2592) && (height==1520))
        picFormat = PIC_2592x1520;
    else if ((width==2592) && (height==1944))
        picFormat = PIC_2592x1944;
    else if ((width==2592) && (height==1536))
        picFormat = PIC_2592x1536;
    else if ((width==2688) && (height==1520))
        picFormat = PIC_2688x1520;
    else if ((width==2716) && (height==1524))
        picFormat = PIC_2716x1524;
    else if ((width==3840) && (height==2160))
        picFormat = PIC_3840x2160;
    else if ((width==3000) && (height==3000))
        picFormat = PIC_3000x3000;
    else if ((width==4000) && (height==3000))
        picFormat = PIC_4000x3000;
    else if ((width==4096) && (height==2160))
        picFormat = PIC_4096x2160;
    else if ((width==7680) && (height==4320))
        picFormat = PIC_7680x4320;
    else if ((width==3840) && (height==8640))
        picFormat = PIC_3840x8640;
    else if ((width==2688) && (height==1536))
        picFormat = PIC_2688x1536;
    else if ((width==2688) && (height==1944))
        picFormat = PIC_2688x1944;
    else
        picFormat = PIC_BUTT;

    return picFormat;
}


