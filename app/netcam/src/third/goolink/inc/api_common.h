#ifndef _API_COMMON_H_
#define _API_COMMON_H_

#include <stdio.h>
#include <string.h>


#include <sys/ioctl.h>
#include <sys/mman.h>
#include "bufCtrl.h"
#include "type_def.h"

#define fh_printf    printf
#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#define SYS_NAME    "/dev/media_process"
#define VPU_NAME    "/dev/vpu"
#define PAE_NAME    "/dev/pae"
#define VOU_NAME    "/dev/vou"
#define JPEG_NAME   "/dev/jpeg"
#define BGM_NAME    "/dev/bgm"
#define FD_NAME     "/dev/fd"
#define AUDIO_NAME  "/dev/fh_audio"

typedef FH_SINT32 DEV_FD;
#define dev_exist(name)         (access(name,F_OK) == 0)
#define dev_ioctl(dev,cmd,data) ioctl(dev,cmd,data)
#define dev_open(name,p1,p2)    open(name,p1,p2)
#define dev_close(dev)          close(dev)

#define ERRNO  ({int _errno=errno; _errno?_errno:errs;})

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif

