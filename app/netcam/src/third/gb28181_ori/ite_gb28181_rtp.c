/*
*/

#include <sys/time.h>
#include <sys/ioctl.h>
#include "ite_sip_api.h"
#include <ite_typedef.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
//#include <ite_common.h>
//#include <ite_msg_define.h>
//#include <ite_msg_key.h>
//#include <ite_message.h>
#include <ite_shared_buffer.h>
#include "g711.h"
#include "media_fifo.h"
#include "netcam_api.h"
#include "ite_gb28181_playback.h"

#define FRAMERATE 25
#define MAX_CHANNEL_NUM 7

#define SOCK_BUF_SIZE          64*1024
#define MAX_UDP_PORT_NUM    128
#define UDP_PORT_BASE       9128
#define M_RTP_PAYLOAD_LEN  1392
#define RTP_HEADER_LEN      12
#define EXT_RTP_HREADER (RTP_HEADER_LEN + 4)
#define EXT_RTP_ID          0x24

#define VIDEO_RTP           0x00
#define VIDEO_RTCP          0x01
#define AUDIO_RTP           0x02
#define AUDIO_RTCP          0x03


#define ACT_SOCKET_PORT     9
#define TCP_LISTEN_PORT     554

#define M_RTP_VER       2


#define HEADLEN_MAX 1024
#define HEADADD_LEN 128
#define M_RTP_PAYLOAD_LEN                   1392

#define M_FRAME_TYPE_I                  0
#define M_FRAME_TYPE_P                  1
#define M_FRAME_TYPE_AUDIO              2
#define M_FRAME_TYPE_ERR               -1
#define RTCP_INTERVAL       3


#define PS_HDR_LEN  14
#define SYS_HDR_LEN 18
#define PSM_HDR_LEN 24
#define PES_HDR_LEN 14

#if 0
#define RTP_PADDING
#ifdef RTP_PADDING
#define RTP_PADDING_LEN 5
#else
#define RTP_PADDING_LEN 0
#endif
#else

#define RTP_PADDING_LEN 5
#endif
//#define RTP_EXTERN

#ifdef RTP_EXTERN
#define RTP_HDR_LEN 18  //12 + 6 extention
#define RTP_TCP_HDR_LEN 20  //12 + 2 + 6 extention
#else
#define RTP_HDR_LEN 12
#define RTP_TCP_HDR_LEN 14
#endif

#define RTP_MAX_PACKET_BUFF 1300
#define PS_PES_PAYLOAD_PACKSIZE 0xF000
#define PS_PYLOAD_MAX_SIZE 0x10000

#define PLAYBACK_CHAN_NUM	2

typedef struct
{
    unsigned char type;
    unsigned char pt;
    unsigned short seq;
    unsigned int ts;
    unsigned int ssrc;
} RTPINFO;

typedef struct
{
    char *sps;
    int spsLen;
    char *pps;
    int ppsLen;
    char *sei;
    int seiLen;
    char *pFrame;
    int frameLen;
} I_FRAME_INFO;

typedef struct
{
    char *sei;
    int seiLen;
    char *pFrame;
    int frameLen;
} P_FRAME_INFO;

typedef struct _RTCP_header
{
    unsigned int version: 2;
    unsigned int padding: 1;
    unsigned int count: 5;
    unsigned int pt: 8;
    unsigned int length: 16;
} RTCP_header;

typedef struct _RTCP_header_SR
{
    unsigned int ssrc;
    unsigned int ntp_timestampH;
    unsigned int ntp_timestampL;
    unsigned int rtp_timestamp;
    unsigned int pkt_count;
    unsigned int octet_count;
} RTCP_header_SR;

typedef struct _RTCP_header_RR
{
    unsigned int ssrc;
} RTCP_header_RR;

struct rtcp_pkt_sr
{
    RTCP_header comm;
    RTCP_header_SR sdec;
} rtcp_pkt_sr;

struct rtcp_pkt_rr
{
    RTCP_header comm;
    RTCP_header_RR sdec;
    char res[20];
} rtcp_pkt_rr;

/**
 ** RTP header body
 **/
typedef struct
{
    unsigned char    flags;         /* Version(2),Padding(1), Ext(1), Cc(4) */
    unsigned char    mk_pt;         /* Marker(1), PlayLoad Type(7) */
    unsigned short sq_nb;           /* Sequence Number */
    unsigned int     ts;            /* Timestamp */
    unsigned int     ssrc;          /* SSRC */
} rtp_hdr;


//extern volatile MEDIA_INFO g_veMediaInfo;
extern volatile int g_bterminate ;
extern volatile int g_streamId ;
ite_rtp_Info SocketInfo;

volatile int g_work = 0 ;

static pthread_mutex_t portPoolMutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned char portPool[MAX_UDP_PORT_NUM / 8];
static int g_sendIframe = 0;
static pthread_mutex_t rtphandleMutex = PTHREAD_MUTEX_INITIALIZER;

extern ite_gb28181Obj sip_gb28281Obj;

typedef struct  bits_buffer_t
{
    unsigned char i_size ;
    unsigned char i_data;
    unsigned char i_mask;
    unsigned char *p_data ;
} bits_buffer_s;

typedef struct  frameData_Info_t
{
    unsigned int u32CurPts;
    unsigned int u32Ssrc;
    unsigned int u32Sec;
    unsigned short u16CSeq ;
    unsigned short IFrame;
    unsigned char szBuff[2048] ;
    unsigned char paddingFlg;
    unsigned int streamType;
    unsigned int u32RtpPts;
} frameData_Info_s;


/*
TS_TEST_GOKE 表示云存测试直接发往测试服务器
TS_TEST_SERVER_ALL 表示同时向魔镜服务器及测试服务器发送数据
TS_TEST_GOKE 与 TS_TEST_SERVER_ALL不能同时打开
*/
//#define TS_TEST_GOKE
#ifndef TS_TEST_GOKE
//#define TS_TEST_SERVER_ALL
#endif

int tsTestFd = -1;
char hostAddr[] = "nas.lpwei.com";
char tsTestIp[64] = "192.168.10.xxx"; //192.168.10.xxx
int tsTestPort = 8000;




extern int rtp_get_data_init(unsigned int streamId);
extern int rtp_get_data(int handle, void **data, int *size, int *type, unsigned int *pts);
extern int rtp_get_data_close(int handle);

/***
 *@remark:  讲传入的数据按地位一个一个的压入数据
 *@param :  buffer   [in]  压入数据的buffer
 *          count    [in]  需要压入数据占的位数
 *          bits     [in]  压入的数值
 */
#define bits_write(buffer, count, bits)\
{\
    bits_buffer_s *p_buffer = (buffer);\
    int i_count = (count);\
    unsigned int i_bits = (bits);\
    while( i_count > 0 )\
    {\
        i_count--;\
        if( ( i_bits >> i_count ) & 0x01 )\
        {\
            p_buffer->p_data[p_buffer->i_data] |= p_buffer->i_mask;\
        }\
        else\
        {p_buffer->p_data[p_buffer->i_data] &= ~p_buffer->i_mask;}\
        p_buffer->i_mask >>= 1;\
        if( p_buffer->i_mask == 0 )\
        {\
            p_buffer->i_data++;\
            p_buffer->i_mask = 0x80;\
        }\
    }\
}

static ite_rtp_Info rtpInfo[MAX_CHANNEL_NUM];
static ite_rtp_Info *rtp_playback_info[PLAYBACK_CHAN_NUM] = {NULL,NULL};
volatile int isPlaybackSending[PLAYBACK_CHAN_NUM] = {0,0};

static unsigned int cloudSendCnt = 0;
static unsigned int cloudSuccessCnt = 0;
static char errinfo[MAX_CHANNEL_NUM][100];
void rtp_stream_status_send(int rtptype, char *ip, int port, int ssrc, int channel,int result, char is_connect, int begin, int end, int total, float *speed);
unsigned int MjEvtcloudSendCnt = 0;
unsigned int MjEvtcloudSuccessCnt = 0;


static void RtpSeqAdd(unsigned short *seq);

int CreateTcpSocket(ite_rtp_Info *pSocketInfo);
int CreateUdpSocket(ite_rtp_Info *pSocketInfo);


int ite_check_test_server(void)
{
#ifdef TS_TEST_GOKE
    if (strcmp(tsTestIp, "192.168.10.xxx") == 0)
    {
        struct hostent* host=gethostbyname(hostAddr);
        if (!host)
        {
            printf("get host:%s ip addr error\n", hostAddr);
        }
        else
        {
            strncpy(tsTestIp, inet_ntoa(*((struct in_addr *)host->h_addr_list[0])), 16);
            printf("get host ip ok:%s\n", tsTestIp);
        }
    }
    mojing_start_cloud_test();
    return 1;
#else
    return 0;
#endif
}


/***
 *@remark:   ps头的封装,里面的具体数据的填写已经占位，可以参考标准
 *@param :   pData  [in] 填充ps头数据的地址
 *           s64Src [in] 时间戳
 *@return:   0 success, others failed
*/

#define marker_bit                  0x1
void  ite_make_ps_header(char *pData, unsigned int u32Scr)//14 Bytes
{
    unsigned int system_clock_reference_base, temp;
    unsigned int system_clock_reference_extension;
    unsigned long mux_rate = 6106;
    system_clock_reference_base = 0;
    system_clock_reference_extension = 0;

    system_clock_reference_base = u32Scr;
    system_clock_reference_extension = 0;



    bits_buffer_s   bitsBuffer;
    bitsBuffer.i_size = PS_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80; // 二进制：10000000 这里是为了后面对一个字节的每一位进行操作，避免大小端夸字节字序错乱
    bitsBuffer.p_data = (unsigned char *)(pData);
    memset(bitsBuffer.p_data, 0, PS_HDR_LEN);


    bitsBuffer.p_data[0] = 0x00;
    bitsBuffer.p_data[1] = 0x00;
    bitsBuffer.p_data[2] = 0x01;
    bitsBuffer.p_data[3] = 0xBA;

    bitsBuffer.p_data[4] = 0x40;
    #if 0
    bitsBuffer.p_data[4] |= (system_clock_reference_base >> 30 << 3) & 0x38; //2
    bitsBuffer.p_data[4] |= (marker_bit << 2) & 0x04;
    bitsBuffer.p_data[4] |= (system_clock_reference_base >> 28) & 0x3; //4=2+2

    bitsBuffer.p_data[5] = (system_clock_reference_base >> 20) & 0xff; //12=4+8

    bitsBuffer.p_data[6] = ((system_clock_reference_base >> 15) & 0x1f) << 3;  // 17=12+5
    bitsBuffer.p_data[6] |= (marker_bit << 2) & 0x4;
    bitsBuffer.p_data[6] |= (system_clock_reference_base >> 13) & 0x3; //19=17+2

    bitsBuffer.p_data[7] = (system_clock_reference_base >> 5) & 0xff; // 27=19+8
    
    bitsBuffer.p_data[8] = system_clock_reference_base & 0x1f << 3; //32
    bitsBuffer.p_data[8] |= (marker_bit << 2) & 0x4;
    bitsBuffer.p_data[8] |= (system_clock_reference_extension >> 7) & 0x3; //20=18+2
    
    bitsBuffer.p_data[9] = (system_clock_reference_extension & 0x7f) << 1;
    #else
    temp = (system_clock_reference_base >> 24) & 0xff;
    bitsBuffer.p_data[4] = 0x40;
    bitsBuffer.p_data[4] |= (temp >> 3) & 0x18;
    bitsBuffer.p_data[4] |= (marker_bit << 2) & 0x04;
    bitsBuffer.p_data[4] |= (temp >> 4) & 0x3;

    bitsBuffer.p_data[5] = (temp << 4) & 0xf0; //4
    temp = (system_clock_reference_base >> 16) & 0xff;
    bitsBuffer.p_data[5] |= (temp >> 4) & 0x0f;//4

    bitsBuffer.p_data[6] = (temp << 4) & 0xf0;  // 4
    temp = (system_clock_reference_base >> 8) & 0xff;

    bitsBuffer.p_data[6] |= (temp >> 4) & 0x8; // 1
    bitsBuffer.p_data[6] |= (marker_bit << 2) & 0x4; //1
    bitsBuffer.p_data[6] |= (temp >> 5) & 0x3; // 2

    bitsBuffer.p_data[7] = (temp << 3) & 0xf8; // 5
    temp = system_clock_reference_base & 0xff;
    bitsBuffer.p_data[7] |= (temp >> 5) & 0x07; // 3

    bitsBuffer.p_data[8] = (temp << 3) & 0xf1; // 5
    bitsBuffer.p_data[8] |= (marker_bit << 2) & 0x4; // 1
    temp = (system_clock_reference_extension >> 8) & 0x1;
    bitsBuffer.p_data[8] |= (temp << 1) & 0x2; // 1
    temp = system_clock_reference_extension & 0xff;
    bitsBuffer.p_data[8] |= (temp >> 7) & 0x1;

    bitsBuffer.p_data[9] = (temp << 1) & 0xfe;
    #endif
    bitsBuffer.p_data[9] |= marker_bit;
    /*program_mux_rate*/
    mux_rate <<= 2;
    mux_rate |= 0x03;
    bitsBuffer.p_data[10] = mux_rate >> 16;
    bitsBuffer.p_data[11] = mux_rate >> 8;
    bitsBuffer.p_data[12] = mux_rate;
    bitsBuffer.p_data[13] = 0xf8;
}

#if 0
int ite_make_ps_header(char *pData, unsigned int u32Scr)
{
#define MUX_RATE_BYTE (20*1024*1024)   //20MB
#define MUX_RATE MUX_RATE_BYTE/50

    // 这里除以100是由于sdp协议返回的video的频率是90000，帧率是25帧/s，所以每次递增的量是3600,
    // 所以实际你应该根据你自己编码里的时间戳来处理以保证时间戳的增量为3600即可，
    //如果这里不对的话，就可能导致卡顿现象了
    bits_buffer_s   bitsBuffer;
    bitsBuffer.i_size = PS_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80; // 二进制：10000000 这里是为了后面对一个字节的每一位进行操作，避免大小端夸字节字序错乱
    bitsBuffer.p_data = (unsigned char *)(pData);
    memset(bitsBuffer.p_data, 0, PS_HDR_LEN);


    char temp = 0x00;

    bitsBuffer.p_data[0] = 0x00;
    bitsBuffer.p_data[1] = 0x00;
    bitsBuffer.p_data[2] = 0x01;
    bitsBuffer.p_data[3] = 0xBA;
    bitsBuffer.p_data[4] = ((0x38 & (u32Scr >> 26)) | 0x44);
    bitsBuffer.p_data[4] = ( bitsBuffer.p_data[4] | ((u32Scr >> 28) & 0x03));

    bitsBuffer.p_data[5] = ((u32Scr >> 20) & (0xFF));
    temp = ((u32Scr >> 12) & (0xF8));
    bitsBuffer.p_data[6] = (temp | 0x04 | ((u32Scr >> 13) & 0x03));

    bitsBuffer.p_data[7] = ((u32Scr >> 5) & (0xFF));
    temp = ((((u32Scr & 0x1f) << 3) & 0xf8) | 0x04);

    //system_clock_reference_extension 设为0  9个bit都置位为0
    bitsBuffer.p_data[8] = (temp | 0x03);
    bitsBuffer.p_data[9] = 0x01;

    bitsBuffer.p_data[10] = (MUX_RATE >> 14) & 0xff;
    bitsBuffer.p_data[11] = (MUX_RATE >> 6) & 0xff;
    bitsBuffer.p_data[12] = (((MUX_RATE << 2) & 0xfc) | 0x03);


    bitsBuffer.p_data[13] = 0xF8;
    //  bitsBuffer.p_data[14] = 0xFF;
    //  bitsBuffer.p_data[15] = 0xFF;


#if 0
    bits_write(&bitsBuffer, 32, 0x000001BA);            /*start codes*/
    bits_write(&bitsBuffer, 2,  1);                     /*marker bits '01b'*/
    bits_write(&bitsBuffer, 3,  (s64Scr >> 30) & 0x07); /*System clock [32..30]*/
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/
    bits_write(&bitsBuffer, 15, (s64Scr >> 15) & 0x7FFF); /*System clock [29..15]*/
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/
    bits_write(&bitsBuffer, 15, s64Scr & 0x7fff);       /*System clock [29..15]*/
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/
    bits_write(&bitsBuffer, 9,  lScrExt & 0x01ff);      /*System clock [14..0]*/
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/
    bits_write(&bitsBuffer, 22, (255) & 0x3fffff);      /*bit rate(n units of 50 bytes per second.)*/
    bits_write(&bitsBuffer, 2,  3);                     /*marker bits '11'*/
    bits_write(&bitsBuffer, 5,  0x1f);                  /*reserved(reserved for future use)*/
    bits_write(&bitsBuffer, 3,  0);                     /*stuffing length*/
#endif
    return 0;
}

#endif


/***
 *@remark:   sys头的封装,里面的具体数据的填写已经占位，可以参考标准
 *@param :   pData  [in] 填充ps头数据的地址
 *@return:   0 success, others failed
*/
int ite_make_sys_header(char *pData)
{

    bits_buffer_s   bitsBuffer;
    bitsBuffer.i_size = SYS_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (unsigned char *)(pData);
    memset(bitsBuffer.p_data, 0, SYS_HDR_LEN);

    bitsBuffer.p_data[0] = 0x00;
    bitsBuffer.p_data[1] = 0x00;
    bitsBuffer.p_data[2] = 0x01;
    bitsBuffer.p_data[3] = 0xBB;

    bitsBuffer.p_data[4] = 0x00;
    bitsBuffer.p_data[5] = 0x0C;

    bitsBuffer.p_data[6] = 0x80;
    bitsBuffer.p_data[7] = 0x1E;

    bitsBuffer.p_data[8] = 0xFF;
    bitsBuffer.p_data[9] = 0x04;

    bitsBuffer.p_data[10] = 0xE1;
    bitsBuffer.p_data[11] = 0x7F;

    bitsBuffer.p_data[12] = 0xE0;
    bitsBuffer.p_data[13] = 0xE0;

    //pDestBuf[14] = 0xE8;
    bitsBuffer.p_data[14] = 0xE8;
    bitsBuffer.p_data[15] = 0xC0;

    bitsBuffer.p_data[16] = 0xC0;
    bitsBuffer.p_data[17] = 0x20;
    return 0;
}


/***
 *@remark:   psm头的封装,里面的具体数据的填写已经占位，可以参考标准
 *@param :   pData  [in] 填充ps头数据的地址
 *@return:   0 success, others failed

*/

uint32_t            s_crc32_table[256] =
{
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
    0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
    0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
    0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
    0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
    0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
    0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
    0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
    0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
    0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
    0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
    0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
    0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
    0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
    0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
    0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
    0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
    0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};//*/
