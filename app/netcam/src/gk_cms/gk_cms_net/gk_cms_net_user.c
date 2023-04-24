#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"


int Gk_CmsSetUser(int sock)
{
#if 0
    DMS_NET_USER user;
    int ret = GkSockRecv(sock, (char *)&user, sizeof(DMS_NET_USER));
    if (ret != sizeof(DMS_NET_USER)) {
        PRINT_ERR();
        return -1;
    }

    //todo
    int i;
    DMS_NET_USER_INFO *user_info = NULL;
    for(i = 0; i < DMS_MAX_USERNUM; i ++) {
        user_info = &(user.stUser[i]);

        runUserCfg.user[i].enable = user_info->bEnable;
        runUserCfg.user[i].userRight = user_info->dwUserRight;
        strncpy(runUserCfg.user[i].userName, user_info->csUserName, sizeof(user_info->csUserName));
        strncpy(runUserCfg.user[i].password, user_info->csPassword, sizeof(user_info->csPassword));
    }

    return 0;
#else
    DMS_NET_USER_INFO stUser;
    int ret = GkSockRecv(sock, (char *)&stUser, sizeof(DMS_NET_USER_INFO));
    if (ret != sizeof(DMS_NET_USER_INFO)) {
        PRINT_ERR();
        return -1;
    }

    printf("index:%lu enable:%d name:%s, password:%s \n", stUser.dwIndex, stUser.bEnable, stUser.csUserName, stUser.csPassword);

    return 0;
#endif
}



int Gk_CmsGetUser(int sock)
{
    DMS_NET_USER user;
    memset(&user, 0, sizeof(DMS_NET_USER));
    user.dwSize = sizeof(DMS_NET_USER);

    int i;
    DMS_NET_USER_INFO *user_info = NULL;
    for(i = 0; i < DMS_MAX_USERNUM; i ++) {
        user_info = &(user.stUser[i]);
        user_info->dwSize = sizeof(DMS_NET_USER_INFO);
        user_info->dwIndex = i;

        if (runUserCfg.user[i].enable) {
            user_info->bEnable = runUserCfg.user[i].enable;
            strncpy(user_info->csUserName, runUserCfg.user[i].userName, sizeof(user_info->csUserName));
            strncpy(user_info->csPassword, runUserCfg.user[i].password, sizeof(user_info->csPassword));
            user_info->dwUserRight = runUserCfg.user[i].userRight;

            user_info->byLocalPreviewRight[0] = 1;
            user_info->byLocalPlaybackRight[0] = 1;
            user_info->byNetPlaybackRight[0] = 1;
            user_info->byLocalRecordRight[0] = 1;
            user_info->byNetRecordRight[0] = 1;
            user_info->byLocalPTZRight[0] = 1;
            user_info->byNetPTZRight[0] = 1;
            user_info->byLocalBackupRight[0] = 1;
            user_info->byNetBackupRight[0] = 1;
            user_info->byPriority = 3;
        }
    }

    GkCmsCmdResq(sock, (char *)&user, sizeof(DMS_NET_USER), DMS_NET_GET_USERCFG);
    return 0;
}



