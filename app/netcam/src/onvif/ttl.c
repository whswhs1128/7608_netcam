// author: xiaomo, at 2016/08/31

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "type.h"
#include "http_export.h"
#include "httpClient.h"
#include "net_time.h"

//#define URL "http://169.254.1.109:8080/reportAlarmData"
#define VIDEO_NAME "./stream.h264"
#define BUF_LEN 1024
unsigned char rx_buf[BUF_LEN] = {0};
#define VIDEO_LEN 4096
// HTTP_MULTI_META_DATA *meta_data;
char send_x[8];
extern int uart;
extern char rtsp_url[64];
enum
{
    TYPE_HEADER,
    TYPE_DATA,
};

struct _SERIAL_DATA
{
    int stat;
    unsigned char recvBuf[BUF_LEN];
    int data_len;
    int pos;
    int x;
};

struct FORM_DATA
{
    char nodeNum[5];
    char alarmCode[8];
    float azimuth;
    float elevation;
    float distance;
    char alarmDate[16];
    char appId[16];
    char signture[128];
    float spl;
    char video[VIDEO_LEN];
    int flag;
};

void print_hex_array(int len, unsigned char *arr);
struct _SERIAL_DATA serial_data;
struct FORM_DATA send_data_x;
char *host = NULL;
char *path = NULL;
unsigned char buf_x[BUF_LEN] = {0};

void send_data_init()
{
    memset(&send_data_x, 0, sizeof(struct FORM_DATA));
    strcpy(send_data_x.nodeNum, "10001");
    strcpy(send_data_x.appId, "ipctest");
    strcpy(send_data_x.signture, "2ff8f22b6992569daa85f0f09bbac988f80e126f51d2a60f1ada1f54d0681f53");
    send_data_x.azimuth = 0;
    send_data_x.distance = 0;
    send_data_x.elevation = 0;
    send_data_x.spl = 0;
    memset(send_data_x.alarmCode, 0, sizeof(send_data_x.alarmCode));
    memset(send_data_x.video, 0, sizeof(send_data_x.video));
    memset(send_data_x.alarmDate, 0, sizeof(send_data_x.alarmDate));
    send_data_x.flag = 0;
}

uint32_t pelco_query_position()
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    uint32_t sum = 0;
    bytes[length] = 0xff;
    length++;
    bytes[length] = 0x01;
    length++;
    bytes[length] = 0x9a;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    for (i = 1; i < length; ++i)
    	sum += bytes[i];
    bytes[length] = sum & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

void send_absolutemove_value(int uart_fd, float panPosition, float tiltPosition, float zoomPosition)
{
    //printf("pantilt_pos_x is %f\n", panPosition);
    //printf("pantilt_pos_y is %f\n", tiltPosition);
    //printf("pantilt_pos_z is %f\n", zoomPosition);
    uint32_t sum = 0;
    unsigned char tx_buf[13] = {0};
//    float p_pos, t_pos, z_pos;
//    p_pos = ((int)(panPosition * 10) / 10.0);
//    t_pos = ((int)(tiltPosition * 10) / 10.0);
//    z_pos = ((int)(zoomPosition * 10) / 10.0);
    tx_buf[0] = 0xff;
    tx_buf[1] = 0x01;
    tx_buf[2] = 0x99;
    tx_buf[3] = (int)((panPosition + 1) * 128) & 0xff;
    tx_buf[4] = (int)((panPosition + 1) * 128 * 256) & 0xff;
    tx_buf[5] = (int)((0 - tiltPosition + 1) * 128) & 0xff;
    tx_buf[6] = (int)((0 - tiltPosition + 1) * 128 * 256) & 0xff;
    tx_buf[7] = 0xff;
    tx_buf[8] = 0xff;
    tx_buf[9] = (int)((zoomPosition )*256) & 0xff;
    tx_buf[10] = (int)((zoomPosition )*256 * 256)&0xff;
    tx_buf[11] = 0xff;
    for (int i = 1; i <= 11; i++)
	sum += tx_buf[i];

    tx_buf[12] = sum & 0xff;
    int tx_length = sizeof(tx_buf) / sizeof(tx_buf[0]);
    do_transmit(uart_fd, tx_length, tx_buf);
//    pelco_query_position();
}

void floattochar(float data, char* send_x)
{

    snprintf(send_x, sizeof(send_x), "%f", data);
}

int mcu_uart_com_init()
{
    // 初始化结构体
    memset(&serial_data, 0, sizeof(struct _SERIAL_DATA));
    serial_data.stat = TYPE_HEADER;
    serial_data.pos = 0;
    serial_data.data_len = 0;
    serial_data.x = 0;
    memset(serial_data.recvBuf, 0, sizeof(serial_data.recvBuf));
}

#include "cfg_system.h"
#include "netcam_api.h"
#include "utility_api.h"
#include "mmc_api.h"
#include "sdk_sys.h"

void parse_485(int len, unsigned char *arr)
{
    int i = 0;
    char a[4];
    int ret;
    char send0[8] ={0}, send1[8]={0}, send2[8]={0}, send3[8]={0};
    if (len == 32 &&send_data_x.flag == 0)
    {
	send_data_x.flag = 1;
        for (i = 0; i < 4; i++)
            a[i] = *(arr + 7 - i);
        send_data_x.azimuth = *((float *)a);
        for (i = 0; i < 4; i++)
            a[i] = *(arr + 12 - i);
        send_data_x.elevation = *((float *)a);
        for (i = 0; i < 4; i++)
            a[i] = *(arr + 17 - i);
        send_data_x.spl = *((float *)a);
        for (i = 0; i < 4; i++)
            a[i] = *(arr + 22 - i);
        send_data_x.distance = *((float *)a);
        for (i = 0; i < 8; i++)
 	    send_data_x.alarmCode[i] = *(arr + 24 + i);
    floattochar(send_data_x.azimuth, send0);
    floattochar(send_data_x.elevation, send1);
    floattochar(send_data_x.spl, send2);
    floattochar(send_data_x.distance, send3);
    int dataLength = 0;
    int dataRecvLen = 0;
    float p_pos, t_pos,z_pos;
    if(send_data_x.azimuth < 180) {
        p_pos = 0 - send_data_x.azimuth / 180.0;
    } else {
        p_pos = 0 - send_data_x.azimuth / 180.0 + 2.0;
    }
    t_pos = send_data_x.elevation * (-1.0 / 45.0) + 1;
    z_pos = 0;
    send_absolutemove_value(uart, p_pos, t_pos, z_pos);

    if(!runSystemCfg.ntpCfg.enable485) {
	send_data_x.flag = 0;
	return;
    }
    HTTP_MULTI_META_DATA *meta_data = goke_http_form_data_create();
    goke_http_form_data_add(meta_data, "alarmDate", send_data_x.alarmDate, strlen(send_data_x.alarmDate), MULTIPART_DATA_TXT, NULL);
    goke_http_form_data_add(meta_data, "azimuth",send0,32,MULTIPART_DATA_TXT, NULL);
    goke_http_form_data_add(meta_data,"elevation",send1,32,MULTIPART_DATA_TXT, NULL);
    goke_http_form_data_add(meta_data, "spl", send2, 32, MULTIPART_DATA_TXT, NULL);
    goke_http_form_data_add(meta_data, "distance",send3,32,MULTIPART_DATA_TXT, NULL);
    goke_http_form_data_add(meta_data, "alarmCode", send_data_x.alarmCode, 32, MULTIPART_DATA_TXT, NULL);
    //goke_http_form_data_add(meta_data, "signture", send_data_x.signture, strlen(send_data_x.signture), MULTIPART_DATA_TXT, NULL);
    //goke_http_form_data_add(meta_data, "appId", send_data_x.appId, strlen(send_data_x.appId), MULTIPART_DATA_TXT, NULL);
    goke_http_form_data_add(meta_data, "rtspUrl", rtsp_url, 64,  MULTIPART_DATA_TXT, NULL);
    // goke_http_form_data_add(meta_data, "video", send_data_x.video, 0, MULTIPART_DATA_STREAM, VIDEO_NAME);
    goke_http_form_data_add(meta_data, "rs485type", "0", 32, MULTIPART_DATA_TXT, NULL);
    char post_url[64];
    strcpy(post_url, runSystemCfg.ntpCfg.platurl);
    goke_http_form_data_request(post_url, meta_data, &dataLength, &dataRecvLen);
    send_data_x.flag = 0;
    }
}

