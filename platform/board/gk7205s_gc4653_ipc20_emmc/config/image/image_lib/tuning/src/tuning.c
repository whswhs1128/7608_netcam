#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <assert.h>
#include <getopt.h>
#include <sched.h>
#include <signal.h>
#include <basetypes.h>
#include "gk_drv.h"
#include "gk_debug.h"
#include "gk_vi.h"
#include "gk_media.h"
#include "gk_isp.h"
#include "gk_isp3a_api.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <netinet/in.h>
#include <pthread.h>

#include "adi_types.h"
#include "adi_sys.h"
#include "adi_venc.h"
#include "adi_isp.h"
#include "tuning.h"
#include "gk_isp3a_img_struct.h"
#include "gk_isp3a_dsp_api.h"
#include "gk_debug.h"


// IR CUT
#define H264TUNING 1

#ifdef  H264TUNING
#include "adi_vi.h"
//#include "../../../../../applications/ctlserver/src/image.h"
//#include "../../../../../applications/ctlserver/src/video.h"
//extern GADI_ERR  gapp_image_set_ir_curt(GADI_BOOL isAuto,GADI_BOOL isDay);
//extern GADI_BOOL gapp_image_get_ir_curt_control_mode(void);
//extern GADI_BOOL gapp_image_get_ir_curt_night_status(void);
//extern GADI_SYS_HandleT gadi_venc_get_handle(void)

// Bin file function
extern GADI_ERR save_bin_sensor_param(Isp3AProcessor *processor, GADI_CHAR *binfile,GADI_S32 size,
    image_sensor_param_t * sensor_param,
    u8 WDRGet,
    u8 *matrix1,
    u8 *matrix2,
    u8 *matrix3,
    u8 *matrix4);

extern void gadi_sensors_set_bin(Isp3AProcessor *processor, char *bin);

extern GADI_ERR bin_sensor_param(Isp3AProcessor *processor, GADI_CHAR *binfile,GADI_S32 size,
    image_sensor_param_t * sensor_param);
extern GADI_ERR gadi_priv_get_framerate(u32 fpsTime);
extern GADI_ERR gadi_priv_driver_get_fd(int *fd);
extern int sensors_size_get(Isp3AProcessor *processor);
extern void *gadi_sensors_get_bin(Isp3AProcessor *processor);
extern Isp3AProcessor *monitor_handle_find_by_id(int id);
extern void blf_template_update(int* simr, int* simr_mv, int* sims, int* sims_mv,
                        bilateral_filter_t* blf_info);
extern GADI_BOOL gadi_isp_get_day_night_mode(GADI_SYS_HandleT ispHandle);

#endif

#define IMGPROC_PARAM_PATH      "/etc/icore"
#define MAX_YUV_BUFFER_SIZE     (720*480)
#define NUM_AAAINFO             1288
#define MAX_DUMP_BUFFER_SIZE    64*1024
#define IMAGE_MEDIA_DEV         "/dev/gk_video"
#define MAX_ZOOM_OF_MAIN_BUFFER (1100)      // 11X
#define MAX_ZOOM_OF_STEPS       (10)
#define MAX_ZOOM_OF_PREVIEW_A_B (500)       // 5X
#define TUNING_VENC_MODE_NORMAL      (0)
#define TUNING_VENC_MODE_HIGH_MEGA   (1)
#define TUNING_VENC_MODE_LOW_DELAY   (2)

#define msleep(x) usleep((x) * 1000)

Isp3AProcessor *handle;

// Add VPS by LYX on 20160725
#define ISP_CONFIG  0
//#define _printf(...)
#define _printf(...) printf(__VA_ARGS__)
#define TUNINGKEY 456789
#define CHIPPIN  99
static int KEYMACTH = 0;

static int closePreview = 1;
static int fdmedia      = 0;
static adj_param_t* bin_adj_param = NULL;
u8 aal_filter_strength;
u32 uv_scale;
static u32 shutter_index, gain_index;
static int flicker_mode = 50;
static int detect_flag  = 0;
static int correct_flag = 0;
static int restore_flag = 0;
static int compensate_ratio = 0;
static int detect_finished = 0;
static int les_shading_enable = 0;

#define LENS_SHADING_CALI_FILENAME   "/usr/local/bin/sensors/lens_shading.bin"
static const char *default_filename = LENS_SHADING_CALI_FILENAME;
static char filename[256];

static int ae_target = 1024;
static blc_level_t blc;
static ISP_FILTER_CFA_NoiseSetupT cfa_denoise_filter;
static dbp_correction_t bp_corr_param;
static wb_gain_t wb_gain;
static u16 dgain;
static chroma_median_filter_t color_filter_config;
//static video_dnr3d_info_t dnr3d_info_weak;
//static video_dnr3d_info_t dnr3d_info_strong;

//static mblk_denoise_params_t mblk_denoise;      // by yke
//static u8 demsaic_noise_reduction_strength;
static rgb_to_yuv_t rgb2yuv_matrix;
//static sharpen_level_t sharpen_setting_min;
//static sharpen_level_t sharpen_setting_overall;
static digital_sat_level_t isp_gain_satuation_level;
static dbp_correction_t dbp_correction_config;
static ISP_StatisticsConfigT gk_3a_statistic_config;
static ISP_StatisticsSetupT af_statistic_config_ex;

static aaa_cntl_t aaa_cntl_station;
static ISP_RgbStatT gk_rgb_stat;
static ISP_CfaStatT gk_cfa_stat;
//static image_mode mode;
static fpn_correction_t fpn;
static cali_badpix_setup_t badpixel_detect_method;
static s32 saturation, contrast, hue, sharpenStatus =
    1, sharpenLevel, brightness, wdrEnable, wdrRange;
//static GADI_BOOL isAuto;
static GADI_BOOL irDayNight = GADI_TRUE; // 0 day 1 night
static color_correction_t color_corr;
static color_correction_reg_t color_corr_reg;

u8 *uv_buffer_clip = NULL;
u8 *y_buffer_clip = NULL;
int uv_width, uv_height;
static VENC_ENCODE_BsFifoInfoT jpg_bs_param;
static CAP_RAW_InfoT raw_info;
static cfa_leakage_filter_t cfa_lk_filter;

static int blc_avg_r = 0;
static int blc_avg_g = 0;
static int blc_avg_b = 0;

//static max_change_t max_change;
//static spatial_filter_t spatial_filter_info;
//static fir_t fir = { -1, {0, -1, 0, 0, 0, 8, 0, 0, 0, 0} };

//static u8 edge_shap_level = 1;
u8 *bsb_mem;
u32 bsb_size;

//PGA  by ChengBo
//static GADI_SYS_HandleT PGAchandle = NULL;
GADI_S32 fps_t;

//VPS by ChengBo
//static vps_mode vps_mode_info;
//static vps_stg0_controller_t vps_stg0_controller;
//static vps_stg1_yuv2d_t vps_stg1_yuv2d;
//static vps_stg2_yuv3d_t vps_stg2_yuv3d;
//static vps_stg3_contrast_t vps_stg3_contrast;
//static tuning_vps_yuv3d_str_t tuning_vps_yuv3d_str;
//static tuning_vps_contrast_t tuning_vps_contrast;


// Added by ChengBo (Created by heqi)
//static tuning_CFA_denoise_t tuning_CFA_denoise;
static tuning_sharp_str_t tuning_sharp_str;
static tuning_LE_str_t tuning_LE_str;

// AWB model Control by ChengBo
//static awb_control_mode_t awb_control_mode;
static u8 awb_mode;
static u8 awb_ratio_r,awb_ratio_b;
static u8 AWBspeed;

// AE Control by ChengBo
u8 autoKnee;
u8 speedLevel;
u8 AESpeedMode;
u16 AECTarget;

//low_bitrate CHENG Bo 2017/03/23
static u8 image_low_bitrate_enable = 0;
static image_low_bitrate_property_t image_low_bitrate;
static u8 tolerance_low_bitrate;
static int delay_frames_low_bitrate;

static int gVencStreamStateSave[GADI_VENC_STREAM_NUM] = {0};


/*
extern int apc_set_hue(int hue);
extern int apc_set_brightness(int bright);
extern int apc_set_contrast(int contrast);
extern int apc_set_saturation(int saturation);
extern int apc_get_img_property(image_property_t * p_image_property);
extern int apc_set_color_conversion(rgb_to_yuv_t * rgb_to_yuv_matrix);
*/
static local_wdr_t local_wdr = { 1, 4, 16, 16, 16, 6,
{1024, 1054, 1140, 1320, 1460, 1538, 1580, 1610, 1630, 1640, 1640, 1635,
1625, 1610, 1592, 1563,
1534, 1505, 1475, 1447, 1417, 1393, 1369, 1345, 1321, 1297, 1273,
1256, 1238, 1226, 1214, 1203,
1192, 1180, 1168, 1157, 1149, 1142, 1135, 1127, 1121, 1113, 1106,
1098, 1091, 1084, 1080, 1077,
1074, 1071, 1067, 1065, 1061, 1058, 1055, 1051, 1048, 1045, 1044,
1043, 1042, 1041, 1040, 1039,
1038, 1037, 1036, 1035, 1034, 1033, 1032, 1031, 1030, 1029, 1029,
1029, 1029, 1028, 1028, 1028,
1028, 1027, 1027, 1027, 1026, 1026, 1026, 1026, 1025, 1025, 1025,
1025, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024}
};

tone_curve_t tone_curve = {
    {                           /* red */
        0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
            64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120,
            124,
            128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 173, 177,
            181, 185, 189,
            193, 197, 201, 205, 209, 213, 217, 221, 225, 229, 233, 237, 241,
            245, 249, 253,
            257, 261, 265, 269, 273, 277, 281, 285, 289, 293, 297, 301, 305,
            309, 313, 317,
            321, 325, 329, 333, 337, 341, 345, 349, 353, 357, 361, 365, 369,
            373, 377, 381,
            385, 389, 393, 397, 401, 405, 409, 413, 417, 421, 425, 429, 433,
            437, 441, 445,
            449, 453, 457, 461, 465, 469, 473, 477, 481, 485, 489, 493, 497,
            501, 505, 509,
            514, 518, 522, 526, 530, 534, 538, 542, 546, 550, 554, 558, 562,
            566, 570, 574,
            578, 582, 586, 590, 594, 598, 602, 606, 610, 614, 618, 622, 626,
            630, 634, 638,
            642, 646, 650, 654, 658, 662, 666, 670, 674, 678, 682, 686, 690,
            694, 698, 702,
            706, 710, 714, 718, 722, 726, 730, 734, 738, 742, 746, 750, 754,
            758, 762, 766,
            770, 774, 778, 782, 786, 790, 794, 798, 802, 806, 810, 814, 818,
            822, 826, 830,
            834, 838, 842, 846, 850, 855, 859, 863, 867, 871, 875, 879, 883,
            887, 891, 895,
            899, 903, 907, 911, 915, 919, 923, 927, 931, 935, 939, 943, 947,
            951, 955, 959,
            963, 967, 971, 975, 979, 983, 987, 991, 995, 999, 1003, 1007, 1011,
            1015, 1019, 1023},
        {                           /* green */
            0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
                64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120,
                124,
                128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 173, 177,
                181, 185, 189,
                193, 197, 201, 205, 209, 213, 217, 221, 225, 229, 233, 237, 241,
                245, 249, 253,
                257, 261, 265, 269, 273, 277, 281, 285, 289, 293, 297, 301, 305,
                309, 313, 317,
                321, 325, 329, 333, 337, 341, 345, 349, 353, 357, 361, 365, 369,
                373, 377, 381,
                385, 389, 393, 397, 401, 405, 409, 413, 417, 421, 425, 429, 433,
                437, 441, 445,
                449, 453, 457, 461, 465, 469, 473, 477, 481, 485, 489, 493, 497,
                501, 505, 509,
                514, 518, 522, 526, 530, 534, 538, 542, 546, 550, 554, 558, 562,
                566, 570, 574,
                578, 582, 586, 590, 594, 598, 602, 606, 610, 614, 618, 622, 626,
                630, 634, 638,
                642, 646, 650, 654, 658, 662, 666, 670, 674, 678, 682, 686, 690,
                694, 698, 702,
                706, 710, 714, 718, 722, 726, 730, 734, 738, 742, 746, 750, 754,
                758, 762, 766,
                770, 774, 778, 782, 786, 790, 794, 798, 802, 806, 810, 814, 818,
                822, 826, 830,
                834, 838, 842, 846, 850, 855, 859, 863, 867, 871, 875, 879, 883,
                887, 891, 895,
                899, 903, 907, 911, 915, 919, 923, 927, 931, 935, 939, 943, 947,
                951, 955, 959,
                963, 967, 971, 975, 979, 983, 987, 991, 995, 999, 1003, 1007, 1011,
                1015, 1019, 1023},
            {                           /* blue */
                0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
                    64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120,
                    124,
                    128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 173, 177,
                    181, 185, 189,
                    193, 197, 201, 205, 209, 213, 217, 221, 225, 229, 233, 237, 241,
                    245, 249, 253,
                    257, 261, 265, 269, 273, 277, 281, 285, 289, 293, 297, 301, 305,
                    309, 313, 317,
                    321, 325, 329, 333, 337, 341, 345, 349, 353, 357, 361, 365, 369,
                    373, 377, 381,
                    385, 389, 393, 397, 401, 405, 409, 413, 417, 421, 425, 429, 433,
                    437, 441, 445,
                    449, 453, 457, 461, 465, 469, 473, 477, 481, 485, 489, 493, 497,
                    501, 505, 509,
                    514, 518, 522, 526, 530, 534, 538, 542, 546, 550, 554, 558, 562,
                    566, 570, 574,
                    578, 582, 586, 590, 594, 598, 602, 606, 610, 614, 618, 622, 626,
                    630, 634, 638,
                    642, 646, 650, 654, 658, 662, 666, 670, 674, 678, 682, 686, 690,
                    694, 698, 702,
                    706, 710, 714, 718, 722, 726, 730, 734, 738, 742, 746, 750, 754,
                    758, 762, 766,
                    770, 774, 778, 782, 786, 790, 794, 798, 802, 806, 810, 814, 818,
                    822, 826, 830,
                    834, 838, 842, 846, 850, 855, 859, 863, 867, 871, 875, 879, 883,
                    887, 891, 895,
                    899, 903, 907, 911, 915, 919, 923, 927, 931, 935, 939, 943, 947,
                    951, 955, 959,
                    963, 967, 971, 975, 979, 983, 987, 991, 995, 999, 1003, 1007, 1011,
                    1015, 1019, 1023}
};

//static coring_table_t coring;
static chroma_scale_filter_t cs;

//GK7202 CHENG Bo 2017-09-25
//DEPURPLE
static depurple_fringe_t depurple_fringe;

//Demosaic
static demosaic_noise_reduction_t demosaic_noise_reduction;
static chroma_median_filter_ex_t  chroma_median_filter_ex;

//Denoise
static nlm_noise_filter_t nlm_noise_filter_info;
static variance_guided_filter_t variance_guided_filter_info;
static gmd_cfa_t gmd_cfa_info;
static pmv_t pmv_info;
static globle_motion_filter_t globle_motion_filter_info;
static bilateral_filter_t  bilateral_filter_info;
static fir_t fir_tuning;
static max_change_t max_change_tuning;
static spatial_filter_t spatial_filter_tuning;
static local_contrast_enhancement_t local_contrast_enhancement_info;


static u8* reg = NULL;
static u8* matrix = NULL;
static u8* matrix1 = NULL;
static u8* matrix2 = NULL;
static u8* matrix3 = NULL;
static u8* matrix4 = NULL;


static u8 isCapture = 0;

static color_correction_t color_cc1;
static color_correction_t color_cc2;
static color_correction_t color_cc3;
static color_correction_t color_cc4;


// Bin file save CHENG Bo
int  RGBindex = 0;
int  table_no;
static int  day = 1;
int dBA;
int dBB;

//static u32 grad_clip_thresh;
//static u32 grad_noise_thresh;
//static u32 act_thresh;
//static u32 act_diff_thresh;

static int venc_fps[4] = {0};
static int venc_Set_fps[4] = {0};


// Bin file new Config
static blc_level_t              bin_blc;
static blc_level_t              bin_blcM;
static blc_level_t              bin_blcN;

static tuning_sharp_str_t       bin_tuning_sharp_str;
static tuning_sharp_str_t       bin_tuning_sharp_strM;
static tuning_sharp_str_t       bin_tuning_sharp_strN;

static chroma_median_filter_t   bin_color_filter_config;
static dbp_correction_t         bin_dbp_correction;
static dbp_correction_t         bin_dbp_correctionM;
static dbp_correction_t         bin_dbp_correctionN;
u8 BinAntiAliasing;

static tuning_CFA_denoise_t     bin_tuning_CFA_denoise;
static tuning_CFA_denoise_t     bin_tuning_CFA_denoiseM;
static tuning_CFA_denoise_t     bin_tuning_CFA_denoiseN;

//static tuning_3d_str_t          bin_tuning_3d_str;
//static tuning_3d_str_t          bin_tuning_3d_strM;
//static tuning_3d_str_t          bin_tuning_3d_strN;


//static vps_stg1_yuv2d_t         bin_vps_stg1_yuv2d;
//static vps_stg1_yuv2d_t         bin_vps_stg1_yuv2dM;
//static vps_stg1_yuv2d_t         bin_vps_stg1_yuv2dN;

//static tuning_vps_yuv3d_str_t   bin_tuning_vps_yuv3d;
//static tuning_vps_yuv3d_str_t   bin_tuning_vps_yuv3dM;
//static tuning_vps_yuv3d_str_t   bin_tuning_vps_yuv3dN;

//static vps_stg2_yuv3d_t         bin_vps_stg2_yuv3d;
//static vps_stg2_yuv3d_t         bin_vps_stg2_yuv3dM;
//static vps_stg2_yuv3d_t         bin_vps_stg2_yuv3dN;

//static tuning_vps_contrast_t    bin_tuning_vps_contrast;
//static tuning_vps_contrast_t    bin_tuning_vps_contrastM;
//static tuning_vps_contrast_t    bin_tuning_vps_contrastN;

static bin_interpolation_t      bin_interpolation;

GADI_ERR errorCode = GADI_OK;


static u8 WDRGet = 0;
static int BinGet = 0; // 0 Bin file receive wrong; 1 right

static u32 histogram_timer = 100;
//static u32 histid = 0;
static u32 PreOrPost = 0;

#define WDRSize 17536

//Video
//static GADI_SYS_HandleT vencHandle  = NULL;
//static GADI_VENC_H264ConfigT vencH264;

static VENC_ENCODE_BitrateInfoT vencbps;
static VENC_H264_GopT vencgop;

static encode_realtime_cntl_t encode_param;
//static encode_realtime_cntl_t bin_encode_param;
//static encode_realtime_cntl_t bin_encode_paramM;
//static encode_realtime_cntl_t bin_encode_paramN;



//extern GADI_SYS_HandleT vencHandle;
//extern GADI_VENC_H264ConfigT vencH264;

GADI_ERR retVal = GADI_OK;

extern float d_tone_curve_get_param(Isp3AProcessor *processor);
extern u8 d_tone_curve_get_status(Isp3AProcessor *processor);

int Int32ToArray(int data, int *argarr)
{
    int i, j;

    i = 1;
    int datacpy = 0;
    int temp = 0;

    if (data > 0) {
        datacpy = data;
        argarr[0] = 0;
    } else {
        datacpy = -data;
        argarr[0] = 1;
    }
    while (datacpy / 10) {
        temp = datacpy % 10;
        argarr[i] = temp;
        datacpy = datacpy / 10;
        i++;
    }
    argarr[i] = datacpy;

    for (j = 1; j < i / 2 + 1; j++) {
        temp = argarr[j];
        argarr[j] = argarr[i - j + 1];
        argarr[i - j + 1] = temp;

    }
    return i + 1;

}

int U32ToCharArray(int data, char *argarr)
{
    int i, j;

    i = 0;
    int datacpy = 0;
    int temp = 0;

    datacpy = data;

    while (datacpy / 10) {
        temp = datacpy % 10;
        argarr[i] = temp;
        datacpy = datacpy / 10;
        i++;
    }
    argarr[i] = datacpy;

    for (j = 0; j < (i + 1) / 2; j++) {
        temp = argarr[j];
        argarr[j] = argarr[i - j];
        argarr[i - j] = temp;

    }

    return i + 1;
}

int Int32ToCharArray(int data, char *argarr)
{
    int i, j;

    i = 1;
    int datacpy = 0;
    int temp = 0;

    if (data > 0) {
        datacpy = data;
        argarr[0] = 0;
    } else {
        datacpy = -data;
        argarr[0] = 1;
    }
    while (datacpy / 10) {
        temp = datacpy % 10;
        argarr[i] = temp;
        datacpy = datacpy / 10;
        i++;
    }
    argarr[i] = datacpy;

    for (j = 1; j < i / 2 + 1; j++) {
        temp = argarr[j];
        argarr[j] = argarr[i - j + 1];
        argarr[i - j + 1] = temp;

    }
    return i + 1;
}

int TCP_get_param(char *param_buffer, int *argarr)
{

    char *ptr = NULL;
    char item[16];
    int i, item_length = 0;

    memset(item, '\0', 16);
    ptr = param_buffer + 1;

    for (i = 0; *ptr != '\0'; i++) {
        while (*ptr != ' ' && *ptr != '\0') {
            ptr++;
            item_length++;
        }
        memcpy(item, ptr - item_length, item_length);
        argarr[i] = atoi(item);
        memset(item, '\0', 16);
        item_length = 0;
        if (*ptr != '\0')
            ptr++;
    }



    return 1;
}

int map_buffer(void)
{
    static int mem_mapped = 0;
    SYS_DRV_MmapInfoT info;

    if (mem_mapped)
        return 0;

    if (ioctl(fdmedia, GK_MEDIA_IOC_MAP_BSB, &info) < 0) {
        perror("tunning: map error 1");
        return -1;
    }
    bsb_mem = info.addr;
    bsb_size = info.length;

    if (ioctl(fdmedia, GK_MEDIA_IOC_MAP_DSP, &info) < 0) {
        perror("tunning: map error 2");
        return -1;
    }
    mem_mapped = 1;
    return 0;
}

static inline int remove_padding_from_pitched_y
    (u8 * output_y, const u8 * input_y, int pitch, int width, int height) {
        int row;

        for (row = 0; row < height; row++) {        //row
            memcpy(output_y, input_y, width);
            input_y = input_y + pitch;
            output_y = output_y + width;
        }
        return 0;
}

static inline int remove_padding_and_deinterlace_from_pitched_uv
    (u8 * output_uv, const u8 * input_uv, int pitch, int width, int height) {
        int row, i;
        u8 *output_v = output_uv;
        u8 *output_u = output_uv + width * height;  //without padding

        for (row = 0; row < height; row++) {        //row
            for (i = 0; i < width; i++) {   //interlaced UV row
                *output_u++ = *input_uv++;  //U Buffer
                *output_v++ = *input_uv++;  //V buffer
            }
            input_uv += (pitch - width) * 2;        //skip padding
        }
        return 0;
}

static int get_vi_real_fps(void)
{
    static GADI_ULONG fps_total[2] = {0, 0};
    static struct timeval tm[2] = {{0, 0}, {0, 0}};

    int fps = 0;
    GADI_ULONG fps_current = 0;
    struct timeval tm_cur;

    if (ioctl(fdmedia, GK_MEDIA_IOC_VI_GET_CORE_IRQ_TIMES,
        &fps_current) < 0) {
            GADI_ERROR("get interrupe times failed.\n");
            return 0;
    }

    gettimeofday(&tm_cur, NULL);
    fps = (int)((float)(fps_current - fps_total[0])*1000/((tm_cur.tv_sec - tm[0].tv_sec)*1000 +
        (tm_cur.tv_usec - tm[0].tv_usec)/1000) + 0.5f);

    /* if > 2 second, need backup fps data*/
    if (tm_cur.tv_sec - tm[1].tv_sec > 1) {
        if (fps_total[0] == 0)
            fps = 0;

        fps_total[0] = fps_total[1];
        tm[0] = tm[1];
        fps_total[1] = fps_current;
        tm[1] = tm_cur;
    }
    return fps;
}

static int get_venc_setting_fps(int streamid)
{
    VENC_ENCODE_FramerateFactorT fameRateFactor;
    VENC_ENCODE_StreamInfoT streamInfo;
    int viFrameRateTime, frameRateTime, frameRate;

    if (streamid < 0 || streamid >= 4)
        return 0;

    /*check stream status.*/
    streamInfo.id = 1 << streamid;
    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_ENCODE_STREAM_INFO_EX,&streamInfo) < 0)
        return 0;
    if(streamInfo.state != MEDIA_STREAM_STATE_ENCODING)
        return 0;

    /*get framerate factor.*/
    fameRateFactor.id = 1 << streamid;
    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_FRAMERATE_FACTOR_EX, &fameRateFactor) < 0)
        return 0;
    if(fameRateFactor.ratio_denominator == 0 || fameRateFactor.ratio_numerator == 0)
        return 0;

    /*get vi framerate time.*/
    if (ioctl(fdmedia, GK_MEDIA_IOC_VI_SRC_GET_FRAME_RATE, &viFrameRateTime) < 0)
        return 0;

    frameRateTime = viFrameRateTime*fameRateFactor.ratio_denominator/fameRateFactor.ratio_numerator;
    frameRate = gadi_priv_get_framerate(frameRateTime);

    return frameRate;
}

static int get_venc_real_fps(int streamid)
{
    static long fps_total[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    static struct timeval tm[4][2] = {{{0, 0}, {0, 0}}, {{0, 0}, {0, 0}},
    {{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}};

    VENC_ENCODE_StreamInfoT streamInfo;
    long fps = 0;
    long fps_current = 0;
    struct timeval tm_cur;

    if (streamid < 0 || streamid >= 4)
        return 0;

    streamInfo.id = 1 << streamid;
    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_ENCODE_STREAM_INFO_EX, &streamInfo) < 0)
        return 0;

    fps_current = streamInfo.cnt;
    if (fps_current < 0)
        return 0;

    gettimeofday(&tm_cur, NULL);
    fps = (int)((float)(fps_current - fps_total[streamid][0])*1000 /
        ((tm_cur.tv_sec - tm[streamid][0].tv_sec)*1000 +
        (tm_cur.tv_usec - tm[streamid][0].tv_usec)/1000) + 0.5f);

    if (fps_current < fps_total[streamid][0]){
        fps = 0;
    }

    /* if > 2 second, need backup fps data*/
    if (tm_cur.tv_sec - tm[streamid][1].tv_sec > 1) {
        if (fps_total[streamid][0] == 0)
            fps = 0;

        fps_total[streamid][0] = fps_total[streamid][1];
        tm[streamid][0] = tm[streamid][1];
        fps_total[streamid][1] = fps_current;
        tm[streamid][1] = tm_cur;
    }

    return fps;
}


int gkiq_get_preview_resource(int fd)
{
    CAP_YUV_BufferInfoT preview_buffer_info;
    int uv_pitch;
    const int yuv_format = 1;   // 0 is yuv422, 1 is yuv420
    int quit_yuv_stream = 0;

    while (!quit_yuv_stream) {
        if (isCapture == 1) {
            return -1;
        }
        preview_buffer_info.source = 1;
        if (ioctl(fdmedia, GK_MEDIA_IOC_READ_YUV_BUFFER_INFO_EX,
            &preview_buffer_info) < 0) {
                if (errno == EINTR) {
                    usleep(500);
                    continue;       //back to for()
                } else {
                    perror("read yuv buffer falied");
                    return -1;
                }
        }
        if (y_buffer_clip == NULL) {
            y_buffer_clip =
                malloc(preview_buffer_info.width * preview_buffer_info.height);
            uv_buffer_clip =
                malloc(preview_buffer_info.width * preview_buffer_info.height);
            if (y_buffer_clip == NULL || uv_buffer_clip == NULL) {
                printf("Tuning malloc yuv buffer failed,width:%d,height:%d\n",
                    preview_buffer_info.width, preview_buffer_info.height);
                return -1;
            } else {
                printf("Tuning : yuv picture width:%d, height:%d\n",
                    preview_buffer_info.width, preview_buffer_info.height);
            }
        }

        if (preview_buffer_info.pitch == preview_buffer_info.width) {
            memcpy(y_buffer_clip, preview_buffer_info.y_addr,
                preview_buffer_info.width * preview_buffer_info.height);
        } else if (preview_buffer_info.pitch > preview_buffer_info.width) {
            remove_padding_from_pitched_y(y_buffer_clip,
                preview_buffer_info.y_addr, preview_buffer_info.pitch,
                preview_buffer_info.width, preview_buffer_info.height);
        } else {
            printf
                ("error->stride size smaller than width!,stride:%d,width:%d,height:%d\n",
                preview_buffer_info.pitch, preview_buffer_info.width,
                preview_buffer_info.height);
            return -1;
        }

        //convert uv data from interleaved into planar format
        if (yuv_format == 1) {
            uv_pitch = preview_buffer_info.pitch / 2;
            uv_width = preview_buffer_info.width / 2;
            uv_height = preview_buffer_info.height / 2;
            //_printf("y1:widht:%d,he:%d\n",uv_width,uv_height);
        } else {
            uv_pitch = preview_buffer_info.pitch / 2;
            uv_width = preview_buffer_info.width / 2;
            uv_height = preview_buffer_info.height;
            //_printf("y2:widht:%d,he:%d\n",uv_width,uv_height);
        }
        remove_padding_and_deinterlace_from_pitched_uv(uv_buffer_clip,
            preview_buffer_info.uv_addr, uv_pitch, uv_width, uv_height);
        quit_yuv_stream = 1;

    }
    //_printf("Output YUV resolution %d x %d in YV12 format\n", preview_buffer_info.width, preview_buffer_info.height);
    return 0;
}

static int send_preview_buffer(int fd_sock)
{
    int rev;
    char char_w_len[10];
    char char_h_len[10];

    int y_buffer_w = 2 * uv_width;
    int w_len = Int32ToCharArray(y_buffer_w, char_w_len);

    send(fd_sock, char_w_len, w_len, 0);
    send(fd_sock, " ", 1, 0);

    int y_buffer_h = 2 * uv_height;
    int h_len = Int32ToCharArray(y_buffer_h, char_h_len);

    send(fd_sock, char_h_len, h_len, 0);
    send(fd_sock, " ", 1, 0);
    send(fd_sock, "!", 1, 0);
    rev = send(fd_sock, y_buffer_clip, y_buffer_w * y_buffer_h, 0);
    if (rev < 0) {
        printf("tunning:send Y data fail\n");
        return rev;
    }
    rev = send(fd_sock, uv_buffer_clip, uv_width * uv_height * 2, 0);
    if (rev < 0) {
        printf("tunning:send UV data fail\n");
        return rev;
    }
    return rev;
}

static int tuning_capture_get_raw(CAP_RAW_InfoT *raw_info)
{

    raw_info->src_id = 0;/*only capture VIN 1*/
    if (ioctl(fdmedia, GK_MEDIA_IOC_READ_RAW_INFO, raw_info) < 0) {
        perror("tunning: read raw data error");
        return -1;
    }
    printf("tunning:raw addr = %p\n", raw_info->raw_addr);
    printf("tunning:bits resolution = %d\n", raw_info->bit_resolution);
    printf("tunning:resolution: (w:d)(%d_%d)\n", raw_info->width,
        raw_info->height);

    return 0;
}

static int tunning_enter_preview_stat(void)
{
    if (ioctl(fdmedia, GK_MEDIA_IOC_ENABLE_PREVIEW) < 0) {
        perror("tunning: enable preview failed");
        return -1;
    }
    return 0;
}

#if 0
static int send_raw_stream(int fd_sock)
{
    int rev;

    rev =
        send(fd_sock, raw_info.raw_addr, raw_info.width * raw_info.height * 2,
        0);
    if (rev <= 0) {
        _printf("tunning:send raw frame failed\n");
    }
    return rev;
}
#else
static int send_raw_stream2(int fd_sock, int number)
{
    int rev;

    rev =
        send(fd_sock, raw_info.raw_addr,
        raw_info.width * raw_info.height * 2 * number, 0);
    if (rev <= 0) {
        _printf("tunning:send raw frame failed\n");
    }
    return rev;
}
#endif
static int send_jpeg_stream(int fd_sock)
{
    int rev;
    int jpegSize = (jpg_bs_param.desc[0].pic_size + 31) & ~31;
    u8 *start_addr = (void *) jpg_bs_param.desc[0].start_addr;

    printf("capute: jpeg size:%dKB\n", jpegSize / 1024);
    if (jpegSize <= 0) {
        return -1;
    }
    if ((start_addr + jpegSize) <= (bsb_mem + bsb_size)) {
        rev = send(fd_sock, start_addr, jpegSize, 0);
        if (rev <= 0) {
            _printf("tunning:send jpeg frame fail!\n");
        }
    } else {
        int sendLen = (uint32_t) bsb_mem + bsb_size - (uint32_t) (start_addr);

        rev = send(fd_sock, start_addr, sendLen, 0);
        if (rev <= 0) {
            _printf("tunning:send jpeg frame fail!\n");
        }
        rev = send(fd_sock, bsb_mem, (jpegSize - sendLen), 0);
        if (rev <= 0) {
            _printf("tunning:send jpeg frame fail!\n");
        }
    }
    return rev;
}
#if 0
static void send_pic_stream(int fd_sock)
{
    char char_raw_len[10];
    int stream_len = raw_info.width * raw_info.height * 2;
    int len = Int32ToCharArray(stream_len, char_raw_len);

    send(fd_sock, char_raw_len, len, 0);
    send(fd_sock, " ", 1, 0);
    //int rev1,rev2;
    //rev1=send_raw_stream(fd_sock);
    send_raw_stream(fd_sock);

    //rev2=send_jpeg_stream(fd_sock);
    send_jpeg_stream(fd_sock);
    send(fd_sock, "!", 1, 0);

}
#else
static void send_pic_stream2(int fd_sock, int number)
{
    char char_raw_len[10];
    int stream_len = raw_info.width * raw_info.height * 2 * number;
    int len = Int32ToCharArray(stream_len, char_raw_len);

    send(fd_sock, char_raw_len, len, 0);
    send(fd_sock, " ", 1, 0);
    //int rev1,rev2;
    //rev1=send_raw_stream(fd_sock);
    send_raw_stream2(fd_sock, number);

    //rev2=send_jpeg_stream(fd_sock);
    send_jpeg_stream(fd_sock);
    send(fd_sock, "!", 1, 0);

}
#endif
static int tuning_capture_get_jpeg_stream(void)
{
    int rval = 0;

    memset(&jpg_bs_param, 0, sizeof(jpg_bs_param));

    /* Read bit stream descriptiors */
    rval = ioctl(fdmedia, GK_MEDIA_IOC_READ_BITSTREAM, &jpg_bs_param);
    if (rval < 0) {
        perror("tunning: read jpeg frame error");
        goto save_jpeg_stream_exit;
    }

    if (jpg_bs_param.count) {
        printf("tunning:Read steam number: %d\n", jpg_bs_param.count);
    } else {
        printf("tunning:no stream available!\n");
        goto save_jpeg_stream_exit;
    }
save_jpeg_stream_exit:
    return rval;
}

static int send_dump_info(int fd_sock, u8 * dump_buffer, int buffer_size)
{
    char char_dump_size[10];
    int len = Int32ToCharArray(buffer_size, char_dump_size);

    send(fd_sock, char_dump_size, len, 0);
    send(fd_sock, " ", 1, 0);

    int rev;

    rev = send(fd_sock, dump_buffer, buffer_size, 0);
    if (rev <= 0) {
        printf("tunning:transfer dsp dump data fail\n");
    }
    return rev;
}

static int send_buffer(int fd_sock, u8 * buffer, int buffer_size)
{
    char char_buffer_size[10];
    int len = Int32ToCharArray(buffer_size, char_buffer_size);

    send(fd_sock, char_buffer_size, len, 0);
    send(fd_sock, " ", 1, 0);

    int rev;

    rev = send(fd_sock, buffer, buffer_size, 0);
    if (rev <= 0) {
        printf("tunning:send buffer fail\n");
    }
    return rev;
}

u16 gain_curve[NUM_CHROMA_GAIN_CURVE] =
{ 256, 299, 342, 385, 428, 471, 514, 557, 600, 643, 686, 729, 772, 815, 858,
901,
936, 970, 990, 1012, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
1024, 1024, 1024, 1024,
1024, 1012, 996, 956, 916, 856, 796, 736, 676, 616, 556, 496, 436, 376, 316,
256
};

static int info_mode = 0;
int sock_mesg_recv(int sock, char *buffer)
{
    recv(sock, buffer, 2, 0);
    if (buffer[0] == '3') {
        if (buffer[1] == '1') {
            info_mode = 1;
            return 3;
        } else if (buffer[1] == '2') {
            info_mode = 2;
            return 3;
        } else {
            return 0;
        }
    } else if (buffer[0] == '2') {
        if (buffer[1] == '1') {
            info_mode = 1;
            return 2;
        } else if (buffer[1] == '2') {
            info_mode = 2;
            return 2;
        } else {
            info_mode = 2;
            return 2;
        }
    } else if (buffer[0] == '1') {
        return 1;
    } else {
        return 0;
    }
}

static int situation = 0;

static void handle_tuning_pipe(int sig)
{
    situation = 1;
}

