
/* SSLHandle.h */

#ifndef  __SSLHANDLE_H__
#define __SSLHANDLE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include <pthread.h>

//#include <algorithm>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define WAIT_FOREVER                    (-1)
#define NETSPEED_CHECK_TIMEOUT_SEC      (5)//S
#define NETSPEED_RESET_TIME_SEC         (20)//S

/*enum type for timewait function*/
typedef enum {
    WAIT_READ = 0,
    WAIT_WRITE = 1,
} wait_event_t;

class SSLContext
{
public:
    static SSLContext *GetInstance();
    SSL_CTX *GetSSLCTX() {return m_sslCtx;}
private:
    SSLContext();
    ~SSLContext();
    SSL_CTX *m_sslCtx;
    static SSLContext *m_instance;
    static pthread_mutex_t m_initMutex;
};

class SSLHandle
{
public:
    SSLHandle();
    ~SSLHandle();
    int ConnectSocketFd(int sockfd);
    int WriteN(void *buf, int size, int timeout_sec);
    int WaitReadEvent(int timeout_sec);
    int ReadN(void *buf, int size, int timeout_sec);
    int SetBitRateKbps(int bit_rate) {m_bytePerMs = bit_rate/8/1000;}
    int GetSendStatus() {return m_sendStatus;}
    void CleanSendStatus() { m_sendStatus = 0;}
    static int NetspeedCheck();/*检测网速:0->网速不好，1->网速好*/

private:
    SSL* m_ssl;
    int m_sslError;
    int m_sockfd;
    pthread_mutex_t m_writeMutex;
    pthread_mutex_t m_readMutex;
    int SSLConnect(int timeout_sec);
    int WaitEvent(wait_event_t event, int timeout_sec);
    int SSLSend(void *buf, int size, int timeout_sec);
    int SSLRecv(void *buf, int size, int timeout_sec);

    /*网速检测*/
    static int m_netspeedOk;
    static time_t m_lastNetspeedCheckTime;

    /*帧发送速率检测*/
    float m_bytePerMs;//byte/ms
    int m_sendUseTime;
    int m_sendDataSize;
    int m_sendStatus;/*0->网速不好，1->网速好*/
};

#endif

