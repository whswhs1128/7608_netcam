#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"

#ifdef MODULE_SUPPORT_LOCAL_REC
#include "avi_rec.h"
#include "avi_search_api.h"
#include "mmc_api.h"


int Gk_CmsGetRecDayInMonth(int sock)
{
    #if 0
    char buf_tmp[4] = {0};
    int ret = GkSockRecv(sock, buf_tmp, sizeof(buf_tmp));
    if (ret != sizeof(buf_tmp)) {
        PRINT_ERR("ret:%d size:%d", ret, sizeof(buf_tmp));
        return -1;
    }

    printf("0:%d 1:%d 2:%d 3:%d", (int)buf_tmp[0], (int)buf_tmp[1], (int)buf_tmp[2], (int)buf_tmp[3]);
    #endif

    DMS_NET_RECDAYINMONTH rec_day_in_month;
    memset(&rec_day_in_month, 0, sizeof(DMS_NET_RECDAYINMONTH));

    #if 1
    int ret = GkSockRecv(sock, (char *)&rec_day_in_month, sizeof(DMS_NET_RECDAYINMONTH));
    if (ret != sizeof(DMS_NET_RECDAYINMONTH)) {
        PRINT_ERR("ret:%d size:%d", ret, sizeof(DMS_NET_RECDAYINMONTH));
        return -1;
    }
    #endif
    printf("\n\n ret%d size:%d ************************ \n", ret, sizeof(DMS_NET_RECDAYINMONTH));
    printf("size:%lu channel:%lu search:%u-%u, type:%x, map:%ld\n",
                rec_day_in_month.dwSize, rec_day_in_month.nChannel,
                rec_day_in_month.year, rec_day_in_month.month,
                rec_day_in_month.type, rec_day_in_month.lCalendarMap);

    rec_day_in_month.lCalendarMap = (long)search_day_by_month(0, 0, rec_day_in_month.year, rec_day_in_month.month);
    rec_day_in_month.dwSize = sizeof(DMS_NET_RECDAYINMONTH);
    rec_day_in_month.nChannel = 0;
    rec_day_in_month.type = 0xff;

    return GkCmsCmdResq(sock, &rec_day_in_month, sizeof(DMS_NET_RECDAYINMONTH), DMS_NET_GET_REC_DAY_IN_MONTH);
}



void schedule_time_15_to_30(unsigned char *slice_30,  SDK_U32 *slice_15)
{
    int i, j;
    unsigned int tmp;
    int mask;
    for (i = 0; i < 7; i ++) {
        for (j = 0; j < 48; j ++) {
            if ((j >= 0) && (j < 16)) {
                tmp = *(slice_15 + i * 3 + 0);
                mask = j * 2;
                if ((tmp & (1 << mask)) || (tmp & (1 << (mask + 1))))
                    *slice_30 = 1;
                else
                    *slice_30 = 0;
            } else if ((j >= 16) && (j < 32)) {
                tmp = *(slice_15 + i * 3 + 1);
                mask = (j - 16) * 2;
                if ((tmp & (1 << mask)) || (tmp & (1 << (mask + 1))))
                    *slice_30 = 1;
                else
                    *slice_30 = 0;
            } else {
                tmp = *(slice_15 + i * 3 + 2);
                mask = (j - 32) * 2;
                if ((tmp & (1 << mask)) || (tmp & (1 << (mask + 1))))
                    *slice_30 = 1;
                else
                    *slice_30 = 0;
            }

            slice_30 ++;
        }
    }
}

void schedule_time_30_to_15(SDK_U32 *slice_15, unsigned char *slice_30)
{
    int i, j;
    unsigned int *tmp;
    int mask;
    for (i = 0; i < 7; i ++) {
        for (j = 0; j < 48; j ++) {
            if ((j >= 0) && (j < 16)) {
                tmp = slice_15 + i * 3 + 0;
                mask = j * 2;
                if (*slice_30 == 0) {
                    *tmp &= ~(1 << mask);
                    *tmp &= ~(1 << (mask + 1));
                } else {
                    *tmp |= (1 << mask);
                    *tmp |= (1 << (mask + 1));
                }
            } else if ((j >= 16) && (j < 32)) {
                tmp = slice_15 + i * 3 + 1;
                mask = (j - 16) * 2;
                if (*slice_30 == 0) {
                    *tmp &= ~(1 << mask);
                    *tmp &= ~(1 << (mask + 1));
                } else {
                    *tmp |= (1 << mask);
                    *tmp |= (1 << (mask + 1));
                }
            } else {
                tmp = slice_15 + i * 3 + 2;
                mask = (j - 32) * 2;
                if (*slice_30 == 0) {
                    *tmp &= ~(1 << mask);
                    *tmp &= ~(1 << (mask + 1));
                } else {
                    *tmp |= (1 << mask);
                    *tmp |= (1 << (mask + 1));
                }
            }

            slice_30 ++;
        }
    }
}


