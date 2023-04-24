#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include "adi_types.h"
#include "adi_sys.h"
#include "adi_pda.h"
#include "lv_color.h"
#include "app_ui_face_detect_menu.h"

#define MAX_BUFFER_NUM 10

typedef struct jpegHeader
{
    int flag;
    int len;
}JPEG_SEND_HEADER;

typedef struct cmdHeader
{
    int flag;
    int len;
    int cmdType;
}CMD_HEADER;

typedef struct faceGetInfo
{
    int available;
    int truckId;
    int leftTopX;
    int leftTopY;
    int rightBottomX;
    int rightBottomY;
}FACE_INFO;

typedef struct faceGetResult
{
    int available;
    int faceId;
    int faceScore;
    char name[12];
}FACE_RESULT;

typedef struct faceMsg
{
    FACE_INFO faceInfo;
    FACE_RESULT faceResult;
}FACE_INFO_MSG;

typedef struct faceYuvInfo
{
    int width;
    int height;
    int uvWidth;
    int uvHeight;
}FACE_YUV_INFO;

typedef struct faceYuv
{
    int status;
    char *yBuffer;
    char *uvBuffer;
}FACE_YUV_DATA;


typedef struct rects
{
    int x;
    int y;
    int width;
    int height;
}FACE_RECT;

typedef struct GY_CMD_HEADER
{
    unsigned short headerFlag;
    char ver;
    char cmd;
    unsigned int dataLen;
}FACE_GY_CMD_HEADER;

extern int hWidth;
extern int hHeight;
int fdRet;
int sockfd;
int faceSockSearchfd;
int nameIndex = 0;
int skipNum = 0;
int curIndex = 0;
int faceServerPort = 5556;
float screenRateW;
float screenRateH;
FACE_RECT lastRect = {0};
char *dataY = NULL;
short *dataUv = NULL;
int faceMsgId = -1;
int faceMsgYuvId = -1;
FACE_INFO faceInfoLast;
FACE_RESULT faceResultLast;
FACE_YUV_INFO yuvInfo;
FACE_YUV_DATA yuvDatas[MAX_BUFFER_NUM];

char faceServerIp[32] = "192.168.10.150";   //gateway.mj.sctel.com.cn  192.168.10.186

void encoder_osd_draw_multi(FACE_INFO faceInfo, FACE_RESULT faceResult)//int x, int y, int width, int height
{
    #if 0
    int x, y, width, height;
    char namestr[12];
    char strbuf[12];

    faceInfo.leftTopX += 20;
    y = faceInfo.leftTopY * screenRateH;
    width = (faceInfo.rightBottomX - faceInfo.leftTopX) * screenRateW;
    height = (faceInfo.rightBottomY - faceInfo.leftTopY) * screenRateH;
    x = faceInfo.leftTopX * screenRateW;

    //x = faceInfo.leftTopY * screenRateH;
    //height = (faceInfo.rightBottomX - faceInfo.leftTopX) * screenRateW;
    //width = (faceInfo.rightBottomY - faceInfo.leftTopY) * screenRateH;
    //y = hWidth - faceInfo.leftTopX * screenRateW - height;
    //printf("new %d,%d,%d,%d\n", x, y, width, height);
    if (faceResult.available == 0)
    {
        memset(namestr,0,sizeof(namestr));
        app_ui_face_menu_detect_refresh(x,y,width,height,LV_COLOR_BUTTON_RED_H);
        app_ui_face_menu_infor_show(namestr, 0);
    }
    else
    {
        memset(namestr,0,sizeof(namestr));
        //memcpy(namestr,faceResult.name,sizeof(namestr));
        app_ui_face_menu_detect_refresh(x,y,width,height,LV_COLOR_GREEN);
        memset(strbuf,0,sizeof(strbuf));
        sprintf(strbuf, "张三");
        utility_gbk_to_utf8(strbuf, namestr, 12);
        app_ui_face_menu_infor_show(namestr, 1);
    }

    #else
    int x, y, width, height;
    struct timeval tt1, tt2;
    int t1;
    int ret = 0;
    static FACE_RESULT lastResult;
    //printf("encoder_osd_draw_multi....\n");
    //return;
    faceInfo.leftTopX += 20;
    x = faceInfo.leftTopY * screenRateH;
    height = (faceInfo.rightBottomX - faceInfo.leftTopX) * screenRateW;
    width = (faceInfo.rightBottomY - faceInfo.leftTopY) * screenRateH;
    y = hWidth - faceInfo.leftTopX * screenRateW - height;
    printf("new %d,%d,%d,%d\n", x, y, width, height);

    if (lastRect.width != 0)
    {
        fb_draw_rect2(lastRect.x, lastRect.y, lastRect.width, lastRect.height, 0, 4);
    }
    //fb_draw_clear();
    gettimeofday(&tt1, NULL);
    if (faceResult.available == 0)
    {
        fb_draw_rect2(x, y, width, height, LV_COLOR_BUTTON_RED_H, 4);
    }
    else
    {
        fb_draw_rect2(x, y, width, height, LV_COLOR_GREEN, 4);

    }
    lastRect.x = x;
    lastRect.y = y;
    lastRect.width = width;
    lastRect.height = height;
    fb_fresh();

    #if 1
    char namestr[12];
    char strbuf[12];

    if (faceResult.available == 0)
    {
        memset(namestr,0,sizeof(namestr));
        app_ui_face_menu_infor_show(namestr, 0);
    }
    else
    {
        memset(strbuf,0,sizeof(strbuf));
        utility_utf8_to_gbk(faceResult.name, strbuf, 12);
        printf("face name:%s\n", strbuf);
        if (strcmp(faceResult.name, lastResult.name) != 0)
        {
            app_ui_face_menu_infor_show(faceResult.name, 2);
        }
        else
        {
            app_ui_face_menu_infor_show(faceResult.name, 1);
        }
    }

    lastResult = faceResult;

    #else
    fb_fresh();
    #endif
    gettimeofday(&tt2, NULL);
    t1 = (tt2.tv_sec-tt1.tv_sec)*1000+(tt2.tv_usec-tt1.tv_usec)/1000;
    //printf("-->draw rect:%dms\n", t1);

    #endif

}

