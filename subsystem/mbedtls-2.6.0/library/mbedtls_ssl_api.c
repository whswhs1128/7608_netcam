#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include <stdlib.h>
#include <string.h>

typedef struct
{
    mbedtls_net_context server_fd;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl ;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
}Mbedtls_HandleT;


#define CA_VERFITY		0

#define MBEDTLS_LOG_TRACE_LEVEL (0)

#define __MBEDTLS_LOG_TRACE(syntax,fmt,arg...) \
	do{\
		printf("\033[%sm [%s, Line:%d] " fmt "\033[0;0m\n", syntax, __FILE__, __LINE__,  ##arg); \
	}while(0)


#if (MBEDTLS_LOG_TRACE_LEVEL == 0)
	#define MBEDTLS_LOG_ERR(fmt,arg...)         __MBEDTLS_LOG_TRACE("1;31",fmt,##arg)
#else
	#define MBEDTLS_LOG_ERR(fmt,arg...)
#endif

#if (MBEDTLS_LOG_TRACE_LEVEL <= 1)
	#define MBEDTLS_LOG_SPECIAL(fmt,arg...)     __MBEDTLS_LOG_TRACE("1;32",fmt,##arg)
#else
	#define MBEDTLS_LOG_SPECIAL(fmt,arg...)
#endif

#if (MBEDTLS_LOG_TRACE_LEVEL <= 2)
	#define MBEDTLS_LOG_WARNING(fmt,arg...)     __MBEDTLS_LOG_TRACE("1;36",fmt,##arg)
#else
	#define MBEDTLS_LOG_WARNING(fmt,arg...)
#endif

#if (MBEDTLS_LOG_TRACE_LEVEL <= 3)
	#define MBEDTLS_LOG_INFO(fmt,arg...)       __MBEDTLS_LOG_TRACE("1;36",fmt,##arg)
#else
	#define MBEDTLS_LOG_INFO(fmt,arg...)
#endif

#if CA_VERFITY
static int my_verify( void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags )
{
    char buf[1024];
    ((void) data);

    printf( "\nVerify requested for (Depth %d):\n", depth );
    mbedtls_x509_crt_info( buf, sizeof( buf ) - 1, "", crt );
    printf( "%s", buf );

    if ( ( *flags ) == 0 )
        printf( "  This certificate has no flags\n" );
    else
    {
        mbedtls_x509_crt_verify_info( buf, sizeof( buf ), "  ! ", *flags );
        printf( "%s\n", buf );
    }

    return( 0 );
}
#endif

void * gk_mbedtls_ssl_init(int fd, char *host, char* port)
{
        int ret, len;
        uint32_t flags;
        const char *pers = "ssl_client1";
    	Mbedtls_HandleT *handle = NULL;
   		mbedtls_ssl_context *ssl ;
        handle = malloc(sizeof(Mbedtls_HandleT));
        if(!handle)
        {
            return NULL;
        }
    	memset(handle,0,sizeof(Mbedtls_HandleT));
        /*
         * 0. Initialize the RNG and the session data
         */
        mbedtls_net_init( &handle->server_fd );
        mbedtls_ssl_init( &handle->ssl );
        mbedtls_ssl_config_init( &handle->conf );
        mbedtls_x509_crt_init( &handle->cacert );
        mbedtls_ctr_drbg_init( &handle->ctr_drbg );

        mbedtls_entropy_init( &handle->entropy );

		ssl = &handle->ssl ;
        if( ( ret = mbedtls_ctr_drbg_seed( &handle->ctr_drbg, mbedtls_entropy_func, &handle->entropy,
                                   (const unsigned char *) pers,
                                   strlen( pers ) ) ) != 0 )
        {
            MBEDTLS_LOG_ERR( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
            goto exit;
        }


        /*
         * 0. Initialize certificates
         */
         #if CA_VERFITY
        printf( "  . Loading the CA root certificate ..." );

   //     printf("xxxx %s, %d\n", __func__, __LINE__);
        ret = mbedtls_x509_crt_parse( &handle->cacert, (const unsigned char *) mbedtls_test_cas_pem,
                              mbedtls_test_cas_pem_len );
        if( ret < 0 )
        {
            MBEDTLS_LOG_ERR( " failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
            goto exit;
        }
		#endif
        //printf("xxxx %s, %d\n", __func__, __LINE__);

        //printf( " ok (%d skipped)\n", ret );

        /*
         * 1. Start the connection
         */
        //printf( "  . Connecting to tcp/%s/%s...", host, port );
        //printf("xxxx %s, %d\n", __func__, __LINE__);
        if(fd > 0)
        {
            handle->server_fd.fd =fd;
        }
        else{
            if( ( ret = mbedtls_net_connect( &handle->server_fd, host,
                                                 port, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
            {
                MBEDTLS_LOG_ERR( " failed\n  ! mbedtls_net_connect returned %d\n\n", ret );
                goto exit;
            }
        }

        //printf( " ok\n" );

        /*
         * 2. Setup stuff
         */
        //printf( "  . Setting up the SSL/TLS structure..." );

        if( ( ret = mbedtls_ssl_config_defaults( &handle->conf,
                        MBEDTLS_SSL_IS_CLIENT,
                        MBEDTLS_SSL_TRANSPORT_STREAM,
                        MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
        {
            MBEDTLS_LOG_ERR( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
            goto exit;
        }

        //printf( " ok\n" );

        //printf("xxxx %s, %d\n", __func__, __LINE__);
        /* OPTIONAL is not optimal for security,
         * but makes interop easier in this simplified example */
		#if CA_VERFITY

        mbedtls_ssl_conf_authmode( &handle->conf, MBEDTLS_SSL_VERIFY_REQUIRED );
        mbedtls_ssl_conf_ca_chain( &handle->conf, &handle->cacert, NULL );
        mbedtls_ssl_conf_verify( &handle->conf, my_verify, NULL );
		#else
        mbedtls_ssl_conf_authmode( &handle->conf, MBEDTLS_SSL_VERIFY_NONE);

    	#endif
		mbedtls_ssl_conf_rng( &handle->conf, mbedtls_ctr_drbg_random, &handle->ctr_drbg );

        if( ( ret = mbedtls_ssl_setup( ssl, &handle->conf ) ) != 0 )
        {
            MBEDTLS_LOG_ERR( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
            goto exit;
        }

        if( ( ret = mbedtls_ssl_set_hostname( ssl, host ) ) != 0 )
        {
            MBEDTLS_LOG_ERR( " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret );
            goto exit;
        }

        mbedtls_ssl_set_bio( ssl, &handle->server_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

        //printf("xxxx %s, %d\n", __func__, __LINE__);
        /*
         * 4. Handshake
         */
        //printf( "  . Performing the SSL/TLS handshake..." );
        mbedtls_ssl_conf_read_timeout(&handle->conf, 5000);

        while( ( ret = mbedtls_ssl_handshake( ssl ) ) != 0 )
        {
            if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
            {
                MBEDTLS_LOG_ERR( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret );
                goto exit;
            }
        }

        //printf( " ok\n" );

        /*
         * 5. Verify the server certificate
         */
        //printf( "  . Verifying peer X.509 certificate..." );

        /* In real life, we probably want to bail out when ret != 0 */
        if( ( flags = mbedtls_ssl_get_verify_result( ssl ) ) != 0 )
        {
            char vrfy_buf[512];

            MBEDTLS_LOG_ERR( " failed\n" );

            mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

            MBEDTLS_LOG_ERR( "%s\n", vrfy_buf );
        }
        else
        {
            MBEDTLS_LOG_INFO( "Mbedtls connect and cerify ok\n" );
        }

    return (void*)handle;
    exit:

#ifdef MBEDTLS_ERROR_C
    if( ret != 0 )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        MBEDTLS_LOG_ERR("Last error was: %d - %s\n\n", ret, error_buf );
    }
#endif

    mbedtls_net_free( &handle->server_fd );

    mbedtls_x509_crt_free( &handle->cacert );
    mbedtls_ssl_free( &handle->ssl );
    mbedtls_ssl_config_free( &handle->conf );
    mbedtls_ctr_drbg_free( &handle->ctr_drbg );
    mbedtls_entropy_free( &handle->entropy );
	free(handle);
        return NULL;
}

int gk_mbedtls_ssl_send(void *Handle, char *buffer, int dataLen)
{
    int ret;
	Mbedtls_HandleT *ssLhandle = (Mbedtls_HandleT *) Handle;
    mbedtls_ssl_context *ssl;
	int len = 0;
	if(Handle == NULL)
		return -1;

	ssl =  &ssLhandle->ssl;

    while( ( ret = mbedtls_ssl_write( ssl, buffer, dataLen ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            MBEDTLS_LOG_ERR( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
            return -1;
        }
    }

    return 0;
}

int gk_mbedtls_ssl_recv(void *Handle, char *buffer, int bufferLen)
{
    int len, ret;
	int recv_len = 0;
	Mbedtls_HandleT *ssLhandle = (Mbedtls_HandleT *) Handle;
    mbedtls_ssl_context *ssl;

	if(Handle == NULL)
		return -1;

    if (bufferLen <= 0)
    {
        return bufferLen;
    }

	ssl =  &ssLhandle->ssl;

	len = bufferLen ;
	memset( buffer, 0, bufferLen );

    do
    {
        ret = mbedtls_ssl_read( ssl, buffer+recv_len, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            MBEDTLS_LOG_ERR( "failed\n  ! mbedtls_ssl_read returned %d\n\n", ret );
            break;
        }

        if( ret == 0 )
        {
            //printf( "\n\nEOF\n\n" );
            break;
        }

        recv_len += ret;
        //printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
        //printf( "-->ssl %d bytes read\n", recv_len);
        if(recv_len < bufferLen)
			break;
		len = bufferLen-recv_len;
    }
    while( len > 0);


    return recv_len;
}


int gk_mbedtls_ssl_exit(void *Handle)
{
	Mbedtls_HandleT *ssLhandle = (Mbedtls_HandleT *) Handle;
    mbedtls_ssl_context *ssl;
	int len = 0;
	if(Handle == NULL)
		return -1;

	ssl =  &ssLhandle->ssl;

    mbedtls_ssl_close_notify( ssl );

    mbedtls_net_free( &ssLhandle->server_fd );

    mbedtls_x509_crt_free( &ssLhandle->cacert );
    mbedtls_ssl_free( &ssLhandle->ssl );
    mbedtls_ssl_config_free( &ssLhandle->conf );
    mbedtls_ctr_drbg_free( &ssLhandle->ctr_drbg );
    mbedtls_entropy_free( &ssLhandle->entropy );
	free(ssLhandle);

    return 0;
}


