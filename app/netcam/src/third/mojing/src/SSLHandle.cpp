

/*SSLHandle.cpp*/

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <sys/syscall.h>

#include "SSLHandle.h"
//#include "app_hemu.h"

//#include "sdk_enc.h"

#ifndef Y_PRINTF
#define Y_PRINTF(fmt, args...)      printf("[SSLHandle] " fmt, ##args)
#endif

#ifndef _GET_ERRNO
#define _GET_ERRNO() errno
#endif

SSLContext *SSLContext::m_instance = NULL;
pthread_mutex_t SSLContext::m_initMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t *lock_cs;
static long *lock_count;

pthread_t pthreads_thread_id(void)
{
    pthread_t ret;
    ret = pthread_self();
    return(ret);
}

void pthreads_locking_callback(int mode, int type, const char *file, int line)
{
    if (mode & CRYPTO_LOCK) {
        pthread_mutex_lock(&(lock_cs[type]));
        lock_count[type]++;
    } else {
        pthread_mutex_unlock(&(lock_cs[type]));
    }
}

//#include "RTPHandle.h"
unsigned int get_time_ms()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    return ((unsigned int)(((unsigned int)tv.tv_sec) * 1000) + (unsigned int)(((unsigned int)tv.tv_usec) / 1000));
}

