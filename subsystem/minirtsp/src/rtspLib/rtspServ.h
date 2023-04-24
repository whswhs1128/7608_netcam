/*!
*****************************************************************************
** \file      $gkprjrtspServ.h
**
** \version	$id: rtspServ.h 15-08-04  8æœ?08:1438655285
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/

#ifndef __RTSPSERV_H__
#define __RTSPSERV_H__

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "list.h"
#include "rtspServLib.h"
#include "rtcp.h"
#include "url.h"
#include "queue.h"
#include "rtp.h"

#define RTSP_VERSION        "RTSP/1.0"
#define BACKLOG             5   /* Max length of listening queue. */
#define RECV_TIMEOUT        15
#define RECV_BUF_SIZE       (4 * 1024)
#define SEND_BUF_SIZE       (4 * 1024)
#define MAX_METHOD_SIZE     32
#define MAX_URI_SIZE        128

/* RTSP method IDs. */
enum {
    RTSP_METHOD_OPTIONS       = 0,
    RTSP_METHOD_DESCRIBE      = 1,
    RTSP_METHOD_SETUP         = 2,
    RTSP_METHOD_TEARDOWN      = 3,
    RTSP_METHOD_PLAY          = 4,
    RTSP_METHOD_PAUSE         = 5,
    RTSP_METHOD_GET_PARAMETER = 6,
    RTSP_METHOD_SET_PARAMETER = 7,
};

/* States of RTSP state machine. */
enum {
    RTSP_STATE_INITED     = 1,
    RTSP_STATE_READY      = 2,
    RTSP_STATE_PLAYING    = 3,
    RTSP_STATE_RECORDING  = 4,
};

#define RTP_MAX_PACKET_SIZE		1472
#define RTP_MAX_PAYLOAD_SIZE	(RTP_MAX_PACKET_SIZE - 12)	/* minus sizeof(rtp_hdr_t) */

/**
 * Network layer over which RTP/etc packet data will be transported.
 */
enum RTSPLowerTransport {
	RTSP_LOWER_TRANSPORT_UDP = 0,           /**< UDP/unicast */
	RTSP_LOWER_TRANSPORT_TCP = 1,           /**< TCP; interleaved in RTSP */
	RTSP_LOWER_TRANSPORT_UDP_MULTICAST = 2, /**< UDP/multicast */
	RTSP_LOWER_TRANSPORT_NB,
	RTSP_LOWER_TRANSPORT_HTTP = 8,          /**< HTTP tunneled - not a proper
											  transport mode as such,
											  only for use via AVOptions */
	RTSP_LOWER_TRANSPORT_CUSTOM = 16,       /**< Custom IO - not a public
											  option for lower_transport_mask,
											  but set in the SDP demuxer based
											  on a flag. */
	RTSP_LOWER_TRANSPORT_SHMEM = 17,
};

/**
 * Packet profile of the data that we will be receiving. Real servers
 * commonly send RDT (although they can sometimes send RTP as well),
 * whereas most others will send RTP.
 */
enum RTSPTransport {
	RTSP_TRANSPORT_RTP, /**< Standards-compliant RTP */
	RTSP_TRANSPORT_RDT, /**< Realmedia Data Transport */
	RTSP_TRANSPORT_RAW, /**< Raw data (over UDP) */
	RTSP_TRANSPORT_NB
};

typedef struct RTSPTransportField {
    /** interleave ids, if TCP transport; each TCP/RTSP data packet starts
     * with a '$', stream length and stream ID. If the stream ID is within
     * the range of this interleaved_min-max, then the packet belongs to
     * this stream. */
    int interleaved_min, interleaved_max;

    /** UDP multicast port range; the ports to which we should connect to
     * receive multicast UDP data. */
    int port_min, port_max;

    /** UDP client ports; these should be the local ports of the UDP RTP
     * (and RTCP) sockets over which we receive RTP/RTCP data. */
    int client_port_min, client_port_max;

    /** UDP unicast server port range; the ports to which we should connect
     * to receive unicast UDP RTP/RTCP data. */
    int server_port_min, server_port_max;

    /** time-to-live value (required for multicast); the amount of HOPs that
     * packets will be allowed to make before being discarded. */
    int ttl;

    /** transport set to record data */
    int mode_record;

    struct sockaddr_storage destination; /**< destination IP address */
    char source[INET6_ADDRSTRLEN + 1]; /**< source IP address */

    /** data/packet transport protocol; e.g. RTP or RDT */
    enum RTSPTransport transport;

    /** network layer transport protocol; e.g. TCP or UDP uni-/multicast */
    enum RTSPLowerTransport lower_transport;

	char shm_name[128];
	int  shm_size;
} RTSPTransportField;

