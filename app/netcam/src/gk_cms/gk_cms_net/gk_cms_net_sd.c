#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"

#ifdef MODULE_SUPPORT_LOCAL_REC
#include "mmc_api.h"
#include "avi_rec.h"
#include "avi_utility.h"
#include "avi_search_api.h"

int Gk_CmsGetFilelist(int sock)
{
    JBNV_FIND_FILE_REQ file_req;
    int ret = GkSockRecv(sock, (char *)&file_req, sizeof(JBNV_FIND_FILE_REQ));
    if (ret != sizeof(JBNV_FIND_FILE_REQ)) {
        PRINT_ERR();
        return -1;
    }
    printf("recv_len:%d size:%d\n", ret, sizeof(JBNV_FIND_FILE_REQ));
    printf("channel:%lu type:%lu starttime: %lu-%lu-%lu %lu:%lu:%lu stoptime: %lu-%lu-%lu %lu:%lu:%lu \n", 
           file_req.nChannel, file_req.nFileType, 
           file_req.BeginTime.dwYear, file_req.BeginTime.dwMonth, file_req.BeginTime.dwDay,
           file_req.BeginTime.dwHour, file_req.BeginTime.dwMinute, file_req.BeginTime.dwSecond,
           file_req.EndTime.dwYear, file_req.EndTime.dwMonth, file_req.EndTime.dwDay,
           file_req.EndTime.dwHour, file_req.EndTime.dwMinute, file_req.EndTime.dwSecond);


    //////////////////////////////////////////////////////////
    u64t start = time_to_u64t((AVI_DMS_TIME *)&(file_req.BeginTime));
    u64t stop = time_to_u64t((AVI_DMS_TIME *)&(file_req.EndTime));
    u32t type = 0;

    // 文件类型 ：0xff － 全部，0 － 定时录像，1 - 移动侦测，2 － 报警触发，3  － 手动录像
    switch (file_req.nFileType) {
        case 255:
            type = 0;
            break;
        case 0:
            type = RECORD_TYPE_SCHED;
            break;
        case 1:
            type = RECORD_TYPE_MOTION;
            break;
        case 2:
            type = RECORD_TYPE_ALARM;
            break;
        case 3:
            type = RECORD_TYPE_MANU;
            break;
        default:
            type = 0;
            break;
    }
    FILE_LIST *list = search_file_by_time(type, 0, start, stop);
    int file_num = get_len_list(list);
    PRINT_INFO("file_num=%d\n", file_num);

    char *send_buf = malloc(sizeof(JBNV_FIND_FILE_RESP) + sizeof(JBNV_FILE_DATA_INFO) * file_num);
    if(send_buf == NULL) {
        PRINT_ERR();
        return -1;
    }
    memset(send_buf, 0, sizeof(JBNV_FIND_FILE_RESP) + sizeof(JBNV_FILE_DATA_INFO) * file_num);
    
    JBNV_FIND_FILE_RESP *file_resp = (JBNV_FIND_FILE_RESP *)send_buf;
    file_resp->dwSize = sizeof(JBNV_FIND_FILE_RESP);
    file_resp->nCount = file_num;
    file_resp->nResult = 0;

    int i = 0;
    JBNV_FILE_DATA_INFO *file1 = NULL;
    FILE_NODE node;
    for (i = 0; i < file_num; i ++) {
        file1 = (JBNV_FILE_DATA_INFO *)(send_buf + sizeof(JBNV_FIND_FILE_RESP) + i * sizeof(JBNV_FILE_DATA_INFO));

        memset(&node, 0, sizeof(FILE_NODE));
        ret = get_file_node(list, &node);
        if (ret != DMS_NET_FILE_SUCCESS) {
            PRINT_ERR("get_file_node failed, ret = %d\n", ret);
            file_resp->nResult = 1;
            break;
        }
        print_node(&node);

        strcpy(file1->sFileName, node.path);
        u64t_to_time((AVI_DMS_TIME *)&(file1->BeginTime), node.start);
        u64t_to_time((AVI_DMS_TIME *)&(file1->EndTime), node.stop);
        file1->nChannel = 0;
        file1->nFileSize = get_file_size(node.path);
    }

    search_close(list);

    Gk_CmsNotify(sock, JB_MSG_FILE_NAME_DATA, send_buf, sizeof(JBNV_FIND_FILE_RESP) + sizeof(JBNV_FILE_DATA_INFO) * file_num);

    free(send_buf);
    return 0;
}

#if 0
int Gk_CmsFindFile(JBNV_FIND_FILE_REQ *pFindCond)
{
    FILE_LIST *list = NULL;
    u32t type = (u32t)pFindCond->dwFileType;
    u32t ch_num = (u32t)pFindCond->dwChannel;
    u64t start = time_to_u64t((AVI_DMS_TIME *)&(pFindCond->stStartTime));
    u64t stop = time_to_u64t((AVI_DMS_TIME *)&(pFindCond->stStopTime));

    PRINT_INFO("call Gk_CmsFindFile\n\n");
    list = search_file_by_time(type, ch_num, start, stop);

    if (list == NULL) {
        PRINT_ERR();
        return -1;
    }

    return (int)list;
}