int ite_make_psm_header(char *pData, int streamType)
{

    unsigned char *p_byte;
    unsigned int CRC_32 = -1;
    int i = 0;
    bits_buffer_s   bitsBuffer;
    bitsBuffer.i_size = PSM_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (unsigned char *)(pData);
    memset(bitsBuffer.p_data, 0, PSM_HDR_LEN);


    bitsBuffer.p_data[0] = 0x00;
    bitsBuffer.p_data[1] = 0x00;
    bitsBuffer.p_data[2] = 0x01;
    bitsBuffer.p_data[3] = 0xBC;

    bitsBuffer.p_data[4] = 0x00;
    bitsBuffer.p_data[5] = 0x12;//0x0e;//0x18/*增加音频类型及ID，长度增加4字节*/

    bitsBuffer.p_data[6] = 0xE1;
    bitsBuffer.p_data[7] = 0xFF;

    bitsBuffer.p_data[8] = 0x00;
    bitsBuffer.p_data[9] = 0x00;

    bitsBuffer.p_data[10] = 0x00;
    bitsBuffer.p_data[11] = 0x08;//0x04;//0x08/*增加音频类型及ID，长度增加4字节*/

    //element_info
    bitsBuffer.p_data[12] = streamType; //H.264 0x1b H.265 0x24

    bitsBuffer.p_data[13] = 0xE0;

    bitsBuffer.p_data[14] = 0x00;
    bitsBuffer.p_data[15] = 0x00;

    bitsBuffer.p_data[16] = 0x90; //G.711/*GB28181检测PSM信息中,检测音频类型,所以增加此项*/
    bitsBuffer.p_data[17] = 0xC0;
    bitsBuffer.p_data[18] = 0x00;
    bitsBuffer.p_data[19] = 0x00;

    //avc timming and hrd descriptor
    /*bitsBuffer.p_data[16] = 0x0a;
       bitsBuffer.p_data[17] = 0x04;
       bitsBuffer.p_data[18] = 0x65;
       bitsBuffer.p_data[19] = 0x6e;
       bitsBuffer.p_data[20] = 0x67;
       bitsBuffer.p_data[21] = 0x00;

       bitsBuffer.p_data[22] = 0x90;//G711
       bitsBuffer.p_data[23] = 0xc0;
       bitsBuffer.p_data[24] = 0x00;
       bitsBuffer.p_data[25] = 0x00;

       bitsBuffer.p_data[26] = 0x0;
       bitsBuffer.p_data[27] = 0x0;
       bitsBuffer.p_data[28] = 0x0;
       bitsBuffer.p_data[29] = 0x0;*/

    #if 1
    p_byte  = &(bitsBuffer.p_data[6]);
    for(i = 0; i < 14; i++)
    {
        CRC_32 = (CRC_32 << 8)^s_crc32_table[(CRC_32 >> 24) ^ (*p_byte)];
        p_byte++;
    }

    /*bitsBuffer.p_data[16] = CRC_32 >> 24;
    bitsBuffer.p_data[17] = CRC_32 >> 16;
    bitsBuffer.p_data[18] = CRC_32 >>  8;
    bitsBuffer.p_data[19] = CRC_32 ;*/
    bitsBuffer.p_data[20] = CRC_32 >> 24;
    bitsBuffer.p_data[21] = CRC_32 >> 16;
    bitsBuffer.p_data[22] = CRC_32 >>  8;
    bitsBuffer.p_data[23] = CRC_32 ;
    #else
    bitsBuffer.p_data[20] = 0;
    bitsBuffer.p_data[21] = 0;
    bitsBuffer.p_data[22] = 0;
    bitsBuffer.p_data[23] = 0;
    #endif



#if 0

    bits_write(&bitsBuffer, 24, 0x000001);  /*start code*/
    bits_write(&bitsBuffer, 8, 0xBC);       /*map stream id*/
    bits_write(&bitsBuffer, 16, 18);        /*program stream map length*/
    bits_write(&bitsBuffer, 1, 1);          /*current next indicator */
    bits_write(&bitsBuffer, 2, 3);          /*reserved*/
    bits_write(&bitsBuffer, 5, 0);          /*program stream map version*/
    bits_write(&bitsBuffer, 7, 0x7F);       /*reserved */
    bits_write(&bitsBuffer, 1, 1);          /*marker bit */
    bits_write(&bitsBuffer, 16, 0);         /*programe stream info length*/
    bits_write(&bitsBuffer, 16, 8);         /*elementary stream map length  is*/
    /*audio*/
    bits_write(&bitsBuffer, 8, 0x90);       /*stream_type*/
    bits_write(&bitsBuffer, 8, 0xC0);       /*elementary_stream_id*/
    bits_write(&bitsBuffer, 16, 0);         /*elementary_stream_info_length is*/
    /*video*/
    bits_write(&bitsBuffer, 8, 0x1B);       /*stream_type*/
    bits_write(&bitsBuffer, 8, 0xE0);       /*elementary_stream_id*/
    bits_write(&bitsBuffer, 16, 0);         /*elementary_stream_info_length */
    /*crc (2e b9 0f 3d)*/
    bits_write(&bitsBuffer, 8, 0x45);       /*crc (24~31) bits*/
    bits_write(&bitsBuffer, 8, 0xBD);       /*crc (16~23) bits*/
    bits_write(&bitsBuffer, 8, 0xDC);       /*crc (8~15) bits*/
    bits_write(&bitsBuffer, 8, 0xF4);       /*crc (0~7) bits*/
#endif
    return 0;
}


/***
 *@remark:   pes头的封装,里面的具体数据的填写已经占位，可以参考标准
 *@param :   pData      [in] 填充ps头数据的地址
 *           stream_id  [in] 码流类型
 *           paylaod_len[in] 负载长度
 *           pts        [in] 时间戳
 *           dts        [in]
 *@return:   0 success, others failed
*/
int ite_make_pes_header(char *pData, int stream_id, int payload_len,  unsigned int videopts, int bFirst)
{

    bits_buffer_s   bitsBuffer;
    bitsBuffer.i_size = PES_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (unsigned char *)(pData);
    memset(bitsBuffer.p_data, 0, PES_HDR_LEN);
#if 1

    unsigned short  nTempLen = 8 + payload_len;

    bitsBuffer.p_data[0] = 0x00;
    bitsBuffer.p_data[1] = 0x00;
    bitsBuffer.p_data[2] = 0x01;
    bitsBuffer.p_data[3] = stream_id & 0xFF;

    //PES包的长度
    bitsBuffer.p_data[4] = (nTempLen >> 8) & 0xFF;
    bitsBuffer.p_data[5] = nTempLen & 0xFF;

    bitsBuffer.p_data[6] = 0x80;
    bitsBuffer.p_data[7] = 0x80;

    //PTS_DTS_flag = '10';
    bitsBuffer.p_data[8] = 0x05;

    {
        bitsBuffer.p_data[9] = ((0x0E & ((videopts >> 30) << 1)) | 0x21);
        bitsBuffer.p_data[10] = ((videopts >> 22) & (0xFF));
        bitsBuffer.p_data[11] = ((((videopts >> 15) << 1) & (0xFE)) | 0x01);
        bitsBuffer.p_data[12] = ((videopts >> 7) & (0xFF));
        bitsBuffer.p_data[13] = (((videopts << 1) & 0xFE) | 0x01);
    }



#else
    unsigned int pts, dts;
    pts = dts = videopts;

    /*system header*/
    bits_write( &bitsBuffer, 24, 0x000001); /*start code*/
    bits_write( &bitsBuffer, 8, (stream_id));   /*streamID*/
    bits_write( &bitsBuffer, 16, (payload_len) + 13);  /*packet_len*/ //指出pes分组中数据长度和该字节后的长度和
    bits_write( &bitsBuffer, 2, 2 );        /*'10'*/
    bits_write( &bitsBuffer, 2, 0 );        /*scrambling_control*/
    bits_write( &bitsBuffer, 1, 0 );        /*priority*/
    bits_write( &bitsBuffer, 1, 0 );        /*data_alignment_indicator*/
    bits_write( &bitsBuffer, 1, 0 );        /*copyright*/
    bits_write( &bitsBuffer, 1, 0 );        /*original_or_copy*/
    bits_write( &bitsBuffer, 1, 1 );        /*PTS_flag*/
    bits_write( &bitsBuffer, 1, 1 );        /*DTS_flag*/
    bits_write( &bitsBuffer, 1, 0 );        /*ESCR_flag*/
    bits_write( &bitsBuffer, 1, 0 );        /*ES_rate_flag*/
    bits_write( &bitsBuffer, 1, 0 );        /*DSM_trick_mode_flag*/
    bits_write( &bitsBuffer, 1, 0 );        /*additional_copy_info_flag*/
    bits_write( &bitsBuffer, 1, 0 );        /*PES_CRC_flag*/
    bits_write( &bitsBuffer, 1, 0 );        /*PES_extension_flag*/
    bits_write( &bitsBuffer, 8, 10);        /*header_data_length*/
    // 指出包含在 PES 分组标题中的可选字段和任何填充字节所占用的总字节数。该字段之前
    //的字节指出了有无可选字段。

    /*PTS,DTS*/
    bits_write( &bitsBuffer, 4, 3 );                    /*'0011'*/
    bits_write( &bitsBuffer, 3, ((pts) >> 30) & 0x07 ); /*PTS[32..30]*/
    bits_write( &bitsBuffer, 1, 1 );
    bits_write( &bitsBuffer, 15, ((pts) >> 15) & 0x7FFF); /*PTS[29..15]*/
    bits_write( &bitsBuffer, 1, 1 );
    bits_write( &bitsBuffer, 15, (pts) & 0x7FFF);       /*PTS[14..0]*/
    bits_write( &bitsBuffer, 1, 1 );
    bits_write( &bitsBuffer, 4, 1 );                    /*'0001'*/
    bits_write( &bitsBuffer, 3, ((dts) >> 30) & 0x07 ); /*DTS[32..30]*/
    bits_write( &bitsBuffer, 1, 1 );
    bits_write( &bitsBuffer, 15, ((dts) >> 15) & 0x7FFF); /*DTS[29..15]*/
    bits_write( &bitsBuffer, 1, 1 );
    bits_write( &bitsBuffer, 15, (dts) & 0x7FFF);       /*DTS[14..0]*/
    bits_write( &bitsBuffer, 1, 1 );
#endif
    return 0;
}


int ite_make_rtp_header(char *pData, int marker_flag,  unsigned int curpts, unsigned int ssrc, unsigned short rtpSeq)
{
    bits_buffer_s   bitsBuffer;
    unsigned int fssrc = ssrc;
    if (pData == NULL)
        return -1;
    bitsBuffer.i_size = RTP_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (unsigned char *)(pData);
    memset(bitsBuffer.p_data, 0, RTP_HDR_LEN);


#ifdef RTP_EXTERN
    bitsBuffer.p_data[0] = 0x90;//0x80 没有扩展  0x90:带有扩展
#else
    bitsBuffer.p_data[0] = 0x80;//0x80 没有扩展  0x90:带有扩展
#endif
    bitsBuffer.p_data[1] = 96 | (marker_flag ? 0x80 : 0);
    bitsBuffer.p_data[2] = (rtpSeq >> 8) & 0xFF;
    bitsBuffer.p_data[3] = rtpSeq & 0xFF;

    bitsBuffer.p_data[4] = (curpts >> 24) & 0xFF;
    bitsBuffer.p_data[5] = (curpts >> 16) & 0xFF;
    bitsBuffer.p_data[6] = (curpts >> 8) & 0xFF;
    bitsBuffer.p_data[7] = curpts & 0xFF;



    bitsBuffer.p_data[8] = (fssrc >> 24) & 0xFF;
    bitsBuffer.p_data[9] = (fssrc >> 16) & 0xFF;;
    bitsBuffer.p_data[10] = (fssrc >> 8) & 0xFF;;
    bitsBuffer.p_data[11] = (fssrc) & 0xFF;

#ifdef RTP_EXTERN
    //exten
    bitsBuffer.p_data[12] = 0;
    bitsBuffer.p_data[13] = 4;//4字节扩展时间

    bitsBuffer.p_data[14] = (0 >> 24) & 0xFF;
    bitsBuffer.p_data[15] = (0 >> 16) & 0xFF;;
    bitsBuffer.p_data[16] = (0 >> 8) & 0xFF;;
    bitsBuffer.p_data[17] = (0) & 0xFF;
#endif

    return 0;
}

int ite_make_rtptcp_header(unsigned short *seq,char *pData, int marker_flag,  unsigned int curpts, unsigned int ssrc, unsigned int cur_sec, int datalen,unsigned char padding_flag)
{
    unsigned char *rtpTcpHeader = NULL;
    bits_buffer_s   bitsBuffer;
    unsigned int fssrc = ssrc;
	int paddingLen = 0;

    if (pData == NULL || seq == NULL)
        return -1;

	if(padding_flag)
		paddingLen = RTP_PADDING_LEN;

    /*RTP OVER TCP,依据GB28181前两个字节增加长度*/
    rtpTcpHeader = (unsigned char *)pData;
    memset(rtpTcpHeader, 0, 2);
    *(unsigned short*)(rtpTcpHeader) = htons((unsigned short)datalen+RTP_HDR_LEN+paddingLen);

    bitsBuffer.i_size = RTP_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (unsigned char *)(pData+2);
    memset(bitsBuffer.p_data, 0, RTP_HDR_LEN);

#if 0
#ifdef RTP_EXTERN
	#ifdef RTP_PADDING
    bitsBuffer.p_data[0] = 0xB0;//0xA0 没有扩展  0xB0:带有扩展padding
    #else
    bitsBuffer.p_data[0] = 0x90;
	#endif
#else
	#ifdef RTP_PADDING
    bitsBuffer.p_data[0] = 0xA0;//0xA0 没有扩展  ,有padding
    #else
    bitsBuffer.p_data[0] = 0x80;//0x80 没有扩展
	#endif
#endif
#else

#ifdef RTP_EXTERN

	if(padding_flag)
    	bitsBuffer.p_data[0] = 0xB0;//0xA0 没有扩展  0xB0:带有扩展padding
    else
    	bitsBuffer.p_data[0] = 0x90;

#else

	if(padding_flag)
    	bitsBuffer.p_data[0] = 0xA0;//0xA0 没有扩展  ,有padding
    else
    	bitsBuffer.p_data[0] = 0x80;//0x80 没有扩展

#endif


#endif
    bitsBuffer.p_data[1] = 96 | (marker_flag ? 0x80 : 0);
    //bitsBuffer.p_data[2] = (g_RtpSeqCnt & 0xff00) >> 8;
   // bitsBuffer.p_data[3] = (g_RtpSeqCnt & 0xff);

    bitsBuffer.p_data[2] = (*seq >> 8) & 0xff;
    bitsBuffer.p_data[3] = (*seq & 0xff);

    bitsBuffer.p_data[4] = (curpts >> 24) & 0xFF;
    bitsBuffer.p_data[5] = (curpts >> 16) & 0xFF;
    bitsBuffer.p_data[6] = (curpts >> 8) & 0xFF;
    bitsBuffer.p_data[7] = curpts & 0xFF;



    bitsBuffer.p_data[8] = (fssrc >> 24) & 0xFF;
    bitsBuffer.p_data[9] = (fssrc >> 16) & 0xFF;;
    bitsBuffer.p_data[10] = (fssrc >> 8) & 0xFF;;
    bitsBuffer.p_data[11] = (fssrc) & 0xFF;

    #ifdef RTP_EXTERN
    //exten
    bitsBuffer.p_data[12] = 0;
    bitsBuffer.p_data[13] = 4;//4字节扩展时间

    bitsBuffer.p_data[14] = (cur_sec >> 24) & 0xFF;
    bitsBuffer.p_data[15] = (cur_sec >> 16) & 0xFF;;
    bitsBuffer.p_data[16] = (cur_sec >> 8) & 0xFF;;
    bitsBuffer.p_data[17] = (cur_sec) & 0xFF;
    #endif

	//printf("seq:%d,ssrc:%d,pts:%u,ts:%u\n",*seq,fssrc,curpts,cur_sec);

    RtpSeqAdd(seq);

    return 0;
}


/***
 *@remark:   rtp头的打包，并循环发送数据
 *@param :   pData      [in] 发送的数据地址
 *           nDatalen   [in] 发送数据的长度
 *           mark_flag  [in] mark标志位
 *           curpts     [in] 时间戳
 *           pPacker    [in] 数据包的基本信息
 *@return:   0 success, others failed
*/

int rtp_send_h264ps(int rtpsockfd, struct sockaddr *to, char *databuff, int nDataLen, int mark_end, frameData_Info_s* pPacker)
{
    int nRes = 0;
    int nPlayLoadLen = 0;
    int nSendSize    = 0;
    char szRtpHdr[RTP_HDR_LEN];
    //unsigned char fu_indicator = 0, fu_header = 0;
    while(nDataLen > 0)
    {
        nPlayLoadLen = (nDataLen > RTP_MAX_PACKET_BUFF ) ? RTP_MAX_PACKET_BUFF : nDataLen; // 每次只能发送的数据长度 除去rtp头
        memset(szRtpHdr, 0, RTP_HDR_LEN);
        if(nDataLen <= nPlayLoadLen)
        {
            //一帧数据发送完，置mark标志位
            if(mark_end == 1)
            {
                ite_make_rtp_header(szRtpHdr, (1),  (pPacker->u32RtpPts), pPacker->u32Ssrc, pPacker->u16CSeq++);
            }
            else
            {
                ite_make_rtp_header(szRtpHdr, (0),  (pPacker->u32RtpPts), pPacker->u32Ssrc, pPacker->u16CSeq++);
            }
            nSendSize = nDataLen;
        }
        else
        {
            ite_make_rtp_header(szRtpHdr, (0), (pPacker->u32RtpPts), pPacker->u32Ssrc, pPacker->u16CSeq++);
            nSendSize = nPlayLoadLen;
        }
        memcpy(pPacker->szBuff, szRtpHdr, RTP_HDR_LEN);
        memcpy(pPacker->szBuff + RTP_HDR_LEN, databuff, nSendSize);
        nRes = sendto(rtpsockfd, pPacker->szBuff, RTP_HDR_LEN + nSendSize, 0, to, sizeof(struct sockaddr));

        if (nRes != (RTP_HDR_LEN + nSendSize))
        {
            printf(" udp send error !\n");
            return -1;
        }
        nDataLen -= nSendSize;
        databuff += nSendSize;
        //因为buffer指针已经向后移动一次rtp头长度后，
        //所以每次循环发送rtp包时，只要向前移动裸数据到长度即可，这是buffer指针实际指向到位置是
        //databuff向后重复的rtp长度的裸数据到位置上

    }

    return 0;
}


int ITE_SHARE_BUFFER_Init_Mgr(int *stFrameHdl)
{
    return 0;
}



int saveps(unsigned char *addr_ps, int size) //淇濆瓨raw 鏁版嵁
{
    char rawfilename[64];
    static FILE *rpsFp = NULL;
    static int saveNO = 1, framecnt = 0;
    if(rpsFp == NULL)
    {
        memset(rawfilename, 0x0, 64);
        sprintf(rawfilename, "/tmp/%d_ps.h264", saveNO);

        rpsFp = fopen(rawfilename, "wb");
        printf("save ps-h264 file :%s size %d \n", rawfilename, size);
        saveNO++;

    }
    if (rpsFp == NULL)
    {
        return -1;
    }
    fwrite(addr_ps, 1, size, rpsFp);
    if(framecnt++ >= 64)
    {
        fclose(rpsFp);
        rpsFp = NULL;
        framecnt = 0;
    }
    return 0;
}

#ifdef TS_TEST_SERVER_ALL
int getTestFdBySsrc(unsigned int ssrc)
{
    int i;
    for (i = 0; i < MAX_CHANNEL_NUM; i++)
    {
        if (rtpInfo[i].isUsed && rtpInfo[i].ssrc == ssrc)
        {
            return rtpInfo[i].selfCloudTestFd;
        }
    }
    return -1;
}

