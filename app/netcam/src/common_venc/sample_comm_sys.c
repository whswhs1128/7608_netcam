/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include "sample_comm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>

#define sample_mem_check_open_return() \
    do { \
        if (g_sample_mem_dev <= 0) { \
            g_sample_mem_dev = open("/dev/mem", O_RDWR | O_SYNC); \
            if (g_sample_mem_dev < 0) { \
                perror("open dev/mem error"); \
                return TD_NULL; \
            } \
        } \
    } while (0)

static td_s32 g_sample_mem_dev = -1;

static ot_mpp_chn g_sample_mpp_chn[] = {
    {OT_ID_VI, OT_VI_MAX_DEV_NUM, OT_VI_MAX_CHN_NUM},
    {OT_ID_VPSS, OT_VPSS_MAX_GRP_NUM, 1},
    {OT_ID_VENC, 1, OT_VENC_MAX_CHN_NUM},
    {OT_ID_VO, OT_VO_MAX_LAYER_NUM, OT_VO_MAX_CHN_NUM},
    {OT_ID_VDEC, 1, OT_VDEC_MAX_CHN_NUM}
};

/* The order of g_sample_pic_size's element must be consistent with the enum value defined in "ot_pic_size". */
static ot_size g_sample_pic_size[PIC_BUTT] = {
    { 352,  288  },  /* PIC_CIF */
    { 640,  360  },  /* PIC_360P */
    { 720,  576  },  /* PIC_D1_PAL */
    { 720,  480  },  /* PIC_D1_NTSC */
    { 960,  576  },  /* PIC_960H */
    { 1280, 720  },  /* PIC_720P */
    { 1920, 1080 },  /* PIC_1080P */
    { 720,  480  },  /* PIC_480P */
    { 720,  576  },  /* PIC_576P */
    { 800,  600  },  /* PIC_800X600 */
    { 1024, 768  },  /* PIC_1024X768 */
    { 1280, 1024 },  /* PIC_1280X1024 */
    { 1366, 768  },  /* PIC_1366X768 */
    { 1440, 900  },  /* PIC_1440X900 */
    { 1280, 800  },  /* PIC_1280X800 */
    { 1600, 1200 },  /* PIC_1600X1200 */
    { 1680, 1050 },  /* PIC_1680X1050 */
    { 1920, 1200 },  /* PIC_1920X1200 */
    { 640,  480  },  /* PIC_640X480 */
    { 1920, 2160 },  /* PIC_1920X2160 */
    { 2560, 1440 },  /* PIC_2560X1440 */
    { 2560, 1600 },  /* PIC_2560X1600 */
    { 2592, 1520 },  /* PIC_2592X1520 */
    { 2592, 1944 },  /* PIC_2592X1944 */
    { 3840, 2160 },  /* PIC_3840X2160 */
    { 4096, 2160 },  /* PIC_4096X2160 */
    { 3000, 3000 },  /* PIC_3000X3000 */
    { 4000, 3000 },  /* PIC_4000X3000 */
    { 6080, 2800 },  /* PIC_6080X2800 */
    { 7680, 4320 },  /* PIC_7680X4320 */
    { 3840, 8640 }   /* PIC_3840X8640 */
};

#ifndef __LITEOS__
td_void sample_sys_signal(void (*func)(int))
{
    struct sigaction sa = { 0 };

    sa.sa_handler = func;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
}
#endif

td_void *sample_sys_io_mmap(td_u64 phy_addr, td_u32 size)
{
    td_u32 diff;
    td_u64 page_phy;
    td_u8 *page_addr = TD_NULL;
    td_ulong page_size;

    sample_mem_check_open_return();

    /* page_size will be 0 when size is 0 and diff is 0, and then mmap will be error(invalid argument) */
    if (!size) {
        printf("func: %s size can't be 0.\n", __FUNCTION__);
        return TD_NULL;
    }

    /* the mmap address should align with page */
    page_phy = phy_addr & 0xfffffffffffff000ULL;
    diff = phy_addr - page_phy;

    /* the mmap size should be multiples of 1024 */
    page_size = ((size + diff - 1) & 0xfffff000UL) + 0x1000;

    page_addr = mmap((void *)0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_sample_mem_dev, page_phy);
    if (page_addr == MAP_FAILED) {
        perror("mmap error");
        return TD_NULL;
    }
    return (td_void *)(page_addr + diff);
}