void parse_come_velocity(int len, unsigned char *arr)
{
    int i = 0;
    char speed[10];

    sprintf(speed,"%d",arr[2]);
    send_data_x.flag = 1;
    int dataLength = 0;
    int dataRecvLen = 0;

    if(!runSystemCfg.ntpCfg.enable485) {
	        send_data_x.flag = 0;
	        return;
        }
    HTTP_MULTI_META_DATA *meta_data = goke_http_form_data_create();
    goke_http_form_data_add(meta_data, "come_velocity", speed, strlen(speed), MULTIPART_DATA_TXT, NULL);
    goke_http_form_data_add(meta_data, "rs485type", "1", 32, MULTIPART_DATA_TXT, NULL);

    char post_url[64];
    strcpy(post_url, runSystemCfg.ntpCfg.platurl);
    goke_http_form_data_request(post_url, meta_data, &dataLength, &dataRecvLen);
    send_data_x.flag = 0;
    
}

void parse_left_velocity(int len, unsigned char *arr)
{
    int i = 0;
    char speed[10];

    sprintf(speed,"%d",arr[2]);
    send_data_x.flag = 1;
    int dataLength = 0;
    int dataRecvLen = 0;

    if(!runSystemCfg.ntpCfg.enable485) {
	        send_data_x.flag = 0;
	        return;
        }
    HTTP_MULTI_META_DATA *meta_data = goke_http_form_data_create();
    goke_http_form_data_add(meta_data, "left_velocity", speed, strlen(speed), MULTIPART_DATA_TXT, NULL);
    goke_http_form_data_add(meta_data, "rs485type", "1", 32, MULTIPART_DATA_TXT, NULL);

    char post_url[64];
    strcpy(post_url, runSystemCfg.ntpCfg.platurl);
    goke_http_form_data_request(post_url, meta_data, &dataLength, &dataRecvLen);
    send_data_x.flag = 0;
    
}

/*
 * 接收到串口数据后的处理函数
 *
 */

void handle_recv_data(const unsigned char *buf, int len)
{
    int i = 0;

    while (len > 0)
    {
        // 判断当前的状态机状态
        switch (serial_data.stat)
        {
        case TYPE_HEADER: // 消息头
            if (buf[i] == 0xdc)
            {
                serial_data.stat = TYPE_DATA;
                if (buf[i + 1] == 0xff)
                    serial_data.data_len = 32;
                else if (buf[i + 1] == 0xfe)
                    serial_data.data_len = 8;
                memcpy(serial_data.recvBuf, buf + i, 2);
                i += 2;
                serial_data.pos += 2;
                len -= 2;
            }
            else if (buf[i] == 0xfc) {
                serial_data.stat = TYPE_DATA;
                if (buf[i + 1] == 0xfa)
                    serial_data.data_len = 4;
                memcpy(serial_data.recvBuf, buf + i, 2);
                i += 2;
                serial_data.pos += 2;
                len -= 2;
            }
            else if (buf[i] == 0xfb) {
                serial_data.stat = TYPE_DATA;
                if (buf[i + 1] == 0xfd)
                    serial_data.data_len = 4;
                memcpy(serial_data.recvBuf, buf + i, 2);
                i += 2;
                serial_data.pos += 2;
                len -= 2;
            }
            else
            {
//                serial_data.x = len;
//                serial_data.stat = TYPE_DATA;
//                memcpy(serial_data.recvBuf, buf + i, 1);
//                len--;
//                i++;
//                serial_data.pos++;
                mcu_uart_com_init();
		return;
            }
            continue;
        case TYPE_DATA:                      // 状态机为data
            serial_data.recvBuf[serial_data.pos] = buf[i]; // 缓存当前buf到缓存recvBuf中，此处recvBuf更新后0xff 0xee 0x00 0xXX,前四个字节已经填充完毕
            // 更新结构体下标
            serial_data.pos++;
            // 更新当前串口数据的长度；
            len--;
            i++;

            if (serial_data.pos == (serial_data.data_len))
            {
                // 已经组成完整的数据包:0xff 0xee 数据长度 .... CRC
                //  next loop

                //for (int j = 0; j < serial_data.data_len; j++)
                //    buf_x[j + serial_data.buf_un] = serial_data.recvBuf[j];
                // memcpy(buf_x+serial_data.buf_un, serial_data.recvBuf ,serial_data.data_len);
                if(serial_data.data_len == 32)
                    parse_485(serial_data.data_len, serial_data.recvBuf);
                else {
                    if(serial_data.recvBuf[0] == 0xfc)
                        parse_come_velocity(serial_data.data_len, serial_data.recvBuf);
                    else
                        parse_left_velocity(serial_data.data_len, serial_data.recvBuf);
                }

                mcu_uart_com_init();
            }
            else
            {
//                mcu_uart_com_init();
//                //strcat(buf_x, serial_data.recvBuf);
//                //serial_data.stat = TYPE_HEADER;
//                //serial_data.buf_un = serial_data.pos;
            }
            continue;
        default:
            break;
        }
    }
}