int closeTestFdBySsrc(unsigned int ssrc)
{
    int i;
    for (i = 0; i < MAX_CHANNEL_NUM; i++)
    {
        if (rtpInfo[i].isUsed && rtpInfo[i].ssrc == ssrc)
        {
            close(rtpInfo[i].selfCloudTestFd);
            rtpInfo[i].selfCloudTestFd = -1;
        }
    }
    return -1;
}
#endif



/***
 *@remark:  音视频数据的打包成ps流，并封装成rtp
 *@param :  pData      [in] 需要发送的音视频数据
 *          nFrameLen  [in] 发送数据的长度
 *          pPacker    [in] 数据包的一些信息，包括时间戳，rtp数据buff，发送的socket相关信息
 *          stream_type[in] 数据类型 0 视频 1 音频
 *@return:  0 success others failed
*/

int ite_RtpSendUdp_PS_H264frame(ite_rtp_Info *pSockInfo, char *pData, int nFrameLen, frameData_Info_s* pPacker, int stream_id, int rtpsockfd, struct sockaddr *to)
{
    int  nSizePos = 0;
    int  nSize = 0, firstpack = 1, endpack = 0;
    struct sockaddr_in veAddr;
    //char szTempPacketHead[PS_PYLOAD_MAX_SIZE];

    /* 128KB is enough
    if (pSockInfo->frameBufferSize - 256 < nFrameLen)
    {
        pSockInfo->frameBufferSize = nFrameLen + 256;
        if (pSockInfo->frameBuffer != NULL)
        {
            free(pSockInfo->frameBuffer);
        }
        pSockInfo->frameBuffer = (char*)malloc(pSockInfo->frameBufferSize);
    }
    */

    if (pSockInfo->frameBuffer == NULL)
    {
        return -1;
    }

    //char *pBuff = NULL;
    //memset(szTempPacketHead, 0, PS_PYLOAD_MAX_SIZE);
    // 1 package for ps header
    ite_make_ps_header(pSockInfo->frameBuffer + nSizePos, pPacker->u32CurPts);
    nSizePos += PS_HDR_LEN;
    // 2 system header
    if( pPacker->IFrame == 1 )
    {
        // 如果是I帧的话，则添加系统头
        ite_make_sys_header(pSockInfo->frameBuffer + nSizePos);
        nSizePos += SYS_HDR_LEN;
        //这个地方我是不管是I帧还是p帧都加上了map的，貌似只是I帧加也没有问题
        ite_make_psm_header(pSockInfo->frameBuffer + nSizePos, pPacker->streamType);
        nSizePos += PSM_HDR_LEN;

    }

    // 这里向后移动是为了方便拷贝pes头
    //这里是为了减少后面音视频裸数据的大量拷贝浪费空间，所以这里就向后移动，在实际处理的时候，要注意地址是否越界以及覆盖等问题
    veAddr.sin_family = AF_INET;
    veAddr.sin_addr.s_addr = inet_addr(pSockInfo->remoteip);
    veAddr.sin_port = htons(pSockInfo->videoPort);

    while(nFrameLen > 0)
    {
        //每次帧的长度不要超过short类型，过了就得分片进循环行发送
        nSize = (nFrameLen > PS_PES_PAYLOAD_PACKSIZE) ? PS_PES_PAYLOAD_PACKSIZE : nFrameLen;

        // 添加pes头
        if(nSize == nFrameLen)
        {
            endpack = 1;
        }

        ite_make_pes_header(pSockInfo->frameBuffer + nSizePos,  stream_id, nSize, (pPacker->u32CurPts), firstpack);
        nSizePos += PES_HDR_LEN;

        memcpy(pSockInfo->frameBuffer + nSizePos, pData, nSize);
        //最后在添加rtp头并发送数据
        //  saveps(szTempPacketHead, nSize + nSizePos);
        if( rtp_send_h264ps(rtpsockfd, (struct sockaddr *)&veAddr, pSockInfo->frameBuffer, nSize + nSizePos, endpack, pPacker) != 0 )
        {
            printf("rtp_send_h264ps  failed!\n");
            return -1;
        }

        //分片后每次发送的数据移动指针操作
        nFrameLen -= nSize;
        //这里也只移动nSize,因为在while向后移动的pes头长度，正好重新填充pes头数据
        pData     += nSize;
        nSizePos = 0;
        firstpack = 0;
    }
    return 0;
}

/***
 *@remark:   rtp头的打包，并循环发送数据
 *@param :   pData      [in] 发送的数据地址
 *           nDatalen   [in] 发送数据的长度
 *           mark_flag  [in] mark标志位
 *           curpts     [in] 时间戳
 *           pPacker    [in] 数据包的基本信息
 *@return:   0 success, others failed
*/

int rtp_sendtcp_h264ps(int rtpsockfd, struct sockaddr *to, char *databuff, int nDataLen, int mark_end, frameData_Info_s* pPacker, int rtptype)
{
    int nRes = 0;
    int nPlayLoadLen = 0;
    int nSendSize    = 0;
	int paddingLen   = 0;
    int allDataLen   = 0;
    int haveSendLen  = 0;
	int retry = 0;
    char szRtpHdr[RTP_TCP_HDR_LEN];
    //unsigned char fu_indicator = 0, fu_header = 0;
    while(nDataLen > 0)
    {
        nPlayLoadLen = (nDataLen > RTP_MAX_PACKET_BUFF ) ? RTP_MAX_PACKET_BUFF : nDataLen; // 每次只能发送的数据长度 除去rtp头
        memset(szRtpHdr, 0, RTP_TCP_HDR_LEN);
        if(nDataLen <= nPlayLoadLen)
        {
            //一帧数据发送完，置mark标志位
            if(mark_end == 1)
            {
                ite_make_rtptcp_header(&pPacker->u16CSeq,szRtpHdr, (1),  (pPacker->u32RtpPts), pPacker->u32Ssrc, pPacker->u32Sec, nDataLen, pPacker->paddingFlg);
            }
            else
            {
                ite_make_rtptcp_header(&pPacker->u16CSeq,szRtpHdr, (0),  (pPacker->u32RtpPts), pPacker->u32Ssrc, pPacker->u32Sec, nDataLen, pPacker->paddingFlg);
            }
            nSendSize = nDataLen;
        }
        else
        {
            ite_make_rtptcp_header(&pPacker->u16CSeq,szRtpHdr, (0), (pPacker->u32RtpPts), pPacker->u32Ssrc, pPacker->u32Sec, nPlayLoadLen, pPacker->paddingFlg);
            nSendSize = nPlayLoadLen;
        }

        memcpy(pPacker->szBuff, szRtpHdr, RTP_TCP_HDR_LEN);
        memcpy(pPacker->szBuff + RTP_TCP_HDR_LEN, databuff, nSendSize);

#if 0
		#ifdef RTP_PADDING
		pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize] = (pPacker->u32Sec >> 24) & 0xFF;
		pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+1] = (pPacker->u32Sec >> 16) & 0xFF;;
		pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+2] = (pPacker->u32Sec >> 8) & 0xFF;;
		pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+3] = (pPacker->u32Sec) & 0xFF;
		pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+4] = RTP_PADDING_LEN;//padding len
		#endif
#else

		if(pPacker->paddingFlg)
		{
			paddingLen = RTP_PADDING_LEN;

			pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize] = (pPacker->u32Sec >> 24) & 0xFF;
			pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+1] = (pPacker->u32Sec >> 16) & 0xFF;
			pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+2] = (pPacker->u32Sec >> 8) & 0xFF;
			pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+3] = (pPacker->u32Sec) & 0xFF;
			pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+4] = paddingLen;
		}

#endif
		#if 0
		printf("rtp:\n");

		int i = 0;

		for(i=0; i<RTP_TCP_HDR_LEN; i++)
			printf("%x ",pPacker->szBuff[i]);

		if(pPacker->paddingFlg)
		{
			printf("%x ",pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize]);
			printf("%x ",pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+1]);
			printf("%x ",pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+2]);
			printf("%x ",pPacker->szBuff[RTP_TCP_HDR_LEN+nSendSize+3]);
		}

		printf("\n");
		#endif

        haveSendLen = 0;
        allDataLen = RTP_TCP_HDR_LEN + nSendSize + paddingLen;
		retry = 0;
        while(haveSendLen < allDataLen)
        {
            nRes = send(rtpsockfd, &pPacker->szBuff[0] + haveSendLen, allDataLen - haveSendLen, 0);
            haveSendLen += nRes;
            if (haveSendLen != allDataLen)
            {
                printf(" tcp send error ! nRes:%d, all:%d/%d\n", nRes, haveSendLen, allDataLen);
                printf("send err %d: %s\r\n", errno, strerror(errno));

                if (nRes == -1)
                {
					if(rtptype == 1 || rtptype == 2)// no retry for file download or playback
						retry = 4;

                    if (errno != 11 || retry > 3)
                    {
						memset(&errinfo[rtptype],0,sizeof(errinfo[rtptype]));
						strncpy(&errinfo[rtptype],strerror(errno),sizeof(errinfo[rtptype]));

                        return -1;
                    }
                    else
                    {
						retry += 1;
                        haveSendLen += 1;
                    }
                }
            }
        }


        #ifdef TS_TEST_SERVER_ALL
        if (getTestFdBySsrc(pPacker->u32Ssrc) != -1)
        {
            haveSendLen = 0;
            //printf("try to send:%d\n", allDataLen);
            while(haveSendLen < allDataLen)
            {
                nRes = send(getTestFdBySsrc(pPacker->u32Ssrc), &pPacker->szBuff[0] + haveSendLen, allDataLen - haveSendLen, 0);
                haveSendLen += nRes;
                if (haveSendLen != allDataLen)
                {
                    //printf("testfd tcp send error ! nRes:%d, all:%d/%d\n", nRes, haveSendLen, allDataLen);
                    //printf("testfd send err %d: %s\r\n", errno, strerror(errno));
                    if (nRes == -1)
                    {
                        closeTestFdBySsrc(pPacker->u32Ssrc);
                        break;
                    }
                }
            }
        }

        #endif

        //printf("send nRes:%d, RTP_HDR_LEN + nSendSize:%d\n", nRes, RTP_HDR_LEN + nSendSize);

        nDataLen -= nSendSize;
        databuff += nSendSize;
        //因为buffer指针已经向后移动一次rtp头长度后，
        //所以每次循环发送rtp包时，只要向前移动裸数据到长度即可，这是buffer指针实际指向到位置是
        //databuff向后重复的rtp长度的裸数据到位置上

    }

    return 0;
}

/***
 *@remark:  音视频数据的打包成ps流，并封装成rtp
 *@param :  pData      [in] 需要发送的音视频数据
 *          nFrameLen  [in] 发送数据的长度
 *          pPacker    [in] 数据包的一些信息，包括时间戳，rtp数据buff，发送的socket相关信息
 *          stream_type[in] 数据类型 0 视频 1 音频
 *@return:  0 success others failed
*/
int ite_RtpSendTcp_PS_H264frame(ite_rtp_Info *pSockInfo, char *pData, int nFrameLen, frameData_Info_s* pPacker, int stream_id, int rtpsockfd, struct sockaddr *to)
{
    int  nSizePos = 0;
    int  nSize = 0, firstpack = 1, endpack = 0;
    //char szTempPacketHead[PS_PYLOAD_MAX_SIZE];

    /*
    //128KB is enough
    if (pSockInfo->frameBufferSize - 256 < nFrameLen)
    {
        pSockInfo->frameBufferSize = nFrameLen + 256;
        if (pSockInfo->frameBuffer != NULL)
        {
            free(pSockInfo->frameBuffer);
        }
        pSockInfo->frameBuffer = (char*)malloc(pSockInfo->frameBufferSize);
    }
    */

    if (pSockInfo->frameBuffer == NULL)
    {
        return -1;
    }

    //char *pBuff = NULL;
    //memset(szTempPacketHead, 0, PS_PYLOAD_MAX_SIZE);
    // 1 package for ps header
    ite_make_ps_header(pSockInfo->frameBuffer + nSizePos, pPacker->u32CurPts);
    nSizePos += PS_HDR_LEN;
    // 2 system header
    if( pPacker->IFrame == 1 )
    {
        // 如果是I帧的话，则添加系统头
        ite_make_sys_header(pSockInfo->frameBuffer + nSizePos);
        nSizePos += SYS_HDR_LEN;
        //这个地方我是不管是I帧还是p帧都加上了map的，貌似只是I帧加也没有问题
        ite_make_psm_header(pSockInfo->frameBuffer + nSizePos, pPacker->streamType);
        nSizePos += PSM_HDR_LEN;

    }

    // 这里向后移动是为了方便拷贝pes头
    //这里是为了减少后面音视频裸数据的大量拷贝浪费空间，所以这里就向后移动，在实际处理的时候，要注意地址是否越界以及覆盖等问题

    while(nFrameLen > 0)
    {
        //每次帧的长度不要超过short类型，过了就得分片进循环行发送
        nSize = (nFrameLen > PS_PES_PAYLOAD_PACKSIZE) ? PS_PES_PAYLOAD_PACKSIZE : nFrameLen;

        // 添加pes头
        if(nSize == nFrameLen)
        {
            endpack = 1;
        }

        ite_make_pes_header(pSockInfo->frameBuffer + nSizePos,  stream_id, nSize, (pPacker->u32CurPts), firstpack);
        nSizePos += PES_HDR_LEN;

        memcpy(pSockInfo->frameBuffer + nSizePos, pData, nSize);
        //最后在添加rtp头并发送数据
        //saveps(frameBuffer, nSize + nSizePos);
        if( rtp_sendtcp_h264ps(rtpsockfd, to, pSockInfo->frameBuffer, nSize + nSizePos, endpack, pPacker, pSockInfo->rtpType) != 0 )
        {
            printf("rtp_sendtcp_h264ps  failed 1!\n");
            return -1;
        }
        //分片后每次发送的数据移动指针操作
        nFrameLen -= nSize;
        //这里也只移动nSize,因为在while向后移动的pes头长度，正好重新填充pes头数据
        pData     += nSize;
        nSizePos = 0;
        firstpack = 0;
    }
    return 0;
}

static void RtpSeqAdd(unsigned short *seq)
{
    if (*seq == 0xffff)
    {
        *seq = 0;
    }
    (*seq)++;
    return;
}

int RtpSendOverUdpPkt3984(int rtpFd, char *dataBuf, int dataLen, struct sockaddr *to, RTPINFO *rtpInfo)// PlayLoad H264
{
    char *pPayload = dataBuf;
    int left_bytes = dataLen;
    int payload_len = 0;
    int rtplen = 0;
    unsigned char fu_indicator = 0, fu_header = 0;
    unsigned char nal_header, nal_header_type;
    int count = (dataLen + M_RTP_PAYLOAD_LEN - 1) / (M_RTP_PAYLOAD_LEN);
    int i, mark;
    int retVal = 0;
    rtp_hdr   RTPHEADER;
    char        RtpSendBuf[2048];
    int         rtpheadlen = 0;
    int         csrclen    = 0;

    nal_header = pPayload[0];
    nal_header_type = nal_header & 0x1f;
    fu_indicator = (nal_header & 0xe0) | 0x1c;
    printf("sendto RTP NAL HEAD [%02x  %02x]\n", pPayload[0], pPayload[1]);

    for (i = 0; i < count; i++)
    {
        memset(&RTPHEADER, 0x0, sizeof(rtp_hdr));
        memset(RtpSendBuf, 0,  sizeof(2048));
        rtpheadlen = 0;
        if (i == (count - 1))
        {
            mark = 1;
            payload_len   = left_bytes;
            left_bytes    = 0;
        }
        else
        {
            mark = 0;
            payload_len = M_RTP_PAYLOAD_LEN;
            left_bytes -= M_RTP_PAYLOAD_LEN;
        }
        rtpheadlen = 12 + csrclen;

        RTPHEADER.flags = (M_RTP_VER) << 6;
        RTPHEADER.mk_pt = ((mark & 0x1) << 7) | (rtpInfo->pt & 0x7F);
        RTPHEADER.sq_nb = htons(rtpInfo->seq);
        RTPHEADER.ts       = htonl(rtpInfo->ts);
        RTPHEADER.ssrc   = 0;

        memcpy(RtpSendBuf, &RTPHEADER, 12);
        RtpSeqAdd(&rtpInfo->seq);
        if (count > 1)
        {
            fu_header = 0;
            if(0 == i)
            {
                fu_header |= (1 << 7);
                memcpy((RtpSendBuf + rtpheadlen + 2), (pPayload + 1), payload_len - 1);
                pPayload += payload_len;
                rtplen = payload_len + 1;
            }
            else if(i == (count - 1))
            {
                fu_header |= (1 << 6);
                memcpy((RtpSendBuf + rtpheadlen + 2), pPayload, payload_len);
                pPayload += payload_len;
                rtplen = payload_len + 2;
            }
            else
            {
                memcpy((RtpSendBuf + rtpheadlen + 2), pPayload, payload_len);
                pPayload += payload_len;
                rtplen = payload_len  + 2;
            }
            fu_header |= nal_header_type;
            RtpSendBuf[rtpheadlen] = fu_indicator;
            RtpSendBuf[rtpheadlen + 1] = fu_header;
            rtpheadlen += rtplen;
        }
        else
        {
            memcpy(RtpSendBuf + rtpheadlen, pPayload, payload_len);
            rtpheadlen += payload_len;
        }
        retVal = sendto(rtpFd, RtpSendBuf, rtpheadlen, 0, to, sizeof(struct sockaddr));

        if (retVal < 0)
        {
            printf("sendto error\n");
            return retVal;
        }
    }
    return retVal;
}

int Pframe_process(char *pin, int inlen, P_FRAME_INFO *PFrameInfo)
{
    int i, sei = 0;

    for(i = 0; i < inlen; i++)
    {
        if(pin[i] == 0)
        {
            if((pin[i + 1] == 0) && (pin[i + 2] == 0) && (pin[i + 3] == 1))
            {
                if ((pin[i + 4] & 0x1f) == 6) //SEI
                {
                    PFrameInfo->sei =  &pin[i];
                    sei = 1;
                }
                else if((pin[i + 4] & 0x1f) == 1) //P frame
                {
                    PFrameInfo->pFrame = &pin[i];
                    if (sei)
                    {
                        PFrameInfo->seiLen = (&pin[i]) - PFrameInfo->sei;
                        PFrameInfo->frameLen = inlen - PFrameInfo->seiLen;
                    }
                    else
                    {
                        PFrameInfo->frameLen = inlen;
                    }
                    break;
                }
                else
                {
                    //_PRINTF_DEBUG_LOG_("not known type\n");
                }
            }
        }
    }
    return 0;
}


