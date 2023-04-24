#include "avi_common.h"
#include "avi_utility.h"
#include "mmc_api.h"
#include "avi_search_api.h"
#include "netcam_api.h"

extern int avi_add_index(AviFile *avi_file);
extern int is_fix_format_file(char *str);

int avi_flag_pause = 0;
//pthread_mutex_t avi_pb_speed_x;
int avi_pb_speed = 1;


#define PB_N_LOOP 10

#define PB_DEBUG 0

static int avi_get_32(void *pp)
{
    char *p = pp;
    int tmp32 = (int)((p[0])|(p[1])<<8|(p[2])<<16|(p[3])<<24);
    return tmp32;
}

static u32t avi_get_u32(void *pp)
{
    char *p = pp;
    u32t tmp32 = (u32t)((p[0])|(p[1])<<8|(p[2])<<16|(p[3])<<24);
    return tmp32;
}

static int check_avi_header_fcc(AviHeader *avi_header)
{
	if (NULL == avi_header)
	{
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
	}

   if(avi_header->fcc_riff != MAKE_FOURCC('R','I','F','F'))
        return -1;

    if(avi_header->fcc_avi != MAKE_FOURCC('A','V','I',' '))
        return -1;

    if(avi_header->fcc_list_hdrl != MAKE_FOURCC('L','I','S','T'))
        return -1;

    if(avi_header->fcc_hdrl != MAKE_FOURCC('h','d','r','l'))
        return -1;

    if(avi_header->fcc_list_strl_video != MAKE_FOURCC('L','I','S','T'))
        return -1;
    if(avi_header->fcc_strl_video != MAKE_FOURCC('s','t','r','l'))
        return -1;
    if(avi_header->fcc_strf_video != MAKE_FOURCC('s','t','r','f'))
        return -1;

    if(avi_header->fcc_list_strl_audio != MAKE_FOURCC('L','I','S','T'))
        return -1;
    if(avi_header->fcc_strl_audio != MAKE_FOURCC('s','t','r','l'))
        return -1;
    if(avi_header->fcc_strf_audio != MAKE_FOURCC('s','t','r','f'))
        return -1;

    if(avi_header->fcc_list_movi != MAKE_FOURCC('L','I','S','T'))
        return -1;
    if(avi_header->fcc_movi != MAKE_FOURCC('m','o','v','i'))
        return -1;

    return 0;
}

/**********************************************************************
函数描述：读取avi文件的头信息
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件的设置参数
返回值：  <0: 错误
           0: 正常
**********************************************************************/
static int avi_read_header_and_index(AviPBHandle *pPBHandle)
{
    int tmp32 = 0;
    int ret   = 0;

    if(!pPBHandle)
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }

    AviHeader avi_header;
	memset(&avi_header,0,sizeof(AviHeader));
	
    ret = fread(&avi_header, sizeof(AviHeader), 1, pPBHandle->file);
    if(ret != 1)
    {
        PRINT_ERR_MSG("read avi header failed. ret = %d\n", ret);
        return -1;
    }

    ret = check_avi_header_fcc(&avi_header);
    if(ret != 0)
    {
        PRINT_ERR_MSG("avi header fcc error.\n");
        return -1;
    }

    if (avi_header.main_header.dwMicroSecPerFrame <= 0)
    {
        PRINT_ERR_MSG("dwMicroSecPerFrame %d error.\n", avi_header.main_header.dwMicroSecPerFrame);
        return -1;
    }

    //param->bps = avi_header.main_header.dwMaxBytesPerSec * 8;
    if(1000000/25 == avi_header.main_header.dwMicroSecPerFrame)
        pPBHandle->fps = 25;
    else
        pPBHandle->fps = 1000000 / avi_header.main_header.dwMicroSecPerFrame;
    PRINT_INFO("fps = %d \n", pPBHandle->fps);

    pPBHandle->video_width  = avi_header.main_header.dwWidth;
    pPBHandle->video_height = avi_header.main_header.dwHeight;
    //param->type = avi_header.video_header.fccHandler;   //H264,h264
    //param->fcctype = avi_header.video_header.fccType;  //vids


    //update avi_file infomation
    pPBHandle->video_count = avi_header.main_header.dwTotalFrames;
    //pPBHandle->audio_bytes = avi_header.audio_header.dwLength * avi_header.audio_header.dwScale;
    //pPBHandle->data_offset = sizeof(AviHeader);

    //rec: int movi_len = avi_file->data_offset - sizeof(AviHeader) + 4;
    ret = fseek(pPBHandle->file, avi_header.movi_len - 4 + sizeof(AviHeader), SEEK_SET);
    if(ret != 0)
    {
        PRINT_ERR_MSG("fseek failed.\n");
        return -1;
    }

    //PRINT_INFO("movi_len = %d.\n", avi_header.movi_len);
    //PRINT_INFO("index offset = %d, data start = %d \n", avi_header.movi_len - 4 + sizeof(AviHeader), sizeof(AviHeader));
    //ret = fread(&tmp32, sizeof(int), 1, avi_file->file);
    int fcc = 0;
    ret = read_fourcc(pPBHandle->file, &fcc);
    if(ret != 1)
    {
        PRINT_ERR_MSG("read fcc idx1 failed. ret = %d\n", ret);
        return -2;
    }
    if(fcc != MAKE_FOURCC('i','d','x','1'))
    {
        PRINT_ERR_MSG("fcc %08x is not idx1.\n", fcc);
        return -2;
    }


    ret = fread(&tmp32, sizeof(int), 1, pPBHandle->file);
    if(ret != 1)
    {
        PRINT_ERR_MSG("read index size failed.\n");
        return -1;
    }
    pPBHandle->index_count = tmp32 / 16;
    PRINT_INFO("index count = %d\n", pPBHandle->index_count);

    //read index to mem
    pPBHandle->idx_array_count = pPBHandle->index_count;
    pPBHandle->idx_array = (int *)malloc((pPBHandle->idx_array_count) * 16);
    if(pPBHandle->idx_array == NULL)
    {
        PRINT_ERR_MSG("avi_read_header, malloc index failed.\n");
        return -1;
    }

    ret = fread(pPBHandle->idx_array, pPBHandle->index_count*16, 1, pPBHandle->file);
    if(ret != 1)
    {
        PRINT_ERR_MSG("avi_read_header, fread error.\n");
		if (pPBHandle->idx_array)
		{
			free(pPBHandle->idx_array);
			pPBHandle->idx_array = NULL;
		}
        return -1;
    }

    PRINT_INFO("avi_read_header_and_index ok.\n");
    return 0;
}



/**********************************************************************
函数描述：通过读取每一帧的信息得到index，index保存在内存中
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件的设置参数
返回值：  <0: 错误
           0: 正常
**********************************************************************/
static int avi_fix_read_data(AviFile *avi_file, int *p_fps)
{
    int i          = 0;
	int fps        = 0;
	int streamid   = 0;
	int size       = 0;
    int ret        = 0;
    int frame_type = 0;
	
    if( (!avi_file))
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }

    #if 1
    //得到fps
    AviHeader avi_header;
	memset(&avi_header,0,sizeof(avi_header));
	
    fseek(avi_file->file, 0, SEEK_SET);
    ret = fread(&avi_header, sizeof(AviHeader), 1, avi_file->file);
    if(ret != 1)
    {
        PRINT_ERR_MSG("read avi header failed. ret = %d\n", ret);
        return -1;
    }

    if(1000000/25 == avi_header.main_header.dwMicroSecPerFrame)
        fps = 25;
    else
        fps = 1000000 / avi_header.main_header.dwMicroSecPerFrame;
	
    *p_fps = fps;

    PRINT_INFO("fps = %d \n", fps);

    //update avi_file infomation
    avi_file->video_count = avi_header.main_header.dwTotalFrames;
    avi_file->audio_bytes = avi_header.audio_header.dwLength * avi_header.audio_header.dwScale;
    avi_file->data_offset = sizeof(AviHeader);
    #endif

    //read frame data frame by frame
    avi_file->idx_array_count = 0;
    avi_file->video_count     = 0;
    avi_file->audio_bytes     = 0;
    avi_file->index_count     = 0;
    avi_file->data_offset     = sizeof(AviHeader);
    avi_file->idx_array       = NULL;

    int last_audio_bytes = 0;

    while(1)
    {
        if( avi_file->idx_array_count <= avi_file->index_count)
        {
        	#if 1
            avi_file->idx_array_count += 6000;
            avi_file->idx_array = (int *)realloc(avi_file->idx_array, avi_file->idx_array_count * 16 );
            if (avi_file->idx_array == NULL)
            {
                PRINT_ERR_MSG("realloc failed.\n");
                break;
            }
			#else

			PRINT_INFO("read : the space of index is too small.\n");
			int count = avi_file->idx_array_count * 16;
			avi_file->idx_array_count += 6000;
			int *lastbuf = (int *)malloc(avi_file->idx_array_count * 16);
			if (lastbuf == NULL)
			{
				 LOG_ERR("malloc failed.\n");
				 return  -1;
			}

			if (avi_file->idx_array)
			{
				memcpy(lastbuf,avi_file->idx_array,count);
				free(avi_file->idx_array);
				
				avi_file->idx_array = NULL;
				avi_file->idx_array = lastbuf;
			}
			else
				avi_file->idx_array = lastbuf;
			
			#endif
        }

        ret = fseek(avi_file->file, avi_file->data_offset, SEEK_SET);

        ret = read_fourcc(avi_file->file, &streamid);
        //读失败，回退一帧数据
        if( ((streamid != MAKE_FOURCC('0','0','d','c'))&&(streamid != MAKE_FOURCC('0','1','w','b'))) || (ret != 1))
        {
            avi_file->index_count --;
            if(avi_file->index_count < 0)
            {
                avi_file->index_count = 0;
                avi_file->data_offset = sizeof(AviHeader);
                avi_file->video_count = 0;
                avi_file->audio_bytes = 0;
                break;
            }

            avi_file->data_offset = avi_file->idx_array[4*avi_file->index_count+2];
            if(frame_type)
                avi_file->video_count--;
            else
                avi_file->audio_bytes = last_audio_bytes;
            break;
        }

        ret = read_int32(avi_file->file, &size);

        avi_set_32( &avi_file->idx_array[4*i+0], streamid);
        if(streamid == MAKE_FOURCC('0','0','d','c'))
        {
            if(avi_file->video_count%fps == 0)
                avi_set_32( &avi_file->idx_array[4*i+1], 0x11); //i
            else
                avi_set_32( &avi_file->idx_array[4*i+1], 0x10); //p

            avi_file->video_count ++;
            frame_type = 1; //video
        }
        else
        {
            avi_set_32( &avi_file->idx_array[4*i+1], 0x00);
            last_audio_bytes = avi_file->audio_bytes;
            avi_file->audio_bytes += size;
            frame_type = 0; //audio
        }

        avi_set_32( &avi_file->idx_array[4*i+2], avi_file->data_offset);
        avi_set_32( &avi_file->idx_array[4*i+3], size);
        i++;

        avi_file->data_offset = avi_file->data_offset + 8 + size + 12;
        avi_file->index_count ++;
    }

    ret = fseek(avi_file->file, avi_file->data_offset, SEEK_SET);
    if(ret < 0)
    {
        PRINT_ERR_MSG("fseek to last offset failed. offset = %d\n", avi_file->data_offset);
        return -1;
    }
    return 0;
}


