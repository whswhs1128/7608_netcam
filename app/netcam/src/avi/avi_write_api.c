/*!
*****************************************************************************
** FileName     : avi_common_rec.c
**
** Description  : record to avi.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create       : 2015-8-11, create
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "avi_common.h"
#include "avi_utility.h"
#include "mmc_api.h"
#include "cfg_all.h"

#define CAL_TIME_REC 0

static int avi_flag_new = 1;
static int avi_flag_open = 0;

extern int grd_sd_is_formated;
extern char *data_array;
extern int data_array_index;
extern int data_array_pos;
extern int rec_buf_size;

static char avi_check_file_name[128];
static char avi_check_file_number = 0;
static pthread_mutex_t rec_check_mutex_t = PTHREAD_MUTEX_INITIALIZER;

void * avi_record_check_file(void *pData)
{
	char check_file[128];
	strcpy(check_file,avi_check_file_name);
    PRINT_INFO("avi_record_check_file:%s \n", check_file);
	
	sdk_sys_thread_set_name("avi_check file");
	pthread_detach(pthread_self());
    avi_check_file_index(check_file);
	pthread_mutex_lock(&rec_check_mutex_t);
	avi_check_file_number--;
	pthread_mutex_unlock(&rec_check_mutex_t);
	return NULL;
}

/**********************************************************************
函数描述：向AVI文件中写入avi头信息
入口参数：AviFile *avi_file:  AVI文件的控制句柄
          AviInitParam *param: 传入的参数信息
返回值：   0: 正常返回
          <0: 错误返回
**********************************************************************/
static int avi_write_header(AviFile *avi_file, AviInitParam *param)
{
    int bitrate, framerate, width, height, ms_per_frame;
    int ret;
    int type, fcctype;
    int a_chans, a_rate, a_bits, a_enc_type, samplesize;

    if( (!avi_file) || (!param))
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }

    //init param
    bitrate = param->bps;
    framerate = param->fps;
    //ms_per_frame = 1000000./framerate + 0.5;
    ms_per_frame = 1000000./framerate;
    width = param->width;
    height = param->height;

    switch(param->codec_type)
    {
        case ENC_TYPE_H264: type = AVI_TYPE_H264; break;
        case ENC_TYPE_MPEG: type = AVI_TYPE_MPEG4; break;
        case ENC_TYPE_MJPEG: type = AVI_TYPE_MJPEG; break;
        case ENC_TYPE_H265: type = AVI_TYPE_H265; break;
        default: type = AVI_TYPE_H264; break;
    }
    switch(type)
    {
        case AVI_TYPE_H264:
        case AVI_TYPE_MPEG4:
        case AVI_TYPE_MJPEG:
        case ENC_TYPE_H265:
            fcctype = MAKE_FOURCC('v','i','d','s');
            break;
        default:  break;
    }

    #if 0
    //audio param
    a_chans = 1; /* Audio channels, 0 for no audio */
    a_rate = 8000; /* Rate in Hz */
    a_bits = 8; /* bits per audio sample */
    a_enc_type = 0;
    #else
    a_chans = param->a_chans;
    a_rate = param->a_rate;
    a_bits = param->a_bits;
    a_enc_type = param->a_enc_type;
    #endif
    samplesize = ((a_bits + 7) / 8) * a_chans;
    if(samplesize == 0)
        samplesize=1; /* avoid possible zero divisions */
    avi_file->samplesize = samplesize;

    AviHeader avi_header;
    memset(&avi_header, 0, sizeof(AviHeader));

    //write RIFF_AVI
    avi_header.fcc_riff = MAKE_FOURCC('R','I','F','F');
    avi_header.avi_len = 0; //should be updated
    avi_header.fcc_avi = MAKE_FOURCC('A','V','I',' ');


    //write LIST_hdrl
    avi_header.fcc_list_hdrl = MAKE_FOURCC('L','I','S','T');
    avi_header.hdrl_len = (int)&(((AviHeader *)0)->fcc_list_movi) - (int)&(((AviHeader *)0)->fcc_hdrl);
    avi_header.fcc_hdrl = MAKE_FOURCC('h','d','r','l');


    //write AviMainHeader
    avi_header.main_header.fcc = MAKE_FOURCC('a','v','i','h');
    avi_header.main_header.cb = 56; //sizeof(AviMainHeader) - 8 = 64 -8 =56
    avi_header.main_header.dwMicroSecPerFrame = ms_per_frame;
    avi_header.main_header.dwMaxBytesPerSec = bitrate / 8;
    avi_header.main_header.dwPaddingGranularity = 0;
    avi_header.main_header.dwFlags = AVIF_HASINDEX | AVIF_WASCAPTUREFILE;
    avi_header.main_header.dwTotalFrames = avi_file->video_count;     //<==need update, framecount, main_header_start+24
    avi_header.main_header.dwInitialFrames = 0;
    avi_header.main_header.dwStreams = 2;
    avi_header.main_header.dwSuggestedBufferSize = width*height*3; //1000000
    avi_header.main_header.dwWidth = width;
    avi_header.main_header.dwHeight = height;

    /************ video start *******************/
    //write LIST_strl
    avi_header.fcc_list_strl_video = MAKE_FOURCC('L','I','S','T');
    avi_header.size_strl_video = (int)&(((AviHeader *)0)->fcc_list_strl_audio) - (int)&(((AviHeader *)0)->fcc_strl_video);
    avi_header.fcc_strl_video = MAKE_FOURCC('s','t','r','l');

    //write AviStreamHeader
    avi_header.video_header.fcc = MAKE_FOURCC('s','t','r','h');
    avi_header.video_header.cb = 56; //sizeof(AviStreamHeader)-8 = 64-8 =56
    avi_header.video_header.fccType = fcctype;  //vids
    avi_header.video_header.fccHandler = type; //MAKE_FOURCC('H','2','6','4');
    avi_header.video_header.dwFlags = 0x0;
    avi_header.video_header.wPriority = 0;
    avi_header.video_header.wLanguage = 0;
    avi_header.video_header.dwInitialFrames = 0;
    avi_header.video_header.dwScale = ms_per_frame;
    avi_header.video_header.dwRate = 1000000;
    avi_header.video_header.dwStart = 0;
    avi_header.video_header.dwLength = avi_file->video_count;  //<==need update, framecount, stream_header_start+52
    avi_header.video_header.dwSuggestedBufferSize = width*height*3; //1024*1024
    avi_header.video_header.dwQuality = -1;
    avi_header.video_header.dwSampleSize = width*height;
    avi_header.video_header.rcFrame.left = 0;
    avi_header.video_header.rcFrame.top = 0;
    avi_header.video_header.rcFrame.right = width;
    avi_header.video_header.rcFrame.bottom = height;

    //write strf+size+AviStreamFormat
    avi_header.fcc_strf_video = MAKE_FOURCC('s','t','r','f');
    avi_header.size_video_format = 40; //sizeof(BitMapInfoHeader)
    avi_header.bmiHeader.biSize = 40;
    avi_header.bmiHeader.biWidth = width;
    avi_header.bmiHeader.biHeight = height;
    avi_header.bmiHeader.biPlanes = 1;
    avi_header.bmiHeader.biBitCount = 24;
    avi_header.bmiHeader.biCompression = type; //MAKE_FOURCC('H','2','6','4'); H264,h264
    avi_header.bmiHeader.biSizeImage = width*height; //width*height
    avi_header.bmiHeader.biXPelsPerMeter = 0;
    avi_header.bmiHeader.biYPelsPerMeter = 0;
    avi_header.bmiHeader.biClrUsed = 0;
    avi_header.bmiHeader.biClrImportant = 0;
    /************ video end *******************/

    /************ audio start *******************/
    //write LIST_strl
    avi_header.fcc_list_strl_audio = MAKE_FOURCC('L','I','S','T');
    avi_header.size_strl_audio = (int)&(((AviHeader *)0)->fcc_list_movi) - (int)&(((AviHeader *)0)->fcc_strl_audio);
    avi_header.fcc_strl_audio = MAKE_FOURCC('s','t','r','l');


    //write AviStreamHeader
    avi_header.audio_header.fcc = MAKE_FOURCC('s','t','r','h');
    avi_header.audio_header.cb = 56; //sizeof(AviStreamHeader)-8 = 64-8 =56
    avi_header.audio_header.fccType = MAKE_FOURCC('a','u','d','s');  //auds
    avi_header.audio_header.fccHandler = MAKE_FOURCC('G','7','1','1');
    //avi_header.audio_header.fccHandler = MAKE_FOURCC('\0','\0','\0','\0');
    avi_header.audio_header.dwFlags = 0x0;
    avi_header.audio_header.wPriority = 0;
    avi_header.audio_header.wLanguage = 0;
    avi_header.audio_header.dwInitialFrames = 0;

    avi_header.audio_header.dwScale = samplesize; // a_bits = 16, 则2 ; a_bits = 8, 则1
    avi_header.audio_header.dwRate = samplesize * a_rate; // 2 * 8000
    avi_header.audio_header.dwStart = 0;
    /*
      dwLength 指定这个流的长度。
      单位由流的头信息中的dwRate 和dwScale 来 确定（即其单位是dwRate/dwScale）。
      （对于视频流，dwLength就是流包含的总 帧数；
      对于音频流，dwLength就是包含的block的数量，block是音频解码器能处理的原子单位）。
      dwLength/(dwRate/dwScale)，即dwLength * dwScale / dwRate，可以得到流的总时长。
    */
    avi_header.audio_header.dwLength = avi_file->audio_bytes / samplesize;
    avi_header.audio_header.dwSuggestedBufferSize = 0; //1024*1024
    avi_header.audio_header.dwQuality = -1;
    avi_header.audio_header.dwSampleSize = samplesize;
    avi_header.audio_header.rcFrame.left = 0;
    avi_header.audio_header.rcFrame.top = 0;
    avi_header.audio_header.rcFrame.right = 0;
    avi_header.audio_header.rcFrame.bottom = 0;

    //write strf+size+AviStreamFormat
    avi_header.fcc_strf_audio = MAKE_FOURCC('s','t','r','f');
    avi_header.size_audio_format = 16; //sizeof(AudioFormateX)
    //G.711音频编码分a-law和u-law，其中a-law 的tAuds.formatTag=0x06,u-law 的tAuds.formatTag=0x07
    if (a_enc_type == 0)
        avi_header.audio_format.wFormatTag = 6;
    else if (a_enc_type == 1)
        avi_header.audio_format.wFormatTag = 7;
    else if (a_enc_type == 2)
        avi_header.audio_format.wFormatTag = 1; //pcm
    else if (a_enc_type == 3)
        avi_header.audio_format.wFormatTag = 2; //adpcm
    avi_header.audio_format.nChannels = a_chans;  //声道数,1
    avi_header.audio_format.nSamplesPerSec = a_rate; //采样率,8000
    avi_header.audio_format.nAvgBytesPerSec = a_rate * samplesize; //WAVE声音中每秒的数据量
    avi_header.audio_format.nBlockAlign = samplesize;  //数据块的对齐标志
    avi_header.audio_format.wBitsPerSample = a_bits; //8, if pcm: 32
	//avi_write_uint16( a,  18);	//cbSize =?18
	//avi_write_uint16( a,  2);
    /************ audio end *******************/
    #if 1
    PRINT_INFO("write avi header: chans:%d rate:%d bit:%d \n", a_chans, a_rate, a_bits);
    #endif

    //write LIST_movi
    avi_header.fcc_list_movi = MAKE_FOURCC('L','I','S','T');
    avi_header.movi_len = 0;
    avi_header.fcc_movi = MAKE_FOURCC('m','o','v','i');

    avi_file->data_offset = sizeof(AviHeader);

    //write avi header
    ret = fwrite(&avi_header, sizeof(AviHeader), 1, avi_file->file);
    if(ret != 1)
    {
        PRINT_ERR_MSG("write avi header failed.\n");
		fclose(avi_file->file);
		avi_file->file = NULL;
        return -1;
    }

    fflush(avi_file->file);

    return 0;
}

