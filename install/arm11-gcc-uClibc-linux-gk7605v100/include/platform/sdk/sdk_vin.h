#ifndef SDK_VIN_H_
#define SDK_VIN_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sdk_def.h"


typedef struct BIT_MAP_FILE_HEADER
{
	char type[2]; // "BM" (0x4d42)
    uint32_t file_size;
    uint32_t reserved_zero;
    uint32_t off_bits; // data area offset to the file set (unit. byte)
	uint32_t info_size;
	uint32_t width;
	uint32_t height;
	uint16_t planes; // 0 - 1
	uint16_t bit_count; // 0 - 1
	uint32_t compression; // 0 - 1
	uint32_t size_image; // 0 - 1
	uint32_t xpels_per_meter;
	uint32_t ypels_per_meter;
	uint32_t clr_used;
	uint32_t clr_important;
}__attribute__((packed)) BIT_MAP_FILE_HEADER_t; //


/*cover parameter */
typedef struct  {
	bool enable;
	float x, y, width, height;
	uint32_t color; /* RGB888 format*/
}ST_GK_VIN_COVER_ATTR, *PS_GK_VIN_COVER_ATTR;

/* motion detect check region */
typedef struct SDK_VIN_CAPTURE_ATTR {
	uint32_t width, height;
	int fps;
}ST_GK_VIN_CAPTURE_ATTR, *PS_GK_VIN_CAPTURE_ATTR;

/* motion detect callback */
typedef void (*GK_MD_DO_TRAP)(int vin);

/*!
*******************************************************************************
** \brief get  motion detect region of hardware
**
** \param[in]  vin              video input id
**
** \param[out]  capture_attr    hardware support motion detect region
**
** \return
** - #SDK_OK   success
**   other     failed
**
**
*******************************************************************************
*/
int sdk_vin_get_capture_attr(int vin, PS_GK_VIN_CAPTURE_ATTR capture_attr);

/*!
*******************************************************************************
** \brief create  motion detect region of hardware
**
** \param[in]  vin           video input id
** \param[in]  ref_hblock    hor block number
** \param[in]  ref_vblock    vertical block number
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_create_md(int vin, size_t ref_hblock, size_t ref_vblock);

/*!
*******************************************************************************
** \brief destroy  motion detect region of hardware
**
** \param[in]  vin           video input id
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_release_md(int vin);

/*!
*******************************************************************************
** \brief dump  motion detect region of hardware
**
** \param[in]  vin           video input id
**
*******************************************************************************
*/
void sdk_vin_dump_md(int vin);

/*!
*******************************************************************************
** \brief start  motion detect
**
** \param[in]  vin           video input id
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_start_md(int vin);

/*!
*******************************************************************************
** \brief stop  motion detect
**
** \param[in]  vin           video input id
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_stop_md(int vin);

/*!
*******************************************************************************
** \brief motion detect threshold
**
** \param[in]  vin           video input id
** \param[in]  threshold     0.1-1,percent value, 1 is hightest accuracy, 0.1 is lowest
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_set_md_bitmap_threshold(int vin, float threshold);

/*!
*******************************************************************************
** \brief set bitmap mask
**
** \param[in]  vin           video input id
** \param[in]  mask_bitflag  array,the size of array is (ref_hblock*ref_vblock)/8
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_set_md_bitmap_mask(int vin, uint8_t *mask_bitflag);

/*!
*******************************************************************************
** \brief set callback when modtion has detected
**
** \param[in]  vin          video input id
** \param[in]  do_trap      callback
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_set_md_trap(int vin, GK_MD_DO_TRAP do_trap);

/*!
*******************************************************************************
** \brief set hardware check freqency for motion
**
** \param[in]  vin          video input id
** \param[in]  ref_freq     ref_freq
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_set_md_ref_freq(int vin, int ref_freq);

/*!
*******************************************************************************
** \brief set vi privacy mask
**
** \param[in]  vin          video input id
** \param[in]  cover        privacy mask attribute
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_set_cover(int vin, int id, PS_GK_VIN_COVER_ATTR cover);

/*!
*******************************************************************************
** \brief get vi privacy mask
**
** \param[in]  vin          video input id
** \param[out]  cover        privacy mask attribute
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_get_cover(int vin, int id, PS_GK_VIN_COVER_ATTR cover);

/*!
*******************************************************************************
** \brief start vin module
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_start(void);

/*!
*******************************************************************************
** \brief stop vin module
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_stop(void);


/*!
*******************************************************************************
** \brief vin module init
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_init();

/*!
*******************************************************************************
** \brief vin module exit
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_destroy();

int sdk_vin_capture_data(int vin, char** data, int *width, int *height);

#ifdef __cplusplus
};
#endif
#endif //SDK_VIN_H_

