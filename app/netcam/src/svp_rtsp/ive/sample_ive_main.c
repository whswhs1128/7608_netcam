/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */
#include "sample_ive_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_common_svp.h"

#define OT_SAMPLE_IVE_ARG_NUM_THREE    3
#define OT_SAMPLE_IVE_ARG_NUM_TWO      2
#define OT_SAMPLE_IVE_ARG_IDX_TWO      2
#define OT_SAMPLE_IVE_CMP_STR_NUM      2

/*
 * function : to process abnormal case
 */
#ifndef __LITEOS__
static td_void sample_ive_handle_sig(td_s32 singal)
{
    if (singal == SIGINT || singal == SIGTERM) {
        
                sample_ive_md_handle_sig();
                
}
}
#endif

/*
 * function : ive sample
 */
#ifdef __LITEOS__
int app_main(int argc, char *argv[])
#else
int svp_main(int argc, char *argv[])
#endif
{
    td_s32 ret;
    td_s32 idx_len;
#ifndef __LITEOS__
    struct sigaction sa;
#endif

#ifndef __LITEOS__
    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sample_ive_handle_sig;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
#endif
    sample_ive_md();

    return 0;
}