void *threadproc_2()
{

    /*    struct sigaction action;
    action.sa_handler = handle_pipe;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE, &action, NULL);*/
    //    signal(SIGPIPE, SIG_IGN);
    //printf("threadproc_2\n");
    int sock_thread = -1, newfd_thread = -1;
    fd_set rfds;
    struct timeval tv;
    int retval, maxfd = -1;

    //    int IsFirst=1;
    prctl(PR_SET_NAME, __func__);
    if ((sock_thread = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    int on = 2048;
    struct sockaddr_in my_addr_thread, their_addr_thread;

    //ret = setsockopt( sock_thread, SOL_SOCKET, SO_REUSEADDR,(char*) &on, sizeof(int) );
    setsockopt(sock_thread, SOL_SOCKET, SO_REUSEADDR, (char *) &on,
        sizeof(int));
    memset(&my_addr_thread, 0, sizeof(my_addr_thread));
    my_addr_thread.sin_family = AF_INET;
    my_addr_thread.sin_port = htons(7102); //8000
    my_addr_thread.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr_thread.sin_zero), 0);

    if (bind(sock_thread, (struct sockaddr *) &my_addr_thread,
        sizeof(struct sockaddr)) == -1) {
            perror("bind");
            exit(1);
    }
    if (listen(sock_thread, 10) == -1) {
        perror("listen");
        exit(1);
    }
    //    int flag=0;
    while (KEYMACTH) {

WaitForConnection:
        _printf("tunning:Waiting client to connect\n");
        socklen_t s_size = sizeof(struct sockaddr_in);

        if ((newfd_thread =
            accept(sock_thread, (struct sockaddr *) &their_addr_thread,
            &s_size)) == -1) {
                perror("accapt");
                continue;
        }
        situation = 0;
        _printf("tunning:Connect success!\n");
        char pre_rev_thread[4];
        int r = 0;

        //int flag=0;
        while (1) {
            struct sigaction action;

            action.sa_handler = handle_tuning_pipe;
            sigemptyset(&action.sa_mask);
            action.sa_flags = 0;
            sigaction(SIGPIPE, &action, NULL);

            //            _printf("situation %d\n",situation);
            if (situation == 1) {
                if (newfd_thread != -1)
                    close(newfd_thread);
                break;
            }
            FD_ZERO(&rfds);
            maxfd = 0;
            FD_SET(newfd_thread, &rfds);
            if (newfd_thread > maxfd)
                maxfd = newfd_thread;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
            //            _printf("select retval %d\n",retval);
            if (retval == -1) {
                printf("select error:%s!\n", strerror(errno));
                break;
            } else if (retval == 0) {
                //printf("no recv now!\n");
                continue;
                //    goto haha;
            } else {
                if (FD_ISSET(newfd_thread, &rfds)) {
                    r = sock_mesg_recv(newfd_thread, pre_rev_thread);
                    //printf("r=%d",r);
                    if (r == 0) {
                        if (1)  //////////////////to be improved
                        {
                            _printf("tunning:stop\n");
                            break;
                        }
                        //continue;

                    } else if (r == 1) {
                        _printf("tunning:3A ISP infor unvisible\n");
                        goto WaitForConnection;

                    } else if (r == 2) {
                        //_printf("Get recv, and begin to send info...\n");
                        Isp3APublicContent *content = &(handle->content.publicData);
                        gk_isp_get_statistics(handle);
                        _printf("tunning:statistic_mode =%d\n", info_mode);
                        if (info_mode == 1) {
                            int i = 0, j = 0, k = 0, t = 0, n = 0;
                            int thread_aaainfo[2000];

                            for (k = 0; k < 40; k++) {
                                thread_aaainfo[k] = content->st_af_cfa_info[k].sum_fv2;
                            }
                            for (i = 0; i < 96; i++) {
                                thread_aaainfo[k + i] = content->st_ae_info[i].lin_y;
                            }
                            for (j = 0; j < 384; j++) {
                                thread_aaainfo[k + i + j] =
                                    content->st_awb_info[j].r_avg;
                            }
                            for (t = 0; t < 384; t++) {
                                thread_aaainfo[k + i + j + t] =
                                    content->st_awb_info[t].g_avg;
                            }
                            for (n = 0; n < 384; n++) {
                                thread_aaainfo[k + i + j + t + n] =
                                    content->st_awb_info[n].b_avg;
                            }

                            char aaainfo[10];

                            for (i = 0; i < NUM_AAAINFO; i++) {
                                int k;

                                k = U32ToCharArray(thread_aaainfo[i], aaainfo);
                                char n_aaainfo[1];

                                n_aaainfo[0] = k;
                                if (situation == 1)
                                    break;
                                send(newfd_thread, n_aaainfo, 1, 0);
                                send(newfd_thread, aaainfo, k, 0);
                            }
                            //_printf("send info time:%d\n",++k_num);
                            //flag=1;
                        } else if (info_mode == 2) {
                            //send(newfd_thread,gk_3a_hist_data.hist_bin_data,4*sizeof(gk_3a_hist_data.hist_bin_data),0);
                            //_printf("HAHA\n");
                            int i = 0;
                            char sendhehe[10];

                            for (i = 0; i < 256; i++) {
                                int k;

                                //k=Int32ToCharArray(gk_3a_hist_data.hist_bin_data[i],sendhehe);
                                //k=Int32ToCharArray(st_dsp_histo_info.cfa_histogram.his_bin_r[i],sendhehe);
                                k = Int32ToCharArray(content->st_dsp_histo_info.
                                    rgb_histogram.his_bin_y[i], sendhehe);
                                //_printf("%d,",gk_3a_hist_data.hist_bin_data[i]);
                                send(newfd_thread, sendhehe, k, 0);
                                send(newfd_thread, " ", 1, 0);
                            }
                            send(newfd_thread, "!", 1, 0);
                        }
                    } else if (r == 3) {
                        /* get real vi fps*/
                        char sendbuf[10];
                        if (info_mode == 1) {
                            int vi_rfps = get_vi_real_fps();
                            int len = Int32ToCharArray(vi_rfps, sendbuf);
                            send(newfd_thread, sendbuf, len, 0);
                            send(newfd_thread, "!", 1, 0);
                            /* get real venc fps*/
                        } else if (info_mode == 2) {
                            int i;
                            int venc_rfps;
                            int len;
                            for (i = 0; i < 4; i++) {
                                venc_rfps = get_venc_real_fps(i);
                                venc_fps[i] = venc_rfps;
                                venc_Set_fps[i] = get_venc_setting_fps(i);
                                len = Int32ToCharArray(venc_rfps, sendbuf);
                                send(newfd_thread, sendbuf, len, 0);
                                send(newfd_thread, " ", 1, 0);
                            }
                            send(newfd_thread, "!", 1, 0);
                        } else {
                            _printf("tunning: status error!\n");
                        }
                    } else {
                        _printf("tunning: status error!\n");

                    }

                }
            }

        }
    }
    if (sock_thread != -1)
        close(sock_thread);
    return NULL;
}

void *preview_proc()
{
    int sock_pre = -1, sock_client;
    struct timeval timeout = { 3, };

    prctl(PR_SET_NAME, __func__);

    if ((sock_pre = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket\n");
        exit(1);
    }
    int on = 2048;              //ret;
    struct sockaddr_in here_addr, their_addr;

    //ret = setsockopt( sock_pre, SOL_SOCKET, SO_REUSEADDR,(char*) &on, sizeof(int) );
    setsockopt(sock_pre, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(int));
    memset(&here_addr, 0, sizeof(here_addr));
    here_addr.sin_family = AF_INET;
    here_addr.sin_port = htons(7103); //7000
    here_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(here_addr.sin_zero), 0);

    if (bind(sock_pre, (struct sockaddr *) &here_addr,
        sizeof(struct sockaddr)) == -1) {
            perror("bind");
            exit(1);
    }
    if (listen(sock_pre, 10) == -1) {
        perror("listen");
        exit(1);
    }
    while (1) {
        signal(SIGFPE, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        socklen_t s_size = sizeof(struct sockaddr_in);

        if ((sock_client =
            accept(sock_pre, (struct sockaddr *) &their_addr,
            &s_size)) == -1) {
                perror("accapt");
                continue;
        }
        _printf("tunning:start picture transfer...!\n");
        setsockopt(sock_client, SOL_SOCKET, SO_SNDTIMEO,
            (const char *) &timeout, sizeof(timeout));

        closePreview = 1;

        while (closePreview && KEYMACTH) {
            //_printf("%d frame\n",n++);

#if 0                           // temporary comment it,this code will cause vsync irq loss, the real resason has not found.
            memset(y_buffer_clip, 0, MAX_YUV_BUFFER_SIZE);
            memset(uv_buffer_clip, 0, MAX_YUV_BUFFER_SIZE);
#endif

            if (gkiq_get_preview_resource(fdmedia) == -1) {
                usleep(1000 * 10);
                continue;
            }
            //_printf("send: %d frames\n",n++);
            if (send_preview_buffer(sock_client) < 0) {
                break;
            }
            usleep(1000 * 20);
            continue;
            //break;
            /*
            * ret=sock_mesg_recv(sock_client,re);
            * _printf("ret=%d\n",ret);
            * if(ret>0)
            * continue;
            * else
            * _printf("not preview recv!\n");
            */
        }
        _printf("tunning: stop send preview data\n");
        if (y_buffer_clip) {
            free(y_buffer_clip);
            y_buffer_clip = NULL;
        }
        if (uv_buffer_clip) {
            free(uv_buffer_clip);
            uv_buffer_clip = NULL;
        }
        close(sock_client);
    }
    return NULL;
}

static inline int get_vi_mode(u32 * vi_mode)
{
    GK_DRV_VideoModeT video_mode;

    video_mode.src_id = 0;
    if (ioctl(fdmedia, GK_MEDIA_IOC_VI_SRC_GET_VIDEO_MODE, &video_mode)) {
        perror("tunning: get video input mode error");
        return -1;
    }
    *vi_mode = video_mode.vmode;
    //    _printf("vi_mode %d\n",*vi_mode);
    return 0;
}

static inline int get_vi_frame_rate(u32 * pFrame_time)
{
    int rval;

    if ((rval =
        ioctl(fdmedia, GK_MEDIA_IOC_VI_SRC_GET_FRAME_RATE,
        pFrame_time)) < 0) {
            perror("tunning: get framerate error");
            return rval;
    }
    //    _printf("frame_rate %d\n",*pFrame_time);
    return 0;
}

static GADI_ERR venc_query(GADI_S32 streamId, GADI_CHN_AttrT *chn_attr)
{
    VENC_ENCODE_StreamInfoT stream_info;
    VENC_ENCODE_FormatT format_info;

    stream_info.id = 1 << streamId;
    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_ENCODE_STREAM_INFO_EX,
        &stream_info) < 0) {
            TUNING_ERR("Fail to query stream info in tuning.");
            return -1;
    }
    format_info.id = 1 << streamId;
    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_ENCODE_FORMAT_EX,
        &format_info) < 0) {
            TUNING_ERR("Fail to query stream format in tuning.");
            return -1;
    }
    chn_attr->state = stream_info.state;
    chn_attr->type = format_info.encode_type;
    chn_attr->id = streamId;

    return GADI_OK;
}

static GADI_ERR venc_stop_stream(GADI_U32 streamId)
{
    GADI_U32 i = 0;
    GADI_U32 indexFormat = 0;
    GADI_U32 mediaState = 0;
    VENC_ENCODE_StreamInfoT streamInfo;

    indexFormat = 1 << streamId;

    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_STATE, &mediaState) < 0) {
        TUNING_ERR("get driver state failed.\n");
        return -1;
    }
    if (mediaState != MEDIA_STATE_ENCODING) {
        TUNING_INFO("venc_stop_stream: driver state:%d\n", mediaState);
        return GADI_OK;
    }

    for (i = 0; i < GADI_VENC_STREAM_NUM; ++i) {
        streamInfo.id = (1 << i);
        if (ioctl(fdmedia, GK_MEDIA_IOC_GET_ENCODE_STREAM_INFO_EX,
            &streamInfo) < 0) {
                TUNING_ERR("get encode stream info failed.\n");
                continue;
        }
        if (streamInfo.state != MEDIA_STREAM_STATE_ENCODING) {
            indexFormat &= ~(1 << i);
        }
    }
    if (indexFormat == 0) {
        TUNING_INFO("venc_stop_stream: stream:%d is not encoding\n",
            streamId);
        return GADI_OK;
    }

    if (ioctl(fdmedia, GK_MEDIA_IOC_STOP_ENCODE_EX, indexFormat) < 0) {
        TUNING_ERR("stop encode failed.\n");
        return -1;
    }

    TUNING_INFO("Stop encoding for stream 0x%x.\n", streamId);

    return GADI_OK;
}

static GADI_ERR venc_start_stream(GADI_U32 streamId)
{
    GADI_U32 i = 0;
    GADI_U32 indexFormat = 0;
    VENC_ENCODE_StreamInfoT info;

    indexFormat = 1 << streamId;
    for (i = 0; i < GADI_VENC_STREAM_NUM; ++i) {
        info.id = (1 << i);
        if (ioctl(fdmedia, GK_MEDIA_IOC_GET_ENCODE_STREAM_INFO_EX, &info) < 0) {
            TUNING_ERR("get encode stream info failed.\n");
            continue;
        }
        if (info.state == MEDIA_STREAM_STATE_ENCODING) {
            indexFormat &= ~(1 << i);
        }
    }
    if (indexFormat == 0) {
        return GADI_OK;
    }

    if (ioctl(fdmedia, GK_MEDIA_IOC_START_ENCODE_EX, indexFormat) < 0) {
        TUNING_ERR("start encode stream failed.\n");
        return -1;
    }

    TUNING_INFO("Stream [%d] encoding started\n", streamId);

    return GADI_OK;
}

static GADI_ERR tuning_venc_stop_stream(void)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U32 cnt;
    GADI_CHN_AttrT stChnAttr;

    /*stop all streams encoding */
    for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++) {
        memset(&stChnAttr, 0, sizeof(stChnAttr));
        gVencStreamStateSave[cnt] = 0;
        if (venc_query(cnt, &stChnAttr) != GADI_OK) {
            continue;
        }
        if (stChnAttr.type != GADI_VENC_TYPE_OFF
            && stChnAttr.state == GADI_VENC_STREAM_STATE_ENCODING) {
                TUNING_INFO("stop stream[%d]\n", cnt);
                if (venc_stop_stream(cnt) < 0) {
                    TUNING_ERR("tuning_venc_stop_stream error, streams:%d\n", cnt);
                    return -1;
                }
                gVencStreamStateSave[cnt] = 1;
        }
    }

    return retVal;
}

static GADI_ERR tuning_venc_start_stream(void)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U32 cnt;
    GADI_CHN_AttrT stChnAttr;
    ISP_StatisticsConfigT statisticConfig;

    /*stop all streams encoding */
    for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++) {
        memset(&stChnAttr, 0, sizeof(stChnAttr));
        if (venc_query(cnt, &stChnAttr) != GADI_OK) {
            continue;
        }
        if (gVencStreamStateSave[cnt] != 0
            && stChnAttr.type != GADI_VENC_TYPE_OFF
            && stChnAttr.state != GADI_VENC_STREAM_STATE_ENCODING) {
                TUNING_INFO("start stream[%d]\n", cnt);
                if (venc_start_stream(cnt) < 0) {
                    TUNING_INFO("tuning_venc_start_stream error, streams:%d\n", cnt);
                    return -1;
                }
                gVencStreamStateSave[cnt] = 0;
        }
    }

    gk_isp_get_config_statistics_params(handle, &statisticConfig);
    gk_isp_config_statistics_params(handle, &statisticConfig);
    img_dsp_reset_denoise(handle);

    return retVal;
}

static GADI_S32 tunning_set_realtime_schedule(pthread_t tid, int priority)
{
    struct sched_param param;
    GADI_S32 policy = SCHED_FIFO;

    if (!tid) {
        return -1;
    }

    memset(&param, 0, sizeof(param));
    param.sched_priority = priority;

    if (pthread_setschedparam(tid, policy, &param) < 0) {
        _printf("tunning:pthread_setschedparam");
    }

    pthread_getschedparam(tid, &policy, &param);

    if (param.sched_priority != priority) {
        return -1;
    }

    return 0;
}

int tuning_BLC_Avg_RGB()
{
    int ii, jj;
    u64 add_r, add_g, add_b;
    u32 sum_r, sum_g, sum_b;
    int max_r, min_r;
    int max_g, min_g;
    int max_b, min_b;
    //int avg_r, avg_g, avg_b;
    u16 *raw_buff;
    //SYS_DRV_MmapInfoT mmap_info;
    CAP_RAW_InfoT raw_info_BLC;
    still_cap_info_t still_cap_info;
    int rval = -1;

    gk_enable_ae(handle, 0);
    gk_enable_awb(handle, 0);
    gk_enable_af(handle, 0);
    gk_enable_auto_param_ctl(handle, 0);
    //sleep(1);

    //Capture raw here
    _printf("Raw capture started\n");
    memset(&still_cap_info, 0, sizeof(still_cap_info_t));
    still_cap_info.capture_num = 1;
    still_cap_info.need_raw = 1;

    _printf("tuning_venc_stop_stream\n");
    tuning_venc_stop_stream();

    tunning_enter_preview_stat();

    _printf("gk_init_image_capture \n");
    gk_init_image_capture(handle, 50);
    _printf("gk_start_image_capture \n");
    gk_start_image_capture(handle, &still_cap_info);


    //get_raw

    _printf("GK_MEDIA_IOC_READ_RAW_INFO Start\n");

    tuning_capture_get_raw(&raw_info_BLC);

    _printf("GK_MEDIA_IOC_READ_RAW_INFO End\n");

    tuning_capture_get_jpeg_stream();
    _printf("get_jpeg_stream() \n\n");


    raw_buff =
        (u16 *) malloc(raw_info_BLC.width * raw_info_BLC.height * sizeof(u16));

    memcpy(raw_buff, raw_info_BLC.raw_addr,
        (raw_info_BLC.width * raw_info_BLC.height * 2));



    _printf("Raw capture stopped\n");

    add_r = 0;
    add_g = 0;
    add_b = 0;

    sum_r = (raw_info_BLC.width * raw_info_BLC.height)/4;
    sum_g = (raw_info_BLC.width * raw_info_BLC.height)/2;
    sum_b = (raw_info_BLC.width * raw_info_BLC.height)/4;

    _printf("sum_r:%d, sum_g:%d, sum_b:%d\n", sum_r, sum_g, sum_b);
    _printf("0x%x, 0x%x, 0x%x, 0x%x\n", raw_buff[0], raw_buff[1],raw_buff[2],raw_buff[3]);

    max_r = 0;
    min_r = 16383;
    max_g = 0;
    min_g = 16383;
    max_b = 0;
    min_b = 16383;

    //raw_info.bayer_pattern = 3;

    _printf("bayer patten:%d\n", raw_info_BLC.bayer_pattern);
    for (ii = 0; ii < raw_info_BLC.height; ii++) {
        for (jj = 0; jj < raw_info_BLC.width; jj ++) {
            if (raw_info_BLC.bayer_pattern == 0) {  //RGGB
                if ((ii % 2 == 0) & (jj % 2 == 0)) {
                    add_r = add_r + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_r) {
                        max_r = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_r) {
                        min_r = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 0) & (jj % 2 == 1)) {
                    add_g = add_g + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_g) {
                        max_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_g) {
                        min_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 1) & (jj % 2 == 0)) {
                    add_g = add_g + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_g) {
                        max_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_g) {
                        min_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 1) & (jj % 2 == 1)) {
                    add_b = add_b + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_b) {
                        max_b = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_b) {
                        min_b = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }
            }

            if (raw_info_BLC.bayer_pattern == 1) {  //BGGR
                if ((ii % 2 == 0) & (jj % 2 == 0)) {
                    add_b = add_b + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_b) {
                        max_b = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_b) {
                        min_b = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 0) & (jj % 2 == 1)) {
                    add_g = add_g + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_g) {
                        max_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_g) {
                        min_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 1) & (jj % 2 == 0)) {
                    add_g = add_g + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_g) {
                        max_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_g) {
                        min_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 1) & (jj % 2 == 1)) {
                    add_r = add_r + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_r) {
                        max_r = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_r) {
                        min_r = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }
            }

            if (raw_info_BLC.bayer_pattern == 2) {  //GRBG
                if ((ii % 2 == 0) & (jj % 2 == 0)) {
                    add_g = add_g + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_g) {
                        max_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_g) {
                        min_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 0) & (jj % 2 == 1)) {
                    add_r = add_r + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_r) {
                        max_r = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_r) {
                        min_r = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 1) & (jj % 2 == 0)) {
                    add_b = add_b + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_b) {
                        max_b = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_b) {
                        min_b = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 1) & (jj % 2 == 1)) {
                    add_g = add_g + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_g) {
                        max_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_g) {
                        min_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }
            }

            if (raw_info_BLC.bayer_pattern == 3) {  //GBRG
                if ((ii % 2 == 0) & (jj % 2 == 0)) {
                    add_g = add_g + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_g) {
                        max_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_g) {
                        min_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 0) & (jj % 2 == 1)) {
                    add_b = add_b + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_b) {
                        max_b = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_b) {
                        min_b = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 1) & (jj % 2 == 0)) {
                    add_r = add_r + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_r) {
                        max_r = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_r) {
                        min_r = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }

                if((ii % 2 == 1) & (jj % 2 == 1)) {
                    add_g = add_g + raw_buff[ii * raw_info_BLC.width + jj];
                    if (raw_buff[ii * raw_info_BLC.width + jj] > max_g) {
                        max_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                    if (raw_buff[ii * raw_info_BLC.width + jj] < min_g) {
                        min_g = raw_buff[ii * raw_info_BLC.width + jj] ;
                    }
                }
            }
        }
    }

    _printf("max_r:0x%x, max_g:0x%x, max_b:0x%x\n", max_r, max_g, max_b);
    _printf("min_r:0x%x, min_g:0x%x, max_b:0x%x\n", min_r, min_g, min_b);
    //_printf("add_r:0x%x\n", add_r);
    //_printf("add_g:0x%x\n", add_g);
    //_printf("add_b:0x%x\n", add_b);
    blc_avg_r = add_r/sum_r;
    blc_avg_g = add_g/sum_g;
    blc_avg_b = add_b/sum_b;

    _printf("\n");
    _printf("avg_r:%d, avg_g:%d, avg_b:%d\n\n", blc_avg_r, blc_avg_g, blc_avg_b);


#if 0
    get_raw();
    _printf("get_raw() \n\n");

    get_jpeg_stream();
    _printf("get_jpeg_stream() \n\n");

#endif
    _printf("gk_stop_image_capture\n");
    rval = gk_stop_image_capture(handle);
    if (rval < 0) {
        _printf("gk_stop_image_capture error!\n");
        //goto vignette_cal_exit;
    }

    gk_leave_image_capture(handle);

    _printf("tuning_venc_start_stream\n");
    _printf("\n");

    tuning_venc_start_stream();

    free(raw_buff);

    _printf("tunning: BLC over\n");

    gk_enable_ae(handle, 1);
    gk_enable_awb(handle, 1);
    gk_enable_af(handle, 1);
    gk_enable_auto_param_ctl(handle, 1);

    return 0;

}

//added by CHENG Bo 09-03-2018 for GK7202 Debug model
//#define DECODE_GK710X        1
#define DECODE_GK720X        1

#ifdef DECODE_GK710X
#define APB_CONFIG_BASE_ADDRESS                    0x70118000
#define APB_DATA_BASE_ADDRESS                      0x70110000
#else
#define APB_CONFIG_BASE_ADDRESS                    0xA0118000
#define APB_DATA_BASE_ADDRESS                      0xA0110000
#endif

#ifdef DECODE_GK720X
#define APB_CONFIG_BASE_ADDRESS                    0xA0118000
#define APB_DATA_BASE_ADDRESS                      0xA0110000
#endif

//*****************************************************************************
//** Section 2
//*****************************************************************************
#define APB_SEC2_LCE                               (0x00000200 + (0x12 << 3))

#define PAGE_SHIFT                 12

typedef struct {
    u16 lce_enable:2;
    u16 pre_lut_en:1;
    u16 pm_en:1;
    u16 mv_scan_en:1;
    u16 disp_mv_en:1;
//added by Hu Yin 31-07-2017 for GK7202
    u16 padding_num :8;
    u16 dual_vin_en:1;
    u16 stream_split_en:1;
//end

    u16 pre_hist_scan_en:1;
    u16 pre_hist_scan_frame:1;
    u16 pre_hist_scan_id:4;
    u16 post_hist_scan_en:1;
    u16 post_hist_scan_frame:1;
    u16 post_hist_scan_id:4;
    u16 hist_scan_sft:1;
    u16 sensor_id:1;
    u16 stream_split_num:2;

    u16 res_width;
    u16 res_height;

    u16 lce_range:8;
    u16 lce_guard:8;

    u16 lce_range_th1:6;
    u16 default2:2;
    u16 lce_range_th2:6;
    u16 default3:2;

    u16 lce_min_th:8;
    u16 lce_max_th:8;

    u16 lce_minb_range:8;
    u16 lce_maxb_range:8;

    u16 lce_contrast:8;
    u16 lce_delta_th:8;

    u16 lce_cutlimit;
    u16 lce_alpha;
    u16 lce_sud_th:5;
    u16 default4:3;
    u16 lce_sud_sum_th:8;

    u16 lce_delta_disable:1;
    u16 lce_index:4;
    u16 lce_search_th:5;
    u16 default5:6;

    u16 pm_y_gain:8;
    u16 default10:4;
    u16 pm_y_shift:4;
    u16 pm_u_gain:8;
    u16 default11:4;
    u16 pm_u_shift:4;
    u16 pm_v_gain:8;
    u16 default12:4;
    u16 pm_v_shift:4;

    u16 pm_y_offset:9;
    u16 default6:7;
    u16 pm_u_offset:9;
    u16 default7:7;
    u16 pm_v_offset:9;
    u16 default8:7;
    u16 mv_statis_th:8;
    u16 default9:8;

    u16 ram_lut[256];
    u16 ram_pm_mask[688];
	u16 post_hist_lut[256];

    u16 dbg_mk_map0;
    u16 dbg_mk_map1;
    u16 dbg_mk_map2;
    u16 dbg_mk_map3;
    u16 dbg_mk_map4;
    u16 dbg_mk_map5;
    u16 dbg_mk_map6;
    u16 dbg_mk_map7;
    u16 dbg_mk_map8;
    u16 dbg_mk_map9;
    u16 dbg_mk_map10;
    u16 dbg_mk_lut0;
    u16 dbg_mk_lut1;
    u16 dbg_mk_hist0;
    u16 dbg_mk_hist1;
    u16 dbg_pm;
    u16 dbg_lce_pipe;
}local_ontrast_enhancement_regs_t;
//end
local_ontrast_enhancement_regs_t local_ontrast_enhancement_regs_data;
#define MAX_STRING_LEN      256
static char op_filename[MAX_STRING_LEN] = "";

static int dump_data(char *pmem, u32 start, u32 size_limit,
              u32 address, u32 size, u32 *pData)
{
    u32     offset, i;
    int     fd;

    printf("\ndump_data:");
    if (address & 0x03) {
        printf("address[0x%08x] will be alined to 0x%08x!\n",
               address, (address & 0xfffffffc));
        address &= 0xfffffffc;
    }

    printf("offset = address - start;\n");
    offset = address - start;
    if (offset >= size_limit) {
        printf("offset too big! 0x%x < 0x%x\n", size_limit, offset);
        return -1;
    }

    printf("(offset + size) >= size_limit\n");
    if ((offset + size) >= size_limit) {
        printf("size too big! 0x%x < 0x%x\n",
            size_limit, (offset + size));
        size = size_limit - offset;
    }
    pmem += offset;

    printf("strlen(op_filename)\n");
    printf("op_filename %s\n",op_filename);
    if (strlen(op_filename)) {
        printf("1 op_filename %s\n",op_filename);
        if (strcmp(op_filename, "stdout") == 0) {
            fd = STDOUT_FILENO;
        } else if ((fd = open(op_filename,
                              O_CREAT | O_TRUNC | O_WRONLY, 0777)) < 0) {
            printf(op_filename);
            return -1;
        }

        if (write(fd, (void *)pmem, size) < 0) {
            printf("write");
            return -1;
        }

        close(fd);
    } else {
        //printf("\n0x%08x:", pData);
        for (i = 0; i < size; i += 4) {
            //if (i % 16 == 0) {
            //    printf("\n0x%08x:", address + i);
            //}

            //data = *(u32 *)(pmem + i);
            *pData = *(u32 *)(pmem + i);
            printf("\t0x%08x", *pData);
             //printf("\t0x%08x", *(u32 *)(pmem + i));
            pData++;
        }
        printf("\n");
    }

    return 0;
}

static int fill_data(char *pmem, u32 start, u32 size_limit,
              u32 address, u32 size, u32 *pData)
{
    u32     offset, i;
    int     fd;

    printf("\nfill_data:");
    if (address & 0x03) {
        printf("address[0x%08x] will be alined to 0x%08x!\n",
                address, (address & 0xfffffffc));
        address &= 0xfffffffc;
    }

    offset = address - start;
    if (offset >= size_limit) {
        printf("offset too big! 0x%x < 0x%x\n", size_limit, offset);
        return -1;
    }

    if ((offset + size) >= size_limit) {
        printf("size too big! 0x%x < 0x%x\n", size_limit, (offset + size));
        size = size_limit - offset;
    }

    pmem += offset;

    if (strlen(op_filename)) {
        if ((fd = open(op_filename, O_RDONLY, 0777)) < 0) {
            printf(op_filename);
            return -1;
        }

        if (read(fd, (void *)pmem, size) < 0) {
            printf("read");
            return -1;
        }

        close(fd);
    } else {
        for (i = 0; i < size; i += 4) {
             *(u32 *)(pmem + i) = *pData;
             printf("\t0x%08x", *pData);
             pData++;
        }

        printf("\n");
    }

    return 0;
}


static void get_local_ontrast_enhancement_params(local_ontrast_enhancement_regs_t* localContrastEnhancementRegs)
{
     int        fd, debug_flag;
     //struct     gk_debug_mem_info mem_info;
     GK_DEBUG_MemInfoT mem_info; ;
     char       *debug_mem = NULL;
     u32        map_start = 0;
     u32        addr_0, reg_0, size_0;
     u32        addr_1, reg_1[256], size_1;
     u32        apbData;
     int        i;

     if ((fd = open("/dev/debug", O_RDWR, 0)) < 0) {
         printf("open /dev/debug error\n");
         return;
     }

     if (ioctl(fd, GK_DEBUG_IOC_GET_DEBUG_FLAG, &debug_flag) < 0) {////??
         printf("GK_DEBUG_IOC_GET_DEBUG_FLAG");
         return;
     }

     printf("debug_flag 0x%08x\n", debug_flag);

     if (ioctl(fd, GK_DEBUG_IOC_GET_MEM_INFO, &mem_info) < 0) {//????
         printf("GK_DEBUG_IOC_GET_MEM_INFO");
         return;
     }

     printf("apb_start: 0x%08x\n",  mem_info.modules[GK_DEBUG_MODULE_APB].start);
     printf("apb_size:  0x%08x\n",  mem_info.modules[GK_DEBUG_MODULE_APB].size);

     map_start = APB_CONFIG_BASE_ADDRESS;//op_address;

     map_start >>= PAGE_SHIFT;
     map_start <<= PAGE_SHIFT;
     debug_mem = (char*)mmap(NULL, APB_CONFIG_BASE_ADDRESS - map_start + 1,
                              PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_start);

     //printf("mmap returns 0x%08x\n", (unsigned)debug_mem);

     //Selcet the AML reg
     apbData = APB_SEC2_LCE;
     fill_data(debug_mem, map_start, 0xffffffff, APB_CONFIG_BASE_ADDRESS, 1, &apbData);

     if (debug_mem)
         munmap(debug_mem, 1);

     map_start = APB_DATA_BASE_ADDRESS;//op_address;

     map_start >>= PAGE_SHIFT;
     map_start <<= PAGE_SHIFT;
     debug_mem = (char*)mmap(NULL, APB_DATA_BASE_ADDRESS - map_start + 1,
                              PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_start);

     // reg 0x0
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 0;
     size_0 = 4 * 1;
     _printf("reg 0x0 address 0x%x\n",addr_0);

     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_enable = reg_0 >> 0;
     localContrastEnhancementRegs->pre_lut_en = reg_0 >> 2;
     localContrastEnhancementRegs->pm_en      = reg_0 >> 3;
     localContrastEnhancementRegs->mv_scan_en = reg_0 >> 4;
     localContrastEnhancementRegs->disp_mv_en = reg_0 >> 5;
     localContrastEnhancementRegs->padding_num = reg_0 >> 6;
     localContrastEnhancementRegs->dual_vin_en     = reg_0 >> 14;
     localContrastEnhancementRegs->stream_split_en = reg_0 >> 15;

     // reg 0x1
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 1;
     size_0 = 4 * 1;
     _printf("reg 0x1 address 0x%x\n",addr_0);

     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->pre_hist_scan_en      = reg_0 >> 0;
     localContrastEnhancementRegs->pre_hist_scan_frame   = reg_0 >> 1;
     localContrastEnhancementRegs->pre_hist_scan_id      = reg_0 >> 2;
     localContrastEnhancementRegs->post_hist_scan_en     = reg_0 >> 6;
     localContrastEnhancementRegs->post_hist_scan_frame  = reg_0 >> 7;
     localContrastEnhancementRegs->post_hist_scan_id     = reg_0 >> 8;
     localContrastEnhancementRegs->hist_scan_sft         = reg_0 >> 12;
     localContrastEnhancementRegs->sensor_id        = reg_0 >> 13;
     localContrastEnhancementRegs->stream_split_num = reg_0 >> 14;


#if 0
     // reg 0x2
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 2;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->res_width      = reg_0 >> 0;

     // reg 0x3
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 3;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->res_height      = reg_0 >> 0;

     // reg 0x4
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 4;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_range         = reg_0 >> 0;
     localContrastEnhancementRegs->lce_guard         = reg_0 >> 8;

     // reg 0x5
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 5;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_range_th1     = reg_0 >> 0;
     localContrastEnhancementRegs->lce_range_th2     = reg_0 >> 8;

     // reg 0x6
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 6;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_min_th     = reg_0 >> 0;
     localContrastEnhancementRegs->lce_max_th     = reg_0 >> 8;

     // reg 0x7
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 7;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_minb_range     = reg_0 >> 0;
     localContrastEnhancementRegs->lce_maxb_range     = reg_0 >> 8;

     // reg 0x8
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 8;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_contrast     = reg_0 >> 0;
     localContrastEnhancementRegs->lce_delta_th     = reg_0 >> 8;

     // reg 0x9
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 9;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_cutlimit     = reg_0;

     // reg 0xA
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 10;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_alpha     = reg_0;

     // reg 0xB
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 11;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_sud_th      = reg_0 >> 0;
     localContrastEnhancementRegs->lce_sud_sum_th  = reg_0 >> 8;

     // reg 0xC
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 12;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->lce_delta_disable  = reg_0 >> 0;
     localContrastEnhancementRegs->lce_index          = reg_0 >> 1;
     localContrastEnhancementRegs->lce_search_th      = reg_0 >> 5;

     // reg 0xD
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 13;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->pm_y_gain   = reg_0 >> 0;
     localContrastEnhancementRegs->pm_y_shift  = reg_0 >> 12;

     // reg 0xE
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 14;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->pm_u_gain   = reg_0 >> 0;
     localContrastEnhancementRegs->pm_u_shift  = reg_0 >> 12;

     // reg 0xF
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 15;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->pm_v_gain   = reg_0 >> 0;
     localContrastEnhancementRegs->pm_v_shift  = reg_0 >> 12;

     // reg 0x10
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 16;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->pm_y_offset   = reg_0 >> 0;

     // reg 0x11
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 17;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->pm_u_offset   = reg_0 >> 0;

     // reg 0x12
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 18;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->pm_v_offset   = reg_0 >> 0;

     // reg 0x13
     addr_0 = APB_DATA_BASE_ADDRESS + 4 * 19;
     size_0 = 4 * 1;
     dump_data(debug_mem, map_start, 0xffffffff, addr_0, size_0, &reg_0);
     localContrastEnhancementRegs->mv_statis_th   = reg_0 >> 0;
#endif

     // reg 0x20
     addr_1 = APB_DATA_BASE_ADDRESS + 4 * 32;
     size_1 = 4 * 256;
     _printf("reg 0x20 address 0x%x\n",addr_1);

     dump_data(debug_mem, map_start, 0xffffffff, addr_1, size_1, reg_1);
     //_printf("ram_lut reg_1 [start]\n");
     for (i=0; i< 256; i++) {
          localContrastEnhancementRegs->ram_lut[i] = (u16)reg_1[i];
          //_printf("cnfg_ram_lut reg_1[%d] : 0x%x\n",i,reg_1[i]);
     }
     //_printf("ram_lut reg_1 [end]\n");


#if 1
     // reg 0x3d0
     int nnn = 256; //256
	 addr_1= APB_DATA_BASE_ADDRESS + 4 * 976;
     size_1= 4 * nnn;
     _printf("reg 0x3d0 address 0x%x\n",addr_1);

     dump_data(debug_mem, map_start, 0xffffffff, addr_1, size_1, reg_1);
     _printf("post_hist_lut reg_1 size %d\n",nnn);
     for (i=0; i< nnn; i++) {
          localContrastEnhancementRegs->post_hist_lut[i] = (u16)reg_1[i];
          _printf("Post-histgram reg_1[%d] : 0x%x\n",i,reg_1[i]);
     }
     _printf("post_hist_lut reg_1 [end]\n");

#endif

#if 0
     // reg 0x120
     addr_2 = APB_DATA_BASE_ADDRESS + 4 * 288;
     size_2 = 4 * 688;
     dump_data(debug_mem, map_start, 0xffffffff, addr_2, size_2, reg_2);
     for (i=0; i< 688; i++) {
          localContrastEnhancementRegs->ram_pm_mask[i] = (u16)reg_2[i];
     }
#endif

     if (debug_mem)
         munmap(debug_mem, 1);

     close(fd);

     return;
}



#define MAX_QP                      (51)
#define MIN_I_QP_REDUCE             (1)
#define MIN_P_QP_REDUCE             (1)
#define MAX_I_QP_REDUCE             (10)
#define MAX_P_QP_REDUCE             (5)

#define H264TUNING  1
#define IRCUTSWITCH 1

#ifdef H264TUNING