void print_hex_array(int len, unsigned char *arr)
{
    int i = 0;
    for (; i < len; ++i)
    {
        printf("0x%02x ", *(arr + i));
    }
    printf("\n");
}

int init_uart(int num)
{
    //-------------------------
    //----- SETUP UART 0 -----
    //-------------------------
    int uart_fs = -1;

    // OPEN THE UART
    char a[32];
    sprintf(a, "%s%d", "/dev/ttyAMA", num);
    uart_fs = open(a, O_RDWR | O_NOCTTY); // | O_NDELAY);
    if (uart_fs == -1)
    {
        // ERROR - CAN'T OPEN SERIAL PORT
        printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
    }
    else
    {
        printf("Opend UART port OK, number: %d\n", uart_fs);
    }

    // CONFIGURE THE UART
    struct termios options;
    tcgetattr(uart_fs, &options);
        if(num == 0) {
            options.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
        } else {
            options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
        }
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart_fs, TCIFLUSH);
    tcsetattr(uart_fs, TCSANOW, &options);
    return uart_fs;
}

void do_transmit(int uart_fd, int send_bytes, unsigned char *p_tx_buffer)
{
    if (uart_fd < 0)
    {
        return;
    }

    int count = write(uart_fd, p_tx_buffer, send_bytes);
}

int checksum(int size, unsigned char *cur_line)
{
    int i = 0, checksum = 0;
    for (; i < size; ++i)
    {
        if (i != 1 && i != size - 1)
        {
            checksum += *(cur_line + i);
        }
    }
    return ~checksum & 255;
}

void on_receive(int uart_fd)
{
    int rx_length = read(uart_fd, (void *)rx_buf, BUF_LEN - 1);
}

void close_uart(int uart_fs)
{
    close(uart_fs);
}

void *rx_receive(void *uart)
{
    int uart_fd = uart;
    while (1)
    {
        on_receive(uart_fd);
    }
}

void on_receive_485(int uart_fd)
{
    if (uart_fd < 0)
        return;
    unsigned char rx_buf[BUF_LEN] = {0};
    int rx_length = read(uart_fd, (void *)rx_buf, BUF_LEN - 1);
    handle_recv_data(rx_buf, rx_length);
}

void *begin_485()
{
    int uart_1 = init_uart(1);
    send_data_init();
    mcu_uart_com_init();
    while (1)
    {
        on_receive_485(uart_1);
    }
}

void send_af_value(int uart_fd, GK_U16 focus1, GK_U16 focus2, GK_U8 agc)
{
    unsigned char tx_buf_af1[16];
    uint32_t sum = 0;
    *(tx_buf_af1) = 0x51;
    *(tx_buf_af1 + 1) = 0x01;
    *(tx_buf_af1 + 2) = 0x04;
    *(tx_buf_af1 + 3) = 0x78;
    *(tx_buf_af1 + 4) = 0x01;
    *(tx_buf_af1 + 5) = 0x0d;
    *(tx_buf_af1 + 6) = (focus2 & 0xFF00) >> 8;
    *(tx_buf_af1 + 7) = focus2 & 0x00FF;
    *(tx_buf_af1 + 8) = (focus1 & 0xFF00) >> 8;
    *(tx_buf_af1 + 9) = focus1 & 0x00FF;
    *(tx_buf_af1 + 10) = agc & 0x00FF;
    *(tx_buf_af1 + 11) = 0x00;
    *(tx_buf_af1 + 12) = 0x02;
    *(tx_buf_af1 + 13) = 0x00;
    *(tx_buf_af1 + 14) = 0x00;

    for (int i = 0; i <= 14; i++)
	sum += tx_buf_af1[i];
        
    tx_buf_af1[15] = sum & 0xff;
    int tx_length = sizeof(tx_buf_af1) / sizeof(tx_buf_af1[0]);
    do_transmit(uart_fd, tx_length, tx_buf_af1);
}