/**********************************************************************
函数描述：计算结束时间
入口参数：char *time_stop: 计算后得到的结束时间
          char *file_path: avi文件路径名
          int frame_count: avi文件中存的总帧数
          int fps: 每秒的帧数
返回值：  无
**********************************************************************/
static void calc_stop_time(char *time_stop, char *file_path, int video_count, int fps)
{
    char str_tmp[10];
    int    y1,m1,d1,h1,n1,s1;  //start
    int duration;
    time_t timep;
    struct tm *p;

    int i;
    char file_name[128];

    memset(file_name, 0 ,sizeof(file_name));
    i = last_index_at(file_path, '/');
    strcpy(file_name, file_path + i + 1);

    avi_substring(str_tmp, file_name, 4, 4+4);
    y1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 8, 8+2);
    m1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 10, 10+2);
    d1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 12, 12+2);
    h1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 14, 14+2);
    n1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 16, 16+2);
    s1 = atoi(str_tmp);

    duration = (int) video_count / fps;

    time(&timep);
    struct tm tt = {0};
    p = localtime_r(&timep, &tt);


    p->tm_year = y1 - 1900;
    p->tm_mon = m1 - 1;
    p->tm_mday = d1;
    p->tm_hour = h1;
    p->tm_min = n1;
    p->tm_sec = s1;

    //PRINT_INFO ("%d-%d-%d ", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
    //PRINT_INFO(" %d:%d:%d\n", p->tm_hour, p->tm_min, p->tm_sec);

    timep = mktime(p);

    timep += duration;
    memset(&tt, 0, sizeof(struct tm));
    p = localtime_r(&timep, &tt);

    //PRINT_INFO ("%d-%d-%d ", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
    //PRINT_INFO(" %d:%d:%d\n", p->tm_hour, p->tm_min, p->tm_sec);

    sprintf(time_stop, "%04d%02d%02d%02d%02d%02d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
                                                   p->tm_hour, p->tm_min, p->tm_sec);
}




/**********************************************************************
函数描述：修复单个未录完成的avi文件
入口参数：char *file_path: avi文件路径名
返回值：  <0: 错误
           0: 正常返回
**********************************************************************/
static int fix_single_avi_file(char *file_path)
{
    AviFile avi_file;
    //AviInitParam param;

    char name_tmp[128];
    char time_stop[30];
    int ret;

    // 类似名字为ch0 的avi文件，里面并没有数据，头信息也可能不完全
    if(0 == is_fix_format_path(file_path))
    {
        PRINT_ERR_MSG("%s can't be fixed.\n", file_path);
        return -1;
    }

    //open file
    strcpy(avi_file.filename, file_path);
    PRINT_INFO("fixing file : %s\n", avi_file.filename);
    avi_file.file = fopen(avi_file.filename, "rb+");
    if(NULL == avi_file.file)
    {
        PRINT_ERR_MSG("open %s failed.\n", avi_file.filename);
        return -1;
    }

    //read data
    int fps;
    ret = avi_fix_read_data(&avi_file, &fps);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_read_stream_offset error.\n");
        if(avi_file.file)
        {
            fclose(avi_file.file);
            avi_file.file = NULL;
        }

        if(avi_file.idx_array)
        {
            free(avi_file.idx_array);
            avi_file.idx_array = NULL;
        }
        return -1;
    }

    ret = avi_add_index(&avi_file);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_read_stream_offset error.\n");
        if(avi_file.file)
        {
            fclose(avi_file.file);
            avi_file.file = NULL;
        }

        if(avi_file.idx_array)
        {
            free(avi_file.idx_array);
            avi_file.idx_array = NULL;
        }
        return -1;
    }

    if(avi_file.file)
    {
        fclose(avi_file.file);
        avi_file.file = NULL;
    }

    if(avi_file.idx_array)
    {
        free(avi_file.idx_array);
        avi_file.idx_array = NULL;
    }

    //cal stop time
    memset(time_stop, 0 ,sizeof(time_stop));
    calc_stop_time(time_stop, avi_file.filename, avi_file.video_count, fps);
    PRINT_INFO(" stoptime= %s \n", time_stop);


    //rename filename_tmp to filename.avi
    memset(name_tmp, 0, sizeof(name_tmp));
    sprintf(name_tmp, "%s", avi_file.filename);   //ch0_20120101010101
    sprintf(avi_file.filename, "%s_%s.avi", name_tmp, time_stop); //ch0_20120101010101_20120101011101.avi

    ret = rename(name_tmp, avi_file.filename);
    if(ret)
    {
        PRINT_ERR_MSG("rename %s to %s failed.\n", name_tmp, avi_file.filename);
        return -1;
    }


    PRINT_INFO("fix %s to %s\n", name_tmp, avi_file.filename);
    return 0;
}


/**********************************************************************
函数描述：修复单个已完成的avi文件，但是最后index错误
入口参数：char *file_path: avi文件路径名
返回值：  <0: 错误
           0: 正常返回
**********************************************************************/
static int fix_single_over_avi_file(char *file_path)
{
    char name_tmp[128] = {0};
    char time_stop[30] = {0};
    int ret            = 0;

	if (NULL == file_path)
	{
        PRINT_ERR_MSG("file_path is NULL.\n");
        return -1;
	}
	
    AviFile avi_file;
    //AviInitParam param;
	memset(&avi_file,0,sizeof(avi_file));
	
    //open file
    strcpy(avi_file.filename, file_path);
    PRINT_INFO("fixing file : %s\n", avi_file.filename);
    avi_file.file = fopen(avi_file.filename, "rb+");
    if(NULL == avi_file.file)
    {
        PRINT_ERR_MSG("open %s failed.\n", avi_file.filename);
        return -1;
    }

    //read data
    int fps = 0;
    ret = avi_fix_read_data(&avi_file, &fps);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_read_stream_offset error.\n");
        if(avi_file.file)
        {
            fclose(avi_file.file);
            avi_file.file = NULL;
        }

        if(avi_file.idx_array)
        {
            free(avi_file.idx_array);
            avi_file.idx_array = NULL;
        }
        return -1;
    }

    ret = avi_add_index(&avi_file);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_read_stream_offset error.\n");
        if(avi_file.file)
        {
            fclose(avi_file.file);
            avi_file.file = NULL;
        }

        if(avi_file.idx_array)
        {
            free(avi_file.idx_array);
            avi_file.idx_array = NULL;
        }
        return -1;
    }

    if(avi_file.file)
    {
        fclose(avi_file.file);
        avi_file.file = NULL;
    }

    if(avi_file.idx_array)
    {
        free(avi_file.idx_array);
        avi_file.idx_array = NULL;
    }

    PRINT_INFO("fix %s ok\n", file_path);
    return 0;
}

/**********************************************************************
函数描述：修复单个已完成的avi文件，但是最后index错误
入口参数：char *file_path: avi文件路径名
返回值：  <0: 错误
           0: 正常返回
**********************************************************************/
int avi_check_file_index(char *file_path)
{
    int ret                = 0;
    AviPBHandle *pPBHandle = NULL;
	if (NULL == file_path)
	{
        PRINT_ERR_MSG("file_path is NULL\n");
        return -1;
	}
	
    pPBHandle = (AviPBHandle *)create_pb_handle();
	if (NULL == pPBHandle)
	{
        PRINT_ERR_MSG("pPBHandle is NULL\n");
        return -1;
	}

    strcpy(pPBHandle->file_path, file_path);
    PRINT_INFO("pb open file : %s\n", pPBHandle->file_path);
	
    pPBHandle->file = fopen(pPBHandle->file_path, "rb+");
    if(NULL == pPBHandle->file)
    {
        PRINT_ERR_MSG("open %s failed.\n", pPBHandle->file_path);
        return -1;
    }

    //read avi header
    ret = avi_read_header_and_index(pPBHandle);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_read_header failed. ret:%d\n", ret);
        if (ret == -2)
        {
            fclose(pPBHandle->file);
            pPBHandle->file = NULL;
            PRINT_ERR_MSG("start to fix avi file.\n");
            fix_single_over_avi_file(pPBHandle->file_path);
        }
        else
        {
		    if (pPBHandle->idx_array)
		    {
		        free(pPBHandle->idx_array);
		        pPBHandle->idx_array = NULL;
		    }

		    if(pPBHandle->file)
		    {
		        fclose(pPBHandle->file);
		        pPBHandle->file = NULL;
		    }
			
		    free(pPBHandle);
		    pPBHandle = NULL;			
            return -1;
        }
    }
    else
    {
        PRINT_INFO("avi file %s is ok.\n", pPBHandle->file_path);
    }

    if (pPBHandle->idx_array)
    {
        free(pPBHandle->idx_array);
        pPBHandle->idx_array = NULL;
    }

    if(pPBHandle->file)
    {
        //PRINT_INFO("avi_pb_close close file.\n");
        fclose(pPBHandle->file);
        pPBHandle->file = NULL;
    }

    free(pPBHandle);
    pPBHandle = NULL;
	return 0;
}


