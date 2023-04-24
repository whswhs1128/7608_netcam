/*!
*****************************************************************************
** FileName     : avi_common.h
**
** Description  : common file
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create       : 2015-8-11, create
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef __AVI_COMMON_H__
#define __AVI_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <assert.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>

#include "common.h"

typedef unsigned long long 	u64t;
typedef long long 			s64t;
typedef	unsigned int		u32t;
typedef	int					s32t;


#define __AVI_LOG(syntax,fmt,arg...) \
	do{\
		char bname[64];\
		strncpy(bname, __FILE__, sizeof(bname));\
		printf("\033[%sm[%16s:%4d]\033[0m "fmt"\r\n", syntax, basename(bname), __LINE__, ##arg);\
	}while(0)

#define PRINT_ERR_MSG(fmt, arg...)  __AVI_LOG("1;31",fmt,##arg)

#define PRINT_WARN_MSG(fmt, arg...)  __AVI_LOG("1;33",fmt,##arg)

#define PRINT_INFO_MSG(fmt, arg...)  __AVI_LOG("1;36",fmt,##arg)



#define REC_BUF_SIZE (600*1024)

#define AUDIO_ARRAY_SIZE 20000

//ONE_FRAME_NODE frame_node;

/* AVI flag, defined by aviriff.h */
#define AVIF_HASINDEX        0x00000010 //表明AVI文 件包含一个index。
#define AVIF_MUSTUSEINDEX    0x00000020 //表明应用程序需要使用index，而不是物理上的顺序，来定义数据的展现顺序。例如，该标志可以用于创建一个编辑用的帧列表。
#define AVIF_ISINTERLEAVED   0x00000100 //表明AVI文 件是交叉的。
#define AVIF_TRUSTCKTYPE     0x00000800 // Use CKType to find key frames
#define AVIF_WASCAPTUREFILE  0x00010000 //表明该文件是一个用于捕获实时视频的，而特别分配的AVI 文 件。如果一个文件设置了该标志，在用户写该文件之前，应用程序应该发出警告，因为用户可能会对该文件进行碎片整理。
#define AVIF_COPYRIGHTED     0x00020000 //表明AVI文 件包含了版权数据和软件。如果设置了改标志，将不允许软件对该数据进行拷贝。

/* record mode */
#define RECORD_FIXED_SIZE         0X1
#define RECORD_FIXED_DURATION     0X2
#define RECORD_DELETE_OLD_FILES   0X4

#define RECORD_IS_FIXED_SIZE(x)          (x&0x1)
#define RECORD_IS_FIXED_DURATION(x)      (x&0x2)
#define RECORD_IS_DELETE(x)              (x&0x4)

/* FOURCC */
#define MAKE_FOURCC(a,b,c,d)         (int)((a)|(b)<<8|(c)<<16|(d)<<24)

//video
#define AVI_TYPE_H264     MAKE_FOURCC('H','2','6','4')
#define AVI_TYPE_H265     MAKE_FOURCC('H','2','6','5')
#define AVI_TYPE_MPEG4    MAKE_FOURCC('D','I','V','X')
#define AVI_TYPE_MJPEG    MAKE_FOURCC('M','J','P','G')
#define AVI_TYPE_GMTAG    MAKE_FOURCC('G','M','T','G')
//audio
#define AVI_TYPE_PCM      MAKE_FOURCC('P','C','M',' ')
#define AVI_TYPE_MP3      MAKE_FOURCC('M','P','E','G')

/*录像参数*/
#define	DMS_NET_RECORD_TYPE_SCHED   0x000001  //定时录像
#define	DMS_NET_RECORD_TYPE_MOTION  0x000002  //移到侦测录像
#define	DMS_NET_RECORD_TYPE_ALARM   0x000004  //报警录像
#define	DMS_NET_RECORD_TYPE_CMD     0x000008  //命令录像
#define	DMS_NET_RECORD_TYPE_MANU    0x000010  //手工录像
#define	DMS_NET_RECORD_TYPE_SCHED_2  0x000011 //手工录像

