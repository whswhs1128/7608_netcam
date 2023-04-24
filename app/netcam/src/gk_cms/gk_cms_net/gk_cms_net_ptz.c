#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"
#include "sdk_ptz.h"


#define DMS_PTZ_CMD_UP              1
#define DMS_PTZ_CMD_DOWN            2
#define DMS_PTZ_CMD_LEFT            3
#define DMS_PTZ_CMD_RIGHT           4
#define DMS_PTZ_CMD_UP_LEFT			31
#define DMS_PTZ_CMD_UP_RIGHT		32
#define DMS_PTZ_CMD_DOWN_LEFT		33
#define DMS_PTZ_CMD_DOWN_RIGHT		34

#define DMS_PTZ_CMD_FOCUS_SUB       5       //Focus Far
#define DMS_PTZ_CMD_FOCUS_ADD       6       //Focus Near
#define DMS_PTZ_CMD_ZOOM_SUB        7       //Zoom Wide
#define DMS_PTZ_CMD_ZOOM_ADD        8       //Zoom Tele
#define DMS_PTZ_CMD_IRIS_SUB        9       //Iris Close
#define DMS_PTZ_CMD_IRIS_ADD        10      //Iris Open
#define DMS_PTZ_CMD_STOP            11
#define DMS_PTZ_CMD_PRESET          12      //预置
#define DMS_PTZ_CMD_CALL            13      //调用

#define DMS_PTZ_CMD_AUTO_STRAT      14      //自动      //STRAT shoule be START
#define DMS_PTZ_CMD_AUTO_STOP       15
#define DMS_PTZ_CMD_LIGHT_OPEN      16      //灯光
#define DMS_PTZ_CMD_LIGHT_CLOSE     17
#define DMS_PTZ_CMD_BRUSH_START     18      //雨刷
#define DMS_PTZ_CMD_BRUSH_STOP      19
#define DMS_PTZ_CMD_TRACK_START     20      //轨迹
#define DMS_PTZ_CMD_TRACK_STOP      21
#define DMS_PTZ_CMD_TRACK_RUN       22
#define DMS_PTZ_CMD_PRESET_CLS      23      //清除预置点

#define DMS_PTZ_CMD_ADD_POS_CRU		24		/* 将预置点加入巡航序列 */
#define DMS_PTZ_CMD_DEL_POS_CRU		25		/* 将巡航点从巡航序列中删除 */
#define DMS_PTZ_CMD_DEL_PRE_CRU		26		/* 将预置点从巡航序列中删除 */
#define DMS_PTZ_CMD_MOD_POINT_CRU	27		/* 修改巡航点*/
#define DMS_PTZ_CMD_START_CRU		28		/* 开始巡航 */
#define DMS_PTZ_CMD_STOP_CRU		29		/* 停止巡航 */
#define DMS_PTZ_CMD_CRU_STATUS		30		/* 巡航状态 */


int Gk_CmsGetAllPreset(int sock)
{
    int cnt = 0;
    DMS_NET_PRESET_INFO preset_info;
    GK_NET_PRESET_INFO  gkPresetCfg;

    get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);

    preset_info.dwSize     = sizeof(DMS_NET_PRESET_INFO);
    preset_info.nChannel   = 0;
    preset_info.nPresetNum = gkPresetCfg.nPresetNum;
    for (cnt = 0; cnt < DMS_MAX_PRESET; cnt++)
    {
        preset_info.no[cnt] = gkPresetCfg.no[cnt];
        strncpy(preset_info.csName[cnt], gkPresetCfg.csName[cnt], sizeof(preset_info.csName[cnt]));
    }

    return GkCmsCmdResq(sock, (char *)&preset_info, sizeof(DMS_NET_PRESET_INFO), DMS_NET_GET_ALL_PRESET);
}