int Gk_CmsGetRecord(int sock)
{
    DMS_NET_CHANNEL_RECORD rec;

    //to do
    // byRecordType ²ÎÕÕ eRecordType
    memset(&rec, 0, sizeof(DMS_NET_CHANNEL_RECORD));
    rec.dwSize = sizeof(DMS_NET_CHANNEL_RECORD);
    rec.dwChannel = 0;

    rec.dwPreRecordTime = runRecordCfg.preRecordTime;

    rec.byAudioRec = runRecordCfg.audioRecEnable;
    rec.byRecordMode = runRecordCfg.recordMode;
    rec.byStreamType = runRecordCfg.stream_no;

    schedule_time_15_to_30(&(rec.stRecordSched.byHour[0][0]), &(runRecordCfg.scheduleSlice[0][0]));

    #if 1
    printf("byhour.\n");
    int i, j;
    for (i = 0; i < 7; i ++) {
        printf("%d: ", i);
        for (j = 0; j < 48; j ++) {
            printf("[%d] %u  ", j, rec.stRecordSched.byHour[i][j]);
        }
        printf("\n");
    }
    #endif

    return GkCmsCmdResq(sock, &rec, sizeof(DMS_NET_CHANNEL_RECORD), DMS_NET_GET_RECORDCFG);
}

int Gk_CmsGetDefRecord(int sock)
{
    DMS_NET_CHANNEL_RECORD rec;

    //to do
    // byRecordType ²ÎÕÕ eRecordType
    memset(&rec, 0, sizeof(DMS_NET_CHANNEL_RECORD));
    rec.dwSize = sizeof(DMS_NET_CHANNEL_RECORD);
    CfgGetDefByName(recordMap, "channel", &(rec.dwChannel));
    CfgGetDefByName(recordMap, "preRecordTime", &(rec.dwPreRecordTime));
    CfgGetDefByName(recordMap, "audioRecEnable", &(rec.byAudioRec));
    CfgGetDefByName(recordMap, "recordMode", &(rec.byRecordMode));
    CfgGetDefByName(recordMap, "channel", &(rec.byStreamType));

    return GkCmsCmdResq(sock, &rec, sizeof(DMS_NET_CHANNEL_RECORD), DMS_NET_GET_DEF_RECORDCFG);
}