/**********************************************************************
函数描述：修复文件夹下所有的未录完成的avi文件
入口参数：char *dir_path: 文件夹路径名
返回值：  无
**********************************************************************/
static void fix_avi_file_in_dir(char *dir_path)
{
    DIR *pDir;
    struct dirent *ent;
    char file_path[128];

    PRINT_INFO("fix avi file in %s\n", dir_path);

    pDir = opendir(dir_path);
    if(!pDir)
    {
		PRINT_ERR("opendir fail:%s\n", dir_path);
		return 0;
	}
    while((ent = readdir(pDir)) != NULL)
    {

        if(ent->d_type & DT_REG)  //判断是否为非目录
        {
            //PRINT_INFO("check file : %s\n", ent->d_name);
            if (0 == is_fix_format_file(ent->d_name))
                continue;
            //PRINT_INFO("%s need to be fixed.\n", ent->d_name);
            memset(file_path, 0, sizeof(file_path));
            sprintf(file_path, "%s/%s", dir_path, ent->d_name);
            fix_single_avi_file(file_path);
        }
    }
	closedir(pDir);
}

/**********************************************************************
函数描述：修复sd卡中所有的未录完成的avi文件
入口参数：无
返回值：  无
**********************************************************************/
void fix_all_avi_file(void)
{
    DIR *pDir;
    struct dirent *ent;
    char dir_path[128];

    //PRINT_INFO("fix all avi file.\n");
    pDir = opendir(GRD_SD_MOUNT_POINT);
    if(!pDir)
    {
		PRINT_ERR("opendir fail:%s\n", GRD_SD_MOUNT_POINT);
		return 0;
	}
    while((ent = readdir(pDir)) != NULL)
    {
        if(ent->d_type & DT_DIR)  //判断是否为目录
        {
            if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
                continue;

            if(is_avi_dir_format(ent->d_name) == 0)
                continue;

            memset(dir_path, 0, sizeof(dir_path));
            sprintf(dir_path, "%s/%s", GRD_SD_MOUNT_POINT, ent->d_name);
            fix_avi_file_in_dir(dir_path);
        }
    }
	closedir(pDir);
}

static int cal_pb_gop(AviPBHandle *pPBHandle)
{
    int gop = 0;
    int no = 0;

    while (1)
    {
        if(no >= pPBHandle->idx_array_count)
            break;
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
        if(no >= pPBHandle->idx_array_count)
            break;
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

/**********************************************************************
函数描述：通过读AVI文件信息，得到他的控制句柄和参数信息
入口参数：char *file_path: 要打开的文件路径名
          AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
返回值：  <0: 错误
           0: 正常
**********************************************************************/
int avi_pb_open(char *file_path, AviPBHandle *pPBHandle)
{
    int ret;

    if(!pPBHandle || !file_path)
    {
        PRINT_ERR_MSG("parameter is error.\n");
        return -1;
    }

#if 0
    //安普对接的原因，加个open flag，防止多次打开
    if(pPBHandle->pb_open_flag == 1)
    {
        PRINT_ERR_MSG(" %s is already opened.\n", file_path);
        return -1;
    }

    pPBHandle->pb_open_flag = 1;
#endif

    //初始化
    if(pPBHandle->file)
    {
        fclose(pPBHandle->file);
        pPBHandle->file = NULL;
    }
    if(pPBHandle->idx_array)
    {
        free(pPBHandle->idx_array);
        pPBHandle->idx_array = NULL;
    }

    #if 0
    if(pPBHandle->pb_buf)
    {
        free(pPBHandle->pb_buf);
        pPBHandle->pb_buf = NULL;
    }
    #endif


    //open file
    strcpy(pPBHandle->file_path, file_path);
    PRINT_INFO("pb open file : %s\n", pPBHandle->file_path);
    pPBHandle->file = fopen(pPBHandle->file_path, "rb+");
    if(NULL == pPBHandle->file)
    {
        PRINT_ERR_MSG("open %s failed.\n", pPBHandle->file_path);
        return -1;
    }

    //read avi header
    ret = avi_read_header_and_index(pPBHandle);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_read_header failed.\n");
        if (ret == -2)
        {
            fclose(pPBHandle->file);
            fix_single_over_avi_file(pPBHandle->file_path);
            pPBHandle->file = fopen(pPBHandle->file_path, "rb+");
            if(NULL == pPBHandle->file)
            {
                PRINT_ERR_MSG("open %s failed.\n", pPBHandle->file_path);
                return -1;
            }

            //read avi header
            ret = avi_read_header_and_index(pPBHandle);
            if(ret < 0)
            {
                fclose(pPBHandle->file);
				pPBHandle->file = NULL;
                return -1;
            }
        }
        else
        {
            fclose(pPBHandle->file);
			pPBHandle->file = NULL;
            return -1;
        }
    }

    pPBHandle->pb_buf_index = 0;
    pPBHandle->pb_buf_pos = 0;
    pPBHandle->no = 0;
	pPBHandle->pb_buf_num = 0;
    cal_pb_gop(pPBHandle);

    PRINT_INFO("avi_pb_open ok.\n");
    return 0;
}



#if 0
/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息，来设置帧数据的控制句柄
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
          AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
返回值：  无
**********************************************************************/
void set_frame_info(AviFile *avi_file, AviInitParam *param, AviFrameInfo *p_frame)
{
    memset(p_frame->file_path, 0, sizeof(p_frame->file_path));
    strcpy(p_frame->file_path, avi_file->filename);
    p_frame->video_count = avi_file->video_count;
    p_frame->index_count = avi_file->index_count;
    p_frame->fps = param->fps;
    p_frame->video_width = param->width;
    p_frame->video_height = param->height;
}
#endif

/**********************************************************************
函数描述：打印出avi文件中帧数据所在的avi文件中的信息
入口参数：AviPBHandle*pPBHandle: avi文件中帧数据的控制句柄
返回值：  无
**********************************************************************/
void print_pb_handle(AviPBHandle*pPBHandle)
{
    PRINT_INFO("******** PRINT PB HANDLE *******\n");
    PRINT_INFO("file_path = %s\n", pPBHandle->file_path);
    PRINT_INFO("video count = %d\n", pPBHandle->video_count);
    PRINT_INFO("index count = %d\n", pPBHandle->index_count);
    PRINT_INFO("no = %d\n", pPBHandle->no);
    PRINT_INFO("fps = %d\n", pPBHandle->fps);
    PRINT_INFO("video width  = %d\n", pPBHandle->video_width);
    PRINT_INFO("video height = %d\n", pPBHandle->video_height);
    if(pPBHandle->list == NULL)
        PRINT_INFO("just signal file, not use file list\n");
    else
        PRINT_INFO("more than one file, use file list\n");

    PRINT_INFO("******** PRINT PB HANDLE *******\n\n");
}

/**********************************************************************
函数描述：打印出avi文件中帧数据的信息
入口参数：AviFrameInfo *p: avi文件中帧数据的控制句柄
返回值：  无
**********************************************************************/
void print_frame_node(AviPBHandle *p)
{
    PRINT_INFO("\n******** PRINT FRAME NODE *******\n");
    PRINT_INFO("frame_type = %d\n", p->node.frame_type);
    PRINT_INFO("offset = %d\n", p->node.offset);
    PRINT_INFO("size = %d\n", p->node.size);
    PRINT_INFO("timetick = %u\n", p->node.timetick);

    PRINT_INFO("******** PRINT FRAME NODE *******\n\n");
}

/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息, 计算出该百分比位置是第几帧
          数据，并更新到 AviFrameInfo 的 no 参数中
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
          int percent: 百分比，如25%，percent = 25
          AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
返回值：  <0: 错误
           0: 正常
**********************************************************************/
int avi_pb_seek_by_percent(AviFile *avi_file, AviInitParam *param, int percent, AviPBHandle *pPBHandle)
{

    int tmp32 = 0;


    if( (!avi_file) || (!param))
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }


    if(0 == is_avi_file_path_format(avi_file->filename))
    {
        PRINT_ERR_MSG("%s is not avi file path format.\n", avi_file->filename);
        return -1;
    }


    tmp32 = (int)(avi_file->video_count * percent / (100 * param->fps) );
    pPBHandle->no = tmp32 * param->fps;

    return 0;
}

/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息, 计算出该定位时间是第几帧
          数据，并更新到 AviFrameInfo 的 no 参数中
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
          char *seek_time: 通过时间在进度条中定位
          AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
返回值：  <0: 错误
           0: 正常
