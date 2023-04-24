/*!
*****************************************************************************
** \file        packages/onvif/src/gk7101/gk7101_ptz.c
**
**
** \brief       Onvif PTZ Control Interface
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <math.h>
#include "gk7101_ptz.h"
#include "gk7101_cmd.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
typedef struct {
	GK_U8 Num;
	GK_S8 PresetName[MAX_NAME_LENGTH];
	GK_S8 PresetToken[MAX_TOKEN_LENGTH];
}Preset;

typedef struct listNode
{
	 Preset Preset;
	 struct listNode *Next;
}Node;


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static void save_preset_list();
static GK_S32 get_all_presets(Preset preset[], GK_U32 *presetSize);
static GK_U8 get_preset_num(Preset *preset);
static GK_BOOL update_preset_name(Preset *preset);
static void preset_list_add(Preset *preset);
static GK_S32 preset_list_del(GK_U32 num);

void ptz_peset_list_init();
GK_S32 ptz_get_device_info(ONVIF_PTZ_DevParamsT *pstPtzDevInfo);
GK_S32 ptz_continous_move(ONVIF_PTZ_ContinousMoveInfoT * info);
GK_S32 ptz_absolute_move(ONVIF_PTZ_AbsoluteMoveInfoT * info);
GK_S32 ptz_relative_move(ONVIF_PTZ_RelativeMoveInfoT * info);
GK_S32 ptz_stop(Onvif_PTZ_StopInfoT* info);
GK_S32 ptz_set_preset(ONVIF_PTZ_PresetInfoT* info);
GK_S32 ptz_goto_preset(ONVIF_PTZ_PresetInfoT* info);
GK_S32 ptz_get_all_presets(ONVIF_PTZ_AllPresetsInfoT* info);
GK_S32 ptz_remove_preset(ONVIF_PTZ_PresetInfoT* info);
GK_BOOL ptz_check_is_preset_name_exist(ONVIF_PTZ_PresetInfoT *info);
GK_U32 ptz_assign_preset_num(void);
//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

static GK_BOOL presetNum[MAX_PTZ_PRESET_NUM + 1];
static Node *head;

PtzAdapterConfiguration_t gPtzAdapterConfiguration =
{
    .ptzContinousMove = ptz_continous_move,
    .ptzAbsoluteMove = ptz_absolute_move,
    .ptzRelativeMove = ptz_relative_move,
    .ptzStop = ptz_stop,
    .ptzSetPreset = ptz_set_preset,
    .ptzGotoPreset = ptz_goto_preset,
    .ptzGetAllPresets = ptz_get_all_presets,
    .ptzRemovePreset = ptz_remove_preset,
    .ptzCheckPresetName = ptz_check_is_preset_name_exist,
    .ptzAssignPresetNum = ptz_assign_preset_num,
    .ptzPresetListInit = ptz_peset_list_init,
    .ptzGetDeviceInfo = ptz_get_device_info,
};


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

void ptz_peset_list_init()
{
    FILE *fp = NULL;
    Node *p = NULL;
    Node *pt = NULL;

    head = (Node *)malloc(sizeof(Node));
    memset(head, 0, sizeof(Node));
    fp = fopen(PTZPRESETINFORMATION, "r");
    if(fp == NULL)
    {
        ONVIF_ERR("No ptz preset\n");
        return;
    }

    memset(presetNum, GK_FALSE, sizeof(presetNum));
    p = head;
    ONVIF_DBG("****************load ptz presets**************\n");
    while(!feof(fp))
    {
        pt = (Node *)malloc(sizeof(Node));
        if(fread(&pt->Preset, sizeof(Preset), 1, fp) == 0)
        {
            free(pt);
            break;
        }
        p->Next = pt;
        p = pt;
        p->Next = NULL;
        ONVIF_DBG("%d\t%s\t%s\n", p->Preset.Num ,p->Preset.PresetToken, p->Preset.PresetName);
        presetNum[p->Preset.Num] = GK_TRUE;
    }
    ONVIF_DBG("******************** end **********************\n");
    fclose(fp);

    return;
}

GK_S32 ptz_get_device_info(ONVIF_PTZ_DevParamsT *pstPtzDevInfo)
{
    GK_S32 retVal;
	retVal = gk7101_Comm_CmdSend(PTZ_GET_DEVICE_INFO, 0, NULL, (GK_U8)sizeof(ONVIF_PTZ_DevParamsT), (GK_U8*)pstPtzDevInfo);
    ONVIF_DBG("[gk7101_ptz]devName:%s,presetsCapacity:%d\n",pstPtzDevInfo->devName,
	    pstPtzDevInfo->presetsCapacity);

	return retVal;
}

GK_S32 ptz_continous_move(ONVIF_PTZ_ContinousMoveInfoT * info)
{
    ONVIF_PTZ_CmdT stCmd;
    IPC_CMDTYPE cmd_type;
    GK_S32 retval = -1;

    if(info == NULL) {
        return retval;
    }

    memset(&stCmd, 0, sizeof(stCmd));

    if(info->panTilt) {
        if(info->panSpeed && info->tiltSpeed) {
            if(info->panSpeed > 0) {
                if(info->tiltSpeed > 0) {
                    cmd_type = PTZ_PANTILT_RIGHTUP;
                }
                else {
                    cmd_type = PTZ_PANTILT_RIGHTDOWN;
                }
            }
            else {
                if(info->tiltSpeed > 0) {
                    cmd_type = PTZ_PANTILT_LEFTUP;
                }
                else {
                    cmd_type = PTZ_PANTILT_LEFTDOWN;
                }
            }
        }
        else if(info->panSpeed) {
            if(info->panSpeed > 0){
                cmd_type = PTZ_PAN_RIGHT;
            }
            else {
                cmd_type = PTZ_PAN_LEFT;
            }
        }
        else if(info->tiltSpeed) {
            if(info->tiltSpeed > 0){
                cmd_type = PTZ_TILT_UP;
            }
            else {
                cmd_type = PTZ_TILT_DOWN;
            }
        }
        else {
            cmd_type = PTZ_STOP;
        }

        stCmd.panSpeed = (GK_U8)((float)0x3F * fabs(info->panSpeed));
        stCmd.tiltSpeed = (GK_U8)((float)0x3F * fabs(info->tiltSpeed));
        retval = gk7101_Comm_CmdSend(cmd_type, sizeof(ONVIF_PTZ_CmdT), (GK_U8*)&stCmd, 0, NULL);
        if(0 != retval) {
            return retval;
        }
    }
    if (info->zoom) {
        if(info->zoom > 0){
            cmd_type = PTZ_ZOOM_TELE;
        }
        else {
            cmd_type = PTZ_ZOOM_WIDE;
        }
        /*ignore zoom speed*/
        stCmd.zoomSpeed = 0;
        retval = gk7101_Comm_CmdSend(cmd_type, sizeof(ONVIF_PTZ_CmdT), (GK_U8*)&stCmd, 0, NULL);
    }
    return retval;
}