void encoder_osd_clean(SDK_U8 areaId)
{
    printf("encoder_osd_clean....\n");

#if 0
    app_ui_face_menu_detect_clear();
#else
    //fb_draw_clear();
    fb_draw_rect2(lastRect.x, lastRect.y, lastRect.width, lastRect.height, 0, 4);
    app_ui_face_menu_infor_show(0, 0);
    fb_fresh();
#endif
}

static inline SDK_S32 delete_padding_from_strided_y
    (SDK_U8* output_y, const SDK_U8* input_y, SDK_S32 pitch, SDK_S32 width, SDK_S32 height)
{
    SDK_S32 row;
    for (row = 0; row < height; row++) {         //row
        memcpy(output_y, input_y, width);
        input_y = input_y + pitch;
        output_y = output_y + width ;
    }
    return 0;
}

static char * rotate_data_y(char *data, int width, int height)
{
    int i = 0;
    int j = 0;
    int n, k;
    return data;
    if (dataY == NULL)
    {
        dataY = malloc(width * height);
    }
    if (dataY == NULL)
    {
        return NULL;
    }

    #if 0
    for(i = 0; i < width; i++)
    {
        k = (i + 1) * height;
        for(j = i * height, n = (height - 1) * width + i; j < k; j++, n -= width)
        {
            dataY[j] = data[n];
        }
    }
    #else
    k = 0;
    int tmp = (height - 1) * width;
    n = tmp;
    for(i = 0; i < width; i++)
    {
        k += height;
        n = tmp++;
        for(; j < k; j++, n -= width)
        {
            dataY[j] = data[n];
        }
    }
    #endif

    memcpy(data, dataY, width * height);
    return data;
}

static char * rotate_data_uv(char *data, int width, int height)
{
    int i = 0;
    int j = 0;
    int n, k;
    short *fromData = (short *)data;

    #if 0
    char *tmpData = NULL;
    tmpData = (char *)malloc(width * height * 2);
    #else
    short *tmpData = NULL;
    if (dataUv == NULL)
    {
        dataUv = (short *)malloc(width * height * 2);
    }
    #endif
    if (dataUv == NULL)
    {
        return NULL;
    }
    //gettimeofday(&tt1, NULL);
    #if 1
    for(i = 0; i < width; i++)
    {
        k = (i + 1) * height;
        for(j = i * height, n = (height - 1) * width + i; j < k; j++, n -= width)
        {
            dataUv[j] = fromData[n];
        }
    }
    #else
    for(i = 0; i < width; i++)
    {
        for(j = 0; j < height; j++)
        {
            #if 0
            tmpData[i * height * 2 + j] = data[(height - j - 1) * width * 2 + i];
            tmpData[i * height * 2 + j + 1] = data[(height - j - 1) * width * 2 + i + 1];
            #else
            tmpData[i * height + j] = fromData[(height - j - 1) * width + i];
            #endif
        }
    }
    #endif
    memcpy(data, dataUv, width * height * 2);
    //free(tmpData);
    return data;
}

int goke_face_get_yuv_buffer(void)
{
    int i;
    for (i = 0; i < MAX_BUFFER_NUM; i++)
    {
        if (yuvDatas[i].status == 0)
        {
            return i;
        }
    }
    return -1;
}


