#ifndef _XMAI_COMMON_API_H_
#define _XMAI_COMMON_API_H_

#include "xmai_common.h"

int ReqRegisterHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqkeepAliveHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqMonitorClaimHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqMonitorReqHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqConfigGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);

int ReqConfigSetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqChannelTileGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqChannelTileSetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);

int ReqChannelTileDotSetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqTimeQueryHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqSyncTimeHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);

int ReqSetIframeHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqSysinfoHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqSysManagerHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqUsersGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);

int ReqGroupsGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqFullAuthorityListHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqGuardHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqPtzHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqAbilityGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqTalkClaimHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);
int ReqTalkReqHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg);

int ReqIpsearchHandle(int fSockSearchfd, char* msg);
int ReqIpsearchRspHandle(int fSockSearchfd, char* msg);
int ReqIpSetReqHandle(int fSockSearchfd, char* msg);
int HandleAddr(char *resultBuf, char *sourceBuf);

int XMaiCreateThread(ThreadEntryPtrType entry, void *para, pthread_t *pid);

int XMaiMakeMsgUseBuf(char *fSendbuf, int buf_size, short messageid, char *jsonMsg, int fSessionInt, int fPeerSeqNum);
int XMaiBoardCastSendTo(int fSockSearchfd, const char* inData, const int inLength);
int XMaiMakeMsg(XMaiSessionCtrl *pXMaiSessionCtrl, short messageid, char *jsonMsg);
int XMaiSockSend(int sock, char *szbuf, int len);

void * XMaiTalkPthread(void * para);

void * XMaiSendMainStreamPthread(void *para);
void * XMaiSendSubStreamPthread(void *para);
int XMaiMakeMsgNoJsonMsg(XMaiSessionCtrl *pXMaiSessionCtrl, short messageid);
void XMaiPrintMsgHeader(char *buf);
int XMaiPrintMsg(char *msg);


int set_ip_addr(char *name,char *net_ip);
int set_mask_addr(char *name,char *mask_ip);


#endif
