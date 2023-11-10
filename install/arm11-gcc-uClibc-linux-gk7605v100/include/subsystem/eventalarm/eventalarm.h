/*!
*****************************************************************************
** FileName     : eventalarm.h
**
** Description  : config for pt callback.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-27
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_PT_API_H__
#define _GK_PT_API_H__

#ifdef __cplusplus
extern "C"
{
#endif


/************************************************************************/
/*       �豸�����¼�����                                               */
/************************************************************************/
typedef enum __GK_ALARM_TYPE_E
{
    GK_ALARM_TYPE_ALARMIN = 0,     //0:�ź���������ʼ
    GK_ALARM_TYPE_DISK_FULL,       //1:Ӳ����
    GK_ALARM_TYPE_VLOST,           //2:�źŶ�ʧ
    GK_ALARM_TYPE_VMOTION,         //3:�ƶ����
    GK_ALARM_TYPE_DISK_UNFORMAT,   //4:Ӳ��δ��ʽ��
    GK_ALARM_TYPE_DISK_RWERR,      //5:��дӲ�̳���,
    GK_ALARM_TYPE_VSHELTER,        //6:�ڵ�����
    GK_ALARM_TYPE_VFORMAT_UNMATCH, //7:��ʽ��ƥ��
    GK_ALARM_TYPE_ILLEGAL_ACCESS,  //8:�Ƿ�����
    GK_ALARM_TYPE_ALARMIN_RESUME,  //9:�ź��������ָ�
    GK_ALARM_TYPE_VLOST_RESUME,    //10:��Ƶ��ʧ�����ָ�
    GK_ALARM_TYPE_VMOTION_RESUME,  //11:��Ƶ�ƶ���ⱨ���ָ�
    GK_ALARM_TYPE_NET_BROKEN,      //12:����Ͽ�
    GK_ALARM_TYPE_IP_CONFLICT,     //13:IP��ͻ
    GK_ALARM_TYPE_ITEV,			//14:���ܷ���
    GK_ALARM_TYPE_ITEV_RESUME,		//15:���ܷ��������ָ�
	GK_ALARM_TYPE_TEMPERATURE,		//16:�¶�
	GK_ALARM_TYPE_HUMIDITY,		//17:ʪ��
    GK_ALARM_TYPE_ITEV_TRIPWIRE,  //18:����
    GK_ALARM_TYPE_ITEV_TRIPWIRE_RESUME,  //19:���߻ָ�
    GK_ALARM_TYPE_ITEV_VIDEOFOOL,  //20:��Ƶ�쳣
    GK_ALARM_TYPE_ITEV_VIDEOFOOL_RESUME,  //21:�쳣�ָ��ָ�
    GK_ALARM_TYPE_AREA_ALARM,  //22:��������
    GK_ALARM_TYPE_UPGRADE_ALARM,  //23:����ʧ��
    GK_ALARM_TYPE_UPLOAD_ALARM,  //24:�ϴ��ļ�ʧ��
    GK_ALARM_TYPE_BUTT,
}GK_ALARM_TYPE_E;

/*
** ����˵����
** Channel��ͨ���ţ���0��ʼ
** AlarmType���澯���� GK_ALARM_TYPE_E
** Action���澯״̬ 1���澯  0������
** param: ����ָ���ݲ���
*/
typedef int (*event_alarm_cb_func)(int nChannel, int nAlarmType, int nAction, void* pParam);

/* ��ʼ���¼����� */
int event_alarm_init(void);


/* ע���¼�������alarm_cbΪ�¼��ص�����������ֵΪ�¼����������NULL����ʾʧ�� */
void* event_alarm_open(event_alarm_cb_func alarm_cb);

/* �ر��¼�����*/
int event_alarm_close(void *eventhandle);

/* ����ģ����øú�������һ���¼��ص�����
** ����˵����
** Channel��ͨ���ţ���0��ʼ
** AlarmType���澯���� GK_ALARM_TYPE_E
** Action���澯״̬ 1���澯  0������
** param: ����ָ���ݲ���
*/
int event_alarm_touch(int channel, GK_ALARM_TYPE_E alarmType, int action, void* param);

/* �����¼����� */
int event_alarm_exit(void);


#ifdef __cplusplus
}
#endif
#endif