typedef enum
{
	ENC_TYPE_H264 , //H264
	ENC_TYPE_MPEG,      //< MPEG4
	ENC_TYPE_MJPEG,    //< Motion JPEG
	ENC_TYPE_YUV422,
	ENC_TYPE_H265,     //<H265
	ENC_TYPE_COUNT     //< do not use
}eEncType;

typedef struct tagAVI_DMS_TIME
{
    unsigned long    dwYear;
    unsigned long    dwMonth;
    unsigned long    dwDay;
    unsigned long    dwHour;
    unsigned long    dwMinute;
    unsigned long    dwSecond;
} AVI_DMS_TIME, *LPAVI_DMS_TIME;

#if 0
/* Status Code */
typedef enum AviStatusTag {
    STS_OK                =  0,
    STS_FISTFRAME_NOT_KEY =  1,
    STS_FRAME_TYPE_ERR    =  2,
    //error
    STS_OPEN_FAILED       = -1,
    STS_INVALID_INPUT     = -2,
    STS_MKDIR_ERROR       = -3,
    STS_INVALID_FORAMT    = -4,
    STS_MALLOC_FAILED     = -5,
    STS_FTRUNCATE_FAILED  = -6,
    STS_RENAME_FAILED     = -7,
    STS_POLL_FAILED       = -8,
    STS_RECORD_MODE_ERR   = -9,
    STS_WRITE_FAILED      = -10,
    STS_READ_FAILED       = -11,
    STS_SDCARD_NOT_MOUNT  = -12,
    STS_SDCARD_NO_SPACE   = -13,
    STS_INDEX_COUNT_ERR   = -14,
    STS_DOUBLE_CLOSE   = -15,

}AviStatus;
#endif

//======================================================//
/* AVI Datastruct */
typedef struct AviFileTag {
    FILE   *file;
    char   filename[128];
    long   time_s;
    long   time_e;
    long   time_b;

    int    data_offset;

    int    video_count;
    int    audio_enable;
    int    audio_bytes;
    int    samplesize;
    int    audio_type; // 0 a-law; 1 u-law; 2 pcm

    //index
    int    index_count;   //real index count
    int    *idx_array;
    int    idx_array_count;
	int	   video_fps; // add video fps
}AviFile;


typedef struct tagAviInitParam {
    int bps;
    int fps;
    int width;
    int height;
    int codec_type;

    int audio_enable;
	int a_enc_type; // 0 a-law; 1 u-law; 2- pcm
    int a_chans;    /* Audio channels, 0 for no audio */
    int a_rate;     /* Rate in Hz */
    int a_bits;     /* bits per audio sample */

    int ch_num;
    int mode;
    int size_m;
    int duration; // min
} AviInitParam;

#ifndef HAVE_AVIMAINHEADER
#define HAVE_AVIMAINHEADER
typedef struct tagAviMainHeader {
    int  fcc;
    int  cb;
    int  dwMicroSecPerFrame;
    int  dwMaxBytesPerSec;
    int  dwPaddingGranularity;
    int  dwFlags;
    int  dwTotalFrames;
    int  dwInitialFrames;
    int  dwStreams;
    int  dwSuggestedBufferSize;
    int  dwWidth;
    int  dwHeight;
    int  dwReserved[4];
} AviMainHeader;
#endif

#ifndef HAVE_AVISTREAMHEADER
#define HAVE_AVISTREAMHEADER
typedef struct tagAviStreamHeader {
     int  fcc;
     int  cb;
     int  fccType;
     int  fccHandler;
     int  dwFlags;
     short int   wPriority;
     short int   wLanguage;
     int  dwInitialFrames;
     int  dwScale;
     int  dwRate;
     int  dwStart;
     int  dwLength;
     int  dwSuggestedBufferSize;
     int  dwQuality;
     int  dwSampleSize;
     struct {
         short int left;
         short int top;
         short int right;
         short int bottom;
     }  rcFrame;
} AviStreamHeader;
#endif