GK_S32 ptz_absolute_move(ONVIF_PTZ_AbsoluteMoveInfoT * info)
{
    GK_S32 retval = 0;
    /* TODO : absolute movement*/
    return retval;
}

GK_S32 ptz_relative_move(ONVIF_PTZ_RelativeMoveInfoT * info)
{
    GK_S32 retval = 0;
    /* TODO : relative movement*/
    return retval;
}


GK_S32 ptz_stop(Onvif_PTZ_StopInfoT* info)
{
    ONVIF_PTZ_CmdT stCmd;
    IPC_CMDTYPE cmd_type;
    GK_S32 retval = -1;

    if(info == NULL) {
        return retval;
    }
    /*always stop all ongoing pan,tlit adn zoom movements*/
    if(info->stopAll) {
        cmd_type = PTZ_STOP;
    }
    else {
        if(info->stopPt) {
            cmd_type = PTZ_STOP;
        }
        if(info->stopZoom) {
            cmd_type = PTZ_STOP;
        }
    }
    memset(&stCmd, 0, sizeof(stCmd));
    retval = gk7101_Comm_CmdSend(cmd_type, sizeof(ONVIF_PTZ_CmdT), (GK_U8*)&stCmd, 0, NULL);

    return retval;
}

GK_S32 ptz_set_preset(ONVIF_PTZ_PresetInfoT* info)
{
    ONVIF_PTZ_CmdT stCmd;
    IPC_CMDTYPE cmd_type;
    GK_S32 retval = -1;
    GK_BOOL create = GK_FALSE;
    GK_BOOL update_flag = GK_FALSE;

    cmd_type = PTZ_SET_PRESET;
    if(info->presetNum) {
        /*create a preset*/
        create = GK_TRUE;
    }
    else {
        /*update a preset*/
        info->presetNum = get_preset_num((Preset*)info);
        if(info->presetNum == 0) {
            return retval;
        }
        if(NULL != info->presetName) {
            update_flag = update_preset_name((Preset*)info);
        }
        create = GK_FALSE;
    }
    memset(&stCmd, 0, sizeof(stCmd));
    stCmd.presetNum = info->presetNum;
    retval = gk7101_Comm_CmdSend(cmd_type, sizeof(ONVIF_PTZ_CmdT), (GK_U8*)&stCmd, 0, NULL);
    if(0 != retval) {
        return retval;
    }

    if(create == GK_TRUE) {
        preset_list_add((Preset*)info);
    }
    if((create == GK_TRUE) || (update_flag == GK_TRUE)) {
        save_preset_list();
    }

    return retval;
}