SSLContext::SSLContext()
    : m_sslCtx(NULL)
{
    int i = 0;
    const char* cipher_list =
    "ECDHE-ECDSA-AES128-GCM-SHA256:"
    "ECDHE-RSA-AES128-GCM-SHA256:"
    "ECDHE-ECDSA-AES128-SHA256:"
    "ECDHE-RSA-AES128-SHA256:"
    "ECDHE-ECDSA-AES256-GCM-SHA384:"
    "ECDHE-RSA-AES256-GCM-SHA384";
    int seed_int[100] = {0};

    //create SSL context
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();
    m_sslCtx = SSL_CTX_new(TLSv1_2_client_method());
    if(m_sslCtx == NULL)
        return ;

    srand((unsigned)time(NULL));

    for (i = 0; i < 100; i++) {
        seed_int[i] = rand();
    }

    RAND_seed(seed_int, sizeof(seed_int));
    SSL_CTX_set_cipher_list(m_sslCtx, cipher_list);
    SSL_CTX_set_mode(m_sslCtx, SSL_MODE_AUTO_RETRY);

    lock_cs = (pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
    lock_count = (long *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
    for (i = 0; i < CRYPTO_num_locks(); i++) {
           lock_count[i] = 0;
           pthread_mutex_init(&(lock_cs[i]), NULL);
    }

    CRYPTO_set_id_callback((unsigned long (*)())pthreads_thread_id);
    CRYPTO_set_locking_callback(pthreads_locking_callback);
}

SSLContext::~SSLContext()
{
	SSL_CTX_free(m_sslCtx);        /* release context */
}

SSLContext *SSLContext::GetInstance()
{
    if (!m_instance) {
        pthread_mutex_lock(&m_initMutex);
        if (!m_instance) {
            m_instance = new SSLContext();
        }
        pthread_mutex_unlock(&m_initMutex);
    }
    return m_instance;
}


int SSLHandle::m_netspeedOk = 1;
time_t SSLHandle::m_lastNetspeedCheckTime = 0;
SSLHandle::SSLHandle()
    : m_ssl(NULL)
    , m_sockfd(-1)
    , m_sslError(0)
{
	pthread_mutex_init(&m_writeMutex, NULL);
	pthread_mutex_init(&m_readMutex, NULL);
	m_bytePerMs = 400000/8/1000;
}

SSLHandle::~SSLHandle()
{
    pthread_mutex_destroy(&m_writeMutex);
    pthread_mutex_destroy(&m_readMutex);
    //FIXME(heyong): m_sockfd will close by app.
    SSL_free(m_ssl);
}

int SSLHandle::SSLConnect(int timeout_sec)
{
    int r = 0;
    int err, ret = 0;
    SSL_set_connect_state (m_ssl);
    while ((r = SSL_do_handshake(m_ssl)) != 1) {
        //Y_PRINTF("SSLConnect: SSL_do_handshake ...\n", timeout_sec);
        err = SSL_get_error(m_ssl, r);
        if (err == SSL_ERROR_NONE) {
            continue;
        } else if (err == SSL_ERROR_WANT_READ) {
            // wait event again...
            ret = WaitEvent(WAIT_READ, timeout_sec);
            if (ret <= 0) {
                Y_PRINTF("SSLConnect: WAIT_READ TIMEOUT %ds\n", timeout_sec);
                ret = -2;// timeout..
                break;
            }
            continue;
        } else if (err == SSL_ERROR_WANT_WRITE) {
            // wait event again...
            ret = WaitEvent(WAIT_WRITE, timeout_sec);
            if (ret <= 0) {
                Y_PRINTF("SSLConnect: WAIT_WRITE TIMEOUT %ds\n", timeout_sec);
                ret = -2;// timeout..
                break;
            }
            continue;
        } else if (err == SSL_ERROR_SYSCALL) {
 		    if ((_GET_ERRNO() != EINTR) && (_GET_ERRNO() != EAGAIN)) {
                Y_PRINTF("SSLConnect: SSL_ERROR_SYSCALL ERROR:(%d)%s\n",
                            _GET_ERRNO(), strerror(_GET_ERRNO()));
                ret = -3;// net error.
                break;
            }
            Y_PRINTF("SSLConnect: SSL_SYSCALL %d\n", _GET_ERRNO());
            continue;
        } else {
            Y_PRINTF("SSLConnect: UNKNOW ERROR %d\n", err);
            ERR_print_errors_fp(stderr);
            ret = -(err);// error.
            break;
        }
    }
    return ret;
}

int SSLHandle::ConnectSocketFd(int sockfd)
{
	int flags;
    SSL_CTX* sslctx = SSLContext::GetInstance()->GetSSLCTX();
    if (!sslctx) {
        Y_PRINTF("ssl ctx is null");
        return -1;
    }
    m_sockfd = sockfd;

    //建立SSL连接
    m_ssl = SSL_new(sslctx);
    if (SSL_set_fd(m_ssl, m_sockfd) <= 0) {
		Y_PRINTF("ssl set fd error");
		return -1;
    }

    //FIXME(heyong): set non-block after SSL_connect.
    flags = fcntl(m_sockfd, F_GETFL, 0);
    fcntl(m_sockfd, F_SETFL, flags | O_NONBLOCK);

#if 0
    //set send buffer
	int size = 0;
    int len = sizeof(size);
    getsockopt(m_sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, (socklen_t * )&len);
    Y_PRINTF("Socket: the default tcp send buffer size is %d\n",size);

    size = 204800;
    setsockopt(m_sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, len);
    Y_PRINTF("Socket: set the tcp send buffer size to %d\n",size);

    getsockopt(m_sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, (socklen_t *)&len);
    Y_PRINTF("Socket: after set, the tcp send buffer size is %d\n",size);

    //set recv buffer
    getsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&size, (socklen_t *)&len);
    Y_PRINTF("Socket: the default tcp recv buffer size is %d\n",size);

    size = 204800;
    setsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&size, len);
    Y_PRINTF("Socket: set the tcp recv buffer size to %d\n",size);

    getsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&size, (socklen_t *)&len);
    Y_PRINTF("Socket: after set, the tcp recv buffer size is %d\n",size);
#endif

    // set nodelay
    int on = 1;
    setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

    Y_PRINTF("ConnectSocketFd SSL connecting...\n");
    if (SSLConnect(20) < 0) {
        m_sslError = 1;
        Y_PRINTF("ConnectSocketFd failed...\n");
		return -1;
    }
    m_sslError = 0;
    Y_PRINTF("ConnectSocketFd ok...\n");
    return 0;
}