**********************************************************************/
int avi_pb_seek_by_time(AviFile *avi_file, AviInitParam *param, char *seek_time, AviPBHandle *pPBHandle)
{

    char str_tmp[10];
    char avi_name[128];
    time_t timep1, timep2;
    struct tm *p1;
    struct tm *p2;
    int duration = 0;
    int i;


    time(&timep1);
    time(&timep2);
    struct tm tt1 = {0};
    p1 = localtime_r(&timep1, &tt1);
    struct tm tt2 = {0};
    p2 = localtime_r(&timep2, &tt2);

    //int fps = param->fps;

    if(!avi_file || !param)
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }

    if(0 == is_avi_file_path_format(avi_file->filename))
    {
        PRINT_ERR_MSG("%s is not avi file path format.\n", avi_file->filename);
        return -1;
    }

    memset(avi_name, 0 ,sizeof(avi_name));
    i = last_index_at(avi_file->filename, '/');
    strcpy(avi_name, avi_file->filename + i + 1);

    PRINT_INFO("avi_pb_seek_by_time avi_name=%s\n",avi_name);
    avi_substring(str_tmp, avi_name, 4, 4+4);
    p1->tm_year = atoi(str_tmp) - 1900;
    avi_substring(str_tmp, avi_name, 8, 8+2);
    p1->tm_mon = atoi(str_tmp) - 1;
    avi_substring(str_tmp, avi_name, 10, 10+2);
    p1->tm_mday = atoi(str_tmp);
    avi_substring(str_tmp, avi_name, 12, 12+2);
    p1->tm_hour = atoi(str_tmp);
    avi_substring(str_tmp, avi_name, 14, 14+2);
    p1->tm_min = atoi(str_tmp);
    avi_substring(str_tmp, avi_name, 16, 16+2);
    p1->tm_sec = atoi(str_tmp);

    avi_substring(str_tmp, seek_time, 0, 0+4);
    p2->tm_year = atoi(str_tmp) - 1900;
    avi_substring(str_tmp, seek_time, 4, 4+2);
    p2->tm_mon = atoi(str_tmp) - 1;
    avi_substring(str_tmp, seek_time, 6, 6+2);
    p2->tm_mday = atoi(str_tmp);
    avi_substring(str_tmp, seek_time, 8, 8+2);
    p2->tm_hour = atoi(str_tmp);
    avi_substring(str_tmp, seek_time, 10, 10+2);
    p2->tm_min = atoi(str_tmp);
    avi_substring(str_tmp, seek_time, 12, 12+2);
    p2->tm_sec = atoi(str_tmp);

    timep1 = mktime(p1);
    timep2 = mktime(p2);
    duration = timep2 - timep1;
    pPBHandle->video_no = duration * param->fps;

    return 0;
}




/*
int avi_pb_start(AviFile *avi_file, AviFrameInfo *avi_pos, int dec_fd, unsigned char *pbbs_buf)
{
    int ret;
    struct pollfd dec_fds;
    dvr_enc_queue_get   data;
    unsigned char *buf;
    int buf_size = 0;

    if(!avi_file)
        return STS_INVALID_INPUT;

    avi_file->file = fopen(avi_file->filename, "rb");
    if(NULL == avi_file->file)
        return STS_OPEN_FAILED;

    // prepare to select(or poll)
    dec_fds.fd = dec_fd;
    dec_fds.events = POLLIN;
    dec_fds.revents = 0;

    while(avi_pos->count --)
    {
        ret = poll(&dec_fds, 1, 2000);
        if(ret<0)
            PRINT_INFO("poll error\n");

        ret = ioctl(dec_fd, DVR_DEC_QUEUE_GET, &data);
        if (ret < 0) {
            PRINT_INFO("buffer is not ready...\n");
            grd_sleep(10000);
            continue;
        }

        buf = pbbs_buf + data.bs.offset;

        fseek(avi_file->file, avi_pos->offset + 4, SEEK_SET);
        read_int32(avi_file->file, &buf_size);

		ret = fread(buf, buf_size, 1, avi_file->file);     //read frame data from avi_file
        if(ret < 0)
        {
            PRINT_INFO("read avi_file data error!\n");
            buf_size = 0;
        }
        data.bs.length = buf_size;

        ret = ioctl(dec_fd, DVR_DEC_QUEUE_PUT, &data);
        if (ret < 0)
            PRINT_INFO("put failed...\n");

        avi_pos->offset = avi_pos->offset + 8 + buf_size;
    }


    if(avi_file->file)
    {
        fclose(avi_file->file);
        avi_file->file = NULL;
    }
    return STS_OK;

}
*/

#if 0
/**********************************************************************
函数描述：计算时间戳，并更新到 AviFrameInfo 的 node 的 timetick 参数中
入口参数：AviFrameInfo *p_frame: avi文件中一帧数据的相关信息的指针
返回值：  无
**********************************************************************/
static void cal_timetick_video(AviPBHandle *pPBHandle)
{
    int i;
    char filename[50];
//    char str[50];
    u64t tmp64;
    AVI_DMS_TIME t1;
    float tmp_f;
    int tmp32;
    time_t timep;
    struct tm *p;

    i = last_index_at(pPBHandle->file_path, '/');

    memset(filename, 0, sizeof(filename));
    strcpy(filename, pPBHandle->file_path + i + 1);

    tmp64 = avi_get_start_time(filename);

    u64t_to_time(&t1, tmp64);

    time(&timep);
    struct tm tt = {0};
    p = localtime_r(&timep, &tt);

    p->tm_year = t1.dwYear- 1900;
    p->tm_mon = t1.dwMonth- 1;
    p->tm_mday = t1.dwDay;
    p->tm_hour = t1.dwHour;
    p->tm_min = t1.dwMinute;
    p->tm_sec = t1.dwSecond;

    timep = mktime(p);

    tmp_f = 1000.0 / pPBHandle->fps;
    tmp32 = (int)((pPBHandle->video_no % pPBHandle->fps) * tmp_f);
    pPBHandle->node.timetick = (timep + pPBHandle->video_no / pPBHandle->fps) * 1000 + tmp32;

}

static void cal_timetick_audio(AviPBHandle *pPBHandle)
{
    int i;
    char filename[50];
//    char str[50];
    u64t tmp64;
    AVI_DMS_TIME t1;
    float tmp_f;
    int tmp32;
    time_t timep;
    struct tm *p;

    i = last_index_at(pPBHandle->file_path, '/');

    memset(filename, 0, sizeof(filename));
    strcpy(filename, pPBHandle->file_path + i + 1);

    tmp64 = avi_get_start_time(filename);

    u64t_to_time(&t1, tmp64);

    time(&timep);
    struct tm tt = {0};
    p = localtime_r(&timep, &tt);

    p->tm_year = t1.dwYear- 1900;
    p->tm_mon = t1.dwMonth- 1;
    p->tm_mday = t1.dwDay;
    p->tm_hour = t1.dwHour;
    p->tm_min = t1.dwMinute;
    p->tm_sec = t1.dwSecond;

    timep = mktime(p);

    int count = 8000/160;
    tmp_f = 1000.0 / count;
    tmp32 = (int)((pPBHandle->audio_no % count) * tmp_f);
    pPBHandle->node.timetick = (timep + pPBHandle->audio_no / count) * 1000 + tmp32;

}

static void cal_timetick(AviPBHandle *pPBHandle)
{
    int i;
    char filename[50];
//    char str[50];
    u64t tmp64;
    AVI_DMS_TIME t1;
    float tmp_f;
    int tmp32;
    time_t timep;
    struct tm *p;

    i = last_index_at(pPBHandle->file_path, '/');

    memset(filename, 0, sizeof(filename));
    strcpy(filename, pPBHandle->file_path + i + 1);

    tmp64 = avi_get_start_time(filename);

    u64t_to_time(&t1, tmp64);

    time(&timep);
    struct tm tt = {0};
    p = localtime_r(&timep, &tt);

    p->tm_year = t1.dwYear- 1900;
    p->tm_mon = t1.dwMonth- 1;
    p->tm_mday = t1.dwDay;
    p->tm_hour = t1.dwHour;
    p->tm_min = t1.dwMinute;
    p->tm_sec = t1.dwSecond;

    timep = mktime(p);

    int count = 8000/160 + pPBHandle->fps;
    tmp_f = 1000.0 / count;
    tmp32 = (int)((pPBHandle->audio_no % count) * tmp_f);
    pPBHandle->node.timetick = (timep + pPBHandle->audio_no / count) * 1000 + tmp32;

}
#endif