td_s32 sample_sys_munmap(td_void *vir_addr, td_u32 size)
{
    td_u64 page_addr;
    td_u32 page_size;
    td_u32 diff;

    page_addr = (((td_uintptr_t)vir_addr) & 0xfffffffffffff000ULL);
    diff = (td_uintptr_t)vir_addr - page_addr;
    page_size = ((size + diff - 1) & 0xfffff000UL) + 0x1000;

    return munmap((td_void *)(td_uintptr_t)page_addr, page_size);
}

td_s32 sample_sys_set_reg(td_u64 addr, td_u32 value)
{
    td_u32 *reg_addr = TD_NULL;
    td_u32 map_len = sizeof(value);

    reg_addr = (td_u32 *)sample_sys_io_mmap(addr, map_len);
    if (reg_addr == TD_NULL) {
        return TD_FAILURE;
    }

    *reg_addr = value;

    return sample_sys_munmap(reg_addr, map_len);
}

td_s32 sample_sys_get_reg(td_u64 addr, td_u32 *value)
{
    td_u32 *reg_addr = TD_NULL;
    td_u32 map_len;

    if (value == TD_NULL) {
        return OT_ERR_SYS_NULL_PTR;
    }

    map_len = sizeof(*value);
    reg_addr = (td_u32 *)sample_sys_io_mmap(addr, map_len);
    if (reg_addr == TD_NULL) {
        return TD_FAILURE;
    }

    *value = *reg_addr;

    return sample_sys_munmap(reg_addr, map_len);
}

/* get picture size(w*h), according pic_size */
td_s32 sample_comm_sys_get_pic_size(ot_pic_size pic_size, ot_size *size)
{
    if (size == TD_NULL) {
        sample_print("null ptr arg!\n");
        return TD_FAILURE;
    }

    if (pic_size >= PIC_BUTT) {
        sample_print("illegal pic_size!\n");
        return TD_FAILURE;
    }

    size->width = g_sample_pic_size[pic_size].width;
    size->height = g_sample_pic_size[pic_size].height;

    return TD_SUCCESS;
}

ot_pic_size sample_comm_sys_get_pic_enum(const ot_size *size)
{
    ot_pic_size i;

    for (i = PIC_CIF; i < PIC_BUTT; i++) {
        if ((g_sample_pic_size[i].width == size->width) &&
            (g_sample_pic_size[i].height == size->height)) {
            return i;
        }
    }

    return PIC_1080P;
}

static td_s32 sample_comm_sys_set_module_mem_config(td_u32 mod)
{
    td_char *mmz_name = TD_NULL;
    ot_mpp_chn mpp_chn;
    td_u32 max_dev_num = g_sample_mpp_chn[mod].dev_id;
    td_u32 max_chn_num = g_sample_mpp_chn[mod].chn_id;
    td_u32 i, j;

    mpp_chn.mod_id = g_sample_mpp_chn[mod].mod_id;

    for (i = 0; i < max_dev_num; ++i) {
        mpp_chn.dev_id = i;

        for (j = 0; j < max_chn_num; ++j) {
            mpp_chn.chn_id = j;
            if (ss_mpi_sys_set_mem_cfg(&mpp_chn, mmz_name) != TD_SUCCESS) {
                sample_print("ss_mpi_sys_set_mem_cfg ERR!\n");
                return TD_FAILURE;
            }
        }
    }

    return TD_SUCCESS;
}