int Iframe_process(char *pin, int inlen, I_FRAME_INFO *IFrameInfo)
{
    int i, sei = 0;

    for(i = 0; i < inlen; i++)
    {
        if(pin[i] == 0)
        {
            if((pin[i + 1] == 0) && (pin[i + 2] == 0) && (pin[i + 3] == 1))
            {

                printf("Get H264 IDR frame Pin -- %d  \n", (pin[i + 4] & 0x1f));

                if((pin[i + 4] & 0x1f) == 7) //SPS
                {
                    IFrameInfo->sps = &pin[i];
                }
                else if((pin[i + 4] & 0x1f) == 8) //PPS
                {
                    IFrameInfo->pps = &pin[i];
                    IFrameInfo->spsLen = (&pin[i]) - IFrameInfo->sps;
                }
                else if ((pin[i + 4] & 0x1f) == 6) //SEI
                {
                    IFrameInfo->sei =  &pin[i];
                    IFrameInfo->ppsLen = (&pin[i]) - IFrameInfo->pps;
                    sei = 1;
                }
                else if((pin[i + 4] & 0x1f) == 5) //I frame
                {
                    IFrameInfo->pFrame = &pin[i];
                    if (sei)
                    {
                        IFrameInfo->seiLen = (&pin[i]) - IFrameInfo->sei;
                        IFrameInfo->frameLen = inlen - IFrameInfo->spsLen - IFrameInfo->ppsLen - IFrameInfo->seiLen;
                    }
                    else
                    {
                        IFrameInfo->ppsLen = (&pin[i]) - IFrameInfo->pps;
                        IFrameInfo->frameLen = inlen - IFrameInfo->spsLen - IFrameInfo->ppsLen;
                    }

                    break;
                }
                else
                {
                    printf("not known type\n");
                }
            }
        }
    }
    return 0;
}

int GetH264FrameType(char *pFrame)
{
    static  char pVOP_H264[4] = {0, 0, 0, 1};

    if (memcmp(pFrame, pVOP_H264, 4) == 0)
    {
        return (((pFrame[5] & 0x7f) == 0x08 || (pFrame[4] & 0x1f) == 0x07) ? M_FRAME_TYPE_I : M_FRAME_TYPE_P);
    }
    else
    {
        return M_FRAME_TYPE_ERR;
    }
}
int bulid_rtcp_sr(struct rtcp_pkt_sr *pkt)
{
    if(!pkt)
    {
        return -1;
    }
    pkt->comm.version = 2;
    pkt->comm.padding = 0;
    pkt->comm.count = 1;
    pkt->comm.pt = 200;
    pkt->comm.length = htons(pkt->comm.count * 6 + 1);
    return (pkt->comm.count * 6 + 2);
}

int bulid_rtcp_rr(struct rtcp_pkt_rr *pkt)
{
    if(!pkt)
    {
        return -1;
    }
    pkt->comm.version = 2;
    pkt->comm.padding = 0;
    pkt->comm.count = 1;
    pkt->comm.pt = 201;
    pkt->comm.length = htons(pkt->comm.count * 6 + 1);
    return (pkt->comm.count * 6 + 2);
}

int recvRtcpRr(ite_rtp_Info *pSockInfo)
{
    int retVal = 0;
    struct timeval tv;
    fd_set rset;
    char buf[256];

    if (1 == pSockInfo->enOverTcp)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&rset);
        FD_SET(pSockInfo->veRtcpSock, &rset);

        retVal = select(pSockInfo->veRtcpSock + 1, &rset, NULL, NULL, &tv);
        if (retVal > 0)
        {
            retVal = recv(pSockInfo->veRtcpSock, buf, 256, 0);
        }
    }
    else
    {
        if (pSockInfo->veRtcpSock > 0)
        {
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            FD_ZERO(&rset);
            FD_SET(pSockInfo->veRtcpSock, &rset);

            retVal = select(pSockInfo->veRtcpSock + 1, &rset, NULL, NULL, &tv);
            if (retVal)
            {
                retVal = recvfrom(pSockInfo->veRtcpSock, buf, 256, 0, NULL, NULL);
            }
        }
        if (pSockInfo->auRtcpSock > 0)
        {
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            FD_ZERO(&rset);
            FD_SET(pSockInfo->auRtcpSock, &rset);

            retVal = select(pSockInfo->auRtcpSock + 1, &rset, NULL, NULL, &tv);
            if (retVal)
            {
                retVal = recvfrom(pSockInfo->auRtcpSock, buf, 256, 0, NULL, NULL);
            }
        }
    }
    return retVal;
}

int sendRtcpSr(ite_rtp_Info *pSockInfo)
{
    struct sockaddr_in to;
    struct rtcp_pkt_sr rtcpPkt;

    bulid_rtcp_sr(&rtcpPkt);

    if (1 == pSockInfo->enOverTcp)
    {
        send(pSockInfo->veRtcpSock, (char *)&rtcpPkt, sizeof(rtcpPkt), 0);
    }
    else
    {
        if (pSockInfo->veRtcpSock > 0)
        {
            to.sin_family = AF_INET;
            to.sin_addr.s_addr = inet_addr(pSockInfo->remoteip);
            to.sin_port = htons(pSockInfo->videoPort + 1);
            sendto(pSockInfo->veRtcpSock, (char *)&rtcpPkt, sizeof(rtcpPkt), 0, (struct sockaddr *)&to, sizeof(struct sockaddr));
        }
        if (pSockInfo->auRtcpSock > 0)
        {
            to.sin_family = AF_INET;
            to.sin_addr.s_addr = inet_addr(pSockInfo->remoteip);
            to.sin_port = htons(pSockInfo->audioPort + 1);
            sendto(pSockInfo->auRtcpSock, (char *)&rtcpPkt, sizeof(rtcpPkt), 0, (struct sockaddr *)&to, sizeof(struct sockaddr));
        }
    }
    return 0;
}

static int SendVideoOverUdp(ite_rtp_Info *pSockInfo)
{
    int veFrameLen = 0, auFrameLen = 0;
    int veRtpFd = pSockInfo->veRtpSock;
    char *pH264frame = NULL;
    int  rc = -1;
    int isSendIframe = 1;
    struct sockaddr_in veAddr;
    //I_FRAME_INFO IFrameInfo;
    //P_FRAME_INFO PFrameInfo;
    int frameType = 0, isSend = 0;
    int retVal = 0;
    char alaw_data[4096];
    int alaw_len = 0;
    unsigned char stream_id =0xE0 ;//0xE0 video     0xC0 audio
    //unsigned int ts_step = (unsigned int)(90000 / FRAMERATE);
    int  stFrameHdl;
    frameData_Info_s frameInfo_rtp;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;


    memset(&stH264Config, 0, sizeof(stH264Config));
    netcam_video_get(0, pSockInfo->mediaChannel, &stH264Config);

    stFrameHdl = rtp_get_data_init(pSockInfo->mediaChannel);

    pSockInfo->frameBufferSize = 128 * 1024;
    if (pSockInfo->frameBuffer == NULL)
    {
        pSockInfo->frameBuffer = (char*)malloc(pSockInfo->frameBufferSize);
        if (pSockInfo->frameBuffer == NULL)
        {
            printf("malloc %d failed.\n", pSockInfo->frameBufferSize);
            return -1;
        }
    }

    veAddr.sin_family = AF_INET;
    veAddr.sin_addr.s_addr = inet_addr(pSockInfo->remoteip);
    veAddr.sin_port = htons(pSockInfo->videoPort);
    memset(&frameInfo_rtp, 0, sizeof(frameData_Info_s));
    frameInfo_rtp.u16CSeq = 1;
    if (stH264Config.enctype == 1)
    {
        frameInfo_rtp.streamType = 0x1b;
    }
    else if (stH264Config.enctype == 3)
    {
        frameInfo_rtp.streamType = 0x24;
    }
    else
    {
        frameInfo_rtp.streamType = 0x1b;
    }

    
    //FILE *f= fopen("t.gk711"
    while (pSockInfo->isRun)
    {
        pH264frame = NULL;
        veFrameLen = 0;
        rc = rtp_get_data(stFrameHdl, (void **)&pH264frame, &veFrameLen, &frameType, &frameInfo_rtp.u32CurPts);
        //rc = ITE_SHARE_BUFFER_Get_Mgr(&stFrameHdl, &stProcMgrState);
        if (rc != MEDIA_DATA_NON_EMPTY)
        {
            usleep(200);
            continue;
        }


        //pH264frame  = stProcMgrState.ptpayload;
        //veFrameLen = stProcMgrState.npayloadlen;
        /*
        if (isSend)
        {
            frameInfo_rtp.u32CurPts += ts_step ;
        }*/
        frameInfo_rtp.u32Ssrc = pSockInfo->ssrc;
        
        if (frameType == M_FRAME_TYPE_I || frameType == M_FRAME_TYPE_P)
        {
            frameInfo_rtp.u32RtpPts = frameInfo_rtp.u32CurPts;
        }
        else if (frameType == M_FRAME_TYPE_AUDIO)
        {
            //音频rtp的pts在视频的基础上加320
            frameInfo_rtp.u32RtpPts += 320;
            frameInfo_rtp.u32CurPts  = frameInfo_rtp.u32RtpPts;
        }

        //printf("-->get data len:%d, type:%d\n", veFrameLen, frameType);
        if (veFrameLen > 0)
        {
            //frameType = GetH264FrameType(pH264frame);
            if (isSendIframe && (frameType != M_FRAME_TYPE_I))
            {
                continue;
            }
            else
            {
                isSendIframe = 0;
            }

            stream_id = 0xE0;
            //printf("send to %s, %d, type:%d, len:%d, %d, type:%x\n", pSockInfo->remoteip, pSockInfo->videoPort, frameType, veFrameLen, frameInfo_rtp.u32CurPts, frameInfo_rtp.streamType);
            if (frameType == M_FRAME_TYPE_I)
            {
                frameInfo_rtp.IFrame = 1;
                retVal = ite_RtpSendUdp_PS_H264frame(pSockInfo, pH264frame, veFrameLen, &frameInfo_rtp, stream_id, veRtpFd, (struct sockaddr *)&veAddr); // RtpSendOverUdpPkt3984(veRtpFd, p, len, (struct sockaddr *)&veAddr, &veRtpInfo);
                if (retVal < 0)
                {
                    break;
                }
                isSend = 1;
            }
            else if (!isSend)
            {
                continue;
            }
            else if (frameType == M_FRAME_TYPE_P)
            {
                frameInfo_rtp.IFrame = 0;
                retVal = ite_RtpSendUdp_PS_H264frame(pSockInfo, pH264frame, veFrameLen, &frameInfo_rtp, stream_id, veRtpFd, (struct sockaddr *)&veAddr); // RtpSendOverUdpPkt3984(veRtpFd, p, len, (struct sockaddr *)&veAddr, &veRtpInfo);
                if (retVal < 0)
                {
                    break;
                }
            }
            else if (frameType == M_FRAME_TYPE_AUDIO)
            {
                //continue;
                frameInfo_rtp.IFrame = 0;
                stream_id = 0xC0;
                alaw_len = audio_alaw_encode((unsigned char*)alaw_data, (short*)pH264frame, veFrameLen);
                //printf("-->audio len:%d, alaw len:%d\n", veFrameLen, alaw_len);

                retVal = ite_RtpSendUdp_PS_H264frame(pSockInfo, alaw_data, alaw_len, &frameInfo_rtp, stream_id, veRtpFd, (struct sockaddr *)&veAddr); // RtpSendOverUdpPkt3984(veRtpFd, p, len, (struct sockaddr *)&veAddr, &veRtpInfo);
                if (retVal < 0)
                {
                    break;
                }
            }
        }

        if ((veFrameLen <= 0) && (auFrameLen <= 0))
        {
            usleep(20000);
        }

    }

    if(pSockInfo->veRtpSock > -1)
    {
        close(pSockInfo->veRtpSock );
    }
    if(pSockInfo->veRtcpSock > -1)
    {
        close(pSockInfo->veRtcpSock );
    }
    if(pSockInfo->auRtpSock > -1)
    {
        close(pSockInfo->auRtpSock );
    }
    if(pSockInfo->auRtcpSock > -1)
    {
        close(pSockInfo->auRtcpSock );
    }
    pSockInfo->veRtpSock = -1;
    pSockInfo->auRtpSock = -1;
    pSockInfo->veRtcpSock = -1;
    pSockInfo->auRtcpSock = -1;
    rtp_get_data_close(stFrameHdl);

    if (pSockInfo->frameBuffer != NULL)
    {
        free(pSockInfo->frameBuffer);
        pSockInfo->frameBuffer = NULL;
    }
    /**/
    return (retVal < 0 ? retVal : 0);
}

static int RtpSendOverTcpPkt3984(int rtpFd, char *dataBuf, int dataLen, RTPINFO *rtpInfo)// PlayLoad H264
{
    char *pPayload = dataBuf;
    int left_bytes = dataLen;
    int payload_len = 0;
    int rtplen = 0;
    unsigned char fu_indicator = 0, fu_header = 0;
    unsigned char nal_header, nal_header_type;
    int count = (dataLen + M_RTP_PAYLOAD_LEN - 1) / (M_RTP_PAYLOAD_LEN);
    int i, mark;
    int retVal = 0;
    char rtpheadbuf[2048];
    int rtpheadlen = 0;
    int csrclen    = 0;

    nal_header = pPayload[0];
    nal_header_type = nal_header & 0x1f;
    fu_indicator = (nal_header & 0xe0) | 0x1c;

    for (i = 0; i < count; i++)
    {
        memset(rtpheadbuf, 0,  sizeof(rtpheadbuf));
        rtpheadlen = 0;
        if (i == (count - 1))
        {
            mark = 1;
            payload_len   = left_bytes + 2;
            left_bytes    = 0;
        }
        else
        {
            mark = 0;
            payload_len = M_RTP_PAYLOAD_LEN;
            left_bytes -= M_RTP_PAYLOAD_LEN;
        }
        rtpheadlen = EXT_RTP_HREADER + csrclen;
        rtpheadbuf[0]   = EXT_RTP_ID;
        rtpheadbuf[1] = rtpInfo->type;
        *(unsigned short*)(rtpheadbuf + 2) = htons(payload_len + RTP_HEADER_LEN);
        rtpheadbuf[4] |= (M_RTP_VER & 0x3) << 6;
        rtpheadbuf[4] |= (0 & 0x1) << 5;
        rtpheadbuf[4] |= (0 & 0x1) << 4;
        rtpheadbuf[4] |= (htons(0) & 0xf);
        rtpheadbuf[5] |= (mark & 0x1) << 7;
        rtpheadbuf[5] |= rtpInfo->pt & 0x7f;
        *(unsigned short*)(rtpheadbuf + 6) = htons(rtpInfo->seq);
        *(unsigned int*)(rtpheadbuf + 8) = htonl(rtpInfo->ts);
        *(unsigned int*)(rtpheadbuf + 12) = htonl(rtpInfo->ssrc);

        RtpSeqAdd(&rtpInfo->seq);

        if (count > 1)
        {
            fu_header = 0;
            if(0 == i)
            {
                fu_header |= (1 << 7);
                payload_len -= 1;
                memcpy((rtpheadbuf + rtpheadlen + 2), (pPayload + 1), payload_len - 1);
                pPayload += payload_len;
                left_bytes += 1;
                rtplen = payload_len + 1;
            }
            else if(i == (count - 1))
            {
                fu_header |= (1 << 6);
                memcpy((rtpheadbuf + rtpheadlen + 2), pPayload, payload_len);
                pPayload += payload_len;
                rtplen = payload_len;
            }
            else
            {
                payload_len -= 2;
                memcpy((rtpheadbuf + rtpheadlen + 2), pPayload, payload_len);
                pPayload += payload_len;
                left_bytes += 2;
                rtplen = payload_len  + 2;
            }
            fu_header |= nal_header_type;
            rtpheadbuf[rtpheadlen] = fu_indicator;
            rtpheadbuf[rtpheadlen + 1] = fu_header;
            rtpheadlen += rtplen;
        }
        else
        {
            memcpy(rtpheadbuf + rtpheadlen, pPayload, payload_len);
            rtpheadlen += payload_len;
        }

        retVal = send(rtpFd, rtpheadbuf, rtpheadlen, 0);

        if (retVal < 0)
        {
            return retVal;
        }
    }

    return retVal;
}

int SendVideoOverTcp_Org(ite_rtp_Info *pSockInfo)
{
    int veFrameLen = 0;
    int rtpFd = pSockInfo->veRtpSock;
    char *streamBuf;
    char *p;
    int len;
    RTPINFO veRtpInfo;
    I_FRAME_INFO IFrameInfo;
    P_FRAME_INFO PFrameInfo;
    int frameType, isSend = 0;
    int retVal = 0, rc = 0;
    int  ts_step = (int)(90000 / FRAMERATE);
    ITE_SHARED_BUFFER_PROCMGR_STATE_T stProcMgrState;


    stProcMgrState.ncodetype = DATA_TYPE_H264;
    stProcMgrState.nindex       = SEM_SLOT_INDEX_USER1;
    stProcMgrState.nmilliseconds = SEMOP_TIMEOUT_MS; //ms

    veRtpInfo.seq = 1;
    veRtpInfo.ssrc = 0;

    while (g_bterminate == ITE_OSAL_FALSE)
    {
        //rc = ITE_SHARE_BUFFER_Get_Mgr(&stFrameHdl, &stProcMgrState);
        if (rc != MEDIA_DATA_NON_EMPTY)
        {
            usleep(200);
            continue;
        }
        else
        {
            usleep(100);
        }
        streamBuf = (char*)stProcMgrState.ptpayload ;
        veFrameLen = stProcMgrState.npayloadlen;

        if (veFrameLen > 0)
        {
            frameType = GetH264FrameType(streamBuf);

            if (frameType == M_FRAME_TYPE_I)
            {
                veRtpInfo.type = VIDEO_RTP;
                veRtpInfo.ts += ts_step;
                veRtpInfo.pt = pSockInfo->playLoadType;

                Iframe_process(streamBuf, veFrameLen, &IFrameInfo);

                len = IFrameInfo.spsLen;
                p = IFrameInfo.sps;
                if (len > 4)
                {
                    len -= 4;
                    p += 4;
                }

                retVal = RtpSendOverTcpPkt3984(rtpFd, p, len, &veRtpInfo);
                if (retVal < 0)
                {
                    //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);
                    break;
                }

                len = IFrameInfo.ppsLen;
                p = IFrameInfo.pps;
                if (len > 4)
                {
                    len -= 4;
                    p += 4;
                }

                retVal = RtpSendOverTcpPkt3984(rtpFd, p, len, &veRtpInfo);
                if (retVal < 0)
                {
                    //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);
                    break;
                }

                len = IFrameInfo.frameLen - 4;
                p = IFrameInfo.pFrame + 4;

                retVal = RtpSendOverTcpPkt3984(rtpFd, p, len, &veRtpInfo);
                if (retVal < 0)
                {
                    //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);
                    break;
                }
                isSend = 1;
            }
            else if (!isSend)
            {
                //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);
                continue;
            }
            else
            {
                veRtpInfo.type = VIDEO_RTP;
                veRtpInfo.ts += ts_step;
                veRtpInfo.pt = pSockInfo->playLoadType;
                Pframe_process(streamBuf, veFrameLen, &PFrameInfo);
                len = PFrameInfo.frameLen - 4;
                p = PFrameInfo.pFrame + 4;

                retVal = RtpSendOverTcpPkt3984(rtpFd, p, len, &veRtpInfo);
                if (retVal < 0)
                {
                    //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);
                    break;
                }
            }
        }
        //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);

        if(g_work == 0)
            break;
        if ((veFrameLen <= 0))
        {
            usleep(2000);
        }
    }

    if(pSockInfo->veRtpSock > -1)
    {
        close(pSockInfo->veRtpSock );
    }
    if(pSockInfo->veRtcpSock > -1)
    {
        close(pSockInfo->veRtcpSock );
    }
    if(pSockInfo->auRtpSock > -1)
    {
        close(pSockInfo->auRtpSock );
    }
    if(pSockInfo->auRtcpSock > -1)
    {
        close(pSockInfo->auRtcpSock );
    }
    pSockInfo->veRtpSock = -1;
    pSockInfo->auRtpSock = -1;
    pSockInfo->veRtcpSock = -1;
    pSockInfo->auRtcpSock = -1;
    //ITE_SHARE_BUFFER_Deinit_Mgr(&stProcMgrState);
    return (retVal < 0 ? retVal : 0);
}