static int update_avi_header(AviFile *avi_file)
{
    int ret;
    int avi_len = ftell(avi_file->file) - 8;
    int movi_len = avi_file->data_offset - sizeof(AviHeader) + 4;

    //PRINT_INFO("index offset = %d, avi_len = %d, movi_len = %d.\n", avi_file->data_offset, avi_len, movi_len);

    fseek(avi_file->file, 4, SEEK_SET);
    ret = write_int32(avi_file->file, avi_len);
    if(ret != 1)
    {
        PRINT_ERR_MSG("write avi len failed.\n");
        return -1;
    }

    fseek(avi_file->file, sizeof(AviHeader) - 8, SEEK_SET);
    ret = write_int32(avi_file->file, movi_len);
    if(ret != 1)
    {
        PRINT_ERR_MSG("write movi len failed.\n");
        return -1;
    }

    fseek(avi_file->file, (int)&(((AviHeader *)0)->main_header.dwTotalFrames), SEEK_SET);
	ret = write_int32(avi_file->file, avi_file->video_count);
    if(ret != 1)
    {
        PRINT_ERR_MSG("write total frames count failed.\n");
        return -1;
    }

	fseek(avi_file->file, (int)&(((AviHeader *)0)->video_header.dwLength), SEEK_SET);
	ret = write_int32(avi_file->file, avi_file->video_count);
    if(ret != 1)
    {
        PRINT_ERR_MSG("write video frames count failed.\n");
        return -1;
    }

	if(!avi_file->samplesize)
	{
		avi_file->samplesize = ((runAudioCfg.sampleBitWidth + 7) / 8) * runAudioCfg.chans;

		if(!avi_file->samplesize)
			avi_file->samplesize = 1;
	}


    int audio_count = avi_file->audio_bytes / avi_file->samplesize;
    #if 1
    if ( (avi_file->audio_type == 0) || (avi_file->audio_type == 1) )
    {
        audio_count = audio_count * 2;
        PRINT_INFO("a or u, audio_count = %d. after adjust, audio_count = %d \n", (avi_file->audio_bytes / avi_file->samplesize), audio_count);
    }
    else if (avi_file->audio_type == 2)
        PRINT_INFO("pcm, audio_count = %d\n", audio_count);
    else if (avi_file->audio_type == 3)
        audio_count = audio_count * 4;
    #endif

	fseek(avi_file->file, (int)&(((AviHeader *)0)->audio_header.dwLength), SEEK_SET);
	ret = write_int32(avi_file->file, audio_count);
    if(ret != 1)
    {
        PRINT_ERR_MSG("write audio block count failed.\n");
        return -1;
    }

    fflush(avi_file->file);
    return 0;
}

