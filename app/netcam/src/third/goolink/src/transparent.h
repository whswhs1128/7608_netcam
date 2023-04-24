#ifndef TRANSPARENT_H
#define TRANSPARENT_H

#if defined( __cplusplus)
extern "C" {
#endif
#define TOKENSIZE 16
#define DOWNLOADFILE "downloadfile" //定义的不能超过15bytes !
    typedef struct _TLV_V_UploadPanoAlarmResponse
    {
        char    szToken[TOKENSIZE];
        char 	filename[32];					//remap.pano 或者 weight.pano
        int		type;							//1:开始下载  2：下载中 3：下载完成  4：错误
        int 	size;							//文件总长度
        int		len;							//当前数据长度
    }TLV_V_UploadAlarmResponse;

    typedef struct _TLV_V_DownloadAlarmResponse
    {
        char szToken[TOKENSIZE];  //用于区别
        int result;								//1--保存完成 0--失败
        int reserve;							//保留
    }TLV_V_DownloadAlarmResponse;


    int  download_file(char * bufrecv);

    int mysystem(const char *cmdstringl);

//=========================================================================================
//当到了maxzoom时返回通知对方
enum{
    PROTOCOL_INQURY_MAXZOOM = 1,
    PROTOCOL_INQURY_CURZOOM = 2,
    PROTOCOL_BEGIN_CALLBACK = 100,
};
#define AF_CALLBACK "af_callback"
#define AF_TOKENSIZE 16
    typedef struct _AF_NOTIFY
    {
        char  szToken[AF_TOKENSIZE];
        int   protocol; 
        int   data;
    }AF_NOTIFY;
/*
1 查询maxzoom: protocol=1: 查询maxzoom, data为查询到的结果
2 查询curzoom: protocol=2: 查询curzoom, data为查询到的结果
3 启动到顶回调: protocol=100, data=1表示到了minzoom; data=2表示到了maxzoom
*/

extern int af_countdown;
extern int g_af_item;
void * Thread_AF_notify(void *lParam);

enum{
    INQUERY_BOARD    = 0x01,
    INQUERY_MAX_ZOOM ,
    INQUERY_CUR_ZOOM ,
};

int handle_af(int item, AF_NOTIFY * phead);
typedef int (*FUNCNOTIFY) (int item, int ntype);


#if defined( __cplusplus)
}
#endif

#endif