int Gk_CmsGetCruise(int sock)
{
    int i   = 0;
    int cnt = 0;
    DMS_NET_CRUISE_CFG  cruise;
    GK_NET_CRUISE_CFG   gkCruiseCfg;

    get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);

    cruise.dwSize          = sizeof(cruise);
    cruise.nChannel        = 0;
    cruise.byIsCruising    = gkCruiseCfg.byIsCruising;
    cruise.byCruisingIndex = gkCruiseCfg.byCruisingIndex;
    cruise.byPointIndex    = gkCruiseCfg.byPointIndex;
    cruise.byEnableCruise  = gkCruiseCfg.byEnableCruise;
    for (cnt = 0; cnt < DMS_MAX_CRUISE_GROUP_NUM; cnt++)
    {
        cruise.struCruise[cnt].byPointNum    = gkCruiseCfg.struCruise[cnt].byPointNum;
        cruise.struCruise[cnt].byCruiseIndex = gkCruiseCfg.struCruise[cnt].byCruiseIndex;
        for (i = 0; i < DMS_MAX_CRUISE_POINT_NUM; i++)
        {
            cruise.struCruise[cnt].struCruisePoint[i].byPointIndex =
                                    gkCruiseCfg.struCruise[cnt].struCruisePoint[i].byPointIndex;

            cruise.struCruise[cnt].struCruisePoint[i].byPresetNo =
                                    gkCruiseCfg.struCruise[cnt].struCruisePoint[i].byPresetNo;

            cruise.struCruise[cnt].struCruisePoint[i].byRemainTime =
                                    gkCruiseCfg.struCruise[cnt].struCruisePoint[i].byRemainTime;

            cruise.struCruise[cnt].struCruisePoint[i].bySpeed =
                                    gkCruiseCfg.struCruise[cnt].struCruisePoint[i].bySpeed;
        }
    }

    return GkCmsCmdResq(sock, (char *)&cruise, sizeof(DMS_NET_CRUISE_CFG), DMS_NET_GET_CRUISE_CFG);
}

int Gk_CmsSetCruise(int sock)
{
    int cnt   = 0;
    int index = 0;
    DMS_NET_CRUISE_INFO cruise;
    GK_NET_CRUISE_CFG   gkCruiseCfg;

    int ret = GkSockRecv(sock, (char *)&cruise, sizeof(DMS_NET_CRUISE_INFO));
    if (ret != sizeof(DMS_NET_CRUISE_INFO)) {
        PRINT_ERR();
        return -1;
    }
    get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
    gkCruiseCfg.nChannel = 0;
    //gkCruiseCfg.byIsCruising   = 0;
    //gkCruiseCfg.byPointIndex   = 0;
    //gkCruiseCfg.byEnableCruise = 0;
    if (cruise.byCruiseIndex >= DMS_MAX_CRUISE_GROUP_NUM)
    {
        PRINT_ERR("The byCruiseIndex:%d is error!  DMS_MAX_CRUISE_GROUP_NUM:%d \n",
                                        cruise.byCruiseIndex, DMS_MAX_CRUISE_GROUP_NUM);
        return -1;
    }
    index = cruise.byCruiseIndex;
    gkCruiseCfg.struCruise[index].byPointNum    = cruise.byPointNum;
    gkCruiseCfg.struCruise[index].byCruiseIndex = cruise.byCruiseIndex;
    for (cnt = 0; cnt < DMS_MAX_CRUISE_POINT_NUM; cnt++)
    {
        gkCruiseCfg.struCruise[index].struCruisePoint[cnt].byPointIndex =
                                       cruise.struCruisePoint[cnt].byPointIndex;

        gkCruiseCfg.struCruise[index].struCruisePoint[cnt].byPresetNo =
                                       cruise.struCruisePoint[cnt].byPresetNo;

        gkCruiseCfg.struCruise[index].struCruisePoint[cnt].byRemainTime =
                                       cruise.struCruisePoint[cnt].byRemainTime;

        gkCruiseCfg.struCruise[index].struCruisePoint[cnt].bySpeed =
                                       cruise.struCruisePoint[cnt].bySpeed;
    }

    set_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
    PresetCruiseCfgSave();
    return 0;
}