static int SendVideoOverTcp(ite_rtp_Info *pSockInfo)
{
    int veFrameLen = 0, auFrameLen = 0;
    int veRtpFd = pSockInfo->veRtpSock;
    char *pH264frame = NULL;
    int  rc = -1;
    int isSendIframe = 1;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    struct sockaddr_in veAddr;
    //I_FRAME_INFO IFrameInfo;
    //P_FRAME_INFO PFrameInfo;
    int frameType = 0, isSend = 0;
    int retVal = 0;
    char alaw_data[4096];
    int alaw_len = 0;
    unsigned char stream_id =0xE0 ;//0xE0 video     0xC0 audio
    //unsigned int ts_step = (unsigned int)(90000 / FRAMERATE);
    int  stFrameHdl;
    frameData_Info_s frameInfo_rtp;
	int total_frame = 0;
    int streamType = 1;
    int curPts = 0;
    int oneFramePts = 0;

    memset(&stH264Config, 0, sizeof(stH264Config));
    netcam_video_get(0, pSockInfo->mediaChannel, &stH264Config);
    oneFramePts = 90000/stH264Config.fps;

    pSockInfo->frameBufferSize = 70 * 1024;
    if (pSockInfo->frameBuffer == NULL)
    {
        pSockInfo->frameBuffer = (char*)malloc(pSockInfo->frameBufferSize);
        if (pSockInfo->frameBuffer == NULL)
        {
            printf("malloc %d failed.\n", pSockInfo->frameBufferSize);
            rtpCloseHandle(pSockInfo);
            return -1;
        }
    }

    stFrameHdl = rtp_get_data_init(pSockInfo->mediaChannel);

    veAddr.sin_family = AF_INET;
    veAddr.sin_addr.s_addr = inet_addr(pSockInfo->remoteip);
    veAddr.sin_port = htons(pSockInfo->videoPort);
    memset(&frameInfo_rtp, 0, sizeof(frameData_Info_s));
    frameInfo_rtp.u16CSeq = 1;
#ifdef MODULE_SUPPORT_MOJING
    frameInfo_rtp.paddingFlg = 1;
#else
    frameInfo_rtp.paddingFlg = 0;
#endif
    if (stH264Config.enctype == 1)
    {
        frameInfo_rtp.streamType = 0x1b;
    }
    else if (stH264Config.enctype == 3)
    {
        frameInfo_rtp.streamType = 0x24;
    }
    else
    {
        frameInfo_rtp.streamType = 0x1b;
    }

    

    while (pSockInfo->isRun)
    {
        pH264frame = NULL;
        veFrameLen = 0;

        rc = rtp_get_data(stFrameHdl, (void **)&pH264frame, &veFrameLen, &frameType, &frameInfo_rtp.u32CurPts);
        if (rc != MEDIA_DATA_NON_EMPTY)
        {
            usleep(200);
            continue;
        }


        //pH264frame  = stProcMgrState.ptpayload;
        //veFrameLen = stProcMgrState.npayloadlen;
        /*
        if(isSend)
        {
            frameInfo_rtp.u32CurPts += ts_step ;
        }
        */


        frameInfo_rtp.u32Ssrc = pSockInfo->ssrc;
        frameInfo_rtp.u32Sec = time(NULL);
        if (frameType == M_FRAME_TYPE_I || frameType == M_FRAME_TYPE_P)
        {
            frameInfo_rtp.u32RtpPts = frameInfo_rtp.u32CurPts;
        }
        else if (frameType == M_FRAME_TYPE_AUDIO)
        {
            //音频rtp的pts在视频的基础上加320
            frameInfo_rtp.u32RtpPts += 320;
            frameInfo_rtp.u32CurPts  = frameInfo_rtp.u32RtpPts;
        }

        
        if (veFrameLen > 0)
        {
            //frameType = GetH264FrameType(pH264frame);
            if (isSendIframe && (frameType != M_FRAME_TYPE_I))
            {
                //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);
                continue;
            }
            else
            {
                isSendIframe = 0;
            }

            stream_id = 0xE0;
            if (frameType == M_FRAME_TYPE_I)
            {
                frameInfo_rtp.IFrame = 1;
                retVal = ite_RtpSendTcp_PS_H264frame(pSockInfo, pH264frame, veFrameLen, &frameInfo_rtp, stream_id, veRtpFd, (struct sockaddr *)&veAddr); // RtpSendOverUdpPkt3984(veRtpFd, p, len, (struct sockaddr *)&veAddr, &veRtpInfo);
                if (retVal < 0)
                {
                    //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);
                    break;
                }
                isSend = 1;
				total_frame++;
            }
            else if (!isSend)
            {
                //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);
                continue;
            }
            else if (frameType == M_FRAME_TYPE_P)
            {
                frameInfo_rtp.IFrame = 0;

                retVal = ite_RtpSendTcp_PS_H264frame(pSockInfo, pH264frame, veFrameLen, &frameInfo_rtp, stream_id, veRtpFd, (struct sockaddr *)&veAddr); // RtpSendOverUdpPkt3984(veRtpFd, p, len, (struct sockaddr *)&veAddr, &veRtpInfo);
                if (retVal < 0)
                {
                    //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);
                    break;
                }

				total_frame++;
            }
			else if (frameType == M_FRAME_TYPE_AUDIO)
			{
                //continue;
				stream_id = 0xC0;
                frameInfo_rtp.IFrame = 0;
				alaw_len = audio_alaw_encode((unsigned char*)alaw_data, (short*)pH264frame, veFrameLen);
				//printf("-->send audio len:%d,%d pts:%d(%d)\n", veFrameLen, alaw_len, frameInfo_rtp.u32CurPts,frameInfo_rtp.u32RtpPts);

				retVal = ite_RtpSendTcp_PS_H264frame(pSockInfo, alaw_data, alaw_len, &frameInfo_rtp, stream_id, veRtpFd, (struct sockaddr *)&veAddr); // RtpSendOverUdpPkt3984(veRtpFd, p, len, (struct sockaddr *)&veAddr, &veRtpInfo);
				if (retVal < 0)
				{
					break;
				}
			}
        }

        if ((veFrameLen <= 0) && (auFrameLen <= 0))
        {
            usleep(20000);
        }
        //ITE_SHARE_BUFFER_Release_Mgr(&stFrameHdl, &stProcMgrState);

    }

	rtp_stream_status_send(pSockInfo->rtpType, pSockInfo->remoteip, pSockInfo->videoPort, pSockInfo->ssrc, pSockInfo->mediaChannel, retVal,0,0,0,total_frame,NULL);

    if(pSockInfo->veRtpSock > -1)
    {
        close(pSockInfo->veRtpSock );
    }
    if(pSockInfo->veRtcpSock > -1)
    {
        close(pSockInfo->veRtcpSock );
    }
    if(pSockInfo->auRtpSock > -1)
    {
        close(pSockInfo->auRtpSock );
    }
    if(pSockInfo->auRtcpSock > -1)
    {
        close(pSockInfo->auRtcpSock );
    }
    pSockInfo->veRtpSock = -1;
    pSockInfo->auRtpSock = -1;
    pSockInfo->veRtcpSock = -1;
    pSockInfo->auRtcpSock = -1;
    rtp_get_data_close(stFrameHdl);
    if (pSockInfo->frameBuffer != NULL)
    {
        free(pSockInfo->frameBuffer);
        pSockInfo->frameBuffer = NULL;
    }
    return (retVal < 0 ? retVal : 0);
}

static int ConnectTcpChannel(ite_rtp_Info *pSockInfo)
{
    int sockFd;
    int on = 1;
    int ret = -1;
    int count = 0;
    int iMode = 1;
    fd_set set;
    struct timeval tm;
    struct sockaddr_in servAddr;
	struct timeval send_timeout = {10,0};


#ifdef TS_TEST_SERVER_ALL
    if (strcmp(tsTestIp, "192.168.10.xxx") == 0)
    {
        struct hostent* host=gethostbyname(hostAddr);
        if (!host)
        {
            printf("get host:%s ip addr error\n", hostAddr);
        }
        else
        {
            strncpy(tsTestIp, inet_ntoa(*((struct in_addr *)host->h_addr_list[0])), 16);
            printf("get host ip ok:%s\n", tsTestIp);
        }
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(tsTestPort);
    servAddr.sin_addr.s_addr = inet_addr(tsTestIp);
    on = connect(pSockInfo->selfCloudTestFd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (on >= 0)
    {
        printf("test server connect ok\n");
    }
    else
    {
        printf("test server connect failed\n");
    }

    if (pSockInfo->selfCloudTestFd != -1)
    {
        send(pSockInfo->selfCloudTestFd, runGB28181Cfg.DeviceUID, 19, 0);
    }
#endif


    sockFd = pSockInfo->veRtpSock;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(pSockInfo->videoPort);
    servAddr.sin_addr.s_addr = inet_addr(pSockInfo->remoteip);
    printf("sockFd:%d, ip:%s, port:%d\r\n", sockFd, pSockInfo->remoteip, pSockInfo->videoPort);
    if (strlen(pSockInfo->remoteip) == 0 || pSockInfo->videoPort == 0)
    {
        return -1;
    }

    ioctl(sockFd, FIONBIO, &iMode);

    while (pSockInfo->isRun)
    {
        on = connect(sockFd, (struct sockaddr *)&servAddr, sizeof(servAddr));
        iMode = 0;
        ioctl(sockFd, FIONBIO, &iMode);
        if ( on < 0)
        {
            while(pSockInfo->isRun)
            {
                printf("err: %s, count:%d\r\n", strerror(errno), count);
                FD_ZERO(&set);
                FD_SET(sockFd, &set);
                tm.tv_sec  = 3;
                tm.tv_usec = 0;
                ret = select(sockFd + 1, NULL, &set, NULL, &tm);
                if (ret <= 0)
                {
                    if(count++ > 40)
                    {
                        close(sockFd);
                        pSockInfo->veRtpSock = -1;
                        pSockInfo->auRtpSock = -1;
                        return -1;
                    }
                }
                else
                {
                    int error = -1;
                    int optLen = sizeof(int);
                    getsockopt(sockFd, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);
                    if (0 != error)
                    {
                        close(sockFd);
                        pSockInfo->veRtpSock = -1;
                        pSockInfo->auRtpSock = -1;
                        return ret;
                    }
                    ret = 0;
                    break;
                }
            }
            break;

        }
        else
        {
            printf("connect ok sockFd:%d, ip:%s, port:%d\r\n", sockFd, pSockInfo->remoteip, pSockInfo->videoPort);
            ret = 0;
            break;
        }
    }

    if (ret != 0)
    {
        close(sockFd);
        pSockInfo->veRtpSock = -1;
        pSockInfo->auRtpSock = -1;
        return ret;
    }

    setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, (void*)&send_timeout, sizeof(struct timeval));
    return 0;
}

static int acceptTcpChannel(ite_rtp_Info *pSockInfo)
{
    if(listen(pSockInfo->veRtpSock,1024) == -1)
    {
        perror("listen: ");
        return -1;
    }

    ///客户端套接字
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    printf("wait client connect\n");
    ///成功返回非负描述字，出错返回-1
    int conn = accept(pSockInfo->veRtpSock, (struct sockaddr*)&client_addr, &length);
    if(conn<0)
    {
        perror("connect: ");
        exit(1);
    }
    printf( "client fd:%d, client IP：%s:%d", conn, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    pSockInfo->veRtpSock = conn;
    return 0;
}

static int CreateSockChannel(ite_rtp_Info *pSockInfo)
{
    int retVal = -1;
    int sendBufLen = 0;
    socklen_t optlen = sizeof(int);

    if (1 == (pSockInfo->enOverTcp & 0x000F))
    {
        retVal = CreateTcpSocket(pSockInfo);
        if (0x0010 == (pSockInfo->enOverTcp & 0x00F0))/*低字节中的高4位为1代表代表IPC是TCP server*/
        {
            retVal = acceptTcpChannel(pSockInfo);/*active*/
        }
        else
        {
            retVal = ConnectTcpChannel(pSockInfo);/*passive*/
        }
    }
    else
    {
        retVal = CreateUdpSocket(pSockInfo);
    }
    if (retVal == 0)
    {
        if (pSockInfo->veRtpSock > 0)
        {
            getsockopt(pSockInfo->veRtpSock, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufLen, &optlen);

            if (sendBufLen < SOCK_BUF_SIZE)
            {
                sendBufLen = SOCK_BUF_SIZE;
                setsockopt(pSockInfo->veRtpSock, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufLen, optlen);

                sendBufLen = 0;
                getsockopt(pSockInfo->veRtpSock, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufLen, &optlen);
            }
        }
    }
    return retVal;
}

void rtp_stream_fail_event_upload(int rtptype, char *ip, int port, int ssrc, int channel,int result, char is_connect, int begin, int end, int tota, float *speed)
{
	char content[500] = {0};
	char err_msg[100] = {0};
	int flag = 1;
		
	if(!ip || result == 0)
		return;

	memset(content,0,sizeof(content));

	//play:0,  download:1, playback:2, audio:3, cloudstorage 4,
	
	if(rtptype == 0)
	{
		snprintf(content, sizeof(content),"MediaSvr=%s:%d,ssrc=%d,Stream_Type=%s",ip,port,ssrc,channel==0?"Main":"Sub");
				
		if(strlen(errinfo[rtptype]))
		{
			strcat(err_msg,errinfo[rtptype]);
		}

		if(is_connect)
			flag = 1;
		else
			flag = 2;

#ifdef MODULE_SUPPORT_MOJING
		mojing_event_log_send(NULL, "GB28181_RealPlay", flag, content, err_msg, NULL);
#endif
	}
	else if(rtptype == 1)
	{
		if(speed)
			snprintf(content, sizeof(content),"MediaSvr=%s:%d,ssrc=%d,times=%02.2f,Protocol=tcp",ip,port,ssrc,*speed);
		else
			snprintf(content, sizeof(content),"MediaSvr=%s:%d,ssrc=%d,times=4.0,Protocol=tcp",ip,port,ssrc);
			
		if(strlen(errinfo[rtptype]))
		{
			strcat(err_msg,errinfo[rtptype]);
		}

		if(is_connect)
			flag = 1;
		else
			flag = 2;

#ifdef MODULE_SUPPORT_MOJING
		mojing_event_log_send(NULL, "GB28181_VideoPlay", flag, content, err_msg, NULL);
#endif
	}
	else if(rtptype == 2)
	{
		if(speed)
			snprintf(content, sizeof(content),"MediaSvr=%s:%d,ssrc=%d,times=%02.2f,Protocol=tcp",ip,port,ssrc,*speed);
		else
			snprintf(content, sizeof(content),"MediaSvr=%s:%d,ssrc=%d,times=1.0,Protocol=tcp",ip,port,ssrc);
			
				
		if(strlen(errinfo[rtptype]))
		{
			strcat(err_msg,errinfo[rtptype]);
		}

		if(is_connect)
			flag = 1;
		else
			flag = 3;
	
#ifdef MODULE_SUPPORT_MOJING
		mojing_event_log_send(NULL, "GB28181_VideoPlay", flag, content, err_msg, NULL);
#endif
	}
	else if(rtptype == 3)
	{
	
		snprintf(content, sizeof(content),"AudioSvr=%s:%d,ssrc=%d,Stream_Type=%s",ip,port,ssrc);
				
		if(strlen(errinfo[rtptype]))
		{
			strcat(err_msg,errinfo[rtptype]);
		}

		flag = is_connect;
	

#ifdef MODULE_SUPPORT_MOJING
		mojing_event_log_send(NULL, "GB28181_AudioPlay", flag, content, err_msg, NULL);
#endif
	}
	else if(rtptype == 4)
	{
		snprintf(content, sizeof(content),"StorageSvr=%s:%d,ssrc=%d,StartTime=%d,EndTime=%d",ip,port,ssrc,begin,end);
		if(strlen(errinfo[rtptype]))
		{
			strcat(err_msg,errinfo[rtptype]);
		}

		if(is_connect)
			flag = 1;
		else
			flag = 2;
	
#ifdef MODULE_SUPPORT_MOJING
		mojing_event_log_send(NULL, "MJ_CloudStorage", flag, content, err_msg, NULL);
#endif
	}
}

void rtp_stream_status_send(int rtptype, char *ip, int port, int ssrc, int channel,int result, char is_connect, int begin, int end, int total, float *speed)
{

	if(!ip)
		return;

	if(result == -1)
		rtp_stream_fail_event_upload(rtptype, ip, port, ssrc, channel,result, is_connect, begin, end, total, speed);

#ifdef MODULE_SUPPORT_UDP_LOG

	char msgStr[500] = {0};
	memset(msgStr,0,sizeof(msgStr));

	//play:0,  download:1, playback:2, audio:3, cloudstorage 4,

	if(rtptype == 0)
	{
		if(is_connect)
		{
			snprintf(msgStr, sizeof(msgStr),"connect(ip=%s, port=%d, ssrc=%d, stream_type=%s) %s.",\
			ip,port,ssrc,channel==0?"Main":"Sub",result==0?"succ":"fail");
		}
		else
		{

			if(result == 0)
			{
				snprintf(msgStr, sizeof(msgStr),"send_end(ip=%s, port=%d, ssrc=%d, stream_type=%s) .",ip,port,ssrc,channel==0?"Main":"Sub");
			}
			else
			{
				snprintf(msgStr, sizeof(msgStr),"sendto(ip=%s, port=%d, ssrc=%d, stream_type=%s) fail, send frames=%d.",ip,port,ssrc,channel==0?"Main":"Sub",total);
				if(strlen(errinfo[rtptype]))
				{
					strcat(msgStr,errinfo[rtptype]);
					memset(&errinfo[rtptype],0,sizeof(errinfo[rtptype]));
				}
			}
		}

		#ifdef MODULE_SUPPORT_MOJING
		mojing_log_send(NULL, result, "gb28181_realplay", msgStr);
		#endif
	}
	else if(rtptype == 1)
	{
		if(is_connect)
		{
			snprintf(msgStr, sizeof(msgStr),"connect(ip=%s, port=%d, ssrc=%d) %s.",\
			ip,port,ssrc,result==0?"succ":"fail");
		}
		else
		{
			if(result == 0)
			{
				snprintf(msgStr, sizeof(msgStr),"send_end(ip=%s, port=%d, ssrc=%d) .",ip,port,ssrc);
			}
			else
			{
				snprintf(msgStr, sizeof(msgStr),"sendto(ip=%s, port=%d, ssrc=%d) fail, send frames=%d.",ip,port,ssrc,total);
				if(strlen(errinfo[rtptype]))
				{
					strcat(msgStr,errinfo[rtptype]);
					memset(&errinfo[rtptype],0,sizeof(errinfo[rtptype]));
				}
			}
		}

		#ifdef MODULE_SUPPORT_MOJING
		mojing_log_send(NULL, result, "gb28181_download", msgStr);
		#endif
	}
	else if(rtptype == 2)
	{
		if(is_connect)
		{
			snprintf(msgStr, sizeof(msgStr),"connect(ip=%s, port=%d, ssrc=%d) %s.",\
			ip,port,ssrc,result==0?"succ":"fail");
		}
		else
		{

			if(result == 0)
			{
				snprintf(msgStr, sizeof(msgStr),"send_end(ip=%s, port=%d, ssrc=%d) .",ip,port,ssrc);
			}
			else
			{
				snprintf(msgStr, sizeof(msgStr),"sendto(ip=%s, port=%d, ssrc=%d) fail, send frames=%d.",ip,port,ssrc,total);
				if(strlen(errinfo[rtptype]))
				{
					strcat(msgStr,errinfo[rtptype]);
					memset(&errinfo[rtptype],0,sizeof(errinfo[rtptype]));
				}
			}
		}

		#ifdef MODULE_SUPPORT_MOJING
		mojing_log_send(NULL, result, "gb28181_playback", msgStr);
		#endif
	}
	else if(rtptype == 4)
	{
		if(is_connect)
		{
			snprintf(msgStr, sizeof(msgStr),"connect(ip=%s, port=%d, ssrc=%d) %s.",\
			ip,port,ssrc,result==0?"succ":"fail");
			#ifdef MODULE_SUPPORT_MOJING
			mojing_log_send(NULL, result, "dsgw_cloud_record", msgStr);
			#endif
		}
		else
		{
			if(result == 0)
			{
				snprintf(msgStr, sizeof(msgStr),"sendto(ip=%s, port=%d, ssrc=%d) over,beg_time=%d, end_time=%d, total=%d.",ip,port,ssrc,begin,end,total);
			}
			else
			{
				snprintf(msgStr, sizeof(msgStr),"sendto(ip=%s, port=%d, ssrc=%d) fail,beg_time=%d, end_time=%d, total=%d.",ip,port,ssrc,begin,end,total);
				if(strlen(errinfo[rtptype]))
				{
					strcat(msgStr,errinfo[rtptype]);
					memset(&errinfo[rtptype],0,sizeof(errinfo[rtptype]));
				}
			}

			#ifdef MODULE_SUPPORT_MOJING
			mojing_log_send(NULL, result, "dsgw_cloud_finish", msgStr);
			#endif
		}
	}
	
#endif

}
void *SendVideoRtpTask(void *pData)
{
    int retVal = -1;
    ite_rtp_Info *pSockInfo = (ite_rtp_Info *)pData;
    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("SendVideoRtpTask");

    retVal = CreateSockChannel(pSockInfo);
    if (retVal != 0)
    {
        printf("CreateSockChannel failed:%d\n", retVal);

		rtp_stream_status_send(pSockInfo->rtpType, pSockInfo->remoteip, pSockInfo->videoPort, pSockInfo->ssrc, pSockInfo->mediaChannel, -1,1,0,0,0,NULL);

        rtpCloseHandle(pSockInfo);
        return NULL;
    }

    printf("CreateSockChannel  [%d]  enOverTcp: 0x%04x, pSockInfo->veRtpSock:%d \n", retVal, pSockInfo->enOverTcp, pSockInfo->veRtpSock);

	rtp_stream_status_send(pSockInfo->rtpType, pSockInfo->remoteip, pSockInfo->videoPort, pSockInfo->ssrc, pSockInfo->mediaChannel, 0,1,0,0,0,NULL);

    netcam_md_play_start_one();
    if (pSockInfo->rtpType == 0)
    {

        if (1 == (pSockInfo->enOverTcp & 0x000F))/*低字节中的低4位为1代表是TCP方式*/
        {
            /*active\passive只有在createsocket时不同，发送数据时相同，不做区分*/
            printf("SendVideoRtpTask tcp, fd:%d, ssrc:%d\n", pSockInfo->veRtpSock, pSockInfo->ssrc);
            retVal = SendVideoOverTcp(pSockInfo);
        }
        else
        {
            printf("SendVideoRtpTask udp, fd:%d\n", pSockInfo->veRtpSock);
            retVal = SendVideoOverUdp(pSockInfo);
        }
        printf("SendVideoProc status [%d]\n", retVal);

        pSockInfo->isUsed = 0;
    }
    else
    {
        //rtpCloseHandle(pSockInfo);
		struct timeval rcto = {6,0};

        if(pSockInfo->rtpType == 1 || pSockInfo->rtpType == 2)
			setsockopt(pSockInfo->veRtpSock, SOL_SOCKET,SO_SNDTIMEO,  (void *)&rcto,sizeof(struct timeval));
    }
    netcam_md_play_stop_one();
    return (void *)NULL;
}

static int tcp_readn(int connfd, void *vptr, int n, char *runflag)
{
	int ret = 0;
	int	nleft;
	int	nread;
	int timeoutCnt = 0;
	char *ptr;
	fd_set rset,excpt_set;
	struct timeval 	timeout;

	if(!runflag)
		return -1;

	ptr = vptr;
	nleft = n;

	while (nleft > 0)
	{
		if(timeoutCnt >= 40)
		{
			printf("tcp_readn timeout!\n");
			break;
		}

		if(*runflag == 0)
		{
			printf("tcp_readn exit for server stop!\n");
			return -1;
		}

		FD_ZERO(&rset);
		FD_ZERO(&excpt_set);
		FD_SET(connfd, &rset);
		FD_SET(connfd, &excpt_set);

		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;
		if((ret = select(connfd+1, &rset, NULL, &excpt_set, &timeout)) < 0)
		{
			printf("tcp_readn select error:%s!\n", strerror(errno));
			return -1;
		}

        if(FD_ISSET(connfd, &excpt_set))
        {
            printf("tcp_readn select excpt_set error:%s!\n", strerror(errno));
            return -1;
        }

        if(0 == ret)
        {
			timeoutCnt++;
            continue;
        }

        if(FD_ISSET(connfd, &rset))
        {
            timeoutCnt = 0;

			if((nread = recv(connfd, ptr, nleft, 0)) < 0)
			{
				if(errno == EINTR || errno == EAGAIN)
				{
					nread = 0;
				}
				else
				{
					printf("tcp_readn recv error:%s!\n", strerror(errno));
					return -1;
				}
			}
			else if (nread == 0)
			{
				printf("tcp_readn recv no data!\n");
				return -1;
			}

			nleft -= nread;
			ptr   += nread;
        }
		else
		{
            printf("tcp_readn select no data!\n");
			break;
		}

	}

	return(n - nleft);
}


static int tcp_writen(int connfd, void *vptr, size_t n)
{
	int nleft, nwritten;
 	char	*ptr;

	ptr = vptr;
	nleft = n;

	while(nleft>0)
	{
		if((nwritten = send(connfd, ptr, nleft, 0)) == -1)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				printf("tcp_writen EINTR\n");
				nwritten = 0;
			}
			else
			{
				printf("tcp_writen error:%s\n", strerror(errno));
				return -1;
			}
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}

	return(n);
}

static int udp_readn(int connfd, char *ip, int port,void *vptr, int n, char *runflag)
{
	int ret = 0;
	int	nleft;
	int	nread;
	char *ptr;
	int timeoutCnt = 0;
	fd_set rset,excpt_set;
    int addr_size = 0;
    struct sockaddr_in addr;
	struct timeval 	timeout;

	if(!runflag)
		return -1;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
	addr_size = sizeof(struct sockaddr_in);

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if(timeoutCnt >= 40)
		{
			printf("udp_readn timeout!\n");
			break;
		}

		if(*runflag == 0)
		{
			printf("udp_readn exit for server stop!\n");
			return -1;
		}

		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;
		FD_ZERO(&rset);
		FD_SET(connfd, &rset);
		if(select(connfd+1, &rset, NULL, NULL, &timeout) <= 0)
		{
			printf("udp_readn error:%s\n", strerror(errno));
			return -1;
		}

        if(FD_ISSET(connfd, &excpt_set))
        {
            printf("udp_readn select excpt_set error:%s!\n", strerror(errno));
            return -1;
        }

        if(0 == ret)
        {
			timeoutCnt++;
            continue;
        }

        if(FD_ISSET(connfd, &rset))
        {
            timeoutCnt = 0;

			if(nread = recvfrom(connfd, ptr, nleft, 0, (struct sockaddr *)&addr, (socklen_t *)&addr_size) < 0)
			{
				if(errno == EINTR || errno == EAGAIN)
				{
					nread = 0;
				}
				else
				{
					return -1;
				}
			}
			else if (nread == 0)
			{
				break;
			}

			nleft -= nread;
			ptr   += nread;
        }
		else
		{
            printf("udp_readn select no data!\n");
			break;
		}
	}

	return(n - nleft);
}