//gadi_venc_set_h264_config
GADI_ERR tuning_venc_set_h264_config(u32 streamId, VENC_ENCODE_BitrateInfoT* bps, VENC_H264_GopT* gop)
{
    VENC_H264_ConfigT h264Config;
    VENC_H264_GopT localGop;

    if (fdmedia <= 0) {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    h264Config.id = 1 << streamId;

    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_H264_CONFIG_EX,
        &h264Config) < 0) {
            GADI_ERROR("get h264 config failed.\n");
            return GADI_VENC_ERR_FROM_DRIVER;
    }

    /*change the giving parameters. */
    h264Config.N = gop->N;

    gop->id = 1 << streamId;
    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_GOP_EX, &localGop) < 0) {
        GADI_ERROR("tunning: get gop failed stream %d\n", streamId);
    }

    localGop.N = gop->N;
    if (ioctl(fdmedia, GK_MEDIA_IOC_SET_GOP_EX, &localGop) < 0) {
        GADI_ERROR("tunning: set gop failed stream %d\n", streamId);
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    if (ioctl(fdmedia, GK_MEDIA_IOC_SET_H264_BITRATE_EX, bps) < 0) {
        GADI_ERROR("set bit rate failed.\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR tuning_venc_get_h264_config(u32 streamId, VENC_ENCODE_BitrateInfoT* bps, VENC_H264_GopT* gop)
{
    VENC_H264_ConfigT h264Config;

    if (fdmedia <= 0) {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    h264Config.id = 1 << streamId;
    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_H264_CONFIG_EX,
        &h264Config) < 0) {
        return GADI_VENC_ERR_FROM_DRIVER;
    }
    gop->id = streamId;
    gop->N = h264Config.N;
    gop->idr_interval = h264Config.idr_interval;

    bps->id = 1 << streamId;
    if (ioctl(fdmedia, GK_MEDIA_IOC_GET_H264_BITRATE_EX, bps) < 0) {
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

#endif

#ifdef IRCUTSWITCH

#define IRCUT_NIGHT       0
#define IRCUT_DAY         1

GADI_ERR tuning_gadi_vi_set_ircut_control( GADI_VI_IRModeEnumT mode )
{
    GK_VI_IRcutCtlT ctlPar;

    if (fdmedia <= 0) {
        return GADI_VI_ERR_BAD_PARAMETER;
    }

    ctlPar.mode  = mode;
    ctlPar.srcid = 0;

    if (ioctl(fdmedia, GK_MEDIA_IOC_SET_IRCUT, &ctlPar) < 0) {
        printf("set ircut failed.\n");
        return GADI_VI_ERR_FROM_DRIVER;
    }


    return GADI_OK;
}

static void tuning_image_ircut_switch(GADI_U8 bEnable)//0:daytime   1:night
{

    if(bEnable == IRCUT_DAY)
    {
        printf("day ...\n");
        gk_set_day_night_mode(handle, 0);

        // ir cut
        tuning_gadi_vi_set_ircut_control(GADI_VI_IRCUT_DAY);
        //image_ircut_control_daylight();
        //gadi_sys_thread_sleep(100);
        msleep(100);
        tuning_gadi_vi_set_ircut_control(GADI_VI_IRCUT_CLEAR);
    }
    else
    {
        printf("night ...\n");
        gk_set_day_night_mode(handle, 1);

        // ir cut
        tuning_gadi_vi_set_ircut_control(GADI_VI_IRCUT_NIGHT);
        //image_ircut_control_night();
        //gadi_sys_thread_sleep(100);
        msleep(100);
        tuning_gadi_vi_set_ircut_control(GADI_VI_IRCUT_CLEAR);
        //image_ircut_control_clear();
    }

}

#endif
int tuning_send_binnary(int socket_fd, Isp3AProcessor *processor)
{
    image_sensor_param_t sensor_param;
    int bin_size;
    char *bin = NULL;
    int offset = 0;
    int sendSize = 0;
    int dataLength = 0;
    char nullData[8]={0,};

    bin_size = sensors_size_get(processor);
    if (bin_size <= 0)
        return -1;

    bin = malloc(bin_size);
    if (bin == NULL)
        return -1;

    memset(bin, 0, bin_size); // Clear the data

    errorCode = bin_sensor_param(processor, bin, bin_size, &sensor_param);
    if (errorCode == 0) {
        errorCode = save_bin_sensor_param(processor, bin, bin_size, &sensor_param,WDRGet,
                matrix1,
                matrix2,
                matrix3,
                matrix4);

        dataLength = *((int *) (bin + 4));
        _printf("sensor bin: Length:0x%x\n", dataLength);
        send(socket_fd, bin, 8, 0);
        usleep(100000);
        offset = 8;
        //_printf("id_Reqiure == 9 used\n");

        while (dataLength > 0) {
            if (dataLength > 4096) {
                sendSize = 4096;
            }
            else{
                sendSize = dataLength;
            }
            if (send(socket_fd, bin + offset, sendSize, 0) != sendSize) {
                _printf("send sensor data not finished\n");
                break;
            }
            usleep(20000);
            dataLength -= sendSize;
            offset += sendSize;
        }

        usleep(200000);
        //free(newbin);
        _printf("send sensor data finished\n");
    }
    else {
        send(socket_fd, nullData, 8, 0);
    }
    free(bin);

    return 0;
}
int tuning_receive_binnary(int socket_fd, Isp3AProcessor *processor)
{
    char *bin = NULL;
    int number1;
    int bin_size;
    int binLength;
    int binoffset = 0;
    image_sensor_param_t sensor_param;

    bin_size = sensors_size_get(processor);
    if (bin_size <= 0)
        return -1;

    bin = malloc(bin_size);
    if (bin == NULL)
        return -1;

    memset(bin, 0, bin_size); // Clear the data

    binLength = bin_size;
    while (binLength > 0) {
        if (binLength > 4096) {
            number1 = 4096;
        }
        else{
            number1 = binLength;
        }
        if (recv(socket_fd,bin+binoffset,number1,0) != number1) {
            _printf("Recv sensor data finished\n");
            break;
        }
        binLength -= number1;
        binoffset += number1;
    }

    printf("binoffset is %d\n",binoffset);

    if (binoffset == bin_size) {
    #if 0
        for(i=0;i<bin_size;i++)
        {
            if(bin[i] != bin[i])
            {
                printf("Location %d is not right\n",i);
            }
        }
    #endif
        //BinGet = 1;
    } else {
        //BinGet = 1;
        BinGet = 0;
        printf("Wrong size");
    }

    if (binoffset == bin_size) {

        errorCode = bin_sensor_param(processor, bin, bin_size, &sensor_param);
        if(errorCode == GADI_OK) {
            gk_auto_load_sensor_param(processor, &sensor_param);
            //gadi_isp_set_sensor_bin_data(newbin);
            gadi_sensors_set_bin(processor, bin);
            BinGet = 1;
        } else {
            BinGet = 0;
        }
        printf("gk_auto_load_sensor_param BinGet : %d\n",BinGet);
    } else {
        BinGet = 0;
        printf("Wrong size");
    }

    free(bin);

    return 0;
}

int tuning_send_icore_dump_file(int socket_fd, Isp3AProcessor *processor, int id_section)
{
    SYS_DRV_IcoreCngInfoT dump_icore;
    u8 *dump_buffer;
    int fd = processor->content.publicData.fd;

    _printf("tunning:id num =%d\n", id_section);

    dump_buffer = malloc(MAX_DUMP_BUFFER_SIZE);     //MAX_DUMP_BUFFER_SIZE
    if (dump_buffer == NULL) {
        printf("Not enough memory to dump ICORE config!\n");
        return -1;
    }

    dump_icore.id_section = id_section;
    dump_icore.addr = dump_buffer;
    if (ioctl(fd, GK_MEDIA_IOC_DSP_DUMP_CFG, &dump_icore) < 0) {
        perror("GK_MEDIA_IOC_DSP_DUMP_CFG");
        free(dump_buffer);
        return -1;
    }

    int fp = open("/tmp/dump_dsp.bin", O_WRONLY | O_CREAT, 0666);

    if (write(fp, dump_buffer, MAX_DUMP_BUFFER_SIZE) < 0) {
        perror("tunning:write error");
        free(dump_buffer);
        return -1;
    }

    close(fp);

    send_dump_info(socket_fd, dump_buffer, MAX_DUMP_BUFFER_SIZE);

    free(dump_buffer);

    _printf("tunning:dsp dump finish\n");

    return 0;
}

int tuning_send_still_capture(int socket_fd, Isp3AProcessor *processor)
{
    char recvBuffer[2000];
    int dataArray[1000];
    still_cap_info_t still_cap_info;

    memset(&still_cap_info, 0, sizeof(still_cap_info));

    isCapture = 1;

    recv(socket_fd, recvBuffer, 2000, 0);

    TCP_get_param(recvBuffer, dataArray);

    printf("dataArray is %d\n", dataArray[0]);
    //dataArray[0] = 1;
    still_cap_info.capture_num = dataArray[0];
    still_cap_info.need_raw = 1;
    printf("Frames number is :%d\n\n", still_cap_info.capture_num);

    tuning_venc_stop_stream();

    if (tunning_enter_preview_stat() < 0) {
        isCapture = 0;
        return -1;
    }

    gk_init_image_capture(processor, 50);

    _printf("gk_init_image_capture\n\n");

    gk_start_image_capture(processor, &still_cap_info);

    _printf("gk_start_image_capture\n\n");

    tuning_capture_get_raw(&raw_info);

    _printf("tuning_capture_get_raw\n\n");

    tuning_capture_get_jpeg_stream();

    _printf("tuning_capture_get_jpeg_stream\n\n");

    send_pic_stream2(socket_fd, still_cap_info.capture_num);

    _printf("send_pic_stream2\n\n");

    gk_stop_image_capture(processor);

    _printf("gk_stop_image_capture\n\n");

    gk_leave_image_capture(processor);

    _printf("send still picture over\n");

    tuning_venc_start_stream();

    _printf("tunning: capture picture over\n");
    isCapture = 0;

    return 0;
}
int tuning_send_sensor_bin(int socket_fd, Isp3AProcessor *processor)
{
    int offset;
    int sendSize;
    int data_len;
    char *bin = gadi_sensors_get_bin(processor);

    if (bin == NULL) {
        _printf("Get binary file failed!\n");
        return -1;
    }

    data_len = *((int *) (bin + 4));
    _printf("sensor bin: Length:0x%x\n", data_len);

    send(socket_fd, bin, 8, 0);

    usleep(100000);

    offset = 8;
    //_printf("id_Reqiure == 9 used\n");

    while (data_len > 0) {
        if (data_len > 4096) {
            sendSize = 4096;
        }
        else{
            sendSize = data_len;
        }
        if (send(socket_fd, bin + offset, sendSize, 0) != sendSize) {
            _printf("send sensor data not finished\n");
            break;
        }

        usleep(20000);
        data_len -= sendSize;
        offset += sendSize;
    }

    usleep(200000);
    free(bin);
    _printf("send sensor data finished\n");

    return 0;
}
void * tuning_process(void * argv)
{
    int ret;
    u32 streamId;
	s16 zoom;
    prctl(PR_SET_NAME, __func__);

    if (NULL == reg)
    {
        reg = malloc(18752);
        if (NULL == reg)
        {
            TUNING_ERR("Fail to malloc reg.\n");
            return NULL;
        }
    }

    if (NULL == matrix)
    {
        matrix = malloc(17536);
        if (NULL == matrix)
        {
            if (reg)
            {
                free(reg);
                reg = NULL;
            }
            TUNING_ERR("Fail to malloc matrix.\n");
            return NULL;
        }
    }

    if (NULL == matrix1)
    {
        matrix1 = malloc(17536);
        if (NULL == matrix1)
        {
            if (reg)
            {
                free(reg);
                reg = NULL;
            }

            if (matrix)
            {
                free(matrix);
                matrix = NULL;
            }
            TUNING_ERR("Fail to malloc matrix1.\n");
            return NULL;
        }
    }

    if (NULL == matrix2)
    {
        matrix2 = malloc(17536);
        if (NULL == matrix2)
        {
            if (reg)
            {
                free(reg);
                reg = NULL;
            }

            if (matrix)
            {
                free(matrix);
                matrix = NULL;
            }

            if (matrix1)
            {
                free(matrix1);
                matrix1 = NULL;
            }
            TUNING_ERR("Fail to malloc matrix2.\n");
            return NULL;
        }
    }

    if (NULL == matrix3)
    {
        matrix3 = malloc(17536);
        if (NULL == matrix3)
        {
            if (reg)
            {
                free(reg);
                reg = NULL;
            }

            if (matrix)
            {
                free(matrix);
                matrix = NULL;
            }

            if (matrix1)
            {
                free(matrix1);
                matrix1 = NULL;
            }

            if (matrix2)
            {
                free(matrix2);
                matrix2 = NULL;
            }
            TUNING_ERR("Fail to malloc matrix3.\n");
            return NULL;
        }
    }

    if (NULL == matrix4)
    {
        matrix4 = malloc(17536);
        if (NULL == matrix4)
        {
            if (reg)
            {
                free(reg);
                reg = NULL;
            }

            if (matrix)
            {
                free(matrix);
                matrix = NULL;
            }

            if (matrix1)
            {
                free(matrix1);
                matrix1 = NULL;
            }

            if (matrix2)
            {
                free(matrix2);
                matrix2 = NULL;
            }

            if (matrix3)
            {
                free(matrix3);
                matrix3 = NULL;
            }
            TUNING_ERR("Fail to malloc matrix4.\n");
            return NULL;
        }
    }

    if (NULL == bin_adj_param)
    {
        bin_adj_param = malloc(sizeof(adj_param_t));
        if (NULL == bin_adj_param)
        {
            if (reg)
            {
                free(reg);
                reg = NULL;
            }

            if (matrix)
            {
                free(matrix);
                matrix = NULL;
            }

            if (matrix1)
            {
                free(matrix1);
                matrix1 = NULL;
            }

            if (matrix2)
            {
                free(matrix2);
                matrix2 = NULL;
            }

            if (matrix3)
            {
                free(matrix3);
                matrix3 = NULL;
            }

            if (matrix4)
            {
                free(matrix4);
                matrix4 = NULL;
            }
            TUNING_ERR("Fail to malloc bin_adj_param.\n");
            return NULL;
        }
    }

    if (!fdmedia) {
        gadi_priv_driver_get_fd(&fdmedia);
        if (fdmedia < 0) {
            TUNING_ERR("Fail to open %s.\n", IMAGE_MEDIA_DEV);
            return NULL;
        }
    }

    tunning_set_realtime_schedule(pthread_self(), 20);

    if (map_buffer() < 0) {
        _printf("map private buffer fail!\n");
        return NULL;
    }

    enum step1 {
        OnlineTuning = 0,
        Caribration,
        AutoTest
    };
    enum step2 {
        Color = 0,
        Noise,
        AAA,
        VPS,
        VIDEO,
        WDR,
        BINDATA,
        Denoise,
        LCEHist
    };
    enum colorprocessing {
        BlackLevelCorrection = 0,
        ColorCorrection,
        ToneCurve,
        RGBtoYUVMatrix,
        WhiteBalanceGains,
        DGainSaturaionLevel,
        LocalExposure,
        ChromaScale,
        ColorAdjust,
        Depurple
    };
    enum noiseprocessing {
        FPNCorrection = 0,
        BadPixelCorrection,
        CFALeakageFilter,
        AntiAliasingFilter,
        CFANoiseFilter,
        ChromaMedianFiler,
        SharpeningControl,
        MCTFControl
    };
    enum denoiseprocessing{
		CFAdenoise = 0,
		VGFdenoise,
		BLFdenoise,
		GMDCFAdenoise,
		PMVdenoise,
		GMF3Ddenoise,
		LSHdenoise,
		LCEdenoise,
		HELPdenoise
	};
    enum aaaprocessing {
        ConfigAAAControl = 0,
        AETileConfiguration,
        AWBTilesConfiguration,
        AFTileConfiguration,
        AFStatisticSetupEx,
        ExposureControl,
        AWBModel,
        AEControl
    };
    enum vpsprocessing {
        stage0 = 0,
        stage1,
        stage2,
        stage3
    };
    enum videoprocessing {
        Videopar = 0,
        Encoder,
        Fpspar,
        Pgapar,
        LowBit
    };
    enum wdrprocessing {
        globalwdr = 0,
        localwdr,
        LensDis,
        LensShade,
        BlcAvg,
        DEMOSAIC
    };
    enum histogramconfig{
        histtimerconfig,
        histogramid
    };
    enum bindataprocessing{
        saveConfig = 0,
        newBinfile
    };
    enum load_apply {
        Load = 0,
        Apply = 1
    };


    int sockfd, on = 2048;      //ret;
    int new_fd;
    struct dataBuffer {
        int layer1;
        int layer2;
        int layer3;

    } data;

    sockfd = -1;
    new_fd = -1;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(int));

    struct sockaddr_in my_addr, their_addr;

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(7101); //6000
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 0);

    if (bind(sockfd, (struct sockaddr *) &my_addr,
        sizeof(struct sockaddr)) == -1) {
            perror("bind");
            exit(1);
    }
    if (listen(sockfd, 10) == -1) {
        perror("listen");
        exit(1);
    }
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //Create a new thread
    pthread_t thread_3A_S = -1;
    int err;
    int r_get_param = 0;

    if ((err = pthread_create(&thread_3A_S, NULL, threadproc_2, NULL) != 0))
        _printf("tunning:create thread fail\n");


    venc_fps[0] = get_venc_real_fps(0);
    venc_fps[1] = get_venc_real_fps(1);
    venc_fps[2] = get_venc_real_fps(2);
    venc_fps[3] = get_venc_real_fps(3);

    venc_Set_fps[0] = get_venc_setting_fps(0);
    venc_Set_fps[1] = get_venc_setting_fps(1);
    venc_Set_fps[2] = get_venc_setting_fps(2);
    venc_Set_fps[3] = get_venc_setting_fps(3);

    pthread_t thread_preview = -1;
    if ((err = pthread_create(&thread_preview, NULL, preview_proc, NULL) != 0))
        _printf("tunning:create oreview thread fail\n");

    ////////create over
    //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////


    char nullData[8]={0,};
    int id_Require;
    char ch;
    char pre_revBuffer[5] = { 0x99, };
    struct timeval timeout = { 3, };

    // bin Data Analysis
    while (1) {

        if (new_fd != -1)
            close(new_fd);

        socklen_t sin_size = sizeof(struct sockaddr_in);

        signal(SIGFPE, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        _printf("\nHandle next connect...\n");
        if ((new_fd =
            accept(sockfd, (struct sockaddr *) &their_addr,
            &sin_size)) == -1) {
                perror("accapt");
                continue;
        }
        _printf("socked id:%d\n", new_fd);

        setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout,
            sizeof(timeout));

        ret = recv(new_fd, pre_revBuffer, 5, 0);
        if (errno == EAGAIN && ret < -1) {
            perror("recv");
            continue;
        }

        int vi_src_id = 0;

        ch = pre_revBuffer[0];
        id_Require = atoi(&ch);
        _printf("id_requie:%d\n", id_Require);
        ch = pre_revBuffer[1];
        data.layer1 = atoi(&ch);
        ch = pre_revBuffer[2];
        data.layer2 = atoi(&ch);
        ch = pre_revBuffer[3];
        data.layer3 = atoi(&ch);

        ch = pre_revBuffer[4];
        vi_src_id = atoi(&ch);
        handle = monitor_handle_find_by_id(vi_src_id/*vi_src_id*/);
        _printf("vi_src_id: %d\n",vi_src_id);

        /* Get sensor binary file. */
        if (id_Require == 9 && KEYMACTH == 1) {
            if (tuning_send_sensor_bin(new_fd, handle) < 0)
                send(new_fd, nullData, 8, 0);
            continue;
        }

        if (id_Require == 8)    //for ip testing AmBar (Kill)
        {
            KEYMACTH = 0;
            if (new_fd != -1)
                close(new_fd);
            new_fd = -1;
            _printf("tunning:Connetc success\n");
            isCapture = 0;
            continue;
        }

        /* Get still capture file. */
        if (id_Require == 7  && KEYMACTH == 1)    // for still capture
        {
            tuning_send_still_capture(new_fd, handle);
            continue;
        }

        /* Get isp icore_dump file. */
        if (id_Require == 6  && KEYMACTH == 1)    //for icore_dump
        {
            int id_section;

            if (data.layer1 < 8)
                id_section = data.layer1;
            else if (data.layer1 == 8)
                id_section = 100;
            else {
                _printf("tunning: error id number\n");
                continue;
            }

            tuning_send_icore_dump_file(new_fd, handle, id_section);
            continue;
        }

        /* Get current binnary file. */
        if(id_Require == 5  && KEYMACTH == 1){
            _printf("\nNew Bin File\n");
            _printf("layer3 %d\n",data.layer3);
            switch(data.layer3)
            {
            /* check binnary file operate is ok? */
            case 0: // Check if the new file refresh OK
                {
                char sendBinCon[10];
                int k;
                k=Int32ToCharArray(BinGet,sendBinCon);
                send(new_fd,sendBinCon,k,0);
                send(new_fd," ",1,0);
                send(new_fd,"!",1,0);
                printf("sendBuffer BinGet %d\n",BinGet);
                }
                break;

            case 1: // Read new Bin file (Tuning)
                tuning_receive_binnary(new_fd, handle);
                break;

            case 2: // Save new Bin file (PC)
                tuning_send_binnary(new_fd, handle);
                break; //end of Send bin file to PC

            default:
                printf("Wrong binFile Option\n");
                break;
            }
            continue;
        }

        /* stop preview stat */
        if (id_Require == 4  && KEYMACTH == 1 ) {
            closePreview = 0;
            _printf("tunning:stop preview\n");
            continue;
        }

        if (id_Require == 3)
        {
            char recvBuffer[2000];
            int  dataArray[1000];

            recv(new_fd, recvBuffer, 2000, 0);
            TCP_get_param(recvBuffer, dataArray);

#if 1
            _printf("\n");
            _printf("dataArray[0] %d\n",dataArray[0]);
            _printf("dataArray[1] %d\n",dataArray[1]);
            _printf("\n");
#endif

            switch (dataArray[0])
            {

            case 0:  // Load result
                {
                    char sendKEYBin[10];

                    int k;
                    k=Int32ToCharArray(KEYMACTH,sendKEYBin);
                    send(new_fd,sendKEYBin,k,0);
                    send(new_fd," ",1,0);
                    send(new_fd,"!",1,0);
                }
                break;
            case 1:  // Check
                {
                    if( dataArray[1] !=TUNINGKEY)
                    {
                        KEYMACTH = 0;
                        _printf("Wrong CHIP or IQ Tool\n");
                    }
                    else
                    {
                        KEYMACTH = 1;
                        _printf("Right CHIP and IQ Tool\n");
                    }
                }
                break;
            default:
                _printf("Wrong CMD\n");
                break;

            }

            continue;
        }

        if (id_Require == 2  && KEYMACTH == 1)    //bw <=>color
        {
            if (data.layer1 == 0) {
                gk_set_day_night_mode(handle, 0);
                _printf("tunning:close black/wight\n");
            } else if (data.layer1 == 1) {
                gk_set_day_night_mode(handle, 1);
                _printf("tunning:open black/wight\n");

            }
            continue;
        }

        char recvBuffer[2000];
        int dataArray[1000];
        int sendBuffer[1000];
        char sendCharArray[100];
        int ifDebug;

        int i;
        if (id_Require == Apply  && KEYMACTH == 1)        //apply
        {
            _printf("Setting\n");
            int IsCC = 0;
            int IsFPN = 0;

            if (data.layer1 == 0 && data.layer2 == 0 && data.layer3 == 1) {
                IsCC = 1;
            } else if (data.layer1 == 0 && data.layer2 == 1 && data.layer3 == 0) {
                IsFPN = 1;
                int flag = 1;
                char temp[15];
                int bytes = 0;
                u32 raw_pitch;

                while (flag) {
                    int rbyte = recv(new_fd, temp + bytes, 1, 0);

                    bytes += rbyte;
                    if (temp[bytes - 1] != 33)
                        continue;
                    else
                        break;
                }
                TCP_get_param(temp, dataArray);
                bp_corr_param.dark_pixel_strength = 0;
                bp_corr_param.hot_pixel_strength = 0;
                bp_corr_param.enable = 0;
                memset(&cfa_denoise_filter, 0, sizeof(cfa_denoise_filter));
                gk_isp_set_cfa_denoise_filter(handle, &cfa_denoise_filter);
                gk_isp_set_auto_bad_pixel_correction(handle, &bp_corr_param);
                gk_isp_set_anti_aliasing_filter(handle, 0);

                if (bytes < 5)  //Detection
                {
                    recv(new_fd, recvBuffer, 2000, 0);
                    TCP_get_param(recvBuffer, dataArray);
                    badpixel_detect_method.cap_width = dataArray[0];
                    u32 width = dataArray[0];

                    badpixel_detect_method.cap_height = dataArray[1];
                    u32 height = dataArray[1];

                    badpixel_detect_method.block_w = dataArray[2];
                    badpixel_detect_method.block_h = dataArray[3];
                    badpixel_detect_method.upper_thres = dataArray[4];
                    badpixel_detect_method.lower_thres = dataArray[5];
                    badpixel_detect_method.agc_idx = dataArray[6];
                    badpixel_detect_method.shutter_idx = dataArray[7];
                    badpixel_detect_method.badpix_type = dataArray[8];
                    badpixel_detect_method.detect_times = dataArray[9];

                    raw_pitch = ROUND_UP(width / 8, 32);
                    u32 fpn_map_size = raw_pitch * height;
                    u8 *fpn_map_addr = malloc(fpn_map_size);

                    memset(fpn_map_addr, 0, (fpn_map_size));
                    badpixel_detect_method.badpixmap_buf = fpn_map_addr;
                    badpixel_detect_method.cali_mode = 0; // 0 for video, 1 for still
                    tuning_venc_stop_stream();

                    int bpc_num =
                        gk_detect_bad_pixel(handle, &badpixel_detect_method);
                    _printf("tunning:all number is %d\n", bpc_num);
                    send_buffer(new_fd, fpn_map_addr, fpn_map_size);
                    free(fpn_map_addr);
                    bpc_num++;
#if 0
                    if (ioctl(fdmedia, GK_MEDIA_IOC_ENABLE_PREVIEW) < 0) {
                        perror("GK_MEDIA_IOC_ENABLE_PREVIEW");
                        return NULL;
                    }
#endif
                    tuning_venc_start_stream();
                    _printf("tunning: bad picxel Detection!\n");
                    continue;

                } else          //Correction
                {
                    fpn.pixel_map_width = dataArray[0];
                    fpn.pixel_map_height = dataArray[1];
                    raw_pitch = ROUND_UP((fpn.pixel_map_width / 8), 32);
                    u32 fpn_map_size = raw_pitch * fpn.pixel_map_height;

                    fpn.enable = 3;
                    fpn.fpn_pitch = raw_pitch;
                    fpn.pixel_map_size = fpn_map_size;
                    _printf("tunning:pixel map w = %d,h =%d,%d\n", dataArray[0],
                        dataArray[1], fpn_map_size);
                }
            }
            if (IsCC != 1 && IsFPN != 1) {
                recv(new_fd, recvBuffer, 2000, 0);
                //_printf("%s\n",recvBuffer);

            }
            switch (data.layer1) {
            case OnlineTuning:
                //_printf("Online Tuning!\n");
                switch (data.layer2) {
                case Color:
                    //_printf("Color Processing!\n");
                    switch (data.layer3) {
                    case BlackLevelCorrection:
                        if ((r_get_param =
                            TCP_get_param(recvBuffer,
                            dataArray)) != 1)
                            _printf
                            ("tunning: get tcp data error\n");
                        blc.r_offset  = dataArray[0];
                        blc.gr_offset = dataArray[1];
                        blc.gb_offset = dataArray[2];
                        blc.b_offset  = dataArray[3];


                        if(dataArray[4] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[4]);
                        }else
                        {
                            gk_isp_set_final_blc(handle, &blc, GB);
                            _printf("Black Level Correction set\n");
                            _printf("Right CHIPPIN %d\n\n",dataArray[4]);
                        }
						
						if(blc.gr_offset < 0 )
							{
							if (handle->content.AFData.lens_drv.af_focus_f)
                				handle->content.AFData.lens_drv.af_focus_f(300,-blc.gr_offset);
        					} else {
            					if (handle->content.AFData.lens_drv.af_focus_n)
                					handle->content.AFData.lens_drv.af_focus_n(300,blc.gr_offset);
        					}
							

#ifdef ISP_CONFIG
                        _printf("\n");                                                                                                          _printf("r_offset  %d\n",blc.r_offset);
                        _printf("gr_offset %d\n",blc.gr_offset);
                        _printf("gb_offset %d\n",blc.gb_offset);
                        _printf("b_offset  %d\n",blc.b_offset);
                        _printf("\n");
#endif


                        break;
                    case ColorCorrection:
                        recv(new_fd, reg, 18752, 0);
                        recv(new_fd, matrix, 17536, 0);
                        color_corr_reg.reg_setting_addr = (u32) reg;
                        color_corr.matrix_3d_table_addr =
                            (u32) matrix;
                        gk_isp_set_ccr(handle, &color_corr_reg);
                        gk_isp_set_cc(handle,
                            &color_corr);
                        gk_isp_enable_cc(handle);
                        //gk_isp_set_tone_curve(fdmedia,&tone_curve);
                        _printf("Color Correction!\n");

                        break;
                    case ToneCurve:
                        TCP_get_param(recvBuffer, dataArray);

                        for (i = 0; i < NUM_EXPOSURE_CURVE; i++) {
                            tone_curve.tone_curve_red[i] =
                                dataArray[i];
                            tone_curve.tone_curve_green[i] =
                                dataArray[i];
                            tone_curve.tone_curve_blue[i] =
                                dataArray[i];

                        }

                        gk_isp_set_tone_curve(handle,
                            &tone_curve);
                        _printf("Tone Dynamic Curve\n ");

                        break;
                    case RGBtoYUVMatrix:
                        TCP_get_param(recvBuffer, dataArray);
                        rgb2yuv_matrix.matrix_values[0] =
                            dataArray[0];
                        rgb2yuv_matrix.matrix_values[1] =
                            dataArray[1];
                        rgb2yuv_matrix.matrix_values[2] =
                            dataArray[2];
                        rgb2yuv_matrix.matrix_values[3] =
                            dataArray[3];
                        rgb2yuv_matrix.matrix_values[4] =
                            dataArray[4];
                        rgb2yuv_matrix.matrix_values[5] =
                            dataArray[5];
                        rgb2yuv_matrix.matrix_values[6] =
                            dataArray[6];
                        rgb2yuv_matrix.matrix_values[7] =
                            dataArray[7];
                        rgb2yuv_matrix.matrix_values[8] =
                            dataArray[8];
                        rgb2yuv_matrix.y_offset = dataArray[9];
                        rgb2yuv_matrix.u_offset = dataArray[10];
                        rgb2yuv_matrix.v_offset = dataArray[11];


                        if(dataArray[12] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[12]);
                            //KEYMACTH = 0;
                        }else
                        {
                            apc_set_color_conversion(handle, &rgb2yuv_matrix);
                            gk_isp_set_rgb2yuv_matrix(handle,
                                &rgb2yuv_matrix);
                            _printf("Color Matrix\n");
                            _printf("Right CHIPPIN %d\n\n",dataArray[12]);
                        }

                        break;

                    case WhiteBalanceGains:

                        TCP_get_param(recvBuffer, dataArray);
                        wb_gain.r_gain = dataArray[0];
                        wb_gain.g_gain = dataArray[1];
                        wb_gain.b_gain = dataArray[2];

                        if(dataArray[3] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Set CHIPPIN %d\n\n",dataArray[3]);
                            //KEYMACTH = 0;
                        }else
                        {
                            gk_isp_set_wb_gain(handle, &wb_gain);
                            _printf("White Balance !\n");
                            _printf("Set CHIPPIN %d\n\n",dataArray[3]);
                        }

                        break;

                    case DGainSaturaionLevel:
                        TCP_get_param(recvBuffer, dataArray);
                        isp_gain_satuation_level.level_red =
                            dataArray[0];
                        isp_gain_satuation_level.level_green_even =
                            dataArray[1];
                        isp_gain_satuation_level.level_green_odd =
                            dataArray[2];
                        isp_gain_satuation_level.level_blue =
                            dataArray[3];

                        if(dataArray[4] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Set CHIPPIN %d\n\n",dataArray[4]);
                            //KEYMACTH = 0;
                        }else
                        {
                            gk_isp_set_dgain_thr(handle,
                                &isp_gain_satuation_level);
                            _printf("D_Gainn");
                            _printf("Set CHIPPIN %d\n\n",dataArray[4]);
                        }

                        break;

                    case LocalExposure:
                        TCP_get_param(recvBuffer, dataArray);
                        local_wdr.enable = dataArray[0];
                        local_wdr.radius = dataArray[1];
                        local_wdr.luma_weight_shift =
                            dataArray[2];
                        local_wdr.luma_weight_red =
                            dataArray[3];
                        local_wdr.luma_weight_green =
                            dataArray[4];
                        local_wdr.luma_weight_blue =
                            dataArray[5];

                        for (i = 0; i < NUM_EXPOSURE_CURVE; i++)
                            local_wdr.gain_curve_table[i] =
                            dataArray[i + 6];
                        gk_isp_set_wdr(handle,
                            &local_wdr);
                        _printf("Exposure controlling\n");
                        break;

                    case ChromaScale:
                        TCP_get_param(recvBuffer, dataArray);
                        cs.enable = dataArray[0];

                        for (i = 0; i < NUM_CHROMA_GAIN_CURVE; i++)
                            cs.gain_curve[i] = dataArray[i + 1];

                        if(dataArray[NUM_CHROMA_GAIN_CURVE+1] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[NUM_CHROMA_GAIN_CURVE+1]);
                            //KEYMACTH = 0;
                        }else
                        {
                            gk_isp_set_uv_scale(handle, &cs);
                            _printf("Chroma Rate\n");
                            _printf("Right CHIPPIN %d\n\n",dataArray[NUM_CHROMA_GAIN_CURVE+1]);
                        }

                        break;

                    case ColorAdjust:
                        TCP_get_param( recvBuffer,  dataArray);
                        brightness      = dataArray[0];
                        saturation      = dataArray[1];
                        hue             = dataArray[2];
                        sharpenStatus   = dataArray[3];
                        sharpenLevel    = dataArray[4];
                        contrast        = dataArray[5];

                        //IR cut
                        //day or night
                        irDayNight      = dataArray[6];
                        day             = dataArray[6];


                        if(dataArray[7] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[7]);
                            //KEYMACTH = 0;
                        }else
                        {
                            gk_set_color_brightness(handle, brightness);
                            gk_set_color_saturation(handle, saturation);
                            //gk_set_color_hue(handle, hue);//tmp test
                            gk_set_af_zoom(handle, hue);
                            if(sharpenStatus != 0)
                            {
                                gk_set_sharpness(handle, sharpenLevel);
                            }
                            else
                            {
                                gk_set_auto_sharpen_enable(handle, sharpenStatus);
                            }

                            gk_set_auto_contrast_strength(handle, contrast);

                            //IR cut
                            tuning_image_ircut_switch(irDayNight);

                            _printf("Right CHIPPIN %d\n\n",dataArray[7]);

                        }


#if  1
                        _printf("\n");
                        _printf("brightness     %d\n",brightness);
                        _printf("saturation     %d\n",saturation);
                        _printf("hue            %d\n",hue);
                        _printf("sharpenStatus  %d\n",sharpenStatus);
                        _printf("sharpenLevel   %d\n",sharpenLevel);
                        _printf("contrast       %d\n",contrast);
                        _printf("\n");
#endif


                        _printf("Color Adjust set\n");
                        break;
                    case Depurple: //Apply depurple
                        {
                            TCP_get_param( recvBuffer,  dataArray);
                            depurple_fringe.depurple_fringe_enable          = dataArray[0];
                            depurple_fringe.depurple_fringe_strength        = dataArray[1];
                            depurple_fringe.depurple_fringe_th_max_a        = dataArray[2];
                            depurple_fringe.depurple_fringe_th_max_b        = dataArray[3];
                            depurple_fringe.depurple_fringe_th_max_c        = dataArray[4];
                            depurple_fringe.depurple_fringe_th_max_d        = dataArray[5];
                            depurple_fringe.depurple_fringe_th_max_e        = dataArray[6];
                            depurple_fringe.depurple_fringe_th_max_f        = dataArray[7];
                            depurple_fringe.depurple_fringe_th_max          = dataArray[8];
                            depurple_fringe.depurple_fringe_green_threshold = dataArray[9];
                            depurple_fringe.depurple_fringe_rb_threshold    = dataArray[10];

                            depurple_fringe.dpf_minA_threshold              = dataArray[11];
                            depurple_fringe.dpf_minB_threshold              = dataArray[12];
                            depurple_fringe.dpf_minC_threshold              = dataArray[13];
                            depurple_fringe.dpf_grad_threshold              = dataArray[14];
                            depurple_fringe.dpf_flag_threshold              = dataArray[15];
                            depurple_fringe.dpf_enhance_enable              = dataArray[16];
                            depurple_fringe.nlm_dpf_enable                  = dataArray[17];
                            depurple_fringe.dpf_max_change                  = dataArray[18];
                            depurple_fringe.dpf_alpha                       = dataArray[19];

                            //gk_isp_set_depurple_fringe(handle,&depurple_fringe);
                            zoom = (s16)depurple_fringe.dpf_minA_threshold;
							zoom = zoom - 3500;
							printf("tuning:zoom = %d \n",zoom);
							if(zoom < 0 )
								{
								if (handle->content.AFData.lens_drv.zoom_out)
									{
										handle->content.AFData.lens_drv.zoom_out(300, -zoom);
									} 
								}else {
										if (handle->content.AFData.lens_drv.zoom_in)
										handle->content.AFData.lens_drv.zoom_in(300,zoom);
									}
                            _printf("Apply depurple \n");
                        }
                        break;
                    }
                    // Steven Yu: lost break?
                    break;
                case Noise:
                    switch (data.layer3) {
                    case FPNCorrection:
                        {
                            u8 *fpn_map_addr = NULL;

                            fpn_map_addr =
                                malloc(fpn.pixel_map_size);
                            memset(fpn_map_addr, 0,
                                fpn.pixel_map_size);
                            int bytes = 0;

                            while (bytes < fpn.pixel_map_size) {
                                int rbyte =
                                    recv(new_fd,
                                    fpn_map_addr + bytes, 2000, 0);
                                bytes += rbyte;
                            }

                            fpn.pixel_map_addr = (u32) fpn_map_addr;
                            gk_isp_set_static_bad_pixel_correction
                                (handle, &fpn);
                            free(fpn_map_addr);
                            _printf("FPN Correction!\n");
                        }
                        break;

                    case BadPixelCorrection:
                        TCP_get_param(recvBuffer, dataArray);
                        dbp_correction_config.enable =
                            dataArray[0];
                        dbp_correction_config.hot_pixel_strength =
                            dataArray[1];
                        dbp_correction_config.dark_pixel_strength =
                            dataArray[2];

                        if(dataArray[3] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[3]);
                            //KEYMACTH = 0;
                        }else
                        {
                            gk_isp_set_auto_bad_pixel_correction
                                (handle, &dbp_correction_config);
                            _printf("Bad Pixels Correction\n");
                            _printf("Right CHIPPIN %d\n\n",dataArray[3]);
                        }

                        break;

                    case CFALeakageFilter:
                        TCP_get_param(recvBuffer, dataArray);

                        cfa_lk_filter.enable = dataArray[0];
                        cfa_lk_filter.alpha_rr = dataArray[1];
                        cfa_lk_filter.alpha_rb = dataArray[2];
                        cfa_lk_filter.alpha_br = dataArray[3];
                        cfa_lk_filter.alpha_bb = dataArray[4];
                        cfa_lk_filter.saturation_level =
                            dataArray[5];

                        gk_isp_set_cfa_lea_filter(handle,
                            &cfa_lk_filter);
                        _printf("CFA Leakage\n");
                        break;

                    case AntiAliasingFilter:
                        TCP_get_param(recvBuffer, dataArray);
                        aal_filter_strength = dataArray[0];

                        if(dataArray[1] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[1]);
                            //KEYMACTH = 0;
                        }else
                        {
                            gk_isp_set_anti_aliasing_filter(handle,
                                aal_filter_strength);
                            _printf("Antialiasing\n");
                            _printf("Right CHIPPIN %d\n\n",dataArray[1]);
                        }

                        break;

                    case CFANoiseFilter:
                        TCP_get_param(recvBuffer, dataArray);


						cfa_denoise_filter.direct_center_weight_red   = dataArray[0];
						cfa_denoise_filter.direct_center_weight_green = dataArray[1];
						cfa_denoise_filter.direct_center_weight_blue  = dataArray[2];
						cfa_denoise_filter.direct_grad_thresh		  = dataArray[3];
						cfa_denoise_filter.direct_thresh_k0_red 	  = dataArray[4];
						cfa_denoise_filter.direct_thresh_k0_green	  = dataArray[5];
						cfa_denoise_filter.direct_thresh_k0_blue	  = dataArray[6];

						cfa_denoise_filter.iso_center_weight_red	  = dataArray[7];
						cfa_denoise_filter.iso_center_weight_green	  = dataArray[8];
						cfa_denoise_filter.iso_center_weight_blue	  = dataArray[9];
						cfa_denoise_filter.iso_thresh_k0_red		  = dataArray[10];
						cfa_denoise_filter.iso_thresh_k0_green		  = dataArray[11];
						cfa_denoise_filter.iso_thresh_k0_blue		  = dataArray[12];
						cfa_denoise_filter.iso_thresh_k0_close		  = dataArray[13];


                        if(dataArray[14] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[9]);
                            //KEYMACTH = 0;
                        }else
                        {
                            //tuning_set_CFA_denoise_str(fdmedia,
                                //&tuning_CFA_denoise);

							gk_isp_set_cfa_denoise_filter(handle, &cfa_denoise_filter);
                            _printf("CFA denoise\n");
                            _printf("Right CHIPPIN %d\n\n",dataArray[9]);
                        }


                        break;

                    case ChromaMedianFiler:
                        TCP_get_param(recvBuffer, dataArray);
                        color_filter_config.enable = dataArray[0];
                        color_filter_config.cb_str = dataArray[1];
                        color_filter_config.cr_str = dataArray[2];

                        chroma_median_filter_ex.ex_blur_en		= dataArray[3];
						chroma_median_filter_ex.blur_nolimit_en = dataArray[4];
						chroma_median_filter_ex.ex_blur_delta_1 = dataArray[5];
						chroma_median_filter_ex.ex_blur_delta_2 = dataArray[6];
						chroma_median_filter_ex.ex_blur_delta_3 = dataArray[7];
						chroma_median_filter_ex.ex_blur_mul_1	= dataArray[8];
						chroma_median_filter_ex.ex_blur_sft_1	= dataArray[9];
						chroma_median_filter_ex.ex_blur_mul_2	= dataArray[10];
						chroma_median_filter_ex.ex_blur_sft_2	= dataArray[11];
						chroma_median_filter_ex.ex_blur_mul_3	= dataArray[12];
						chroma_median_filter_ex.ex_blur_sft_3	= dataArray[13];
						chroma_median_filter_ex.ex_blur_data_u	= dataArray[14];
						chroma_median_filter_ex.ex_blur_data_v	= dataArray[15];


						gk_isp_set_color_denoise_filter(handle,
														&color_filter_config);
						//7202
						gk_isp_set_chroma_median_filter_ex(handle, &chroma_median_filter_ex);

                        break;

                    case SharpeningControl:
                        TCP_get_param(recvBuffer, dataArray);

                        tuning_sharp_str.fir_strength =
                            dataArray[0];
                        tuning_sharp_str.isotropic_strength =
                            dataArray[1];
                        tuning_sharp_str.directional_strength =
                            dataArray[2];
                        tuning_sharp_str.edge_threshold =
                            dataArray[3];
                        tuning_sharp_str.max_change_up =
                            dataArray[4];
                        tuning_sharp_str.max_change_down =
                            dataArray[5];

                        if(dataArray[6] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[6]);
                            //KEYMACTH = 0;
                        }else
                        {
                            tuning_set_sharpen_str(handle,
                                &tuning_sharp_str);

                            _printf("Sharpening Setting\n");
                            _printf("Right CHIPPIN %d\n\n",dataArray[6]);
                        }

                        break;

                    case MCTFControl:
                        _printf("3D denoise\n");

                        break;
                    }
                    break;
                case AAA:
                    switch (data.layer3) {
                    case ConfigAAAControl:
                        TCP_get_param(recvBuffer, dataArray);


                        if(dataArray[4] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[4]);
                            //KEYMACTH = 0;
                        }else
                        {

                            gk_enable_ae(handle, dataArray[0]);
                            gk_enable_awb(handle, dataArray[1]);

                            gk_enable_af(handle, dataArray[2]);
                            gk_enable_auto_param_ctl(handle, dataArray[3]);
                            _printf("AAA Set\n");

                            _printf("Right CHIPPIN %d\n\n",dataArray[4]);
                        }

                        _printf(" AAA setting\n");
                        break;

                    case AETileConfiguration:
                        TCP_get_param(recvBuffer, dataArray);

                        gk_isp_get_config_statistics_params
                            (handle, &gk_3a_statistic_config);
                        gk_3a_statistic_config.ae_tile_num_col =
                            dataArray[0];
                        gk_3a_statistic_config.ae_tile_num_row =
                            dataArray[1];
                        gk_3a_statistic_config.ae_tile_col_start =
                            dataArray[2];
                        gk_3a_statistic_config.ae_tile_row_start =
                            dataArray[3];
                        gk_3a_statistic_config.ae_tile_width =
                            dataArray[4];
                        gk_3a_statistic_config.ae_tile_height =
                            dataArray[5];
                        gk_3a_statistic_config.ae_pix_min_value =
                            dataArray[6];
                        gk_3a_statistic_config.ae_pix_max_value =
                            dataArray[7];

                        gk_3a_statistic_config.enable = 1;

                        gk_isp_config_statistics_params(handle,
                            &gk_3a_statistic_config);
                        _printf("AE Tile Configuration!\n");
                        break;

                    case AWBTilesConfiguration:
                        TCP_get_param(recvBuffer, dataArray);

                        gk_isp_get_config_statistics_params
                            (handle, &gk_3a_statistic_config);
                        gk_3a_statistic_config.awb_tile_num_col =
                            dataArray[0];
                        gk_3a_statistic_config.awb_tile_num_row =
                            dataArray[1];
                        gk_3a_statistic_config.awb_tile_col_start =
                            dataArray[2];
                        gk_3a_statistic_config.awb_tile_row_start =
                            dataArray[3];
                        gk_3a_statistic_config.awb_tile_width =
                            dataArray[4];
                        gk_3a_statistic_config.awb_tile_height =
                            dataArray[5];
                        gk_3a_statistic_config.awb_pix_min_value =
                            dataArray[6];
                        gk_3a_statistic_config.awb_pix_max_value =
                            dataArray[7];
                        gk_3a_statistic_config.awb_tile_active_width =
                            dataArray[8];
                        gk_3a_statistic_config.
                            awb_tile_active_height = dataArray[9];

                        gk_3a_statistic_config.enable = 1;

                        gk_isp_config_statistics_params(handle,
                            &gk_3a_statistic_config);
                        _printf("AWB Chunk Setting\n");
                        break;

                    case AFTileConfiguration:
                        TCP_get_param(recvBuffer, dataArray);

                        gk_isp_get_config_statistics_params
                            (handle, &gk_3a_statistic_config);
                        gk_3a_statistic_config.af_tile_num_col =
                            dataArray[0];
                        gk_3a_statistic_config.af_tile_num_row =
                            dataArray[1];
                        gk_3a_statistic_config.af_tile_col_start =
                            dataArray[2];
                        gk_3a_statistic_config.af_tile_row_start =
                            dataArray[3];
                        gk_3a_statistic_config.af_tile_width =
                            dataArray[4];
                        gk_3a_statistic_config.af_tile_height =
                            dataArray[5];
                        gk_3a_statistic_config.af_tile_active_width =
                            dataArray[6];
                        gk_3a_statistic_config.af_tile_active_height =
                            dataArray[7];

                        gk_3a_statistic_config.enable = 1;

                        gk_isp_config_statistics_params(handle,
                            &gk_3a_statistic_config);

                        _printf("AF Chunk Setting\n");
                        break;

                    case AFStatisticSetupEx:
                        TCP_get_param(recvBuffer, dataArray);
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_mode =
                            dataArray[0];
                        // StevenYu: Add
                        af_statistic_config_ex.data1.
                            af_filter_select = 0;
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_stage1_enb =
                            dataArray[1];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_stage2_enb =
                            dataArray[2];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_stage3_enb =
                            dataArray[3];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[0] =
                            dataArray[4];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[1] =
                            dataArray[5];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[2] =
                            dataArray[6];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[3] =
                            dataArray[7];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[4] =
                            dataArray[8];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[5] =
                            dataArray[9];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[6] =
                            dataArray[10];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_shift[0] =
                            dataArray[11];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_shift[1] =
                            dataArray[12];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_shift[2] =
                            dataArray[13];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_shift[3] =
                            dataArray[14];
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_bias_off =
                            dataArray[15];
                        // StevenYu: Add
                        af_statistic_config_ex.data1.
                            af_horizontal_filter_thresh = 0x00;
                        //
                        af_statistic_config_ex.data1.
                            af_vertical_filter_thresh =
                            dataArray[16];
                        af_statistic_config_ex.data1.
                            af_tile_fv_horizontal_shift =
                            dataArray[17];
                        af_statistic_config_ex.data1.
                            af_tile_fv_horizontal_weight =
                            dataArray[18];
                        af_statistic_config_ex.data1.
                            af_tile_fv_vertical_shift =
                            dataArray[19];
                        af_statistic_config_ex.data1.
                            af_tile_fv_vertical_weight =
                            dataArray[20];

                        af_statistic_config_ex.data2.
                            af_horizontal_filter_mode =
                            dataArray[21];
                        // StevenYu: Add
                        af_statistic_config_ex.data2.
                            af_filter_select = 0;
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_stage1_enb =
                            dataArray[22];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_stage2_enb =
                            dataArray[23];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_stage3_enb =
                            dataArray[24];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[0] =
                            dataArray[25];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[1] =
                            dataArray[26];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[2] =
                            dataArray[27];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[3] =
                            dataArray[28];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[4] =
                            dataArray[29];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[5] =
                            dataArray[30];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[6] =
                            dataArray[31];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_shift[0] =
                            dataArray[32];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_shift[1] =
                            dataArray[33];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_shift[2] =
                            dataArray[34];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_shift[3] =
                            dataArray[35];
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_bias_off =
                            dataArray[36];
                        // StevenYu: Add
                        af_statistic_config_ex.data2.
                            af_horizontal_filter_thresh = 0x00;
                        //
                        af_statistic_config_ex.data2.
                            af_vertical_filter_thresh =
                            dataArray[37];
                        af_statistic_config_ex.data2.
                            af_tile_fv_horizontal_shift =
                            dataArray[38];
                        af_statistic_config_ex.data2.
                            af_tile_fv_horizontal_weight =
                            dataArray[39];
                        af_statistic_config_ex.data2.
                            af_tile_fv_vertical_shift =
                            dataArray[40];
                        af_statistic_config_ex.data2.
                            af_tile_fv_vertical_weight =
                            dataArray[41];

                        gk_isp_set_af_statistics_ex(handle, &af_statistic_config_ex, 1);   //??
                        _printf("Auto Focus statistic 2\n");
                        break;

                    case ExposureControl:
                        _printf("Exposure set \n");
                        TCP_get_param(recvBuffer, dataArray);
                        gain_index    = dataArray[0];
                        shutter_index = dataArray[1];
                        dgain         = dataArray[2];

                        if(dataArray[3] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[3]);
                            //KEYMACTH = 0;
                        }else
                        {
                            gk_set_sensor_gain_index(handle, gain_index,
                              handle->content.publicData.sensor_double_step);
                            gk_set_sensor_shutter_time_index(handle,
                                shutter_index);

                            gk_isp_get_rgb_gain(handle, &wb_gain, &dgain); // ISP Dgain
                            dgain         = dataArray[2];
                            gk_isp_set_rgb_gain(handle,&wb_gain, dgain);
                            //gk_isp_set_ae_dgain(fdmedia,dgain);

                            _printf("gain_index is %d \n", gain_index);
                            _printf("shutter_index is %d \n",
                                shutter_index);
                            _printf("dgain is %d \n", dgain);

                            _printf("Right CHIPPIN %d\n\n",dataArray[3]);
                        }

                        _printf("Exposure controlling Setting\n");

                        break;

                    case AWBModel: // Apply by ChengBo
                        _printf("Set awb_control_mode\n");
                        TCP_get_param(recvBuffer, dataArray);
                        //awb_control_mode = (awb_control_mode_t)dataArray[0];
                        awb_ratio_r = dataArray[0];
                        awb_ratio_b = dataArray[1];
                        AWBspeed    = dataArray[2];

                        if(dataArray[3] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[3]);
                            //KEYMACTH = 0;
                        }else
                        {
                            img_awb_set_rb_ratio(handle, awb_ratio_r,awb_ratio_b);
                            gk_awb_set_speed(handle, AWBspeed);

                            _printf("awb_control_mode %d\n", awb_mode);
                            _printf("AWBspeed %d\n", AWBspeed);
                            _printf("Right CHIPPIN %d\n\n",dataArray[3]);
                        }

                        break;

                    case AEControl:        // Apply by ChengBo
                        _printf("Set AEControl\n");
                        TCP_get_param(recvBuffer, dataArray);
                        AECTarget = dataArray[0];
                        autoKnee = dataArray[1];
                        speedLevel = dataArray[2];
                        AESpeedMode = 0;

                        if(dataArray[3] != CHIPPIN)
                        {
                            _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                            _printf("Wrong CHIPPIN %d\n\n",dataArray[3]);
                            //KEYMACTH = 0;
                        }else
                        {
                            img_ae_set_target(handle, AECTarget);
                            gk_set_ae_auto_luma_control(handle, autoKnee);
                            gk_set_ae_speed(handle, AESpeedMode, speedLevel);

                            _printf("AECTarget %d\n", AECTarget);
                            _printf("autoKnee strength %d\n", autoKnee);
                            _printf("speedLevel%d\n", speedLevel);
                            _printf("Right CHIPPIN %d\n\n",dataArray[3]);
                        }

                        break;
                    }

                    break;

                case VPS:      //id_Require==Apply  ChengBo
                    {
                        recv(new_fd, recvBuffer, 2000, 0);
                        switch (data.layer3) {

                        case stage0:
                            _printf("VPS stage0 Set\n");
                            TCP_get_param(recvBuffer, dataArray);
                            //vps_mode_info = (vps_mode)dataArray[0];

                            //vps_stg0_controller.padding = 0;

                            //img_dsp_set_vps_control_mode(fdmedia, &vps_mode_info);
                            //img_dsp_set_vps_stage0_controller(fdmedia, &vps_stg0_controller);
                            break;

                        case stage1:
                            _printf("VPS stage1 Set\n");
                            TCP_get_param(recvBuffer, dataArray);

                            //for(i = 0; i<=26; i++)
                            //  {printf("%d  ",dataArray[i]);
                            //  }

                            if(dataArray[27] != CHIPPIN)
                            {
                                _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                                _printf("Wrong CHIPPIN %d\n\n",dataArray[27]);
                                //KEYMACTH = 0;
                            }else
                            {
                                _printf("VPS stage1 finished\n");
                                _printf("Right CHIPPIN %d\n\n",dataArray[27]);
                            }


                            break;

                        case stage2:
                            _printf("VPS stage2 Set\n");
                            TCP_get_param(recvBuffer, dataArray);
                            //0x0000  vps_yuv3d_ctrl_reg
                            //0x0001  vps_yuv3d_expand
                            //0x0002  vps_yuv3d_resolution
                            //0x0003  vps_noise
                            //0x0004  vps_filter_th
                            _printf("VPS stage2 finished\n");
                            break;

                        case stage3:
                            _printf("VPS stage3 Set\n");
                            TCP_get_param(recvBuffer, dataArray);

                            _printf("VPS stage3 finished\n");
                            break;
                        }
                    }
                    break;


                case VIDEO:    // id_Require==Apply  ChengBo
                    _printf("VIDEO\n");
                    switch (data.layer3) {
                    case Videopar:
                        TCP_get_param(recvBuffer, dataArray);

                        streamId = dataArray[0];
                        vencbps.id = streamId;
                        vencbps.rate_control_mode = dataArray[1]; //*0: CBR; 1: VBR
                        vencbps.vbr_max_bitrate = dataArray[2];
                        vencbps.vbr_min_bitrate = dataArray[3];
                        vencbps.cbr_avg_bitrate = dataArray[4];
                        vencgop.id = streamId;
                        vencgop.N = dataArray[5];

                        retVal = tuning_venc_set_h264_config(streamId, &vencbps, &vencgop);

                        _printf("Video Parameter set \n");
                        break;

                    case Encoder: // Set Encoder
                        TCP_get_param(recvBuffer, dataArray);
                        encode_param.qp_min_on_I_3a      = dataArray[0];
                        encode_param.qp_max_on_I_3a      = dataArray[1];
                        encode_param.qp_min_on_p_3a      = dataArray[2];
                        encode_param.qp_max_on_p_3a      = dataArray[3];
                        encode_param.aqp_3a              = dataArray[4];
                        encode_param.i_qp_reduce_3a      = dataArray[5];
                        encode_param.p_qp_reduce_3a      = dataArray[6];
                        encode_param.p_intraBiasAdd_3a   = dataArray[7];
                        encode_param.intra_16_16_bias_3a = dataArray[8];
                        encode_param.intra_4_4_bias_3a   = dataArray[9];
                        encode_param.inter_16_16_bias_3a = dataArray[10];
                        encode_param.inter_8_8_bias_3a   = dataArray[11];
                        encode_param.me_lamda_qp_offset_3a  = dataArray[12];
                        encode_param.alpha_3a               = dataArray[13];
                        encode_param.beta_3a                = dataArray[14];
                        encode_param.zmv_thresh_3a          = dataArray[15];

                        encode_param.uv_qp_offset_3a        = dataArray[16];
                        encode_param.IsizeCtlThresh_3a      = dataArray[17];
                        encode_param.IsizeCtlStrength_3a    = dataArray[18];
                        encode_param.reenc_threshStC_3a     = dataArray[19];
                        encode_param.reenc_strengthStC_3a   = dataArray[20];
                        encode_param.reenc_threshCtS_3a     = dataArray[21];
                        encode_param.reenc_strengthCtS_3a   = dataArray[22];
                        encode_param.stream_id              = dataArray[23];

                        gk_isp_set_encode_param(handle, &encode_param);
                        printf("Set Encoder\n");

                        break;

                    case Pgapar:
                        _printf("data.layer3 is %d \n",
                            data.layer3);
                        TCP_get_param(recvBuffer, dataArray);

                        _printf("dataArray[0] is %d \n",
                            dataArray[0]);
                        _printf("dataArray[1] is %d \n",
                            dataArray[1]);
                        _printf("dataArray[2] is %d \n",
                            dataArray[2]);
                        _printf("dataArray[3] is %d \n",
                            dataArray[3]);

                        gain_index = dataArray[0];
                        dgain = dataArray[1];
                        shutter_index = dataArray[2];
                        //fps_t = dataArray[3];
                        gk_set_sensor_gain_index(handle, gain_index,
                            handle->content.publicData.sensor_double_step);
                        //gk_isp_set_rgb_gain(handle, &wb_gain, dgain);
                        gk_set_sensor_shutter_time_index(handle,
                            shutter_index);

                        //gadi_isp_set_vi_frame(PGAchandle,fps_t);

                        _printf("PGA parameter set \n");
                        break;

                    case LowBit: // Set LowBit
                        {
                            _printf("Set LowBit\n");
                            TCP_get_param(recvBuffer, dataArray);
                            image_low_bitrate_enable = dataArray[0];

                            image_low_bitrate.sharpness_low_bitrate_ratio                   = dataArray[1];
                            //image_low_bitrate.dnr3d_low_bitrate_ratio                       = dataArray[2];
                            //image_low_bitrate.vps_3d_low_bitrate_ratio                      = dataArray[3];
                            //image_low_bitrate.vps_contrast_low_bitrate_ratio                = dataArray[4];
                            image_low_bitrate.sharpen_property_low_bitrate_up_ratio         = dataArray[5];
                            image_low_bitrate.sharpen_property_low_bitrate_down_ratio       = dataArray[6];
                            image_low_bitrate.cfa_denoise_property_low_bitrate_up_ratio     = dataArray[7];
                            image_low_bitrate.cfa_denoise_property_low_bitrate_down_ratio   = dataArray[8];
                            //image_low_bitrate.dnr3d_property_low_bitrate_ratio              = dataArray[9];

                            image_low_bitrate.sharpness_low_bitrate_param                   = dataArray[10];
                            //image_low_bitrate.dnr3d_low_bitrate_param                       = dataArray[11];
                            //image_low_bitrate.vps_3d_low_bitrate_param                      = dataArray[12];
                            //image_low_bitrate.vps_contrast_low_bitrate_param                = dataArray[13];
                            image_low_bitrate.sharpen_property_low_bitrate_up_param         = dataArray[14];
                            image_low_bitrate.sharpen_property_low_bitrate_down_param       = dataArray[15];
                            image_low_bitrate.cfa_denoise_property_low_bitrate_up_param     = dataArray[16];
                            image_low_bitrate.cfa_denoise_property_low_bitrate_down_param   = dataArray[17];
                            //image_low_bitrate.dnr3d_property_low_bitrate_param              = dataArray[18];

                            tolerance_low_bitrate                                           = dataArray[19];
                            delay_frames_low_bitrate                                        = dataArray[20];

                            gk_set_low_bitrate_mode(handle, image_low_bitrate_enable);
                            gk_set_low_bitrate_param(handle, &image_low_bitrate,
                                &tolerance_low_bitrate, &delay_frames_low_bitrate);

#if 1

                            _printf("sharpness_low_bitrate_ratio                 : %d\n",image_low_bitrate.sharpness_low_bitrate_ratio                   );
                            //_printf("dnr3d_low_bitrate_ratio                     : %d\n",image_low_bitrate.dnr3d_low_bitrate_ratio                       );
                            //_printf("vps_3d_low_bitrate_ratio                    : %d\n",image_low_bitrate.vps_3d_low_bitrate_ratio                      );
                            //_printf("vps_contrast_low_bitrate_ratio              : %d\n",image_low_bitrate.vps_contrast_low_bitrate_ratio                );
                            _printf("sharpen_property_low_bitrate_up_ratio       : %d\n",image_low_bitrate.sharpen_property_low_bitrate_up_ratio         );
                            _printf("sharpen_property_low_bitrate_down_ratio     : %d\n",image_low_bitrate.sharpen_property_low_bitrate_down_ratio       );
                            _printf("cfa_denoise_property_low_bitrate_up_ratio   : %d\n",image_low_bitrate.cfa_denoise_property_low_bitrate_up_ratio     );
                            _printf("cfa_denoise_property_low_bitrate_down_ratio : %d\n",image_low_bitrate.cfa_denoise_property_low_bitrate_down_ratio   );
                            //_printf("dnr3d_property_low_bitrate_ratio            : %d\n",image_low_bitrate.dnr3d_property_low_bitrate_ratio              );

                            _printf("sharpness_low_bitrate_param                 : %d\n",image_low_bitrate.sharpness_low_bitrate_param                   );
                            //_printf("dnr3d_low_bitrate_param                     : %d\n",image_low_bitrate.dnr3d_low_bitrate_param                       );
                            //_printf("vps_3d_low_bitrate_param                    : %d\n",image_low_bitrate.vps_3d_low_bitrate_param                      );
                            //_printf("vps_contrast_low_bitrate_param              : %d\n",image_low_bitrate.vps_contrast_low_bitrate_param                );
                            _printf("sharpen_property_low_bitrate_up_param       : %d\n",image_low_bitrate.sharpen_property_low_bitrate_up_param         );
                            _printf("sharpen_property_low_bitrate_down_param     : %d\n",image_low_bitrate.sharpen_property_low_bitrate_down_param       );
                            _printf("cfa_denoise_property_low_bitrate_up_param   : %d\n",image_low_bitrate.cfa_denoise_property_low_bitrate_up_param     );
                            _printf("cfa_denoise_property_low_bitrate_down_param : %d\n",image_low_bitrate.cfa_denoise_property_low_bitrate_down_param   );
                            //_printf("dnr3d_property_low_bitrate_param            : %d\n",image_low_bitrate.dnr3d_property_low_bitrate_param              );

                            _printf("tolerance_low_bitrate                       : %d\n",tolerance_low_bitrate);
                            _printf("delay_frames_low_bitrate                    : %d\n",delay_frames_low_bitrate);

#endif

                        }
                        break;

                    }

                    break;

                case WDR: // id_Require==Apply  ChengBo
                    {
                        _printf("WDR set\n");
                        switch (data.layer3)
                        {
                        case globalwdr:
                            _printf("globalwdr set\n");
                            //int select = 0;
                            //int number = 0;
                            int number1 = 0;
                            int number2 = 0;
                            int number3 = 0;
                            int number4 = 0;

                            WDRGet     = 0;
                            //recv(new_fd,select,1,0);
                            //recv(new_fd,recvBuffer,3,0);
                            //TCP_get_param( recvBuffer,    dataArray);
                            //_printf("recvBuffer[0] %c\n",recvBuffer[0]);
                            //_printf("dataArray[0] %d\n",dataArray[0]);

                            //select = dataArray[0];

                            number1 = recv(new_fd,matrix1,17536,0);
                            number2 = recv(new_fd,matrix2,17536,0);
                            number3 = recv(new_fd,matrix3,17536,0);
                            number4 = recv(new_fd,matrix4,17536,0);

                            if (
                                number1 != WDRSize ||
                                number2 != WDRSize ||
                                number3 != WDRSize ||
                                number4 != WDRSize
                                )
                            {

                                WDRGet = 0;
                            }
                            else
                            {
                                color_cc1.matrix_3d_table_addr =(u32)matrix1;
                                color_cc2.matrix_3d_table_addr =(u32)matrix2;
                                color_cc3.matrix_3d_table_addr =(u32)matrix3;
                                color_cc4.matrix_3d_table_addr =(u32)matrix4;
                                gk_set_auto_contrast(handle,0);
                                tuning_set_color_correction(handle,&color_cc1,&color_cc2,&color_cc3,&color_cc4);
                                WDRGet = 1;
                            }

                            break;
                        case localwdr:
                            _printf("localwdr set\n");
                            TCP_get_param(recvBuffer, dataArray);
                            tuning_LE_str.str = dataArray[0];
                            tuning_LE_str.low_or_high =
                                dataArray[1];
                            tuning_LE_str.enable = dataArray[2];
                            tuning_LE_str.radius = dataArray[3];
                            tuning_LE_str.luma_weight_shift = dataArray[4];
                            tuning_LE_str.luma_weight_red   = dataArray[5];
                            tuning_LE_str.luma_weight_green = dataArray[6];
                            tuning_LE_str.luma_weight_blue  = dataArray[7];

                            if(dataArray[8] != CHIPPIN)
                            {
                                _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                                _printf("Wrong CHIPPIN %d\n\n",dataArray[8]);
                                //KEYMACTH = 0;
                            }else
                            {
                                tuning_set_LE_str(handle,
                                    &tuning_LE_str);

                                _printf("tuning_LE_str.enable               %d\n",tuning_LE_str.enable);
                                _printf("tuning_LE_str.radius               %d\n",tuning_LE_str.radius);
                                _printf("tuning_LE_str.luma_weight_shift    %d\n",tuning_LE_str.luma_weight_shift);
                                _printf("tuning_LE_str.luma_weight_red      %d\n",tuning_LE_str.luma_weight_red);
                                _printf("tuning_LE_str.luma_weight_green    %d\n",tuning_LE_str.luma_weight_green);
                                _printf("tuning_LE_str.luma_weight_blue     %d\n",tuning_LE_str.luma_weight_blue);

                                _printf("Right CHIPPIN %d\n\n",dataArray[8]);
                            }


#if 0
                            _printf("tuning_LE_str.enable               %d\n",tuning_LE_str.enable);
                            _printf("tuning_LE_str.radius               %d\n",tuning_LE_str.radius);
                            _printf("tuning_LE_str.luma_weight_shift    %d\n",tuning_LE_str.luma_weight_shift);
                            _printf("tuning_LE_str.luma_weight_red      %d\n",tuning_LE_str.luma_weight_red);
                            _printf("tuning_LE_str.luma_weight_green    %d\n",tuning_LE_str.luma_weight_green);
                            _printf("tuning_LE_str.luma_weight_blue     %d\n",tuning_LE_str.luma_weight_blue);
#endif


                            break;

                        case LensDis:
                            _printf("Lense Warp set\n");
                            TCP_get_param( recvBuffer,  dataArray);

                            if(dataArray[4] != CHIPPIN)
                            {
                                _printf("Set cmd %d%d%d%d\n\n",id_Require,data.layer1,data.layer2,data.layer3);
                                _printf("Wrong CHIPPIN %d\n\n",dataArray[4]);
                                //KEYMACTH = 0;
                            }else
                            {
                                warp_config_t bin_warp_config;

                                bin_warp_config.warp_control            = dataArray[0];
                                bin_warp_config.warp_multiplier         = dataArray[1];
                                bin_warp_config.vert_warp_enable        = dataArray[2];
                                bin_warp_config.vert_warp_multiplier    = dataArray[3];

                                img_dsp_set_dewarp(handle,&bin_warp_config);

                                _printf("Hori Config : %d\n",bin_warp_config.warp_control);
                                _printf("Hori Level  : %d\n",bin_warp_config.warp_multiplier);
                                _printf("Vert Config : %d\n",bin_warp_config.vert_warp_enable);
                                _printf("Hori Level  : %d\n",bin_warp_config.vert_warp_multiplier);

                                _printf("Right CHIPPIN %d\n\n",dataArray[4]);
                            }

                            break;

                        case LensShade:
                            _printf("Lense Shade set\n");
                            TCP_get_param( recvBuffer,  dataArray);
                            u16 *raw_buff;
                            u32 lookup_shift;
                            CAP_RAW_InfoT raw_info_lense;
                            u16 vignette_table[33 * 33 * 4] = { 0 };
                            blc_level_t Lensblc;
                            still_cap_info_t still_cap_info;
                            vignette_cal_t vig_detect_setup;
                            vignette_info_t vignette_info = { 0 };
                            int fd_lenshading = -1;
                            int rval = -1;

                            detect_flag      = dataArray[0];
                            correct_flag     = dataArray[1];
                            restore_flag     = dataArray[2];
                            compensate_ratio = dataArray[3];

                            if(correct_flag == 1)
                            {
                                gk_set_lens_shading_enable(handle, 1);
                            }
                            else
                            {
                                gk_set_lens_shading_enable(handle, 0);
                            }

#if  1
                            _printf("detect_flag  %d\n",detect_flag);
                            _printf("correct_flag %d\n",correct_flag);
                            _printf("restore_flag %d\n",restore_flag);

#endif
                            if (filename[0] == '\0'){
                                strcpy(filename, default_filename);
                            }

                            if (detect_flag) {
                                detect_finished = 0;
                                shutter_index = gk_get_sensor_shutter_time_index(handle);
                                ae_target = gk_ae_get_target_ratio(handle);
                                gk_ae_get_antiflicker(handle);

                                gk_enable_ae(handle, 0);
                                gk_enable_awb(handle, 0);
                                gk_enable_af(handle, 0);
                                gk_enable_auto_param_ctl(handle, 0);
                                sleep(1);

                                _printf("shutter_index = %d\n", shutter_index);
                                _printf("ae_target = %d\n", ae_target);
                                _printf("Light strength is okay.\n");

                                //Capture raw here
                                _printf("Raw capture started\n");
                                memset(&still_cap_info, 0, sizeof(still_cap_info));
                                still_cap_info.capture_num = 1;
                                still_cap_info.need_raw = 1;

                                _printf("tuning_venc_stop_stream\n");
                                tuning_venc_stop_stream();
                                if (ioctl(fdmedia, GK_MEDIA_IOC_ENABLE_PREVIEW) < 0) {
                                    _printf("tunning: enable preview failed");
                                    return NULL;
                                }
                                _printf("gk_init_image_capture \n");
                                gk_init_image_capture(handle, 50);
                                _printf("gk_start_image_capture \n");
                                gk_start_image_capture(handle, &still_cap_info);

#if 0
                                printf("GK_MEDIA_IOC_MAP_DSP \n");
                                rval = ioctl(fdmedia, GK_MEDIA_IOC_MAP_DSP, &mmap_info);
                                if (rval < 0) {
                                    printf("GK_MEDIA_IOC_MAP_DSP Error\n");
                                    return NULL;
                                }
#endif

                                printf("GK_MEDIA_IOC_READ_RAW_INFO Start\n");
                                raw_info_lense.src_id = 0;
                                rval = ioctl(fdmedia, GK_MEDIA_IOC_READ_RAW_INFO, &raw_info_lense);
                                if (rval < 0) {
                                    _printf("GK_MEDIA_IOC_READ_RAW_INFO");
                                    //return NULL;
                                }

                                _printf("GK_MEDIA_IOC_READ_RAW_INFO End\n");

                                tuning_capture_get_jpeg_stream();
                                _printf("get_jpeg_stream() \n\n");

                                raw_buff =
                                    (u16 *) malloc(raw_info_lense.width * raw_info_lense.height * sizeof(u16));

                                memcpy(raw_buff, raw_info_lense.raw_addr,
                                    (raw_info_lense.width * raw_info_lense.height * 2));


                                /*input */
                                vig_detect_setup.raw_addr = raw_buff;
                                vig_detect_setup.raw_w = raw_info_lense.width;
                                vig_detect_setup.raw_h = raw_info_lense.height;
                                vig_detect_setup.bp = raw_info_lense.bayer_pattern;
                                vig_detect_setup.threshold = 8192;
                                //vig_detect_setup.compensate_ratio = 1024;

                                vig_detect_setup.compensate_ratio = compensate_ratio;
                                vig_detect_setup.lookup_shift = 255;

                                /*output */
                                vig_detect_setup.r_tab = vignette_table + 0 * VIGNETTE_MAX_SIZE;
                                vig_detect_setup.ge_tab = vignette_table + 1 * VIGNETTE_MAX_SIZE;
                                vig_detect_setup.go_tab = vignette_table + 2 * VIGNETTE_MAX_SIZE;
                                vig_detect_setup.b_tab = vignette_table + 3 * VIGNETTE_MAX_SIZE;
                                gk_isp_get_final_blc(handle, &Lensblc);
                                vig_detect_setup.blc.r_offset = Lensblc.r_offset;
                                vig_detect_setup.blc.gr_offset = Lensblc.gr_offset;
                                vig_detect_setup.blc.gb_offset = Lensblc.gb_offset;
                                vig_detect_setup.blc.b_offset = Lensblc.b_offset;

                                _printf("r_offset:%d; \n",vig_detect_setup.blc.r_offset);
                                _printf("gr_offset:%d; \n",vig_detect_setup.blc.gr_offset);
                                _printf("gb_offset:%d; \n",vig_detect_setup.blc.gb_offset);
                                _printf("b_offset: %d;\n",vig_detect_setup.blc.b_offset);

                                _printf("Cali lens shading started\n");
                                rval = gk_cali_lens_shading(&vig_detect_setup);
                                if (rval < 0) {
                                    //printf("gk_cali_lens_shading error!\n");
                                    //goto vignette_cal_exit;
                                }

                                lookup_shift = vig_detect_setup.lookup_shift;

                                if ((fd_lenshading =
                                    open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0777)) < 0) {
                                        _printf("vignette table file open error!\n");
                                        //goto vignette_cal_exit;
                                }
                                rval =
                                    write(fd_lenshading, vignette_table,
                                    (4 * VIGNETTE_MAX_SIZE * sizeof(u16)));
                                if (rval < 0) {
                                    _printf("vignette table file write error!\n");
                                    //goto vignette_cal_exit;
                                }

                                rval = write(fd_lenshading, &lookup_shift, sizeof(lookup_shift));
                                if (rval < 0) {
                                    _printf("vignette table file write error!\n");
                                    //goto vignette_cal_exit;
                                }

                                rval = write(fd_lenshading, &raw_info_lense.width, sizeof(raw_info_lense.width));
                                if (rval < 0) {
                                    _printf("vignette table file write error!\n");
                                    //goto vignette_cal_exit;
                                }

                                rval = write(fd_lenshading, &raw_info_lense.height, sizeof(raw_info_lense.height));
                                if (rval < 0) {
                                    _printf("vignette table file write error!\n");
                                    //goto vignette_cal_exit;
                                }
                                close(fd_lenshading);
                                _printf("Cali lens shading stopped\n");

                                _printf("gk_stop_image_capture\n");
                                rval = gk_stop_image_capture(handle);
                                if (rval < 0) {
                                    _printf("gk_stop_image_capture error!\n");
                                    //goto vignette_cal_exit;
                                }

                                if (ioctl(fdmedia, GK_MEDIA_IOC_LEAVE_STILL_CAPTURE) < 0) {
                                    _printf("tunning: leave capture picture failed");
                                    return NULL;
                                }
                                tuning_venc_start_stream();

                                _printf("vignette_cal_exit\n");
                                free(raw_buff);
                                detect_finished = 1;
#if 0
vignette_cal_exit:
                                printf("vignette_cal_exit Goto\n");
                                free(raw_buff);
                                printf("gk_stop_image_capture\n");
                                gk_stop_image_capture(fdmedia);

                                if (ioctl(fdmedia, GK_MEDIA_IOC_LEAVE_STILL_CAPTURE) < 0) {
                                    printf("tunning: leave capture picture failed");
                                    return NULL;
                                }

                                printf("tuning_venc_start_stream Start\n");
                                tuning_venc_start_stream();
                                printf("tuning_venc_start_stream Start\n");

                                return NULL;
#endif

                            }

                            if (correct_flag || restore_flag) {
                                int count;
                                static u32 gain_shift;

                                gk_get_3A_ctl_state(handle, &aaa_cntl_station);

                                if (aaa_cntl_station.ae_enable == 0) {
                                    gk_enable_ae(handle, 1);
                                }

                                if (aaa_cntl_station.awb_enable == 0) {
                                    gk_enable_awb(handle,1);
                                }

                                if (aaa_cntl_station.af_enable == 0) {
                                    gk_enable_af(handle,1);
                                }

                                if (aaa_cntl_station.adj_enable == 0) {
                                    gk_enable_auto_param_ctl(handle,1);
                                }

                                if (correct_flag) {
                                    if ((fd_lenshading = open(filename, O_RDONLY, 0)) < 0) {
                                        _printf("lens_shading.bin cannot be opened\n");
                                        return NULL;
                                    }
                                    count =
                                        read(fd_lenshading, vignette_table,
                                        4 * VIGNETTE_MAX_SIZE * sizeof(u16));
                                    if (count != 4 * VIGNETTE_MAX_SIZE * sizeof(u16)) {
                                        _printf("read lens_shading.bin error\n");
                                        return NULL;
                                    }
                                    count = read(fd_lenshading, &gain_shift, sizeof(u32));
                                    if (count != sizeof(u32)) {
                                        _printf("read lens_shading.bin error\n");
                                        return NULL;
                                    }
                                    vignette_info.enable = 1;
                                } else if (restore_flag) {
                                    vignette_info.enable = 0;
                                }

                                _printf("vignette_info.enable = %d\n",vignette_info.enable);
                                vignette_info.gain_shift = (u8) gain_shift;
                                vignette_info.vignette_red_gain_addr =
                                    (u32) (vignette_table + 0 * VIGNETTE_MAX_SIZE);
                                vignette_info.vignette_green_even_gain_addr =
                                    (u32) (vignette_table + 1 * VIGNETTE_MAX_SIZE);
                                vignette_info.vignette_green_odd_gain_addr =
                                    (u32) (vignette_table + 2 * VIGNETTE_MAX_SIZE);
                                vignette_info.vignette_blue_gain_addr =
                                    (u32) (vignette_table + 3 * VIGNETTE_MAX_SIZE);

                                _printf("Comp lens shading start\n");
                                rval = gk_isp_set_lens_shading_compensation(handle, &vignette_info);

                                if (rval < 0) {
                                    _printf("gk_isp_set_lens_shading_compensation error!\n");
                                    return NULL;
                                }
                            }

                            break;

                        case DEMOSAIC:
                            {
                                _printf("DEMOSAIC set\n");
                                TCP_get_param( recvBuffer,  dataArray);
                                demosaic_noise_reduction.thresh_grad_clipping   = dataArray[0];
                                demosaic_noise_reduction.thresh_grad_noise      = dataArray[1];
                                demosaic_noise_reduction.thresh_activity        = dataArray[2];
                                demosaic_noise_reduction.thresh_activity_diff   = dataArray[3];
                                demosaic_noise_reduction.threshedge             = dataArray[4];
                                demosaic_noise_reduction.shift                  = dataArray[5];
                                demosaic_noise_reduction.demoire_A_Y            = dataArray[6];
                                demosaic_noise_reduction.demoire_B_Y            = dataArray[7];
                                demosaic_noise_reduction.demoire_C_Y            = dataArray[8];
                                demosaic_noise_reduction.demoire_D_Y            = dataArray[9];
                                demosaic_noise_reduction.demoire_A_C            = dataArray[10];
                                demosaic_noise_reduction.demoire_B_C            = dataArray[11];
                                demosaic_noise_reduction.demoire_C_C            = dataArray[12];
                                demosaic_noise_reduction.demoire_D_C            = dataArray[13];
                                demosaic_noise_reduction.conti_th1              = dataArray[14];
                                demosaic_noise_reduction.conti_th2              = dataArray[15];
                                demosaic_noise_reduction.conti_th3              = dataArray[16];
                                demosaic_noise_reduction.conti_th4              = dataArray[17];

                                gk_isp_set_demosaic_noise_reduction_new(handle, &demosaic_noise_reduction);
                            }
                            break;

                        }
                    }
                    break;
                case Denoise:
					{
						_printf("Denoise set\n");
                        switch (data.layer3)
                        {
							case CFAdenoise:
								{
									_printf("NLM CFAdenoise set\n");
                                    TCP_get_param( recvBuffer,  dataArray);
									nlm_noise_filter_info.nlm_noise_filter_enable				= dataArray[0];
									nlm_noise_filter_info.nlm_noise_filter_mv_enable			= dataArray[1];
									nlm_noise_filter_info.nlm_noise_filter_luma_offset_red		= dataArray[2];
									nlm_noise_filter_info.nlm_noise_filter_luma_offset_green	= dataArray[3];
									nlm_noise_filter_info.nlm_noise_filter_luma_offset_blue 	= dataArray[4];
									nlm_noise_filter_info.nlm_noise_filter_strength_red 		= dataArray[5];
									nlm_noise_filter_info.nlm_noise_filter_strength_green		= dataArray[6];
									nlm_noise_filter_info.nlm_noise_filter_strength_blue		= dataArray[7];
									nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_red	= dataArray[8];
									nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_green = dataArray[9];
									nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_blue	= dataArray[10];
									nlm_noise_filter_info.nlm_noise_filter_mv_strength_red		= dataArray[11];
									nlm_noise_filter_info.nlm_noise_filter_mv_strength_green	= dataArray[12];
									nlm_noise_filter_info.nlm_noise_filter_mv_strength_blue 	= dataArray[13];
									nlm_noise_filter_info.nlm_noise_filter_factor				= dataArray[14];
                                    nlm_noise_filter_info.nlm_max_change                        = dataArray[15];

									gk_isp_set_nlm_noise_filter(handle,&nlm_noise_filter_info);
								}
								break;

							case VGFdenoise:
								{
									_printf("VGFdenoise set\n");
									TCP_get_param( recvBuffer,  dataArray);
									variance_guided_filter_info.vgf_enable			   = dataArray[0];
									variance_guided_filter_info.vgf_bpc_enable		   = dataArray[1];
									variance_guided_filter_info.vgf_mv_enable		   = dataArray[2];
									variance_guided_filter_info.vgf_mv_offset		   = dataArray[3];
									variance_guided_filter_info.vgf_cutoff_th1		   = dataArray[4];
									variance_guided_filter_info.vgf_cutoff_value1	   = dataArray[5];
									variance_guided_filter_info.vgf_cutoff_th2		   = dataArray[6];
									variance_guided_filter_info.vgf_cutoff_value2	   = dataArray[7];
									variance_guided_filter_info.vgf_bpc_vgf_th		   = dataArray[8];
									variance_guided_filter_info.vgf_bpc_w_th		   = dataArray[9];
									variance_guided_filter_info.vgf_bpc_b_th		   = dataArray[10];
									variance_guided_filter_info.vgf_edge_th 		   = dataArray[11];
									variance_guided_filter_info.vgf_edge_wide_weight   = dataArray[12];
									variance_guided_filter_info.vgf_edge_narrow_weight = dataArray[13];
									variance_guided_filter_info.vgf_grad_th 		   = dataArray[14];
									variance_guided_filter_info.vgf_disp_info_en	   = dataArray[15];
									variance_guided_filter_info.vgf_disp_var_th1	   = dataArray[16];
									variance_guided_filter_info.vgf_disp_var_th2	   = dataArray[17];
									variance_guided_filter_info.vgf_disp_grad_th1	   = dataArray[18];
									variance_guided_filter_info.vgf_disp_grad_th2	   = dataArray[19];
									variance_guided_filter_info.vgf_edge_var_th 	   = dataArray[20];
									variance_guided_filter_info.vgf_edge_str_th1       = dataArray[21];
                                    variance_guided_filter_info.edge_offset            = dataArray[22];

                                    variance_guided_filter_info.lut_offset_r           = dataArray[23];
                                    variance_guided_filter_info.lut_offset_g           = dataArray[24];
                                    variance_guided_filter_info.lut_offset_b           = dataArray[25];

                                    variance_guided_filter_info.strength               = dataArray[26];

									//gk_isp_set_variance_guided_filter(handle, &variance_guided_filter_info);
									gk_set_af_focus(handle,variance_guided_filter_info.vgf_cutoff_th2);
								}
								break;

							case BLFdenoise:
								{
									_printf("BLFdenoise set\n");
                                    TCP_get_param( recvBuffer,  dataArray);
                                    bilateral_filter_info.y_enable = dataArray[0];
                                    bilateral_filter_info.c_enable = dataArray[1];
                                    bilateral_filter_info.use_mv   = dataArray[2];

                                    bilateral_filter_info.simr_str    = dataArray[3];
                                    bilateral_filter_info.sims_str    = dataArray[4];
                                    bilateral_filter_info.simr_mv_str = dataArray[5];
                                    bilateral_filter_info.sims_mv_str = dataArray[6];

                                    bilateral_filter_info.adj_enable    = dataArray[7];
                                    bilateral_filter_info.th_enable    = dataArray[8];
                                    bilateral_filter_info.multi_coff = dataArray[9];
                                    bilateral_filter_info.min_mv_str = dataArray[10];


                                    gk_isp_set_bilateral_filter(handle, &bilateral_filter_info);
								}
								break;

							case GMDCFAdenoise:
								{
									_printf("GMDCFAdenoise set\n");
                                    TCP_get_param( recvBuffer,  dataArray);
                                    gmd_cfa_info.en                             = dataArray[0];
                                    gmd_cfa_info.gamma_en                       = dataArray[1];
                                    gmd_cfa_info.blur_en_cfa                    = dataArray[2];
                                    gmd_cfa_info.cfa_interp_en                  = dataArray[3];
                                    gmd_cfa_info.noise_cfa                      = dataArray[4];
                                    gmd_cfa_info.black_level_globe_offset_green = dataArray[5];
                                    gmd_cfa_info.digital_gain_multiplier_green  = dataArray[6];
                                    gmd_cfa_info.digital_gain_shift_green       = dataArray[7];
									gk_isp_set_gmd_cfa(handle,&gmd_cfa_info);
								}
								break;

							case PMVdenoise:
								{
									_printf("PMVdenoise set\n");
                                    TCP_get_param( recvBuffer,  dataArray);

                                    pmv_info.bpc_cor_enable     = dataArray[0];
                                    pmv_info.psc_cor_enable     = dataArray[1];
                                    pmv_info.lea_expand_enable  = dataArray[2];
                                    pmv_info.dpf_exphor1_enable = dataArray[3];
                                    pmv_info.dpf_expver_enable  = dataArray[4];

                                    pmv_info.bpc_cor_range      = dataArray[5];
                                    pmv_info.bpc_cor_thresh     = dataArray[6];
                                    pmv_info.psc_cor_range      = dataArray[7];
                                    pmv_info.psc_feedthrough    = dataArray[8];
                                    pmv_info.lea_expand_range   = dataArray[9];
                                    pmv_info.dpf_exphor1_range  = dataArray[10];
                                    pmv_info.dpf_expver_range   = dataArray[11];

									gk_isp_set_pmv(handle,&pmv_info);
								}
								break;

							case GMF3Ddenoise:
								{
									_printf("GMF3Ddenoise set\n");
                                    TCP_get_param( recvBuffer,  dataArray);

                                    globle_motion_filter_info.enable      = dataArray[0];
                                    globle_motion_filter_info.gmd_sel     = dataArray[1];
                                    globle_motion_filter_info.lpf_enable  = dataArray[2];
                                    globle_motion_filter_info.noise_l     = dataArray[3];
                                    globle_motion_filter_info.noise_h     = dataArray[4];
                                    globle_motion_filter_info.lpf3x3      = dataArray[5];
                                    globle_motion_filter_info.lpf_str     = dataArray[6];
                                    globle_motion_filter_info.maxchange_y_static = dataArray[7];
                                    globle_motion_filter_info.maxchange_c_static = dataArray[8];
									globle_motion_filter_info.maxchange_y_move   = dataArray[9];
                                    globle_motion_filter_info.maxchange_c_move   = dataArray[10];

									globle_motion_filter_info.static_alpha_y_max	= dataArray[11];
									globle_motion_filter_info.static_alpha_y_min	= dataArray[12];
									globle_motion_filter_info.static_threshold_y_1	= dataArray[13];
									globle_motion_filter_info.static_threshold_y_2	= dataArray[14];
									globle_motion_filter_info.static_alpha_uv_max	= dataArray[15];
									globle_motion_filter_info.static_alpha_uv_min	= dataArray[16];
									globle_motion_filter_info.static_threshold_uv_1 = dataArray[17];
									globle_motion_filter_info.static_threshold_uv_2 = dataArray[18];
									globle_motion_filter_info.move_alpha_y_max		= dataArray[19];
									globle_motion_filter_info.move_alpha_y_min		= dataArray[20];
									globle_motion_filter_info.move_threshold_y_1	= dataArray[21];
									globle_motion_filter_info.move_threshold_y_2	= dataArray[22];
									globle_motion_filter_info.move_alpha_uv_max 	= dataArray[23];
									globle_motion_filter_info.move_alpha_uv_min 	= dataArray[24];
									globle_motion_filter_info.move_threshold_uv_1	= dataArray[25];
									globle_motion_filter_info.move_threshold_uv_2	= dataArray[26];

                                    globle_motion_filter_info.adj_enable    = dataArray[27];
                                    globle_motion_filter_info.th_enable    = dataArray[28];

                                    globle_motion_filter_info.alpha_multi_coff      = dataArray[29];
                                    globle_motion_filter_info.max_change_multi_coff = dataArray[30];
                                    globle_motion_filter_info.alpha_th              = dataArray[31];
                                    globle_motion_filter_info.max_change_th         = dataArray[32];


									globle_motion_filter_info.exp_str_y 			 = dataArray[33];
									globle_motion_filter_info.exp_str_uv			 = dataArray[34];

                                    globle_motion_filter_info.mv_percent_th1	 = dataArray[35];
									globle_motion_filter_info.static_alpha_y_max_2	 = dataArray[36];
									globle_motion_filter_info.static_alpha_y_min_2	 = dataArray[37];
									globle_motion_filter_info.static_threshold_y_12  = dataArray[38];
									globle_motion_filter_info.static_threshold_y_22  = dataArray[39];
									globle_motion_filter_info.static_alpha_uv_max_2  = dataArray[40];
									globle_motion_filter_info.static_alpha_uv_min_2  = dataArray[41];
									globle_motion_filter_info.static_threshold_uv_12 = dataArray[42];
									globle_motion_filter_info.static_threshold_uv_22 = dataArray[43];

									globle_motion_filter_info.static_alpha_y_max_3	 = dataArray[44];
									globle_motion_filter_info.static_alpha_y_min_3	 = dataArray[45];
									globle_motion_filter_info.static_threshold_y_13  = dataArray[46];
									globle_motion_filter_info.static_threshold_y_23  = dataArray[47];
									globle_motion_filter_info.static_alpha_uv_max_3  = dataArray[48];
									globle_motion_filter_info.static_alpha_uv_min_3  = dataArray[49];
									globle_motion_filter_info.static_threshold_uv_13 = dataArray[50];
									globle_motion_filter_info.static_threshold_uv_23 = dataArray[51];

									globle_motion_filter_info.move_alpha_y_max_2	 = dataArray[52];
									globle_motion_filter_info.move_alpha_y_min_2	 = dataArray[53];
									globle_motion_filter_info.move_threshold_y_12	 = dataArray[54];
									globle_motion_filter_info.move_threshold_y_22	 = dataArray[55];
									globle_motion_filter_info.move_alpha_uv_max_2	 = dataArray[56];
									globle_motion_filter_info.move_alpha_uv_min_2	 = dataArray[57];
									globle_motion_filter_info.move_threshold_uv_12	 = dataArray[58];
									globle_motion_filter_info.move_threshold_uv_22	 = dataArray[59];

									globle_motion_filter_info.move_alpha_y_max_3	 = dataArray[60];
									globle_motion_filter_info.move_alpha_y_min_3	 = dataArray[61];
									globle_motion_filter_info.move_threshold_y_13	 = dataArray[62];
									globle_motion_filter_info.move_threshold_y_23	 = dataArray[63];
									globle_motion_filter_info.move_alpha_uv_max_3	 = dataArray[64];
									globle_motion_filter_info.move_alpha_uv_min_3	 = dataArray[65];
									globle_motion_filter_info.move_threshold_uv_13	 = dataArray[66];
									globle_motion_filter_info.move_threshold_uv_23	 = dataArray[67];
                                    globle_motion_filter_info.mv_percent_th1= dataArray[68];
                                    globle_motion_filter_info.mv_percent_th2= dataArray[69];
                                   globle_motion_filter_info.mv_percent_th3        = dataArray[70];
                                    //globle_motion_filter_info.mv_count              = dataArray[71];
									globle_motion_filter_info.mv_delta              = dataArray[71];
                                    globle_motion_filter_info.mv_count              = dataArray[72];
									globle_motion_filter_info.luma_diff_th1              = dataArray[73];
                                    globle_motion_filter_info.luma_diff_th2              = dataArray[74];

									//printf("globle_motion_filter_info.gmf_count =%d %d %d %d\n",globle_motion_filter_info.gmf_count,
									//	globle_motion_filter_info.mv_percent_th1,globle_motion_filter_info.mv_percent_th2,globle_motion_filter_info.gmf_range);


                                    /*globle_motion_filter_info.static_alpha_y_max_2	= dataArray[11];
									globle_motion_filter_info.static_alpha_y_min_2	= dataArray[12];
									globle_motion_filter_info.static_threshold_y_12	= dataArray[13];
									globle_motion_filter_info.static_threshold_y_22	= dataArray[14];
									globle_motion_filter_info.static_alpha_uv_max_2	= dataArray[15];
									globle_motion_filter_info.static_alpha_uv_min_2	= dataArray[16];
									globle_motion_filter_info.static_threshold_uv_12 = dataArray[17];
									globle_motion_filter_info.static_threshold_uv_22 = dataArray[18];
									globle_motion_filter_info.move_alpha_y_max_2		= dataArray[19];
									globle_motion_filter_info.move_alpha_y_min_2		= dataArray[20];
									globle_motion_filter_info.move_threshold_y_12	= dataArray[21];
									globle_motion_filter_info.move_threshold_y_22	= dataArray[22];
									globle_motion_filter_info.move_alpha_uv_max_2 	= dataArray[23];
									globle_motion_filter_info.move_alpha_uv_min_2 	= dataArray[24];
									globle_motion_filter_info.move_threshold_uv_12	= dataArray[25];
									globle_motion_filter_info.move_threshold_uv_22	= dataArray[26];

                                    globle_motion_filter_info.static_alpha_y_max_3	= dataArray[11];
									globle_motion_filter_info.static_alpha_y_min_3	= dataArray[12];
									globle_motion_filter_info.static_threshold_y_13	= dataArray[13];
									globle_motion_filter_info.static_threshold_y_23	= dataArray[14];
									globle_motion_filter_info.static_alpha_uv_max_3	= dataArray[15];
									globle_motion_filter_info.static_alpha_uv_min_3	= dataArray[16];
									globle_motion_filter_info.static_threshold_uv_13 = dataArray[17];
									globle_motion_filter_info.static_threshold_uv_23 = dataArray[18];
									globle_motion_filter_info.move_alpha_y_max_3		= dataArray[19];
									globle_motion_filter_info.move_alpha_y_min_3		= dataArray[20];
									globle_motion_filter_info.move_threshold_y_13	= dataArray[21];
									globle_motion_filter_info.move_threshold_y_23	= dataArray[22];
									globle_motion_filter_info.move_alpha_uv_max_3 	= dataArray[23];
									globle_motion_filter_info.move_alpha_uv_min_3 	= dataArray[24];
									globle_motion_filter_info.move_threshold_uv_13	= dataArray[25];
									globle_motion_filter_info.move_threshold_uv_23	= dataArray[26];*/

                                    gk_isp_set_globle_motion_filter(handle, &globle_motion_filter_info);
								}
								break;
                            case LSHdenoise:
                                {
                                    _printf("LSHdenoise set\n");
									TCP_get_param( recvBuffer,  dataArray);
                                    //mode == 0 for video mode
									gk_isp_get_spatial_filter(handle, 0, &spatial_filter_tuning);

									//MV Enable did not open
                                    fir_tuning.fir1_s_enable                = dataArray[1];
                                    fir_tuning.fir1_w_enable                = dataArray[2];
                                    fir_tuning.fir3_enable                  = dataArray[3];
                                    fir_tuning.edge_disp_enable             = dataArray[4];
                                    fir_tuning.edge_dir_sel                 = dataArray[5];
                                    fir_tuning.edge_dir_shift               = dataArray[6];
                                    fir_tuning.edge_dir_info                = dataArray[7];
                                    max_change_tuning.max_change_sel        = dataArray[8];
                                    fir_tuning.fir_th1                      = dataArray[9];
                                    fir_tuning.fir_th2                      = dataArray[10];
                                    fir_tuning.fir_th3                      = dataArray[11];
                                    max_change_tuning.max_change_down_mv    = dataArray[12];
                                    max_change_tuning.max_change_up_mv      = dataArray[13];
                                    max_change_tuning.max_change_down_fir1s = dataArray[14];
                                    max_change_tuning.max_change_up_fir1s   = dataArray[15];
                                    max_change_tuning.max_change_down_fir1w = dataArray[16];
                                    max_change_tuning.max_change_up_fir1w   = dataArray[17];
                                    fir_tuning.fir1_w_str                   = dataArray[18];
                                    fir_tuning.fir1_s_str                   = dataArray[19];
                                    fir_tuning.fir3_str_iso                 = dataArray[20];
                                    fir_tuning.fir1_mv_str                  = dataArray[21];
									fir_tuning.fir2_mv_str					= dataArray[22];
                                    fir_tuning.fir_strength                 = dataArray[23];
                                    max_change_tuning.max_change_down       = dataArray[24];
									max_change_tuning.max_change_up         = dataArray[25];
									spatial_filter_tuning.directional_strength = dataArray[26];
									spatial_filter_tuning.isotropic_strength   = dataArray[27];
									spatial_filter_tuning.edge_threshold	   = dataArray[28];

									gk_isp_set_sharp_max_change(handle, 0, &max_change_tuning); //MAX
									gk_isp_set_sharpen_template(handle, 0, &fir_tuning,&spatial_filter_tuning);  // Fir
									gk_isp_set_spatial_filter(handle, 0, &spatial_filter_tuning);    // mode == 0 for video mode

                                }
                                break;

							case LCEdenoise:
								{
									_printf("LCEdenoise set\n");
									TCP_get_param( recvBuffer,  dataArray);

                                    local_contrast_enhancement_info.lce_enable           = dataArray[0];
                                    local_contrast_enhancement_info.pre_lut_en           = dataArray[1];
                                    local_contrast_enhancement_info.pm_en                = dataArray[2];
                                    local_contrast_enhancement_info.mv_scan_en           = dataArray[3];
                                    local_contrast_enhancement_info.disp_mv_en           = dataArray[4];
                                    local_contrast_enhancement_info.pre_hist_scan_en     = dataArray[5];
                                    local_contrast_enhancement_info.pre_hist_scan_frame  = dataArray[6];
                                    local_contrast_enhancement_info.pre_hist_scan_id     = dataArray[7];
                                    local_contrast_enhancement_info.post_hist_scan_en    = dataArray[8];
                                    local_contrast_enhancement_info.post_hist_scan_frame = dataArray[9];
                                    local_contrast_enhancement_info.post_hist_scan_id    = dataArray[10];
                                    local_contrast_enhancement_info.hist_scan_sft        = dataArray[11];
                                    local_contrast_enhancement_info.res_width            = dataArray[12];
                                    local_contrast_enhancement_info.res_height           = dataArray[13];
                                    local_contrast_enhancement_info.lce_range            = dataArray[14];
                                    local_contrast_enhancement_info.lce_guard            = dataArray[15];
                                    local_contrast_enhancement_info.lce_range_th1        = dataArray[16];
                                    local_contrast_enhancement_info.lce_range_th2        = dataArray[17];
                                    local_contrast_enhancement_info.lce_min_th           = dataArray[18];
                                    local_contrast_enhancement_info.lce_max_th           = dataArray[19];
                                    local_contrast_enhancement_info.lce_minb_range       = dataArray[20];
                                    local_contrast_enhancement_info.lce_maxb_range       = dataArray[21];
                                    local_contrast_enhancement_info.lce_contrast         = dataArray[22];
                                    local_contrast_enhancement_info.lce_delta_th         = dataArray[23];
                                    local_contrast_enhancement_info.lce_cutlimit         = dataArray[24];
                                    local_contrast_enhancement_info.lce_alpha            = dataArray[25];
                                    local_contrast_enhancement_info.lce_sud_th           = dataArray[26];
                                    local_contrast_enhancement_info.lce_sud_sum_th       = dataArray[27];
                                    local_contrast_enhancement_info.lce_delta_disable    = dataArray[28];
                                    local_contrast_enhancement_info.lce_index            = dataArray[29];
                                    local_contrast_enhancement_info.lce_search_th        = dataArray[30];

#if 0

                                    local_contrast_enhancement_info.pm_y_gain            = dataArray[31];
                                    local_contrast_enhancement_info.pm_y_shift           = dataArray[32];
                                    local_contrast_enhancement_info.pm_u_gain            = dataArray[33];
                                    local_contrast_enhancement_info.pm_u_shift           = dataArray[34];
                                    local_contrast_enhancement_info.pm_v_gain            = dataArray[35];
                                    local_contrast_enhancement_info.pm_v_shift           = dataArray[36];
                                    local_contrast_enhancement_info.pm_y_offset          = dataArray[37];
                                    local_contrast_enhancement_info.pm_u_offset          = dataArray[38];
                                    local_contrast_enhancement_info.pm_v_offset          = dataArray[39];
                                    local_contrast_enhancement_info.cnfg_mv_statis_th    = dataArray[40];

#endif

                                    gk_isp_set_local_contrast_enhancement(handle,
                                        &local_contrast_enhancement_info);
								}
                            break;
                            case HELPdenoise:
								{
									_printf("HELPdenoise set\n");
                                    TCP_get_param( recvBuffer,  dataArray);
                                    //LCE
                                    local_contrast_enhancement_info.disp_mv_en      = dataArray[0];

                                    //LSH 1 - 8
                                    fir_tuning.fir1_s_enable                        = dataArray[1];
                                    fir_tuning.fir1_w_enable                        = dataArray[2];
                                    fir_tuning.fir3_enable                          = dataArray[3];
                                    fir_tuning.edge_disp_enable                     = dataArray[4];
                                    fir_tuning.edge_dir_sel                         = dataArray[5];
                                    fir_tuning.edge_dir_shift                       = dataArray[6];
                                    fir_tuning.edge_dir_info                        = dataArray[7];
                                    max_change_tuning.max_change_sel                = dataArray[8];

                                    //LCE 9 -15
                                    local_contrast_enhancement_info.pre_hist_scan_en     = dataArray[9];
                                    local_contrast_enhancement_info.pre_hist_scan_frame  = dataArray[10];
                                    local_contrast_enhancement_info.pre_hist_scan_id     = dataArray[11];
                                    local_contrast_enhancement_info.post_hist_scan_en    = dataArray[12];
                                    local_contrast_enhancement_info.post_hist_scan_frame = dataArray[13];
                                    local_contrast_enhancement_info.post_hist_scan_id    = dataArray[14];
                                    local_contrast_enhancement_info.hist_scan_sft        = dataArray[15];

                                    //VGF NULL

                                    //LCE
                                    gk_isp_set_local_contrast_enhancement(handle,
                                        &local_contrast_enhancement_info);

                                    //LSH
                                    gk_isp_set_sharp_max_change(handle, 0, &max_change_tuning); //MAX
                                    //gk_isp_set_sharpen_template(handle, 0, &fir_tuning);  // Fir

								}
								break;
                        }
					}
					break;
                case LCEHist: // id_Require==Apply  ChengBo
                    {
                        _printf("case HIST Apply!\n");
                        recv(new_fd, recvBuffer, 2000, 0);
                        TCP_get_param(recvBuffer, dataArray);
                        _printf("histogram timer : %d\n\n",dataArray[0]);

                        switch (data.layer3) {
                        case histtimerconfig:
                            {
                                _printf("Apply histtimer");
                                histogram_timer = 1000 * dataArray[0];
                            }
                            break;
                        case histogramid:
                            {
                                _printf("hist data 0-15 Can not Apply");
                            }
                            break;
                        }
                    }
                    break;

                case BINDATA: // id_Require==Apply  ChengBo
                    {
                        _printf("Bindata set\n");
                        recv(new_fd, recvBuffer, 2000, 0);
                        switch (data.layer3)
                        {
                        case saveConfig:
                            TCP_get_param(recvBuffer, dataArray);
                            _printf("saveConfig \n");
                            _printf("\n");

#if                                 0
                            printf("dataArray[0] %d\n",dataArray[0]); // no_dlg
                            printf("dataArray[1] %d\n",dataArray[1]); // RGB 2 YUV
#endif
                            int binOption   = dataArray[0];
                            int table_index = gk_get_sensor_gain_index(handle);
                            //day             = 1;                      // day or night
                            _printf("\n");
                            _printf("Bin file day %d\n\n",day);
                            RGBindex        = dataArray[1];
                            table_no        = (table_index+64)>>7;

                            //float ExdB = ( (table_index * 3 *100)>>6 )/100;
                            float ExdB = table_index*3.0/64.0;
                            int dBAIndex;
                            int dBBIndex;
                            int antiEnable = 0;
                            //int signal1,signal2;
                            int binLen = 0;

                            dBA = ((int)ExdB/6)*6;
                            dBB = dBA + 6;
                            dBAIndex = (dBA<<6)/3;
                            dBBIndex = (dBB<<6)/3;

#if 0

                            int currentdB = table_no*6;
                            _printf("\n");
                            _printf("dBA  %d\n", dBA);
                            _printf("dBB  %d\n", dBB);
                            _printf("dBAIndex  %d\n", dBAIndex);
                            _printf("dBBIndex  %d\n", dBBIndex);
                            _printf("ExdB %f\n", ExdB);
                            _printf("currentdB %d\n", table_no*6);
                            _printf("\n");
                            _printf("Now Option is %d\n",binOption);

#endif
                            if(abs(table_index - dBAIndex) < 20 || abs(table_index - dBBIndex) < 20)
                            {
                                antiEnable = 0;
                            }
                            else
                            {
                                antiEnable = 1;
                            }



                            switch(binOption)
                            {
                            case 1: // blc
                                {
                                    binLen = 4;
                                    if(antiEnable == 0)
                                    {
                                        gk_isp_get_final_blc(handle, &bin_blc);
                                        tuning_save_blc_parameters(bin_adj_param,bin_blc,day,table_index);
                                    }
                                    else
                                    {
                                        adj_param_t bin_adj_param_init;
                                        tuning_get_bin_adj_param_init(handle, &bin_adj_param_init);

                                        //1. Get Z value
                                        gk_isp_get_final_blc(handle, &bin_blc);

                                        //2. Calculate bin value
                                        bin_assign_blc(bin_adj_param_init, &bin_interpolation, day,
                                            table_index, binLen, &bin_blc, &bin_blcM, &bin_blcN);

                                        //3. Save new bin value
                                        tuning_save_blc_parameters(bin_adj_param,bin_blcM,day,dBAIndex);
                                        tuning_save_blc_parameters(bin_adj_param,bin_blcN,day,dBBIndex);

                                    }

                                    //printf("black_level %d\n",image_sensor_param.p_adj_param->def.black_level[0].value[0]);
                                    //image_sensor_param.p_adj_param->def.black_level[0].value[0] = 0;
                                }
                                break;
                            case 3:
                                {
                                    _printf("RGBindex %d\n",RGBindex);
                                    _printf("\n RGB YUV \n");
                                }
                                break;

                            case 6:
                                {
                                    _printf("\n ChromaScale \n");
                                }
                                break;

                            case 10:
                                {
                                    binLen = 6;
                                    if(antiEnable == 0)
                                    {
                                        tuning_get_sharpen_str(handle,
                                            &bin_tuning_sharp_str);
                                        tuning_save_sharpen_parameters(bin_adj_param,bin_tuning_sharp_str,day,table_index);
                                    }
                                    else
                                    {
                                        adj_param_t bin_adj_param_init;
                                        tuning_get_bin_adj_param_init(handle, &bin_adj_param_init);
                                        //1. Get Z value
                                        tuning_get_sharpen_str(handle,
                                            &bin_tuning_sharp_str);

                                        //2. Calculate bin value
                                        tuning_bin_assign_sharpen(bin_adj_param_init,
                                            &bin_interpolation, day, table_index, binLen,
                                            &bin_tuning_sharp_str,
                                            &bin_tuning_sharp_strM,
                                            &bin_tuning_sharp_strN);

                                        //3. Save new bin value
                                        tuning_save_sharpen_parameters(bin_adj_param,bin_tuning_sharp_strM,day,dBAIndex);
                                        tuning_save_sharpen_parameters(bin_adj_param,bin_tuning_sharp_strN,day,dBBIndex);

                                    }

                                }
                                break;

                            case 12:
                                {
                                    binLen = 3;
                                    // BadPixelCorrection
                                    if(antiEnable == 0)
                                    {
                                        gk_isp_get_auto_bad_pixel_correction
                                            (handle, &bin_dbp_correction);
                                        tuning_save_dynamic_bad_pixel_parameters(bin_adj_param,bin_dbp_correction,day,table_index);
                                    }
                                    else
                                    {
                                        adj_param_t bin_adj_param_init;
                                        tuning_get_bin_adj_param_init(handle, &bin_adj_param_init);
                                        //1. Get Z value
                                        gk_isp_get_auto_bad_pixel_correction
                                            (handle, &bin_dbp_correction);

                                        //2. Calculate bin value
                                        tuning_bin_assign_dynamic_bad_pixel(bin_adj_param_init,
                                            &bin_interpolation, day, table_index, binLen,
                                            &bin_dbp_correction,
                                            &bin_dbp_correctionM,
                                            &bin_dbp_correctionN);

                                        //3. Save new bin value
                                        tuning_save_dynamic_bad_pixel_parameters(bin_adj_param,bin_dbp_correctionM,day,dBAIndex);
                                        tuning_save_dynamic_bad_pixel_parameters(bin_adj_param,bin_dbp_correctionN,day,dBBIndex);
                                    }

                                    // AntiAliasingFilter
                                    BinAntiAliasing = gk_isp_get_anti_aliasing_filter(handle);
                                    bin_adj_param->ev_img.table[dBA/6].value[6] = BinAntiAliasing;
                                    bin_adj_param->ev_img.table[dBB/6].value[6] = BinAntiAliasing;

                                }
                                break;

                            case 14:
                                {
                                    gk_isp_get_color_denoise_filter
                                        (handle, &bin_color_filter_config);
                                    u8 color_str = bin_color_filter_config.cr_str;

                                    if(antiEnable == 0)
                                    {
                                        tuning_save_color_denoise_parameters(bin_adj_param,color_str,table_index);
                                    }
                                    else
                                    {
                                        adj_param_t bin_adj_param_init;
                                        tuning_get_bin_adj_param_init(handle, &bin_adj_param_init);

                                        float binA = (float)bin_adj_param_init.def.noise_table[dBA/6].value[3];
                                        float binB = (float)bin_adj_param_init.def.noise_table[dBB/6].value[3];
                                        //float binY = 0;
                                        float binZ = color_str;
                                        float binReA = 0;
                                        float binReB = 0;


                                        binCaculate(binA, binB, (float)dBA, (float)dBB, ExdB, binZ,&binReA,&binReB);
                                        //save
                                        color_str = binReA;
                                        tuning_save_color_denoise_parameters(bin_adj_param,color_str,dBAIndex);

                                        color_str = binReB;
                                        tuning_save_color_denoise_parameters(bin_adj_param,color_str,dBBIndex);
                                    }

                                    //printf("color_denoise end\n");
                                }
                                break;

                            case 15:
                                break;
                            case 16:
                                {

                                    binLen = 9;
                                    if(antiEnable == 0)
                                    {
                                        tuning_get_CFA_denoise_str(handle,
                                            &bin_tuning_CFA_denoise);
                                        tuning_save_CFA_denoise_parameters
                                            (bin_adj_param,bin_tuning_CFA_denoise,day,table_index);
                                    }
                                    else
                                    {
                                        adj_param_t bin_adj_param_init;
                                        tuning_get_bin_adj_param_init(handle, &bin_adj_param_init);
                                        //1. Get Z value
                                        tuning_get_CFA_denoise_str(handle,
                                            &bin_tuning_CFA_denoise);

                                        //2. Calculate bin value
                                        tuning_bin_assign_CFA_denoise(bin_adj_param_init,
                                            &bin_interpolation, day, table_index, binLen,
                                            &bin_tuning_CFA_denoise,
                                            &bin_tuning_CFA_denoiseM,
                                            &bin_tuning_CFA_denoiseN);

                                        //3. Save new bin value
                                        tuning_save_CFA_denoise_parameters
                                            (bin_adj_param,bin_tuning_CFA_denoiseM,day,dBAIndex);
                                        tuning_save_CFA_denoise_parameters
                                            (bin_adj_param,bin_tuning_CFA_denoiseN,day,dBBIndex);


                                    }

                                }
                                break;

                            case 25:
                                {
                                    u16 binAECTarget = gk_ae_get_target(handle);
                                    float AECTargetA,AECTargetB;

                                    if(antiEnable == 0)
                                    {
                                        tuning_save_AE_parameters
                                            (bin_adj_param, binAECTarget, table_index);
                                    }
                                    else
                                    {
                                        adj_param_t bin_adj_param_init;
                                        tuning_get_bin_adj_param_init(handle, &bin_adj_param_init);
                                        binCaculate((float)bin_adj_param_init.awbae.table[dBA/6].value[6],
                                            (float)bin_adj_param_init.awbae.table[dBB/6].value[6],
                                            (float)dBA, (float)dBB, ExdB,(float)binAECTarget,
                                            &AECTargetA,&AECTargetB);

                                        tuning_save_AE_parameters
                                            (bin_adj_param, (u8)AECTargetA, dBAIndex);
                                        tuning_save_AE_parameters
                                            (bin_adj_param, (u8)AECTargetB, dBBIndex);
                                    }

                                    //printf("tuning_save_AE_parameters \n");
                                }
                                break;

                            case 41:
                                {
                                    binLen = 24;
                                    if(antiEnable == 0)
                                    {

                                    }
                                    else
                                    {

                                    }

                                }
                                break;

                            case 42:
                                {
                                    binLen = 87;

                                    if(antiEnable == 0)
                                    {

                                    }
                                    else
                                    {
                                    }

                                }
                                break;

                            case 43:
                                {
                                    binLen = 4;
                                    if(antiEnable == 0)
                                    {
                                    }
                                    else
                                    {
                                    }
                                    //printf("vps_contrast Alpha %d\n",bin_tuning_vps_contrast.vps_contrast_alpha);
                                }
                                break;

                            case 44:
                                {

                                }
                                break;

                            case 46:
                                {
                                    _printf("Bin file Save Encode Par\n");
                                    binLen = 16;
#if 1
                                    if(antiEnable == 0)
                                    {
                                    }
                                    else
                                    {
                                    }
#endif
                                }
                                break;

                            default:
                                _printf("\n option %d\n",binOption);
                                break;
                            }
                            break;

                        case newBinfile:

                            break;
                        }
                    }
                }
                break;
            case Caribration:
                //_printf("break layer1\n");
                break;
            case AutoTest:
                //_printf("break layer1\n");
                break;

            }

        }
        else if (id_Require == Load  && KEYMACTH == 1)  //load
        {
            _printf("Load!\n");
            _printf("data.layer1 %d\n",data.layer1);
            _printf("data.layer2 %d\n",data.layer2);
            _printf("data.layer3 %d\n",data.layer3);

            switch (data.layer1) {
            case OnlineTuning:
            {
                //_printf("Online Tuning!\n");
                switch (data.layer2) {
                case Color:
                {
                    _printf("case Color Load!\n");
                    recv(new_fd, recvBuffer, 2000, 0);
                    TCP_get_param(recvBuffer, dataArray);
                    _printf("Load CHIPID : %d\n\n",dataArray[0]);

                    if(dataArray[0] != CHIPPIN)
                    {
                        _printf("Load cmd %d%d%d\n\n",data.layer1,data.layer2,data.layer3);
                        //KEYMACTH = 0;
                        break;
                    }

                    switch (data.layer3) {
                    case BlackLevelCorrection:

                        gk_isp_get_final_blc(handle, &blc);
                        sendBuffer[0] = blc.r_offset;
                        sendBuffer[1] = blc.gr_offset;
                        sendBuffer[2] = blc.gb_offset;
                        sendBuffer[3] = blc.b_offset;

                        sendBuffer[4] = blc_avg_r;
                        sendBuffer[5] = blc_avg_g;
                        sendBuffer[6] = blc_avg_b;
                        char sendBlc[10];

                        for (i = 0; i < 7; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendBlc);
                            send(new_fd, sendBlc, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        //send(new_fd,&blc,sizeof(blc_level_t),0);
                        send(new_fd, "!", 1, 0);
                        _printf("Black Level Correction Load\n");

#ifdef ISP_CONFIG
                        _printf("\n");                                                                                                  _printf("r_offset  %d\n",blc.r_offset);
                        _printf("gr_offset %d\n",blc.gr_offset);
                        _printf("gb_offset %d\n",blc.gb_offset);
                        _printf("b_offset  %d\n",blc.b_offset);
                        _printf("blc_avg_r %d\n",blc_avg_r);
                        _printf("blc_avg_g %d\n",blc_avg_g);
                        _printf("blc_avg_b %d\n",blc_avg_b);
                        _printf("\n");
#endif

                        break;

                    case ColorCorrection:
                        break;

                    case ToneCurve:
                        gk_isp_get_tone_curve(handle, &tone_curve);
                        for (i = 0; i < TONE_CURVE_SIZE; i++)
                            sendBuffer[i] =
                            tone_curve.tone_curve_blue[i];
                        char sendToneCurve[10];

                        for (i = 0; i < TONE_CURVE_SIZE; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendToneCurve);
                            send(new_fd, sendToneCurve, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);
                        _printf("Tone Dynamic Curve\n ");
                        break;

                    case RGBtoYUVMatrix:
                        gk_isp_get_rgb2yuv_matrix(handle, &rgb2yuv_matrix);
                        sendBuffer[0] =
                            rgb2yuv_matrix.matrix_values[0];
                        sendBuffer[1] =
                            rgb2yuv_matrix.matrix_values[1];
                        sendBuffer[2] =
                            rgb2yuv_matrix.matrix_values[2];
                        sendBuffer[3] =
                            rgb2yuv_matrix.matrix_values[3];
                        sendBuffer[4] =
                            rgb2yuv_matrix.matrix_values[4];
                        sendBuffer[5] =
                            rgb2yuv_matrix.matrix_values[5];
                        sendBuffer[6] =
                            rgb2yuv_matrix.matrix_values[6];
                        sendBuffer[7] =
                            rgb2yuv_matrix.matrix_values[7];
                        sendBuffer[8] =
                            rgb2yuv_matrix.matrix_values[8];
                        sendBuffer[9] = rgb2yuv_matrix.y_offset;
                        sendBuffer[10] = rgb2yuv_matrix.u_offset;
                        sendBuffer[11] = rgb2yuv_matrix.v_offset;

                        char sendRGBtoYUVMatrix[10];

                        for (i = 0; i < 12; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendRGBtoYUVMatrix);
                            send(new_fd, sendRGBtoYUVMatrix, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);
                        _printf("Color Matrix\n");
                        break;

                    case WhiteBalanceGains:
                        gk_isp_get_rgb_gain(handle, &wb_gain, &dgain);
                        sendBuffer[0] = wb_gain.r_gain;
                        sendBuffer[1] = wb_gain.g_gain;
                        sendBuffer[2] = wb_gain.b_gain;

                        printf("dgain %d\n",dgain);

                        char sendWB[10];

                        for (i = 0; i < 3; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendWB);
                            send(new_fd, sendWB, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);
                        _printf("White Balance Gains\n");
                        break;

                    case DGainSaturaionLevel:
                        gk_isp_get_dgain_thr
                            (handle, &isp_gain_satuation_level);
                        sendBuffer[0] =
                            isp_gain_satuation_level.level_red;
                        sendBuffer[1] =
                            isp_gain_satuation_level.level_green_even;
                        sendBuffer[2] =
                            isp_gain_satuation_level.level_green_odd;
                        sendBuffer[3] =
                            isp_gain_satuation_level.level_blue;

                        char sendDGainSaturaionLevel[10];

                        for (i = 0; i < 4; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendDGainSaturaionLevel);
                            send(new_fd, sendDGainSaturaionLevel, k,
                                0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);
                        _printf("D_Gainn");
                        break;

                    case LocalExposure:
                        gk_isp_get_wdr(handle, &local_wdr);
                        sendBuffer[0] = local_wdr.enable;
                        sendBuffer[1] = local_wdr.radius;
                        sendBuffer[2] =
                            local_wdr.luma_weight_shift;
                        sendBuffer[3] =
                            local_wdr.luma_weight_red;
                        sendBuffer[4] =
                            local_wdr.luma_weight_green;
                        sendBuffer[5] =
                            local_wdr.luma_weight_blue;

                        for (i = 0; i < NUM_EXPOSURE_CURVE; i++)
                            sendBuffer[6 + i] =
                            local_wdr.gain_curve_table[i];

                        //send(new_fd,sendCharArray,6,0);
                        //_printf("%d,%d,%d,%d,%d,%d\n",sendCharArray[0],sendCharArray[1],sendCharArray[2],sendCharArray[3],sendCharArray[4],sendCharArray[5]);

                        char sendLocalExposure[10];

                        for (i = 0; i < NUM_EXPOSURE_CURVE + 6; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendLocalExposure);

                            send(new_fd, sendLocalExposure, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        _printf("Exposure controlling\n");
                        break;

                    case ChromaScale:
                        gk_isp_get_uv_scale(handle, &cs);
                        sendBuffer[0] = cs.enable;

                        for (i = 0; i < NUM_CHROMA_GAIN_CURVE; i++)
                            sendBuffer[i + 1] = cs.gain_curve[i];

                        char sendChromaScale[10];
                        int num_cs = NUM_CHROMA_GAIN_CURVE + 1;

                        for (i = 0; i < num_cs; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendChromaScale);
                            send(new_fd, sendChromaScale, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        _printf("Chroma Rate\n");
                        break;

                    case ColorAdjust:
                        {
                            image_property_t image_property;
                            float dync;

                            apc_get_img_property(handle, &image_property);

                            sendBuffer[0] = image_property.brightness;
                            sendBuffer[1] = image_property.saturation;
                            sendBuffer[2] = image_property.hue;

                            sendBuffer[3] = sharpenStatus; //sharpen can't get status
                            sendBuffer[4] = image_property.sharpness;

                            sendBuffer[5] = d_tone_curve_get_status(handle);

                            sendBuffer[6] = image_property.contrast;

                            dync  =  (0x0080 * d_tone_curve_get_param(handle)); // wdr range
                            sendBuffer[7] =(int) dync;

                            sendBuffer[8] = gadi_isp_get_day_night_mode(handle);

                            irDayNight    = sendBuffer[8];
                            tuning_image_ircut_switch(irDayNight);

                            _printf("irDayNight %d\n",sendBuffer[8]);

                            char sendColorAdj[20];
                            for(i=0;i<9;i++)
                            {
                                int k;
                                k=Int32ToCharArray(sendBuffer[i],sendColorAdj);
                                send(new_fd,sendColorAdj,k,0);
                                send(new_fd," ",1,0);
                            }
                            send(new_fd,"!",1,0);

#ifdef ISP_CONFIG
                            _printf("\n");
                            _printf("brightness     %d\n",image_property.brightness);
                            _printf("saturation     %d\n",image_property.saturation);
                            _printf("hue            %d\n",image_property.hue);
                            _printf("sharpenStatus  %d\n",image_property.sharpness);
                            _printf("sharpenLevel   %d\n",sharpenLevel);
                            _printf("image sharpness    %d\n",image_property.sharpness);
                            _printf("wdrEnable      %d\n",wdrEnable);
                            _printf("contrast       %d\n",image_property.contrast);
                            _printf("wdrRange       %d\n",wdrRange);
                            _printf("\n");
#endif


                            _printf("Color Adjust Load\n");
                        }
                        break;
                    case Depurple: //Load depurple
						{
                            gk_isp_get_depurple_fringe(handle,&depurple_fringe);
                            sendBuffer[0]   = depurple_fringe.depurple_fringe_enable;
                            sendBuffer[1]   = depurple_fringe.depurple_fringe_strength;
                            sendBuffer[2]   = depurple_fringe.depurple_fringe_th_max_a;
                            sendBuffer[3]   = depurple_fringe.depurple_fringe_th_max_b;
                            sendBuffer[4]   = depurple_fringe.depurple_fringe_th_max_c;
                            sendBuffer[5]   = depurple_fringe.depurple_fringe_th_max_d;
                            sendBuffer[6]   = depurple_fringe.depurple_fringe_th_max_e;
                            sendBuffer[7]   = depurple_fringe.depurple_fringe_th_max_f;
                            sendBuffer[8]   = depurple_fringe.depurple_fringe_th_max;
                            sendBuffer[9]   = depurple_fringe.depurple_fringe_green_threshold;
                            sendBuffer[10]  = depurple_fringe.depurple_fringe_rb_threshold;

                            sendBuffer[11]  = depurple_fringe.dpf_minA_threshold;
                            sendBuffer[12]  = depurple_fringe.dpf_minB_threshold;
                            sendBuffer[13]  = depurple_fringe.dpf_minC_threshold;
                            sendBuffer[14]  = depurple_fringe.dpf_grad_threshold;
                            sendBuffer[15]  = depurple_fringe.dpf_flag_threshold;
							sendBuffer[16]  = depurple_fringe.dpf_enhance_enable;
                            sendBuffer[17]  = depurple_fringe.nlm_dpf_enable;
                            sendBuffer[18]  = depurple_fringe.dpf_max_change;
                            sendBuffer[19]  = depurple_fringe.dpf_alpha;

#if 1
                            _printf("depurple_fringe.depurple_fringe_enable          :%d\n",depurple_fringe.depurple_fringe_enable);
                            _printf("depurple_fringe.depurple_fringe_strength        :%d\n",depurple_fringe.depurple_fringe_strength);
                            _printf("depurple_fringe.depurple_fringe_th_max_a        :%d\n",depurple_fringe.depurple_fringe_th_max_a);
                            _printf("depurple_fringe.depurple_fringe_th_max_b        :%d\n",depurple_fringe.depurple_fringe_th_max_b);
                            _printf("depurple_fringe.depurple_fringe_th_max_c        :%d\n",depurple_fringe.depurple_fringe_th_max_c);
                            _printf("depurple_fringe.depurple_fringe_th_max_d        :%d\n",depurple_fringe.depurple_fringe_th_max_d);
                            _printf("depurple_fringe.depurple_fringe_th_max_e        :%d\n",depurple_fringe.depurple_fringe_th_max_e);
                            _printf("depurple_fringe.depurple_fringe_th_max_f        :%d\n",depurple_fringe.depurple_fringe_th_max_f);
                            _printf("depurple_fringe.depurple_fringe_th_max          :%d\n",depurple_fringe.depurple_fringe_th_max);
                            _printf("depurple_fringe.depurple_fringe_green_threshold :%d\n",depurple_fringe.depurple_fringe_green_threshold);
                            _printf("depurple_fringe.depurple_fringe_rb_threshold    :%d\n",depurple_fringe.depurple_fringe_rb_threshold);
                            _printf("depurple_fringe.dpf_minA_threshold              :%d\n",depurple_fringe.dpf_minA_threshold);
                            _printf("depurple_fringe.dpf_minB_threshold              :%d\n",depurple_fringe.dpf_minB_threshold);
                            _printf("depurple_fringe.dpf_minC_threshold              :%d\n",depurple_fringe.dpf_minC_threshold);
                            _printf("depurple_fringe.dpf_grad_threshold              :%d\n",depurple_fringe.dpf_grad_threshold);
                            _printf("depurple_fringe.dpf_flag_threshold              :%d\n",depurple_fringe.dpf_flag_threshold);
                            _printf("depurple_fringe.dpf_enhance_enable              :%d\n",depurple_fringe.dpf_enhance_enable);
                            _printf("depurple_fringe.nlm_dpf_enable                  :%d\n",depurple_fringe.nlm_dpf_enable);
                            _printf("depurple_fringe.dpf_max_change                  :%d\n",depurple_fringe.dpf_max_change);
                            _printf("depurple_fringe.dpf_alpha                       :%d\n",depurple_fringe.dpf_alpha);

#endif
                            char sendDepurple[100];

                            for(i=0;i<20;i++)
                            {
                                int k;
                                k=Int32ToCharArray(sendBuffer[i],sendDepurple);
                                send(new_fd,sendDepurple,k,0);
                                send(new_fd," ",1,0);
                            }
                            send(new_fd,"!",1,0);

						}
						break;
                    }
                }
                break;

                case Noise:
                {
                    _printf("case Noise Load!\n");
                    recv(new_fd, recvBuffer, 2000, 0);
                    TCP_get_param(recvBuffer, dataArray);
                    _printf("Load CHIPID : %d\n\n",dataArray[0]);

                    if(dataArray[0] != CHIPPIN)
                    {
                        _printf("Load cmd %d%d%d\n\n",data.layer1,data.layer2,data.layer3);
                        //KEYMACTH = 0;
                        break;
                    }


                    switch (data.layer3) {
                    case FPNCorrection:
                        break;

                    case BadPixelCorrection:
                        gk_isp_get_auto_bad_pixel_correction
                            (handle, &dbp_correction_config);
                        sendBuffer[0] =
                            dbp_correction_config.enable;

                        sendBuffer[1] =
                            dbp_correction_config.
                            hot_pixel_strength;
                        sendBuffer[2] =
                            dbp_correction_config.
                            dark_pixel_strength;
                        char sendBPC[10];

                        for (i = 0; i < 3; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendBPC);
                            send(new_fd, sendBPC, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        _printf("Bad Pixels Correction\n");
                        break;

                    case CFALeakageFilter:
                        gk_isp_get_cfa_lea_filter
                            (handle, &cfa_lk_filter);
                        sendBuffer[0] = cfa_lk_filter.enable;

                        sendBuffer[1] = cfa_lk_filter.alpha_rr;
                        sendBuffer[2] = cfa_lk_filter.alpha_rb;
                        sendBuffer[3] = cfa_lk_filter.alpha_br;
                        sendBuffer[4] = cfa_lk_filter.alpha_bb;
                        sendBuffer[5] =
                            cfa_lk_filter.saturation_level;
                        char sendLKG[10];

                        for (i = 0; i < 6; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendLKG);
                            send(new_fd, sendLKG, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        _printf("CFA Leakage\n");
                        break;

                    case AntiAliasingFilter:
                        sendBuffer[0] = gk_isp_get_anti_aliasing_filter(handle);
                        _printf("%d\n", sendBuffer[0]);
                        send(new_fd, sendBuffer, 1, 0);
                        send(new_fd, "!", 1, 0);

                        _printf("Antialiasing\n");
                        break;

                    case CFANoiseFilter:


						gk_isp_get_cfa_denoise_filter(handle, &cfa_denoise_filter);

						sendBuffer[0] = cfa_denoise_filter.direct_center_weight_red;
						sendBuffer[1] = cfa_denoise_filter.direct_center_weight_green;
						sendBuffer[2] = cfa_denoise_filter.direct_center_weight_blue;
						sendBuffer[3] = cfa_denoise_filter.direct_grad_thresh;
						sendBuffer[4] = cfa_denoise_filter.direct_thresh_k0_red;
						sendBuffer[5] = cfa_denoise_filter.direct_thresh_k0_green;
						sendBuffer[6] = cfa_denoise_filter.direct_thresh_k0_blue;

						sendBuffer[7]  = cfa_denoise_filter.iso_center_weight_red;
						sendBuffer[8]  = cfa_denoise_filter.iso_center_weight_green;
						sendBuffer[9]  = cfa_denoise_filter.iso_center_weight_blue;
						sendBuffer[10] = cfa_denoise_filter.iso_thresh_k0_red;
						sendBuffer[11] = cfa_denoise_filter.iso_thresh_k0_green;
						sendBuffer[12] = cfa_denoise_filter.iso_thresh_k0_blue;
						sendBuffer[13] = cfa_denoise_filter.iso_thresh_k0_close;


                        char sendCFA[100];

                        for (i = 0; i < 14; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendCFA);
                            send(new_fd, sendCFA, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);
                        _printf("CFA denoise\n");
                        break;

                    case ChromaMedianFiler:
                        {
                            gk_isp_get_color_denoise_filter
                                (handle, &color_filter_config);
                            gk_isp_get_chroma_median_filter_ex(handle, &chroma_median_filter_ex);
                            sendBuffer[0] = color_filter_config.enable;
                            sendBuffer[1] = color_filter_config.cb_str;
                            sendBuffer[2] = color_filter_config.cr_str;

                            sendBuffer[3]  = chroma_median_filter_ex.ex_blur_en;
    						sendBuffer[4]  = chroma_median_filter_ex.blur_nolimit_en;
    						sendBuffer[5]  = chroma_median_filter_ex.ex_blur_delta_1;
    						sendBuffer[6]  = chroma_median_filter_ex.ex_blur_delta_2;
    						sendBuffer[7]  = chroma_median_filter_ex.ex_blur_delta_3;
    						sendBuffer[8]  = chroma_median_filter_ex.ex_blur_mul_1;
    						sendBuffer[9]  = chroma_median_filter_ex.ex_blur_sft_1;
    						sendBuffer[10] = chroma_median_filter_ex.ex_blur_mul_2;
    						sendBuffer[11] = chroma_median_filter_ex.ex_blur_sft_2;
    						sendBuffer[12] = chroma_median_filter_ex.ex_blur_mul_3;
    						sendBuffer[13] = chroma_median_filter_ex.ex_blur_sft_3;
    						sendBuffer[14] = chroma_median_filter_ex.ex_blur_data_u;
    						sendBuffer[15] = chroma_median_filter_ex.ex_blur_data_v;

#if 1

                            _printf("color_filter_config.enable              %d\n",color_filter_config.enable);
                            _printf("color_filter_config.cb_str              %d\n",color_filter_config.cb_str);
                            _printf("color_filter_config.cr_str              %d\n",color_filter_config.cr_str);
                            _printf("chroma_median_filter_ex.ex_blur_en      %d\n",chroma_median_filter_ex.ex_blur_en);
                            _printf("chroma_median_filter_ex.blur_nolimit_en %d\n",chroma_median_filter_ex.blur_nolimit_en);
                            _printf("chroma_median_filter_ex.ex_blur_delta_1 %d\n",chroma_median_filter_ex.ex_blur_delta_1);
                            _printf("chroma_median_filter_ex.ex_blur_delta_2 %d\n",chroma_median_filter_ex.ex_blur_delta_2);
                            _printf("chroma_median_filter_ex.ex_blur_delta_3 %d\n",chroma_median_filter_ex.ex_blur_delta_3);
                            _printf("chroma_median_filter_ex.ex_blur_mul_1   %d\n",chroma_median_filter_ex.ex_blur_mul_1);
                            _printf("chroma_median_filter_ex.ex_blur_sft_1   %d\n",chroma_median_filter_ex.ex_blur_sft_1);
                            _printf("chroma_median_filter_ex.ex_blur_mul_2   %d\n",chroma_median_filter_ex.ex_blur_mul_2);
                            _printf("chroma_median_filter_ex.ex_blur_sft_2   %d\n",chroma_median_filter_ex.ex_blur_sft_2);
                            _printf("chroma_median_filter_ex.ex_blur_mul_3   %d\n",chroma_median_filter_ex.ex_blur_mul_3);
                            _printf("chroma_median_filter_ex.ex_blur_sft_3   %d\n",chroma_median_filter_ex.ex_blur_sft_3);
                            _printf("chroma_median_filter_ex.ex_blur_data_u  %d\n",chroma_median_filter_ex.ex_blur_data_u);
                            _printf("chroma_median_filter_ex.ex_blur_data_v  %d\n",chroma_median_filter_ex.ex_blur_data_v);

#endif

                            char sendCMF[100];

                            for (i = 0; i < 16; i++) {
                                int k;

                                k = Int32ToCharArray(sendBuffer[i],
                                    sendCMF);
                                send(new_fd, sendCMF, k, 0);
                                send(new_fd, " ", 1, 0);
                            }
                            send(new_fd, "!", 1, 0);

                            _printf("Chroma Median Filter\n");
                        }
                        break;

                    case SharpeningControl:

                        tuning_get_sharpen_str(handle,
                            &tuning_sharp_str);
                        printf("tuning_sharp_str %d \n",
                            tuning_sharp_str.fir_strength);
                        sendBuffer[0] =
                            tuning_sharp_str.fir_strength;
                        sendBuffer[1] =
                            tuning_sharp_str.isotropic_strength;
                        sendBuffer[2] =
                            tuning_sharp_str.directional_strength;
                        sendBuffer[3] =
                            tuning_sharp_str.edge_threshold;
                        sendBuffer[4] =
                            tuning_sharp_str.max_change_up;
                        sendBuffer[5] =
                            tuning_sharp_str.max_change_down;

                        char sendSC[10];

                        for (i = 0; i < 6; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendSC);
                            send(new_fd, sendSC, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

#ifdef ISP_CONFIG
                        _printf("\n");
                        _printf("fir_strength :%d\n",tuning_sharp_str.fir_strength);
                        _printf("\n");

#endif


                        _printf("Sharpening Load\n");
                        break;

                    case MCTFControl:
                        {
                        char sendMCTF[100];

                        for (i = 0; i < 12; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendMCTF);
                            send(new_fd, sendMCTF, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        _printf("3D denoise\n");
                        }

                        break;
                    }
                }
                break;

                case AAA:
                {
                    _printf("case AAA Load!\n");
                    recv(new_fd, recvBuffer, 2000, 0);
                    TCP_get_param(recvBuffer, dataArray);
                    _printf("Load CHIPID : %d\n\n",dataArray[0]);

                    if(dataArray[0] != CHIPPIN)
                    {
                        _printf("Load cmd %d%d%d\n\n",data.layer1,data.layer2,data.layer3);
                        //KEYMACTH = 0;
                        break;
                    }

                    switch (data.layer3) {
                    case ConfigAAAControl:
                        gk_get_3A_ctl_state(handle, &aaa_cntl_station);
                        sendCharArray[0] =
                            aaa_cntl_station.ae_enable;
                        sendCharArray[1] =
                            aaa_cntl_station.awb_enable;
                        sendCharArray[2] =
                            aaa_cntl_station.af_enable;
                        sendCharArray[3] =
                            aaa_cntl_station.adj_enable;
                        //_printf("%d,%d,%d,%d\n",sendCharArray[0],sendCharArray[1],sendCharArray[2],sendCharArray[3]);

                        send(new_fd, sendCharArray, 4, 0);
                        send(new_fd, "!", 1, 0);

                        _printf("AAA setting\n");
                        break;

                    case AETileConfiguration:
                        gk_isp_get_config_statistics_params
                            (handle, &gk_3a_statistic_config);
                        gk_isp_get_statistics_raw(handle,
                            &gk_rgb_stat, &gk_cfa_stat);

                        sendBuffer[0] = gk_cfa_stat.tile_info.ae_tile_num_col;       //gk_3a_tile_get_config1.ae_tile_num_col;
                        sendBuffer[1] =
                            gk_cfa_stat.tile_info.ae_tile_num_row;
                        sendBuffer[2] =
                            gk_cfa_stat.tile_info.ae_tile_col_start;
                        sendBuffer[3] =
                            gk_cfa_stat.tile_info.ae_tile_row_start;
                        sendBuffer[4] =
                            gk_cfa_stat.tile_info.ae_tile_width;
                        sendBuffer[5] =
                            gk_cfa_stat.tile_info.ae_tile_height;
                        sendBuffer[6] =
                            gk_cfa_stat.tile_info.ae_linear_y_shift;
                        sendBuffer[7] =
                            gk_cfa_stat.tile_info.ae_y_shift;

                        sendBuffer[8] =
                            gk_3a_statistic_config.ae_tile_num_col;
                        sendBuffer[9] =
                            gk_3a_statistic_config.ae_tile_num_row;
                        sendBuffer[10] =
                            gk_3a_statistic_config.ae_tile_col_start;
                        sendBuffer[11] =
                            gk_3a_statistic_config.ae_tile_row_start;
                        sendBuffer[12] =
                            gk_3a_statistic_config.ae_tile_width;
                        sendBuffer[13] =
                            gk_3a_statistic_config.ae_tile_height;
                        sendBuffer[14] =
                            gk_3a_statistic_config.ae_pix_min_value;
                        sendBuffer[15] =
                            gk_3a_statistic_config.ae_pix_max_value;

                        char sendAETileConfiguration[10];

                        for (i = 0; i < 16; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendAETileConfiguration);
                            send(new_fd, sendAETileConfiguration, k,
                                0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);
                        _printf("AE Chunk Setting\n");
                        break;

                    case AWBTilesConfiguration:
                        gk_isp_get_statistics_raw(handle,
                            &gk_rgb_stat, &gk_cfa_stat);
                        gk_isp_get_config_statistics_params
                            (handle, &gk_3a_statistic_config);

                        sendBuffer[0] =
                            gk_cfa_stat.tile_info.awb_tile_num_col;
                        sendBuffer[1] =
                            gk_cfa_stat.tile_info.awb_tile_num_row;
                        sendBuffer[2] =
                            gk_cfa_stat.tile_info.awb_tile_col_start;
                        sendBuffer[3] =
                            gk_cfa_stat.tile_info.awb_tile_row_start;
                        sendBuffer[4] =
                            gk_cfa_stat.tile_info.awb_tile_width;
                        sendBuffer[5] =
                            gk_cfa_stat.tile_info.awb_tile_height;
                        sendBuffer[6] =
                            gk_cfa_stat.tile_info.awb_y_shift;
                        sendBuffer[7] =
                            gk_cfa_stat.tile_info.awb_rgb_shift;
                        sendBuffer[8] =
                            gk_cfa_stat.tile_info.
                            awb_tile_active_width;
                        sendBuffer[9] =
                            gk_cfa_stat.tile_info.
                            awb_tile_active_height;
                        sendBuffer[10] =
                            gk_cfa_stat.tile_info.awb_min_max_shift;

                        sendBuffer[11] =
                            gk_3a_statistic_config.awb_tile_num_col;
                        sendBuffer[12] =
                            gk_3a_statistic_config.awb_tile_num_row;
                        sendBuffer[13] =
                            gk_3a_statistic_config.awb_tile_col_start;
                        sendBuffer[14] =
                            gk_3a_statistic_config.awb_tile_row_start;
                        sendBuffer[15] =
                            gk_3a_statistic_config.awb_tile_width;
                        sendBuffer[16] =
                            gk_3a_statistic_config.awb_tile_height;
                        sendBuffer[17] =
                            gk_3a_statistic_config.awb_pix_min_value;
                        sendBuffer[18] =
                            gk_3a_statistic_config.awb_pix_max_value;
                        sendBuffer[19] =
                            gk_3a_statistic_config.
                            awb_tile_active_width;
                        sendBuffer[20] =
                            gk_3a_statistic_config.
                            awb_tile_active_height;

                        char sendAWBTilesConfiguration[10];

                        for (i = 0; i < 21; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendAWBTilesConfiguration);
                            send(new_fd, sendAWBTilesConfiguration,
                                k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        _printf("AWB Chunk Setting\n");
                        break;

                    case AFTileConfiguration:
                        gk_isp_get_statistics_raw(handle,
                            &gk_rgb_stat, &gk_cfa_stat);
                        gk_isp_get_config_statistics_params
                            (handle, &gk_3a_statistic_config);

                        sendBuffer[0] =
                            gk_cfa_stat.tile_info.af_tile_num_col;
                        sendBuffer[1] =
                            gk_cfa_stat.tile_info.af_tile_num_row;
                        sendBuffer[2] =
                            gk_cfa_stat.tile_info.af_tile_col_start;
                        sendBuffer[3] =
                            gk_cfa_stat.tile_info.af_tile_row_start;
                        sendBuffer[4] =
                            gk_cfa_stat.tile_info.af_tile_width;
                        sendBuffer[5] =
                            gk_cfa_stat.tile_info.af_tile_height;
                        sendBuffer[6] =
                            gk_cfa_stat.tile_info.af_y_shift;
                        sendBuffer[7] =
                            gk_cfa_stat.tile_info.af_cfa_y_shift;
                        sendBuffer[8] =
                            gk_cfa_stat.tile_info.
                            af_tile_active_width;
                        sendBuffer[9] =
                            gk_cfa_stat.tile_info.
                            af_tile_active_height;

                        sendBuffer[10] =
                            gk_3a_statistic_config.af_tile_num_col;
                        sendBuffer[11] =
                            gk_3a_statistic_config.af_tile_num_row;
                        sendBuffer[12] =
                            gk_3a_statistic_config.af_tile_col_start;
                        sendBuffer[13] =
                            gk_3a_statistic_config.af_tile_row_start;
                        sendBuffer[14] =
                            gk_3a_statistic_config.af_tile_width;
                        sendBuffer[15] =
                            gk_3a_statistic_config.af_tile_height;
                        sendBuffer[16] =
                            gk_3a_statistic_config.
                            af_tile_active_width;
                        sendBuffer[17] =
                            gk_3a_statistic_config.
                            af_tile_active_height;

                        char sendAFTileConfiguration[10];

                        for (i = 0; i < 18; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendAFTileConfiguration);
                            send(new_fd, sendAFTileConfiguration, k,
                                0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);
                        _printf("AF Chunk Setting\n");
                        break;

                    case AFStatisticSetupEx:
                        gk_isp_get_af_statistics_ex
                            (handle, &af_statistic_config_ex);

                        sendBuffer[0] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_mode;
                        sendBuffer[1] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_stage1_enb;
                        sendBuffer[2] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_stage2_enb;
                        sendBuffer[3] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_stage3_enb;
                        sendBuffer[4] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[0];
                        sendBuffer[5] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[1];
                        sendBuffer[6] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[2];
                        sendBuffer[7] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[3];
                        sendBuffer[8] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[4];
                        sendBuffer[9] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[5];
                        sendBuffer[10] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_gain[6];
                        sendBuffer[11] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_shift[0];
                        sendBuffer[12] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_shift[1];
                        sendBuffer[13] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_shift[2];
                        sendBuffer[14] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_shift[3];
                        sendBuffer[15] =
                            af_statistic_config_ex.data1.
                            af_horizontal_filter_bias_off;
                        sendBuffer[16] =
                            af_statistic_config_ex.data1.
                            af_vertical_filter_thresh;
                        sendBuffer[17] =
                            af_statistic_config_ex.data1.
                            af_tile_fv_horizontal_shift;
                        sendBuffer[18] =
                            af_statistic_config_ex.data1.
                            af_tile_fv_horizontal_weight;
                        sendBuffer[19] =
                            af_statistic_config_ex.data1.
                            af_tile_fv_vertical_shift;
                        sendBuffer[20] =
                            af_statistic_config_ex.data1.
                            af_tile_fv_vertical_weight;

                        sendBuffer[21] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_mode;
                        sendBuffer[22] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_stage1_enb;
                        sendBuffer[23] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_stage2_enb;
                        sendBuffer[24] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_stage3_enb;
                        sendBuffer[25] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[0];
                        sendBuffer[26] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[1];
                        sendBuffer[27] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[2];
                        sendBuffer[28] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[3];
                        sendBuffer[29] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[4];
                        sendBuffer[30] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[5];
                        sendBuffer[31] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_gain[6];
                        sendBuffer[32] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_shift[0];
                        sendBuffer[33] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_shift[1];
                        sendBuffer[34] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_shift[2];
                        sendBuffer[35] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_shift[3];
                        sendBuffer[36] =
                            af_statistic_config_ex.data2.
                            af_horizontal_filter_bias_off;
                        sendBuffer[37] =
                            af_statistic_config_ex.data2.
                            af_vertical_filter_thresh;
                        sendBuffer[38] =
                            af_statistic_config_ex.data2.
                            af_tile_fv_horizontal_shift;
                        sendBuffer[39] =
                            af_statistic_config_ex.data2.
                            af_tile_fv_horizontal_weight;
                        sendBuffer[40] =
                            af_statistic_config_ex.data2.
                            af_tile_fv_vertical_shift;
                        sendBuffer[41] =
                            af_statistic_config_ex.data2.
                            af_tile_fv_vertical_weight;

                        char sendAFS[10];

                        for (i = 0; i < 42; i++) {

                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendAFS);
                            send(new_fd, sendAFS, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);
                        _printf("Auto Focus statistic 2\n");
                        break;

                    case ExposureControl:
                        gain_index = gk_get_sensor_gain_index(handle);
                        shutter_index =
                            gk_get_sensor_shutter_time_index(handle);

                        gk_isp_get_rgb_gain(handle, &wb_gain, &dgain); // ISP Dgain

                        sendBuffer[0] = gain_index;
                        sendBuffer[1] = shutter_index;
                        sendBuffer[2] = dgain;

                        char sendEC[10];

                        for (i = 0; i < 3; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendEC);
                            send(new_fd, sendEC, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        _printf("gain_index=%d,shutter_index=%d,dgain=%d\n",gain_index,shutter_index,dgain);

                        _printf("Exposure controlling Loading\n");
                        break;

                    case AWBModel: // Load by ChengBo
                        _printf("Load awb_mode \n");
                        img_awb_get_rb_ratio(handle, &awb_ratio_r,&awb_ratio_b);
                        AWBspeed = img_awb_get_speed(handle);
                        gk_get_3A_ctl_state(handle, &aaa_cntl_station);

                        sendBuffer[0] = awb_ratio_r;
                        sendBuffer[1] = awb_ratio_b;
                        sendBuffer[2] = AWBspeed;
                        sendBuffer[3] = aaa_cntl_station.awb_enable;

                        char sendAWB[10];

                        for(i=0;i<4;i++)
                        {
                            int k;
                            k=Int32ToCharArray(sendBuffer[i],sendAWB);
                            send(new_fd,sendAWB,k,0);
                            send(new_fd," ",1,0);
                        }
                        send(new_fd,"!",1,0);

                        _printf("awb_control_mode %d\n",awb_mode);
                        _printf("AWBspeed %d\n",AWBspeed);
                        break;

                    case AEControl:        // Load by ChengBo
                        //_printf("Load AEControl \n");
                        AECTarget = gk_ae_get_target(handle);
                        autoKnee = img_get_ae_auto_luma_control(handle);
                        speedLevel = img_get_ae_speed(handle);
                        gk_get_3A_ctl_state(handle, &aaa_cntl_station);
                        sendBuffer[0] = AECTarget;
                        sendBuffer[1] = autoKnee;
                        sendBuffer[2] = speedLevel;
                        sendBuffer[3] = aaa_cntl_station.ae_enable;

                        char sendAEControl[10];

                        for (i = 0; i < 4; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendAEControl);
                            send(new_fd, sendAEControl, k, 0);
                            send(new_fd, " ", 1, 0);

                        }
                        send(new_fd, "!", 1, 0);

                        //_printf("AECTarget %d\n", AECTarget);
                        //_printf("autoKnee strength %d\n", autoKnee);
                        //_printf("speedLevel%d\n", speedLevel);
                        break;

                    }
                }
                break;

                case VPS:      //id_Require==Load  ChengBo
                {
                    _printf("case VPS Load!\n");
                    recv(new_fd, recvBuffer, 2000, 0);
                    TCP_get_param(recvBuffer, dataArray);

                    _printf("Load CHIPID : %d\n\n",dataArray[1]);

                    if(dataArray[1] != CHIPPIN)
                    {
                        _printf("Load cmd %d%d%d\n\n",data.layer1,data.layer2,data.layer3);
                        //KEYMACTH = 0;
                        break;
                    }

                    switch (data.layer3) {

                    case stage0:
                        _printf("VPS stage0 Load\n");
                        char sendVPS0[10];

                        for (i = 0; i < 3; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendVPS0);
                            send(new_fd, sendVPS0, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);
                        break;

                    case stage1:
                        _printf("VPS stage1 Load\n");

                        char sendVPS1[10];

                        for (i = 0; i < 27; i++) {
                            int k;
                            k = Int32ToCharArray(sendBuffer[i],
                                sendVPS1);
                            send(new_fd, sendVPS1, k, 0);
                            send(new_fd, " ", 1, 0);
                        }


                        send(new_fd, "!", 1, 0);

                        _printf("VPS stage1 finished\n");
                        break;

                    case stage2:
                        _printf("VPS stage2 Load\n");
                        //recv(new_fd, recvBuffer, 2000, 0);

                        _printf("dataArray is %d\n",
                            dataArray[0]);
                        int VPS3DCnt = 0;

                        ifDebug = dataArray[0];
                        ifDebug = 1;
                        if (ifDebug) {
                            _printf("VPS 3D Debug Tool\n");

                            char sendVPS2[10];
                            _printf("VPS3DCnt %d\n",VPS3DCnt);

                            for (i = 0; i < VPS3DCnt; i++) {
                                int k;

                                k = Int32ToCharArray(sendBuffer
                                    [i], sendVPS2);
                                send(new_fd, sendVPS2, k, 0);
                                send(new_fd, " ", 1, 0);
                                //_printf("sendBuffer[%d] %d\n",i,sendBuffer[i]);
                            }
                            send(new_fd, "!", 1, 0);
                        } else {
                            _printf("VPS 3D IQ Tool\n");

                            char sendVPS2[10];

                            for (i = 0; i < 41; i++) {
                                int k;

                                k = Int32ToCharArray(sendBuffer
                                    [i], sendVPS2);
                                send(new_fd, sendVPS2, k, 0);
                                send(new_fd, " ", 1, 0);
                            }
                            send(new_fd, "!", 1, 0);
                        }

                        _printf("VPS stage2 finished\n");
                        break;

                    case stage3:
                        _printf("VPS stage3 Load\n");

                        printf("dataArray is %d\n",
                            dataArray[0]);

                        _printf("VPS stage3 finished\n");
                        break;
                    }
                }
                break;

                case VIDEO:    // Load by Chengbo
                {
                    _printf("case VIDEO Load!\n");

                    switch (data.layer3) {
                    case Videopar:
                        recv(new_fd, recvBuffer, 2000, 0);
                        TCP_get_param(recvBuffer, dataArray);
                        _printf("streamId is %d\n", dataArray[0]);

                        streamId = dataArray[0];
                        retVal = tuning_venc_get_h264_config(streamId, &vencbps, &vencgop);

                        sendBuffer[0] = vencbps.rate_control_mode;
                        sendBuffer[1] = vencbps.vbr_max_bitrate;
                        sendBuffer[2] = vencbps.vbr_min_bitrate;
                        sendBuffer[3] = vencbps.cbr_avg_bitrate;
                        sendBuffer[4] = vencgop.N;

                        char sendVIDEO[20];

                        for (i = 0; i < 5; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer
                                [i], sendVIDEO);
                            send(new_fd, sendVIDEO, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

#if 0
                        _printf("brcMode    %d\n",vencH264.brcMode);
                        _printf("vbrMinbps  %d\n",vencH264.vbrMinbps);
                        _printf("vbrMaxbps  %d\n",vencH264.vbrMaxbps);
                        _printf("cbrAvgBps  %d\n",vencH264.cbrAvgBps);
                        _printf("gopN       %d\n",vencH264.gopN);
                        _printf("adaptQp    %d\n",vencH264.adaptQp);

#endif

                        _printf("Video Parameter load \n");
                        break;

                    case Encoder:
                        recv(new_fd, recvBuffer, 2000, 0);
                        TCP_get_param(recvBuffer, dataArray);
                        _printf("streamId is %d\n", dataArray[0]);
                        encode_param.stream_id = dataArray[0];

                        gk_isp_get_encode_param(handle, &encode_param);
                        sendBuffer[0]  = encode_param.qp_min_on_I_3a;
                        sendBuffer[1]  = encode_param.qp_max_on_I_3a;
                        sendBuffer[2]  = encode_param.qp_min_on_p_3a;
                        sendBuffer[3]  = encode_param.qp_max_on_p_3a;
                        sendBuffer[4]  = encode_param.aqp_3a;
                        sendBuffer[5]  = encode_param.i_qp_reduce_3a;
                        sendBuffer[6]  = encode_param.p_qp_reduce_3a;
                        sendBuffer[7]  = encode_param.p_intraBiasAdd_3a;
                        sendBuffer[8]  = encode_param.intra_16_16_bias_3a;
                        sendBuffer[9]  = encode_param.intra_4_4_bias_3a;
                        sendBuffer[10] = encode_param.inter_16_16_bias_3a;
                        sendBuffer[11] = encode_param.inter_8_8_bias_3a;
                        sendBuffer[12] = encode_param.me_lamda_qp_offset_3a;
                        sendBuffer[13] = encode_param.alpha_3a;
                        sendBuffer[14] = encode_param.beta_3a;
                        sendBuffer[15] = encode_param.zmv_thresh_3a;
                        sendBuffer[16] = encode_param.uv_qp_offset_3a;
                        sendBuffer[17] = encode_param.IsizeCtlThresh_3a;
                        sendBuffer[18] = encode_param.IsizeCtlStrength_3a;
                        sendBuffer[19] = encode_param.reenc_threshStC_3a;
                        sendBuffer[20] = encode_param.reenc_strengthStC_3a;
                        sendBuffer[21] = encode_param.reenc_threshCtS_3a;
                        sendBuffer[22] = encode_param.reenc_strengthCtS_3a;

                        char sendEncoder[20];

                        for (i = 0; i < 23; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer
                                [i], sendEncoder);
                            send(new_fd, sendEncoder, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        _printf("Load Encoder\n");

                        break;

                    case Fpspar:

                        sendBuffer[0] = venc_fps[0];
                        sendBuffer[1] = venc_fps[1];
                        sendBuffer[2] = venc_fps[2];
                        sendBuffer[3] = venc_fps[3];

                        sendBuffer[4] = get_vi_real_fps();

                        sendBuffer[5] = venc_Set_fps[0] - venc_fps[0];
                        sendBuffer[6] = venc_Set_fps[1] - venc_fps[1];
                        sendBuffer[7] = venc_Set_fps[2] - venc_fps[2];
                        sendBuffer[8] = venc_Set_fps[3] - venc_fps[3];

                        _printf("venc[0] fps %d\n",sendBuffer[0]);
                        _printf("venc[1] fps %d\n",sendBuffer[1]);
                        _printf("venc[2] fps %d\n",sendBuffer[2]);
                        _printf("venc[3] fps %d\n",sendBuffer[3]);

                        _printf("vi      fps %d\n",sendBuffer[4]);

                        _printf("Lost fps[0] %d\n",sendBuffer[5]);
                        _printf("Lost fps[1] %d\n",sendBuffer[6]);
                        _printf("Lost fps[2] %d\n",sendBuffer[7]);
                        _printf("Lost fps[3] %d\n",sendBuffer[8]);

                        char sendFPS[20];
                        for(i=0; i<9; i++)
                        {
                            int k;
                            k = Int32ToCharArray(sendBuffer[i], sendFPS);
                            send(new_fd, sendFPS, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        for (i = 0; i < 4; i++) {
                            venc_fps[i]     = get_venc_real_fps(i);
                            venc_Set_fps[i] = get_venc_setting_fps(i);
                        }

                        printf("Fps Parameter load \n");
                        break;

                    case Pgapar:
                        //TCP_get_param( recvBuffer,  dataArray);
                        _printf("data.layer3 is %d \n",
                            data.layer3);

                        gain_index = gk_get_sensor_gain_index(handle);     //angain index
                        shutter_index = gk_get_sensor_shutter_time_index(handle);     //shutter index
                        gk_isp_get_rgb_gain(handle, &wb_gain, &dgain);     //dgain index

                        //fps
                        fps_t = 30;

                        sendBuffer[0] = gain_index;
                        sendBuffer[1] = dgain;
                        sendBuffer[2] = shutter_index;
                        sendBuffer[3] = fps_t;
                        _printf("sendBuffer[0] %d\n", gain_index);
                        _printf("sendBuffer[1] %d\n", dgain);
                        _printf("sendBuffer[2] %d\n",
                            shutter_index);
                        char sendPGA[10];

                        for (i = 0; i < 3; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendPGA);
                            send(new_fd, sendPGA, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

                        _printf("PGA parameter load \n");
                        break;

                    case LowBit: // Get LowBit
                        {
                            _printf("Get LowBit\n");

#if 1
                            image_low_bitrate_enable = gk_get_low_bitrate_mode(handle);
                            gk_get_low_bitrate_param(handle, &image_low_bitrate,
                                &tolerance_low_bitrate, &delay_frames_low_bitrate);

                            sendBuffer[0] = image_low_bitrate_enable;

                            sendBuffer[1] = image_low_bitrate.sharpness_low_bitrate_ratio;
                            //sendBuffer[2] = image_low_bitrate.dnr3d_low_bitrate_ratio;
                            //sendBuffer[3] = image_low_bitrate.vps_3d_low_bitrate_ratio;
                            //sendBuffer[4] = image_low_bitrate.vps_contrast_low_bitrate_ratio;
                            sendBuffer[5] = image_low_bitrate.sharpen_property_low_bitrate_up_ratio;
                            sendBuffer[6] = image_low_bitrate.sharpen_property_low_bitrate_down_ratio;
                            sendBuffer[7] = image_low_bitrate.cfa_denoise_property_low_bitrate_up_ratio;
                            sendBuffer[8] = image_low_bitrate.cfa_denoise_property_low_bitrate_down_ratio;
                            //sendBuffer[9] = image_low_bitrate.dnr3d_property_low_bitrate_ratio;

                            sendBuffer[10] = image_low_bitrate.sharpness_low_bitrate_param;
                            //sendBuffer[11] = image_low_bitrate.dnr3d_low_bitrate_param;
                            //sendBuffer[12] = image_low_bitrate.vps_3d_low_bitrate_param;
                            //sendBuffer[13] = image_low_bitrate.vps_contrast_low_bitrate_param;
                            sendBuffer[14] = image_low_bitrate.sharpen_property_low_bitrate_up_param;
                            sendBuffer[15] = image_low_bitrate.sharpen_property_low_bitrate_down_param;
                            sendBuffer[16] = image_low_bitrate.cfa_denoise_property_low_bitrate_up_param;
                            sendBuffer[17] = image_low_bitrate.cfa_denoise_property_low_bitrate_down_param;
                            //sendBuffer[18] = image_low_bitrate.dnr3d_property_low_bitrate_param;
                            sendBuffer[19] = tolerance_low_bitrate;
                            sendBuffer[20] = delay_frames_low_bitrate;
#endif

                            char sendLowBit[100];
                            for(i=0; i<21; i++)
                            {
                                int k;
                                k = Int32ToCharArray(sendBuffer[i],
                                    sendLowBit);
                                send(new_fd, sendLowBit, k, 0);
                                send(new_fd, " ", 1, 0);
                                _printf("sendBuffer[%d] : %d\n",i,sendBuffer[i]);
                            }
                            send(new_fd, "!", 1, 0);

                        }
                        break;
                    }
                }
                break;

                case WDR:      // id_Require==Load  ChengBo
                {
                    _printf("case WDR Load!\n");
                    recv(new_fd, recvBuffer, 2000, 0);
                    TCP_get_param(recvBuffer, dataArray);
                    _printf("Load CHIPID : %d\n\n",dataArray[0]);

                    if(dataArray[0] != CHIPPIN)
                    {
                        _printf("Load cmd %d%d%d\n\n",data.layer1,data.layer2,data.layer3);
                        //KEYMACTH = 0;
                        break;
                    }

                    switch (data.layer3) {
                    case globalwdr:
                        _printf("globalwdr Load\n");
                        //TCP_get_param( recvBuffer,  dataArray);
                        //_printf("WDRGet %d\n",WDRGet);

                        sendBuffer[0] = WDRGet;
                        char sendWDRBin[10];

                        int k;
                        k=Int32ToCharArray(sendBuffer[0],sendWDRBin);
                        send(new_fd,sendWDRBin,k,0);
                        send(new_fd," ",1,0);
                        send(new_fd,"!",1,0);
                        break;
                    case localwdr:
                        _printf("localwdr Load\n");
                        tuning_get_LE_str(handle,
                            &tuning_LE_str);

                        sendBuffer[0] = tuning_LE_str.str;
                        sendBuffer[1] = tuning_LE_str.low_or_high;
                        sendBuffer[2] = tuning_LE_str.enable;
                        sendBuffer[3] = tuning_LE_str.radius;
                        sendBuffer[4] = tuning_LE_str.luma_weight_shift;
                        sendBuffer[5] = tuning_LE_str.luma_weight_red;
                        sendBuffer[6] = tuning_LE_str.luma_weight_green;
                        sendBuffer[7] = tuning_LE_str.luma_weight_blue;

                        char sendLocWDR[10];

                        for (i = 0; i < 8; i++) {
                            int k;

                            k = Int32ToCharArray(sendBuffer[i],
                                sendLocWDR);
                            send(new_fd, sendLocWDR, k, 0);
                            send(new_fd, " ", 1, 0);
                        }
                        send(new_fd, "!", 1, 0);

#if 1
                        printf("tuning_LE_str.str               %d\n",tuning_LE_str.str);
                        printf("tuning_LE_str.low_or_high       %d\n",tuning_LE_str.low_or_high);

                        printf("tuning_LE_str.enable               %d\n",tuning_LE_str.enable);
                        printf("tuning_LE_str.radius               %d\n",tuning_LE_str.radius);
                        printf("tuning_LE_str.luma_weight_shift    %d\n",tuning_LE_str.luma_weight_shift);
                        printf("tuning_LE_str.luma_weight_red      %d\n",tuning_LE_str.luma_weight_red);
                        printf("tuning_LE_str.luma_weight_green    %d\n",tuning_LE_str.luma_weight_green);
                        printf("tuning_LE_str.luma_weight_blue     %d\n",tuning_LE_str.luma_weight_blue);
#endif

                        break;
                    case LensDis:
                        {
                        warp_config_t bin_warp_config;
                        _printf("Lense Warp get\n");
                        img_dsp_get_dewarp(handle,&bin_warp_config);
                        sendBuffer[0] = bin_warp_config.warp_control;
                        sendBuffer[1] = bin_warp_config.warp_multiplier;
                        sendBuffer[2] = bin_warp_config.vert_warp_enable;
                        sendBuffer[3] = bin_warp_config.vert_warp_multiplier;

                        _printf("Hori Config : %d\n",bin_warp_config.warp_control);
                        _printf("Hori Level  : %d\n",bin_warp_config.warp_multiplier);

                        _printf("Vert Config : %d\n",bin_warp_config.vert_warp_enable);
                        _printf("Hori Level  : %d\n",bin_warp_config.vert_warp_multiplier);

                        char sendLensDis[10];

                        for(i=0;i<4;i++)
                        {
                            int k;
                            k=Int32ToCharArray(sendBuffer[i],sendLensDis);
                            send(new_fd,sendLensDis,k,0);
                            send(new_fd," ",1,0);
                        }
                        send(new_fd,"!",1,0);
                        }

                        break;

                    case LensShade:

                        _printf("Lense Shade get\n");

                        shutter_index    = gk_get_sensor_shutter_time_index(handle);
                        ae_target      = gk_ae_get_target_ratio(handle);
                        flicker_mode   = gk_ae_get_antiflicker(handle);
                        les_shading_enable = gk_get_lens_shading_enable(handle);

                        sendBuffer[0] = shutter_index;
                        sendBuffer[1] = ae_target;
                        sendBuffer[2] = flicker_mode;
                        sendBuffer[3] = les_shading_enable;
                        sendBuffer[4] = detect_finished;
                        sendBuffer[5] = compensate_ratio;

                        char sendLensShade[20];

                        for(i=0;i<6;i++)
                        {
                            int k;
                            k=Int32ToCharArray(sendBuffer[i],sendLensShade);
                            send(new_fd,sendLensShade,k,0);
                            send(new_fd," ",1,0);
                        }
                        send(new_fd,"!",1,0);

#if 1
                        _printf("shutter_idx = %d\n", shutter_index);
                        _printf("ae_target   = %d\n", ae_target);
                        _printf("antiflicker = %d\n", flicker_mode);
                        _printf("enable      = %d\n", les_shading_enable);
                        _printf("detect_finished = %d\n", detect_finished);
#endif

                        break;

                    case BlcAvg:
                        tuning_BLC_Avg_RGB();
                        break;
                    case DEMOSAIC:
                        {
                            gk_isp_get_demosaic_noise_reduction_new(handle, &demosaic_noise_reduction);
							sendBuffer[0]  = demosaic_noise_reduction.thresh_grad_clipping;
							sendBuffer[1]  = demosaic_noise_reduction.thresh_grad_noise;
							sendBuffer[2]  = demosaic_noise_reduction.thresh_activity;
							sendBuffer[3]  = demosaic_noise_reduction.thresh_activity_diff;
							sendBuffer[4]  = demosaic_noise_reduction.threshedge;
							sendBuffer[5]  = demosaic_noise_reduction.shift;
							sendBuffer[6]  = demosaic_noise_reduction.demoire_A_Y;
							sendBuffer[7]  = demosaic_noise_reduction.demoire_B_Y;
							sendBuffer[8]  = demosaic_noise_reduction.demoire_C_Y;
							sendBuffer[9]  = demosaic_noise_reduction.demoire_D_Y;
							sendBuffer[10] = demosaic_noise_reduction.demoire_A_C;
							sendBuffer[11] = demosaic_noise_reduction.demoire_B_C;
							sendBuffer[12] = demosaic_noise_reduction.demoire_C_C;
							sendBuffer[13] = demosaic_noise_reduction.demoire_D_C;
							sendBuffer[14] = demosaic_noise_reduction.conti_th1;
							sendBuffer[15] = demosaic_noise_reduction.conti_th2;
							sendBuffer[16] = demosaic_noise_reduction.conti_th3;
							sendBuffer[17] = demosaic_noise_reduction.conti_th4;
#if 1

                            _printf("demosaic_noise_reduction.thresh_grad_clipping :%d\n",demosaic_noise_reduction.thresh_grad_clipping);
                            _printf("demosaic_noise_reduction.thresh_grad_noise    :%d\n",demosaic_noise_reduction.thresh_grad_noise);
                            _printf("demosaic_noise_reduction.thresh_activity      :%d\n",demosaic_noise_reduction.thresh_activity);
                            _printf("demosaic_noise_reduction.thresh_activity_diff :%d\n",demosaic_noise_reduction.thresh_activity_diff);
                            _printf("demosaic_noise_reduction.threshedge           :%d\n",demosaic_noise_reduction.threshedge);
                            _printf("demosaic_noise_reduction.shift                :%d\n",demosaic_noise_reduction.shift);
                            _printf("demosaic_noise_reduction.demoire_A_Y          :%d\n",demosaic_noise_reduction.demoire_A_Y);
                            _printf("demosaic_noise_reduction.demoire_B_Y          :%d\n",demosaic_noise_reduction.demoire_B_Y);
                            _printf("demosaic_noise_reduction.demoire_C_Y          :%d\n",demosaic_noise_reduction.demoire_C_Y);
                            _printf("demosaic_noise_reduction.demoire_D_Y          :%d\n",demosaic_noise_reduction.demoire_D_Y);
                            _printf("demosaic_noise_reduction.demoire_A_C          :%d\n",demosaic_noise_reduction.demoire_A_C);
                            _printf("demosaic_noise_reduction.demoire_B_C          :%d\n",demosaic_noise_reduction.demoire_B_C);
                            _printf("demosaic_noise_reduction.demoire_C_C          :%d\n",demosaic_noise_reduction.demoire_C_C);
                            _printf("demosaic_noise_reduction.demoire_D_C          :%d\n",demosaic_noise_reduction.demoire_D_C);
                            _printf("demosaic_noise_reduction.conti_th1            :%d\n",demosaic_noise_reduction.conti_th1);
                            _printf("demosaic_noise_reduction.conti_th2            :%d\n",demosaic_noise_reduction.conti_th2);
                            _printf("demosaic_noise_reduction.conti_th3            :%d\n",demosaic_noise_reduction.conti_th3);
                            _printf("demosaic_noise_reduction.conti_th4            :%d\n",demosaic_noise_reduction.conti_th4);

#endif

                            char sendDemosaic[20];
                            for(i=0;i<18;i++)
                            {
                                int k;
                                k=Int32ToCharArray(sendBuffer[i],sendDemosaic);
                                send(new_fd,sendDemosaic,k,0);
                                send(new_fd," ",1,0);
                            }
                            send(new_fd,"!",1,0);
                        }
                        break;

                    }
                }
                break;

                case Denoise: //Load
    			{
                    _printf("7202 Denoise\n");
    				switch (data.layer3)
    				{
    					case CFAdenoise:
						{
                            _printf("NLM CFAdenoise Load\n");
							gk_isp_get_nlm_noise_filter(handle,&nlm_noise_filter_info);

                            sendBuffer[0]  = nlm_noise_filter_info.nlm_noise_filter_enable;
                            sendBuffer[1]  = nlm_noise_filter_info.nlm_noise_filter_mv_enable;
                            sendBuffer[2]  = nlm_noise_filter_info.nlm_noise_filter_luma_offset_red;
                            sendBuffer[3]  = nlm_noise_filter_info.nlm_noise_filter_luma_offset_green;
                            sendBuffer[4]  = nlm_noise_filter_info.nlm_noise_filter_luma_offset_blue;
                            sendBuffer[5]  = nlm_noise_filter_info.nlm_noise_filter_strength_red;
                            sendBuffer[6]  = nlm_noise_filter_info.nlm_noise_filter_strength_green;
                            sendBuffer[7]  = nlm_noise_filter_info.nlm_noise_filter_strength_blue;
                            sendBuffer[8]  = nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_red;
                            sendBuffer[9]  = nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_green;
                            sendBuffer[10] = nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_blue;
                            sendBuffer[11] = nlm_noise_filter_info.nlm_noise_filter_mv_strength_red;
                            sendBuffer[12] = nlm_noise_filter_info.nlm_noise_filter_mv_strength_green;
                            sendBuffer[13] = nlm_noise_filter_info.nlm_noise_filter_mv_strength_blue;
                            sendBuffer[14] = nlm_noise_filter_info.nlm_noise_filter_factor;
                            sendBuffer[15] = nlm_noise_filter_info.nlm_max_change;

#if 1
                            _printf("nlm_noise_filter_info.nlm_noise_filter_enable               %d\n",nlm_noise_filter_info.nlm_noise_filter_enable);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_mv_enable            %d\n",nlm_noise_filter_info.nlm_noise_filter_mv_enable);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_luma_offset_red      %d\n",nlm_noise_filter_info.nlm_noise_filter_luma_offset_red);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_luma_offset_green    %d\n",nlm_noise_filter_info.nlm_noise_filter_luma_offset_green);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_luma_offset_blue     %d\n",nlm_noise_filter_info.nlm_noise_filter_luma_offset_blue);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_strength_red         %d\n",nlm_noise_filter_info.nlm_noise_filter_strength_red);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_strength_green       %d\n",nlm_noise_filter_info.nlm_noise_filter_strength_green);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_strength_blue        %d\n",nlm_noise_filter_info.nlm_noise_filter_strength_blue);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_red   %d\n",nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_red);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_green %d\n",nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_green);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_blue  %d\n",nlm_noise_filter_info.nlm_noise_filter_mv_luma_offset_blue);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_mv_strength_red      %d\n",nlm_noise_filter_info.nlm_noise_filter_mv_strength_red);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_mv_strength_green    %d\n",nlm_noise_filter_info.nlm_noise_filter_mv_strength_green);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_mv_strength_blue     %d\n",nlm_noise_filter_info.nlm_noise_filter_mv_strength_blue);
                            _printf("nlm_noise_filter_info.nlm_noise_filter_factor               %d\n",nlm_noise_filter_info.nlm_noise_filter_factor);
                            _printf("nlm_noise_filter_info.nlm_max_change                        %d\n",nlm_noise_filter_info.nlm_max_change);

#endif

							char sendCFAdenoise[100];
                        	for(i=0;i<16;i++)
                        	{
                            	int k;
                            	k=Int32ToCharArray(sendBuffer[i],sendCFAdenoise);
                            	send(new_fd,sendCFAdenoise,k,0);
                            	send(new_fd," ",1,0);
                        	}
                        	send(new_fd,"!",1,0);

						}
    					break;

    					case VGFdenoise:
						{
                            gk_isp_get_variance_guided_filter(handle,&variance_guided_filter_info);

                            sendBuffer[0]  = variance_guided_filter_info.vgf_enable;
                            sendBuffer[1]  = variance_guided_filter_info.vgf_bpc_enable;
                            sendBuffer[2]  = variance_guided_filter_info.vgf_mv_enable;
                            sendBuffer[3]  = variance_guided_filter_info.vgf_mv_offset;
                            sendBuffer[4]  = variance_guided_filter_info.vgf_cutoff_th1;
                            sendBuffer[5]  = variance_guided_filter_info.vgf_cutoff_value1;
                            sendBuffer[6]  = variance_guided_filter_info.vgf_cutoff_th2;
                            sendBuffer[7]  = variance_guided_filter_info.vgf_cutoff_value2;
                            sendBuffer[8]  = variance_guided_filter_info.vgf_bpc_vgf_th;
                            sendBuffer[9]  = variance_guided_filter_info.vgf_bpc_w_th;
                            sendBuffer[10] = variance_guided_filter_info.vgf_bpc_b_th;
                            sendBuffer[11] = variance_guided_filter_info.vgf_edge_th;
                            sendBuffer[12] = variance_guided_filter_info.vgf_edge_wide_weight;
                            sendBuffer[13] = variance_guided_filter_info.vgf_edge_narrow_weight;
                            sendBuffer[14] = variance_guided_filter_info.vgf_grad_th;
                            sendBuffer[15] = variance_guided_filter_info.vgf_disp_info_en;
                            sendBuffer[16] = variance_guided_filter_info.vgf_disp_var_th1;
                            sendBuffer[17] = variance_guided_filter_info.vgf_disp_var_th2;
                            sendBuffer[18] = variance_guided_filter_info.vgf_disp_grad_th1;
                            sendBuffer[19] = variance_guided_filter_info.vgf_disp_grad_th2;
                            sendBuffer[20] = variance_guided_filter_info.vgf_edge_var_th;
                            sendBuffer[21] = variance_guided_filter_info.vgf_edge_str_th1;
                            sendBuffer[22] = variance_guided_filter_info.edge_offset;

							sendBuffer[23] = variance_guided_filter_info.lut_offset_r;
                            sendBuffer[24] = variance_guided_filter_info.lut_offset_g;
                            sendBuffer[25] = variance_guided_filter_info.lut_offset_b;
							sendBuffer[26] = variance_guided_filter_info.strength;

#if 0
                            for(i=0;i<256;i++)
                            {
                                sendBuffer[26 + i] = variance_guided_filter_info.table.lookup_table[i];
                            }
#endif

#if 1
                            _printf("variance_guided_filter_info.vgf_enable              %d\n",variance_guided_filter_info.vgf_enable);
                            _printf("variance_guided_filter_info.vgf_bpc_enable          %d\n",variance_guided_filter_info.vgf_bpc_enable);
                            _printf("variance_guided_filter_info.vgf_mv_enable           %d\n",variance_guided_filter_info.vgf_mv_enable);
                            _printf("variance_guided_filter_info.vgf_mv_offset           %d\n",variance_guided_filter_info.vgf_mv_offset);
                            _printf("variance_guided_filter_info.vgf_cutoff_th1          %d\n",variance_guided_filter_info.vgf_cutoff_th1);
                            _printf("variance_guided_filter_info.vgf_cutoff_value1       %d\n",variance_guided_filter_info.vgf_cutoff_value1);
                            _printf("variance_guided_filter_info.vgf_cutoff_th2          %d\n",variance_guided_filter_info.vgf_cutoff_th2);
                            _printf("variance_guided_filter_info.vgf_cutoff_value2       %d\n",variance_guided_filter_info.vgf_cutoff_value2);
                            _printf("variance_guided_filter_info.vgf_bpc_vgf_th          %d\n",variance_guided_filter_info.vgf_bpc_vgf_th);
                            _printf("variance_guided_filter_info.vgf_bpc_w_th            %d\n",variance_guided_filter_info.vgf_bpc_w_th);
                            _printf("variance_guided_filter_info.vgf_bpc_b_th            %d\n",variance_guided_filter_info.vgf_bpc_b_th);
                            _printf("variance_guided_filter_info.vgf_edge_th             %d\n",variance_guided_filter_info.vgf_edge_th);
                            _printf("variance_guided_filter_info.vgf_edge_wide_weight    %d\n",variance_guided_filter_info.vgf_edge_wide_weight);
                            _printf("variance_guided_filter_info.vgf_edge_narrow_weight  %d\n",variance_guided_filter_info.vgf_edge_narrow_weight);
                            _printf("variance_guided_filter_info.vgf_grad_th             %d\n",variance_guided_filter_info.vgf_grad_th);
                            _printf("variance_guided_filter_info.vgf_disp_info_en        %d\n",variance_guided_filter_info.vgf_disp_info_en);
                            _printf("variance_guided_filter_info.vgf_disp_var_th1        %d\n",variance_guided_filter_info.vgf_disp_var_th1);
                            _printf("variance_guided_filter_info.vgf_disp_var_th2        %d\n",variance_guided_filter_info.vgf_disp_var_th2);
                            _printf("variance_guided_filter_info.vgf_disp_grad_th1       %d\n",variance_guided_filter_info.vgf_disp_grad_th1);
                            _printf("variance_guided_filter_info.vgf_disp_grad_th2       %d\n",variance_guided_filter_info.vgf_disp_grad_th2);
                            _printf("variance_guided_filter_info.vgf_edge_var_th         %d\n",variance_guided_filter_info.vgf_edge_var_th);
                            _printf("variance_guided_filter_info.vgf_edge_str_th1        %d\n",variance_guided_filter_info.vgf_edge_str_th1);
                            _printf("variance_guided_filter_info.edge_offset             %d\n",variance_guided_filter_info.edge_offset);
                            _printf("variance_guided_filter_info.lut_offset_r            %d\n",variance_guided_filter_info.lut_offset_r);
                            _printf("variance_guided_filter_info.lut_offset_g            %d\n",variance_guided_filter_info.lut_offset_g);
                            _printf("variance_guided_filter_info.lut_offset_b            %d\n",variance_guided_filter_info.lut_offset_b);
							_printf("variance_guided_filter_info.strength                %d\n",variance_guided_filter_info.strength);

#endif
                            char sendVGFdenoise[100];
                            for(i=0;i<27;i++)
                            {
                                int k;
                                k=Int32ToCharArray(sendBuffer[i],sendVGFdenoise);
                                send(new_fd,sendVGFdenoise,k,0);
                                send(new_fd," ",1,0);
                            }
                            send(new_fd,"!",1,0);
                        }
    					break;

    					case BLFdenoise:
						{
							int simr_coefs[48];
                            int simr_mv_coefs[48];
                            int sims_coefs[6];
                            int sims_mv_coefs[6];

#if 0

							if (_ioctl(fdmedia, GK_MEDIA_IOC_IMG_GET_BILATERAL_FILTER, &bilateralFilter) < 0) {
                                    perror("GK_MEDIA_IOC_IMG_GET_BILATERAL_FILTER");
                                    _printf("error: get bilateral filter failed\n");
                                    return -1;
                            }

#endif

                            gk_isp_get_bilateral_filter(handle, &bilateral_filter_info);
                            blf_template_update(simr_coefs, simr_mv_coefs, sims_coefs, sims_mv_coefs,
                                                 &bilateral_filter_info);

                            sendBuffer[0] = bilateral_filter_info.y_enable;
                            sendBuffer[1] = bilateral_filter_info.c_enable;
                            sendBuffer[2] = bilateral_filter_info.use_mv;

                            sendBuffer[3] = bilateral_filter_info.simr_str;
                            sendBuffer[4] = bilateral_filter_info.sims_str;
                            sendBuffer[5] = bilateral_filter_info.simr_mv_str;
                            sendBuffer[6] = bilateral_filter_info.sims_mv_str;
                            sendBuffer[7] = bilateral_filter_info.adj_enable;
                            sendBuffer[8] = bilateral_filter_info.th_enable;
                            sendBuffer[9] = bilateral_filter_info.multi_coff;
                            sendBuffer[10] = bilateral_filter_info.min_mv_str;


#if 1
                            _printf("bilateral_filter_info.y_enable    :%d\n",bilateral_filter_info.y_enable);
                            _printf("bilateral_filter_info.c_enable    :%d\n",bilateral_filter_info.c_enable);
                            _printf("bilateral_filter_info.use_mv      :%d\n",bilateral_filter_info.use_mv);
                            _printf("bilateral_filter_info.simr_str    :%d\n",bilateral_filter_info.simr_str);
                            _printf("bilateral_filter_info.sims_str    :%d\n",bilateral_filter_info.sims_str);
                            _printf("bilateral_filter_info.simr_mv_str :%d\n",bilateral_filter_info.simr_mv_str);
                            _printf("bilateral_filter_info.sims_mv_str :%d\n",bilateral_filter_info.sims_mv_str);
#endif

                            //simr & simr_mv
                            for(i=0; i<48; i++)
                            {
                                sendBuffer[7 + i] = simr_coefs[i];
                                sendBuffer[7 + 48 + i] = simr_mv_coefs[i];
                            }

                            //sims & sims_mv 6
                            for(i=0; i<6; i++)
                            {
                                sendBuffer[7 + 48 + 48 + i] = sims_coefs[i];
                                sendBuffer[7 + 48 + 48 + 6 + i] = sims_mv_coefs[i];
                            }
#if 1
                            //simr & simr_mv
                            for(i=0; i<48; i++)
                            {
                                _printf("simr_coefs[%d]    %d\n",i,simr_coefs[i]);
                                _printf("simr_mv_coefs[%d] %d\n",i,simr_mv_coefs[i]);
                            }

                            //sims & sims_mv 6
                            for(i=0; i<6; i++)
                            {
                                _printf("sims_coefs[%d]    %d\n",i,sims_coefs[i]);
                                _printf("sims_mv_coefs[%d] %d\n",i,sims_mv_coefs[i]);
                            }

#endif
                            sendBuffer[115] = bilateral_filter_info.adj_enable;
                            sendBuffer[116] = bilateral_filter_info.th_enable;
                            sendBuffer[117] = bilateral_filter_info.multi_coff;
                            sendBuffer[118] = bilateral_filter_info.min_mv_str;

                            char sendBLFdenoise[100];
                            for(i=0;i<119;i++)
                            {
                                int k;
                                k=Int32ToCharArray(sendBuffer[i],sendBLFdenoise);
                                send(new_fd,sendBLFdenoise,k,0);
                                send(new_fd," ",1,0);
                            }
                            send(new_fd,"!",1,0);

						}
    					break;

    					case GMDCFAdenoise:
						{
							gk_isp_get_gmd_cfa(handle, &gmd_cfa_info);

                            sendBuffer[0] = gmd_cfa_info.en;
                            sendBuffer[1] = gmd_cfa_info.gamma_en;
                            sendBuffer[2] = gmd_cfa_info.blur_en_cfa;
                            sendBuffer[3] = gmd_cfa_info.cfa_interp_en;
                            sendBuffer[4] = gmd_cfa_info.noise_cfa;
                            sendBuffer[5] = gmd_cfa_info.black_level_globe_offset_green;
                            sendBuffer[6] = gmd_cfa_info.digital_gain_multiplier_green;
                            sendBuffer[7] = gmd_cfa_info.digital_gain_shift_green;
#if 1
                            _printf("gmd_cfa_info.en                             %d\n",gmd_cfa_info.en);
                            _printf("gmd_cfa_info.gamma_en                       %d\n",gmd_cfa_info.gamma_en);
                            _printf("gmd_cfa_info.blur_en_cfa                    %d\n",gmd_cfa_info.blur_en_cfa);
                            _printf("gmd_cfa_info.cfa_interp_en                  %d\n",gmd_cfa_info.cfa_interp_en);
                            _printf("gmd_cfa_info.noise_cfa                      %d\n",gmd_cfa_info.noise_cfa);
                            _printf("gmd_cfa_info.black_level_globe_offset_green %d\n",gmd_cfa_info.black_level_globe_offset_green);
                            _printf("gmd_cfa_info.digital_gain_multiplier_green  %d\n",gmd_cfa_info.digital_gain_multiplier_green);
                            _printf("gmd_cfa_info.digital_gain_shift_green       %d\n",gmd_cfa_info.digital_gain_shift_green);

#endif
					        char sendGMDCFAdenoise[100];
							for(i=0;i<8;i++)
							{
								int k;
								k=Int32ToCharArray(sendBuffer[i],sendGMDCFAdenoise);
								send(new_fd,sendGMDCFAdenoise,k,0);
								send(new_fd," ",1,0);
							}
							send(new_fd,"!",1,0);
						}
    					break;

    					case PMVdenoise:
						{
							gk_isp_get_pmv(handle,&pmv_info);

                            sendBuffer[0]  = pmv_info.bpc_cor_enable;
                            sendBuffer[1]  = pmv_info.psc_cor_enable;
                            sendBuffer[2]  = pmv_info.lea_expand_enable;
                            sendBuffer[3]  = pmv_info.dpf_exphor1_enable;
                            sendBuffer[4]  = pmv_info.dpf_expver_enable;
                            sendBuffer[5]  = pmv_info.bpc_cor_range;
                            sendBuffer[6]  = pmv_info.bpc_cor_thresh;
                            sendBuffer[7]  = pmv_info.psc_cor_range;
                            sendBuffer[8]  = pmv_info.psc_feedthrough;
                            sendBuffer[9]  = pmv_info.lea_expand_range;
                            sendBuffer[10] = pmv_info.dpf_exphor1_range;
                            sendBuffer[11] = pmv_info.dpf_expver_range;

#if 1

                            _printf("pmv_info.bpc_cor_enable        : %d\n",pmv_info.bpc_cor_enable);
                            _printf("pmv_info.psc_cor_enable        : %d\n",pmv_info.psc_cor_enable);
                            _printf("pmv_info.lea_expand_enable     : %d\n",pmv_info.lea_expand_enable);
                            _printf("pmv_info.dpf_exphor1_enable    : %d\n",pmv_info.dpf_exphor1_enable);
                            _printf("pmv_info.dpf_expver_enable     : %d\n",pmv_info.dpf_expver_enable);

                            _printf("pmv_info.bpc_cor_range         : %d\n",pmv_info.bpc_cor_range);
                            _printf("pmv_info.bpc_cor_thresh        : %d\n",pmv_info.bpc_cor_thresh);
                            _printf("pmv_info.psc_cor_range         : %d\n",pmv_info.psc_cor_range);
                            _printf("pmv_info.psc_feedthrough       : %d\n",pmv_info.psc_feedthrough);
                            _printf("pmv_info.lea_expand_range      : %d\n",pmv_info.lea_expand_range);
                            _printf("pmv_info.dpf_exphor1_range     : %d\n",pmv_info.dpf_exphor1_range);
                            _printf("pmv_info.dpf_expver_range      : %d\n",pmv_info.dpf_expver_range);

#endif

                            char sendPMVdenoise[100];
                            for(i=0;i<12;i++)
                            {
                                int k;
                                k=Int32ToCharArray(sendBuffer[i],sendPMVdenoise);
                                send(new_fd,sendPMVdenoise,k,0);
                                send(new_fd," ",1,0);
                            }
                            send(new_fd,"!",1,0);

						}
    					break;

    					case GMF3Ddenoise:
						{
                            gk_isp_get_globle_motion_filter(handle, &globle_motion_filter_info);

                            sendBuffer[0] = globle_motion_filter_info.enable;
                            sendBuffer[1] = globle_motion_filter_info.gmd_sel;
                            sendBuffer[2] = globle_motion_filter_info.lpf_enable;
                            sendBuffer[3] = globle_motion_filter_info.noise_l;
                            sendBuffer[4] = globle_motion_filter_info.noise_h;
                            sendBuffer[5] = globle_motion_filter_info.lpf3x3;
                            sendBuffer[6] = globle_motion_filter_info.lpf_str;
                            sendBuffer[7]  = globle_motion_filter_info.maxchange_y_static;
                            sendBuffer[8]  = globle_motion_filter_info.maxchange_c_static;
                            sendBuffer[9]  = globle_motion_filter_info.maxchange_y_move;
                            sendBuffer[10] = globle_motion_filter_info.maxchange_c_move;

							sendBuffer[11] = globle_motion_filter_info.static_alpha_y_max;
							sendBuffer[12] = globle_motion_filter_info.static_alpha_y_min;
							sendBuffer[13] = globle_motion_filter_info.static_threshold_y_1;
							sendBuffer[14] = globle_motion_filter_info.static_threshold_y_2;
							sendBuffer[15] = globle_motion_filter_info.static_alpha_uv_max;
							sendBuffer[16] = globle_motion_filter_info.static_alpha_uv_min;
							sendBuffer[17] = globle_motion_filter_info.static_threshold_uv_1;
							sendBuffer[18] = globle_motion_filter_info.static_threshold_uv_2;
							sendBuffer[19] = globle_motion_filter_info.move_alpha_y_max;
							sendBuffer[20] = globle_motion_filter_info.move_alpha_y_min;
							sendBuffer[21] = globle_motion_filter_info.move_threshold_y_1;
							sendBuffer[22] = globle_motion_filter_info.move_threshold_y_2;
							sendBuffer[23] = globle_motion_filter_info.move_alpha_uv_max;
							sendBuffer[24] = globle_motion_filter_info.move_alpha_uv_min;
							sendBuffer[25] = globle_motion_filter_info.move_threshold_uv_1;
							sendBuffer[26] = globle_motion_filter_info.move_threshold_uv_2;
                            sendBuffer[27] = globle_motion_filter_info.adj_enable;
                            sendBuffer[28] = globle_motion_filter_info.th_enable ;

                            sendBuffer[29] = globle_motion_filter_info.alpha_multi_coff;
                            sendBuffer[30] = globle_motion_filter_info.max_change_multi_coff;
                            sendBuffer[31] = globle_motion_filter_info.alpha_th;
                            sendBuffer[32] = globle_motion_filter_info.max_change_th;

                            sendBuffer[33] = globle_motion_filter_info.exp_str_y;
                            sendBuffer[34] = globle_motion_filter_info.exp_str_uv;

							sendBuffer[35] = 0;//globle_motion_filter_info.gmf_mv_percent[0] ;
                            //printf("%d  %d %d\n",globle_motion_filter_info.gmf_mv_percent[0],globle_motion_filter_info.exp_str_y,globle_motion_filter_info.exp_str_uv);
							sendBuffer[36]	= globle_motion_filter_info.static_alpha_y_max_2;
							sendBuffer[37]	= globle_motion_filter_info.static_alpha_y_min_2;
							sendBuffer[38]	= globle_motion_filter_info.static_threshold_y_12;
							sendBuffer[39]	= globle_motion_filter_info.static_threshold_y_22;
							sendBuffer[40]	= globle_motion_filter_info.static_alpha_uv_max_2;
							sendBuffer[41]	= globle_motion_filter_info.static_alpha_uv_min_2;
							sendBuffer[42]	= globle_motion_filter_info.static_threshold_uv_12;
							sendBuffer[43]	= globle_motion_filter_info.static_threshold_uv_22;
							sendBuffer[44]	= globle_motion_filter_info.static_alpha_y_max_3;
							sendBuffer[45]	= globle_motion_filter_info.static_alpha_y_min_3;
							sendBuffer[46]	= globle_motion_filter_info.static_threshold_y_13;
							sendBuffer[47]	= globle_motion_filter_info.static_threshold_y_23;
							sendBuffer[48]	= globle_motion_filter_info.static_alpha_uv_max_3;
							sendBuffer[49]	= globle_motion_filter_info.static_alpha_uv_min_3;
							sendBuffer[50]	= globle_motion_filter_info.static_threshold_uv_13;
							sendBuffer[51]	= globle_motion_filter_info.static_threshold_uv_23;
							sendBuffer[52]	= globle_motion_filter_info.move_alpha_y_max_2;
							sendBuffer[53]	= globle_motion_filter_info.move_alpha_y_min_2;
							sendBuffer[54]	= globle_motion_filter_info.move_threshold_y_12;
							sendBuffer[55]	= globle_motion_filter_info.move_threshold_y_22;
							sendBuffer[56]	= globle_motion_filter_info.move_alpha_uv_max_2;
							sendBuffer[57]	= globle_motion_filter_info.move_alpha_uv_min_2;
							sendBuffer[58]	= globle_motion_filter_info.move_threshold_uv_12;
							sendBuffer[59]	= globle_motion_filter_info.move_threshold_uv_22;
							sendBuffer[60]	= globle_motion_filter_info.move_alpha_y_max_3;
							sendBuffer[61]	= globle_motion_filter_info.move_alpha_y_min_3;
							sendBuffer[62]	= globle_motion_filter_info.move_threshold_y_13;
							sendBuffer[63]	= globle_motion_filter_info.move_threshold_y_23;
							sendBuffer[64]	= globle_motion_filter_info.move_alpha_uv_max_3;
							sendBuffer[65]	= globle_motion_filter_info.move_alpha_uv_min_3;
							sendBuffer[66]	= globle_motion_filter_info.move_threshold_uv_13;
							sendBuffer[67]	= globle_motion_filter_info.move_threshold_uv_23;

                            //2018-08-28
							//2019-02-14

                            sendBuffer[68] = globle_motion_filter_info.mv_percent_th1;
                            sendBuffer[69] = globle_motion_filter_info.mv_percent_th2;
                            sendBuffer[70] = globle_motion_filter_info.mv_percent_th3;
                            sendBuffer[71] = globle_motion_filter_info.mv_delta;
                            sendBuffer[72] = globle_motion_filter_info.mv_count;
							sendBuffer[73] = globle_motion_filter_info.luma_diff_th1;
                            sendBuffer[74] = globle_motion_filter_info.luma_diff_th2;
							//printf("%d %d %d %d %d %d\n",globle_motion_filter_info.mv_percent_th1,
							//	globle_motion_filter_info.mv_percent_th2,globle_motion_filter_info.mv_percent_th3,
							//	globle_motion_filter_info.mv_delta,globle_motion_filter_info.mv_count,sendBuffer[72]);

#if 1
                            _printf("globle_motion_filter_info.enable                   %d\n",globle_motion_filter_info.enable);
                            _printf("globle_motion_filter_info.gmd_sel                  %d\n",globle_motion_filter_info.gmd_sel);
                            _printf("globle_motion_filter_info.lpf_enable               %d\n",globle_motion_filter_info.lpf_enable);
                            _printf("globle_motion_filter_info.noise_l                  %d\n",globle_motion_filter_info.noise_l);
                            _printf("globle_motion_filter_info.noise_h                  %d\n",globle_motion_filter_info.noise_h);
                            _printf("globle_motion_filter_info.lpf3x3                   %d\n",globle_motion_filter_info.lpf3x3);
                            _printf("globle_motion_filter_info.lpf_str                  %d\n",globle_motion_filter_info.lpf_str);
                            _printf("globle_motion_filter_info.maxchange_y_static       %d\n",globle_motion_filter_info.maxchange_y_static);
                            _printf("globle_motion_filter_info.maxchange_c_static       %d\n",globle_motion_filter_info.maxchange_c_static);
                            _printf("globle_motion_filter_info.maxchange_y_move         %d\n",globle_motion_filter_info.maxchange_y_move);
                            _printf("globle_motion_filter_info.maxchange_c_move         %d\n",globle_motion_filter_info.maxchange_c_move);
                            _printf("globle_motion_filter_info.static_alpha_y_max       %d\n",globle_motion_filter_info.static_alpha_y_max);
                            _printf("globle_motion_filter_info.static_alpha_y_min       %d\n",globle_motion_filter_info.static_alpha_y_min);
                            _printf("globle_motion_filter_info.static_threshold_y_1     %d\n",globle_motion_filter_info.static_threshold_y_1);
                            _printf("globle_motion_filter_info.static_threshold_y_2     %d\n",globle_motion_filter_info.static_threshold_y_2);
                            _printf("globle_motion_filter_info.static_alpha_uv_max      %d\n",globle_motion_filter_info.static_alpha_uv_max);
                            _printf("globle_motion_filter_info.static_alpha_uv_min      %d\n",globle_motion_filter_info.static_alpha_uv_min);
                            _printf("globle_motion_filter_info.static_threshold_uv_1    %d\n",globle_motion_filter_info.static_threshold_uv_1);
                            _printf("globle_motion_filter_info.static_threshold_uv_2    %d\n",globle_motion_filter_info.static_threshold_uv_2);
                            _printf("globle_motion_filter_info.move_alpha_y_max         %d\n",globle_motion_filter_info.move_alpha_y_max);
                            _printf("globle_motion_filter_info.move_alpha_y_min         %d\n",globle_motion_filter_info.move_alpha_y_min);
                            _printf("globle_motion_filter_info.move_threshold_y_1       %d\n",globle_motion_filter_info.move_threshold_y_1);
                            _printf("globle_motion_filter_info.move_threshold_y_2       %d\n",globle_motion_filter_info.move_threshold_y_2);
                            _printf("globle_motion_filter_info.move_alpha_uv_max        %d\n",globle_motion_filter_info.move_alpha_uv_max);
                            _printf("globle_motion_filter_info.move_alpha_uv_min        %d\n",globle_motion_filter_info.move_alpha_uv_min);
                            _printf("globle_motion_filter_info.move_threshold_uv_1      %d\n",globle_motion_filter_info.move_threshold_uv_1);
                            _printf("globle_motion_filter_info.move_threshold_uv_2      %d\n",globle_motion_filter_info.move_threshold_uv_2);

#endif
                            char sendGMF3Ddenoise[100];
                            for(i=0;i<75;i++)
                            {
                                int k;
                                k=Int32ToCharArray(sendBuffer[i],sendGMF3Ddenoise);
                                send(new_fd,sendGMF3Ddenoise,k,0);
                                send(new_fd," ",1,0);
                            }
                            send(new_fd,"!",1,0);
                        }
    					break;
                        case LSHdenoise:
    					{
    						_printf("LSHdenoise load\n");
    						gk_isp_get_sharp_max_change(handle, 0, &max_change_tuning);//MAX
    						gk_isp_get_sharpen_template(handle, 0, &fir_tuning);       //FIR
						    gk_isp_get_spatial_filter(handle, 0, &spatial_filter_tuning); // mode == 0 for video mode
    						// MV Enable missed

    						sendBuffer[0]  = 0; 	 // MV Enable
    						sendBuffer[1]  = fir_tuning.fir1_s_enable;
    						sendBuffer[2]  = fir_tuning.fir1_w_enable;
    						sendBuffer[3]  = fir_tuning.fir3_enable;
    						sendBuffer[4]  = fir_tuning.edge_disp_enable;
    						sendBuffer[5]  = fir_tuning.edge_dir_sel;
    						sendBuffer[6]  = fir_tuning.edge_dir_shift;
    						sendBuffer[7]  = fir_tuning.edge_dir_info;
    						sendBuffer[8]  = max_change_tuning.max_change_sel;
    						sendBuffer[9]  = fir_tuning.fir_th1;
    						sendBuffer[10] = fir_tuning.fir_th2;
    						sendBuffer[11] = fir_tuning.fir_th3;
    						sendBuffer[12] = max_change_tuning.max_change_down_mv;
    						sendBuffer[13] = max_change_tuning.max_change_up_mv;
    						sendBuffer[14] = max_change_tuning.max_change_down_fir1s;
    						sendBuffer[15] = max_change_tuning.max_change_up_fir1s;
    						sendBuffer[16] = max_change_tuning.max_change_down_fir1w;
    						sendBuffer[17] = max_change_tuning.max_change_up_fir1w;
    						sendBuffer[18] = fir_tuning.fir1_w_str;
    						sendBuffer[19] = fir_tuning.fir1_s_str;
    						sendBuffer[20] = fir_tuning.fir3_str_iso;
    						sendBuffer[21] = fir_tuning.fir1_mv_str;
    						sendBuffer[22] = fir_tuning.fir2_mv_str;
                            sendBuffer[23] = fir_tuning.fir_strength;
                            sendBuffer[24] = max_change_tuning.max_change_down;
                            sendBuffer[25] = max_change_tuning.max_change_up;
						    sendBuffer[26] = spatial_filter_tuning.directional_strength;
							sendBuffer[27] = spatial_filter_tuning.isotropic_strength;
							sendBuffer[28] = spatial_filter_tuning.edge_threshold;

#if 1
                            _printf("MV Enable: 0 \n");     // MV Enable
                            _printf("fir1_s_enable             %d\n",fir_tuning.fir1_s_enable);
                            _printf("fir1_w_enable             %d\n",fir_tuning.fir1_w_enable);
                            _printf("fir3_enable               %d\n",fir_tuning.fir3_enable);
                            _printf("edge_disp_enable          %d\n",fir_tuning.edge_disp_enable);
                            _printf("edge_dir_sel              %d\n",fir_tuning.edge_dir_sel);
                            _printf("edge_dir_shift            %d\n",fir_tuning.edge_dir_shift);
                            _printf("edge_dir_info             %d\n",fir_tuning.edge_dir_info);
                            _printf("max_change_sel            %d\n",max_change_tuning.max_change_sel);
                            _printf("fir_th1                   %d\n",fir_tuning.fir_th1);
                            _printf("fir_th2                   %d\n",fir_tuning.fir_th2);
                            _printf("fir_th3                   %d\n",fir_tuning.fir_th3);
                            _printf("max_change_down_mv        %d\n",max_change_tuning.max_change_down_mv);
                            _printf("max_change_up_mv          %d\n",max_change_tuning.max_change_up_mv);
                            _printf("max_change_down_fir1s     %d\n",max_change_tuning.max_change_down_fir1s);
                            _printf("max_change_up_fir1s       %d\n",max_change_tuning.max_change_up_fir1s);
                            _printf("max_change_down_fir1w     %d\n",max_change_tuning.max_change_down_fir1w);
                            _printf("max_change_up_fir1w       %d\n",max_change_tuning.max_change_up_fir1w);
                            _printf("max_change_down           %d\n",max_change_tuning.max_change_down);
						    _printf("max_change_up             %d\n",max_change_tuning.max_change_up);
                            _printf("fir1_w_str                %d\n",fir_tuning.fir1_w_str);
                            _printf("fir1_s_str                %d\n",fir_tuning.fir1_s_str);
                            _printf("fir3_str_iso              %d\n",fir_tuning.fir3_str_iso);
                            _printf("fir1_mv_str               %d\n",fir_tuning.fir1_mv_str);
                            _printf("fir2_mv_str               %d\n",fir_tuning.fir2_mv_str);

							_printf("directional_strength      %d\n",spatial_filter_tuning.directional_strength);
							_printf("isotropic_strength        %d\n",spatial_filter_tuning.isotropic_strength);
							_printf("edge_threshold 	       %d\n",spatial_filter_tuning.edge_threshold);

#endif
    						char sendLSHdenoise[100];
							for(i=0;i<29;i++)
    						{
    							int k;
    							k=Int32ToCharArray(sendBuffer[i],sendLSHdenoise);
    							send(new_fd,sendLSHdenoise,k,0);
    							send(new_fd," ",1,0);
    						}
    						send(new_fd,"!",1,0);

    					}
        				break;

        				case LCEdenoise:
    					{
                            _printf("LCEdenoise load\n");
                            gk_isp_get_local_contrast_enhancement(handle,
                                &local_contrast_enhancement_info);

                            sendBuffer[0]  = local_contrast_enhancement_info.lce_enable;
                            sendBuffer[1]  = local_contrast_enhancement_info.pre_lut_en;
                            sendBuffer[2]  = local_contrast_enhancement_info.pm_en;
                            sendBuffer[3]  = local_contrast_enhancement_info.mv_scan_en;
                            sendBuffer[4]  = local_contrast_enhancement_info.disp_mv_en;
                            sendBuffer[5]  = local_contrast_enhancement_info.pre_hist_scan_en;
                            sendBuffer[6]  = local_contrast_enhancement_info.pre_hist_scan_frame;
                            sendBuffer[7]  = local_contrast_enhancement_info.pre_hist_scan_id;
                            sendBuffer[8]  = local_contrast_enhancement_info.post_hist_scan_en;
                            sendBuffer[9]  = local_contrast_enhancement_info.post_hist_scan_frame;
                            sendBuffer[10] = local_contrast_enhancement_info.post_hist_scan_id;
                            sendBuffer[11] = local_contrast_enhancement_info.hist_scan_sft;
                            sendBuffer[12] = local_contrast_enhancement_info.res_width;
                            sendBuffer[13] = local_contrast_enhancement_info.res_height;
                            sendBuffer[14] = local_contrast_enhancement_info.lce_range;
                            sendBuffer[15] = local_contrast_enhancement_info.lce_guard;
                            sendBuffer[16] = local_contrast_enhancement_info.lce_range_th1;
                            sendBuffer[17] = local_contrast_enhancement_info.lce_range_th2;
                            sendBuffer[18] = local_contrast_enhancement_info.lce_min_th;
                            sendBuffer[19] = local_contrast_enhancement_info.lce_max_th;
                            sendBuffer[20] = local_contrast_enhancement_info.lce_minb_range;
                            sendBuffer[21] = local_contrast_enhancement_info.lce_maxb_range;
                            sendBuffer[22] = local_contrast_enhancement_info.lce_contrast;
                            sendBuffer[23] = local_contrast_enhancement_info.lce_delta_th;
                            sendBuffer[24] = local_contrast_enhancement_info.lce_cutlimit;
                            sendBuffer[25] = local_contrast_enhancement_info.lce_alpha;
                            sendBuffer[26] = local_contrast_enhancement_info.lce_sud_th;
                            sendBuffer[27] = local_contrast_enhancement_info.lce_sud_sum_th;
                            sendBuffer[28] = local_contrast_enhancement_info.lce_delta_disable;
                            sendBuffer[29] = local_contrast_enhancement_info.lce_index;
                            sendBuffer[30] = local_contrast_enhancement_info.lce_search_th;

#if 0
                            sendBuffer[31] = local_contrast_enhancement_info.pm_y_gain;
                            sendBuffer[32] = local_contrast_enhancement_info.pm_y_shift;
                            sendBuffer[33] = local_contrast_enhancement_info.pm_u_gain;
                            sendBuffer[34] = local_contrast_enhancement_info.pm_u_shift;
                            sendBuffer[35] = local_contrast_enhancement_info.pm_v_gain;
                            sendBuffer[36] = local_contrast_enhancement_info.pm_v_shift;
                            sendBuffer[37] = local_contrast_enhancement_info.pm_y_offset;
                            sendBuffer[38] = local_contrast_enhancement_info.pm_u_offset;
                            sendBuffer[39] = local_contrast_enhancement_info.pm_v_offset;
                            sendBuffer[40] = local_contrast_enhancement_info.cnfg_mv_statis_th;


#endif


#if 1
                            _printf("lce_enable              :%d\n",local_contrast_enhancement_info.lce_enable);
                            _printf("pre_lut_en              :%d\n",local_contrast_enhancement_info.pre_lut_en);
                            _printf("pm_en                   :%d\n",local_contrast_enhancement_info.pm_en);
                            _printf("mv_scan_en              :%d\n",local_contrast_enhancement_info.mv_scan_en);
                            _printf("disp_mv_en              :%d\n",local_contrast_enhancement_info.disp_mv_en);
                            _printf("pre_hist_scan_en        :%d\n",local_contrast_enhancement_info.pre_hist_scan_en);
                            _printf("pre_hist_scan_frame     :%d\n",local_contrast_enhancement_info.pre_hist_scan_frame);
                            _printf("pre_hist_scan_id        :%d\n",local_contrast_enhancement_info.pre_hist_scan_id);
                            _printf("post_hist_scan_en       :%d\n",local_contrast_enhancement_info.post_hist_scan_en);
                            _printf("post_hist_scan_frame    :%d\n",local_contrast_enhancement_info.post_hist_scan_frame);
                            _printf("post_hist_scan_id       :%d\n",local_contrast_enhancement_info.post_hist_scan_id);
                            _printf("hist_scan_sft           :%d\n",local_contrast_enhancement_info.hist_scan_sft);
                            _printf("res_width               :%d\n",local_contrast_enhancement_info.res_width);
                            _printf("res_height              :%d\n",local_contrast_enhancement_info.res_height);
                            _printf("lce_range               :%d\n",local_contrast_enhancement_info.lce_range);
                            _printf("lce_guard               :%d\n",local_contrast_enhancement_info.lce_guard);
                            _printf("lce_range_th1           :%d\n",local_contrast_enhancement_info.lce_range_th1);
                            _printf("lce_range_th2           :%d\n",local_contrast_enhancement_info.lce_range_th2);
                            _printf("lce_min_th              :%d\n",local_contrast_enhancement_info.lce_min_th);
                            _printf("lce_max_th              :%d\n",local_contrast_enhancement_info.lce_max_th);
                            _printf("lce_minb_range          :%d\n",local_contrast_enhancement_info.lce_minb_range);
                            _printf("lce_maxb_range          :%d\n",local_contrast_enhancement_info.lce_maxb_range);
                            _printf("lce_contrast            :%d\n",local_contrast_enhancement_info.lce_contrast);
                            _printf("lce_delta_th            :%d\n",local_contrast_enhancement_info.lce_delta_th);
                            _printf("lce_cutlimit            :%d\n",local_contrast_enhancement_info.lce_cutlimit);
                            _printf("lce_alpha               :%d\n",local_contrast_enhancement_info.lce_alpha);
                            _printf("lce_sud_th              :%d\n",local_contrast_enhancement_info.lce_sud_th);
                            _printf("lce_sud_sum_th          :%d\n",local_contrast_enhancement_info.lce_sud_sum_th);
                            _printf("lce_delta_disable       :%d\n",local_contrast_enhancement_info.lce_delta_disable);
                            _printf("lce_index               :%d\n",local_contrast_enhancement_info.lce_index);
                            _printf("lce_search_th           :%d\n",local_contrast_enhancement_info.lce_search_th);

#endif
    						char sendLCEdenoise[100];
                            for(i=0;i<31;i++)
                            {
                                int k;
                                k=Int32ToCharArray(sendBuffer[i],sendLCEdenoise);
                                send(new_fd,sendLCEdenoise,k,0);
                                send(new_fd," ",1,0);
                            }
                            send(new_fd,"!",1,0);


    					}
        				break;
                        case HELPdenoise:
						{
                            _printf("HELPdenoise load\n");

                            //LCE
                            gk_isp_get_local_contrast_enhancement(handle,
                                &local_contrast_enhancement_info);

                            //LSH
                            gk_isp_get_sharp_max_change(handle, 0, &max_change_tuning);//MAX
                            gk_isp_get_sharpen_template(handle, 0, &fir_tuning);       //FIR

                            //LCE
                            sendBuffer[0] = local_contrast_enhancement_info.disp_mv_en;

                            //LSH 1 - 8
                            sendBuffer[1] = fir_tuning.fir1_s_enable;
                            sendBuffer[2] = fir_tuning.fir1_w_enable;
                            sendBuffer[3] = fir_tuning.fir3_enable;
                            sendBuffer[4] = fir_tuning.edge_disp_enable;
                            sendBuffer[5] = fir_tuning.edge_dir_sel;
                            sendBuffer[6] = fir_tuning.edge_dir_shift;
                            sendBuffer[7] = fir_tuning.edge_dir_info;
                            sendBuffer[8] = max_change_tuning.max_change_sel;

                            //LCE 9 -15
                            sendBuffer[9]  = local_contrast_enhancement_info.pre_hist_scan_en;
                            sendBuffer[10] = local_contrast_enhancement_info.pre_hist_scan_frame;
                            sendBuffer[11] = local_contrast_enhancement_info.pre_hist_scan_id;
                            sendBuffer[12] = local_contrast_enhancement_info.post_hist_scan_en;
                            sendBuffer[13] = local_contrast_enhancement_info.post_hist_scan_frame;
                            sendBuffer[14] = local_contrast_enhancement_info.post_hist_scan_id;
                            sendBuffer[15] = local_contrast_enhancement_info.hist_scan_sft;

                            //VGF NULL

#if 1
                            //LCE
                            _printf("disp_mv_en %d\n",local_contrast_enhancement_info.disp_mv_en);

                            //LSH 1 - 8
                            _printf("fir1_s_enable    %d\n",fir_tuning.fir1_s_enable);
                            _printf("fir1_w_enable    %d\n",fir_tuning.fir1_w_enable);
                            _printf("fir3_enable      %d\n",fir_tuning.fir3_enable);
                            _printf("edge_disp_enable %d\n",fir_tuning.edge_disp_enable);
                            _printf("edge_dir_sel     %d\n",fir_tuning.edge_dir_sel);
                            _printf("edge_dir_shift   %d\n",fir_tuning.edge_dir_shift);
                            _printf("edge_dir_info    %d\n",fir_tuning.edge_dir_info);
                            _printf("max_change_sel   %d\n",max_change_tuning.max_change_sel);

                            //LCE 9 -15
                            _printf("pre_hist_scan_en     %d\n",local_contrast_enhancement_info.pre_hist_scan_en);
                            _printf("pre_hist_scan_frame  %d\n",local_contrast_enhancement_info.pre_hist_scan_frame);
                            _printf("pre_hist_scan_id     %d\n",local_contrast_enhancement_info.pre_hist_scan_id);
                            _printf("post_hist_scan_en    %d\n",local_contrast_enhancement_info.post_hist_scan_en);
                            _printf("post_hist_scan_frame %d\n",local_contrast_enhancement_info.post_hist_scan_frame);
                            _printf("post_hist_scan_id    %d\n",local_contrast_enhancement_info.post_hist_scan_id);
                            _printf("hist_scan_sft        %d\n",local_contrast_enhancement_info.hist_scan_sft);

#endif
                            char sendHELPdenoise[100];
                            for(i=0;i<16;i++)
                            {
                                int k;
                                k=Int32ToCharArray(sendBuffer[i],sendHELPdenoise);
                                send(new_fd,sendHELPdenoise,k,0);
                                send(new_fd," ",1,0);
                            }
                            send(new_fd,"!",1,0);

                        }
    					break;
        			}
                }
                break;

                case LCEHist://Load
    			{
    				_printf("case HIST Load!\n");
                    recv(new_fd, recvBuffer, 2000, 0);
                    TCP_get_param(recvBuffer, dataArray);
                    _printf("histogram ID : %d\n\n",dataArray[0]);
                    //histid = dataArray[0];
    				PreOrPost = dataArray[0];

                    // dataArray[1]
                    switch (data.layer3)
                    {
                        case histtimerconfig:
                        {
                            _printf("Can not histtimerconfig in Load Area");
                        }
                        break;
                        case histogramid:
                        {
                            _printf("Read Hist data\n");
                            //_printf("Hist Number %d\n",histid);
                            _printf("PreOrPost %d\n",PreOrPost);

                            //_printf("gk_isp_get_local_contrast_enhancement [start]\n");
                            //gk_isp_get_local_contrast_enhancement(fdmedia,
                            //        &local_contrast_enhancement_info);
                            //_printf("gk_isp_get_local_contrast_enhancement [end]\n");


#if 0
                            ISP_LocalContrastEnhancementT localContrastEnhance;

                            if (_ioctl(fdmedia, GK_MEDIA_IOC_IMG_GET_LOCAL_CONTRAST_ENHANCEMENT, &localContrastEnhance) < 0) {
                                perror("GK_MEDIA_IOC_IMG_GET_LOCAL_CONTRAST_ENHANCEMENT");
                                _printf("error: get local contrast enhancement failed\n");
                                return -1;
                            }
#endif


                            gk_enable_ae(handle, 0);
                            gk_enable_awb(handle, 0);
                            gk_enable_af(handle, 0);
                            gk_enable_auto_param_ctl(handle, 0);

#if 0
                            _printf("\n");
                            _printf("pre_hist_scan_en     %d\n",localContrastEnhance.pre_hist_scan_en);
                            _printf("post_hist_scan_en    %d\n",localContrastEnhance.post_hist_scan_en);
                            _printf("\n");

                            _printf("pre_hist_scan_frame  %d\n",localContrastEnhance.pre_hist_scan_frame);
                            _printf("pre_hist_scan_id	  %d\n",localContrastEnhance.pre_hist_scan_id);
                            _printf("post_hist_scan_frame %d\n",localContrastEnhance.post_hist_scan_frame);
                            _printf("post_hist_scan_id	  %d\n",localContrastEnhance.post_hist_scan_id);
                            _printf("\n");
#endif


#if 0
                            _printf("\n");
                            _printf("pre_hist_scan_en     %d\n",local_contrast_enhancement_info.pre_hist_scan_en);
                            _printf("post_hist_scan_en    %d\n",local_contrast_enhancement_info.post_hist_scan_en);
                            _printf("\n");

                            _printf("pre_hist_scan_frame  %d\n",local_contrast_enhancement_info.pre_hist_scan_frame);
                            _printf("pre_hist_scan_id     %d\n",local_contrast_enhancement_info.pre_hist_scan_id);
                            _printf("post_hist_scan_frame %d\n",local_contrast_enhancement_info.post_hist_scan_frame);
                            _printf("post_hist_scan_id    %d\n",local_contrast_enhancement_info.post_hist_scan_id);
                            _printf("\n");
#endif

                            get_local_ontrast_enhancement_params(&local_ontrast_enhancement_regs_data);
                            usleep(histogram_timer);
                            _printf("get_local_contrast_enhancement_params [start]\n");
                            //get_local_ontrast_enhancement_params(&local_ontrast_enhancement_regs_data);

#if 1
                            _printf("\n");
                            _printf("APB Config\n");
                            _printf("lce_enable           : %d\n",local_ontrast_enhancement_regs_data.lce_enable);
                            _printf("pre_hist_scan_en     : %d\n",local_ontrast_enhancement_regs_data.pre_hist_scan_en);
                            _printf("pre_hist_scan_frame  : %d\n",local_ontrast_enhancement_regs_data.pre_hist_scan_frame);
                            _printf("pre_hist_scan_id     : %d\n",local_ontrast_enhancement_regs_data.pre_hist_scan_id);
                            _printf("post_hist_scan_en    : %d\n",local_ontrast_enhancement_regs_data.post_hist_scan_en);
                            _printf("post_hist_scan_frame : %d\n",local_ontrast_enhancement_regs_data.post_hist_scan_frame);
                            _printf("post_hist_scan_id    : %d\n",local_ontrast_enhancement_regs_data.post_hist_scan_id);
                            _printf("hist_scan_sft        : %d\n",local_ontrast_enhancement_regs_data.hist_scan_sft);
                            _printf("sensor_id            : %d\n",local_ontrast_enhancement_regs_data.sensor_id);
                            _printf("stream_split_num     : %d\n",local_ontrast_enhancement_regs_data.stream_split_num);
                            _printf("\n");
#endif

                            char sendHist[100];
                            //srand((int)time(0));
                            for (i=0; i<256;i++)
                            {
                                if(PreOrPost == 0) //Pre
                                {
                                    sendBuffer[i] = local_ontrast_enhancement_regs_data.ram_lut[i];
                                }
                                else //Post
                                {
                                    sendBuffer[i] = local_ontrast_enhancement_regs_data.post_hist_lut[i];
                                }
                                _printf("sendBuffer[%d]:0x%x\n", i, sendBuffer[i]);

                                int k;
                                k = Int32ToCharArray(sendBuffer[i],
                                    sendHist);
                                send(new_fd, sendHist, k, 0);
                                send(new_fd, " ", 1, 0);
                            }

                            send(new_fd, "!", 1, 0);
                        }
                        break;
                    }
                }
                break;

                }
            }
            break;

            case Caribration:
                //_printf("break layer1\n");
                break;

            case AutoTest:
                //_printf("break layer1\n");
                break;

            }

        }
        else
        {
            printf("%d \n",id_Require);
            _printf("Tunning:Get error id number....\n");

        }
    }
    if (sockfd != -1)
        close(sockfd);

    if (fdmedia) {
        close(fdmedia);
        fdmedia = 0;
    }

    if (reg)
    {
        free(reg);
        reg = NULL;
    }

    if (matrix)
    {
        free(matrix);
        matrix = NULL;
    }

    if (matrix1)
    {
        free(matrix1);
        matrix1 = NULL;
    }

    if (matrix2)
    {
        free(matrix2);
        matrix2 = NULL;
    }

    if (matrix3)
    {
        free(matrix3);
        matrix3 = NULL;
    }

    if (matrix4)
    {
        free(matrix4);
        matrix4 = NULL;
    }

    if (bin_adj_param)
    {
        free(bin_adj_param);
        bin_adj_param = NULL;
    }

    return NULL;
}
