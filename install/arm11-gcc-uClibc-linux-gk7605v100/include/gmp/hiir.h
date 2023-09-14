/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef HI_IR_H
#define HI_IR_H

#include "ot_ir.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* define device name */
#define HIIR_DEVICE_NAME OT_IR_DEVICE_NAME

#define HIIR_DEFAULT_FREQ OT_IR_DEFAULT_FREQ

/* DEFINE KEY STATE */
#define HIIR_KEY_DOWN   OT_IR_KEY_DOWN
#define HIIR_KEY_UP     OT_IR_KEY_UP

/* device parameter */
typedef ot_ir_dev_param hiir_dev_param;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* End of HI_IR_H */