#ifndef HAVE_BITMAPINFOHEADER
#define HAVE_BITMAPINFOHEADER
typedef struct tagBitMapInfoHeader{
  int   biSize;
  int   biWidth;
  int   biHeight;
  short int   biPlanes;
  short int   biBitCount;
  int   biCompression;
  int   biSizeImage;
  int   biXPelsPerMeter;
  int   biYPelsPerMeter;
  int   biClrUsed;
  int   biClrImportant;
} BitMapInfoHeader;
#endif

#ifndef HAVE_RGBQUAD
#define HAVE_RGBQUAD
typedef struct tagRGBQuad {
  unsigned char    rgbBlue;
  unsigned char    rgbGreen;
  unsigned char    rgbRed;
  unsigned char    rgbReserved;
} RGBQuad ;
#endif

#ifndef HAVE_BITMAPINFO
#define HAVE_BITMAPINFO
typedef struct tagBitmapInfo {
  BitMapInfoHeader bmiHeader;
  //RGBQuad          bmiColors[1];
} BitmapInfo;
#endif

#ifndef HAVE_WAVEFORMATEX
#define HAVE_WAVEFORMATEX
typedef struct tagWaveFormateX{
  short int  wFormatTag;
  short int  nChannels;
  int nSamplesPerSec;
  int nAvgBytesPerSec;
  short int  nBlockAlign;
  short int  wBitsPerSample;
  short int  cbSize;
} WaveFormateX;
#endif

#ifndef HAVE_AUDIOFORMATEX
#define HAVE_AUDIOFORMATEX
typedef struct tagAudioFormateX{
  short int  wFormatTag;
  short int  nChannels;
  int nSamplesPerSec;
  int nAvgBytesPerSec;
  short int  nBlockAlign;
  short int  wBitsPerSample;
  //short int  cbSize;
} AudioFormateX;
#endif


#ifndef HAVE_AVIINDEX
#define HAVE_AVIINDEX
typedef struct tagAviIndex {
  int   dwChunkId;
  int   dwFlags;
  int   dwOffset;
  int   dwSize;
} AviIndex;
#endif

typedef union AviStreamFormatTag {
    BitmapInfo    video_format;
    WaveFormateX  audio_format;
}AviStreamFormat;

typedef struct tagINDEX_NODE
{
    u32t ch_num;
    u64t start;
    u64t stop;
}INDEX_NODE;

typedef struct tagFILE_NODE
{
    char path[128];
    u64t start;
    u64t stop;
    struct tagFILE_NODE *next;
} FILE_NODE;

typedef struct tagFILE_LIST
{
    u64t start;
    u64t stop;
    u32t type;
    u32t ch_num;

    FILE_NODE *head;
    u32t len;
    u32t index;
}FILE_LIST, *LPFILE_LIST;

#define ONE_FRAME_BUF_SIZE 700000

typedef struct tagAviFrameNode {
    int frame_type; // 0: audio; 0x11: I frame, 0x10: P frame
    int offset;
    int size;       //帧大小
    unsigned int timetick;  //帧时间信息
    char *one_frame_buf;  //帧数据
} AviFrameNode;


typedef struct tagAviPBHandle {
    AviFrameNode node;
    FILE_LIST *list;

    int no;
    int video_no;
    int audio_no;

    int gop;
	int fps;
    int video_width;
    int video_height;

    FILE    *file;
    char   file_path[128];

    int    video_count;

    //index
    int    index_count;   //real index count
    int    *idx_array;
    int    idx_array_count;

    //buffer for playback
    char *pb_buf;
    int pb_buf_size;
    int pb_buf_index;
    int pb_buf_pos;
    u32t pb_timetick_tmp;

    int pb_open_flag;
    int last_tick;
	int pb_buf_num;
} AviPBHandle;

typedef struct tagAviHeader {
    int fcc_riff;  //RIFF
    int avi_len;
    int fcc_avi;  //AVI

    int fcc_list_hdrl;  //LIST
    int hdrl_len;
    int fcc_hdrl;  //hdrl

    AviMainHeader main_header;

    int fcc_list_strl_video;  //LIST
    int size_strl_video;
    int fcc_strl_video;  //strl
    AviStreamHeader video_header;
    int fcc_strf_video;  //strf
    int size_video_format;
    //AviStreamFormat video_format;
    BitMapInfoHeader bmiHeader;

    int fcc_list_strl_audio;  //LIST
    int size_strl_audio;
    int fcc_strl_audio;  //strl
    AviStreamHeader audio_header;
    int fcc_strf_audio;  //strf
    int size_audio_format;
    AudioFormateX audio_format;

    int fcc_list_movi;
    int movi_len;
    int fcc_movi;
} AviHeader;





