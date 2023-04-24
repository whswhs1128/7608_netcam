#ifndef GOKE_MOJING_H
#define GOKE_MOJING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int freshNow;
    int upgradeNow;
    int cmdType;
    char extralData[512];
}GOKE_SHARE_INFO;


typedef enum
{
    GOKE_CMD_FORMAT = 1
}GOKE_CMD_TYPE;

/*********************
goke_share_udp_info
type:
1.重启
2.恢复出厂
3.开始升级
4.格式化
5.内存卡错误
6.内存卡检查错误
7.段错误
8.升级完成
*********************/
int goke_share_udp_info(int type, char *contentStr);

#ifdef __cplusplus
    }
#endif

#endif
