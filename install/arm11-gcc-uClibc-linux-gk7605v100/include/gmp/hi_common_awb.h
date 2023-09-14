/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __HI_COMMON_AWB_H__
#define __HI_COMMON_AWB_H__

#include "hi_type.h"
#include "hi_debug.h"
#include "hi_common_isp.h"
#include "ot_common_awb.h"


#ifdef __cplusplus
extern "C" {
#endif

#define HI_AWB_LIB_NAME OT_AWB_LIB_NAME
typedef ot_isp_awb_dbg_attr hi_isp_awb_dbg_attr;
typedef ot_isp_awb_zone_dbg hi_isp_awb_zone_dbg;
typedef ot_isp_awb_dbg_status hi_isp_awb_dbg_status;
typedef ot_isp_awb_ccm_tab hi_isp_awb_ccm_tab;
typedef ot_isp_awb_ccm hi_isp_awb_ccm;
typedef ot_isp_awb_agc_table hi_isp_awb_agc_table;
typedef ot_isp_awb_sensor_default hi_isp_awb_sensor_default;
typedef ot_isp_awb_sensor_exp_func hi_isp_awb_sensor_exp_func;
typedef ot_isp_awb_sensor_register hi_isp_awb_sensor_register;

#ifdef __cplusplus
}
#endif
#endif /* __HI_COMMON_AWB_H__ */