static void goke_face_get_one_yuv(SDK_U32 channelIndex, SDK_U32 viId)
{
    SDK_ERR ret;
    GADI_PDA_Frame_InfoT info;
    SDK_U8 * uvBuffer = NULL;
    SDK_U8 * yBuffer = NULL;
    SDK_U8 fileName[64];
    SDK_U32 fileSize = 0;
    SDK_S32 uv_width, uv_height,uv_pitch;
    SDK_S32 wsize;
    SDK_S32 row,col;
    GADI_PDA_CAP_Buffer_TypeT buffer = GADI_PDA_CAP_MAIN_A_BUFFER;
    JPEG_SEND_HEADER jpegHeader;
    FILE *yuv_stream = NULL;
    int yuvIndex = -1;

    if (0 == channelIndex)
    {
        buffer = GADI_PDA_CAP_MAIN_A_BUFFER;
    }
    else if (1 == channelIndex)
    {
        buffer = GADI_PDA_CAP_MAIN_B_BUFFER;
    }
    else if (2 == channelIndex)
    {
        buffer = GADI_PDA_CAP_SUB_A_BUFFER;
    }
    else if (4 == channelIndex)
    {
        buffer = GADI_PDA_CAP_SUB_C_BUFFER;
    }
    else
    {
        return ;
    }
    curIndex++;

    ret = gadi_pda_capture_buffer(buffer, &info);
    if(ret != SDK_OK)
    {
        GADI_ERROR("pda_capture_bmp: failed:%d\n", ret);

        goto error_exit;
    }

    if (curIndex % 2 != 0)
        return;

    #if 0
    if (curIndex % 2 != 0)
    {
        return;
    }
    #endif

    //printf("gadi_pda_capture_buffer %d*%d stride:%d, type:%d\n", info.width, info.height, info.stride, info.frameFormat);

    yuvIndex = goke_face_get_yuv_buffer();
    if (yuvIndex == -1)
    {
        printf("not free yuv data buffer.\n");
        return;
    }

    if (yuvDatas[yuvIndex].yBuffer == NULL)
    {
        yuvDatas[yuvIndex].yBuffer = malloc(info.width * info.height);
        if (yuvDatas[yuvIndex].yBuffer == NULL) {
            printf("Not enough memory for Y buffer:%dKB !\n",info.width * info.height);
            goto error_exit;
        }

        yuvDatas[yuvIndex].uvBuffer = malloc(info.width * info.height);
        if (yuvDatas[yuvIndex].uvBuffer == NULL) {
            printf("Not enough memory for UV buffer:%dKB !\n",info.width * info.height);
            goto error_exit;
        }
    }


    sprintf((char*)fileName,"/tmp/pda_channel%d_%d_%d_yv12.yuv",channelIndex, info.width,info.height);

    if (info.stride == info.width)
    {
        memcpy(yuvDatas[yuvIndex].yBuffer, info.yAddr, info.width * info.height);
    } else if (info.stride > info.width)
    {
        delete_padding_from_strided_y(yuvDatas[yuvIndex].yBuffer,
        info.yAddr, info.stride, info.width, info.height);
    } else
    {
        printf("stride size smaller than width!\n");
        goto error_exit;
    }

    //convert uv data from interleaved into planar format
    if (FRAME_FORMAT_YUV_SEMIPLANAR_420 == info.frameFormat) {
        uv_pitch = info.stride/ 2;
        uv_width = info.width / 2;
        uv_height = info.height / 2;
    } else { // FRAME_FORMAT_YUV_SEMIPLANAR_422
        uv_pitch = info.stride/ 2;
        uv_width = info.width / 2;
        uv_height = info.height;
    }
    memcpy(yuvDatas[yuvIndex].uvBuffer, info.uvAddr, info.width * info.height);

    if (yuvInfo.width == 0)
    {
        yuvInfo.width = info.width;
        yuvInfo.height = info.height;
        yuvInfo.uvWidth = uv_width;
        yuvInfo.uvHeight = uv_height;
    }

    yuvDatas[yuvIndex].status = 1;
    ret = msgsnd(faceMsgYuvId, (void *)&yuvIndex, sizeof(yuvIndex), 0);
    return;

    #if 1
    #if 0
    delete_padding_and_deinterlace_from_strided_uv(uvBuffer,
        info.uvAddr, uv_pitch, uv_width, uv_height,1);
    #endif
    #else
    SDK_U8 * output_u = uvBuffer;
    SDK_U8 * output_v = uvBuffer+info.width * info.height / 4;

	SDK_U8 * output_uv = info.uvAddr;

	//printf("uv %dx%d\n", uv_width, uv_height);

    for (row = 0; row < uv_height; row++) {
        for (col = 0; col < uv_width; col++) {
			if (FRAME_FORMAT_YUV_SEMIPLANAR_420 == info.frameFormat)
			{
                *output_u++ = *output_uv++;
                *output_v++ = *output_uv++;
			}
			else
			{
				if (row%2)
				{
                    *output_u++ = *output_uv++;
                    *output_v++ = *output_uv++;
				}
				else
				{
                    output_uv += 2;
				}
			}
        }
        output_uv += (uv_pitch - uv_width) * 2;
    }
    #endif
    struct timeval tt1, tt2;
    int t1;

    fileSize = info.width*info.height + uv_width*uv_height*2;
    #if 1
    //nv12
    #if 1
    jpegHeader.flag = 0x55443355;
    jpegHeader.len = fileSize;
    wsize = write(sockfd, &jpegHeader, sizeof(jpegHeader));
    rotate_data_y(yBuffer, info.width, info.height);
    gettimeofday(&tt1, NULL);
    wsize = write(sockfd, yBuffer, info.width*info.height);
    gettimeofday(&tt2, NULL);
    t1 = (tt2.tv_sec-tt1.tv_sec)*1000 + (tt2.tv_usec-tt1.tv_usec)/1000;
    if (wsize > 0)
    {
        //wsize += write(sockfd, uvBuffer, uv_width*uv_height*2);
        #if 0
        memcpy(uvBuffer, info.uvAddr, info.width * info.height);
        rotate_data_uv(uvBuffer, uv_width, uv_height);
        wsize += write(sockfd, info.uvAddr, uv_width*uv_height*2);
        #else
        char tmpi;
        int i;
        for (i  = 0; i < uv_width*uv_height*2; )
        {
            tmpi = uvBuffer[i];
            uvBuffer[i] = uvBuffer[i + 1];
            uvBuffer[i + 1] = tmpi;
            i += 2;
        }
        rotate_data_uv(uvBuffer, uv_width, uv_height);
        wsize += write(sockfd, uvBuffer, uv_width*uv_height*2);
        #endif
    }

    //printf("write len:%d, ret:%d\n", fileSize, wsize);
    if (wsize == -1)
    {
        fdRet = -1;
        close(sockfd);
        sockfd=socket(AF_INET,SOCK_STREAM,0);
    }
    #endif
    #else
    sprintf(fileName, "%03d.yuv", nameIndex++);
    yuv_stream = fopen((char *)fileName,"wb");
    if(yuv_stream == NULL)
    {
        GADI_ERROR("pda_capture_bmp: open file error\n");
        goto error_exit;
    }
    rotate_data_y(yBuffer, info.width, info.height);
    wsize = fwrite(yBuffer, 1, info.width*info.height, yuv_stream);

    //nv21->nv12
    char tmpi;
    int i;
    memcpy(uvBuffer, info.uvAddr, info.width * info.height);
    for (i  = 0;  i < uv_width*uv_height*2; )
    {
        tmpi = uvBuffer[i];
        uvBuffer[i] = uvBuffer[i + 1];
        uvBuffer[i + 1] = tmpi;
        i += 2;
    }
    rotate_data_uv(uvBuffer, uv_width, uv_height);
    wsize += fwrite(uvBuffer, 1, uv_width*uv_height*2, yuv_stream);
    printf("write %03d len:%d, ret:%d\n", nameIndex - 1, fileSize, wsize);
    #endif

    //GADI_INFO(" Cap ture yuv picture success: %s\n",fileName);
error_exit:
    if(yBuffer != NULL)
    {
        free(yBuffer);
        yBuffer = NULL;
    }
    if(uvBuffer != NULL)
    {
        free(uvBuffer);
        uvBuffer = 0;
    }

    return;
}