static int rtp_parser_header(unsigned char *buf,int len,int *payload_type,int *payload_offset)
{
	rtp_hdr RTPHEADER;
	int Version = 0,Padding = 0, Extension = 0, Cc = 0,marker = 0,PT = 0;
	int SeqNum = 0,Timestamp = 0,SSRC = 0,CSRC = 0;
	int payloadOffset = 0;

	if(len < 12)
	{
		printf("RTP header too short!\n");
		return -1;
	}

	memset(&RTPHEADER, 0x0, sizeof(rtp_hdr));

	RTPHEADER.flags = buf[0];

	Version = (RTPHEADER.flags >> 6) & 0x3;
	Padding = (RTPHEADER.flags >> 5) & 0x1;
	Extension = (RTPHEADER.flags >> 4) & 0x1;

	if(Padding)
	{
		int paddingLength = buf[len-1];
		printf("paddingLength = %d\n",paddingLength);

		if(paddingLength + 12 > len)
		{
			printf("paddingLength error!\n");
		}
		else
			len -= paddingLength;
	}

	Cc =  RTPHEADER.flags & 0x0f;

	payloadOffset = 12 + 4*Cc;
	if(len < payloadOffset)
	{
		printf("payloadOffset %d error!\n",payloadOffset);
		return -1;
	}

	if(Extension)
	{
		if(len < payloadOffset + 4)
		{
			printf("Extension may error!\n");
			return -1;
		}

		unsigned char *extData = &buf[payloadOffset];
		int extLen = 4*(extData[2]<<8 | extData[3]);

		if(len < payloadOffset + 4 + extLen)
		{
			printf("Extension len error!\n");
			return -1;
		}

		payloadOffset += (4 + extLen);
	}

	marker = buf[1]&0x80?1:0;
	PT = buf[1]&0x7f;
	SeqNum = buf[2] << 8 | buf[3];
	Timestamp = (buf[4] << 24) | (buf[5] << 16) | buf[6] << 8 | buf[7];
	SSRC = (buf[8] << 24) | (buf[9] << 16) | buf[10] << 8 | buf[11];

	//printf("marker = %d,PT = %d,seqNum = %d,Timestamp = %d,SSRC = %d,payloadOffset:%d!\n",marker,PT,SeqNum,Timestamp,SSRC,payloadOffset);

	if(payload_type)
		*payload_type = PT;

	if(payload_offset)
		*payload_offset = payloadOffset;

	return 0;
}
static int RecvAudioOverUdp(ite_rtp_Info *pSockInfo)
{
#define MAX_BUF_SIZE  1024*8
    struct sockaddr_in auAddr;
    fd_set read_set;
	int ret = 0;
	unsigned char buf[MAX_BUF_SIZE];
	int len = 0;
	int i = 0;
	int payloadType = 0;
	int payloadOffset = 0;
	char szRtpHdr[RTP_TCP_HDR_LEN];
	unsigned short seq = 1;
	unsigned char recvHeader[2];
	unsigned short recvLen = 0;
    int auRtpFd = pSockInfo->veRtpSock;

	memset(buf,0,sizeof(buf));
	memset(szRtpHdr, 0, RTP_TCP_HDR_LEN);
	ite_make_rtptcp_header(&seq, szRtpHdr, 0, 0, pSockInfo->ssrc, time(NULL), 0, 0);

    auAddr.sin_family = AF_INET;
    auAddr.sin_addr.s_addr = inet_addr(pSockInfo->remoteip);
    auAddr.sin_port = htons(pSockInfo->videoPort);

	len = 0;

	while(len < RTP_TCP_HDR_LEN)
	{
	    ret = sendto(auRtpFd, (char *)&szRtpHdr[len], RTP_TCP_HDR_LEN - len, 0, (struct sockaddr*)&auAddr,  sizeof(struct sockaddr_in));
		if (ret < 0)
		{
			if (errno != EINTR && errno != EAGAIN )
			{
				printf("socket %d error :%s\n", auRtpFd, strerror(errno));
				ret = -1;
				goto EXIT_UDP_RECV_AUDIO;
			}
			else
				continue;
		}

		len += ret;
	}

    while (pSockInfo->isRun)
    {

		memset(&recvHeader,0,sizeof(recvHeader));
		len = udp_readn(auRtpFd, pSockInfo->remoteip,pSockInfo->videoPort,recvHeader, 2, &pSockInfo->isRun);
		if(len != 2)
		{
			printf("RecvAudioOverUdp recv len failed!\n");
			ret = -1;
			goto EXIT_UDP_RECV_AUDIO;
		}
		else
		{
			recvLen = recvHeader[0]<<8 | recvHeader[1];
			//printf("RecvAudioOverUdp recv len = %d!\n",recvLen);
		}

		if(recvLen > MAX_BUF_SIZE)
		{
			printf("RecvAudioOverUdp exceed max size!\n");
			ret = -1;
			goto EXIT_UDP_RECV_AUDIO;
		}

		memset(buf,0,sizeof(buf));
		len = udp_readn(auRtpFd, pSockInfo->remoteip,pSockInfo->videoPort,buf, recvLen, &pSockInfo->isRun);
		if(len != recvLen)
		{
			printf("RecvAudioOverUdp recv data error,len = %d!\n",len);
			ret = -1;
			goto EXIT_UDP_RECV_AUDIO;

		}

		if(len > 12)
		{
#if 0
			printf("\n");

			for(i=0; i<12; i++)
			{
				printf("0x%x ",buf[i]);
			}

			printf("\n");
#endif

			ret = rtp_parser_header(buf,len,&payloadType,&payloadOffset);

			if(ret < 0)
				printf("parser rtp header error!\n");
			else
			{
//				if(payloadType == 8 && payloadOffset < len)	xqq
//            		netcam_audio_output((char *)&buf[payloadOffset],len-payloadOffset, NETCAM_AUDIO_ENC_A_LAW, SDK_TRUE);	xqq
			//	else
					printf("RecvAudioOverUdp payloadType = %d,payloadOffset = %d\n",payloadType,payloadOffset);
			}
		}

    }

EXIT_UDP_RECV_AUDIO:

	rtpCloseHandle(pSockInfo);

    return (ret < 0 ? ret : 0);

}

static int RecvAudioOverTcp(ite_rtp_Info *pSockInfo)
{

#define MAX_BUF_SIZE  1024*8

    int ret = 0;
    int len = 0;
	int i = 0;
	int payloadType = 0;
	int payloadOffset = 0;
    unsigned char buf[MAX_BUF_SIZE];
	char szRtpHdr[RTP_TCP_HDR_LEN];
	unsigned short seq = 1;
	unsigned char recvHeader[2];
	unsigned short recvLen = 0;
    int auRtpFd = pSockInfo->veRtpSock;

	memset(szRtpHdr, 0, RTP_TCP_HDR_LEN);
	memset(buf,0,sizeof(buf));

	ite_make_rtptcp_header(&seq, szRtpHdr, 1, 0, pSockInfo->ssrc, time(NULL), 0, 0);
	len = tcp_writen(auRtpFd, szRtpHdr, RTP_TCP_HDR_LEN);
	if(len != RTP_TCP_HDR_LEN)
	{
		printf("RecvAudioOverTcp send rtp header failed!\n");
		ret = -1;
		goto EXIT_TCP_RECV_AUDIO;
	}

    while (pSockInfo->isRun)
    {
		memset(&recvHeader,0,sizeof(recvHeader));
		len = tcp_readn(auRtpFd, recvHeader, 2, &pSockInfo->isRun);
		if(len != 2)
		{
			printf("RecvAudioOverTcp recv header len failed!\n");
			ret = -1;
			goto EXIT_TCP_RECV_AUDIO;
		}
		else
		{
			recvLen = recvHeader[0]<<8 | recvHeader[1];
			//printf("RecvAudioOverTcp recv len = %d!\n",recvLen);
		}

		if(recvLen > MAX_BUF_SIZE)
		{
			printf("RecvAudioOverTcp exceed max size!\n");
			ret = -1;
			goto EXIT_TCP_RECV_AUDIO;
		}

		len = tcp_readn(auRtpFd, buf, recvLen, &pSockInfo->isRun);
		if(len != recvLen)
		{
			printf("RecvAudioOverTcp recv data error,len = %d!\n",len);
			ret = -1;
			goto EXIT_TCP_RECV_AUDIO;

		}

		if(len > 12)
		{
#if 0
			printf("\n");

			for(i=0; i<12; i++)
			{
				printf("0x%x ",buf[i]);
			}

			printf("\n");
#endif
			ret = rtp_parser_header(buf,len,&payloadType,&payloadOffset);

			if(ret < 0)
				printf("parser rtp header error!\n");
			else
			{
			//	if(payloadType == 8 && (payloadOffset < len))
            	//	netcam_audio_output_gb28181((char *)&buf[payloadOffset],len-payloadOffset, NETCAM_AUDIO_ENC_A_LAW, SDK_TRUE);
		//		else	xqq
					printf("RecvAudioOverTcp payloadType = %d,payloadOffset = %d\n",payloadType,payloadOffset);
			}
		}
    }

EXIT_TCP_RECV_AUDIO:

	rtpCloseHandle(pSockInfo);

    return (ret < 0 ? ret : 0);
}

void *RecvAudioRtpTask(void *pData)
{
    int retVal = -1;
    ite_rtp_Info *pSockInfo = (ite_rtp_Info *)pData;
    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("RecvAudioRtpTask");

    retVal = CreateSockChannel(pSockInfo);
    if (retVal != 0)
    {
		rtp_stream_status_send(pSockInfo->rtpType, pSockInfo->remoteip, pSockInfo->videoPort, pSockInfo->ssrc, pSockInfo->mediaChannel, -1,1,0,0,0,NULL);
        return NULL;
    }

    if (pSockInfo->rtpType == 3)
    {
        if (1 == (pSockInfo->enOverTcp & 0x000F))/*低字节中的低4位为1代表是TCP方式*/
        {
            /*active\passive只有在createsocket时不同，发送数据时相同，不做区分*/
            printf("RecvAudioRtpTask tcp, fd:%d\n", pSockInfo->veRtpSock);
            retVal = RecvAudioOverTcp(pSockInfo);
        }
        else
        {
            printf("RecvAudioRtpTask udp, fd:%d\n", pSockInfo->veRtpSock);
            retVal = RecvAudioOverUdp(pSockInfo);
        }
        printf("RecvAudioRtpTask status [%d]\n", retVal);
        pSockInfo->isUsed = 0;

		if(retVal < 0)
		{
			rtp_stream_status_send(pSockInfo->rtpType, pSockInfo->remoteip, pSockInfo->videoPort, pSockInfo->ssrc, pSockInfo->mediaChannel, -1,3,0,0,0,NULL);
		}
    }
    return (void *)NULL;
}

