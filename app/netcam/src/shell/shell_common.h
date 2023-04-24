#ifndef _SHELL_COMMON_H_
#define _SHELL_COMMON_H_
#ifdef __cplusplus
extern "C"
{
#endif

int ircut_register_testcase(void);
int wifi_register_testcase(void);
int isp_register_testcase(void);
int ptz_register_testcase(void);
int upgrade_register_testcase(void);
int eraseapp_register_testcase(void);
int ftp_register_testcase(void);
int bpt_register_testcase(void);


#ifdef __cplusplus
}
#endif
#endif