GK_S32 ptz_goto_preset(ONVIF_PTZ_PresetInfoT* info)
{
    ONVIF_PTZ_CmdT stCmd;
    IPC_CMDTYPE cmd_type;
    GK_U8 preset_num;
    GK_S32 retval = -1;

    cmd_type = PTZ_GOTO_PRESET;

    preset_num = get_preset_num((Preset*)info);
    if(preset_num == 0) {
        return 1;
    }

    stCmd.presetNum = preset_num;
    retval = gk7101_Comm_CmdSend(cmd_type, sizeof(ONVIF_PTZ_CmdT), (GK_U8*)&stCmd, 0, NULL);
    if(0 != retval) {
        return retval;
    }

    return retval;
}

GK_S32 ptz_get_all_presets(ONVIF_PTZ_AllPresetsInfoT* info)
{
    return get_all_presets((Preset*)info->presets, &info->presetSize);
}

GK_S32 ptz_remove_preset(ONVIF_PTZ_PresetInfoT* info)
{
    ONVIF_PTZ_CmdT stCmd;
    IPC_CMDTYPE cmd_type;
    GK_U8 preset_num;
    GK_S32 retval = -1;

    cmd_type = PTZ_CLEAR_PRESET;

    preset_num = get_preset_num((Preset*)info);
    if(preset_num == 0) {
        return 1;
    }

    stCmd.presetNum = preset_num;
    retval = gk7101_Comm_CmdSend(cmd_type, sizeof(ONVIF_PTZ_CmdT), (GK_U8*)&stCmd, 0, NULL);
    if(0 != retval) {
        return retval;
    }

    preset_list_del(preset_num);
    save_preset_list();

    return 0;
}

GK_BOOL ptz_check_is_preset_name_exist(ONVIF_PTZ_PresetInfoT *info)
{
    Node *p;

    p = head->Next;

    while (NULL != p) {
        if(strcmp(p->Preset.PresetName, info->presetName) == 0) {
            return GK_TRUE;
        }
        p = p->Next;
    }

    return GK_FALSE;
}

GK_U32 ptz_assign_preset_num(void)
{
    GK_U32 num;

    for(num = 1; num < MAX_PTZ_PRESET_NUM + 1; num++) {
        if(!presetNum[num]){
            return num;
        }
    }

    return 0;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static void save_preset_list()
{
    FILE *fp;
    Node *p;

    if((fp = fopen(PTZPRESETINFORMATION, "w")) == NULL) {
        ONVIF_ERR("error: Open ptz preset config file failed\n");
        return;
    }

    p = head->Next;
    while(NULL != p) {
        fwrite(&p->Preset, sizeof(Preset), 1, fp);
        p = p->Next;
    }

    fclose(fp);
    ONVIF_DBG("\nsave %s succeeded\n", PTZPRESETINFORMATION);
    return;
}

static GK_S32 get_all_presets(Preset preset[], GK_U32 *presetSize)
{
    Node *p;
    GK_U32 size = 0;

    p = head->Next;
    while (NULL != p) {
        memcpy(preset, &p->Preset, sizeof(Preset));
        if(size < MAX_PTZ_PRESET_NUM) {
            size ++;
            preset++;
        }
        else {
            ONVIF_ERR("the preset array not big enough");
            return -1;
        }
        p = p->Next;
    }
    *presetSize = size;

    return 0;
}

static GK_U8 get_preset_num(Preset *preset)
{
    Node *p;

    p = head->Next;

    while (NULL != p) {
        if(strcmp(p->Preset.PresetToken, preset->PresetToken) == 0) {
            return p->Preset.Num;
        }
        p = p->Next;
    }

    return 0;
}

static GK_BOOL update_preset_name(Preset *preset)
{
    Node *p;
    GK_BOOL update = GK_TRUE;

    p = head->Next;

    while (NULL != p) {
        if(strcmp(p->Preset.PresetToken, preset->PresetToken) == 0) {
            if(strcmp(p->Preset.PresetName, preset->PresetName) != 0) {
                memcpy(p->Preset.PresetName, preset->PresetName, sizeof(preset->PresetName));
                update = GK_TRUE;
            }
            break;
        }
        p = p->Next;
    }

    return update;
}

static void preset_list_add(Preset *preset)
{
    Node *p;
    Node *pst;
    Node *pt;

    pst = (Node *)malloc(sizeof(Node));
    memcpy(&pst->Preset, preset, sizeof(Preset));

    p = head;
    while(NULL != p->Next) {
        if(preset->Num < p->Next->Preset.Num) {
            pt = p->Next;
            p->Next = pst;
            pst->Next = pt;
            break;
        }
        p = p->Next;
    }
    if(NULL == p->Next) {
        p->Next = pst;
        pst->Next = NULL;
    }

    presetNum[preset->Num] = GK_TRUE;
    return;
}

static GK_S32 preset_list_del(GK_U32 num)
{
    Node *p;
    Node *pt;

    p = head;
    while(NULL != p->Next) {
        if(num == p->Next->Preset.Num) {
            pt = p->Next;
            p->Next= p->Next->Next;
            free(pt);
            presetNum[num] = GK_TRUE;
            return 0;
        }
        p = p->Next;
    }

    ONVIF_ERR("The preset does not exists");
    return 1;
}