enum FramesType {
	FRAMES_TYPE_ALL	= 0,
	FRAMES_TYPE_INTRA,
	FRAMES_TYPE_PREDICTED,
};

typedef struct RTSPFramesField {
	int type;		/* 0 - all, 1 - intra, 2 - predicted */
	int interval;	/* in microsecond */
} RTSPFramesField;

typedef struct RTSPMessageHeader {
    /** length of the data following this header */
    int content_length;

//    enum RTSPStatusCode status_code; /**< response code from server */

    /** number of items in the 'transports' variable below */
//    int nb_transports;

    /** Time range of the streams that the server will stream. In
     * AV_TIME_BASE unit, AV_NOPTS_VALUE if not used */
    int64_t range_start, range_end;

    /** describes the complete "Transport:" line of the server in response
     * to a SETUP RTSP command by the client */
//    RTSPTransportField transports[RTSP_MAX_TRANSPORTS];

    int seq;                         /**< sequence number */

    /** the "Session:" field. This value is initially set by the server and
     * should be re-transmitted by the client in every RTSP command. */
    char session_id[512];

    /** the "Location:" field. This value is used to handle redirection.
     */
    char location[4096];

    /** the "RealChallenge1:" field from the server */
    char real_challenge[64];

    /** the "Server: field, which can be used to identify some special-case
     * servers that are not 100% standards-compliant. We use this to identify
     * Windows Media Server, which has a value "WMServer/v.e.r.sion", where
     * version is a sequence of digits (e.g. 9.0.0.3372). Helix/Real servers
     * use something like "Helix [..] Server Version v.e.r.sion (platform)
     * (RealServer compatible)" or "RealServer Version v.e.r.sion (platform)",
     * where platform is the output of $uname -msr | sed 's/ /-/g'. */
    char server[64];

    /** The "timeout" comes as part of the server response to the "SETUP"
     * command, in the "Session: <xyz>[;timeout=<value>]" line. It is the
     * time, in seconds, that the server will go without traffic over the
     * RTSP/TCP connection before it closes the connection. To prevent
     * this, sent dummy requests (e.g. OPTIONS) with intervals smaller
     * than this value. */
    int timeout;

    /** The "Notice" or "X-Notice" field value. See
     * http://tools.ietf.org/html/draft-stiemerling-rtsp-announce-00
     * for a complete list of supported values. */
    int notice;

    /** The "reason" is meant to specify better the meaning of the error code
     * returned
     */
    char reason[256];

    /**
     * Content type header
     */
    char content_type[64];

	/**
	 * ONVIF extended
	 */
	int onvif_replay;
	double scale;
	int rate_control;
	int immediate;
	RTSPFramesField frames;

	int max_interval;
} RTSPMessageHeader;

/* Interleaved header for transport(RTP over RTSP). */
typedef struct intlvd_s {
        unsigned char dollar;
        unsigned char chn;
        unsigned short size;
} intlvd_t;

typedef struct rtp_hdr_s {
#ifdef BIGENDIAN
	uint8_t v:2;          /* protocol version */
	uint8_t p:1;         	/* padding flag */
	uint8_t x:1;         	/* header extension flag */
	uint8_t cc:4;       	/* CSRC count */
	uint8_t m:1;         	/* marker bit */
	uint8_t pt:7;        	/* payload type */
#else
	uint8_t cc:4;
	uint8_t x:1;
	uint8_t p:1;
	uint8_t v:2;
	uint8_t pt:7;
	uint8_t m:1;
#endif
	uint16_t seq;			/* sequence number */
	uint32_t ts;          /* timestamp */
	uint32_t ssrc;        /* synchronization source */
//	uint32_t csrc[2];		/* optional CSRC list */
} rtp_hdr_t;

