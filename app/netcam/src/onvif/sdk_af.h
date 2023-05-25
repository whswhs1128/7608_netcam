#ifndef __SDK_AF_H__
#define __SDK_AF_H__
#include "sdk_lens.h"

#ifdef __cplusplus
extern "C" {
#endif

int sdk_af_lens_init(af_lens_cb cb);
int sdk_af_lens_exit();
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* __SDK_AF_H__ */
