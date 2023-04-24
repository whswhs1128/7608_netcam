/*!
*****************************************************************************
** FileName     : gk_cms_utility.h
**
** Description  : utility api of cms.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CMS_UTILITY_H_
#define _GK_CMS_UTILITY_H_

#ifdef __cplusplus
extern "C"
{
#endif





int GetTimetick(int *timetick);

int SetSockAttr(int fd);


int CreateTcpSock(char *ipaddr, int port);
int GkSockSend(int sock, char *szbuf, int len, int timeout);
int GkSockSendFrame(int sock, char *szbuf, int len);
int GkSockSendTalk(int sock, char *szbuf, int len, int timeout);
int GkSockRecv(int sock, char *szbuf, int len);

int safe_tcp_recv (int sockfd, void *buf, int bufsize);
int CreateBroadcastSock(int port);
int CreateMulticastSock(char *multi_group, int recv_port);
int BoardCastSendTo(int fSockSearchfd, char *multi_group, int send_port, const char* inData, const int inLength);

void PrintStringToUInt(char *str, int len);
int RecvExtData(int sock, char *recv_buf, int buf_size, int ext_len);

int gk_cms_rate_to_data(int rate, int total);
int gk_cms_data_to_rate(int data, int total);
int gk_cms_float_rate_to_data(float rate, int total);
float gk_cms_data_to_float_rate(int data, int total);
void Gk_CmsTestIRCut();
int Gk_CmsTestGetTestResult(int sock);
int Gk_CmsGetFacCFG(int sock);
void Gk_CmsTestWhiteLed();
int Gk_CmsTestCmd(int sock);
int Gk_CmsTestSetTestResult(int sock);
int Gk_CmsSetFacCFG(int sock);
int CreateBroadcastSock(int recv_port);
int Gk_CmsGetFactoryKeyTestStatus(void);
void Gk_CmsSetFactoryKeyTestResult(int keyStatus);


#ifdef __cplusplus
}
#endif
#endif
