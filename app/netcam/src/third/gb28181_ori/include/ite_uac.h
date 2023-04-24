#ifndef ITE_UAC_H
#define ITE_UAC_H


#include "ite_sip_api.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <pthread.h>

//#include "ite_sip_interface.h"

extern OS_S32 g_bterminate;


int uac_init(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig)
{

    int ret=0;
    char localIP[128];
    eXosip_guess_localip(excontext,AF_INET, localIP, 128);
	#ifdef MODULE_SUPPORT_ZK_WAPI
    gb28181_get_guess_localip(localIP);
	#endif
    printf("uac_init -->start\n");

    ite_eXosip_launch(excontext,SipConfig);


    printf("uac_init ->ite_eXosip_init [%p    %p]\n",excontext,SipConfig);
    do
    {
        ret = ite_eXosip_init(excontext,SipConfig);
    }
    while (ret);
    eXosip_masquerade_contact (excontext, localIP, SipConfig->sipDevicePort);

    eXosip_set_user_agent(excontext, "GK sip v1.0");

    printf("eXosip_init success!--->linsten port: %d \r\n",SipConfig->sipDevicePort);

    ret = eXosip_listen_addr(excontext,IPPROTO_UDP, NULL, SipConfig->sipDevicePort, AF_INET, 0);/*IPPROTO_TCP IPPROTO_UDP*/
    //ret = eXosip_listen_addr(excontext,IPPROTO_TCP, NULL, SipConfig->sipDevicePort, AF_INET, 0);/*IPPROTO_TCP IPPROTO_UDP*/
    if (0 != ret)/*´«Êä²ã³õÊ¼»¯Ê§°Ü*/
    {
        eXosip_quit(excontext);
        printf("eXosip_listen_addr error!\r\n");
        return -1;
    }
    printf("eXosip_listen_addr success!\r\n");

    return 0;
}

int uac_register(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig)
{
    int nret =0;
    nret =ite_eXosip_register(excontext,SipConfig,SipConfig->regExpire_s);
    return nret;
}
int uac_UNregister(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig)
{
    int nret =0;
    nret =ite_eXosip_register(excontext,SipConfig,0);
    return nret;
}
int uac_invite(struct eXosip_t *excontext,sessionId * inviteId, char *to, char * sdp_message, char * responseSdp,IPNC_SipIpcConfig * SipConfig)
{
    //interface 1: do something in playSdp and inviteIp, and open the video transportation
    //char playSdp[4096];
    /*snprintf (playSdp, 4096,
        "v=0\r\n"
        "o=josua  0 0 IN IP4 %s\r\n"
        "s=Play\r\n"
        "c=IN IP4 %s\r\n"
        "t=0 0\r\n"
        "m=audio 8000 RTP/AVP 0 8 101\r\n", device_info.ipc_ip,device_info.ipc_ip);
        */
    //char inviteIp[100]="34020000001180000002@192.168.17.1:5060";
    //char inviteIp[100]="34020000001310000051@192.168.17.129:5060";
    //end interface 1:

    //char *responseSdp;
    //sessionId inviteId;
    ite_eXosip_invit(excontext,inviteId, to, sdp_message, responseSdp,SipConfig);

    //interface 2: do something with responseSdp ;
    //printf("main return sdp:%s \n",responseSdp);
    //end interface 2:
    return 0;
}

int uac_bye(struct eXosip_t *excontext,sessionId inviteId)
{
    return ite_eXosip_bye(excontext,inviteId);

    //interface 3: do something with responseSdp ;
    //turn off the steam of the video transport
    //end interface 3:

    //return 0;
}

int uac_send_info(struct eXosip_t *excontext,sessionId inviteId)
{
    osip_message_t *info;
    char info_body[1000];
    int i;
    eXosip_lock (excontext);
    i = eXosip_call_build_info (excontext,inviteId.did, &info);
    if (i == 0)
    {
        snprintf (info_body, 999, "Signal=sss\r\nDuration=250\r\n");
        osip_message_set_content_type (info, "Application/MANSRTSP");
        osip_message_set_body (info, info_body, strlen (info_body));
        i = eXosip_call_send_request (excontext,inviteId.did, info);
    }
    eXosip_unlock (excontext);
    return i;
}

int uac_send_message(struct eXosip_t *excontext,sessionId inviteId, char * type , char * type_info, char * message_str)
{
    osip_message_t *message;
    char message_body[1000];
    int i;
    eXosip_lock (excontext);
    i = eXosip_call_build_request (excontext,inviteId.did, type/*"MESSAGE" or "INFO"*/, &message);
    if (i == 0)
    {
        snprintf (message_body, 999, message_str/*"message_info"*/);
        if(type_info != NULL)
            osip_message_set_content_type (message, type_info/*"Application/MANSRTSP"*/);
        osip_message_set_body (message, message_body, strlen (message_body));
        i = eXosip_call_send_request (excontext,inviteId.did, message);
    }
    eXosip_unlock (excontext);

    return i;
}

int getlocalip(char* outip)
{
    int i = 0;
    int sockfd;
    struct ifconf ifconf;
    char *buf = (char*)malloc(512);
    struct ifreq *ifreq;
    char* ip;

    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return -1;
    }
    ioctl(sockfd, SIOCGIFCONF, &ifconf);    //
    close(sockfd);
    ifreq = (struct ifreq*)buf;
    i = ifconf.ifc_len / sizeof(struct ifreq);
    char *pos = outip;
    int count;
    for(count = 0; (count < 1 && i > 0); i--)
    {
        ip = inet_ntoa(((struct sockaddr_in*) & (ifreq->ifr_addr))->sin_addr);
        if(strncmp(ip, "127.0.0.1", 3) == 0) //
        {
            ifreq++;
            continue;
        }
        else
        {
            printf("%s\n", ip);
            strcpy(pos, ip);
            int len = strlen(ip);
            pos[len] = '\t';
            pos += len + 1;
            count ++;
            ifreq++;
        }
    }
    free(buf);
    return 0;

}



#endif //ITE_UAC_H