int SSLHandle::WaitEvent(wait_event_t event, int timeout_sec)
{
	int ret = 0, times = timeout_sec;
	fd_set fset;
	struct timeval tv;

    //Y_PRINTF("WaitEvent: %s sock[%d] event start..., %ds\n",
    //            (event == WAIT_READ)?"read":"write", m_sockfd, time(NULL));

	while (1) {
		FD_ZERO(&fset);
		FD_SET(m_sockfd, &fset);
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		if (event == WAIT_READ) {
			ret = select(m_sockfd + 1, &fset, NULL, NULL, &tv);
		} else if (event == WAIT_WRITE) {
			ret = select(m_sockfd + 1, NULL, &fset, NULL, &tv);
		} else {
            Y_PRINTF("WaitEvent: %s sock[%d] event error, %d\n",
                        (event == WAIT_READ)?"read":"write", m_sockfd,
                        event);
		}

 		if (ret > 0) {
            break;// select ok.
		}
 		if (((ret < 0) && (_GET_ERRNO() != EINTR) && (_GET_ERRNO() != EAGAIN))) {
            Y_PRINTF("WaitEvent: %s sock[%d] select error, [%d]%s\n",
                        (event == WAIT_READ)?"read":"write", m_sockfd,
                        _GET_ERRNO(), strerror(_GET_ERRNO()));
            break;
		}
		if (timeout_sec > 0) {
    		timeout_sec--;
    		if (timeout_sec == 0) {
                //Y_PRINTF("WaitEvent: %s sock[%d] select timeout, %ds\n",
                //        (event == WAIT_READ)?"read":"write", m_sockfd,
                //        times);
    		    break;
    		}
        }
	}

    //Y_PRINTF("WaitEvent: %s sock[%d] event end, ret = %d..., %ds\n",
    //            (event == WAIT_READ)?"read":"write", m_sockfd, ret, time(NULL));
	return ret;
}

int SSLHandle::SSLSend(void *buf, int size, int timeout_sec)
{
    int err = 0, ret = 0, count = 0;
    while (1) {
        ret = SSL_write(m_ssl, ((char *)buf) + count, size - count);
        if (ret <= 0) {
            err = SSL_get_error(m_ssl, ret);
            if (err == SSL_ERROR_WANT_WRITE) {
                //Y_PRINTF("SSLSend: SSL_ERROR_WANT_WRITE\n");
            } else if (err == SSL_ERROR_WANT_READ) {
                Y_PRINTF("SSLSend: SSL_ERROR_WANT_READ >>> FIXME(heyong): maybe error\n");
                count = -4;// error.
                break;
            } else if (err == SSL_ERROR_SYSCALL) {
     		    if ((_GET_ERRNO() != EINTR) && (_GET_ERRNO() != EAGAIN)) {
                    Y_PRINTF("SSLSend: SSL_ERROR_SYSCALL ERROR:(%d)%s\n",
                                _GET_ERRNO(), strerror(_GET_ERRNO()));
                    count = -3;// net error.
                    break;
                }
                //Y_PRINTF("SSLSend: SSL_SYSCALL %d\n", _GET_ERRNO());
            } else {
                Y_PRINTF("SSLSend: UNKNOW ERROR %d\n", err);
                ERR_print_errors_fp(stderr);
                count = -(err);// error.
                break;
            }

            // wait event again...
            ret = WaitEvent(WAIT_WRITE, timeout_sec);
            if (ret <= 0) {
                Y_PRINTF("SSLSend: TIMEOUT %ds, size = %d\n", timeout_sec, size);
                count = -2;// timeout..
                break;
            }
        }else {
            count += ret;
            if (count >= size) {
                break;
            }
            Y_PRINTF("SSLSend: send again %d(%d)\n", count, size);
        }
    }
    return count;
}