int gk_rtp_playback_send(void *data, int size, GK_NET_FRAME_HEADER *header,int session_id,unsigned short *u16CSeq)
{
    int i;
    int retVal;
    unsigned char stream_id =0xE0 ;//0xE0 video     0xC0 audio
    static frameData_Info_s frameInfo_rtp;
    //static int isSendIframe = 1;
    static struct sockaddr_in veAddr;

	if(session_id != 0 && session_id != 1)
		return -1;

    isPlaybackSending[session_id] = 1;

#ifndef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD
    //get and init info
    if (rtp_playback_info[session_id] == NULL)
    {
        for (i = 0; i < MAX_CHANNEL_NUM; i++)
        {
            if (rtpInfo[i].isUsed && ((session_id == 0 && rtpInfo[i].rtpType == 1) ||
				(session_id == 1 && rtpInfo[i].rtpType == 2)))
            {
                rtp_playback_info[session_id] = &rtpInfo[i];
                //isSendIframe = 1;

                rtp_playback_info[session_id]->frameBufferSize = 70 * 1024;
                if (rtp_playback_info[session_id]->frameBuffer != NULL)
                {
                    free(rtp_playback_info[session_id]->frameBuffer);
                }
                if (rtp_playback_info[session_id]->frameBuffer == NULL)
                {
                    rtp_playback_info[session_id]->frameBuffer = (char*)malloc(rtp_playback_info[session_id]->frameBufferSize);
                    if (rtp_playback_info[session_id]->frameBuffer == NULL)
                    {
                        printf("malloc %d failed.\n", rtp_playback_info[session_id]->frameBufferSize);
                        isPlaybackSending[session_id] = 0;
                        return -1;
                    }
                }

                break;
            }
        }
    }
#endif

    if (rtp_playback_info[session_id] == NULL)
    {
        printf("not find playback socket info\n");
        isPlaybackSending[session_id] = 0;
        return -1;
    }

    veAddr.sin_family = AF_INET;
    veAddr.sin_addr.s_addr = inet_addr(rtp_playback_info[session_id]->remoteip);
    veAddr.sin_port = htons(rtp_playback_info[session_id]->videoPort);

    if (rtp_playback_info[session_id]->isRun == 0)
    {
        isPlaybackSending[session_id] = 0;
        return -1;
    }

    memset(&frameInfo_rtp, 0, sizeof(frameData_Info_s));
    frameInfo_rtp.u16CSeq = *u16CSeq;
#ifdef MODULE_SUPPORT_MOJING
    frameInfo_rtp.paddingFlg = 1;
#else
    frameInfo_rtp.paddingFlg = 0;
#endif

#if 0
    if (isSendIframe && (header->frame_type != M_FRAME_TYPE_I))
    {
        isPlaybackSending[session_id] = 0;
        return 0;
    }
    else
    {
        isSendIframe = 0;
    }
#endif

    //是否发送音频
    #if 0
    if (header->frame_type == M_FRAME_TYPE_AUDIO)
    {
        isPlaybackSending[session_id] = 0;
        return 0;
    }
    #endif


    if (header->frame_type == M_FRAME_TYPE_I)
    {
		printf("playback send type:%d, len:%d\n", header->frame_type, size);
        frameInfo_rtp.IFrame = 1;
    }
    else
    {
        frameInfo_rtp.IFrame = 0;
    }

    frameInfo_rtp.u32Ssrc = rtp_playback_info[session_id]->ssrc;
    frameInfo_rtp.u32CurPts = header->pts;

    if (header->frame_type == M_FRAME_TYPE_I || header->frame_type == M_FRAME_TYPE_P)
    {
        frameInfo_rtp.u32RtpPts = frameInfo_rtp.u32CurPts;
        stream_id = 0xE0;
    }
    else if (header->frame_type == M_FRAME_TYPE_AUDIO)
    {
        //音频rtp的pts在视频的基础上加1
        frameInfo_rtp.u32RtpPts++;
        stream_id =0xC0;
    }

    frameInfo_rtp.streamType = header->video_standard;
    frameInfo_rtp.u32Sec = header->sec;
    //printf("playback send type:%d, len:%d\n", header->frame_type, size);
    if (1 == (rtp_playback_info[session_id]->enOverTcp & 0x000F))/*低字节中的低4位为1代表是TCP方式*/
    {
        retVal = ite_RtpSendTcp_PS_H264frame(rtp_playback_info[session_id], data, size, &frameInfo_rtp, stream_id, rtp_playback_info[session_id]->veRtpSock, (struct sockaddr *)&veAddr);
    }
    else
    {
        retVal = ite_RtpSendUdp_PS_H264frame(rtp_playback_info[session_id], data, size, &frameInfo_rtp, stream_id, rtp_playback_info[session_id]->veRtpSock, (struct sockaddr *)&veAddr);
    }

	*u16CSeq = frameInfo_rtp.u16CSeq;
    isPlaybackSending[session_id] = 0;

    return retVal;
}

int gk_rtp_playback_close(ite_rtp_Info * pSockInfo,int session_id)
{
	if(session_id != 0 && session_id != 1)
		return -1;

    pSockInfo->isRun = 0;
    while(isPlaybackSending[session_id])
    {
        usleep(50000);
		printf("wait for playback %d send!\n",session_id);
    }

	rtpCloseHandle(pSockInfo);

    rtp_playback_info[session_id] = NULL;
	return 0;
}

int gk_rtp_play_close(ite_rtp_Info * pSockInfo)
{
    pSockInfo->isRun = 0;
    while(pSockInfo->isUsed == 1)
    {
        usleep(50000);
		printf("wait for play exit!\n");
    }

	return 0;
}


void SetIFameFlag(int flag)
{
    g_sendIframe = flag;
}

int rtpGetPushNum()
{
    int i, pushNum=0;
    for (i = 0; i < MAX_CHANNEL_NUM; i++)
    {
        if (rtpInfo[i].isRun && rtpInfo[i].isUsed)
        {
            pushNum++;
            printf("rtp pust No:%d,remoteip:%s,enOverTcp:%d, rtpType:%d\n", pushNum, rtpInfo[i].remoteip, rtpInfo[i].enOverTcp, rtpInfo[i].rtpType);
        }
    }
    return pushNum;
}

/*媒体控制：实时点播/回放/下载*/
/*control_type：媒体控制类型，实时点播/Play，回放/Playback，下载/Download*/
/*local_ip : 本地ip  设备ip */
/*remote_ip：媒体服务器IP地址*/
/*media_port：媒体服务器IP端口*/
/*返回值：成功时返回0，失败时返回负值*/

ite_rtp_Info *rtpGetHandle(char *controlType, char *media_server_ip,
                    int media_server_port, int ssrc)
{
    int i;
    char rtpType = 0;
    int playCnt = 0;

    if (0 == strcmp(controlType, "Play"))
    {
        rtpType = 0;
    }
    /*下载*/
    else if (0 == strcmp(controlType, "Download"))
    {
        rtpType = 1;

    }
    /*回放*/
    else if (0 == strcmp(controlType, "Playback"))
    {
        rtpType = 2;
    }
    /*语音对讲*/
    else if (0 == strcmp(controlType, "audio"))
    {
        rtpType = 3;
    }
    /*云存*/
    else if (0 == strcmp(controlType, "CloudStorage"))
    {
        rtpType = 4;
    }
#ifdef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD
    pthread_mutex_lock(&rtphandleMutex);
#endif

    for (i = 0; i < MAX_CHANNEL_NUM; i++)
    {
        if (rtpType == rtpInfo[i].rtpType && rtpInfo[i].isUsed)
        {
            if (rtpInfo[i].videoPort == media_server_port && rtpInfo[i].ssrc == ssrc
                && strcmp(rtpInfo[i].remoteip, media_server_ip) == 0)
            {
                printf("conect:%s:%d, ssrc:%d is connected.\n", media_server_ip, media_server_port, ssrc);
#ifdef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD
				pthread_mutex_unlock(&rtphandleMutex);
#endif
                return NULL;
            }
        }
    }

    for (i = 0; i < MAX_CHANNEL_NUM; i++)
    {
#ifndef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD
        if (rtpType == 1 && rtpInfo[i].isUsed && rtpInfo[i].rtpType == 1)
        {
            //only open one download
            gk_rtp_playback_close(&rtpInfo[i],0);
        }
        else if (rtpType == 2 && rtpInfo[i].isUsed && rtpInfo[i].rtpType == 2)
        {
            //only open one playback
            gk_rtp_playback_close(&rtpInfo[i],1);
        }
#endif
        if (rtpType == 0 && rtpInfo[i].isUsed && rtpInfo[i].rtpType == 0)
        {
            playCnt++;
            if (playCnt >= 3)
            {
                gk_rtp_play_close(&rtpInfo[i]);
            }
        }
        if (!rtpInfo[i].isUsed)
        {
            rtpInfo[i].isUsed = 1;
            rtpInfo[i].isRun = 1;
            rtpInfo[i].rtpType = rtpType;
#ifdef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD
			pthread_mutex_unlock(&rtphandleMutex);
#endif
            return &rtpInfo[i];
        }
    }

#ifdef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD
    pthread_mutex_unlock(&rtphandleMutex);
#endif

    return NULL;
}

int rtpCloseHandle(ite_rtp_Info *rtpInfo)
{
	if(!rtpInfo)
		return 1;

#ifdef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD
    pthread_mutex_lock(&rtphandleMutex);
#endif

    if(rtpInfo->veRtpSock > -1)
    {
        close(rtpInfo->veRtpSock );
    }
    if(rtpInfo->veRtcpSock > -1)
    {
        close(rtpInfo->veRtcpSock );
    }
    if(rtpInfo->auRtpSock > -1)
    {
        close(rtpInfo->auRtpSock );
    }
    if(rtpInfo->auRtcpSock > -1)
    {
        close(rtpInfo->auRtcpSock );
    }
    rtpInfo->veRtpSock = -1;
    rtpInfo->auRtpSock = -1;
    rtpInfo->veRtcpSock = -1;
    rtpInfo->auRtcpSock = -1;
    if (rtpInfo->frameBuffer != NULL)
    {
        free(rtpInfo->frameBuffer);
        rtpInfo->frameBuffer = NULL;
    }

    rtpInfo->isUsed = 0;

#ifdef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD
    pthread_mutex_unlock(&rtphandleMutex);
#endif

    return 1;
}



int rtpInit(void)
{
    memset(rtpInfo, 0, sizeof(ite_rtp_Info) * MAX_CHANNEL_NUM);
}

int ProcessInviteMediaPlay(char *control_type, char *media_fromat, char *local_ip, char *media_server_ip,
                    char *media_server_port, unsigned int enOverTcp, int channleIndex, int ssrc, int eventId)
{
    int ret = 0;
    pthread_t thread_Rtp = -1;
    ite_rtp_Info* rtpHandle = rtpGetHandle(control_type, media_server_ip, atoi(media_server_port), ssrc);

    if (rtpHandle == NULL)
    {
        printf("not enough handle or download thread is running\n");
        return -1;
    }

    strcpy(rtpHandle->localip, local_ip);
    strcpy(rtpHandle->remoteip, media_server_ip);
    rtpHandle->videoPort  = atoi(media_server_port);
    rtpHandle->enOverTcp = enOverTcp;
    rtpHandle->playLoadType = 98; //= atoi(media_fromat);
    rtpHandle->mediaChannel = channleIndex;
    rtpHandle->ssrc = ssrc;
    rtpHandle->isRun = 1;
    rtpHandle->eventId = eventId;
    printf("Invite  status  create SendVideoRtpTask  mediaformat[%d], enOverTcp:0x%04x, server:%s, port:%d\n",
        rtpHandle->playLoadType, rtpHandle->enOverTcp, rtpHandle->remoteip, rtpHandle->videoPort);

    ret = pthread_create(&thread_Rtp, NULL, SendVideoRtpTask, rtpHandle);
    if (ret || thread_Rtp <= 0)
    {
        ret = -1;
    }

    return ret;
}


int ProcessInviteAudioPlay(char *control_type, char *media_fromat, char *local_ip, char *media_server_ip,
                    char *media_server_port, unsigned int enOverTcp, int channleIndex, int ssrc, int eventId)
{
    int ret = 0;
    pthread_t thread_Rtp = -1;
    ite_rtp_Info* rtpHandle = rtpGetHandle(control_type, media_server_ip, atoi(media_server_port), ssrc);

    if (rtpHandle == NULL)
    {
        printf("not enough handle for audio play!\n");
        return -1;
    }

    strcpy(rtpHandle->localip, local_ip);
    strcpy(rtpHandle->remoteip, media_server_ip);
    rtpHandle->videoPort  = atoi(media_server_port);
    rtpHandle->audioPort  = atoi(media_server_port);
    rtpHandle->enOverTcp = enOverTcp;
    rtpHandle->playLoadType = 8; //= atoi(media_fromat);
    rtpHandle->mediaChannel = channleIndex;
    rtpHandle->ssrc = ssrc;
    rtpHandle->isRun = 1;
    rtpHandle->eventId = eventId;
    printf("ProcessInviteAudioPlay  mediaformat[%d], enOverTcp:0x%04x, server:%s, port:%d, ssrc:%d\n",
        rtpHandle->playLoadType, rtpHandle->enOverTcp, rtpHandle->remoteip, rtpHandle->audioPort,ssrc);

    ret = pthread_create(&thread_Rtp, NULL, RecvAudioRtpTask, rtpHandle);
    if (ret || thread_Rtp <= 0)
    {
        ret = -1;
    }

    return ret;
}

#ifdef MODULE_SUPPORT_MOJING
static int mojing_cloud_storage_send_over_tcp(ite_rtp_Info *pSockInfo)
{
    int frameLen = 0;
    int veRtpFd = pSockInfo->veRtpSock;
    char *pH264frame = NULL;
    int isSendIframe = 1;
    struct sockaddr_in veAddr;
    int frameType = 0, isSend = 0;
    int retVal = 0;
    char alaw_data[4096];
    int alaw_len = 0;
    unsigned char stream_id =0xE0 ;//0xE0 video     0xC0 audio
    frameData_Info_s frameInfo_rtp;
	int readLen = 0;
	unsigned int first_timetick = 0,cur_timetick = 0;
	GK_NET_FRAME_HEADER header = {0};
	int diff_time = 0;
    unsigned int curPts = 0;
    unsigned int onePts = 0;
	struct timeval rcto = {12,0};

	int gopInterval = 2;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    retVal = netcam_video_get(0, pSockInfo->mediaChannel, &stH264Config);
    if (retVal != 0)
    {
        printf("mojing_cloud_storage_send_over_tcp: get video parameters failed.\n");
    }
	else
	{
		if(stH264Config.fps)
		{
			gopInterval = stH264Config.gop / stH264Config.fps;
			printf("gopInterval = %d.\n",gopInterval);
		}
	}

    onePts = 90000/stH264Config.fps;

    MEDIABUF_HANDLE handle = mediabuf_add_reader(pSockInfo->mediaChannel);

    if(handle)
        mediabuf_set_oldest_frame(handle);
    else
    {
        printf("mojing_cloud_storage_send_over_tcp add reader error.\n");
		retVal = -1;
		goto SEND_END;
    }

    pSockInfo->frameBufferSize = 70 * 1024;
    if (pSockInfo->frameBuffer == NULL)
    {
        pSockInfo->frameBuffer = (char*)malloc(pSockInfo->frameBufferSize);
        if (pSockInfo->frameBuffer == NULL)
        {
            printf("mojing_cloud_storage_send_over_tcp malloc %d failed.\n", pSockInfo->frameBufferSize);
			retVal = -1;
			goto SEND_END;
        }
    }

    veAddr.sin_family = AF_INET;
    veAddr.sin_addr.s_addr = inet_addr(pSockInfo->remoteip);
    veAddr.sin_port = htons(pSockInfo->videoPort);
    memset(&frameInfo_rtp, 0, sizeof(frameData_Info_s));
    frameInfo_rtp.u16CSeq = 1;
    frameInfo_rtp.paddingFlg = 1;
    
    if (stH264Config.enctype == 1)
    {
        frameInfo_rtp.streamType = 0x1b;
    }
    else if (stH264Config.enctype == 3)
    {
        frameInfo_rtp.streamType = 0x24;
    }
    else
    {
        frameInfo_rtp.streamType = 0x1b;
    }

	setsockopt(veRtpFd, SOL_SOCKET, SO_SNDTIMEO,  (void *)&rcto,sizeof(struct timeval));

	if(pSockInfo->cloudCtrl.transType == 2)
	{
		if(pSockInfo->cloudCtrl.tls_init_cb)
			pSockInfo->cloudCtrl.ssl = pSockInfo->cloudCtrl.tls_init_cb(veRtpFd,pSockInfo->remoteip,pSockInfo->videoPort);
	}
    while (pSockInfo->isRun)
    {
		if(netcam_is_prepare_update() || (netcam_get_update_status() != 0))
		{
			printf("\n#########stop cloud storage when upgrade!\n");
			break;
		}

		if(pSockInfo->cloudCtrl.status_check_cb)
		{
			if(pSockInfo->cloudCtrl.status_check_cb(pSockInfo->cloudCtrl.cloudStorageType,&pSockInfo->cloudCtrl.motionEndTime))
				break;
		}

		memset(&header,0,sizeof(GK_NET_FRAME_HEADER));
        pH264frame = NULL;
        frameLen = 0;

		readLen = mediabuf_read_frame(handle, (void **)&pH264frame, &frameLen, &header);
		if(readLen <= 0 || frameLen <= 0)
		{
            usleep(10000);
            continue;
		}

		frameType = header.frame_type;
		frameInfo_rtp.u32CurPts = header.pts;
        frameInfo_rtp.u32Ssrc = pSockInfo->ssrc;
        frameInfo_rtp.u32Sec = header.sec;//time(NULL);

        if (isSendIframe && (frameType != M_FRAME_TYPE_I))
        {
            continue;
        }
        else
        {
            isSendIframe = 0;
        }

        #if 1
        if (header.frame_type == M_FRAME_TYPE_I || header.frame_type == M_FRAME_TYPE_P)
        {
            #if 0
            frameInfo_rtp.u32RtpPts = frameInfo_rtp.u32CurPts;
            frameInfo_rtp.u32CurPts = 0;
            #else
            curPts += onePts;
            frameInfo_rtp.u32RtpPts = curPts;
            frameInfo_rtp.u32CurPts = curPts;
            #endif
        }
        else if (header.frame_type == M_FRAME_TYPE_AUDIO)
        {
            //音频rtp的pts在视频的基础上加1
            frameInfo_rtp.u32RtpPts++;
        }
        #endif

        stream_id = 0xE0;
        cur_timetick = header.sec;
        if (frameType == M_FRAME_TYPE_I || frameType == M_FRAME_TYPE_P)
        {
            if(pSockInfo->cloudCtrl.cloudStorageType == CLOUD_STORAGE_DYNAMIC)
    		{
    			if(cur_timetick > pSockInfo->cloudCtrl.motionEndTime)
    			{
    				printf("mojing_cloud_storage_send_over_tcp, cur_timetick:%d, motionEndTime:%d! len:%ds\n",
                        cur_timetick,pSockInfo->cloudCtrl.motionEndTime, cur_timetick - first_timetick);
    				break;
    			}
    		}
        }
        if (frameType == M_FRAME_TYPE_AUDIO)
        {
            continue;
            stream_id = 0xC0;
            alaw_len = audio_alaw_encode((unsigned char*)alaw_data, (short*)pH264frame, frameLen);
            //printf("-->send audio len:%d,%d pts:%d\n", frameLen, alaw_len, frameInfo_rtp.u32CurPts);

            retVal = ite_RtpSendTcp_PS_H264frame(pSockInfo, alaw_data, alaw_len, &frameInfo_rtp, stream_id, veRtpFd, (struct sockaddr *)&veAddr);
            if (retVal < 0)
            {
                break;
            }
        }
        else if (frameType == M_FRAME_TYPE_I)
        {

			if(pSockInfo->cloudCtrl.cloudStorageType == CLOUD_STORAGE_DYNAMIC)
			{
				if(!isSend)
				{
					diff_time = pSockInfo->cloudCtrl.motionStartTime - header.sec;

		            printf("header:%d,motionStartTime:%d,diff:%d\n", header.sec, pSockInfo->cloudCtrl.motionStartTime, diff_time);
					if((diff_time - pSockInfo->cloudCtrl.preStorageTimeSec) > (gopInterval - 1))
					{
						continue;
					}
				}
			}
			
			if(!isSend)
			{
				first_timetick = header.sec;

				if(cur_timetick < 1577808000 || (pSockInfo->cloudCtrl.motionEndTime < 1577808000 && pSockInfo->cloudCtrl.cloudStorageType == CLOUD_STORAGE_DYNAMIC))//20200101000000
				{
					printf("mojing_cloud_storage_send_over_tcp, time not sync: cur_timetick:%d, motionEndTime:%d \n",
						cur_timetick,pSockInfo->cloudCtrl.motionEndTime);
									
					break;
				}
			}

            frameInfo_rtp.IFrame = 1;
            retVal = ite_RtpSendTcp_PS_H264frame(pSockInfo, pH264frame, frameLen, &frameInfo_rtp, stream_id, veRtpFd, (struct sockaddr *)&veAddr);
            if (retVal < 0)
            {
                break;
            }

            isSend = 1;
			cur_timetick = header.sec;
        }
        else if (!isSend)
        {
            continue;
        }
        else
        {
            frameInfo_rtp.IFrame = 0;

            retVal = ite_RtpSendTcp_PS_H264frame(pSockInfo, pH264frame, frameLen, &frameInfo_rtp, stream_id, veRtpFd, (struct sockaddr *)&veAddr); // RtpSendOverUdpPkt3984(veRtpFd, p, len, (struct sockaddr *)&veAddr, &veRtpInfo);
            if (retVal < 0)
            {
                break;
            }

			cur_timetick = header.sec;
        }
    }

SEND_END:

    mediabuf_del_reader(handle);
	rtpCloseHandle(pSockInfo);
	pSockInfo->isRun = 0;

    #ifdef TS_TEST_SERVER_ALL
    close(pSockInfo->selfCloudTestFd);
    #endif

	if(pSockInfo->cloudCtrl.notify_end_cb)
		pSockInfo->cloudCtrl.notify_end_cb(pSockInfo->cloudCtrl.cloudStorageType,pSockInfo->cloudCtrl.taskID,first_timetick,cur_timetick);

	if(pSockInfo->cloudCtrl.transType == 2)
	{
		if(pSockInfo->cloudCtrl.ssl)
			pSockInfo->cloudCtrl.tls_exit_cb(pSockInfo->cloudCtrl.ssl);
	}

	printf("mojing_cloud_storage_send_over_tcp send end,ret = %d.\n",retVal);

	if(!retVal)
	{
		cloudSuccessCnt++;
		MjEvtcloudSuccessCnt++;
	}

	rtp_stream_status_send(pSockInfo->rtpType, pSockInfo->remoteip, pSockInfo->videoPort, pSockInfo->ssrc, pSockInfo->mediaChannel, retVal,0,first_timetick,cur_timetick,cur_timetick - first_timetick,NULL);


    return (retVal < 0 ? retVal : 0);
}