/* set system memory location */
td_s32 sample_comm_sys_mem_config(td_void)
{
    td_u32 i;

    /* config memory */
    for (i = 0; i < sizeof(g_sample_mpp_chn) / sizeof(g_sample_mpp_chn[0]); ++i) {
        if (sample_comm_sys_set_module_mem_config(i) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

/* vb init & MPI system init */
td_s32 sample_comm_sys_init(const ot_vb_cfg *vb_cfg)
{
    td_s32 ret;

    ss_mpi_sys_exit();
    ss_mpi_vb_exit();

    if (vb_cfg == TD_NULL) {
        sample_print("input parameter is null, it is invalid!\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_vb_set_cfg(vb_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vb_set_conf failed!\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_vb_init();
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vb_init failed!\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_sys_init();
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_sys_init failed!\n");
        ss_mpi_vb_exit();
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

/* vb init with vb_supplement & MPI system init */
td_s32 sample_comm_sys_init_with_vb_supplement(const ot_vb_cfg *vb_conf, td_u32 supplement_config)
{
    td_s32 ret;
    ot_vb_supplement_cfg supplement_conf = {0};

    ss_mpi_sys_exit();
    ss_mpi_vb_exit();

    if (vb_conf == TD_NULL) {
        sample_print("input parameter is null, it is invalid!\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_vb_set_cfg(vb_conf);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vb_set_conf failed!\n");
        return TD_FAILURE;
    }

    supplement_conf.supplement_cfg = supplement_config;
    ret = ss_mpi_vb_set_supplement_cfg(&supplement_conf);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vb_set_supplement_conf failed!\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_vb_init();
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vb_init failed!\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_sys_init();
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_sys_init failed!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

/* vb exit & MPI system exit */
td_void sample_comm_sys_exit(td_void)
{
    ss_mpi_sys_exit();
    ss_mpi_vb_exit_mod_common_pool(OT_VB_UID_VDEC);
    ss_mpi_vb_exit();
    return;
}

td_s32 sample_comm_vi_bind_vo(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = vo_layer;
    dest_chn.chn_id = vo_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VI-VO)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vi_un_bind_vo(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = vo_layer;
    dest_chn.chn_id = vo_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VI-VO)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vi_bind_vpss(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dest_chn.mod_id = OT_ID_VPSS;
    dest_chn.dev_id = vpss_grp;
    dest_chn.chn_id = vpss_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VI-VPSS)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vi_un_bind_vpss(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dest_chn.mod_id = OT_ID_VPSS;
    dest_chn.dev_id = vpss_grp;
    dest_chn.chn_id = vpss_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VI-VPSS)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vi_bind_venc(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_venc_chn venc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dest_chn.mod_id = OT_ID_VENC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = venc_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VI-VENC)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vi_un_bind_venc(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_venc_chn venc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dest_chn.mod_id = OT_ID_VENC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = venc_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VI-VENC)");

    return TD_SUCCESS;
}

td_s32 sample_comm_avs_bind_venc(ot_avs_grp avs_grp, ot_avs_chn avs_chn, ot_venc_chn venc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_AVS;
    src_chn.dev_id = avs_grp;
    src_chn.chn_id = avs_chn;

    dest_chn.mod_id = OT_ID_VENC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = venc_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(AVS-VENC)");

    return TD_SUCCESS;
}

td_s32 sample_comm_avs_un_bind_venc(ot_avs_grp avs_grp, ot_avs_chn avs_chn, ot_venc_chn venc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_AVS;
    src_chn.dev_id = avs_grp;
    src_chn.chn_id = avs_chn;

    dest_chn.mod_id = OT_ID_VENC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = venc_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(AVS-VENC)");

    return TD_SUCCESS;
}

td_s32 sample_comm_avs_bind_vo(ot_avs_grp avs_grp, ot_avs_chn avs_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_AVS;
    src_chn.dev_id = avs_grp;
    src_chn.chn_id = avs_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = vo_layer;
    dest_chn.chn_id = vo_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(AVS-VO)");

    return TD_SUCCESS;
}

td_s32 sample_comm_avs_un_bind_vo(ot_avs_grp avs_grp, ot_avs_chn avs_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_AVS;
    src_chn.dev_id = avs_grp;
    src_chn.chn_id = avs_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = vo_layer;
    dest_chn.chn_id = vo_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(AVS-VO)");

    return TD_SUCCESS;
}


td_s32 sample_comm_vpss_bind_vo(ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VPSS;
    src_chn.dev_id = vpss_grp;
    src_chn.chn_id = vpss_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = vo_layer;
    dest_chn.chn_id = vo_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VPSS-VO)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vpss_un_bind_vo(ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VPSS;
    src_chn.dev_id = vpss_grp;
    src_chn.chn_id = vpss_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = vo_layer;
    dest_chn.chn_id = vo_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VPSS-VO)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vpss_bind_avs(ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn, ot_avs_grp avs_grp, ot_avs_pipe avs_pipe)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VPSS;
    src_chn.dev_id = vpss_grp;
    src_chn.chn_id = vpss_chn;

    dest_chn.mod_id = OT_ID_AVS;
    dest_chn.dev_id = avs_grp;
    dest_chn.chn_id = avs_pipe;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VPSS-AVS)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vpss_un_bind_avs(ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn,
                                    ot_avs_grp avs_grp, ot_avs_pipe avs_pipe)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VPSS;
    src_chn.dev_id = vpss_grp;
    src_chn.chn_id = vpss_chn;

    dest_chn.mod_id = OT_ID_AVS;
    dest_chn.dev_id = avs_grp;
    dest_chn.chn_id = avs_pipe;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VPSS-AVS)");

    return TD_SUCCESS;
}


td_s32 sample_comm_vpss_bind_venc(ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn, ot_venc_chn venc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VPSS;
    src_chn.dev_id = vpss_grp;
    src_chn.chn_id = vpss_chn;

    dest_chn.mod_id = OT_ID_VENC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = venc_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VPSS-VENC)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vpss_un_bind_venc(ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn, ot_venc_chn venc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VPSS;
    src_chn.dev_id = vpss_grp;
    src_chn.chn_id = vpss_chn;

    dest_chn.mod_id = OT_ID_VENC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = venc_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VPSS-VENC)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vdec_bind_vpss(ot_vdec_chn vdec_chn, ot_vpss_grp vpss_grp)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VDEC;
    src_chn.dev_id = 0;
    src_chn.chn_id = vdec_chn;

    dest_chn.mod_id = OT_ID_VPSS;
    dest_chn.dev_id = vpss_grp;
    dest_chn.chn_id = 0;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VDEC-VPSS)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vdec_un_bind_vpss(ot_vdec_chn vdec_chn, ot_vpss_grp vpss_grp)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VDEC;
    src_chn.dev_id = 0;
    src_chn.chn_id = vdec_chn;

    dest_chn.mod_id = OT_ID_VPSS;
    dest_chn.dev_id = vpss_grp;
    dest_chn.chn_id = 0;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VDEC-VPSS)");

    return TD_SUCCESS;
}

td_s32 sample_comm_vo_bind_vo(ot_vo_layer src_vo_layer, ot_vo_chn src_vo_chn,
                              ot_vo_layer dst_vo_layer, ot_vo_chn dst_vo_chn)
{
    ot_mpp_chn src_chn, dest_chn;

    src_chn.mod_id = OT_ID_VO;
    src_chn.dev_id = src_vo_layer;
    src_chn.chn_id = src_vo_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = dst_vo_layer;
    dest_chn.chn_id = dst_vo_chn;

    return ss_mpi_sys_bind(&src_chn, &dest_chn);
}

td_s32 sample_comm_vo_un_bind_vo(ot_vo_layer dst_vo_layer, ot_vo_chn dst_vo_chn)
{
    ot_mpp_chn dest_chn;

    dest_chn.mod_id = OT_ID_VO;
    dest_chn.dev_id = dst_vo_layer;
    dest_chn.chn_id = dst_vo_chn;

    return ss_mpi_sys_unbind(TD_NULL, &dest_chn);
}