/**********************************************************************
函数描述：检查SD卡存储空间，创建文件，写AVI头信息
入口参数：AviFile *avi_file, AVI文件的控制句柄
          AviInitParam *param, 传入的参数信息
返回值： 0: 正常返回
        <0: 错误返回
**********************************************************************/
extern int avi_record_open(AviFile *avi_file, AviInitParam *param);


/**********************************************************************
函数描述：向AVI文件中写入一帧数据
入口参数：AviFile *avi_file:  AVI文件的控制句柄
          AviInitParam *param, 传入的参数信息
          unsigned char *data:  写入的数据的地址
          int length: 写入的数据的长度
          int intra: 1,表示为关键桢数据;0表示不为关键桢数据

返回值： 0: 正常返回
        <0: 错误返回
**********************************************************************/
extern int avi_record_write(AviFile *avi_file,
                                AviInitParam *param,
                                char *data,
                                int length,
                                int intra,
                                unsigned int timetick);


/**********************************************************************
函数描述：重命名AVI文件为最终格式，并关闭文件，释放内存
入口参数：AviFile *avi_file:  AVI文件的控制句柄
返回值： 0: 正常返回
        <0: 错误返回
**********************************************************************/
extern  int avi_record_close(AviFile *avi_file);


/**********************************************************************
回放部分接口函数使用描述：
1，调用 avi_pb_open 打开要播放的avi文件,文件名为file_name。
2，调用 avi_pb_seek_by_percent 或者 avi_pb_seek_by_time 定位播放起始位置，
   如果不使用这2个函数，则默认从头开始播放。
3，调用 set_frame_info 去设置 avi帧数据控制句柄 AviFrameInfo
4，调用 avi_pb_set_speed 去设置播放速度，不使用则播放速度默认为1倍速。
5，循环调用 avi_pb_get_frame 去得到每一帧数据。返回值为1003则播放到了末尾。
6，调用 avi_pb_close 结束。
注意: 当开启一个线程进行回放时，对回放速度的操作(avi_pb_set_speed)是
在另一个线程中(一般是主线程)，2个线程对 avi_pb_speed 的操作是互斥的。
**********************************************************************/
/**********************************************************************
函数描述：通过读AVI文件信息，得到他的控制句柄和参数信息
入口参数：char *file_name: 要打开的文件路径名
          AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
返回值：  <0: 错误
           0: 正常
**********************************************************************/
extern int avi_pb_open(char *file_path, AviPBHandle *pPBHandle);

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
extern int avi_pb_seek_by_percent(AviFile *avi_file, AviInitParam *param, int percent, AviPBHandle *pPBHandle);

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
extern int avi_pb_seek_by_time(AviFile *avi_file, AviInitParam *param, char *seek_time, AviPBHandle *pPBHandle);

/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息，来设置帧数据的控制句柄
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
          AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
返回值：  无
**********************************************************************/
//extern void set_frame_info(AviFile *avi_file, AviInitParam *param, AviFrameInfo *p_frame);

/**********************************************************************
函数描述：打印出avi文件中帧数据所在的avi文件中的信息
入口参数：AviPBHandle *pPBHandle: avi文件中帧数据的控制句柄
返回值：  无
**********************************************************************/
extern void print_pb_handle(AviPBHandle *pPBHandle);

/**********************************************************************
函数描述：打印出avi文件中帧数据的信息
入口参数：AviPBHandle *pPBHandle: avi文件中帧数据的控制句柄
返回值：  无
**********************************************************************/
extern void print_frame_node(AviPBHandle *pPBHandle);

extern AviPBHandle *create_pb_handle(void);