typedef struct onvif_exthdr_s {
	uint16_t id;		/* always 0xABAC */
	uint16_t length;	/* length = 3 */
	uint32_t ntp_timestamp_msw;
	uint32_t ntp_timestamp_lsw;
#ifdef BIGENDIAN
	uint8_t c:1;          	/* clean point */
	uint8_t e:1;         	/* end of a contiguous section */
	uint8_t d:1;         	/* discontinuity in transmission */
	uint8_t mbz:5;       	/* reserved, must be zero */
#else
	uint8_t mbz:5;       	/* reserved, must be zero */
	uint8_t d:1;         	/* discontinuity in transmission */
	uint8_t e:1;         	/* end of a contiguous section */
	uint8_t c:1;          	/* clean point */
#endif
	uint8_t cseq;
	uint16_t pad;
} onvif_exthdr_t;

//#define MAX_MEDIA_DESCRIBE	(4)
//#define MAX_MEDIA_STREAM	(2)

typedef struct RtspRoute {
	char route[128];
	RtspOps ops;
} RtspRoute;

typedef struct _y_test_buf {
    uint8_t *g_y_buffer;
    uint32_t g_y_buffer_total;
    uint    g_y_buffer_len;
} y_test_buf;

typedef struct userInfo{
    int   security;     //need security
    int   flag;         //success: 1, fail: 0
    char  username[64];
    char  userpw[64];
}userInfo_t;

typedef struct rtsp_session_s {
	void *handle;
    userInfo_t user;
	RtspOps *ops;

	RTSPTransportField transport[MEDIA_TYPE_MAX];
	RTSPMessageHeader message_headers;

//	RTSPMessageHeader last_play;

	int multicast;

	char request_uri[1024];
	UrlData *url;

	unsigned int session_id;
	unsigned int cseq;

	int8_t state;                /* State of RTSP state machine. */
	int8_t method_id;            /* Method ID. */

	pthread_t recv_data_tid;    /* Thread to recv data. */
	pthread_t send_data_tid;    /* Thread to send data. */

	char recv_buf[RECV_BUF_SIZE];

	/* session type: rtsp/http */
	int is_rtsp;

	/* sockets for RTSP/HTTP session */
	int rtsp_rcv_sock;
	int rtsp_snd_sock;

	int rtsp_snd_lock_flag;
	pthread_mutex_t rtsp_snd_lock;

	int abort;
	int timeout;

	int max_payload_size;

	/* multicast specified */
	int mcast_refcount;

	int ndesc;
	MediaDesc mdescs[MEDIA_TYPE_MAX];

	int pause;

	/* replay specified */
	float scale;
	int64_t range_start;
	int64_t range_end;
	int max_interval;	/* in millisecond */
	JQueue *cmdq;

	uint8_t *saved_keyframe;
	int saved_keyframe_length;
	frame_head_t saved_head;

	/* RTP information: [0] for video, [1] for audio. */
	struct {
		int enable;
		rtp_hdr_t rtp_hdr;
		rtp_stat_t stat;
		int64_t pts;
		int64_t pts_base;
		int64_t timebase;

		/* udp unicast specified */
		int rtp_sock;            /* Server RTP socket descriptor. */
		int rtcp_sock;           /* Server RTCP socket descriptor. */
	} rtp[2];

    y_test_buf *y_buf;
} rtsp_session_t;

typedef struct rtsp_serv_e {
	int abort_listen_thread;
	pthread_t rtsp_listen_tid;

	int rtspsock;             /* rtsp connection listening socket. */
	int httpsock;             /* http connection listening socket. */

	int abort_reaper;
	pthread_t rtsp_reaper_tid;

	unsigned short rtsp_port;
	unsigned short http_port;

	pthread_mutex_t sess_list_lock;
	JList *allsessions; /* Hold the RTSP sessions' information. */

	pthread_mutex_t mcast_list_lock;
	JList *allmulticasts;

	pthread_mutex_t route_list_lock;
	JList *allroutes;

	int enable_rtcp;
	int rtcp_interval;
} rtsp_serv_t;

extern rtsp_serv_t rtsp_serv;

int rtsp_startRtspServ(void);
int rtsp_stopRtspServ(void);
int rtsp_closeRtspSession(rtsp_session_t *sessp);
int rtsp_closeRtspMcastSession(rtsp_session_t *sessp);

uint16_t rtp_get_multicast_addr(rtsp_session_t *sessp, int media_type, char *addr, int addrlen, uint16_t *port, int *ttl);

rtsp_session_t *rtsp_find_multicast_session(const char *url);
rtsp_session_t *rtsp_create_multicast_session(const char *request_uri);
int rtsp_multicast_session_add_session(rtsp_session_t *sessp);
int rtsp_multicast_session_del_session(rtsp_session_t *sessp);
int rtsp_stopAll(void);


#endif