static int avi_read_data_to_buf(AviPBHandle *pPBHandle)
{
	if ((NULL == pPBHandle) || (NULL == pPBHandle->idx_array))
	{
        PRINT_ERR_MSG("pPBHandle is NULL!\n");
		return -1;

	}

    //避免定位后计算出来的no超出范围，如果超出范围读取最后一帧
    if (pPBHandle->no >= pPBHandle->index_count)
        pPBHandle->no = pPBHandle->index_count - 1;

    int ret;
    int bytes_loop = 0;
    int index_tmp = pPBHandle->no;
    pPBHandle->pb_buf_index = pPBHandle->no;
    pPBHandle->pb_buf_pos = 0;

    if(index_tmp + PB_N_LOOP > pPBHandle->index_count)
    {
        //pPBHandle->pb_buf_loop_n = pPBHandle->index_count - pPBHandle->pb_buf_index;
        //PRINT_INFO("last loop_n = %d, pb_buf_index = %d, index_count = %d\n",
        //        pPBHandle->pb_buf_loop_n, pPBHandle->pb_buf_index, pPBHandle->index_count);
        /* 加上最后一帧的大小 8 + pPBHandle->idx_array[4*(pPBHandle->index_count - 1)+3]， 最后一帧的trunk 12 */
        bytes_loop = pPBHandle->idx_array[4*(pPBHandle->index_count - 1)+2] - pPBHandle->idx_array[4*index_tmp+2] + 8 + pPBHandle->idx_array[4*(pPBHandle->index_count - 1)+3] + 12;
        pPBHandle->pb_buf_index = pPBHandle->index_count;
		pPBHandle->pb_buf_num = pPBHandle->index_count - index_tmp;
    }
    else
    {
    	int i = PB_N_LOOP;
    	while(bytes_loop < 640 * 1024)
    	{
			bytes_loop = pPBHandle->idx_array[4*(index_tmp+i)+2] - pPBHandle->idx_array[4*index_tmp+2];
			i ++;
		    if(index_tmp + i > pPBHandle->index_count)
		    {
		        //pPBHandle->pb_buf_loop_n = pPBHandle->index_count - pPBHandle->pb_buf_index;
		        //PRINT_INFO("last loop_n = %d, pb_buf_index = %d, index_count = %d\n",
		        //        pPBHandle->pb_buf_loop_n, pPBHandle->pb_buf_index, pPBHandle->index_count);
		        /* 加上最后一帧的大小 8 + pPBHandle->idx_array[4*(pPBHandle->index_count - 1)+3]， 最后一帧的trunk 12 */
		        bytes_loop = pPBHandle->idx_array[4*(pPBHandle->index_count - 1)+2] - pPBHandle->idx_array[4*index_tmp+2] + 8 + pPBHandle->idx_array[4*(pPBHandle->index_count - 1)+3] + 12;
		        pPBHandle->pb_buf_index = pPBHandle->index_count;
				pPBHandle->pb_buf_num = pPBHandle->index_count - index_tmp;
				break;
		    }
			else
			{
				pPBHandle->pb_buf_index = pPBHandle->no + i;
				pPBHandle->pb_buf_num = i;
			}
		    
		}
        //n_loop = PB_N_LOOP;
        
    }


    if(bytes_loop >= pPBHandle->pb_buf_size)
    {
        if(pPBHandle->pb_buf!= NULL)
            free(pPBHandle->pb_buf);
        pPBHandle->pb_buf = malloc(bytes_loop+60*1024);
        if(pPBHandle->pb_buf == NULL)
        {
            PRINT_ERR_MSG("Malloc read frame buffer(%dKB) failed",(bytes_loop+60*1024)>>10);
			pPBHandle->pb_buf_size = 0;
            return -1;
        }
        else
        {
            PRINT_INFO_MSG("Remalloc read frame buffer(%dKB) ",(bytes_loop+60*1024)>>10);
        }
        pPBHandle->pb_buf_size = (bytes_loop+60*1024);
    }

#if 0
    PRINT_INFO("\n\n pos = %d\n", pPBHandle->pb_buf_pos);
    PRINT_INFO(" offset= %d, bytes = %d \n", pPBHandle->idx_array[4*index_tmp+2], bytes_loop);
#endif
	if(pPBHandle->pb_buf == NULL)
	{
        PRINT_ERR_MSG("pb_buf is NULL!\n");
		pPBHandle->pb_buf_size = 0;
		return -1;
	}

    //memset(pPBHandle->pb_buf, 0, pPBHandle->pb_buf_size);
    fseek(pPBHandle->file, pPBHandle->idx_array[4*index_tmp+2], SEEK_SET);
    ret = fread(pPBHandle->pb_buf, 1, bytes_loop, pPBHandle->file);
    if(ret != bytes_loop)
    {
        PRINT_ERR_MSG("fread error. read %d bytes, cal %d byets\n", ret, bytes_loop);
		if(pPBHandle->pb_buf)
		{
			free(pPBHandle->pb_buf);
			pPBHandle->pb_buf = NULL;
		}
		
        return -1;
    }


    return 0;
}


/**********************************************************************
函数描述：得到一帧数据
入口参数：AviPBHandle *pPBHandle: avi文件中一帧数据的相关信息的指针
返回值：  <0: 错误
           0: 正常
**********************************************************************/
int avi_get_frame_loop(AviPBHandle *pPBHandle, char *p_buff, int *p_size)
{
    int ret;

    if(!pPBHandle)
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }

    /* 一次从AVI文件中读取 PB_N_LOOP 帧数据 */
    //if ((pPBHandle->no >= pPBHandle->pb_buf_index) || (pPBHandle->no <= (pPBHandle->pb_buf_index - PB_N_LOOP)))
    //printf("xxxx no:%d, pb_buf_index:%d, buffer_num:%d\n", pPBHandle->no, pPBHandle->pb_buf_index, pPBHandle->pb_buf_num);
    if ((pPBHandle->no >= pPBHandle->pb_buf_index -1) || (pPBHandle->no < (pPBHandle->pb_buf_index -pPBHandle->pb_buf_num)))
    {
        //printf("============ no = %d, pb_buf_index = %d \n", pPBHandle->no, pPBHandle->pb_buf_index);
        ret = avi_read_data_to_buf(pPBHandle);
        if(ret < 0)
        {
            PRINT_ERR_MSG("call avi_read_data_to_buf failed");
            return -1;
        }
    }

#if 0
    //只给视频，不给音频
    while(pb_avi_file.idx_array[4*p_frame->no] == MAKE_FOURCC('0','1','w','b'))
    //while(pb_avi_file.idx_array[4*p_frame->no] == MAKE_FOURCC('0','0','d','c'))
    {
        p_frame->no ++;

        if(p_frame->no >= pb_data_array_index)
        {
            PRINT_INFO("no = %d, pb_data_array_index = %d \n", p_frame->no, pb_data_array_index);
            ret = avi_read_file_data(p_frame);
            if(ret < 0)
            {
                PRINT_ERR_MSG("call avi_read_file_data failed");
                if(pb_avi_file.file)
                {
                    fclose(pb_avi_file.file);
                    pb_avi_file.file = NULL;
                }
                return -1;
            }
            i_pb = p_frame->no;
        }
    }
    pb_data_array_pos = pb_avi_file.idx_array[4*p_frame->no+2] - pb_avi_file.idx_array[4*i_pb+2];
#endif

    //PRINT_INFO("in avi_get_frame_loop, pPBHandle->no = %d.\n", pPBHandle->no);

    /* 依次从 pb_data_array 中解析出一帧数据 */
    //fcc32 为 00dc或者 01wb
    int fcc32 = avi_get_32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    if(fcc32 != pPBHandle->idx_array[4*pPBHandle->no])
    {
        PRINT_ERR_MSG("fcc is not equal.\n");
        //PRINT_ERR_MSG("streamid = 0x%08x, index fcc = 0x%08x \n", fcc32, pPBHandle->idx_array[4*pPBHandle->no]);
        //PRINT_ERR_MSG("pb_buf_pos:%d, no=%d,offset :0x%x ,size:0x%x\n", pPBHandle->pb_buf_pos, pPBHandle->no,pPBHandle->idx_array[4*pPBHandle->no+2],pPBHandle->idx_array[4*pPBHandle->no+3]);

        //pPBHandle->no ++;
        //return -2;
        return  -1;
    }
    else
    {
        //usleep(3*1000);
        //PRINT_INFO("xx no=%d,offset :0x%x ,size:0x%x,video count = %d, all count =%d\n",
         //   pPBHandle->no,pPBHandle->idx_array[4*pPBHandle->no+2],pPBHandle->idx_array[4*pPBHandle->no+3],pPBHandle->video_count, pPBHandle->index_count);
    }
    pPBHandle->pb_buf_pos += 4;
    //PRINT_INFO("fcc32 = 0x%08x\n", fcc32);

    //frame_size 为 这一帧数据的大小
    int frame_size = avi_get_32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    if(frame_size != pPBHandle->idx_array[4*pPBHandle->no+3])
    {
        PRINT_ERR_MSG("size is not equal.\n");
        PRINT_INFO("buf_size = %d, index size = %d \n", frame_size, pPBHandle->idx_array[4*pPBHandle->no+3]);
        return -1;
    }
    pPBHandle->pb_buf_pos += 4;
    *p_size = frame_size;


    //p_buff 得到这一帧数据
    if(frame_size > ONE_FRAME_BUF_SIZE)
    {
        PRINT_ERR_MSG("ONE_FRAME_BUF_SIZE is too small, frame size = %d.\n", frame_size);
        return -1;
    }

    pPBHandle->node.one_frame_buf = pPBHandle->pb_buf + pPBHandle->pb_buf_pos;

    pPBHandle->pb_buf_pos += frame_size;
	pPBHandle->pb_buf_pos += frame_size % 2;


    /* get time stamp*/
    int tmp32 = avi_get_32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    if(tmp32 != MAKE_FOURCC('J','U','N','K'))
    {
        PRINT_ERR_MSG("fcc junk error.\n");
        return  -1;
    }
    pPBHandle->pb_buf_pos += 4;
    //PRINT_INFO("JUNK fcc = 0x%08x\n", tmp32);

    tmp32 = avi_get_32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    if(tmp32 != 4)
    {
        PRINT_ERR_MSG("junk size is not 4.\n");
        return -1;
    }
    pPBHandle->pb_buf_pos += 4;
    //PRINT_INFO("JUNK size = %d\n", tmp32);

    u32t timetick = avi_get_u32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    pPBHandle->pb_buf_pos += 4;
    //PRINT_INFO("JUNK timetick = %u\n", timetick);

    pPBHandle->node.frame_type = pPBHandle->idx_array[4*pPBHandle->no+1];
    pPBHandle->node.offset = pPBHandle->idx_array[4*pPBHandle->no+2];
    pPBHandle->node.size = pPBHandle->idx_array[4*pPBHandle->no+3];  //not use

    if(fcc32 == MAKE_FOURCC('0','0','d','c'))
    {
        //cal_timetick_video(p_frame);
        pPBHandle->video_no ++;
        pPBHandle->node.timetick = timetick;
        pPBHandle->pb_timetick_tmp = timetick;
    }
    else
    {
        //cal_timetick_audio(p_frame);
        pPBHandle->audio_no ++;
        pPBHandle->node.timetick = pPBHandle->pb_timetick_tmp;
        //p_frame->node.timetick = timetick;
    }

    //PRINT_INFO("no = %d, frame_size = %d\n", pPBHandle->no, *p_size);
    pPBHandle->no ++;
    return 0;
}