static int record_start_check_file(char *filename)
{
	pthread_t pthid = 0;
	int ret;
	strcpy(avi_check_file_name, filename);
	ret = pthread_create(&pthid, NULL, avi_record_check_file, NULL);
	if(ret == 0)
	{
		pthread_mutex_lock(&rec_check_mutex_t);
		avi_check_file_number++;
		pthread_mutex_unlock(&rec_check_mutex_t);
	}
	return 0;
}


/**********************************************************************
函数描述：向AVI文件中加入avi帧数据的index信息
入口参数：AviFile *avi_file:  AVI文件的控制句柄
返回值：   0: 正常返回
          <0: 错误返回
**********************************************************************/
int avi_add_index(AviFile *avi_file)
{
    int ret;

    if(!avi_file || !(avi_file->file))
    {
        PRINT_ERR_MSG("parameter is error.\n");
        return -1;
    }

    //check the sd card is mounted.
    if(grd_sd_is_mount() == 0)
    {
        PRINT_ERR_MSG("sd card is not mounted.\n");
        return -1;
    }

    //test
    //int tmp32 = ftell(avi_file->file);
    //PRINT_INFO("add index,index offset = %d. data offset = %d\n", tmp32, avi_file->data_offset);

    //add index
    ret = write_fourcc(avi_file->file, MAKE_FOURCC('i','d','x','1'));
    if(ret != 1)
    {
        PRINT_ERR_MSG("write avi index fcc failed.\n");
        return -1;
    }
    ret = write_int32(avi_file->file, 16*avi_file->index_count);
    if(ret != 1)
    {
        PRINT_ERR_MSG("write avi index size failed.\n");
        return -1;
    }
    PRINT_INFO("add index, all count = %d. video count = %d\n", avi_file->index_count, avi_file->video_count);
    ret = fwrite(avi_file->idx_array, avi_file->index_count*16, 1, avi_file->file);  
    if(ret != 1)    
    {
        PRINT_ERR_MSG("write avi index array failed.\n");
        return -1;
    }

    //updata avi header
    ret = update_avi_header(avi_file);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call update_avi_header failed.\n");
        return -1;
    }

    return 0;
}


