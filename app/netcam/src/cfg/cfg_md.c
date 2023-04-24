/*!
*****************************************************************************
** FileName     : cfg_md.c
**
** Description  : md config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#include "cfg_md.h"

GK_NET_MD_CFG runMdCfg;

#define MAX_MD_GRID_NUM 4


#if USE_DEFAULT_PARAM
GK_CFG_MAP mdMap[] = {
    {"md.channel",           &(runMdCfg.channel),       GK_CFG_DATA_TYPE_S32, "0", 1, 0, 3, NULL},
    {"md.enable",            &(runMdCfg.enable),        GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1, NULL},
    {"md.sensitive",         &(runMdCfg.sensitive),     GK_CFG_DATA_TYPE_S32, "50", 1, 0, 100, "0-100"},
    {"md.compensation",      &(runMdCfg.compensation),  GK_CFG_DATA_TYPE_S32, "1", 1, 0, 1, "0, 1"},
    {"md.detectionType",     &(runMdCfg.detectionType), GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1, "0 grid, 1 region"},

    {"md.grid.row",          &(runMdCfg.mdGrid.row),         GK_CFG_DATA_TYPE_S32,  "15", 1, 0, 100, NULL},
    {"md.grid.column",       &(runMdCfg.mdGrid.column),      GK_CFG_DATA_TYPE_S32,  "22", 1, 0, 100, NULL},
    {"md.grid.granularity",  &(runMdCfg.mdGrid.granularity), GK_CFG_DATA_TYPE_STRING,   "0",  1, 1, GK_NET_MD_GRID_ROW * GK_NET_MD_GRID_COLUMN + 1, NULL},

    {"md.region0.enable",    &(runMdCfg.mdRegion[0].enable),    GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1,    NULL},
    {"md.region0.x",         &(runMdCfg.mdRegion[0].x),         GK_CFG_DATA_TYPE_S32, "0", 1, 1, 1920, NULL},
    {"md.region0.y",         &(runMdCfg.mdRegion[0].y),         GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1280, NULL},
    {"md.region0.width",     &(runMdCfg.mdRegion[0].width),     GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1920, NULL},
    {"md.region0.height",    &(runMdCfg.mdRegion[0].height),    GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1280, NULL},

    {"md.region1.enable",    &(runMdCfg.mdRegion[1].enable),    GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1,    NULL},
    {"md.region1.x",         &(runMdCfg.mdRegion[1].x),         GK_CFG_DATA_TYPE_S32, "0", 1, 1, 1920, NULL},
    {"md.region1.y",         &(runMdCfg.mdRegion[1].y),         GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1280, NULL},
    {"md.region1.width",     &(runMdCfg.mdRegion[1].width),     GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1920, NULL},
    {"md.region1.height",    &(runMdCfg.mdRegion[1].height),    GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1280, NULL},

    {"md.region2.enable",    &(runMdCfg.mdRegion[2].enable),    GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1,    NULL},
    {"md.region2.x",         &(runMdCfg.mdRegion[2].x),         GK_CFG_DATA_TYPE_S32, "0", 1, 1, 1920, NULL},
    {"md.region2.y",         &(runMdCfg.mdRegion[2].y),         GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1280, NULL},
    {"md.region2.width",     &(runMdCfg.mdRegion[2].width),     GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1920, NULL},
    {"md.region2.height",    &(runMdCfg.mdRegion[2].height),    GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1280, NULL},

    {"md.region3.enable",    &(runMdCfg.mdRegion[3].enable),    GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1,    NULL},
    {"md.region3.x",         &(runMdCfg.mdRegion[3].x),         GK_CFG_DATA_TYPE_S32, "0", 1, 1, 1920, NULL},
    {"md.region3.y",         &(runMdCfg.mdRegion[3].y),         GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1280, NULL},
    {"md.region3.width",     &(runMdCfg.mdRegion[3].width),     GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1920, NULL},
    {"md.region3.height",    &(runMdCfg.mdRegion[3].height),    GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1280, NULL},

    {"md.handle.intervalTime", &(runMdCfg.handle.intervalTime),   GK_CFG_DATA_TYPE_U32,  "3", 1, 0, 60, NULL},
    {"md.handle.is_email",     &(runMdCfg.handle.is_email),       GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1, NULL},
    {"md.handle.is_rec",       &(runMdCfg.handle.is_rec),         GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1, NULL},
    {"md.handle.recTime",      &(runMdCfg.handle.recTime),        GK_CFG_DATA_TYPE_U32,  "1", 1, 1, 1440, NULL},
    {"md.handle.recStreamNo",  &(runMdCfg.handle.recStreamNo),    GK_CFG_DATA_TYPE_U32,  "1", 1, 0, 3, NULL},
   
    {"md.handle.is_snap",            &(runMdCfg.handle.is_snap),           GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1, NULL},
    {"md.handle.isSnapUploadToFtp",  &(runMdCfg.handle.isSnapUploadToFtp), GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1, NULL},
    {"md.handle.isSnapUploadToWeb",  &(runMdCfg.handle.isSnapUploadToWeb), GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1, NULL},
    {"md.handle.isSnapUploadToCms",  &(runMdCfg.handle.isSnapUploadToCms), GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1, NULL},
    {"md.handle.isSnapSaveToSd",     &(runMdCfg.handle.isSnapSaveToSd),    GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1, NULL},
    {"md.handle.snapNum",            &(runMdCfg.handle.snapNum),           GK_CFG_DATA_TYPE_U32,  "1", 1, 1, 100, NULL},
    {"md.handle.interval",           &(runMdCfg.handle.interval),          GK_CFG_DATA_TYPE_U32,  "1", 1, 1, 10000, NULL},

    {"md.handle.is_alarmout",   &(runMdCfg.handle.is_alarmout),    GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1, NULL},
    {"md.handle.duration",      &(runMdCfg.handle.duration),       GK_CFG_DATA_TYPE_U32,  "1", 1, 1, 60, NULL},
    {"md.handle.is_beep",       &(runMdCfg.handle.is_beep),        GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1, NULL},
    {"md.handle.beepTime",      &(runMdCfg.handle.beepTime),       GK_CFG_DATA_TYPE_U32,  "3", 1, 1, 600, NULL},
    {"md.handle.is_ptz",        &(runMdCfg.handle.is_ptz),         GK_CFG_DATA_TYPE_U8,   "1", 1, 0, 1, NULL},
    {"md.handle.ptzLink.type",  &(runMdCfg.handle.ptzLink.type),   GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 3, NULL},
    {"md.handle.ptzLink.value", &(runMdCfg.handle.ptzLink.value),  GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 100, NULL},
    {"md.handle.is_pushMsg",    &(runMdCfg.handle.is_pushMsg),     GK_CFG_DATA_TYPE_U8,   "1", 1, 0, 1, NULL},

    {"md.schedule_mode",     &(runMdCfg.schedule_mode),         GK_CFG_DATA_TYPE_S32,   "1", 1, 0, 3, NULL},
    {"md.scheduleTime",      &(runMdCfg.scheduleTime[0][0]),    GK_CFG_DATA_TYPE_STIME, "0", 1, 0, 0, NULL},
    {"md.scheduleSlice",     &(runMdCfg.scheduleSlice[0][0]),   GK_CFG_DATA_TYPE_SLICE, "4294967295", 1, 0, 0, NULL},

    {"md.screenShotStatus",          &(runMdCfg.mojingMdCfg.screenShotStatus),          GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL},
    {"md.smartStatus",               &(runMdCfg.mojingMdCfg.smartStatus),               GK_CFG_DATA_TYPE_S8,   "-1", 1, -1, 2, NULL},
    {"md.smartType",                 &(runMdCfg.mojingMdCfg.smartType),                 GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 3, NULL},
    {"md.defenceStatus",             &(runMdCfg.mojingMdCfg.defenceStatus),             GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 2, NULL},

    {"md.DefenceWeek0",              &(runMdCfg.mojingMdCfg.defenceWeek[0]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.DefenceWeek1",              &(runMdCfg.mojingMdCfg.defenceWeek[1]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.DefenceWeek2",              &(runMdCfg.mojingMdCfg.defenceWeek[2]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.DefenceWeek3",              &(runMdCfg.mojingMdCfg.defenceWeek[3]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.DefenceWeek4",              &(runMdCfg.mojingMdCfg.defenceWeek[4]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.DefenceWeek5",              &(runMdCfg.mojingMdCfg.defenceWeek[5]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.DefenceWeek6",              &(runMdCfg.mojingMdCfg.defenceWeek[6]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},

    {"md.AreaEventStatus",               &(runMdCfg.mojingMdCfg.areaEventStatus),          GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 3, NULL},
    {"md.AreaSensitive",                 &(runMdCfg.mojingMdCfg.areaSensitive),                 GK_CFG_DATA_TYPE_S8,   "50", 1, 1, 100, NULL},
    {"md.AreaActition",                  &(runMdCfg.mojingMdCfg.areaAction),                    GK_CFG_DATA_TYPE_S8,   "1", 1,  1, 2, NULL},
    {"md.AreaShowArea",                  &(runMdCfg.mojingMdCfg.areaShowArea),                    GK_CFG_DATA_TYPE_S8,   "1", 1,  1, 2, NULL},
    {"md.AreaScreenShotStatus",          &(runMdCfg.mojingMdCfg.areaScreenShotStatus),          GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL},
    {"md.AreaSmartStatus",               &(runMdCfg.mojingMdCfg.areaSmartStatus),               GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 2, NULL},
    {"md.AreaSmartType",                 &(runMdCfg.mojingMdCfg.areaSmartType),                 GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 3, NULL},
    {"md.AreaPresetPosition",            &(runMdCfg.mojingMdCfg.areaPresetPosition),            GK_CFG_DATA_TYPE_S8,   "0", 1, 0, 100, NULL},
    {"md.AreaDefenceStatus",             &(runMdCfg.mojingMdCfg.areaDefenceStatus),             GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 2, NULL},
    {"md.AreaGroupResolution",           &(runMdCfg.mojingMdCfg.areaGroupResolution),            GK_CFG_DATA_TYPE_STRING,   "1920*1080", 1, 1, 12, NULL},
    {"md.AreaGroupPoints",               &(runMdCfg.mojingMdCfg.areaGroupPoints),               GK_CFG_DATA_TYPE_STRING,   "0,0:1920,0:1920,1080:0,1080", 1, 1, 128, NULL},
    {"md.AreaDefenceWeek0",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[0]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek1",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[1]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek2",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[2]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek3",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[3]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek4",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[4]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek5",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[5]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek6",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[6]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    
    {"md.handle.humanDetection.enable",             &(runMdCfg.handle.humanDetection.enable), GK_CFG_DATA_TYPE_U8, "1", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect0.enable",       &(runMdCfg.handle.humanDetection.rect[0].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect0.x",            &(runMdCfg.handle.humanDetection.rect[0].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.humanDetection.rect0.y",            &(runMdCfg.handle.humanDetection.rect[0].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.humanDetection.rect0.width",        &(runMdCfg.handle.humanDetection.rect[0].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.humanDetection.rect0.height",       &(runMdCfg.handle.humanDetection.rect[0].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.humanDetection.rect0.sensitivity",  &(runMdCfg.handle.humanDetection.rect[0].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 1, 100, NULL},
    {"md.handle.humanDetection.rect1.enable",       &(runMdCfg.handle.humanDetection.rect[1].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect1.x",            &(runMdCfg.handle.humanDetection.rect[1].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.humanDetection.rect1.y",            &(runMdCfg.handle.humanDetection.rect[1].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.humanDetection.rect1.width",        &(runMdCfg.handle.humanDetection.rect[1].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.humanDetection.rect1.height",       &(runMdCfg.handle.humanDetection.rect[1].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.humanDetection.rect1.sensitivity",  &(runMdCfg.handle.humanDetection.rect[1].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 1, 100, NULL},
    {"md.handle.humanDetection.rect2.enable",       &(runMdCfg.handle.humanDetection.rect[2].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect2.x",            &(runMdCfg.handle.humanDetection.rect[2].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.humanDetection.rect2.y",            &(runMdCfg.handle.humanDetection.rect[2].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.humanDetection.rect2.width",        &(runMdCfg.handle.humanDetection.rect[2].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.humanDetection.rect2.height",       &(runMdCfg.handle.humanDetection.rect[2].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.humanDetection.rect2.sensitivity",  &(runMdCfg.handle.humanDetection.rect[2].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 1, 100, NULL},
    {"md.handle.humanDetection.rect3.enable",       &(runMdCfg.handle.humanDetection.rect[3].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect3.x",            &(runMdCfg.handle.humanDetection.rect[3].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.humanDetection.rect3.y",            &(runMdCfg.handle.humanDetection.rect[3].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.humanDetection.rect3.width",        &(runMdCfg.handle.humanDetection.rect[3].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.humanDetection.rect3.height",       &(runMdCfg.handle.humanDetection.rect[3].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.humanDetection.rect3.sensitivity",  &(runMdCfg.handle.humanDetection.rect[3].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 1, 100, NULL},

    {"md.handle.motionTracing.enable",             &(runMdCfg.handle.motionTracing.enable), GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect0.enable",       &(runMdCfg.handle.motionTracing.rect[0].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect0.x",            &(runMdCfg.handle.motionTracing.rect[0].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.motionTracing.rect0.y",            &(runMdCfg.handle.motionTracing.rect[0].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.motionTracing.rect0.width",        &(runMdCfg.handle.motionTracing.rect[0].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.motionTracing.rect0.height",       &(runMdCfg.handle.motionTracing.rect[0].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.motionTracing.rect1.enable",       &(runMdCfg.handle.motionTracing.rect[1].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect1.x",            &(runMdCfg.handle.motionTracing.rect[1].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.motionTracing.rect1.y",            &(runMdCfg.handle.motionTracing.rect[1].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.motionTracing.rect1.width",        &(runMdCfg.handle.motionTracing.rect[1].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.motionTracing.rect1.height",       &(runMdCfg.handle.motionTracing.rect[1].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.motionTracing.rect2.enable",       &(runMdCfg.handle.motionTracing.rect[2].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect2.x",            &(runMdCfg.handle.motionTracing.rect[2].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.motionTracing.rect2.y",            &(runMdCfg.handle.motionTracing.rect[2].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.motionTracing.rect2.width",        &(runMdCfg.handle.motionTracing.rect[2].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.motionTracing.rect2.height",       &(runMdCfg.handle.motionTracing.rect[2].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.motionTracing.rect3.enable",       &(runMdCfg.handle.motionTracing.rect[3].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect3.x",            &(runMdCfg.handle.motionTracing.rect[3].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.motionTracing.rect3.y",            &(runMdCfg.handle.motionTracing.rect[3].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.motionTracing.rect3.width",        &(runMdCfg.handle.motionTracing.rect[3].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.motionTracing.rect3.height",       &(runMdCfg.handle.motionTracing.rect[3].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.motionTracing.retPoint",           &(runMdCfg.handle.motionTracing.retPoint), GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.retTime",            &(runMdCfg.handle.motionTracing.retTime),  GK_CFG_DATA_TYPE_U32, "60", 1, 1, 300, NULL},

    {"md.handle.transboundaryDetection.enable",            &(runMdCfg.handle.transboundaryDetection.enable), GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line0.enable",      &(runMdCfg.handle.transboundaryDetection.line[0].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line0.startX",      &(runMdCfg.handle.transboundaryDetection.line[0].startX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.transboundaryDetection.line0.startY",      &(runMdCfg.handle.transboundaryDetection.line[0].startY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.transboundaryDetection.line0.endX",        &(runMdCfg.handle.transboundaryDetection.line[0].endX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.transboundaryDetection.line0.endY",        &(runMdCfg.handle.transboundaryDetection.line[0].endY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.transboundaryDetection.line0.direction",   &(runMdCfg.handle.transboundaryDetection.line[0].direction),  GK_CFG_DATA_TYPE_U32, "2", 1, 0, 2, NULL},
    {"md.handle.transboundaryDetection.line1.enable",      &(runMdCfg.handle.transboundaryDetection.line[1].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line1.startX",      &(runMdCfg.handle.transboundaryDetection.line[1].startX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.transboundaryDetection.line1.startY",      &(runMdCfg.handle.transboundaryDetection.line[1].startY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.transboundaryDetection.line1.endX",        &(runMdCfg.handle.transboundaryDetection.line[1].endX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.transboundaryDetection.line1.endY",        &(runMdCfg.handle.transboundaryDetection.line[1].endY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.transboundaryDetection.line1.direction",   &(runMdCfg.handle.transboundaryDetection.line[1].direction),  GK_CFG_DATA_TYPE_U32, "2", 1, 0, 2, NULL},
    {"md.handle.transboundaryDetection.line2.enable",      &(runMdCfg.handle.transboundaryDetection.line[2].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line2.startX",      &(runMdCfg.handle.transboundaryDetection.line[2].startX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.transboundaryDetection.line2.startY",      &(runMdCfg.handle.transboundaryDetection.line[2].startY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.transboundaryDetection.line2.endX",        &(runMdCfg.handle.transboundaryDetection.line[2].endX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.transboundaryDetection.line2.endY",        &(runMdCfg.handle.transboundaryDetection.line[2].endY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.transboundaryDetection.line2.direction",   &(runMdCfg.handle.transboundaryDetection.line[2].direction),  GK_CFG_DATA_TYPE_U32, "2", 1, 0, 2, NULL},
    {"md.handle.transboundaryDetection.line3.enable",      &(runMdCfg.handle.transboundaryDetection.line[3].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line3.startX",      &(runMdCfg.handle.transboundaryDetection.line[3].startX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.transboundaryDetection.line3.startY",      &(runMdCfg.handle.transboundaryDetection.line[3].startY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.transboundaryDetection.line3.endX",        &(runMdCfg.handle.transboundaryDetection.line[3].endX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.transboundaryDetection.line3.endY",        &(runMdCfg.handle.transboundaryDetection.line[3].endY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.transboundaryDetection.line3.direction",   &(runMdCfg.handle.transboundaryDetection.line[3].direction),  GK_CFG_DATA_TYPE_U32, "2", 1, 0, 2, NULL},
    {"md.handle.transboundaryDetection.schedule_mode",     &(runMdCfg.handle.transboundaryDetection.schedule_mode),         GK_CFG_DATA_TYPE_S32,   "1", 1, 0, 3, NULL},
    {"md.handle.transboundaryDetection.scheduleTime",      &(runMdCfg.handle.transboundaryDetection.scheduleTime[0][0]),    GK_CFG_DATA_TYPE_STIME, "0", 1, 0, 0, NULL},
    {"md.handle.transboundaryDetection.scheduleSlice",     &(runMdCfg.handle.transboundaryDetection.scheduleSlice[0][0]),   GK_CFG_DATA_TYPE_SLICE, "4294967295", 1, 0, 0, NULL},

    {"md.handle.regionIntrusion.enable",                   &(runMdCfg.handle.regionIntrusion.enable), GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect0.enable",             &(runMdCfg.handle.motionTracing.rect[0].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect0.x",                  &(runMdCfg.handle.motionTracing.rect[0].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.regionIntrusion.rect0.y",                  &(runMdCfg.handle.motionTracing.rect[0].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.regionIntrusion.rect0.width",              &(runMdCfg.handle.motionTracing.rect[0].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.regionIntrusion.rect0.height",             &(runMdCfg.handle.motionTracing.rect[0].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.regionIntrusion.rect0.sensitivity",        &(runMdCfg.handle.regionIntrusion.rect[0].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 1, 100, NULL},
    {"md.handle.regionIntrusion.rect0.regionTime",         &(runMdCfg.handle.regionIntrusion.rect[0].regionTime),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 10, NULL},
    {"md.handle.regionIntrusion.rect0.regionRatio",        &(runMdCfg.handle.regionIntrusion.rect[0].regionRatio),  GK_CFG_DATA_TYPE_U8, "1", 1, 1, 100, NULL},
    {"md.handle.regionIntrusion.rect1.enable",             &(runMdCfg.handle.motionTracing.rect[1].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect1.x",                  &(runMdCfg.handle.motionTracing.rect[1].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.regionIntrusion.rect1.y",                  &(runMdCfg.handle.motionTracing.rect[1].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.regionIntrusion.rect1.width",              &(runMdCfg.handle.motionTracing.rect[1].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.regionIntrusion.rect1.height",             &(runMdCfg.handle.motionTracing.rect[1].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.regionIntrusion.rect1.sensitivity",        &(runMdCfg.handle.regionIntrusion.rect[1].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 1, 100, NULL},
    {"md.handle.regionIntrusion.rect1.regionTime",         &(runMdCfg.handle.regionIntrusion.rect[1].regionTime),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 10, NULL},
    {"md.handle.regionIntrusion.rect1.regionRatio",        &(runMdCfg.handle.regionIntrusion.rect[1].regionRatio),  GK_CFG_DATA_TYPE_U8, "1", 1, 1, 100, NULL},
    {"md.handle.regionIntrusion.rect2.enable",             &(runMdCfg.handle.motionTracing.rect[2].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect2.x",                  &(runMdCfg.handle.motionTracing.rect[2].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.regionIntrusion.rect2.y",                  &(runMdCfg.handle.motionTracing.rect[2].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.regionIntrusion.rect2.width",              &(runMdCfg.handle.motionTracing.rect[2].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.regionIntrusion.rect2.height",             &(runMdCfg.handle.motionTracing.rect[2].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.regionIntrusion.rect2.sensitivity",        &(runMdCfg.handle.regionIntrusion.rect[2].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 1, 100, NULL},
    {"md.handle.regionIntrusion.rect2.regionTime",         &(runMdCfg.handle.regionIntrusion.rect[2].regionTime),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 10, NULL},
    {"md.handle.regionIntrusion.rect2.regionRatio",        &(runMdCfg.handle.regionIntrusion.rect[2].regionRatio),  GK_CFG_DATA_TYPE_U8, "1", 1, 1, 100, NULL},
    {"md.handle.regionIntrusion.rect3.enable",             &(runMdCfg.handle.motionTracing.rect[3].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect3.x",                  &(runMdCfg.handle.motionTracing.rect[3].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.regionIntrusion.rect3.y",                  &(runMdCfg.handle.motionTracing.rect[3].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.regionIntrusion.rect3.width",              &(runMdCfg.handle.motionTracing.rect[3].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 2560, NULL},
    {"md.handle.regionIntrusion.rect3.height",             &(runMdCfg.handle.motionTracing.rect[3].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 1440, NULL},
    {"md.handle.regionIntrusion.rect3.sensitivity",        &(runMdCfg.handle.regionIntrusion.rect[3].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 1, 100, NULL},
    {"md.handle.regionIntrusion.rect3.regionTime",         &(runMdCfg.handle.regionIntrusion.rect[3].regionTime),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 10, NULL},
    {"md.handle.regionIntrusion.rect3.regionRatio",        &(runMdCfg.handle.regionIntrusion.rect[3].regionRatio),  GK_CFG_DATA_TYPE_U8, "1", 1, 1, 100, NULL},
    {"md.handle.regionIntrusion.schedule_mode",            &(runMdCfg.handle.regionIntrusion.schedule_mode),         GK_CFG_DATA_TYPE_S32,   "1", 1, 0, 3, NULL},
    {"md.handle.regionIntrusion.scheduleTime",             &(runMdCfg.handle.regionIntrusion.scheduleTime[0][0]),    GK_CFG_DATA_TYPE_STIME, "0", 1, 0, 0, NULL},
    {"md.handle.regionIntrusion.scheduleSlice",            &(runMdCfg.handle.regionIntrusion.scheduleSlice[0][0]),   GK_CFG_DATA_TYPE_SLICE, "4294967295", 1, 0, 0, NULL},

    {NULL,},

};
#else
GK_CFG_MAP mdMap[] = {
    {"md.channel",           &(runMdCfg.channel),               },
    {"md.enable",            &(runMdCfg.enable),                },
    {"md.sensitive",         &(runMdCfg.sensitive),             },
    {"md.compensation",      &(runMdCfg.compensation),          },
    {"md.detectionType",     &(runMdCfg.detectionType),         },

    {"md.grid.row",          &(runMdCfg.mdGrid.row),            },
    {"md.grid.column",       &(runMdCfg.mdGrid.column),         },
    {"md.grid.granularity",  &(runMdCfg.mdGrid.granularity),    },

    {"md.region0.enable",    &(runMdCfg.mdRegion[0].enable),    },
    {"md.region0.x",         &(runMdCfg.mdRegion[0].x),         },
    {"md.region0.y",         &(runMdCfg.mdRegion[0].y),         },
    {"md.region0.width",     &(runMdCfg.mdRegion[0].width),     },
    {"md.region0.height",    &(runMdCfg.mdRegion[0].height),    },

    {"md.region1.enable",    &(runMdCfg.mdRegion[1].enable),    },
    {"md.region1.x",         &(runMdCfg.mdRegion[1].x),         },
    {"md.region1.y",         &(runMdCfg.mdRegion[1].y),         },
    {"md.region1.width",     &(runMdCfg.mdRegion[1].width),     },
    {"md.region1.height",    &(runMdCfg.mdRegion[1].height),    },

    {"md.region2.enable",    &(runMdCfg.mdRegion[2].enable),    },
    {"md.region2.x",         &(runMdCfg.mdRegion[2].x),         },
    {"md.region2.y",         &(runMdCfg.mdRegion[2].y),         },
    {"md.region2.width",     &(runMdCfg.mdRegion[2].width),     },
    {"md.region2.height",    &(runMdCfg.mdRegion[2].height),    },

    {"md.region3.enable",    &(runMdCfg.mdRegion[3].enable),    },
    {"md.region3.x",         &(runMdCfg.mdRegion[3].x),         },
    {"md.region3.y",         &(runMdCfg.mdRegion[3].y),         },
    {"md.region3.width",     &(runMdCfg.mdRegion[3].width),     },
    {"md.region3.height",    &(runMdCfg.mdRegion[3].height),    },

    {"md.handle.intervalTime", &(runMdCfg.handle.intervalTime),   },
    {"md.handle.is_email",     &(runMdCfg.handle.is_email),       },
    {"md.handle.is_rec",       &(runMdCfg.handle.is_rec),         },
    {"md.handle.recTime",      &(runMdCfg.handle.recTime),        },
    {"md.handle.recStreamNo",  &(runMdCfg.handle.recStreamNo),    },

    {"md.handle.is_snap",            &(runMdCfg.handle.is_snap),           },
    {"md.handle.isSnapUploadToFtp",  &(runMdCfg.handle.isSnapUploadToFtp), },
    {"md.handle.isSnapUploadToWeb",  &(runMdCfg.handle.isSnapUploadToWeb), },
    {"md.handle.isSnapUploadToCms",  &(runMdCfg.handle.isSnapUploadToCms), },
    {"md.handle.isSnapSaveToSd",     &(runMdCfg.handle.isSnapSaveToSd),    },
    {"md.handle.snapNum",            &(runMdCfg.handle.snapNum),           },
    {"md.handle.interval",           &(runMdCfg.handle.interval),          },

    {"md.handle.is_alarmout",        &(runMdCfg.handle.is_alarmout),       },
    {"md.handle.duration",           &(runMdCfg.handle.duration),          },
    {"md.handle.is_beep",            &(runMdCfg.handle.is_beep),           },
    {"md.handle.beepTime",           &(runMdCfg.handle.beepTime),          },
    {"md.handle.is_ptz",             &(runMdCfg.handle.is_ptz),            },
    {"md.handle.ptzLink.type",       &(runMdCfg.handle.ptzLink.type),      },
    {"md.handle.ptzLink.value",      &(runMdCfg.handle.ptzLink.value),     },
    //{"md.handle.is_pushMsg",         &(runMdCfg.handle.is_pushMsg),     GK_CFG_DATA_TYPE_U8,   "1", 1, 0, 1, NULL},
    {"md.handle.is_pushMsg",         &(runMdCfg.handle.is_pushMsg),        },
    
	/*used by CUS_HC_GK7202_GC2053_V10 board */
    {"md.handle.alarmAudioType",     &(runMdCfg.handle.alarmAudioType),    GK_CFG_DATA_TYPE_U8, "2", 1, 0, 10, NULL},
    {"md.handle.is_light0",          &(runMdCfg.handle.is_light[0]),       GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.is_light1",          &(runMdCfg.handle.is_light[1]),       GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.is_light2",          &(runMdCfg.handle.is_light[2]),       GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.is_light3",          &(runMdCfg.handle.is_light[3]),       GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.bCheckedFigure",     &(runMdCfg.handle.bCheckedFigure),    GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.colorModeTime",      &(runMdCfg.handle.colorModeTime),    GK_CFG_DATA_TYPE_U8, "30", 1, 0, 255, NULL},

    {"md.schedule_mode",             &(runMdCfg.schedule_mode),            },
    {"md.scheduleTime",              &(runMdCfg.scheduleTime[0][0]),       },
    {"md.scheduleSlice",             &(runMdCfg.scheduleSlice[0][0]),      },

    {"md.screenShotStatus",          &(runMdCfg.mojingMdCfg.screenShotStatus),          GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL},
    {"md.smartStatus",               &(runMdCfg.mojingMdCfg.smartStatus),               GK_CFG_DATA_TYPE_S8,   "-1", 1, -1, 2, NULL},
    {"md.smartType",                 &(runMdCfg.mojingMdCfg.smartType),                 GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 3, NULL},
    {"md.defenceStatus",             &(runMdCfg.mojingMdCfg.defenceStatus),             GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 2, NULL},

    {"md.DefenceWeek0",              &(runMdCfg.mojingMdCfg.defenceWeek[0]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.DefenceWeek1",              &(runMdCfg.mojingMdCfg.defenceWeek[1]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.DefenceWeek2",              &(runMdCfg.mojingMdCfg.defenceWeek[2]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.DefenceWeek3",              &(runMdCfg.mojingMdCfg.defenceWeek[3]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.DefenceWeek4",              &(runMdCfg.mojingMdCfg.defenceWeek[4]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.DefenceWeek5",              &(runMdCfg.mojingMdCfg.defenceWeek[5]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.DefenceWeek6",              &(runMdCfg.mojingMdCfg.defenceWeek[6]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},

    {"md.AreaEventStatus",               &(runMdCfg.mojingMdCfg.areaEventStatus),          GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 3, NULL},
    {"md.AreaSensitive",                 &(runMdCfg.mojingMdCfg.areaSensitive),                 GK_CFG_DATA_TYPE_S8,   "50", 1, 1, 100, NULL},
    {"md.AreaActition",                  &(runMdCfg.mojingMdCfg.areaAction),                    GK_CFG_DATA_TYPE_S8,   "1", 1,  1, 2, NULL},
    {"md.AreaShowArea",                  &(runMdCfg.mojingMdCfg.areaShowArea),                    GK_CFG_DATA_TYPE_S8,   "1", 1,  1, 2, NULL},
    {"md.AreaScreenShotStatus",          &(runMdCfg.mojingMdCfg.areaScreenShotStatus),          GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL},
    {"md.AreaSmartStatus",               &(runMdCfg.mojingMdCfg.areaSmartStatus),               GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 2, NULL},
    {"md.AreaSmartType",                 &(runMdCfg.mojingMdCfg.areaSmartType),                 GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 3, NULL},
    {"md.AreaPresetPosition",            &(runMdCfg.mojingMdCfg.areaPresetPosition),            GK_CFG_DATA_TYPE_S8,   "0", 1, 0, 100, NULL},
    {"md.AreaDefenceStatus",             &(runMdCfg.mojingMdCfg.areaDefenceStatus),             GK_CFG_DATA_TYPE_S8,   "1", 1, -1, 2, NULL},
    {"md.AreaGroupResolution",           &(runMdCfg.mojingMdCfg.areaGroupResolution),            GK_CFG_DATA_TYPE_STRING,   "1920*1080", 1, 1, 12, NULL},
    {"md.AreaGroupPoints",               &(runMdCfg.mojingMdCfg.areaGroupPoints),               GK_CFG_DATA_TYPE_STRING,   "0,0:1920,0:1920,1080:0,1080", 1, 1, 128, NULL},
    {"md.AreaDefenceWeek0",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[0]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek1",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[1]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek2",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[2]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek3",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[3]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek4",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[4]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek5",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[5]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},
    {"md.AreaDefenceWeek6",              &(runMdCfg.mojingMdCfg.areaDefenceWeek[6]),            GK_CFG_DATA_TYPE_STRING,   "00:00-24:00", 1, 1, 256, NULL},

    {"md.handle.humanDetection.enable",             &(runMdCfg.handle.humanDetection.enable),     GK_CFG_DATA_TYPE_U8, "1", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect0.enable",       &(runMdCfg.handle.humanDetection.rect[0].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect0.x",            &(runMdCfg.handle.humanDetection.rect[0].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect0.y",            &(runMdCfg.handle.humanDetection.rect[0].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect0.width",        &(runMdCfg.handle.humanDetection.rect[0].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect0.height",       &(runMdCfg.handle.humanDetection.rect[0].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect0.sensitivity",  &(runMdCfg.handle.humanDetection.rect[0].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 0, 100, NULL},
    {"md.handle.humanDetection.rect1.enable",       &(runMdCfg.handle.humanDetection.rect[1].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect1.x",            &(runMdCfg.handle.humanDetection.rect[1].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect1.y",            &(runMdCfg.handle.humanDetection.rect[1].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect1.width",        &(runMdCfg.handle.humanDetection.rect[1].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect1.height",       &(runMdCfg.handle.humanDetection.rect[1].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect1.sensitivity",  &(runMdCfg.handle.humanDetection.rect[1].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 0, 100, NULL},
    {"md.handle.humanDetection.rect2.enable",       &(runMdCfg.handle.humanDetection.rect[2].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect2.x",            &(runMdCfg.handle.humanDetection.rect[2].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect2.y",            &(runMdCfg.handle.humanDetection.rect[2].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect2.width",        &(runMdCfg.handle.humanDetection.rect[2].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect2.height",       &(runMdCfg.handle.humanDetection.rect[2].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect2.sensitivity",  &(runMdCfg.handle.humanDetection.rect[2].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 0, 100, NULL},
    {"md.handle.humanDetection.rect3.enable",       &(runMdCfg.handle.humanDetection.rect[3].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.humanDetection.rect3.x",            &(runMdCfg.handle.humanDetection.rect[3].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect3.y",            &(runMdCfg.handle.humanDetection.rect[3].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect3.width",        &(runMdCfg.handle.humanDetection.rect[3].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect3.height",       &(runMdCfg.handle.humanDetection.rect[3].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.humanDetection.rect3.sensitivity",  &(runMdCfg.handle.humanDetection.rect[3].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 0, 100, NULL},

    {"md.handle.motionTracing.enable",             &(runMdCfg.handle.motionTracing.enable), GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect0.enable",       &(runMdCfg.handle.motionTracing.rect[0].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect0.x",            &(runMdCfg.handle.motionTracing.rect[0].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect0.y",            &(runMdCfg.handle.motionTracing.rect[0].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect0.width",        &(runMdCfg.handle.motionTracing.rect[0].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect0.height",       &(runMdCfg.handle.motionTracing.rect[0].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect1.enable",       &(runMdCfg.handle.motionTracing.rect[1].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect1.x",            &(runMdCfg.handle.motionTracing.rect[1].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect1.y",            &(runMdCfg.handle.motionTracing.rect[1].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect1.width",        &(runMdCfg.handle.motionTracing.rect[1].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect1.height",       &(runMdCfg.handle.motionTracing.rect[1].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect2.enable",       &(runMdCfg.handle.motionTracing.rect[2].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect2.x",            &(runMdCfg.handle.motionTracing.rect[2].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect2.y",            &(runMdCfg.handle.motionTracing.rect[2].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect2.width",        &(runMdCfg.handle.motionTracing.rect[2].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect2.height",       &(runMdCfg.handle.motionTracing.rect[2].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect3.enable",       &(runMdCfg.handle.motionTracing.rect[3].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.rect3.x",            &(runMdCfg.handle.motionTracing.rect[3].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect3.y",            &(runMdCfg.handle.motionTracing.rect[3].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect3.width",        &(runMdCfg.handle.motionTracing.rect[3].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.rect3.height",       &(runMdCfg.handle.motionTracing.rect[3].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.motionTracing.retPoint",           &(runMdCfg.handle.motionTracing.retPoint), GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.motionTracing.retTime",            &(runMdCfg.handle.motionTracing.retTime),  GK_CFG_DATA_TYPE_U32, "60", 1, 0, 300, NULL},

    {"md.handle.transboundaryDetection.enable",            &(runMdCfg.handle.transboundaryDetection.enable), GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line0.enable",      &(runMdCfg.handle.transboundaryDetection.line[0].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line0.startX",      &(runMdCfg.handle.transboundaryDetection.line[0].startX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line0.startY",      &(runMdCfg.handle.transboundaryDetection.line[0].startY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line0.endX",        &(runMdCfg.handle.transboundaryDetection.line[0].endX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line0.endY",        &(runMdCfg.handle.transboundaryDetection.line[0].endY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line0.direction",   &(runMdCfg.handle.transboundaryDetection.line[0].direction),  GK_CFG_DATA_TYPE_U8, "2", 1, 0, 2, NULL},
    {"md.handle.transboundaryDetection.line1.enable",      &(runMdCfg.handle.transboundaryDetection.line[1].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line1.startX",      &(runMdCfg.handle.transboundaryDetection.line[1].startX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line1.startY",      &(runMdCfg.handle.transboundaryDetection.line[1].startY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line1.endX",        &(runMdCfg.handle.transboundaryDetection.line[1].endX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line1.endY",        &(runMdCfg.handle.transboundaryDetection.line[1].endY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line1.direction",   &(runMdCfg.handle.transboundaryDetection.line[1].direction),  GK_CFG_DATA_TYPE_U8, "2", 1, 0, 2, NULL},
    {"md.handle.transboundaryDetection.line2.enable",      &(runMdCfg.handle.transboundaryDetection.line[2].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line2.startX",      &(runMdCfg.handle.transboundaryDetection.line[2].startX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line2.startY",      &(runMdCfg.handle.transboundaryDetection.line[2].startY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line2.endX",        &(runMdCfg.handle.transboundaryDetection.line[2].endX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line2.endY",        &(runMdCfg.handle.transboundaryDetection.line[2].endY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line2.direction",   &(runMdCfg.handle.transboundaryDetection.line[2].direction),  GK_CFG_DATA_TYPE_U8, "2", 1, 0, 2, NULL},
    {"md.handle.transboundaryDetection.line3.enable",      &(runMdCfg.handle.transboundaryDetection.line[3].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.transboundaryDetection.line3.startX",      &(runMdCfg.handle.transboundaryDetection.line[3].startX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line3.startY",      &(runMdCfg.handle.transboundaryDetection.line[3].startY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line3.endX",        &(runMdCfg.handle.transboundaryDetection.line[3].endX),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line3.endY",        &(runMdCfg.handle.transboundaryDetection.line[3].endY),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.transboundaryDetection.line3.direction",   &(runMdCfg.handle.transboundaryDetection.line[3].direction),  GK_CFG_DATA_TYPE_U8, "2", 1, 0, 2, NULL},
    {"md.handle.transboundaryDetection.schedule_mode",     &(runMdCfg.handle.transboundaryDetection.schedule_mode),         GK_CFG_DATA_TYPE_S32, "1", 1, 0, 3, NULL},
    {"md.handle.transboundaryDetection.scheduleTime",      &(runMdCfg.handle.transboundaryDetection.scheduleTime[0][0]),    GK_CFG_DATA_TYPE_STIME, "0",   1, 0, 0,  NULL},
    {"md.handle.transboundaryDetection.scheduleSlice",     &(runMdCfg.handle.transboundaryDetection.scheduleSlice[0][0]),   GK_CFG_DATA_TYPE_SLICE, "4294967295", 1, 0, 0, NULL},

    {"md.handle.regionIntrusion.enable",                   &(runMdCfg.handle.regionIntrusion.enable), GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect0.enable",             &(runMdCfg.handle.regionIntrusion.rect[0].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect0.x",                  &(runMdCfg.handle.regionIntrusion.rect[0].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect0.y",                  &(runMdCfg.handle.regionIntrusion.rect[0].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect0.width",              &(runMdCfg.handle.regionIntrusion.rect[0].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect0.height",             &(runMdCfg.handle.regionIntrusion.rect[0].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect0.sensitivity",        &(runMdCfg.handle.regionIntrusion.rect[0].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 0, 100, NULL},
    {"md.handle.regionIntrusion.rect0.regionTime",         &(runMdCfg.handle.regionIntrusion.rect[0].regionTime),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 10, NULL},
    {"md.handle.regionIntrusion.rect0.regionRatio",        &(runMdCfg.handle.regionIntrusion.rect[0].regionRatio),  GK_CFG_DATA_TYPE_U8, "1", 1, 0, 100, NULL},
    {"md.handle.regionIntrusion.rect1.enable",             &(runMdCfg.handle.regionIntrusion.rect[1].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect1.x",                  &(runMdCfg.handle.regionIntrusion.rect[1].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect1.y",                  &(runMdCfg.handle.regionIntrusion.rect[1].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect1.width",              &(runMdCfg.handle.regionIntrusion.rect[1].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect1.height",             &(runMdCfg.handle.regionIntrusion.rect[1].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect1.sensitivity",        &(runMdCfg.handle.regionIntrusion.rect[1].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 0, 100, NULL},
    {"md.handle.regionIntrusion.rect1.regionTime",         &(runMdCfg.handle.regionIntrusion.rect[1].regionTime),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 10, NULL},
    {"md.handle.regionIntrusion.rect1.regionRatio",        &(runMdCfg.handle.regionIntrusion.rect[1].regionRatio),  GK_CFG_DATA_TYPE_U8, "1", 1, 0, 100, NULL},
    {"md.handle.regionIntrusion.rect2.enable",             &(runMdCfg.handle.regionIntrusion.rect[2].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect2.x",                  &(runMdCfg.handle.regionIntrusion.rect[2].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect2.y",                  &(runMdCfg.handle.regionIntrusion.rect[2].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect2.width",              &(runMdCfg.handle.regionIntrusion.rect[2].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect2.height",             &(runMdCfg.handle.regionIntrusion.rect[2].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect2.sensitivity",        &(runMdCfg.handle.regionIntrusion.rect[2].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 0, 100, NULL},
    {"md.handle.regionIntrusion.rect2.regionTime",         &(runMdCfg.handle.regionIntrusion.rect[2].regionTime),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 10, NULL},
    {"md.handle.regionIntrusion.rect2.regionRatio",        &(runMdCfg.handle.regionIntrusion.rect[2].regionRatio),  GK_CFG_DATA_TYPE_U8, "1", 1, 0, 100, NULL},
    {"md.handle.regionIntrusion.rect3.enable",             &(runMdCfg.handle.regionIntrusion.rect[3].enable),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"md.handle.regionIntrusion.rect3.x",                  &(runMdCfg.handle.regionIntrusion.rect[3].x),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect3.y",                  &(runMdCfg.handle.regionIntrusion.rect[3].y),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect3.width",              &(runMdCfg.handle.regionIntrusion.rect[3].width),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect3.height",             &(runMdCfg.handle.regionIntrusion.rect[3].height),  GK_CFG_DATA_TYPE_U32, "0", 1, 0, 65536, NULL},
    {"md.handle.regionIntrusion.rect3.sensitivity",        &(runMdCfg.handle.regionIntrusion.rect[3].sensitivity),  GK_CFG_DATA_TYPE_U8, "50", 1, 0, 100, NULL},
    {"md.handle.regionIntrusion.rect3.regionTime",         &(runMdCfg.handle.regionIntrusion.rect[3].regionTime),  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 10, NULL},
    {"md.handle.regionIntrusion.rect3.regionRatio",        &(runMdCfg.handle.regionIntrusion.rect[3].regionRatio),  GK_CFG_DATA_TYPE_U8, "1", 1, 0, 100, NULL},
    {"md.handle.regionIntrusion.schedule_mode",            &(runMdCfg.handle.regionIntrusion.schedule_mode),         GK_CFG_DATA_TYPE_S32, "1", 1, 0, 3, NULL},
    {"md.handle.regionIntrusion.scheduleTime",             &(runMdCfg.handle.regionIntrusion.scheduleTime[0][0]),    GK_CFG_DATA_TYPE_STIME, "0",   1, 0, 0,  NULL},
    {"md.handle.regionIntrusion.scheduleSlice",            &(runMdCfg.handle.regionIntrusion.scheduleSlice[0][0]),   GK_CFG_DATA_TYPE_SLICE, "4294967295", 1, 0, 0, NULL},
    {NULL,},

};
#endif


void MdCfgPrint()
{
    printf("********** Md *********\n");
    CfgPrintMap(mdMap);
    printf("********** Md *********\n\n");
}

int MdCfgSave()
{
    int ret = CfgSave(MD_CFG_FILE, "md", mdMap);
    if (ret != 0) {
        PRINT_ERR("CfgSave %s error.", MD_CFG_FILE);
        return -1;
    }

    return 0;
}

int MdCfgLoad()
{
    int ret = CfgLoad(MD_CFG_FILE, "md", mdMap);
    if (ret != 0) {
        PRINT_ERR("CfgLoad %s error.", MD_CFG_FILE);
        return -1;
    }

    return 0;
}

int MdCfgLoadDefValue()
{
    CfgLoadDefValue(mdMap);

    return 0;
}