/**********************************************************************
函数描述：得到一帧数据
入口参数：AviPBHandle *pPBHandle: avi文件中一帧数据的相关信息的指针
返回值：  <0: 错误
           0: 正常
**********************************************************************/
int avi_get_frame_by_no(AviPBHandle *pPBHandle, char *p_buff, int *p_size)
{
    int ret;

    if(!pPBHandle)
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }

    /* 计算offset 和 size */
    int frame_size = pPBHandle->idx_array[4*pPBHandle->no+3];
    //PRINT_INFO("frame_size : %d\n", frame_size);
    int one_len = 8+frame_size+12;

    /* 读入数据 */

    if(one_len >= pPBHandle->pb_buf_size)
    {
        if(pPBHandle->pb_buf!= NULL)
            free(pPBHandle->pb_buf);
        pPBHandle->pb_buf = malloc(one_len+60*1024);
        if(pPBHandle->pb_buf == NULL)
        {
            PRINT_ERR_MSG("Malloc read frame buffer(%dKB) failed",(one_len+60*1024)>>10);
            return -1;
        }
        else
        {
            PRINT_INFO_MSG("REmalloc read frame buffer(%dKB) ",(one_len+60*1024)>>10);
        }
        pPBHandle->pb_buf_size = (one_len+60*1024);
    }


    int offset = pPBHandle->idx_array[4*pPBHandle->no+2];
    fseek(pPBHandle->file, offset, SEEK_SET);
    ret = fread(pPBHandle->pb_buf, 1, one_len, pPBHandle->file);
    if(ret != one_len)
    {
        PRINT_ERR_MSG("fread error. read %d bytes, cal %d byets\n", ret, one_len);
        return -1;
    }

    // 读fcc
    pPBHandle->pb_buf_pos = 0;
    int fcc32 = avi_get_32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    if(fcc32 != pPBHandle->idx_array[4*pPBHandle->no])
    {
        PRINT_ERR_MSG("fcc is not equal.\n");
        PRINT_INFO("streamid = 0x%08x, index fcc = 0x%08x \n", fcc32, pPBHandle->idx_array[4*pPBHandle->no]);
        return  -1;
    }
	#if 0
    if(fcc32 != MAKE_FOURCC('0','0','d','c'))
    {
        PRINT_ERR_MSG("fcc error.\n");
        return  -1;
    }
	#endif
    pPBHandle->pb_buf_pos += 4;

    // 读frame_size
    frame_size = avi_get_32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    if(frame_size != pPBHandle->idx_array[4*pPBHandle->no+3])
    {
        PRINT_ERR_MSG("size is not equal.\n");
        PRINT_INFO("buf_size = %d, index size = %d \n", frame_size, pPBHandle->idx_array[4*pPBHandle->no+3]);
        return -1;
    }
    pPBHandle->pb_buf_pos += 4;
    *p_size = frame_size;

    //p_buff 得到这一帧数据
    if(frame_size > ONE_FRAME_BUF_SIZE)
    {
        PRINT_ERR_MSG("ONE_FRAME_BUF_SIZE is too small, frame size = %d.\n", frame_size);
        return -1;
    }
    //memcpy(p_buff, (char *)(pPBHandle->pb_buf + pPBHandle->pb_buf_pos), frame_size);
    pPBHandle->node.one_frame_buf = (char *)(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    pPBHandle->pb_buf_pos += frame_size;

    /* get time stamp*/
    int tmp32 = avi_get_32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    if(tmp32 != MAKE_FOURCC('J','U','N','K'))
    {
        PRINT_ERR_MSG("fcc junk error.\n");
        return  -1;
    }
    pPBHandle->pb_buf_pos += 4;
    //PRINT_INFO("JUNK fcc = 0x%08x\n", tmp32);

    tmp32 = avi_get_32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    if(tmp32 != 4)
    {
        PRINT_ERR_MSG("junk size is not 4.\n");
        return -1;
    }
    pPBHandle->pb_buf_pos += 4;
    //PRINT_INFO("JUNK size = %d\n", tmp32);

    u32t timetick = avi_get_u32(pPBHandle->pb_buf + pPBHandle->pb_buf_pos);
    pPBHandle->pb_buf_pos += 4;
    //PRINT_INFO("JUNK timetick = %u\n", timetick);

    pPBHandle->node.frame_type = pPBHandle->idx_array[4*pPBHandle->no+1];
    pPBHandle->node.offset = pPBHandle->idx_array[4*pPBHandle->no+2];
    pPBHandle->node.size = pPBHandle->idx_array[4*pPBHandle->no+3];  //not use

    if(fcc32 == MAKE_FOURCC('0','0','d','c'))
    {
        //cal_timetick_video(p_frame);
        pPBHandle->video_no ++;
        pPBHandle->node.timetick = timetick;
        pPBHandle->pb_timetick_tmp = timetick;
    }
    else
    {
        //cal_timetick_audio(p_frame);
        pPBHandle->audio_no ++;
        pPBHandle->node.timetick = pPBHandle->pb_timetick_tmp;
        //p_frame->node.timetick = timetick;
    }

    //PRINT_INFO("no = %d, frame_size = %d\n", pPBHandle->no, *p_size);
    pPBHandle->no ++;
    pPBHandle->pb_buf_index = pPBHandle->no;
    return 0;
}


/**********************************************************************
函数描述：返回 n1 - n2 的值
入口参数：u64t n1: 操作数1
          u64t n2: 操作数2
返回值：  n1 - n2 的值，可正可负，单位为秒
**********************************************************************/
static int cal_del(u64t n1, u64t n2)
{
    int r;

    time_t timep1, timep2;
    struct tm *p1;
    struct tm *p2;


    AVI_DMS_TIME t1, t2;
    memset(&t1, 0, sizeof(t1));
    memset(&t2, 0, sizeof(t2));
    u64t_to_time(&t1, n1);
    u64t_to_time(&t2, n2);

    time(&timep1);
    struct tm tm1 = {0};
    p1 = localtime_r(&timep1, &tm1);
    time(&timep2);
    struct tm tm2 = {0};
    p2 = localtime_r(&timep2, &tm2);

    p1->tm_year = t1.dwYear- 1900;
    p1->tm_mon = t1.dwMonth- 1;
    p1->tm_mday = t1.dwDay;
    p1->tm_hour = t1.dwHour;
    p1->tm_min = t1.dwMinute;
    p1->tm_sec = t1.dwSecond;

    p2->tm_year = t2.dwYear- 1900;
    p2->tm_mon = t2.dwMonth- 1;
    p2->tm_mday = t2.dwDay;
    p2->tm_hour = t2.dwHour;
    p2->tm_min = t2.dwMinute;
    p2->tm_sec = t2.dwSecond;

    timep1 = mktime(p1);
    timep2 = mktime(p2);

    r = timep1 - timep2;
    return r;
}


/**********************************************************************
函数描述：返回 n1 - n2 的值
入口参数：AviPBHandle *pPBHandle
返回值：  计算文件的总时长，单位为秒
**********************************************************************/
int cal_pb_all_time(AviPBHandle *pPBHandle)
{
    //PRINT_INFO("in cal_pb_all_time, file path: %s\n", pPBHandle->file_path);
    char file_name[128];
    memset(file_name, 0 ,sizeof(file_name));
    int i = last_index_at(pPBHandle->file_path, '/');
    strcpy(file_name, pPBHandle->file_path + i + 1);
    //PRINT_INFO("in cal_pb_all_time, file name: %s\n", file_name);


    u64t time_start_64 = avi_get_start_time(file_name);
    u64t time_stop_64 = avi_get_stop_time(file_name);
    //PRINT_INFO("in cal_pb_all_time, start time: %llu\n", time_start_64);
    //PRINT_INFO("in cal_pb_all_time, stop  time: %llu\n", time_stop_64);

    int ret = cal_del(time_stop_64, time_start_64);
    //PRINT_INFO("in cal_pb_all_time, all time: %d\n", ret);

    return ret;
}