/**********************************************************************
函数描述：检查SD卡存储空间，创建文件，写AVI头信息
入口参数：AviFile *avi_file, AVI文件的控制句柄
          AviInitParam *param, 传入的参数信息
返回值： 0: 正常返回
        <0: 错误返回
**********************************************************************/
int avi_record_open(AviFile *avi_file, AviInitParam *param)
{
    int ret = 0;
    char date[10];
    char date_path[128];
    u64t all_size;
    char time_start[20];
	int t1,t2;
    if( (!avi_file) || (!param))
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }

    //init avi_file
    avi_file->data_offset = 0;
    avi_file->video_count = 0;
    avi_file->audio_bytes = 0;
    avi_file->index_count = 0;
    avi_file->idx_array_count = 0;
	
	avi_file->video_fps = param->fps;
	
    //调整大小，从MB到Byte
    if(RECORD_IS_FIXED_SIZE(param->mode))
    {
        all_size = param->size_m * 1024 * 1024; // M to Byte

        //估算多少帧数据
        avi_file->idx_array_count = all_size  * (param->fps) / (param->bps / 8) * 4;
        PRINT_INFO("fps = %d, rec size = %d Mb, idx_array_count = %d\n",
               param->fps, param->size_m, avi_file->idx_array_count);
    }
    else if(RECORD_IS_FIXED_DURATION(param->mode))
    {
        #if 1
		
#ifndef MODULE_SUPPORT_GB28181
        all_size = (param->bps / 8) * (param->duration) * 6 * 13 ;  //add space to 130%
#else
		if(param->duration == 1)
			all_size = (param->bps / 8) * (param->duration) * 6 * 13 * runRecordCfg.recordLen; //motion detect may expand file size.
		else
			all_size = (param->bps / 8) * (param->duration) * 6 * 13;
#endif

        param->size_m = (int) (all_size/1024/1024);
        #else
        switch(param->ch_num)
        {
            case 0:
                param->size_m = 70 * param->duration;
                break;
            case 1:
                param->size_m = 30 * param->duration;
                break;
            case 2:
                param->size_m = 20 * param->duration;
                break;
            case 3:
                param->size_m = 60 * param->duration;
                break;
            default:
                param->size_m = 70 * param->duration;
                break;
        }
        #endif

        //估算多少帧数据
        //int tmp = param->a_rate * (param->a_bits / 8) / 2048 + 1 + param->fps;
        int tmp = 9 + 25;
        avi_file->idx_array_count = (param->duration) * 70 * (tmp);
        PRINT_INFO("bps = %d, fps = %d, rec duration = %d minutes(%d MB), idx_array_count = %d\n",
                    param->bps, param->fps, param->duration, param->size_m, avi_file->idx_array_count);
    }
    else
    {
        PRINT_ERR_MSG("record mode is error. mode = %d\n", param->mode);
        return -1;
    }


    //check the sd card is mounted.
    if(grd_sd_is_mount() == 0)
    {
        PRINT_ERR_MSG("sd card is not mounted.\n");
		mmc_sdcard_status_check();
        return -1;
    }

	
    //预留文件的大小后，剩余空间如果小于10M
    t1 = avi_get_sys_runtime(1);
    while(grd_sd_get_free_size() < (param->size_m + 300))
    {
        #if 0 // sd卡大小真正等于0时，将无法录像
        if(grd_sd_get_free_size() == 0)
        {
            PRINT_ERR_MSG("sd card is not mounted, so free size is 0.\n");
            return -1;
        }
        #endif
        PRINT_INFO("sd free size = %d MB, avi file size = %d MB\n", grd_sd_get_free_size() ,  param->size_m);

        if((!RECORD_IS_DELETE(param->mode))) //设置成没空间不删除旧文件，则没有空间创建新文件
        {
            PRINT_ERR_MSG("sd card have not enough space and the record flag is not delete, so return.\n");
            return -1;
        }
        else  //设置成没空间删除旧文件
        {
            PRINT_INFO("delete old files ...\n");
            ret = grd_sd_del_oldest_avi_file(); //删除旧文件
            if(ret == 0)
            {
                PRINT_ERR_MSG("no dir to del, dir error\n");
				if(runRecordCfg.sdCardFormatFlag == 1)  //手动格式化
				{
              	  	PRINT_ERR_MSG("no dir to del, counter=10 sd_format\n");
					grd_sd_format(); //多次删除文件或文件夹失败，则格式化SD卡
				}
				if(runRecordCfg.sdCardFormatFlag == 0)  //默认不自动格式化,OSD显示
				{
              	  	PRINT_ERR_MSG("no dir to del, counter=10 osd display format\n");
					mmc_set_sdcard_status_error();
                    return -1;
				}
            }
        }
		t2 = avi_get_sys_runtime(1);
		if(t2 - t1 > 30)
		{
			//mmc_set_sdcard_status();
			
            PRINT_ERR_MSG("delete file time exceed 20 seconds, may sd exception,format it\n");
			grd_sd_format();
			//return -1;
		}
    }

    //avi_file->idx_array_count = (param->duration) * 60 * (param->fps) *2;
    avi_file->audio_enable = param->audio_enable;
    avi_file->audio_type = param->a_enc_type;

    //释放上一个文件结构申请的资源
    if(avi_file->idx_array)
    {
        free(avi_file->idx_array);
        avi_file->idx_array = NULL;
    }
    avi_file->idx_array = (int *)malloc(avi_file->idx_array_count * 16);
    if(NULL == avi_file->idx_array)
    {
        PRINT_ERR_MSG("malloc idx_array failed.\n");
        return -1;
    }

	memset(avi_file->idx_array,0,avi_file->idx_array_count * 16);
	
    //释放上一个文件结构申请的资源
    if(avi_file->file)
    {
        fclose(avi_file->file);
        avi_file->file = NULL;
    }

    avi_flag_open = 1;
    avi_flag_new = 1;

    data_array_pos = 0;
    data_array_index = 0;

    return 0;
}