void goke_face_msg_recv(void *data)
{
    int size = 0;
    char buffer[1024];
    char *tmpBuff;
    char *tmpHeader;
    int *headerCheck = NULL;
    CMD_HEADER header;
    int recvLen = 0;
    int headerLen = sizeof(header);
    int flags;
	fd_set readfd;
    int len;
    int ret;
    int i, j;
    int receive_len;
    int needReset = 1;
    FACE_INFO *faceInfo = NULL;
    FACE_RESULT faceResult;
    int sockfdRev = sockfd;
    int fdRecvRet = -1;
    char nameGbk[256] = {0};
    FACE_INFO_MSG faceInfoMsg;
	struct timeval timeout;



    sdk_sys_thread_set_name("faceRecv");
    memset(buffer, 0, sizeof(buffer));

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    memset(&faceInfoLast, 0, sizeof(faceInfoLast));

    //osd_font_init();
	//fb_canvas_name = canvas_init(200,32);
	tmpHeader = (char*)&header;
    recvLen = headerLen;
    while(1)
    {
        if (fdRet >= 0)
        {
            if (needReset)
            {
                setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
                needReset = 0;
            }
            #if 1
            size = recv(sockfd, tmpHeader, recvLen, 0);
            if (size == sizeof(header) && header.flag == 0x55443355)
            {
                receive_len = header.len;
                tmpBuff = buffer;
                //printf("header len:%d, type:%d\n", receive_len, header.cmdType);
                while(1)
                {
                    len = recv(sockfd, tmpBuff, receive_len,0);
                    //printf("try to recv:%d, ret:%d\n", receive_len, len);
                    if (len <= 0)
                    {
                        break;
                    }


                    if (len < receive_len)
                    {
                        tmpBuff += len;
                        receive_len -= len;
                    }
                    else
                    {
                        //printf("receive type:%d, data len:%d\n", header.cmdType, header.len);
                        //printf("%s\n", buffer);
                        //save_jpeg_file(buffer, jpegHeader.len);
                        break;
                    }
                }

                tmpHeader = (char*)&header;
                recvLen = headerLen;

                if (len < receive_len)
                {
                    continue;
                }

                if (header.cmdType == 0)//face info
                {
                    faceInfo = (FACE_INFO*)tmpBuff;
                    #if 0
                    printf("face info:%d, id:%d, %d,%d,%d,%d\n", faceInfo->available, faceInfo->truckId,
                        faceInfo->leftTopX, faceInfo->leftTopY, faceInfo->rightBottomX, faceInfo->rightBottomY);
                    #endif
                    if (faceInfo->available && memcmp(&faceInfoLast, faceInfo, sizeof(faceInfoLast)) != 0)
                    {
                        faceInfoLast = *faceInfo;
                        faceInfoMsg.faceInfo = faceInfoLast;
                        faceInfoMsg.faceResult = faceResultLast;
                        ret = msgsnd(faceMsgId, (void *)&faceInfoMsg, sizeof(faceInfoMsg), 0);
                    }
                    else if (faceInfo->available == 0 && faceInfoLast.available != faceInfo->available)
                    {
                        encoder_osd_clean(0);
                        faceInfoLast.available = 0;
                        faceResultLast.available = 0;
                    }
                }
                else if (header.cmdType == 1)//face result
                {
                    memset(&faceResult, 0, sizeof(faceResult));
                    memcpy(&faceResult, tmpBuff, header.len);

                    if (faceResult.available != faceResultLast.available)
                    {
                        //encoder_osd_draw_multi(faceInfoLast, faceResult);
                    }

                    #if 1
                    memset(nameGbk, 0, sizeof(nameGbk));
                    utility_utf8_to_gbk(faceResult.name, nameGbk, 12);
                    #if 0
                    for (i = 0; i < header.len - 12; i++)
                    {
                        if (i % 16 == 0)
                            printf("\n");
                        printf("%02x ", faceResult.name[i]);
                    }
                    printf("\n");
                    #endif
                    #endif
                    #if 1
                    printf("face result:%d,%d, len:%d, id:%d, score:%d, name:%s\n", faceResult.available, faceResultLast.available,
                        header.len, faceResult.faceId, faceResult.faceScore, nameGbk);
                    #endif
                    faceResultLast = faceResult;
                    faceInfoMsg.faceInfo = faceInfoLast;
                    faceInfoMsg.faceResult = faceResult;
                    ret = msgsnd(faceMsgId, (void *)&faceInfoMsg, sizeof(faceInfoMsg), 0);
                    /*
                    pthread_mutex_lock(&mFaceMutexQue);
                    memcpy(&faceResultLast, &faceResult, sizeof(faceResultLast));
                    pthread_mutex_unlock(&mFaceMutexQue);
                    */
                }

            }
            else
            {
                if (faceInfoLast.available || faceResultLast.available)
                {
                    encoder_osd_clean(0);
                    faceInfoLast.available = 0;
                    faceResultLast.available = 0;
                }

                if (size == headerLen)
                {
                    headerCheck = (int*)tmpHeader;
                    for (i = 0; i < headerLen/sizeof(int); i++)
                    {
                        if (headerCheck[i] == 0x55443355)
                        {
                            break;
                        }
                    }

                    for (j = 0; j < headerLen - i; j++, i++)
                    {
                        headerCheck[j] = headerCheck[i];
                    }
                    j++;
                    tmpHeader = tmpHeader + j * sizeof(int);
                    recvLen = headerLen - j * sizeof(int);
                }
                else
                {
                    tmpHeader = (char*)&header;
                    recvLen = headerLen;
                }
                printf("recv error..%d, flag:%x,%x,%x\n", size, header.flag, header.len, header.cmdType);
                usleep(200000);
            }
            #else
            if (needReset)
            {
                flags = fcntl(sockfd,F_GETFL,0);
                fcntl(sockfd,F_SETFL,flags|O_NONBLOCK);

                FD_ZERO(&readfd);
                FD_SET(sockfd, &readfd);

                timeout.tv_sec = 3;
                timeout.tv_usec = 0;
                needReset = 0;
            }
            ret = select(sockfd + 1, &readfd, NULL, NULL, &timeout);

            if(FD_ISSET(sockfd, &readfd))
            {
                size = recv(sockfd, &header, sizeof(header), 0);
                if (size == sizeof(header) && header.flag == 0x55443355)
                {
                    receive_len = header.len;
                    tmpBuff = buffer;
                    while(1)
                    {
                        len = recv(sockfd, tmpBuff, receive_len,0);
                        printf("try to recv:%d, ret:%d\n", receive_len, len);

                        if (len < receive_len)
                        {
                            tmpBuff += len;
                            receive_len -= len;
                        }
                        else
                        {
                            printf("receive pic len:%d\n", header.len);
                            printf("%s\n", buffer);
                            //save_jpeg_file(buffer, jpegHeader.len);
                            break;
                        }
                    }
                }
                else
                {
                    printf("recv error..%d, flag:%x\n", size, header.flag);
                }
            }
            printf("select timeout\n");
            #endif
        }
        else
        {
            printf("connect recv ret:%d\n", fdRet);
            needReset = 1;
            sleep(1);
        }
    }
}