/**********************************************************************
函数描述：返回 n1 - n2 的值
入口参数：AviPBHandle *pPBHandle
返回值：  计算文件的总时长，单位为秒
**********************************************************************/
extern int cal_pb_all_time(AviPBHandle *pPBHandle);


/**********************************************************************
函数描述：返回 n1 - n2 的值
入口参数：AviPBHandle *pPBHandle,AVI_DMS_TIME *ptime
返回值：  计算指定时间到文件开始时间的长度
**********************************************************************/
int cal_pb_seek_time(AviPBHandle *pPBHandle,AVI_DMS_TIME *ptime);


/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息, 计算出该定位时间是第几帧
          数据，并更新到 AviFrameInfo 的 no 参数中
入口参数：AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
          char *pBuff: 向该缓存区位置指针存入数据
          int *pSize: 得到这帧数据的大小
返回值：  <0: 错误
           0: 正常
          DMS_NET_NOMOREFILE: 播放结束(DMS_NET_NOMOREFILE值为1003)
**********************************************************************/
extern int avi_pb_slide_to_next_file(AviPBHandle *pPBHandle);

extern int avi_pb_get_frame(AviPBHandle *pPBHandle, char *pBuff, int *pSize);

/**********************************************************************
函数描述：设置avi文件播放的速度，目前只支持快进
入口参数：int speed: 播放速度，如 1,2,4,8,16,32
返回值：  <0: 错误
           0: 正常
**********************************************************************/
extern int avi_pb_set_speed(int speed);

/**********************************************************************
函数描述：关闭帧数据的控制句柄
入口参数：AviFrameInfo *p_frame: avi文件中一帧数据的控制句柄
返回值：  <0: 错误
           0: 正常
**********************************************************************/
extern int avi_pb_close(AviPBHandle *pPBHandle);

/**********************************************************************
函数描述：将每次录像动作记录到管理文件中
入口参数：u32t type: 安普定义如下
          GK_NET_RECORD_TYPE_SCHED   定时录像
          GK_NET_RECORD_TYPE_MOTION  移到侦测录像
          GK_NET_RECORD_TYPE_ALARM   手动报警录像
          GK_NET_RECORD_TYPE_MANU    手工录像
          u32t ch_num: 通道号
          u64t start: 起始时间
          u64t stop:  结束时间
返回值：  <0: 错误
           0: 正常
**********************************************************************/
extern int write_manage_info(u32t type, u32t ch_num, u64t start, u64t stop);

/**********************************************************************
函数描述：以下6函数只针对安普对接
**********************************************************************/
extern int avi_pb_start(void);
extern int avi_pb_restart(AviPBHandle *pPBHandle);
extern int avi_pb_pause(void);
extern int avi_pb_get_pos(AviPBHandle *pPBHandle);
extern int avi_pb_set_pos(AviPBHandle *pPBHandle, int *pTimePos);
extern int avi_pb_seek(AviPBHandle *pPBHandle, int seek_no);
extern int avi_pb_get_time(AviPBHandle *pPBHandle,  struct tm *p_nowtime);
extern int avi_get_frame_loop(AviPBHandle *pPBHandle, char *p_buff, int *p_size);
extern int avi_get_frame_by_no(AviPBHandle *pPBHandle, char *p_buff, int *p_size);

/**********************************************************************
函数描述：修复sd卡中所有的未录完成的avi文件
入口参数：无
返回值：  无
**********************************************************************/
extern void fix_all_avi_file(void);


/**********************************************************************
函数描述：检查一个文件是否需要修复
入口参数：无
返回值：  无
**********************************************************************/
int avi_check_file_index(char *file_path);

/**********************************************************************
函数描述：根据参数打印出错误提示.
入口参数：int ret: avi_record_open,avi_record_write,avi_record_close
                   的返回值作为参数
返回值：空
**********************************************************************/
//extern  void avi_print_err(int ret);


// 将录像cache 数据刷入SD卡中
int avi_record_write_cache(AviFile *avi_file);

//检查录像是否停止
int avi_reocrd_check_file_stoped();

#ifdef __cplusplus
}
#endif

#endif /* __GRD_AVI_TYPEDEF_H__ */