static void *mojing_cloud_storage_send_thread(void *pData)
{
    int retVal = -1;
    ite_rtp_Info *pSockInfo = (ite_rtp_Info *)pData;
    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("cloud_storage_send");

	cloudSendCnt++;
	MjEvtcloudSendCnt++;

    retVal = CreateSockChannel(pSockInfo);
    if (retVal != 0)
    {
        rtpCloseHandle(pSockInfo);

		if(pSockInfo->cloudCtrl.notify_end_cb)
			pSockInfo->cloudCtrl.notify_end_cb(pSockInfo->cloudCtrl.cloudStorageType,pSockInfo->cloudCtrl.taskID,time(NULL),time(NULL));

		printf("mojing_cloud_storage_send_thread create sock failed!\n");

		rtp_stream_status_send(pSockInfo->rtpType, pSockInfo->remoteip, pSockInfo->videoPort, pSockInfo->ssrc, pSockInfo->mediaChannel, -1,1,0,0,0,NULL);
        return NULL;
    }
/*
#ifdef TS_TEST_SERVER_ALL
    if (tsTestFd != -1)
    {
        retVal = send(tsTestFd, runGB28181Cfg.DeviceUID, 19, 0);
        printf("test send id:%s, fd:%d, ret = %d\n", runGB28181Cfg.DeviceUID, tsTestFd, retVal);
    }
#endif
*/

#ifdef TS_TEST_GOKE
    if (strcmp(pSockInfo->remoteip, tsTestIp) == 0)
    {
        retVal = send(pSockInfo->veRtpSock, runGB28181Cfg.DeviceUID, 19, 0);
        printf("test send id:%s, ret = %d\n", runGB28181Cfg.DeviceUID, retVal);
    }
#endif

	rtp_stream_status_send(pSockInfo->rtpType, pSockInfo->remoteip, pSockInfo->videoPort, pSockInfo->ssrc, pSockInfo->mediaChannel, 0,1,0,0,0,NULL);

    retVal = mojing_cloud_storage_send_over_tcp(pSockInfo);

    return NULL;
}

static int get_local_ip(char *local_ip)
{
	int ret = 0;
	char wifi_name[20] = {0};
	ST_SDK_NETWORK_ATTR  net_attr_local,net_attr_wifi;

	if(!local_ip)
		return -1;

	ret = netcam_net_get_detect("eth0");
	if(0 == ret)
	{
		memset(&net_attr_local, 0, sizeof(ST_SDK_NETWORK_ATTR));
		strncpy(net_attr_local.name, "eth0", sizeof(net_attr_local.name)-1);
		ret = netcam_net_get(&net_attr_local);
		if(ret == 0)
		{
			strcpy(local_ip,net_attr_local.ip);
			return 0;
		}
	}

	if(netcam_net_wifi_get_devname() != NULL)
	{
		strcpy(wifi_name, netcam_net_wifi_get_devname());

		if((ret = netcam_net_get_detect(wifi_name)))
		{
			printf("Detect WIFI_DEV%s error!\n", wifi_name);
			return -1;
		}

		memset(&net_attr_wifi, 0, sizeof(ST_SDK_NETWORK_ATTR));
		strncpy(net_attr_wifi.name, wifi_name, sizeof(net_attr_wifi.name)-1);
		ret = netcam_net_get(&net_attr_wifi);
		if(ret == 0)
		{
			strcpy(local_ip,net_attr_wifi.ip);
			return 0;
		}
	}

	return -1;
}

int mojing_cloud_storage_start(char *media_server_ip,int media_server_port, unsigned int trans_type,
										int ssrc, CloudStorageControlT *cloudInfo)
{
	int ret = 0;
	pthread_t thread_Rtp = -1;
	char local_ip[64];
	ite_rtp_Info* rtpHandle = NULL;

	if(trans_type != 0 && trans_type != 2)
	{
		printf("trans_type = %d,not support!\n",trans_type);
		return -1;
	}

	memset(local_ip,0,sizeof(local_ip));
	if(get_local_ip(local_ip) < 0)
	{
		printf("mojing_cloud_storage_start get_local_ip failed!\n");
		return -1;
	}

	rtpHandle = rtpGetHandle("CloudStorage", media_server_ip, media_server_port, ssrc);
	if (rtpHandle == NULL)
	{
		printf("not enough handle for cloud storage!\n");
		return -1;
	}

	strcpy(rtpHandle->localip, local_ip);
	strcpy(rtpHandle->remoteip, media_server_ip);
	rtpHandle->videoPort  = media_server_port;
	rtpHandle->enOverTcp = 0x21;
	rtpHandle->playLoadType = 98;
	rtpHandle->ssrc = ssrc;
	rtpHandle->isRun = 1;
	rtpHandle->eventId = 0;

	memset(&rtpHandle->cloudCtrl,0,sizeof(CloudStorageControlT));

	rtpHandle->cloudCtrl.transType = trans_type;

	if(cloudInfo)
	{
		rtpHandle->mediaChannel = cloudInfo->streamType;
		rtpHandle->cloudCtrl.preStorageTimeSec = cloudInfo->preStorageTimeSec;
		rtpHandle->cloudCtrl.keepTimeSec = cloudInfo->keepTimeSec;
		rtpHandle->cloudCtrl.cloudStorageType = cloudInfo->cloudStorageType;
		rtpHandle->cloudCtrl.motionStartTime = cloudInfo->motionStartTime;
		rtpHandle->cloudCtrl.motionEndTime = cloudInfo->motionEndTime;

		rtpHandle->cloudCtrl.notify_end_cb = cloudInfo->notify_end_cb;
		rtpHandle->cloudCtrl.status_check_cb = cloudInfo->status_check_cb;
		rtpHandle->cloudCtrl.tls_init_cb = cloudInfo->tls_init_cb;
		rtpHandle->cloudCtrl.tls_send_cb = cloudInfo->tls_send_cb;
		rtpHandle->cloudCtrl.tls_exit_cb = cloudInfo->tls_exit_cb;

		memset(rtpHandle->cloudCtrl.taskID,0,sizeof(rtpHandle->cloudCtrl.taskID));
		if(strlen(cloudInfo->taskID))
		{
			strncpy(rtpHandle->cloudCtrl.taskID,cloudInfo->taskID,sizeof(rtpHandle->cloudCtrl.taskID));
		}
	}
	else
	{
		rtpHandle->mediaChannel = 0;
	}

	printf("mojing_cloud_storage_start local:%s, server:%s, port:%d\n",local_ip,rtpHandle->remoteip, rtpHandle->videoPort);

	ret = pthread_create(&thread_Rtp, NULL, mojing_cloud_storage_send_thread, rtpHandle);
	if (ret || thread_Rtp <= 0)
	{
		rtpHandle->isUsed = 0;
		rtpHandle->isRun = 0;
		rtpHandle->rtpType = 0;
		ret = -1;
	}

	return ret;
}

void mojing_cloud_storage_get_send_status(unsigned int *totalCnt, unsigned int *successCnt)
{

	if(!totalCnt || !successCnt)
		return;

	*totalCnt = cloudSendCnt;
	*successCnt = cloudSuccessCnt;
}

#endif

static int ProcessMediaStop(int eventdid, char rtpType)
{
    int i;
    for (i = 0; i < MAX_CHANNEL_NUM; i++)
    {
        if (rtpInfo[i].isUsed && eventdid == rtpInfo[i].eventId && rtpInfo[i].rtpType == rtpType)
        {
            printf("ProcessMediaStop,eventdid:%d,rtpType:%d\n",eventdid, rtpType);
            rtpInfo[i].isRun = 0;
            return 1;
        }
    }
    return 0;
}

int ProcessPlayStop(int eventdid)
{
    return ProcessMediaStop(eventdid, 0);
}

int ProcessVoiceStop(int eventdid)
{
    return ProcessMediaStop(eventdid, 3);
}


int ProcessMediaStopAll(void)
{
    int i;
    for (i = 0; i < MAX_CHANNEL_NUM; i++)
    {
        if (rtpInfo[i].isUsed && rtpInfo[i].rtpType != 4)
        {
            rtpInfo[i].isRun = 0;
        }
    }
    return 0;
}


/*
   在portPool中获取未使用的UDP端口
*/
static int getUdpPort(unsigned short *port)
{
    int i, j;

    pthread_mutex_lock(&portPoolMutex);
    for (i = 0; i < (MAX_UDP_PORT_NUM / 8); i++)
    {
        for (j = 0; j < 8; j++)
        {
            if (!(portPool[i] & (1 << j)))
            {
                portPool[i] |= (1 << j);
                *port = UDP_PORT_BASE + i * 8 + j * 2;
                pthread_mutex_unlock(&portPoolMutex);
                return 0;
            }
        }
    }
    pthread_mutex_unlock(&portPoolMutex);
    return -1;
}

/*
    把已使用的UDP端口在portPool标注
*/
static int putUdpPort(unsigned short port)
{
    unsigned short poolPort = (port - UDP_PORT_BASE) >> 1;

    if (poolPort > (MAX_UDP_PORT_NUM - 1))
    {
        return -1;
    }

    pthread_mutex_lock(&portPoolMutex);
    portPool[poolPort / 8] ^= (1 << (poolPort % 8));
    pthread_mutex_unlock(&portPoolMutex);

    return 0;
}

/*
    创建RTP传输SOCKET  针对UDP
*/
int CreateUdpSocket(ite_rtp_Info *pSocketInfo)
{
    int rtpFd = -1, rtcpFd = -1, i;
    int ret1 = -1, ret2 = -1;
    unsigned short port = 0, nextPort = 0;
    unsigned short videoSocketInit = 0, audioSocketInit = 0;
    struct sockaddr_in addr;
    int opt = 1;
    int len = sizeof(opt);

nextSocket:
    if ((rtpFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return -1;
    }
    if ((rtcpFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        close(rtpFd);
        return -1;
    }

    #if 0
    getUdpPort(&port);
    //port = 9898;
    for (i = 0; i < MAX_UDP_PORT_NUM; i++)
    {
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(pSocketInfo->localip);
        addr.sin_port = htons(port);
        setsockopt(rtpFd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
        ret1 = bind(rtpFd, (struct sockaddr *)&addr, sizeof(addr));

        addr.sin_port = htons(port + 1);
        setsockopt(rtcpFd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
        ret2 = bind(rtcpFd, (struct sockaddr *)&addr, sizeof(addr));
        if(ret1 == 0 && ret2 == 0)
        {
            break;
        }
        getUdpPort(&nextPort);
        putUdpPort(port);
        port = nextPort;
    }

    if (i >= MAX_UDP_PORT_NUM)
    {
        if (rtpFd > 0)
        {
            close(rtpFd);
        }
        if (rtcpFd > 0)
        {
            close(rtcpFd);
        }
        return -1;
    }
    #endif

    if (videoSocketInit == 0)
    {
        printf("video rtpsock:%d, auport:%d\n", rtpFd, port);
        pSocketInfo->veRtpSock = rtpFd;
        pSocketInfo->veRtcpSock = rtcpFd;
        videoSocketInit = 1;
        goto nextSocket;
    }
    else if (audioSocketInit == 0)
    {
        printf("audio rtpsock:%d, auport:%d\n", rtpFd, port);
        pSocketInfo->auRtpSock = rtpFd;
        pSocketInfo->auRtcpSock = rtcpFd;
    }
    return 0;
}

/*
    创建RTP传输SOCKET  针对TCP
*/
int CreateTcpSocket(ite_rtp_Info *pSocketInfo)
{
    int sockFd, i;
    int ret1 = -1;
    unsigned short port = 0, nextPort = 0;
    struct sockaddr_in addr;
    int opt = 1;
    int len = sizeof(opt);

    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
#if 0
    getUdpPort(&port);
    //port = 9998;
    for (i = 0; i < MAX_UDP_PORT_NUM; i++)
    {
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(pSocketInfo->localip);
        addr.sin_port = htons(port);
        setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
        ret1 = bind(sockFd, (struct sockaddr *)&addr, sizeof(addr));
        if(ret1 == 0)
        {
            printf("bind ip:%s, port:%d\n", pSocketInfo->localip, port);
            break;
        }
        getUdpPort(&nextPort);
        putUdpPort(port);
        port = nextPort;
    }

    if (i >= MAX_UDP_PORT_NUM)
    {
        if (sockFd > 0)
        {
            close(sockFd);
        }
        return -1;
    }
#endif
    pSocketInfo->veRtpSock = sockFd;
    pSocketInfo->auRtpSock = sockFd;
    #ifdef TS_TEST_SERVER_ALL
    if ((pSocketInfo->selfCloudTestFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create test fd failed.\n");
    }
    #endif
    return 0;
}

int gk_rtp_playback_close_rtp_channel(int session_id)
{
#ifdef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD

	if((session_id != 0 && session_id != 1) || !rtp_playback_info[session_id])
		return -1;

    rtp_playback_info[session_id]->isRun = 0;
	rtpCloseHandle(rtp_playback_info[session_id]);
    rtp_playback_info[session_id] = NULL;
#endif

	return 0;
}

int gk_rtp_playback_open_rtp_channel(int session_id,char *local_ip, char *media_server_ip,
                    int port, unsigned int enOverTcp, int ssrc, int eventId)
{
#ifdef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD

	int ret = 0;
	int i = 0;

	struct timeval rcto = {25,0};

	if((session_id != 0 && session_id != 1) || !local_ip || !media_server_ip || !port)
		return -1;

	if(!strlen(local_ip) || !strlen(media_server_ip))
		return -1;

    if(rtp_playback_info[session_id])
		gk_rtp_playback_close_rtp_channel(rtp_playback_info[session_id]);

	if(session_id == 0)
		rtp_playback_info[session_id] = rtpGetHandle("Download", media_server_ip, port, ssrc);
	else if(session_id == 1)
		rtp_playback_info[session_id] = rtpGetHandle("Playback", media_server_ip, port, ssrc);

	if(!rtp_playback_info[session_id])
	{
		printf("gk_rtp_playback_open_rtp_channel session %d rtpGetHandle failed.\n", session_id);
		return -1;
	}

    strcpy(rtp_playback_info[session_id]->localip, local_ip);
    strcpy(rtp_playback_info[session_id]->remoteip, media_server_ip);
    rtp_playback_info[session_id]->videoPort  = port;
    rtp_playback_info[session_id]->enOverTcp = enOverTcp;
    rtp_playback_info[session_id]->playLoadType = 98;
    rtp_playback_info[session_id]->ssrc = ssrc;
    rtp_playback_info[session_id]->isRun = 1;
    rtp_playback_info[session_id]->eventId = eventId;

	rtp_playback_info[session_id]->frameBufferSize = 128 * 1024;

	if(rtp_playback_info[session_id]->frameBuffer)
	{
		free(rtp_playback_info[session_id]->frameBuffer);
		rtp_playback_info[session_id]->frameBuffer = NULL;
	}

	rtp_playback_info[session_id]->frameBuffer = (char*)malloc(rtp_playback_info[session_id]->frameBufferSize);
	if(!rtp_playback_info[session_id]->frameBuffer)
	{
		printf("gk_rtp_playback_open_rtp_channel session %d malloc %d failed.\n", session_id,rtp_playback_info[session_id]->frameBufferSize);
		rtp_playback_info[session_id]->isRun = 0;
		rtp_playback_info[session_id]->isUsed= 0;
		isPlaybackSending[session_id] = 0;
		rtp_playback_info[session_id] = NULL;
		return -1;
	}

    ret = CreateSockChannel(rtp_playback_info[session_id]);
    if (ret != 0)
    {
        printf("gk_rtp_playback_open_rtp_channel session %d creat sock failed:%d\n", session_id, ret);
		rtp_playback_info[session_id]->isRun = 0;
        rtpCloseHandle(rtp_playback_info[session_id]);
		rtp_playback_info[session_id] = NULL;
        return -1;
    }

	setsockopt(rtp_playback_info[session_id]->veRtpSock, SOL_SOCKET,SO_SNDTIMEO,  (void *)&rcto,sizeof(struct timeval));

#endif

	return 0;
}