int Gk_CmsSetRecord(int sock)
{
    DMS_NET_CHANNEL_RECORD rec;
    int ret = GkSockRecv(sock, (char *)&rec, sizeof(DMS_NET_CHANNEL_RECORD));
    if (ret != sizeof(DMS_NET_CHANNEL_RECORD)) {
        PRINT_ERR();
        return -1;
    }

    //todo
    if (rec.dwSize != sizeof(DMS_NET_CHANNEL_RECORD)) {
        PRINT_ERR();
        return -1;
    }
    PRINT_INFO("dwChannel:%lu, byStreamType:%u byRecordMode:%u\n", rec.dwChannel, rec.byStreamType, rec.byRecordMode);
    runRecordCfg.stream_no = rec.byStreamType;
    runRecordCfg.preRecordTime = rec.dwPreRecordTime;
    runRecordCfg.audioRecEnable = rec.byAudioRec;
    runRecordCfg.recordMode = rec.byRecordMode;

    int i, j;
    for (i = 0; i < 7; i ++) {
        printf("%d:  ", i);
        for (j = 0; j < 48; j ++) {
            printf("%u ", rec.stRecordSched.byHour[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    schedule_time_30_to_15(&(runRecordCfg.scheduleSlice[0][0]), &(rec.stRecordSched.byHour[0][0]));
    for (i = 0; i < 7; i ++) {
        printf("%d:  ", i);
        for (j = 0; j < 3; j ++) {
            printf("%u ", runRecordCfg.scheduleSlice[i][j]);
        }
        printf("\n");
    }


    return 0;
}

int Gk_CmsGetRecordStreamMode(int sock)
{
    DMS_NET_RECORD_STREAMMODE rec_stream_mode;

    //to do
    memset(&rec_stream_mode, 0, sizeof(DMS_NET_RECORD_STREAMMODE));
    rec_stream_mode.dwSize = sizeof(DMS_NET_RECORD_STREAMMODE);
    rec_stream_mode.dwStreamType = runRecordCfg.stream_no;

    return GkCmsCmdResq(sock, &rec_stream_mode, sizeof(DMS_NET_RECORD_STREAMMODE), DMS_NET_GET_RECORDSTREAMMODE);
}

int Gk_CmsSetRecordStreamMode(int sock)
{
    DMS_NET_RECORD_STREAMMODE stream;
    int ret = GkSockRecv(sock, (char *)&stream, sizeof(DMS_NET_RECORD_STREAMMODE));
    if (ret != sizeof(DMS_NET_RECORD_STREAMMODE)) {
        PRINT_ERR();
        return -1;
    }

    //todo
    if (stream.dwSize != sizeof(DMS_NET_RECORD_STREAMMODE)) {
        PRINT_ERR();
        return -1;
    }
    runRecordCfg.stream_no = stream.dwStreamType;
    RecordCfgSave();

    return 0;
}

int Gk_CmsGetRecordMode(int sock)
{
    DMS_NET_CHANNEL_RECORD rec;

    //to do
    memset(&rec, 0, sizeof(DMS_NET_CHANNEL_RECORD));
    rec.dwSize = sizeof(DMS_NET_CHANNEL_RECORD);
    rec.dwChannel = 0;

    rec.dwPreRecordTime = runRecordCfg.preRecordTime;

    rec.byAudioRec = runRecordCfg.audioRecEnable;
    rec.byRecordMode = runRecordCfg.recordMode;
    rec.byStreamType = runRecordCfg.stream_no;

    return GkCmsCmdResq(sock, &rec, sizeof(DMS_NET_CHANNEL_RECORD), DMS_NET_GET_RECORDMODECFG);
}

int Gk_CmsSetRecordMode(int sock)
{
    DMS_NET_CHANNEL_RECORD rec;
    int ret = GkSockRecv(sock, (char *)&rec, sizeof(DMS_NET_CHANNEL_RECORD));
    if (ret != sizeof(DMS_NET_CHANNEL_RECORD)) {
        PRINT_ERR();
        return -1;
    }

    //todo

    return 0;
}

int Gk_CmdRecControl(int sock)
{
    DMS_NET_REC_CONTROL stRecControl;
    int ret = GkSockRecv(sock, (char *)&stRecControl, sizeof(DMS_NET_REC_CONTROL));
    if (ret != sizeof(DMS_NET_REC_CONTROL)) {
        PRINT_ERR();
        return -1;
    }

    //todo
    DMS_NET_REC_CONTROL *rec_control = (DMS_NET_REC_CONTROL *)&stRecControl;
    int rec_time_once = runRecordCfg.recordLen;
    if ((rec_control->nRecordType == GK_NET_RECORD_TYPE_MANU) || (rec_control->nRecordType == GK_NET_RECORD_TYPE_CMD)) {
        if (rec_control->wAction == 1) {
            if (rec_control->wDuration == 0xFFFF)
                manu_rec_start_alltime(runRecordCfg.stream_no, rec_time_once);
            else
                manu_rec_start_fixtime(runRecordCfg.stream_no, rec_control->wDuration, rec_time_once);

        } else if (rec_control->wAction == 0)
            manu_rec_stop(runRecordCfg.stream_no);
        else
            PRINT_INFO("wAction error.\n");

    } else {
        PRINT_INFO("nRecordType error.\n");
    }

    return 0;
}

int Gk_CmsRecordStart(int sock)
{
    int rec_time_once = runRecordCfg.recordLen;
    PRINT_INFO("manu rec stram%d start. once_len:%d\n", runRecordCfg.stream_no, rec_time_once);

	if (grd_sd_is_device_exist() == 0)
		return 0;
	
    manu_rec_start_alltime(runRecordCfg.stream_no, rec_time_once);
    return 0;
}


int Gk_CmsRecordStop(int sock)
{
    PRINT_INFO("manu rec stram%d stop\n", runRecordCfg.stream_no);
    manu_rec_stop(runRecordCfg.stream_no);
    return 0;
}

#endif