#if 1
/**********************************************************************
函数描述：返回 n1 - n2 的值
入口参数：AviPBHandle *pPBHandle,AVI_DMS_TIME *ptime
返回值：  计算指定时间到文件开始时间的长度
**********************************************************************/
int cal_pb_seek_time(AviPBHandle *pPBHandle,AVI_DMS_TIME *ptime)
{
    if(NULL == pPBHandle)
    {
        PRINT_ERR("pPBHandle is null!\n");
        return -1;
    }
    //PRINT_INFO("in cal_pb_all_time, file path: %s\n", pPBHandle->file_path);

    int r;
    time_t timep1, timep2 ,timep3;
    struct tm *p1,*p2,*p3;

    int ret = 0;
    char file_name[128] = {0};
    u64t time_start_64 = 0;
    u64t time_stop_64 = 0;
    u64t cur_time_64 = time_to_u64t(ptime);

    if (pPBHandle->list)
        pPBHandle->list->index = 0;
    while (1)
    {
        //PRINT_INFO("pb to check file : %s, cur_time:%llu\n", pPBHandle->file_path, cur_time_64);
        int i = last_index_at(pPBHandle->file_path, '/');
        strcpy(file_name, pPBHandle->file_path + i + 1);
        //PRINT_INFO("in cal_pb_all_time, file name: %s\n", file_name);
        time_start_64 = avi_get_start_time(file_name);
        time_stop_64 = avi_get_stop_time(file_name);

        //PRINT_INFO("----------->time_start_64:%llu time_stop_64:%llu cur_time_64:%llu\n",
        //    time_start_64, time_stop_64, cur_time_64);

        if ((cur_time_64 >= time_start_64) && (cur_time_64 <= time_stop_64))
        {
            PRINT_INFO("cur_time_64:%llu, find file: %s\n", cur_time_64, pPBHandle->file_path);
            break;
        }

        if (pPBHandle->list == NULL)
        {
            PRINT_ERR("pPBHandle list is null!\n");
            return -1;
        }
        else
        {
            //读下一个文件
            FILE_NODE node;
			memset(&node,0,sizeof(FILE_NODE));
            ret = get_file_node(pPBHandle->list, &node);
            if (ret != DMS_NET_FILE_SUCCESS) {
                PRINT_ERR("get_file_node failed, ret = %d\n", ret);
                return -1;
            }
            //print_node(&node);
            strcpy(pPBHandle->file_path, node.path);
        }
    }
    //PRINT_INFO("find file -- pb open file : %s\n", pPBHandle->file_path);


    AVI_DMS_TIME t1 = {0};
    u64t_to_time(&t1, time_start_64);
    AVI_DMS_TIME t2 = {0};
    u64t_to_time(&t2, time_stop_64);

    #if 0
    PRINT_INFO("----------->start %lu-%lu-%lu %lu:%lu:%lu\n",
        t1.dwYear,t1.dwMonth,t1.dwDay,t1.dwHour,t1.dwMinute,t1.dwSecond);

    PRINT_INFO("----------->stop %lu-%lu-%lu %lu:%lu:%lu\n",
        t2.dwYear,t2.dwMonth,t2.dwDay,t2.dwHour,t2.dwMinute,t2.dwSecond);

    PRINT_INFO("----------->setpos %lu-%lu-%lu %lu:%lu:%lu\n",
        ptime->dwYear,ptime->dwMonth,ptime->dwDay,ptime->dwHour,ptime->dwMinute,ptime->dwSecond);
    #endif

    /*初始化timep1*/
    time(&timep1);
    struct tm tm1 = {0};
    p1 = localtime_r(&timep1, &tm1);

    /*初始化timep2*/
    time(&timep2);
    struct tm tm2 = {0};
    p2 = localtime_r(&timep2, &tm2);

    /*初始化timep3*/
    time(&timep3);
    struct tm tm3 = {0};
    p3 = localtime_r(&timep3, &tm3);

    p1->tm_year = t1.dwYear- 1900;
    p1->tm_mon = t1.dwMonth- 1;
    p1->tm_mday = t1.dwDay;
    p1->tm_hour = t1.dwHour;
    p1->tm_min = t1.dwMinute;
    p1->tm_sec = t1.dwSecond;

    p2->tm_year = ptime->dwYear- 1900;
    p2->tm_mon = ptime->dwMonth- 1;
    p2->tm_mday = ptime->dwDay;
    p2->tm_hour = ptime->dwHour;
    p2->tm_min = ptime->dwMinute;
    p2->tm_sec = ptime->dwSecond;

    p3->tm_year = t2.dwYear- 1900;
    p3->tm_mon = t2.dwMonth- 1;
    p3->tm_mday = t2.dwDay;
    p3->tm_hour = t2.dwHour;
    p3->tm_min = t2.dwMinute;
    p3->tm_sec = t2.dwSecond;

    timep1 = mktime(p1);
    timep2 = mktime(p2);
    timep3 = mktime(p3);
    #if 0
    PRINT_INFO("----------->timep1 %d-%d-%d %d:%d:%d\n",
        p1->tm_year,p1->tm_mon,p1->tm_mday,p1->tm_hour,p1->tm_min,p1->tm_sec);

    PRINT_INFO("timep1:%ld\n",timep1);

    PRINT_INFO("----------->timep2 %d-%d-%d %d:%d:%d\n",
        p2->tm_year,p2->tm_mon,p2->tm_mday,p2->tm_min,p2->tm_min,p2->tm_sec);

    PRINT_INFO("timep2:%ld\n",timep2);

    PRINT_INFO("----------->timep3 %d-%d-%d %d:%d:%d\n",
        p3->tm_year,p3->tm_mon,p3->tm_mday,p3->tm_hour,p3->tm_min,p3->tm_sec);

    PRINT_INFO("timep3:%ld\n",timep3);

    PRINT_INFO("----------->\n");
    #endif

    r = timep2 - timep1;

    PRINT_INFO("----------->SEEK pos r:%d\n", r);
    if((r < 0) || r > timep3 - timep1)
    {
        PRINT_ERR("SEEK pos of file %d err,should between %llu to %llu !\n",r,time_start_64,time_stop_64);
        return -1;
    }

    return r;
}
#endif

/**********************************************************************
函数描述：将时间戳 timetick 转换成 AVI_DMS_TIME 类型的数据结构
入口参数：AVI_DMS_TIME *ptime: DMS_TIME 类型的指针，记录时间
          int timetick: 时间戳，毫秒为单位
返回值：  无
**********************************************************************/
void timetick_to_time(AVI_DMS_TIME *ptime, unsigned int timetick)
{
    struct tm *p;
    long timep = timetick /1000;

    //PRINT_INFO("timep4 = %d \n", timep);

    struct tm t1 = {0};
    p = gmtime_r(&timep, &t1);


    ptime->dwYear = p->tm_year + 1900;
    ptime->dwMonth = p->tm_mon + 1;
    ptime->dwDay = p->tm_mday;
    ptime->dwHour = p->tm_hour;
    ptime->dwMinute = p->tm_min;
    ptime->dwSecond = p->tm_sec;


}


AviPBHandle *create_pb_handle(void)
{
    AviPBHandle *pPBHandle = NULL;
    pPBHandle = (AviPBHandle *)malloc(sizeof(AviPBHandle));
    if(NULL == pPBHandle)
    {
        PRINT_ERR();
        return NULL;
    }
	
    memset(pPBHandle, 0, sizeof(AviPBHandle));
    pPBHandle->list      = NULL;
    pPBHandle->file      = NULL;
    pPBHandle->idx_array = NULL;
    pPBHandle->pb_buf    = NULL;

    return pPBHandle;
}


#if 0
int init_frameinfo(AviFrameInfo *p_frame)
{
    int ret;
    FILE_NODE node;
    char seek_time[20];
    u64t start_time;
    AviInitParam param;

    memset(&node, 0, sizeof(FILE_NODE));
    ret = get_file_node(p_frame->list, &node);

    if(ret == GK_NET_NOMOREFILE)
        return ret;


    if(ret != GK_NET_FILE_SUCCESS)
    {
        PRINT_INFO("ret = %d\n", ret);
        PRINT_ERR();
        return -1;
    }


    memset(&pb_avi_file, 0, sizeof(AviFile));
    memset(&param, 0 ,sizeof(AviInitParam));
    ret = avi_pb_open(node.path, &pb_avi_file, &param);
    if(ret < 0)
    {
        if(pb_avi_file.file)
        {
            fclose(pb_avi_file.file);
            pb_avi_file.file = NULL;
        }
        avi_pb_close(p_frame);

        return -1;
    }


    if(p_frame->list->start > node.start)
    {
        start_time = p_frame->list->start;
        memset(seek_time, 0, sizeof(seek_time));
        sprintf(seek_time, "%llu", start_time);
        //avi_pb_seek_by_time(&pb_avi_file, &param, seek_time, p_frame);
    }
    else
    {
        p_frame->no = 0;
    }
    set_frame_info(&pb_avi_file, &param, p_frame);

    if(p_frame->list->stop < node.stop)
    {
        int tmp1 = cal_del(node.stop, p_frame->list->stop);

        p_frame->video_count = p_frame->video_count - tmp1 * p_frame->fps;

    }
    return 0;
}
#endif

int avi_pb_slide_to_next_file(AviPBHandle *pPBHandle)
{
    if(NULL == pPBHandle)
    {
        PRINT_ERR("param error.\n");
        return -1;
    }

    //单文件
    if(pPBHandle->list == NULL) //pb by name
        return -1;
    else //多文件
    {
        int ret = 0;
        FILE_NODE node;
		memset(&node,0,sizeof(FILE_NODE));
        ret = get_file_node(pPBHandle->list, &node);
        if (ret != DMS_NET_FILE_SUCCESS) {
            PRINT_ERR("get_file_node failed, ret = %d\n", ret);
            return -1;
        }
        print_node(&node);
        //先关闭，再打开
        ret = avi_pb_open(node.path, pPBHandle);
        if (ret < 0) {
            PRINT_ERR("avi_pb_open %s error.\n", node.path);
            return -1;
        }
    }

    return 0;
}

int avi_pb_slide_to_next_file_by_time(AviPBHandle *pPBHandle, u64t cur_time_64)
{
    if(NULL == pPBHandle)
    {
        PRINT_ERR("param error.\n");
        return -1;
    }


    return 0;
}


/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息, 计算出该定位时间是第几帧
          数据，并更新到 AviFrameInfo 的 no 参数中
入口参数：AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
          char *pBuff: 向该缓存区位置指针存入数据
          int *pSize: 得到这帧数据的大小
返回值：  -1: 错误
           0: 正常
          GK_NET_NOMOREFILE: 播放结束(GK_NET_NOMOREFILE值为1003)
**********************************************************************/
int avi_pb_get_frame(AviPBHandle *pPBHandle, char *p_buff, int *p_size)
{
    int ret;
    int sleep_um = 0;
    //正在升级，退出录像
    if(netcam_get_update_status() != 0)
    {
        PRINT_ERR("call avi_get_frame_loop, exit for updating.");
        return  -1;
    }

    if(pPBHandle->no >= pPBHandle->index_count)
    {
        ret = avi_pb_slide_to_next_file(pPBHandle);
        if (ret != 0)
        {
            PRINT_ERR("avi_pb_slide_to_next_file error.");
            return -1;
        }
    }

#if PB_DEBUG
    PRINT_INFO("get frame, no = %d, video count = %d, all count =%d \n", pPBHandle->no, pPBHandle->video_count, pPBHandle->index_count);
#endif

    if (avi_pb_speed == 1)
    {
        ret = avi_get_frame_loop(pPBHandle, p_buff, p_size);
        if(ret < 0)
        {

            PRINT_ERR("call avi_get_frame_loop error.");
            //return -1;
            return ret;
        }

        /* 控制时间发送, ms */
        //static unsigned int last_timetick = 0;
        if (pPBHandle->last_tick == 0)
            pPBHandle->last_tick = pPBHandle->node.timetick;
        else
        {
            int interval = pPBHandle->node.timetick - pPBHandle->last_tick;

            //PRINT_INFO("XXX interval %d",interval);
            if ((interval > 0) && (interval < 200))
            {
                //gk_cms_msleep(interval);
                sleep_um = 800*1000/pPBHandle->fps ;
                //printf("sleep_um : %d\n",sleep_um);
                usleep(sleep_um);
            }
            else if (interval < 0)
            {
                PRINT_ERR("timetick error. now timetick:%u last timetick: %u\n", pPBHandle->node.timetick, pPBHandle->last_tick);
            }
            else if(0 == interval)//one  video frame devide into number of pices，each pices has the same timetick
            {
                //usleep(1*1000);
            }
            pPBHandle->last_tick = pPBHandle->node.timetick;
        }
    }
    else
    {
        //PRINT_INFO("gop:%d fps:%d\n", pPBHandle->gop, pPBHandle->fps);

        /* 如果是I帧，则跳出 */
        while (1)
        {
            if (pPBHandle->no >= pPBHandle->index_count)
            {
                PRINT_INFO("last frame. no = %d, all count =%d \n", pPBHandle->no, pPBHandle->index_count);
                pPBHandle->no --;
                break;
            }

            if ((pPBHandle->idx_array[4 * pPBHandle->no] == MAKE_FOURCC('0','0','d','c'))
            && (pPBHandle->idx_array[4 * pPBHandle->no + 1] == 0x11))
            {
                break;
            }
            pPBHandle->no ++;
        }

        ret = avi_get_frame_by_no(pPBHandle, p_buff, p_size);
        if(ret < 0)
        {
            PRINT_ERR("call avi_get_frame_by_no error.");
            return -1;
        }

        /* 控制时间发送, ms */
        int interval = pPBHandle->gop * 1000 / (pPBHandle->fps * avi_pb_speed);
        if (interval > 0)
            gk_cms_msleep(interval);

    }

    return 0;
 }