/**********************************************************************
函数描述：向AVI文件中写入一帧数据
入口参数：AviFile *avi_file:  AVI文件的控制句柄
          AviInitParam *param: 传入的参数信息
          unsigned char *data:  写入的数据的地址
          int length: 写入的数据的长度
          int intra: 1,表示为关键桢数据;0表示不为关键桢数据
返回值：   0: 正常返回
          -1: 错误返回
           1: 不是关键帧
**********************************************************************/
static int avi_write_frame_data(AviFile *avi_file,
                                AviInitParam *param,
                                char *data,
                                int length,
                                int intra,
                                unsigned int timetick)
{
    int ret = 0;
    int is_align = length % 2;
    //int is_align = 0;

    if(!avi_file)
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }

    #if 1
    if(avi_flag_new)
    {
        if(intra == 1 || intra == 3) //第一帧数据，重命名文件
        {
			if(mmc_sdcard_write_detect() != 0)
			{
				printf("Record check file system is readonly\n");
				ret = -2;
				goto sd_error;// 文件操作出错，需要检查SD卡状态
			}
            if(avi_file->file)
            {
                fclose(avi_file->file);
                avi_file->file = NULL;
            }

            char time_start[30];
            char date[10];
            char date_path[128];

            memset(time_start, 0, sizeof(time_start));
            time_to_str(avi_file->time_s, time_start); // get_nowtime_str(time_start);
            
            //日期目录不存在，则创建
            memset(date, 0, sizeof(date));            
            memset(date_path, 0, sizeof(date_path));
            avi_substring(date, time_start, 0, 0+8);
            sprintf(date_path, "%s/%s", GRD_SD_MOUNT_POINT, date); // str = /mnt/sd_card/20120914
            if(access(date_path, F_OK) != 0 ) //日期目录不存在，则创建
            {
                ret = mkdir(date_path, 0777);
                if(ret < 0)
                {
                    PRINT_ERR_MSG("mkdir %s failed\n", date_path);
        			goto sd_error;
                }
            }

            //create avi file
            memset(avi_file->filename, 0, sizeof(avi_file->filename));
            sprintf(avi_file->filename, "%s/ch%d_%s", date_path, param->ch_num, time_start);  // filename: ch0_20120101010101
            PRINT_INFO("1.Rec filename = %s \n", avi_file->filename);

            //释放上一个文件结构申请的资源
            avi_file->file = fopen(avi_file->filename, "wb+");
            if(NULL == avi_file->file)
            {
                PRINT_ERR_MSG("fopen %s failed.\n", avi_file->filename);
				goto sd_error;// 文件操作出错，需要检查SD卡状态
            }

            //write avi header
            ret = avi_write_header(avi_file, param);
            if(ret < 0)
            {
                PRINT_ERR_MSG("avi_write_header failed.\n");
        		goto sd_error;
            }
            
            avi_file->data_offset = sizeof(AviHeader);
            fseek(avi_file->file, avi_file->data_offset, SEEK_SET);
            avi_flag_new = 0;
        }
        else
        {
            PRINT_INFO("It is not key frame.\n");
            return 1; //第一帧不是I帧
        }
    }
    #endif

    /* write data, 判断buf是否足够存储 */
	int write_data = data_array_pos + 8 + length + is_align + 12;
    if(write_data > rec_buf_size)
    {
        PRINT_ERR_MSG("REC_BUF_SIZE is too small. data_array_pos = %d %d\n", data_array_pos,write_data);
        return -1;
    }

    //*((int *)&(data_array[data_array_pos])) = MAKE_FOURCC('0','0','d','c'); //0x63643030
    //*((int *)&(data_array[data_array_pos + 4])) = length + is_align;
    if(intra == 1 || intra == 0 || intra == 3 )
    {
        avi_set_32( &(data_array[data_array_pos]), MAKE_FOURCC('0','0','d','c'));
		data_array_index ++;
    }
    else if(intra == 2)
    {
        avi_set_32( &(data_array[data_array_pos]), MAKE_FOURCC('0','1','w','b'));
    }
    else
    {
        PRINT_ERR_MSG("frame type error.\n");
        return -1;
    }
    avi_set_32( &(data_array[data_array_pos + 4]), length + is_align);
    data_array_pos += 8 + length + is_align;

    avi_set_32( &(data_array[data_array_pos]), MAKE_FOURCC('J','U','N','K'));
    avi_set_32( &(data_array[data_array_pos + 4]), 4);
    avi_set_32( &(data_array[data_array_pos + 8]), timetick);
    data_array_pos += 12;

    /* 每秒写入一次数据，避免频繁写文件,数据达到buf的80%时写入一次 */
    //data_array_index ++;
    if( (write_data > (rec_buf_size*4/5)) || (data_array_index >= avi_file->video_fps))
    {
        data_array_index = 0;
		if(mmc_sdcard_write_detect() != 0)
		{
			printf("Record check file system is readonly\n");
			ret = -2;
			goto sd_error;// 文件操作出错，需要检查SD卡状态
		}

#if CAL_TIME_REC
        struct timeval t_rec1, t_rec2, t_rec3;
        gettimeofday(&t_rec1, NULL);
#endif

        ret = fwrite(data_array, data_array_pos, 1, avi_file->file);
		if(ferror(avi_file->file))
		{
			PRINT_ERR_MSG("fwrite failed.\n");
			clearerr(avi_file->file);
			goto sd_error;// 文件操作出错，需要检查SD卡状态
		}

#if CAL_TIME_REC
        gettimeofday(&t_rec2, NULL);
        printf("write buf time: %d us, size = %d \n", (t_rec2.tv_sec-t_rec1.tv_sec)*1000000+(t_rec2.tv_usec-t_rec1.tv_usec), data_array_pos);
#endif

#if CAL_TIME_REC
        //memset(data_array, 0, sizeof(data_array));
        gettimeofday(&t_rec3, NULL);
        printf("memset time: %d us, ret = %d \n", (t_rec3.tv_sec-t_rec2.tv_sec)*1000000+(t_rec3.tv_usec-t_rec2.tv_usec), ret);
#endif
        data_array_pos = 0;
		if(mmc_sdcard_write_detect() != 0)
		{
			printf("Record check file system is readonly\n");
			ret = -2;
			goto sd_error;// 文件操作出错，需要检查SD卡状态
		}
    }


    if(avi_file->index_count >= avi_file->idx_array_count)
    {
    	#if 0
        PRINT_INFO("the space of index is too small.%d:%d\n",avi_file->index_count,avi_file->idx_array_count);
        avi_file->idx_array_count += 3000;
        avi_file->idx_array = (int *)realloc(avi_file->idx_array, avi_file->idx_array_count * 16);
        if (avi_file->idx_array == NULL)
        {
            PRINT_ERR_MSG("realloc failed.\n");
            return -1;
        }

		#else
		
        PRINT_INFO("the space of index is too small.%d:%d\n",avi_file->index_count,avi_file->idx_array_count);
		int count = avi_file->idx_array_count * 16;
		avi_file->idx_array_count += 3000;
		int *lastbuf = (int *)malloc(avi_file->idx_array_count * 16);
		if (lastbuf == NULL)
		{
			 PRINT_ERR_MSG("malloc failed.\n");
			 return  -1;
		}
		
		memset((void *)lastbuf,0,avi_file->idx_array_count * 16);
		memcpy(lastbuf,avi_file->idx_array,count);
		free(avi_file->idx_array);
		avi_file->idx_array = NULL;
		avi_file->idx_array = lastbuf;
		#endif
    }

    if(intra == 1 || intra == 0 || intra == 3)
    {
        avi_set_32( &avi_file->idx_array[4*avi_file->index_count+0], MAKE_FOURCC('0','0','d','c'));
        //avi_set_32( &avi_file->idx_array[4*avi_file->index_count+1], (intra) ? 0x10:0);
        //avi_set_32( &avi_file->idx_array[4*avi_file->index_count+1], 0x10);
        avi_set_32( &avi_file->idx_array[4*avi_file->index_count+1], (intra) ? 0x11:0x10);
        avi_file->video_count++;
    }
    else
    {
        avi_set_32( &avi_file->idx_array[4*avi_file->index_count+0], MAKE_FOURCC('0','1','w','b'));
        avi_set_32( &avi_file->idx_array[4*avi_file->index_count+1], 0x00);
        avi_file->audio_bytes += length+is_align;
    }

    avi_set_32( &avi_file->idx_array[4*avi_file->index_count+2], avi_file->data_offset);
    avi_set_32( &avi_file->idx_array[4*avi_file->index_count+3], length + is_align);


    avi_file->data_offset += (8+length+is_align+12);
    avi_file->index_count++;


    return 0;
	sd_error:
		mmc_sdcard_status_check();
		return -1;
}