#define PELCO_SYNC 0xFF
#define PELCO_CMD 0x01

#define UP 0x08
#define DOWN 0x10
#define LEFT 0x04
#define RIGHT 0x02
#define ZOOM_TELE 0x20
#define ZOOM_WIDE 0x40
#define FOCUS_FAR 0x80
#define FOCUS_NEAR 0x01
#define AGC_FOCUS_VALUE 0x80

#define SET_PRESET 0x03
#define DEL_PRESET 0x05
#define CALL_PRESET 0x07

uint32_t pelco_set_stop()
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}


uint32_t pelco_set_up(unsigned char speed)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = UP;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = speed;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_down(unsigned char speed)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = DOWN;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = speed;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_left(unsigned char speed)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = LEFT;
    length++;
    bytes[length] = speed;
    length++;
    bytes[length] = 0;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_right(unsigned char speed)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = RIGHT;
    length++;
    bytes[length] = speed;
    length++;
    bytes[length] = 0;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_left_top(unsigned char pan_speed)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = UP | LEFT;
    length++;
    bytes[length] = 0xFF;
    length++;
    bytes[length] = 0xFF;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_right_top(unsigned char pan_speed)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = UP | RIGHT;
    length++;
    bytes[length] = 0xFF;
    length++;
    bytes[length] = 0xFF;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_left_bottom(unsigned char pan_speed)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = DOWN | LEFT;
    length++;
    bytes[length] = 0xFF;
    length++;
    bytes[length] = 0xFF;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_right_bottom(unsigned char pan_speed)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = DOWN | RIGHT;
    length++;
    bytes[length] = 0xFF;
    length++;
    bytes[length] = 0xFF;
    length++;
    //for (i = 1; i < length; ++i)
    //    bytes[length] = (bytes[length] + bytes[i]) & 0xff;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_preset(unsigned char num)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = SET_PRESET;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = num;
    length++;
//    for (i = 1; i < length; ++i) {
//        bytes[length] += bytes[i];
//    }
//    bytes[length] = bytes[length] & 0xff;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_call_preset(unsigned char num)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = CALL_PRESET;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = num;
    length++;
   // for (i = 1; i < length; ++i)  {
   //     bytes[length] += bytes[i];
   // }
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_del_preset(unsigned char num)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = DEL_PRESET;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = num;
    length++;
    //for (i = 1; i < length; ++i)
    //    bytes[length] += bytes[i];
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_zoom_tele()
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = ZOOM_TELE;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_zoom_wide()
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = ZOOM_WIDE;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_focus_far()
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = FOCUS_FAR;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_focus_near()
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = FOCUS_NEAR;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    bytes[length] = 0;
    length++;
    bytes[6] = (bytes[1] + bytes[2] + bytes[3] + bytes[4] + bytes[5]) & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

uint32_t pelco_set_agc_and_focus_value(uint8_t agc, uint16_t focus1, uint16_t focus2)
{
    unsigned char bytes[32];
    uint32_t i, length = 0;
    uint32_t sum = 0;
    bytes[length] = PELCO_SYNC;
    length++;
    bytes[length] = PELCO_CMD;
    length++;
    bytes[length] = AGC_FOCUS_VALUE;
    length++;
    bytes[length] = agc;
    length++;
    bytes[length] = (focus2 & 0xFF00) >> 8;
    length++;
    bytes[length] = focus2 & 0x00FF;
    length++;
    bytes[length] = (focus1 & 0xFF00) >> 8;
    length++;
    bytes[length] = focus1 & 0x00FF;
    length++;
    for (i = 1; i < length; ++i)
    	sum += bytes[i];
    bytes[length] = sum & 0xff;
    length++;
    do_transmit(uart, length, bytes);
}

#if 0 
int main()
{
    int uart = init_uart();
    while (1)
    {
        on_receive(uart);
        sleep(1);
    }
    close_uart(uart);
    return 0;
}
#endif
