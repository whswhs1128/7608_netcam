#ifndef FH_AUIDO_MPIPARA_H_
#define FH_AUIDO_MPIPARA_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

typedef enum{
	AC_SR_8K   = 8000,
	AC_SR_16K  = 16000,
	AC_SR_32K  = 32000,
	AC_SR_441K = 44100,
	AC_SR_48K  = 48000,
} FH_AC_SAMPLE_RATE_E;

typedef enum{
	AC_BW_8  = 8,
	AC_BW_16 = 16,
	AC_BW_24 = 24,
} FH_AC_BIT_WIDTH_E;


typedef struct{
	FH_UINT32 len;
	FH_UINT8 *data;
}FH_AC_FRAME_S;

typedef enum{
	FH_AC_MIC_IN = 0,
	FH_AC_LINE_IN = 1,
	FH_AC_SPK_OUT = 2,
	FH_AC_LINE_OUT = 3
}FH_AC_IO_TYPE_E;


typedef struct {
	FH_AC_IO_TYPE_E io_type;
	FH_AC_SAMPLE_RATE_E sample_rate;
	FH_AC_BIT_WIDTH_E bit_width;
	FH_UINT32 channels;         //通道个数
	FH_UINT32 period_size;      //一帧数据中的采样点个数
	FH_UINT32 volume;
} FH_AC_CONFIG;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* FH_AUIDO_MPIPARA_H_ */