void goke_face_msg_recv_other(void *data)
{
    int size = 0;
    char buffer[1024];
    char *tmpBuff;
    CMD_HEADER header;
    int flags;
	fd_set readfd;
    int len;
    int ret;
    int i;
    int receive_len;
    int needReset = 1;
    FACE_INFO *faceInfo = NULL;
    FACE_INFO faceInfoLast;
    FACE_RESULT faceResult;
    FACE_RESULT faceResultLast;
    int sockfdRev = -1;
    int fdRecvRet = -1;
    char nameGbk[256] = {0};
	struct timeval timeout;

    sockfdRev = socket(AF_INET,SOCK_STREAM,0);

    if(sockfdRev<0)
    {
        printf("create recv error\n");
    }
    sdk_sys_thread_set_name("faceSend");
    //连接服务器，设置服务器的地址(ip和端口)
    struct sockaddr_in svraddr;
    //char faceServerIp[] = "192.168.10.150";
    memset(&svraddr,0,sizeof(svraddr));
    svraddr.sin_family=AF_INET;
    svraddr.sin_addr.s_addr= inet_addr(faceServerIp);
    svraddr.sin_port=htons(5557);
    printf("connect recv server:%s\n", faceServerIp);
    fdRecvRet = connect(sockfdRev,(struct sockaddr *)&svraddr,sizeof(svraddr));
    if(fdRecvRet < 0)
    {
        printf("connect recv error\n");
    }
    else
    {
        printf("connect recv ok\n");
    }

    sdk_sys_thread_set_name("faceRecv");
    memset(buffer, 0, sizeof(buffer));

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    memset(&faceInfoLast, 0, sizeof(faceInfoLast));

    //osd_font_init();
	//fb_canvas_name = canvas_init(200,32);
    while(1)
    {
        if (fdRecvRet >= 0)
        {
            if (needReset)
            {
                setsockopt(sockfdRev,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
                needReset = 0;
            }
            #if 1
            size = recv(sockfdRev, &header, sizeof(header), 0);
            if (size == sizeof(header) && header.flag == 0x55443355)
            {
                receive_len = header.len;
                tmpBuff = buffer;
                printf("header len:%d, type:%d\n", receive_len, header.cmdType);
                while(1)
                {
                    len = recv(sockfdRev, tmpBuff, receive_len,0);
                    //printf("try to recv:%d, ret:%d\n", receive_len, len);

                    if (len <= 0)
                    {
                        break;
                    }

                    if (len < receive_len)
                    {
                        tmpBuff += len;
                        receive_len -= len;
                    }
                    else
                    {
                        //printf("receive type:%d, data len:%d\n", header.cmdType, header.len);
                        //printf("%s\n", buffer);
                        //save_jpeg_file(buffer, jpegHeader.len);
                        break;
                    }
                }

                if (len < receive_len)
                {
                    continue;
                }

                if (header.cmdType == 0)//face info
                {
                    faceInfo = (FACE_INFO*)tmpBuff;
                    #if 1
                    printf("face info:%d, id:%d, %d,%d,%d,%d\n", faceInfo->available, faceInfo->truckId,
                        faceInfo->leftTopX, faceInfo->leftTopY, faceInfo->rightBottomX, faceInfo->rightBottomY);
                    #endif
                    if (faceInfo->available && memcmp(&faceInfoLast, faceInfo, sizeof(faceInfoLast)) != 0)
                    {
                        memcpy(&faceInfoLast, faceInfo, sizeof(faceInfoLast));
                        encoder_osd_draw_multi(faceInfoLast, faceResultLast);
                    }
                    else if (faceInfo->available == 0)
                    {
                        encoder_osd_clean(0);
                        faceInfoLast.available = 0;
                        faceResultLast.available = 0;
                    }
                }
                else if (header.cmdType == 1)//face result
                {
                    memset(&faceResult, 0, sizeof(faceResult));
                    memcpy(&faceResult, tmpBuff, header.len);

                    if (faceResult.available != faceResultLast.available)
                    {
                        encoder_osd_draw_multi(faceInfoLast, faceResult);
                    }

                    #if 0
                    memset(nameGbk, 0, sizeof(nameGbk));
                    utility_utf8_to_gbk(faceResult.name, nameGbk, 256);
                    for (i = 0; i < header.len - 12; i++)
                    {
                        if (i % 16 == 0)
                            printf("\n");
                        printf("%02x ", faceResult.name[i]);
                    }
                    printf("\n");
                    #endif
                    #if 1
                    printf("face result:%d,%d, len:%d, id:%d, score:%d, name:%s\n", faceResult.available, faceResultLast.available,
                        header.len, faceResult.faceId, faceResult.faceScore, nameGbk);
                    #endif
                    memcpy(&faceResultLast, &faceResult, sizeof(faceResultLast));
                }
            }
            else
            {
                if (faceInfoLast.available || faceResultLast.available)
                {
                    encoder_osd_clean(0);
                    faceInfoLast.available = 0;
                    faceResultLast.available = 0;
                }

                if (size <= 0)
                {
                    fdRecvRet = -1;
                }
                printf("recv error..%d, flag:%x,%x,%x\n", size, header.flag, header.len, header.cmdType);
                usleep(200000);
            }
            #else
            if (needReset)
            {
                flags = fcntl(sockfd,F_GETFL,0);
                fcntl(sockfd,F_SETFL,flags|O_NONBLOCK);

                FD_ZERO(&readfd);
                FD_SET(sockfd, &readfd);

                timeout.tv_sec = 3;
                timeout.tv_usec = 0;
                needReset = 0;
            }
            ret = select(sockfd + 1, &readfd, NULL, NULL, &timeout);

            if(FD_ISSET(sockfd, &readfd))
            {
                size = recv(sockfd, &header, sizeof(header), 0);
                if (size == sizeof(header) && header.flag == 0x55443355)
                {
                    receive_len = header.len;
                    tmpBuff = buffer;
                    while(1)
                    {
                        len = recv(sockfd, tmpBuff, receive_len,0);
                        printf("try to recv:%d, ret:%d\n", receive_len, len);

                        if (len < receive_len)
                        {
                            tmpBuff += len;
                            receive_len -= len;
                        }
                        else
                        {
                            printf("receive pic len:%d\n", header.len);
                            printf("%s\n", buffer);
                            //save_jpeg_file(buffer, jpegHeader.len);
                            break;
                        }
                    }
                }
                else
                {
                    printf("recv error..%d, flag:%x\n", size, header.flag);
                }
            }
            printf("select timeout\n");
            #endif
        }
        else
        {
            if (sockfdRev != -1)
            {
                close(sockfdRev);
            }
            sockfdRev = socket(AF_INET,SOCK_STREAM,0);
            fdRecvRet = connect(sockfdRev,(struct sockaddr *)&svraddr,sizeof(svraddr));
            printf("connect recv ret:%d\n", fdRecvRet);
            needReset = 1;
            sleep(1);
        }
    }
}

static int goke_face_send_data_udp(char *data, int length)
{
    int maxLen = 65000;
    int curLen = 0;
    int curSendLen = 0;
    if (length > maxLen)
    {
        while(curLen < length)
        {
            if ((length - curLen) > maxLen)
            {
                curSendLen = maxLen;
            }
            else
            {
                curSendLen = length - curLen;
            }
            BoardCastSendTo(faceSockSearchfd, "192.168.10.114", 5556, data + curLen, curSendLen);
            curLen += curSendLen;
        }
    }
    else
    {
        BoardCastSendTo(faceSockSearchfd, "192.168.10.114", 5556, data, length);
    }
}

static void goke_face_get_one_yuv_udp(SDK_U32 channelIndex, SDK_U32 viId)
{
    SDK_ERR ret;
    GADI_PDA_Frame_InfoT info;
    SDK_U8 * uvBuffer = NULL;
    SDK_U8 * yBuffer = NULL;
    SDK_U8 fileName[64];
    SDK_U32 fileSize = 0;
    SDK_S32 uv_width, uv_height,uv_pitch;
    SDK_S32 wsize;
    SDK_S32 row,col;
    GADI_PDA_CAP_Buffer_TypeT buffer = GADI_PDA_CAP_MAIN_A_BUFFER;
    JPEG_SEND_HEADER jpegHeader;
    FILE *yuv_stream = NULL;

    if (0 == channelIndex)
    {
        buffer = GADI_PDA_CAP_MAIN_A_BUFFER;
    }
    else if (1 == channelIndex)
    {
        buffer = GADI_PDA_CAP_MAIN_B_BUFFER;
    }
    else if (2 == channelIndex)
    {
        buffer = GADI_PDA_CAP_SUB_A_BUFFER;
    }
    else if (4 == channelIndex)
    {
        buffer = GADI_PDA_CAP_SUB_C_BUFFER;
    }
    else
    {
        return ;
    }
    curIndex++;

    ret = gadi_pda_capture_buffer(GADI_PDA_CAP_SUB_A_BUFFER, &info);
    if(ret != SDK_OK)
    {
        GADI_ERROR("pda_capture_bmp: failed:%d\n", ret);

        goto error_exit;
    }

    if (curIndex % 2 == 0)
        goto error_exit;

    #if 0
    if (curIndex % 2 != 0)
    {
        return;
    }
    #endif

    //printf("gadi_pda_capture_buffer %d*%d stride:%d, type:%d\n", info.width, info.height, info.stride, info.frameFormat);

    yBuffer = malloc(info.width * info.height);
    if (yBuffer == NULL) {
        GADI_ERROR("Not enough memory for Y buffer:%dKB !\n",info.width * info.height);
        goto error_exit;
    }
    uvBuffer = malloc(info.width * info.height);
    if (uvBuffer == NULL) {
        GADI_ERROR("Not enough memory for UV buffer:%dKB !\n",info.width * info.height);
        goto error_exit;
    }

    sprintf((char*)fileName,"/tmp/pda_channel%d_%d_%d_yv12.yuv",channelIndex, info.width,info.height);

    if (info.stride== info.width)
    {
        memcpy(yBuffer, info.yAddr, info.width * info.height);
    } else if (info.stride > info.width)
    {
        delete_padding_from_strided_y(yBuffer,
        info.yAddr, info.stride, info.width, info.height);
    } else
    {
        GADI_ERROR("stride size smaller than width!\n");
        goto error_exit;
    }

    //convert uv data from interleaved into planar format
    if (FRAME_FORMAT_YUV_SEMIPLANAR_420 == info.frameFormat) {
        uv_pitch = info.stride/ 2;
        uv_width = info.width / 2;
        uv_height = info.height / 2;
    } else { // FRAME_FORMAT_YUV_SEMIPLANAR_422
        uv_pitch = info.stride/ 2;
        uv_width = info.width / 2;
        uv_height = info.height;
    }

    #if 1
    #if 0
    delete_padding_and_deinterlace_from_strided_uv(uvBuffer,
        info.uvAddr, uv_pitch, uv_width, uv_height,1);
    #endif
    #else
    SDK_U8 * output_u = uvBuffer;
    SDK_U8 * output_v = uvBuffer+info.width * info.height / 4;

	SDK_U8 * output_uv = info.uvAddr;

	//printf("uv %dx%d\n", uv_width, uv_height);

    for (row = 0; row < uv_height; row++) {
        for (col = 0; col < uv_width; col++) {
			if (FRAME_FORMAT_YUV_SEMIPLANAR_420 == info.frameFormat)
			{
                *output_u++ = *output_uv++;
                *output_v++ = *output_uv++;
			}
			else
			{
				if (row%2)
				{
                    *output_u++ = *output_uv++;
                    *output_v++ = *output_uv++;
				}
				else
				{
                    output_uv += 2;
				}
			}
        }
        output_uv += (uv_pitch - uv_width) * 2;
    }
    #endif
    struct timeval tt1, tt2;
    int t1;

    fileSize = info.width*info.height + uv_width*uv_height*2;
    #if 1
    //nv12
    #if 1
    jpegHeader.flag = 0x55443355;
    jpegHeader.len = fileSize;
    wsize = goke_face_send_data_udp(&jpegHeader, sizeof(jpegHeader));
    rotate_data_y(yBuffer, info.width, info.height);
    gettimeofday(&tt1, NULL);
    wsize = goke_face_send_data_udp(yBuffer, info.width*info.height);
    gettimeofday(&tt2, NULL);
    t1 = (tt2.tv_sec-tt1.tv_sec)*1000 + (tt2.tv_usec-tt1.tv_usec)/1000;
    if (wsize > 0)
    {
        //wsize += write(sockfd, uvBuffer, uv_width*uv_height*2);
        #if 0
        wsize += write(sockfd, info.uvAddr, uv_width*uv_height*2);
        #else
        char tmpi;
        int i;
        memcpy(uvBuffer, info.uvAddr, info.width * info.height);
        for (i  = 0;  i < uv_width*uv_height*2; )
        {
            tmpi = uvBuffer[i];
            uvBuffer[i] = uvBuffer[i + 1];
            uvBuffer[i + 1] = tmpi;
            i += 2;
        }
        rotate_data_uv(uvBuffer, uv_width, uv_height);
        wsize += goke_face_send_data_udp(uvBuffer, uv_width*uv_height*2);
        #endif
    }


    #endif
    #else
    sprintf(fileName, "%03d.yuv", nameIndex++);
    yuv_stream = fopen((char *)fileName,"wb");
    if(yuv_stream == NULL)
    {
        GADI_ERROR("pda_capture_bmp: open file error\n");
        goto error_exit;
    }
    rotate_data_y(yBuffer, info.width, info.height);
    wsize = fwrite(yBuffer, 1, info.width*info.height, yuv_stream);

    //nv21->nv12
    char tmpi;
    int i;
    memcpy(uvBuffer, info.uvAddr, info.width * info.height);
    for (i  = 0;  i < uv_width*uv_height*2; )
    {
        tmpi = uvBuffer[i];
        uvBuffer[i] = uvBuffer[i + 1];
        uvBuffer[i + 1] = tmpi;
        i += 2;
    }
    rotate_data_uv(uvBuffer, uv_width, uv_height);
    wsize += fwrite(uvBuffer, 1, uv_width*uv_height*2, yuv_stream);
    printf("write %03d len:%d, ret:%d\n", nameIndex - 1, fileSize, wsize);
    #endif

    //GADI_INFO(" Cap ture yuv picture success: %s\n",fileName);
error_exit:
    if(yBuffer != NULL)
    {
        free(yBuffer);
        yBuffer = NULL;
    }
    if(uvBuffer != NULL)
    {
        free(uvBuffer);
        uvBuffer = 0;
    }

    return ;
}


void goke_face_get_yuv_thread_udp(void *data)
{
    faceSockSearchfd = CreateBroadcastSock(5556);
    sdk_sys_thread_set_name("faceSend_udp");
    while(1)
    {
        goke_face_get_one_yuv_udp(0, 0);
    }
}

void goke_face_get_yuv_thread(void *data)
{
    sdk_sys_thread_set_name("faceyuv");
    while(1)
    {
        goke_face_get_one_yuv(0, 0);
    }
}

void goke_face_draw_thread(void *data)
{
    FACE_INFO_MSG faceInfoMsg;
    int isDraw = 0;
    sdk_sys_thread_set_name("faceDraw");
    while(1)
    {
        if (faceMsgId== -1)
        {
            sleep(1);
            continue;
        }
        if(msgrcv(faceMsgId, &faceInfoMsg, sizeof(FACE_INFO_MSG), 0, 0) != -1)
        {
            encoder_osd_draw_multi(faceInfoMsg.faceInfo, faceInfoMsg.faceResult);
        }
        else
        {
            printf("draw recv msg error.\n");
            sleep(1);
        }
    }
}

void goke_face_send_thread(void *data)
{
    int fileSize;
    int wsize;
    pthread_t recv_thread;
    int yuvIndex;
    int uvLen;
    JPEG_SEND_HEADER jpegHeader;

    sdk_sys_thread_set_name("faceSend");
    sockfd=socket(AF_INET,SOCK_STREAM,0);

    if(sockfd<0)
    {
        printf("create error");
    }
    //连接服务器，设置服务器的地址(ip和端口)
    struct sockaddr_in svraddr;
    memset(&svraddr,0,sizeof(svraddr));
    svraddr.sin_family=AF_INET;
    svraddr.sin_addr.s_addr= inet_addr(faceServerIp);
    svraddr.sin_port=htons(faceServerPort);
    printf("connect server:%s\n", faceServerIp);
    fdRet = connect(sockfd,(struct sockaddr *)&svraddr,sizeof(svraddr));
    if(fdRet < 0)
    {
        printf("connect error\n");
    }
    else
    {
        printf("connect ok\n");
    }


    pthread_create(&recv_thread, NULL, goke_face_msg_recv, NULL);
    //pthread_create(&recv_thread, NULL, goke_face_msg_recv_other, NULL);
    while(1)
    {
        if (faceMsgYuvId == -1)
        {
            sleep(1);
            continue;
        }

        if(msgrcv(faceMsgYuvId, &yuvIndex, sizeof(yuvIndex), 0, 0) != -1)
        {
            //printf("recevve yuv msg:%d\n", yuvIndex);
            if (fdRet < 0)
            {
                fdRet = connect(sockfd,(struct sockaddr *)&svraddr,sizeof(svraddr));
                printf("connect ret:%d\n", fdRet);
                sleep(1);
            }

            if (fdRet >= 0)
            {
                uvLen = yuvInfo.uvWidth * yuvInfo.uvHeight*2;
                fileSize = yuvInfo.width * yuvInfo.height + uvLen;
                jpegHeader.flag = 0x55443355;
                jpegHeader.len = fileSize;

                wsize = write(sockfd, &jpegHeader, sizeof(jpegHeader));
                rotate_data_y(yuvDatas[yuvIndex].yBuffer, yuvInfo.width, yuvInfo.height);
                wsize = write(sockfd, yuvDatas[yuvIndex].yBuffer, yuvInfo.width * yuvInfo.height);
                if (wsize > 0)
                {
#if 0
                    memcpy(uvBuffer, info.uvAddr, info.width * info.height);
                    rotate_data_uv(uvBuffer, uv_width, uv_height);
                    wsize += write(sockfd, info.uvAddr, uv_width*uv_height*2);
#else
                    char tmpi;
                    int i;
                    for (i  = 0; i < uvLen; )
                    {
                        tmpi = yuvDatas[yuvIndex].uvBuffer[i];
                        yuvDatas[yuvIndex].uvBuffer[i] = yuvDatas[yuvIndex].uvBuffer[i + 1];
                        yuvDatas[yuvIndex].uvBuffer[i + 1] = tmpi;
                        i += 2;
                    }
                    rotate_data_uv(yuvDatas[yuvIndex].uvBuffer, yuvInfo.uvWidth, yuvInfo.uvHeight);
                    wsize += write(sockfd, yuvDatas[yuvIndex].uvBuffer, uvLen);
#endif
                }

                //printf("write len:%d, ret:%d\n", fileSize, wsize);
                if (wsize == -1)
                {
                    fdRet = -1;
                    close(sockfd);
                    sockfd=socket(AF_INET,SOCK_STREAM,0);
                }
            }

            yuvDatas[yuvIndex].status = 0;
            //printf("yuvDatas[%d].status:%d\n", yuvIndex, yuvDatas[yuvIndex].status);
        }
        else
        {
            printf("yuv get msg error.\n");
            sleep(1);
        }


    }
}


void goke_face_start(void)
{
    pthread_t recv_thread;


    //fb_open();
    //fb_blank();
    screenRateW = (hWidth * 1.0)/360;
    screenRateH = (hHeight * 1.0)/640;
    printf("enter goke_face_start, %dx%d\n", hWidth, hHeight);
    memset(&faceInfoLast, 0, sizeof(faceInfoLast));
    memset(&faceResultLast, 0, sizeof(faceResultLast));
    memset(&yuvInfo, 0, sizeof(yuvInfo));
    memset(&yuvDatas, 0, sizeof(FACE_YUV_DATA) * MAX_BUFFER_NUM);

    faceMsgId = msgget((key_t)1234, 0666 | IPC_CREAT);
    if (faceMsgId == -1)
    {
        printf("msgget faceMsgId failed width error: %d\n", errno);
    }

    faceMsgYuvId  = msgget((key_t)1235, 0666 | IPC_CREAT);
    if (faceMsgYuvId  == -1)
    {
        printf("msgget faceMsgYuvId failed width error: %d\n", errno);
    }

    pthread_create(&recv_thread, NULL, goke_face_get_yuv_thread, NULL);
    pthread_create(&recv_thread, NULL, goke_face_send_thread, NULL);
    pthread_create(&recv_thread, NULL, goke_face_draw_thread, NULL);
    //pthread_create(&recv_thread, NULL, goke_face_get_yuv_thread_udp, NULL);
}

