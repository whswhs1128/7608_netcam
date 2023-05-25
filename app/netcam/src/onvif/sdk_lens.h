#ifndef __SDK_LENS_H__
#define __SDK_LENS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*af_lens_cb)(char* buf, int len);

int sdk_lens_send(char *buf, int len);
int sdk_lens_exit();
int sdk_lens_init(af_lens_cb cb);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* __SDK_LENS_H__ */
