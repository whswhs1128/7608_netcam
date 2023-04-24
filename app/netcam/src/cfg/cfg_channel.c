/*!
*****************************************************************************
** FileName     : cfg_channel.c
**
** Description  : channel config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-1
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "cfg_channel.h"
#include "sdk_sys.h"
extern int copy_file(char *src_name, char *des_name);


extern char device_local_time[128];
extern char device_local_week;


GK_NET_CHANNEL_CFG runChannelCfg;

#if USE_DEFAULT_PARAM
GK_CFG_MAP osdMap[MAX_CHANNEL_NUM][MAX_CHANNEL_ITEM_NUM] = {
    {
        {"id",                     &(runChannelCfg.channelInfo[0].id),                     GK_CFG_DATA_TYPE_S32, "0", "rw", 0, 3, "ch0-ch3"},
        {"osdChannelName_enable",  &(runChannelCfg.channelInfo[0].osdChannelName.enable),  GK_CFG_DATA_TYPE_S32,    "1",   "rw", 0, 1,               NULL},
        {"osdChannelName_text",    &(runChannelCfg.channelInfo[0].osdChannelName.text),    GK_CFG_DATA_TYPE_STRING, "IPC", "rw", 1, MAX_STR_LEN_128, NULL},
        {"osdChannelName_x",       &(runChannelCfg.channelInfo[0].osdChannelName.x),       GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelName_y",       &(runChannelCfg.channelInfo[0].osdChannelName.y),       GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdDatetime_enable",     &(runChannelCfg.channelInfo[0].osdDatetime.enable),     GK_CFG_DATA_TYPE_S32, "1",   "rw", 0, 1,    NULL},
        {"osdDatetime_dateFormat", &(runChannelCfg.channelInfo[0].osdDatetime.dateFormat), GK_CFG_DATA_TYPE_U8,  "0",   "rw", 1, 120,  "0:XXXX-XX-XX year mon date; 1:XX-XX-XXXX mon date year 2:XX-XX-XXXX date mon year"},
        {"osdDatetime_dateSprtr",  &(runChannelCfg.channelInfo[0].osdDatetime.dateSprtr),  GK_CFG_DATA_TYPE_U8,  "0",   "rw", 0, 3,    "0 :, 1 -, 2 //, 3 ."},
        {"osdDatetime_timeFmt",    &(runChannelCfg.channelInfo[0].osdDatetime.timeFmt),    GK_CFG_DATA_TYPE_U8,  "0",   "rw", 0, 1,    "0 - 24,1 - 12"},
        {"osdDatetime_x",          &(runChannelCfg.channelInfo[0].osdDatetime.x),          GK_CFG_DATA_TYPE_FLOAT, "0.2", "rw", 0, 1.0, NULL},
        {"osdDatetime_y",          &(runChannelCfg.channelInfo[0].osdDatetime.y),          GK_CFG_DATA_TYPE_FLOAT, "0",  "rw", 0, 1.0, NULL},
	    {"osdDatetime_displayWeek", &(runChannelCfg.channelInfo[0].osdDatetime.displayWeek),    GK_CFG_DATA_TYPE_U8,  "1",	"rw", 0, 1, "0 - off,1 - on"},

        {"osdChannelID_enable", &(runChannelCfg.channelInfo[0].osdChannelID.enable), GK_CFG_DATA_TYPE_S32,    "0",   "rw", 0, 1,               NULL},
        {"osdChannelID_text",   &(runChannelCfg.channelInfo[0].osdChannelID.text),   GK_CFG_DATA_TYPE_STRING, "ID", "rw", 1, MAX_STR_LEN_128, NULL},
        {"osdChannelID_x",      &(runChannelCfg.channelInfo[0].osdChannelID.x),      GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelID_y",      &(runChannelCfg.channelInfo[0].osdChannelID.y),      GK_CFG_DATA_TYPE_FLOAT,    "0.8",  "rw", 0, 1.0,            NULL},

        {NULL,},
    },
    {
        {"id",                     &(runChannelCfg.channelInfo[1].id),                     GK_CFG_DATA_TYPE_S32, "1", "rw", 0, 3, "ch0-ch3"},
        {"osdChannelName_enable",  &(runChannelCfg.channelInfo[1].osdChannelName.enable),  GK_CFG_DATA_TYPE_S32,    "1",   "rw", 0, 1,               NULL},
        {"osdChannelName_text",    &(runChannelCfg.channelInfo[1].osdChannelName.text),    GK_CFG_DATA_TYPE_STRING, "IPC", "rw", 1, MAX_STR_LEN_128, NULL},
        {"osdChannelName_x",       &(runChannelCfg.channelInfo[1].osdChannelName.x),       GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelName_y",       &(runChannelCfg.channelInfo[1].osdChannelName.y),       GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdDatetime_enable",     &(runChannelCfg.channelInfo[1].osdDatetime.enable),     GK_CFG_DATA_TYPE_S32, "1",   "rw", 0, 1,    NULL},
        {"osdDatetime_dateFormat", &(runChannelCfg.channelInfo[1].osdDatetime.dateFormat), GK_CFG_DATA_TYPE_U8,  "0",   "rw", 1, 120,  "0:XXXX-XX-XX year mon date; 1:XX-XX-XXXX mon date year 2:XX-XX-XXXX date mon year"},
        {"osdDatetime_dateSprtr",  &(runChannelCfg.channelInfo[1].osdDatetime.dateSprtr),  GK_CFG_DATA_TYPE_U8,  "0",   "rw", 0, 3,    "0 :, 1 -, 2 //, 3 ."},
        {"osdDatetime_timeFmt",    &(runChannelCfg.channelInfo[1].osdDatetime.timeFmt),    GK_CFG_DATA_TYPE_U8,  "0",   "rw", 0, 1,    "0 - 24,1 - 12"},
        {"osdDatetime_x",          &(runChannelCfg.channelInfo[1].osdDatetime.x),          GK_CFG_DATA_TYPE_FLOAT, "0.2", "rw", 0, 1.0, NULL},
        {"osdDatetime_y",          &(runChannelCfg.channelInfo[1].osdDatetime.y),          GK_CFG_DATA_TYPE_FLOAT, "0",  "rw", 0, 1.0, NULL},
        {"osdDatetime_displayWeek", &(runChannelCfg.channelInfo[1].osdDatetime.displayWeek),    GK_CFG_DATA_TYPE_U8,  "1",	"rw", 0, 1, "0 - off,1 - on"},
        {"osdChannelID_enable", &(runChannelCfg.channelInfo[1].osdChannelID.enable), GK_CFG_DATA_TYPE_S32,    "0",   "rw", 0, 1,               NULL},
        {"osdChannelID_text",   &(runChannelCfg.channelInfo[1].osdChannelID.text),   GK_CFG_DATA_TYPE_STRING, "ID", "rw", 1, MAX_STR_LEN_128, NULL},
        {"osdChannelID_x",      &(runChannelCfg.channelInfo[1].osdChannelID.x),      GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelID_y",      &(runChannelCfg.channelInfo[1].osdChannelID.y),      GK_CFG_DATA_TYPE_FLOAT,    "0.8",  "rw", 0, 1.0,            NULL},

        {NULL,},
    },
    {
        {"id",                     &(runChannelCfg.channelInfo[2].id),                     GK_CFG_DATA_TYPE_S32, "2", "rw", 0, 3, "ch0-ch3"},
        {"osdChannelName_enable",  &(runChannelCfg.channelInfo[2].osdChannelName.enable),  GK_CFG_DATA_TYPE_S32,    "1",   "rw", 0, 1,               NULL},
        {"osdChannelName_text",    &(runChannelCfg.channelInfo[2].osdChannelName.text),    GK_CFG_DATA_TYPE_STRING, "CH-2", "rw", 1, MAX_STR_LEN_128, NULL},
        {"osdChannelName_x",       &(runChannelCfg.channelInfo[2].osdChannelName.x),       GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelName_y",       &(runChannelCfg.channelInfo[2].osdChannelName.y),       GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdDatetime_enable",     &(runChannelCfg.channelInfo[2].osdDatetime.enable),     GK_CFG_DATA_TYPE_S32, "1",   "rw", 0, 1,    NULL},
        {"osdDatetime_dateFormat", &(runChannelCfg.channelInfo[2].osdDatetime.dateFormat), GK_CFG_DATA_TYPE_U8,  "0",   "rw", 1, 120,  "0:XXXX-XX-XX year mon date; 1:XX-XX-XXXX mon date year 2:XX-XX-XXXX date mon year"},
        {"osdDatetime_dateSprtr",  &(runChannelCfg.channelInfo[2].osdDatetime.dateSprtr),  GK_CFG_DATA_TYPE_U8,  "0",   "rw", 0, 3,    "0 :, 1 -, 2 //, 3 ."},
        {"osdDatetime_timeFmt",    &(runChannelCfg.channelInfo[2].osdDatetime.timeFmt),    GK_CFG_DATA_TYPE_U8,  "0",   "rw", 0, 1,    "0 - 24,1 - 12"},
        {"osdDatetime_x",          &(runChannelCfg.channelInfo[2].osdDatetime.x),          GK_CFG_DATA_TYPE_FLOAT, "0.2", "rw", 0, 1.0, NULL},
        {"osdDatetime_y",          &(runChannelCfg.channelInfo[2].osdDatetime.y),          GK_CFG_DATA_TYPE_FLOAT, "0",  "rw", 0, 1.0, NULL},
        {"osdDatetime_displayWeek", &(runChannelCfg.channelInfo[2].osdDatetime.displayWeek),    GK_CFG_DATA_TYPE_U8,  "0",	"rw", 0, 1, "0 - off，1 - on"},
        {"osdChannelID_enable", &(runChannelCfg.channelInfo[2].osdChannelID.enable), GK_CFG_DATA_TYPE_S32,    "1",   "rw", 0, 1,               NULL},
        {"osdChannelID_text",   &(runChannelCfg.channelInfo[2].osdChannelID.text),   GK_CFG_DATA_TYPE_STRING, "ID", "rw", 1, MAX_STR_LEN_128, NULL},
        {"osdChannelID_x",      &(runChannelCfg.channelInfo[2].osdChannelID.x),      GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelID_y",      &(runChannelCfg.channelInfo[2].osdChannelID.y),      GK_CFG_DATA_TYPE_FLOAT,    "0.8",  "rw", 0, 1.0,            NULL},
        {NULL,},
    },
    {
        {"id",                     &(runChannelCfg.channelInfo[3].id),                     GK_CFG_DATA_TYPE_S32, "3", "rw", 0, 3, "ch0-ch3"},
        {"osdChannelName_enable",  &(runChannelCfg.channelInfo[3].osdChannelName.enable),  GK_CFG_DATA_TYPE_S32,    "1",   "rw", 0, 1,               NULL},
        {"osdChannelName_text",    &(runChannelCfg.channelInfo[3].osdChannelName.text),    GK_CFG_DATA_TYPE_STRING, "CH-3", "rw", 1, MAX_STR_LEN_128, NULL},
        {"osdChannelName_x",       &(runChannelCfg.channelInfo[3].osdChannelName.x),       GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelName_y",       &(runChannelCfg.channelInfo[3].osdChannelName.y),       GK_CFG_DATA_TYPE_FLOAT,    "0.3",  "rw", 0, 1.0,            NULL},
        {"osdDatetime_enable",     &(runChannelCfg.channelInfo[3].osdDatetime.enable),     GK_CFG_DATA_TYPE_S32, "1",   "rw", 0, 1,    NULL},
        {"osdDatetime_dateFormat", &(runChannelCfg.channelInfo[3].osdDatetime.dateFormat), GK_CFG_DATA_TYPE_U8,  "0",   "rw", 1, 120,  "0:XXXX-XX-XX year mon date; 1:XX-XX-XXXX mon date year 2:XX-XX-XXXX date mon year"},
        {"osdDatetime_dateSprtr",  &(runChannelCfg.channelInfo[3].osdDatetime.dateSprtr),  GK_CFG_DATA_TYPE_U8,  "0",   "rw", 0, 3,    "0 :, 1 -, 2 //, 3 ."},
        {"osdDatetime_timeFmt",    &(runChannelCfg.channelInfo[3].osdDatetime.timeFmt),    GK_CFG_DATA_TYPE_U8,  "20",   "rw", 0, 1,    "0 - 24,1 - 12"},
        {"osdDatetime_x",          &(runChannelCfg.channelInfo[3].osdDatetime.x),          GK_CFG_DATA_TYPE_FLOAT, "0.2", "rw", 0, 1.0, NULL},
        {"osdDatetime_y",          &(runChannelCfg.channelInfo[3].osdDatetime.y),          GK_CFG_DATA_TYPE_FLOAT, "0",  "rw", 0, 1.0, NULL},
        {"osdDatetime_displayWeek", &(runChannelCfg.channelInfo[3].osdDatetime.displayWeek),    GK_CFG_DATA_TYPE_U8,  "1",	"rw", 0, 1, "0 - off,1 - on"},
        {"osdChannelID_enable", &(runChannelCfg.channelInfo[3].osdChannelID.enable), GK_CFG_DATA_TYPE_S32,    "1",   "rw", 0, 1,               NULL},
        {"osdChannelID_text",   &(runChannelCfg.channelInfo[3].osdChannelID.text),   GK_CFG_DATA_TYPE_STRING, "ID", "rw", 1, MAX_STR_LEN_128, NULL},
        {"osdChannelID_x",      &(runChannelCfg.channelInfo[3].osdChannelID.x),      GK_CFG_DATA_TYPE_FLOAT,    "0",  "rw", 0, 1.0,            NULL},
        {"osdChannelID_y",      &(runChannelCfg.channelInfo[3].osdChannelID.y),      GK_CFG_DATA_TYPE_FLOAT,    "0.8",  "rw", 0, 1.0,            NULL},
        {NULL,},
    },
};

GK_CFG_MAP shelterRectMap[MAX_CHANNEL_NUM][MAX_SHELTER_RECT_ITEM_NUM] = {
    {
        {"shelterRect_enable", &(runChannelCfg.shelterRect[0].enable), GK_CFG_DATA_TYPE_S32, 	"0", "rw", 0, 1,    NULL},
        {"shelterRect_x",      &(runChannelCfg.shelterRect[0].x),      GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_y",      &(runChannelCfg.shelterRect[0].y),      GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_width",  &(runChannelCfg.shelterRect[0].width),  GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_height", &(runChannelCfg.shelterRect[0].height), GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_color",  &(runChannelCfg.shelterRect[0].color),  GK_CFG_DATA_TYPE_U32, 	"0", "rw", 0, 0x7FFFFFFF,  NULL},
        {NULL,},
    },
    {
        {"shelterRect_enable", &(runChannelCfg.shelterRect[1].enable), GK_CFG_DATA_TYPE_S32, 	"0", "rw", 0, 1,    NULL},
        {"shelterRect_x",      &(runChannelCfg.shelterRect[1].x),      GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_y",      &(runChannelCfg.shelterRect[1].y),      GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_width",  &(runChannelCfg.shelterRect[1].width),  GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_height", &(runChannelCfg.shelterRect[1].height), GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_color",  &(runChannelCfg.shelterRect[1].color),  GK_CFG_DATA_TYPE_U32, 	"0", "rw", 0, 0x7FFFFFFF,  NULL},
        {NULL,},
    },
    {
        {"shelterRect_enable", &(runChannelCfg.shelterRect[2].enable), GK_CFG_DATA_TYPE_S32, 	"0", "rw", 0, 1,    NULL},
        {"shelterRect_x",      &(runChannelCfg.shelterRect[2].x),      GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_y",      &(runChannelCfg.shelterRect[2].y),      GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_width",  &(runChannelCfg.shelterRect[2].width),  GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_height", &(runChannelCfg.shelterRect[2].height), GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_color",  &(runChannelCfg.shelterRect[2].color),  GK_CFG_DATA_TYPE_U32, 	"0", "rw", 0, 0x7FFFFFFF,  NULL},

        {NULL,},
    },
    {
        {"shelterRect_enable", &(runChannelCfg.shelterRect[3].enable), GK_CFG_DATA_TYPE_S32, 	"0", "rw", 0, 1,    NULL},
        {"shelterRect_x",      &(runChannelCfg.shelterRect[3].x),      GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_y",      &(runChannelCfg.shelterRect[3].y),      GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_width",  &(runChannelCfg.shelterRect[3].width),  GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_height", &(runChannelCfg.shelterRect[3].height), GK_CFG_DATA_TYPE_FLOAT, 	"0", "rw", 0, 1.0, NULL},
        {"shelterRect_color",  &(runChannelCfg.shelterRect[3].color),  GK_CFG_DATA_TYPE_U32, 	"0", "rw", 0, 0x7FFFFFFF,  NULL},

        {NULL,},
    },
};


#else
GK_CFG_MAP osdMap[MAX_CHANNEL_NUM][MAX_CHANNEL_ITEM_NUM] = {
    {
        {"id",                          &(runChannelCfg.channelInfo[0].id),                          },
        {"osdChannelName_enable",       &(runChannelCfg.channelInfo[0].osdChannelName.enable),       },
        {"osdChannelName_text",         &(runChannelCfg.channelInfo[0].osdChannelName.text),         },
        {"osdChannelName_x",            &(runChannelCfg.channelInfo[0].osdChannelName.x),            },
        {"osdChannelName_y",            &(runChannelCfg.channelInfo[0].osdChannelName.y),            },
        {"osdDatetime_enable",          &(runChannelCfg.channelInfo[0].osdDatetime.enable),          },
        {"osdDatetime_dateFormat",      &(runChannelCfg.channelInfo[0].osdDatetime.dateFormat),      },
        {"osdDatetime_dateSprtr",       &(runChannelCfg.channelInfo[0].osdDatetime.dateSprtr),       },
        {"osdDatetime_timeFmt",         &(runChannelCfg.channelInfo[0].osdDatetime.timeFmt),         },
        {"osdDatetime_x",               &(runChannelCfg.channelInfo[0].osdDatetime.x),               },
        {"osdDatetime_y",               &(runChannelCfg.channelInfo[0].osdDatetime.y),               },
	    {"osdDatetime_displayWeek",     &(runChannelCfg.channelInfo[0].osdDatetime.displayWeek),     },

        {"osdChannelID_enable",         &(runChannelCfg.channelInfo[0].osdChannelID.enable),         },
        {"osdChannelID_text",           &(runChannelCfg.channelInfo[0].osdChannelID.text),           },
        {"osdChannelID_x",              &(runChannelCfg.channelInfo[0].osdChannelID.x),              },
        {"osdChannelID_y",              &(runChannelCfg.channelInfo[0].osdChannelID.y),              },

        {NULL,},
    },
    {
        {"id",                          &(runChannelCfg.channelInfo[1].id),                          },
        {"osdChannelName_enable",       &(runChannelCfg.channelInfo[1].osdChannelName.enable),       },
        {"osdChannelName_text",         &(runChannelCfg.channelInfo[1].osdChannelName.text),         },
        {"osdChannelName_x",            &(runChannelCfg.channelInfo[1].osdChannelName.x),            },
        {"osdChannelName_y",            &(runChannelCfg.channelInfo[1].osdChannelName.y),            },
        {"osdDatetime_enable",          &(runChannelCfg.channelInfo[1].osdDatetime.enable),          },
        {"osdDatetime_dateFormat",      &(runChannelCfg.channelInfo[1].osdDatetime.dateFormat),      },
        {"osdDatetime_dateSprtr",       &(runChannelCfg.channelInfo[1].osdDatetime.dateSprtr),       },
        {"osdDatetime_timeFmt",         &(runChannelCfg.channelInfo[1].osdDatetime.timeFmt),         },
        {"osdDatetime_x",               &(runChannelCfg.channelInfo[1].osdDatetime.x),               },
        {"osdDatetime_y",               &(runChannelCfg.channelInfo[1].osdDatetime.y),               },
	    {"osdDatetime_displayWeek",     &(runChannelCfg.channelInfo[1].osdDatetime.displayWeek),     },

        {"osdChannelID_enable",         &(runChannelCfg.channelInfo[1].osdChannelID.enable),         },
        {"osdChannelID_text",           &(runChannelCfg.channelInfo[1].osdChannelID.text),           },
        {"osdChannelID_x",              &(runChannelCfg.channelInfo[1].osdChannelID.x),              },
        {"osdChannelID_y",              &(runChannelCfg.channelInfo[1].osdChannelID.y),              },

        {NULL,},
    },
    {
        {"id",                          &(runChannelCfg.channelInfo[2].id),                          },
        {"osdChannelName_enable",       &(runChannelCfg.channelInfo[2].osdChannelName.enable),       },
        {"osdChannelName_text",         &(runChannelCfg.channelInfo[2].osdChannelName.text),         },
        {"osdChannelName_x",            &(runChannelCfg.channelInfo[2].osdChannelName.x),            },
        {"osdChannelName_y",            &(runChannelCfg.channelInfo[2].osdChannelName.y),            },
        {"osdDatetime_enable",          &(runChannelCfg.channelInfo[2].osdDatetime.enable),          },
        {"osdDatetime_dateFormat",      &(runChannelCfg.channelInfo[2].osdDatetime.dateFormat),      },
        {"osdDatetime_dateSprtr",       &(runChannelCfg.channelInfo[2].osdDatetime.dateSprtr),       },
        {"osdDatetime_timeFmt",         &(runChannelCfg.channelInfo[2].osdDatetime.timeFmt),         },
        {"osdDatetime_x",               &(runChannelCfg.channelInfo[2].osdDatetime.x),               },
        {"osdDatetime_y",               &(runChannelCfg.channelInfo[2].osdDatetime.y),               },
	    {"osdDatetime_displayWeek",     &(runChannelCfg.channelInfo[2].osdDatetime.displayWeek),     },

        {"osdChannelID_enable",         &(runChannelCfg.channelInfo[2].osdChannelID.enable),         },
        {"osdChannelID_text",           &(runChannelCfg.channelInfo[2].osdChannelID.text),           },
        {"osdChannelID_x",              &(runChannelCfg.channelInfo[2].osdChannelID.x),              },
        {"osdChannelID_y",              &(runChannelCfg.channelInfo[2].osdChannelID.y),              },

        {NULL,},
    },
    {
        {"id",                          &(runChannelCfg.channelInfo[3].id),                          },
        {"osdChannelName_enable",       &(runChannelCfg.channelInfo[3].osdChannelName.enable),       },
        {"osdChannelName_text",         &(runChannelCfg.channelInfo[3].osdChannelName.text),         },
        {"osdChannelName_x",            &(runChannelCfg.channelInfo[3].osdChannelName.x),            },
        {"osdChannelName_y",            &(runChannelCfg.channelInfo[3].osdChannelName.y),            },
        {"osdDatetime_enable",          &(runChannelCfg.channelInfo[3].osdDatetime.enable),          },
        {"osdDatetime_dateFormat",      &(runChannelCfg.channelInfo[3].osdDatetime.dateFormat),      },
        {"osdDatetime_dateSprtr",       &(runChannelCfg.channelInfo[3].osdDatetime.dateSprtr),       },
        {"osdDatetime_timeFmt",         &(runChannelCfg.channelInfo[3].osdDatetime.timeFmt),         },
        {"osdDatetime_x",               &(runChannelCfg.channelInfo[3].osdDatetime.x),               },
        {"osdDatetime_y",               &(runChannelCfg.channelInfo[3].osdDatetime.y),               },
	    {"osdDatetime_displayWeek",     &(runChannelCfg.channelInfo[3].osdDatetime.displayWeek),     },

        {"osdChannelID_enable",         &(runChannelCfg.channelInfo[3].osdChannelID.enable),         },
        {"osdChannelID_text",           &(runChannelCfg.channelInfo[3].osdChannelID.text),           },
        {"osdChannelID_x",              &(runChannelCfg.channelInfo[3].osdChannelID.x),              },
        {"osdChannelID_y",              &(runChannelCfg.channelInfo[3].osdChannelID.y),              },

        {NULL,},
    },
};

GK_CFG_MAP shelterRectMap[MAX_CHANNEL_NUM][MAX_SHELTER_RECT_ITEM_NUM] = {
    {
        {"shelterRect_enable", &(runChannelCfg.shelterRect[0].enable), },
        {"shelterRect_x",      &(runChannelCfg.shelterRect[0].x),      },
        {"shelterRect_y",      &(runChannelCfg.shelterRect[0].y),      },
        {"shelterRect_width",  &(runChannelCfg.shelterRect[0].width),  },
        {"shelterRect_height", &(runChannelCfg.shelterRect[0].height), },
        {"shelterRect_color",  &(runChannelCfg.shelterRect[0].color),  },
        {NULL,},
    },
    {
        {"shelterRect_enable", &(runChannelCfg.shelterRect[1].enable), },
        {"shelterRect_x",      &(runChannelCfg.shelterRect[1].x),      },
        {"shelterRect_y",      &(runChannelCfg.shelterRect[1].y),      },
        {"shelterRect_width",  &(runChannelCfg.shelterRect[1].width),  },
        {"shelterRect_height", &(runChannelCfg.shelterRect[1].height), },
        {"shelterRect_color",  &(runChannelCfg.shelterRect[1].color),  },
        {NULL,},
    },
    {
        {"shelterRect_enable", &(runChannelCfg.shelterRect[2].enable), },
        {"shelterRect_x",      &(runChannelCfg.shelterRect[2].x),      },
        {"shelterRect_y",      &(runChannelCfg.shelterRect[2].y),      },
        {"shelterRect_width",  &(runChannelCfg.shelterRect[2].width),  },
        {"shelterRect_height", &(runChannelCfg.shelterRect[2].height), },
        {"shelterRect_color",  &(runChannelCfg.shelterRect[2].color),  },

        {NULL,},
    },
    {
        {"shelterRect_enable", &(runChannelCfg.shelterRect[3].enable), },
        {"shelterRect_x",      &(runChannelCfg.shelterRect[3].x),      },
        {"shelterRect_y",      &(runChannelCfg.shelterRect[3].y),      },
        {"shelterRect_width",  &(runChannelCfg.shelterRect[3].width),  },
        {"shelterRect_height", &(runChannelCfg.shelterRect[3].height), },
        {"shelterRect_color",  &(runChannelCfg.shelterRect[3].color),  },

        {NULL,},
    },
};
#endif


void ChannelCfgPrint()
{
    int i;
    printf("********** Channel osd *********\n");
    for(i = 0; i < MAX_CHANNEL_NUM; i ++) {
        printf("Channel_%d osd:\n", i);
        CfgPrintMap(osdMap[i]);
        printf("\n");
    }
    printf("********** Channel osd *********\n\n");

    printf("********** shelter rect *********\n");
    for(i = 0; i < MAX_CHANNEL_NUM; i ++) {
        printf("shelter rect_%d:\n", i);
        CfgPrintMap(shelterRectMap[i]);
        printf("\n");
    }
    printf("********** shelter rect *********\n\n");
}

int ChannelCfgSave()
{
    int i, ret;
    cJSON *root, *array1, *array2, *item;
    char *out;

    root = cJSON_CreateObject();//创建项目
    array1 = cJSON_CreateArray();
    for (i = 0; i < MAX_CHANNEL_NUM; i ++) {
        item = CfgDataToCjsonByMap(osdMap[i]);
        cJSON_AddItemToObject(array1, "osd", item);
    }
    cJSON_AddItemToObject(root, "channel osd", array1);

	array2 = cJSON_CreateArray();
    for (i = 0; i < MAX_CHANNEL_NUM; i ++) {
        item = CfgDataToCjsonByMap(shelterRectMap[i]);
        cJSON_AddItemToObject(array2, "rect", item);
    }
    cJSON_AddItemToObject(root, "shelter rect", array2);

    out = cJSON_Print(root);

    ret = CfgWriteToFile(CHANNEL_CFG_FILE, out);
    if (ret != 0) {
        PRINT_ERR("CfgWriteToFile %s error.", CHANNEL_CFG_FILE);
        //return -1;
    }

    free(out);
    cJSON_Delete(root);

    return ret;
}

int ChannelCfgLoadDefValue()
{
    int i;
    for (i = 0; i < MAX_CHANNEL_NUM; i ++) {
        CfgLoadDefValue(osdMap[i]);
    }

    for (i = 0; i < MAX_CHANNEL_NUM; i ++) {
        CfgLoadDefValue(shelterRectMap[i]);
    }

    return 0;
}


int ChannelCfgLoad()
{
    int load_def_config = 0;
    char *data = NULL;

start_load:
    data = CfgReadFromFile(CHANNEL_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        PRINT_INFO("load %s error, so to load default cfg param.\n", CHANNEL_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        PRINT_INFO("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    // get channel osd
    cJSON *array = NULL;
    array = cJSON_GetObjectItem(json, "channel osd");
    if(!array){
        PRINT_ERR("get venc stream error\n");
        goto err1;
    }

	int arraySize = cJSON_GetArraySize(array);
	//PRINT_INFO("channel arraySize=%d\n", arraySize);

    int index;
    cJSON *arrayItem = NULL;
    for(index = 0; index < arraySize; index++) {
        arrayItem = cJSON_GetArrayItem(array, index);
        if(!arrayItem){
            PRINT_ERR("cJSON_GetArrayItem error\n");
            goto err1;
        }
        CfgCjsonToDataByMap(osdMap[index], arrayItem);
    }

    // get shelter rect
    cJSON *array2 = cJSON_GetObjectItem(json, "shelter rect");
    if(!array2){
        PRINT_ERR("get venc stream error\n");
        goto err1;
    }

	arraySize = cJSON_GetArraySize(array2);
	//PRINT_INFO("shelter rect arraySize=%d\n", arraySize);

    for(index = 0; index < arraySize; index++) {
        arrayItem = cJSON_GetArrayItem(array2, index);
        if(!arrayItem){
            PRINT_ERR("cJSON_GetArrayItem error\n");
            goto err1;
        }
        CfgCjsonToDataByMap(shelterRectMap[index], arrayItem);
    }

    cJSON_Delete(json);
    free(data);
    return 0;

err1:
    cJSON_Delete(json);
    free(data);
err:
    if(!load_def_config)
    {
        load_def_config = 1;
        char src_name[64], des_name[64];
        memset(src_name, 0, sizeof(src_name));
        snprintf(src_name,sizeof(src_name), "%s%s", DEFCFG_DIR, CHANNEL_CFG_FILE);
        memset(des_name, 0, sizeof(des_name));
        snprintf(des_name,sizeof(des_name), "%s%s", CFG_DIR, CHANNEL_CFG_FILE);
        if(!copy_file(src_name, des_name))
            goto start_load;
    }
    ChannelCfgLoadDefValue();
    ChannelCfgSave();
    return 0;

}

char* ChannelCfgLoadOverlayJson( int channelId)
{
    cJSON *item;
    char *out;
    item = CfgDataToCjsonByMap(osdMap[channelId]);
    if(item == 0)
    {
        return 0;
    }
    cJSON_AddItemToObject(item, "dev_time", cJSON_CreateString(device_local_time));
    cJSON_AddItemToObject(item, "dev_week", cJSON_CreateNumber(device_local_week));
	out = cJSON_Print(item);
    cJSON_Delete(item);
    return out;
}

char* ChannelCfgLoadCoverJson( int channelId)
{
	cJSON *arrayItem = cJSON_CreateArray();
    cJSON *item;
	int index;
	char *out;

	for(index = 0; index < 4; index++)
	{
		item = CfgDataToCjsonByMap(shelterRectMap[index]);
		if(!item)
		{
			PRINT_ERR("CfgDataToCjsonByMap error\n");
			return 0;
		}
		cJSON_AddItemToArray(arrayItem, item);
	}

	out = cJSON_Print(arrayItem);
    cJSON_Delete(arrayItem);
    return out;
}