int SSLHandle::SSLRecv(void *buf, int size, int timeout_sec)
{
    int err = 0, ret = 0, count = 0;
    while (1) {
        ret = SSL_read(m_ssl, ((char *)buf) + count, size - count);
        if (ret <= 0) {
            err = SSL_get_error(m_ssl, ret);
            if (err == SSL_ERROR_WANT_READ) {
                Y_PRINTF("SSLRecv: SSL_ERROR_WANT_READ\n");
            } else if (err == SSL_ERROR_WANT_WRITE) {
                Y_PRINTF("SSLRecv: SSL_ERROR_WANT_WRITE >>> FIXME(heyong): maybe error\n");
                count = -4;// error.
                break;
            } else if (err == SSL_ERROR_SYSCALL) {
     		    if ((_GET_ERRNO() != EINTR) && (_GET_ERRNO() != EAGAIN)) {
                    Y_PRINTF("SSLRecv: SSL_ERROR_SYSCALL ERROR:(%d)%s\n",
                                _GET_ERRNO(), strerror(_GET_ERRNO()));
                    count = -3;// net error.
                    break;
                }
                //Y_PRINTF("SSLRecv: SSL_SYSCALL %d\n", _GET_ERRNO());
            } else {
                Y_PRINTF("SSLRecv: UNKNOW ERROR %d\n", err);
                ERR_print_errors_fp(stderr);
                count = -(err);// error.
                break;
            }

            // wait event again...
            ret = WaitEvent(WAIT_READ, timeout_sec);
            if (ret <= 0) {
                Y_PRINTF("SSLRecv: TIMEOUT %ds, size = %d\n", timeout_sec, size);
                count = -2;// timeout..
                break;
            }
        }else {
            count += ret;
            if (count >= size) {
                break;
            }
            Y_PRINTF("SSLRecv: recv again %d(%d)\n", count, size);
        }
    }
    return count;
}

int SSLHandle::WriteN(void *buf, int size, int timeout_sec)
{
    int ret = 0;
    unsigned int in_time;
    float byte_per_ms = 0;
    if (size <= 0) return size;
    in_time = get_time_ms();

    pthread_mutex_lock(&m_writeMutex);
    if (m_sslError) {
	    pthread_mutex_unlock(&m_writeMutex);
	    return -110;
    }
    ret = SSLSend(buf, size, timeout_sec);
    if (ret < 0) {
        m_sslError = 1;
        printf("SSL WriteN Error...%d...", ret);
    }
	pthread_mutex_unlock(&m_writeMutex);
	if (ret > 0) {
        m_sendDataSize += size;
        m_sendUseTime += get_time_ms()-in_time;

        if (m_sendUseTime >= 1000) {
            byte_per_ms = ((float)m_sendDataSize)/m_sendUseTime;
            if ((m_bytePerMs-byte_per_ms) > 0.0) {
#if LIVE_STREAM_FORCE_I_FRAME
                extern int g_now_stream_id;
                sdk_enc_iframe_force(g_now_stream_id);
                m_sendStatus = 1;
#endif
                printf("WriteN: net busy %f, time=%d...\n", (m_bytePerMs-byte_per_ms), time(NULL));

                // add for support NetspeedCheck api.
                m_netspeedOk = 0;
                m_lastNetspeedCheckTime = time(NULL);
            }
            //printf("WriteN: speed=>(%d/%d)=%f byte/ms...\n", m_sendDataSize, m_sendUseTime, byte_per_ms);
            m_sendDataSize = m_sendUseTime = 0;
        }
    }
	return ret;
}

int SSLHandle::WaitReadEvent(int timeout_sec)
{
	return WaitEvent(WAIT_READ, timeout_sec);
}

int SSLHandle::ReadN(void *buf, int size, int timeout_sec)
{
    int ret = 0;
    pthread_mutex_lock(&m_readMutex);
    if (m_sslError) {
	    pthread_mutex_unlock(&m_readMutex);
	    return -110;
    }
    ret = SSLRecv(buf, size, timeout_sec);
    if (ret < 0) {
        m_sslError = 1;
        printf("SSL ReadN Error...%d...", ret);
    }
	pthread_mutex_unlock(&m_readMutex);
	return ret;
}

int SSLHandle::NetspeedCheck()
{
    if (time(NULL) - m_lastNetspeedCheckTime > NETSPEED_RESET_TIME_SEC)
        m_netspeedOk = 1;
    return m_netspeedOk;
}