/**********************************************************************
函数描述：设置avi文件播放的速度，目前只支持快进
入口参数：int speed: 播放速度，如 1,2,4,8,16,32
返回值：  <0: 错误
           0: 正常
**********************************************************************/
int avi_pb_set_speed(int speed)
{
    PRINT_INFO("avi_pb_set_speed = %d\n", speed);
    //pthread_mutex_lock(&avi_pb_speed_x);
    avi_pb_speed = speed;
    //pthread_mutex_unlock(&avi_pb_speed_x);

    return 0;
}

int avi_pb_start(void)
{
    PRINT_INFO("avi playback start.\n");

    avi_pb_set_speed(1);

 /*   int ret = 0;
   // while(ret != GK_NET_NOMOREFILE)
   // {
        while(avi_flag_pause)
            sleep(2);
        PRINT_INFO("in start 2.\n");
        ret = avi_pb_get_frame(p_frame, pBuff, pSize);
        if(ret < 0)
            PRINT_INFO("in start 3.\n");
        else
            PRINT_INFO("in start 4.\n");
        p_frame->no = p_frame->no + speed - 1;
        PRINT_INFO("in start 5. no = %d\n", p_frame->no);
   // }
*/
    return 0;
}

/**********************************************************************
函数描述：从头开始重新播放avi文件
入口参数：AviFrameInfo *p_frame: avi文件中一帧数据的控制句柄
返回值：  <0: 错误
           0: 正常
**********************************************************************/
int avi_pb_restart(AviPBHandle *pPBHandle)
{
    if(pPBHandle->list == NULL)
    {
        pPBHandle->no = 0;
        avi_pb_set_speed(1);
    }

    pPBHandle->list->index = 0;
    //init_frameinfo(p_frame);
    avi_pb_set_speed(1);

    //avi_pb_start();

    return 0;
}

int avi_pb_pause(void)
{
    avi_flag_pause = (avi_flag_pause + 1) % 2;

    return 0;
}

/**********************************************************************
函数描述：关闭帧数据的控制句柄
入口参数：AviFrameInfo *p_frame: avi文件中一帧数据的控制句柄
返回值：  <0: 错误
           0: 正常
**********************************************************************/
int avi_pb_close(AviPBHandle *pPBHandle)
{
    if(!pPBHandle)
    {
        PRINT_ERR_MSG("parameter is error.\n");
        return -1;
    }
    PRINT_INFO("avi_pb_close.\n");
#if 0
    //安普对接的原因，加个open flag，防止多次打开
    if(pPBHandle->pb_open_flag == 0)
    {
        PRINT_INFO("call avi_pb_close() twice. \n");
        return -1;
    }
    pPBHandle->pb_open_flag = 0;
#endif

    //close & free
    if(pPBHandle->list)
    {
        //PRINT_INFO("avi_pb_close close list.\n");
        search_close(pPBHandle->list);
        pPBHandle->list = NULL;
    }
    if(pPBHandle->file)
    {
        //PRINT_INFO("avi_pb_close close file.\n");
        fclose(pPBHandle->file);
        pPBHandle->file = NULL;
    }
    if(pPBHandle->idx_array)
    {
        //PRINT_INFO("avi_pb_close close idx_array.\n");
        free(pPBHandle->idx_array);
        pPBHandle->idx_array = NULL;
    }
    if(pPBHandle->pb_buf)
    {
        //PRINT_INFO("avi_pb_close close pb_buf.\n");
        free(pPBHandle->pb_buf);
        pPBHandle->pb_buf = NULL;
        pPBHandle->pb_buf_size = 0;
    }
    //xxx should free pPBHandle
    free(pPBHandle);
    pPBHandle = NULL;

    //pthread_mutex_destroy(&avi_pb_speed_x);

    return 0;
}



int avi_pb_get_pos(AviPBHandle *pPBHandle)
{
   int pos;
   pos = (pPBHandle->no * 100) / pPBHandle->video_count;
   return pos;
}

int avi_pb_set_pos(AviPBHandle *pPBHandle, int *pTimePos)
{
    int ret;
    int time_pos = *pTimePos;
    int all_time = cal_pb_all_time(pPBHandle);
    PRINT_INFO("set pos, time_pos = %d, all time = %d\n", time_pos, all_time);

    /*
    int video_no = (int)(pPBHandle->video_count * time_pos / all_time);
    int no = 0;

    PRINT_INFO("set pos, video_no = %d, video_count = %d\n", video_no, pPBHandle->video_count);

    while(video_no)
    {
        int fcc = pPBHandle->idx_array[4*no];
        if(fcc == MAKE_FOURCC('0','0','d','c'))
            video_no --;

        no ++;
    }
    */

    pPBHandle->no = (int)(pPBHandle->index_count * time_pos / all_time);
    //第一帧是i帧
    while(pPBHandle->no < pPBHandle->index_count)
    {
        if ((pPBHandle->idx_array[4*pPBHandle->no] == MAKE_FOURCC('0','0','d','c'))
            && (pPBHandle->idx_array[4*pPBHandle->no+1] == 0x11))
            break;
        else
            pPBHandle->no ++;
    }
    PRINT_INFO("set pos, get no = %d, index count = %d\n", pPBHandle->no, pPBHandle->index_count);

    ret = avi_read_data_to_buf(pPBHandle);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_read_data_to_buf failed");
        return -1;
    }

    return 0;
}

int avi_pb_seek(AviPBHandle *pPBHandle, int seek_no)
{
    int ret;

    pPBHandle->no = seek_no;

    //第一帧是i帧
    while(pPBHandle->no < pPBHandle->index_count)
    {
        if ((pPBHandle->idx_array[4*pPBHandle->no] == MAKE_FOURCC('0','0','d','c'))
            && (pPBHandle->idx_array[4*pPBHandle->no+1] == 0x11))
            break;
        else
            pPBHandle->no ++;
    }
    PRINT_INFO("set pos, get no = %d, index count = %d\n", pPBHandle->no, pPBHandle->index_count);

    ret = avi_read_data_to_buf(pPBHandle);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_read_data_to_buf failed");
        return -1;
    }

    return 0;
}


int avi_pb_get_time(AviPBHandle *pPBHandle, struct tm *p_nowtime)
{
    char file_name[128];
    int i;
    char str_tmp[20];

    int    y1,m1,d1,h1,n1,s1;  //start
    int duration;
    time_t timep;
    struct tm *p;

    i = last_index_at(pPBHandle->file_path, '/');

    memset(file_name, 0 ,sizeof(file_name));
    strcpy(file_name, pPBHandle->file_path + i + 1);

    avi_substring(str_tmp, file_name, 4, 4+4);
    y1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 8, 8+2);
    m1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 10, 10+2);
    d1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 12, 12+2);
    h1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 14, 14+2);
    n1 = atoi(str_tmp);
    avi_substring(str_tmp, file_name, 16, 16+2);
    s1 = atoi(str_tmp);

    duration = (int) pPBHandle->video_no / pPBHandle->fps;

    time(&timep);
    struct tm tm1 = {0};
    p = localtime_r(&timep, &tm1);

    p->tm_year = y1 - 1900;
    p->tm_mon = m1 - 1;
    p->tm_mday = d1;
    p->tm_hour = h1;
    p->tm_min = n1;
    p->tm_sec = s1;

    timep = mktime(p);

    timep += duration;

    localtime_r(&timep, p_nowtime);

    return 0;
}


int avi_mv_pic(char *name) //name = /tmp/%04d%02d%02d_%02d%02d%02d.jpg
{
    int ret;
    char pic_date[10];
    memset(pic_date, 0, sizeof(pic_date));
    avi_substring(pic_date, name, 5, 5+8);

    char pic_dir[30];
    memset(pic_dir, 0, sizeof(pic_dir));
    sprintf(pic_dir, "%s/pic_snap", GRD_SD_MOUNT_POINT);
    if(access(pic_dir, F_OK) != 0 ) //日期目录不存在，则创建
    {
        ret = mkdir(pic_dir, 0777);
        if(ret < 0)
        {
            PRINT_ERR_MSG("mkdir %s failed\n", pic_dir);
            return -1;
        }
    }
    memset(pic_dir, 0, sizeof(pic_dir));
    sprintf(pic_dir, "%s/pic_snap/%s", GRD_SD_MOUNT_POINT, pic_date);
    if(access(pic_dir, F_OK) != 0 ) //日期目录不存在，则创建
    {
        ret = mkdir(pic_dir, 0777);
        if(ret < 0)
        {
            PRINT_ERR_MSG("mkdir %s failed\n", pic_dir);
            return -1;
        }
    }

    char cmd[128];
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "mv  %s  %s", name, pic_dir);
    new_system_call(cmd);

    return 0;
}