int Gk_CmsPtzControl(int sock)
{
    int cnt     = 0;
    int speed   = 0;
    int step    = 0;
    int num     = 0;
    int grp_idx = 0;
    int pot_idx = 0;
    DMS_NET_PTZ_CONTROL  ptz_control;
    DMS_NET_CRUISE_POINT cruisePoint;
    GK_NET_PRESET_INFO   gkPresetCfg;
    GK_NET_CRUISE_CFG    gkCruiseCfg;
    GK_NET_CRUISE_GROUP  cruise_info;

    int ret = GkSockRecv(sock, (char *)&ptz_control, sizeof(DMS_NET_PTZ_CONTROL));
    if (ret != sizeof(DMS_NET_PTZ_CONTROL)) {
        PRINT_ERR("ret:%d  sizeof(DMS_NET_PTZ_CONTROL)=%d \n", ret, sizeof(DMS_NET_PTZ_CONTROL));
        return -1;
    }

    PRINT_INFO(" dwSize:%ld  nChannel:%d   dwCommand:%ld  dwParam:%ld  my_size:%d\n",
                                                         ptz_control.dwSize,
                                                         ptz_control.nChannel,
                                                         ptz_control.dwCommand,
                                                         ptz_control.dwParam,
                                                         sizeof(DMS_NET_PTZ_CONTROL));
	step  = 30;
    speed = ptz_control.dwParam;
    switch(ptz_control.dwCommand)
    {
        case DMS_PTZ_CMD_STOP_CRU:      /* 停止巡航 */
			if ((ret = netcam_ptz_ciruise_stop()))
            {
                PRINT_ERR("call  netcam_ptz_stop error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_AUTO_STOP:
        case DMS_PTZ_CMD_STOP:
            if ((ret = netcam_ptz_stop()))
            {
                PRINT_ERR("call  netcam_ptz_stop error!\n");
                return -1;
            }
            break;

        case DMS_PTZ_CMD_UP:
            if ((ret = netcam_ptz_up(step, speed)))
            {
                PRINT_ERR("call  netcam_ptz_up error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_DOWN:
            if ((ret = netcam_ptz_down(step, speed)))
            {
                PRINT_ERR("call  netcam_ptz_down error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_LEFT:
            if ((ret = netcam_ptz_left(step, speed)))
            {
                PRINT_ERR("call  netcam_ptz_left error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_RIGHT:
            if ((ret = netcam_ptz_right(step, speed)))
            {
                PRINT_ERR("call  netcam_ptz_right error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_UP_LEFT:
            if ((ret = netcam_ptz_left_up(step, speed)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_UP_LEFT error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_UP_RIGHT:
            if ((ret = netcam_ptz_right_up(step, speed)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_UP_RIGHT error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_DOWN_LEFT:
            if ((ret = netcam_ptz_left_down(step, speed)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_DOWN_LEFT error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_DOWN_RIGHT:
            if ((ret = netcam_ptz_right_down(step, speed)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_DOWN_RIGHT error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_AUTO_STRAT:
            if ((ret = netcam_ptz_hor_ver_cruise(speed)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_AUTO_STRAT error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_PRESET:
            num = ptz_control.dwParam;
            if (num > PTZ_MAX_PRESET)
                num = PTZ_MAX_PRESET - 1;
            else if (num > 0)
                num -= 1;
            else
                num = 0;
            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum++;
            if (gkPresetCfg.nPresetNum >= NETCAM_PTZ_MAX_PRESET_NUM)
            {
                gkPresetCfg.nPresetNum = NETCAM_PTZ_MAX_PRESET_NUM;
            }
            gkPresetCfg.no[gkPresetCfg.nPresetNum-1] = num;
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            if ((ret = netcam_ptz_set_preset(num, NULL)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_PRESET error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_PRESET_CLS:
            num = ptz_control.dwParam;
            if (num > PTZ_MAX_PRESET)
                num = PTZ_MAX_PRESET - 1;
            else if (num > 0)
                num -= 1;
            else
                num = 0;
            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum--;
            if (gkPresetCfg.nPresetNum <= 0)
            {
                gkPresetCfg.nPresetNum = 0;
            }
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            if ((ret = netcam_ptz_clr_preset(num)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_PRESET_CLS error!\n");
                return -1;
            }
            break;
        case DMS_PTZ_CMD_CALL:
            num = ptz_control.dwParam;
            if (num > PTZ_MAX_PRESET)
                num = PTZ_MAX_PRESET - 1;
            else if (num > 0)
                num -= 1;
            else
                num = 0;
            cruise_info.byPointNum    = 1;
            cruise_info.byCruiseIndex = 0;
            cruise_info.struCruisePoint[0].byPointIndex = 0;
            cruise_info.struCruisePoint[0].byPresetNo   = num;
            cruise_info.struCruisePoint[0].byRemainTime = 0;
            cruise_info.struCruisePoint[0].bySpeed      = -1;
            if ((ret = netcam_ptz_stop()))
            {
                PRINT_ERR("call  netcam_ptz_stop error!\n");
                return -1;
            }
            if ((ret = netcam_ptz_preset_cruise(&cruise_info)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_AUTO_STRAT error!\n");
                return -1;
            }
            break;

        case DMS_PTZ_CMD_ADD_POS_CRU:   /* 将预置点加入巡航序列 */
            {
                grp_idx = ptz_control.dwParam;
                memcpy(&cruisePoint, ptz_control.byRes, sizeof(DMS_NET_CRUISE_POINT));
                if (grp_idx >= PTZ_MAX_CRUISE_GROUP_NUM)
                {
                    PRINT_ERR("index:%d > PTZ_MAX_CRUISE_GROUP_NUM!\n", grp_idx);
                    return -1;
                }

                get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);

                PRINT_INFO("ADD index:%d   byPointNum:%d  byPointIndex:%d  byPresetNo:%d  byRemainTime:%d  bySpeed:%d \n",
                                                                grp_idx,
                                                                gkCruiseCfg.struCruise[grp_idx].byPointNum,
                                                                cruisePoint.byPointIndex,
                                                                cruisePoint.byPresetNo,
                                                                cruisePoint.byRemainTime,
                                                                cruisePoint.bySpeed);

                if(gkCruiseCfg.struCruise[grp_idx].byPointNum < PTZ_MAX_CRUISE_POINT_NUM)
                {
                    gkCruiseCfg.struCruise[grp_idx].byPointNum++;
                }
                else
                {
                    gkCruiseCfg.struCruise[grp_idx].byPointNum = PTZ_MAX_CRUISE_POINT_NUM;
                    PRINT_INFO("total preset point num:PTZ_MAX_CRUISE_POINT_NUM ,don't add preset point!\n");
                    break;
                }

                /* 巡航组中的下标,如果值大于PTZ_MAX_CRUISE_POINT_NUM 表示添加到末尾 */
                if (cruisePoint.byPointIndex >= gkCruiseCfg.struCruise[grp_idx].byPointNum)
                {
                    cruisePoint.byPointIndex = gkCruiseCfg.struCruise[grp_idx].byPointNum - 1;
                }
                pot_idx = cruisePoint.byPointIndex;
                for (cnt = gkCruiseCfg.struCruise[grp_idx].byPointNum -1; cnt > pot_idx; cnt--)
                {
                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPointIndex =
                                                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPointIndex;

                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPresetNo   =
                                                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo;

                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byRemainTime =
                                                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byRemainTime;

                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].bySpeed      =
                                                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].bySpeed;
                }

                gkCruiseCfg.nChannel =0;
                gkCruiseCfg.struCruise[grp_idx].byCruiseIndex = grp_idx;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].byPointIndex =
                                                                                        cruisePoint.byPointIndex;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].byPresetNo   =
                                                                                        cruisePoint.byPresetNo;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].byRemainTime =
                                                                                        cruisePoint.byRemainTime;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].bySpeed      =
                                                                                        cruisePoint.bySpeed;

                set_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
                PresetCruiseCfgSave();
            }
            break;

        case DMS_PTZ_CMD_DEL_POS_CRU:   /* 将巡航点从巡航序列中删除 */
            {
                grp_idx = ptz_control.dwParam;
                memcpy(&cruisePoint, ptz_control.byRes, sizeof(DMS_NET_CRUISE_POINT));
                if (grp_idx >= PTZ_MAX_CRUISE_GROUP_NUM)
                {
                    PRINT_ERR("index:%d > PTZ_MAX_CRUISE_GROUP_NUM!\n", grp_idx);
                    return -1;
                }
                if (cruisePoint.byPointIndex > PTZ_MAX_CRUISE_POINT_NUM)
                {
                    PRINT_ERR("cruisePoint.byPointIndex:%d > PTZ_MAX_CRUISE_GROUP_NUM!\n", cruisePoint.byPointIndex);
                    cruisePoint.byPointIndex = PTZ_MAX_CRUISE_POINT_NUM - 1;
                }
                else if (cruisePoint.byPointIndex < 0)
                {
                    PRINT_ERR("cruisePoint.byPointIndex:%d < 0\n", cruisePoint.byPointIndex);
                    cruisePoint.byPointIndex = 0;
                }

                get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);

                PRINT_INFO("DELETE  index:%d   byPointNum:%d  byPointIndex:%d  byPresetNo:%d  byRemainTime:%d  bySpeed:%d \n",
                                                                grp_idx,
                                                                gkCruiseCfg.struCruise[grp_idx].byPointNum,
                                                                cruisePoint.byPointIndex,
                                                                cruisePoint.byPresetNo,
                                                                cruisePoint.byRemainTime,
                                                                cruisePoint.bySpeed);

                gkCruiseCfg.nChannel = 0;
                if (cruisePoint.byPointIndex > gkCruiseCfg.struCruise[grp_idx].byPointNum)
                {
                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPointIndex = 0;
                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPresetNo   = 0;
                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byRemainTime = 0;
                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].bySpeed      = 0;
                    break;
                }

                gkCruiseCfg.struCruise[grp_idx].byPointNum--;
                pot_idx = cruisePoint.byPointIndex;
                gkCruiseCfg.struCruise[grp_idx].byCruiseIndex = grp_idx;
                for (cnt = pot_idx; cnt < gkCruiseCfg.struCruise[grp_idx].byPointNum; cnt++)
                {
                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPointIndex =
                                                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPointIndex;

                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo   =
                                                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPresetNo;

                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byRemainTime =
                                                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byRemainTime;

                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].bySpeed      =
                                                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].bySpeed;
                }

                pot_idx = gkCruiseCfg.struCruise[grp_idx].byPointNum;
                //printf("----------> pot_idx:%d \n", pot_idx);
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPointIndex = 0;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPresetNo   = 0;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byRemainTime = 0;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].bySpeed      = 0;

                set_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
                PresetCruiseCfgSave();
            }
            break;

        case DMS_PTZ_CMD_DEL_PRE_CRU:   /* 将预置点从巡航序列中删除 */
            break;

        case DMS_PTZ_CMD_MOD_POINT_CRU: /* 修改巡航点*/
             {
                grp_idx = ptz_control.dwParam;
                memcpy(&cruisePoint, ptz_control.byRes, sizeof(DMS_NET_CRUISE_POINT));
                if (grp_idx >= PTZ_MAX_CRUISE_GROUP_NUM)
                {
                    PRINT_ERR("index:%d > PTZ_MAX_CRUISE_GROUP_NUM!\n", grp_idx);
                    return -1;
                }
                if (cruisePoint.byPointIndex > PTZ_MAX_CRUISE_POINT_NUM)
                {
                    PRINT_ERR("cruisePoint.byPointIndex:%d > PTZ_MAX_CRUISE_GROUP_NUM!\n", cruisePoint.byPointIndex);
                    return -1;
                }

                get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);

                PRINT_INFO("MOD  index:%d   byPointNum:%d  byPointIndex:%d  byPresetNo:%d  byRemainTime:%d  bySpeed:%d \n",
                                                                    grp_idx,
                                                                    gkCruiseCfg.struCruise[grp_idx].byPointNum,
                                                                    cruisePoint.byPointIndex,
                                                                    cruisePoint.byPresetNo,
                                                                    cruisePoint.byRemainTime,
                                                                    cruisePoint.bySpeed);

                gkCruiseCfg.nChannel =0;
                gkCruiseCfg.struCruise[grp_idx].byCruiseIndex = grp_idx;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].byPointIndex =
                                                                                        cruisePoint.byPointIndex;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].byPresetNo   =
                                                                                        cruisePoint.byPresetNo;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].byRemainTime =
                                                                                        cruisePoint.byRemainTime;
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].bySpeed      =
                                                                                        cruisePoint.bySpeed;
                set_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
                PresetCruiseCfgSave();
            }
            break;

        case DMS_PTZ_CMD_START_CRU:     /* 开始巡航 */
            {
                grp_idx = ptz_control.dwParam;
                if (grp_idx >= PTZ_MAX_CRUISE_GROUP_NUM)
                {
                    PRINT_ERR("START_CRU grp_idx:%d > PTZ_MAX_CRUISE_GROUP_NUM:%d\n", grp_idx, PTZ_MAX_CRUISE_GROUP_NUM);
                    return -1;
                }
                get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
                for (cnt = 0; cnt < PTZ_MAX_CRUISE_POINT_NUM; cnt++)
                {
                    if (gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo > 0)
                    {
                        gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo--;
                    }
                }
                ret = netcam_ptz_preset_cruise(&gkCruiseCfg.struCruise[grp_idx]);
                if (ret < 0)
                {
                    PRINT_ERR("Call netcam_ptz_preset_cruise error!\n");
                    return -1;
                }
            }
            break;

        case DMS_PTZ_CMD_FOCUS_SUB:     /* 聚焦 -  */
        case DMS_PTZ_CMD_FOCUS_ADD:     /* 聚焦 +  */
        case DMS_PTZ_CMD_ZOOM_SUB:      /* 变倍 -  */
        case DMS_PTZ_CMD_ZOOM_ADD:      /* 变倍 +  */
        case DMS_PTZ_CMD_IRIS_SUB:      /* 光圈 -  */
        case DMS_PTZ_CMD_IRIS_ADD:      /* 光圈 + */
        //case DMS_PTZ_CMD_AUTO_STRAT:    /* 自动 STRAT  */
        //case DMS_PTZ_CMD_AUTO_STOP:     /* 自动 STOP    */
        case DMS_PTZ_CMD_CRU_STATUS:    /* 巡航状态 */
        case DMS_PTZ_CMD_LIGHT_OPEN:    /* 灯光open  */
        case DMS_PTZ_CMD_LIGHT_CLOSE:   /* 灯光close  */
        case DMS_PTZ_CMD_BRUSH_START:   /* 雨刷start  */
        case DMS_PTZ_CMD_BRUSH_STOP:    /* 雨刷stop  */
            PRINT_INFO("This command:%ld  don't support now!\n", ptz_control.dwCommand);
            break;

        default:
            PRINT_ERR("This command:%ld is error!\n", ptz_control.dwCommand);
            break;
    }

    return 0;
}


int Gk_CmsGetRs232(int sock)
{
    DMS_NET_RS232CFG    rs232;
    GK_NET_DECODERCFG   uart_dec;

    get_param(PTZ_DEC_PARAM_ID, (void *)&uart_dec);

    rs232.dwSize        = sizeof(DMS_NET_RS232CFG);
    rs232.dwBaudRate    = uart_dec.baudRate;
    rs232.byDataBit     = uart_dec.dataBit;
    rs232.byStopBit     = uart_dec.stopBit;
    rs232.byParity      = uart_dec.parity;
    rs232.byFlowcontrol = uart_dec.flowcontrol;
    rs232.dwWorkMode    = uart_dec.workMode;

    GkSockSend(sock, (char *)&rs232, sizeof(DMS_NET_RS232CFG), 100);
    return 0;
}

int Gk_CmsGetDefRs232(int sock)
{
    int ret = 0;
    DMS_NET_RS232CFG  rs232;
    GK_NET_PTZ_CFG    ptzCfg_old;
    GK_NET_DECODERCFG ptzDecCfg_old;
    GK_NET_DECODERCFG ptzDecCfg_def;

    get_param(PTZ_PARAM_ID,     &ptzCfg_old);
    get_param(PTZ_DEC_PARAM_ID, &ptzDecCfg_old);

    PtzCfgLoadDefValue();
    get_param(PTZ_DEC_PARAM_ID, &ptzDecCfg_def);

    set_param(PTZ_PARAM_ID,     &ptzCfg_old);
    set_param(PTZ_DEC_PARAM_ID, &ptzDecCfg_old);
    ret = PtzCfgSave();
    if(ret < 0)
    {
        PRINT_ERR("PtzCfgSave error!\n");
        return -1;
    }

    rs232.dwSize        = sizeof(DMS_NET_RS232CFG);
    rs232.dwBaudRate    = ptzDecCfg_def.baudRate;
    rs232.byDataBit     = ptzDecCfg_def.dataBit;
    rs232.byStopBit     = ptzDecCfg_def.stopBit;
    rs232.byParity      = ptzDecCfg_def.parity;
    rs232.byFlowcontrol = ptzDecCfg_def.flowcontrol;
    rs232.dwWorkMode    = ptzDecCfg_def.workMode;

    GkSockSend(sock, (char *)&rs232, sizeof(DMS_NET_RS232CFG), 100);
    return 0;
}

int Gk_CmsSetRs232(int sock)
{
    GK_NET_DECODERCFG  uart_dec;
    DMS_NET_RS232CFG   rs232;
    int ret = GkSockRecv(sock, (char *)&rs232, sizeof(DMS_NET_RS232CFG));
    if (ret != sizeof(DMS_NET_RS232CFG)) {
        PRINT_ERR();
        return -1;
    }

    uart_dec.baudRate     = rs232.dwBaudRate;
    uart_dec.dataBit      = rs232.byDataBit;
    uart_dec.stopBit      = rs232.byStopBit;
    uart_dec.parity       = rs232.byParity;
    uart_dec.flowcontrol  = rs232.byFlowcontrol;
    uart_dec.workMode     = rs232.dwWorkMode;

    set_param(PTZ_DEC_PARAM_ID, (void *)&uart_dec);

    ret = PtzCfgSave();
    if(ret < 0)
    {
        PRINT_ERR("PtzCfgSave error!\n");
        return -1;
    }

    return 0;
}


int Gk_CmsGetPtzDecode(int sock)
{
    DMS_NET_DECODERCFG  ptz_decode;
    GK_NET_DECODERCFG   uart_dec;
    memset(&ptz_decode, 0, sizeof(DMS_NET_DECODERCFG));

    get_param(PTZ_DEC_PARAM_ID, (void *)&uart_dec);

    ptz_decode.dwSize           = sizeof(DMS_NET_DECODERCFG);
    ptz_decode.dwChannel        = 0;
    ptz_decode.dwBaudRate       = uart_dec.baudRate;
    ptz_decode.byDataBit        = uart_dec.dataBit;
    ptz_decode.byStopBit        = uart_dec.stopBit;
    ptz_decode.byParity         = uart_dec.parity;
    ptz_decode.byFlowcontrol    = uart_dec.flowcontrol;
    memcpy(ptz_decode.csDecoderType, uart_dec.decoderType, sizeof(uart_dec.decoderType));
    ptz_decode.wDecoderAddress  = uart_dec.decoderAddress;
    ptz_decode.byHSpeed         = uart_dec.speedH;
    ptz_decode.byVSpeed         = uart_dec.speedV;
    ptz_decode.byWatchPos       = uart_dec.watchPos;

    return GkCmsCmdResq(sock, &ptz_decode, sizeof(DMS_NET_DECODERCFG), DMS_NET_GET_DECODERCFG);
}

int Gk_CmsSetPtzDecode(int sock)
{
    DMS_NET_DECODERCFG ptz_decode;
    GK_NET_DECODERCFG  uart_dec;

    int ret = GkSockRecv(sock, (char *)&ptz_decode, sizeof(DMS_NET_DECODERCFG));
    if (ret != sizeof(DMS_NET_DECODERCFG)) {
        PRINT_ERR();
        return -1;
    }

    uart_dec.baudRate     = ptz_decode.dwBaudRate;
    uart_dec.dataBit      = ptz_decode.byDataBit;
    uart_dec.stopBit      = ptz_decode.byStopBit;
    uart_dec.parity       = ptz_decode.byParity;
    uart_dec.flowcontrol  = ptz_decode.byFlowcontrol;
    memcpy(uart_dec.decoderType, ptz_decode.csDecoderType, sizeof(uart_dec.decoderType));
    uart_dec.decoderAddress  = ptz_decode.wDecoderAddress;
    uart_dec.speedH          = ptz_decode.byHSpeed;
    uart_dec.speedV          = ptz_decode.byVSpeed;
    uart_dec.watchPos        = ptz_decode.byWatchPos;

    set_param(PTZ_DEC_PARAM_ID, (void *)&uart_dec);

    ret = PtzCfgSave();
    if(ret < 0)
    {
        PRINT_ERR("PtzCfgSave error!\n");
        return -1;
    }

    return 0;
}

int Gk_CmsGetDefPtzDecode(int sock)
{
    int ret = 0;
    DMS_NET_DECODERCFG ptz_decode;
    GK_NET_PTZ_CFG     ptzCfg_old;
    GK_NET_DECODERCFG  ptzDecCfg_old;
    GK_NET_DECODERCFG  ptzDecCfg_def;

    get_param(PTZ_PARAM_ID,     &ptzCfg_old);
    get_param(PTZ_DEC_PARAM_ID, &ptzDecCfg_old);

    PtzCfgLoadDefValue();
    get_param(PTZ_DEC_PARAM_ID, &ptzDecCfg_def);

    set_param(PTZ_PARAM_ID,     &ptzCfg_old);
    set_param(PTZ_DEC_PARAM_ID, &ptzDecCfg_old);
    ret = PtzCfgSave();
    if(ret < 0)
    {
        PRINT_ERR("PtzCfgSave error!\n");
        return -1;
    }

    memset(&ptz_decode, 0, sizeof(DMS_NET_DECODERCFG));

    ptz_decode.dwSize           = sizeof(DMS_NET_DECODERCFG);
    ptz_decode.dwChannel        = 0;
    ptz_decode.dwBaudRate       = ptzDecCfg_def.baudRate;
    ptz_decode.byDataBit        = ptzDecCfg_def.dataBit;
    ptz_decode.byStopBit        = ptzDecCfg_def.stopBit;
    ptz_decode.byParity         = ptzDecCfg_def.parity;
    ptz_decode.byFlowcontrol    = ptzDecCfg_def.flowcontrol;
    memcpy(ptz_decode.csDecoderType, ptzDecCfg_def.decoderType, sizeof(ptzDecCfg_def.decoderType));
    ptz_decode.wDecoderAddress  = ptzDecCfg_def.decoderAddress;
    ptz_decode.byHSpeed         = ptzDecCfg_def.speedH;
    ptz_decode.byVSpeed         = ptzDecCfg_def.speedV;
    ptz_decode.byWatchPos       = ptzDecCfg_def.watchPos;

    return GkCmsCmdResq(sock, &ptz_decode, sizeof(DMS_NET_DECODERCFG), DMS_NET_GET_DEF_DECODERCFG);
}