/**********************************************************************
函数描述：向AVI文件中写入一帧数据
入口参数：AviFile *avi_file:  AVI文件的控制句柄
          AviInitParam *param: 传入的参数信息
          unsigned char *data:  写入的数据的地址
          int length: 写入的数据的长度
          int intra: 1 I, 0 P, 2 A, 3 JPEG
返回值：   0: 正常返回
          <0: 错误返回
**********************************************************************/
int avi_record_write(AviFile *avi_file,
                     AviInitParam *param,
                     char *data,
                     int length,
                     int intra,
                     unsigned int timetick)
{
    int ret = 0;

    ret = avi_write_frame_data(avi_file, param, data, length, intra, timetick);
	if(ret < 0)
	{
		#if 0
		if(access("/dev/mmcblk0", F_OK) != 0)
		{
			if (avi_file->file)
				{fclose(avi_file->file);avi_file->file = NULL;}
			sync();
			grd_sd_umount();
			sync();
			ret = -2;
		}
		else
		{
			grd_sd_umount();
			sleep(3); //避免频繁占用CPU
			grd_sd_mount();
		}
		#endif
		ret = -2;
        //重新OPEN，重置flag
        avi_flag_new = 0;
        avi_flag_open = 0;

        memset(data_array, 0, rec_buf_size);
        data_array_pos = 0;
        data_array_index = 0;
    }

    return ret;
}