int Gk_CmsFindNextFile(int FindHandle, JBNV_FILE_DATA_INFO *lpFindData)
{
    PRINT_INFO("call Gk_CmsFindNextFile\n");
    FILE_LIST *list = (FILE_LIST *)FindHandle;
    FILE_NODE node;
    AVI_DMS_TIME t;
    int ret;

    memset(&node, 0, sizeof(FILE_NODE));
    ret = get_file_node(list, &node);
    if (ret != GK_NET_FILE_SUCCESS) {
        printf("ret = %d\n", ret);
        return ret;
    }

#if 1
    print_node(&node);
#endif

    strcpy(lpFindData->sFileName, node.path);
    u64t_to_time(&t, node.start);
    memcpy(&(lpFindData->BeginTime), &t, sizeof(AVI_DMS_TIME));
    u64t_to_time(&t, node.stop);
    memcpy(&(lpFindData->EndTime), &t, sizeof(AVI_DMS_TIME));
    lpFindData->nChannel = 0;
    lpFindData->nFileSize = get_file_size(node.path);
    lpFindData->nState = 0;

    return GK_NET_FILE_SUCCESS;
}


int Gk_CmsFindClose(int FindHandle)
{
    FILE_LIST *list = (FILE_LIST *)FindHandle;
    PRINT_INFO("\n\ncall Gk_CmsFindClose\n\n");
    search_close(list);

    return 0;
}

#endif


int Gk_GetNas(int sock)
{
    DMS_NET_NAS_CFG nas_cfg;

    //to do
    memset(&nas_cfg, 0, sizeof(DMS_NET_NAS_CFG));
    nas_cfg.dwSize = sizeof(DMS_NET_NAS_CFG);
    int count = sizeof(nas_cfg.stServerInfo) / sizeof(nas_cfg.stServerInfo[0]);
    printf("count=%d\n", count);
    int i = 0;
    for (i = 0; i < count; i ++) {
        nas_cfg.stServerInfo[i].byEnable = 0;
    }

    return GkCmsCmdResq(sock, &nas_cfg, sizeof(DMS_NET_NAS_CFG), DMS_NET_GET_NASCFG);
}

int Gk_SetNas(int sock)
{
    DMS_NET_NAS_CFG nas_cfg;

    int ret = GkSockRecv(sock, (char *)&nas_cfg, sizeof(DMS_NET_NAS_CFG));
    if (ret != sizeof(DMS_NET_NAS_CFG)) {
        PRINT_ERR();
        return -1;
    }
    
    //to do
    

    return 0;
}

int Gk_CmsGetHd(int sock)
{
    DMS_NET_HDCFG hd_cfg;

    //to do
    memset(&hd_cfg, 0, sizeof(DMS_NET_HDCFG));
    hd_cfg.dwSize = sizeof(DMS_NET_HDCFG);
    hd_cfg.dwHDCount = 1;

    hd_cfg.stHDInfo[0].dwSize = sizeof(DMS_NET_SINGLE_HD);
    hd_cfg.stHDInfo[0].dwHDNo = 0;
    hd_cfg.stHDInfo[0].dwHDType = 0;
    hd_cfg.stHDInfo[0].dwCapacity = (unsigned long)grd_sd_get_all_size();
    hd_cfg.stHDInfo[0].dwFreeSpace = (unsigned long)grd_sd_get_free_size();
    hd_cfg.stHDInfo[0].dwHdStatus = mmc_get_sdcard_stauts();
    hd_cfg.stHDInfo[0].byHDAttr = 0;
    hd_cfg.stHDInfo[0].byRecStatus = is_thread_record_running();
    hd_cfg.stHDInfo[0].dwHdGroup = 0;

    //printf("all:%lu  free:%lu, is_record:%u\n", hd_cfg.stHDInfo[0].dwCapacity,
    //    hd_cfg.stHDInfo[0].dwFreeSpace, hd_cfg.stHDInfo[0].byRecStatus);
    return GkCmsCmdResq(sock, &hd_cfg, sizeof(DMS_NET_HDCFG), DMS_NET_GET_HDCFG);
}

int Gk_CmsSetHd(int sock)
{
    DMS_NET_HDCFG hd_cfg;
    int ret = GkSockRecv(sock, (char *)&hd_cfg, sizeof(DMS_NET_HDCFG));
    if (ret != sizeof(DMS_NET_HDCFG)) {
        PRINT_ERR();
        return -1;
    }

    //todo

    return 0;
}


int Gk_CmsHdFormat(int sock)
{
    DMS_NET_DISK_FORMAT hd_format;
    int ret = GkSockRecv(sock, (char *)&hd_format, sizeof(DMS_NET_DISK_FORMAT));
    if (ret != sizeof(DMS_NET_DISK_FORMAT)) {
        PRINT_ERR();
        return -1;
    }
    
    grd_sd_format();
    return 0;
}

//extern int format_process;

int Gk_CmsHdGetFormatStaus(int sock)
{
    DMS_NET_DISK_FORMAT_STATUS status;
    memset(&status, 0, sizeof(DMS_NET_DISK_FORMAT_STATUS));
    status.dwSize = sizeof(DMS_NET_DISK_FORMAT_STATUS);
    status.dwHDNo = 0;
    if (100 == grd_sd_get_format_process()) {
        status.dwHdStatus = 2;
        status.dwProcess = 100;
    } else {
        status.dwHdStatus = 1;
        status.dwProcess = grd_sd_get_format_process();
    }
    status.dwResult = 0;

    printf("size:%d no:%d status:%d process:%d result:%d\n", (int)status.dwSize, (int)status.dwHDNo, (int)status.dwHdStatus,
        (int)status.dwProcess, (int)status.dwResult);
    return GkCmsCmdResq(sock, &status, sizeof(DMS_NET_DISK_FORMAT_STATUS), DMS_NET_GET_HD_FORMAT_STATUS);
}

int Gk_CmsUnloadDisk()
{
    grd_sd_umount();
    return 0;
}

#endif