/**********************************************************************
函数描述：重命名AVI文件为最终格式，并关闭文件，释放内存
入口参数：AviFile *avi_file:  AVI文件的控制句柄
返回值： 0: 正常返回
        <0: 错误返回
**********************************************************************/
int avi_record_close(AviFile *avi_file)
{
    int ret = 0;
    char oldname[128];
    char time_stop[30];

    if(!avi_file)
    {
        PRINT_ERR_MSG("parameter is error.\n");
        return -1;
    }

    if(avi_flag_new == 0)
    {
        //get stop time to rename file name.
        memset(time_stop, 0, sizeof(time_stop));
        time_to_str(avi_file->time_e, time_stop); // get_nowtime_str(time_stop);

        //将buff里面最后的数据写入AVI文件中
        if(data_array_pos != 0)
        {
        	//PRINT_INFO("ftell:%d, dataoffset:%d,data_array_pos:%d",ftell(avi_file->file),avi_file->data_offset,data_array_pos);
			if(avi_file->file)
                ret = fwrite(data_array, 1,data_array_pos, avi_file->file);
            if(ret != data_array_pos)
            {
                PRINT_ERR_MSG("fwrite failed.pos:%d ret:%d %s\n",data_array_pos,ret,strerror(errno));
				mmc_sdcard_status_check();
                ret = -1;
                goto close_end;
            }
        	//PRINT_INFO("ftell:%d, dataoffset:%d,data_array_pos:%d",ftell(avi_file->file),avi_file->data_offset,data_array_pos);
			
            data_array_index = 0;
            data_array_pos = 0;
         }
        //int t1 = ftell(avi_file->file);
        //printf("t1 = %d\n", t1);

        //add index to avi file
        ret = avi_add_index(avi_file);
        if(ret < 0)
        {
            PRINT_ERR_MSG("call avi_add_index failed.\n");
			mmc_sdcard_status_check();
            goto close_end;
        }

        //rename filename_tmp to filename.avi
        if(avi_file->file)
        {
			fflush(avi_file->file);
			fsync(fileno(avi_file->file));
			
            fclose(avi_file->file);
            avi_file->file = NULL;
        }

        if(avi_file->idx_array)
        {
            free(avi_file->idx_array);
            avi_file->idx_array = NULL;
        }

        memset(oldname, 0, sizeof(oldname));
        strcpy(oldname, avi_file->filename);   //ch0_20120101010101
        sprintf(avi_file->filename, "%s_%s.avi", oldname, time_stop); //ch0_20120101010101_20120101011101.avi
        ret = rename(oldname, avi_file->filename);
        if(ret)
        {
            PRINT_ERR_MSG("rename %s to %s failed.\n", oldname, avi_file->filename);
			mmc_sdcard_status_check();
            ret = -1;
            goto close_end;
        }
        PRINT_INFO("3. Record file is %s\n", avi_file->filename);
        sync();

		record_start_check_file(avi_file->filename);
    }
close_end:
	data_array_index = 0;
	data_array_pos = 0;

    avi_flag_new = 1;
    avi_flag_open = 0;
    //close & free
    if(avi_file->file)
    {
        fflush(avi_file->file);
        fsync(fileno(avi_file->file));

        fclose(avi_file->file);
        avi_file->file = NULL;
    }

    if(avi_file->idx_array)
    {
        free(avi_file->idx_array);
        avi_file->idx_array = NULL;
    }

    return ret;
}


int avi_reocrd_check_file_stoped()
{
	int ret = 0;
	if(avi_check_file_number != 0)
		ret = 1;
	return ret;
}

int avi_record_write_cache(AviFile *avi_file)
{
	int ret = 0;
	if(data_array_pos == 0)
		return 0;

	if (NULL == avi_file)
	{
		PRINT_ERR_MSG("avi_file is NULL.\n");
		return -2;
	}
	
	ret = fwrite(data_array, data_array_pos, 1, avi_file->file);
	if(ferror(avi_file->file))
	{
		PRINT_ERR_MSG("fwrite failed.\n");
		clearerr(avi_file->file);
		return  -2;
	}
	else
	{
		ret = 0;
	}
	
	data_array_pos = 0;
	data_array_index = 0;
	if(mmc_sdcard_write_detect() != 0)
	{
		printf("Record check file system is readonly\n");
		ret = -2;
	}

	return ret;
}

