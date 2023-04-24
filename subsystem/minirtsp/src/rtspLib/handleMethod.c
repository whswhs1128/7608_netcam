/*!
*****************************************************************************
** \file      $gkprjhandleMethod.c
**
** \version	$id: handleMethod.c 15-08-04  8月:08:1438655132
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include "list.h"
#include "rtspServLib.h"
#include "handleMethod.h"
#include "rtp.h"
#include "parser.h"
#include "h264.h"
#include "base64.h"
#include "utils.h"
#include "netio.h"
#include "rtspServ.h"
#include "md5.h"


static const char *supported_methods =
	"OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER";

int initRtpHdr(rtp_hdr_t *h, int codec)
{
	if (!h)
		return -1;

	memset(h, 0, sizeof(rtp_hdr_t));

	h->v  = 2;
	h->p  = 0;
	h->x  = 0;
	h->cc = 0;
	h->m  = 0;
    h->pt = rtp_get_payload_type(codec);
    if(h->pt < 0)
        return -1;
	h->seq  = random();
	h->ts   = random();
	h->ssrc = random();

	return 0;
}

static int socket_get_port(int fd, uint16_t *port)
{
	struct sockaddr_in localaddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (getsockname(fd, (struct sockaddr *)&localaddr, &addrlen) < 0)
		return -1;

	*port = ntohs(localaddr.sin_port);

	return 0;
}

static int socket_get_localaddr(int fd, in_addr_t *s_addr)
{
	struct sockaddr_in localaddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (getsockname(fd, (struct sockaddr *)&localaddr, &addrlen) < 0)
		return -1;

	*s_addr = localaddr.sin_addr.s_addr;

	return 0;
}

static int socket_get_peeraddr(int fd, in_addr_t *s_addr)
{
	struct sockaddr_in peeraddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (getpeername(fd, (struct sockaddr *)&peeraddr, &addrlen) < 0)
		return -1;

	*s_addr = peeraddr.sin_addr.s_addr;

	return 0;
}

static int udp_socket_bind(char *ip, uint16_t port)
{
	struct sockaddr_in localaddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;

	memset(&localaddr, 0, addrlen);
	localaddr.sin_family = AF_INET;
	if (ip)
		localaddr.sin_addr.s_addr = inet_addr(ip);
	else
		localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *)&localaddr, addrlen) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

static int get_random_even_port(void)
{
	int i, r;
	int seed = 0;

	for (i = 0; i < 100; i++) {
		struct timeval tv;
		gettimeofday(&tv, NULL);

		seed += i + tv.tv_sec * 1000000 + tv.tv_usec;

		srand(seed);

		r = ((rand()  % 32767) >> 1) << 1;

		if (r < 1800)
			continue;

		return r;
	}

	return -1;
}

/**
 * Create socket pair for RTP & RTCP.
 * We just ask the kernel for one random port.
 * If the port is even, then usr for RTP, and RTCP's just plus one.
 * If the port is odd, then use for RTCP, and RTP's just minus one.
 */
static int setup_rtp_over_udp(rtsp_session_t *sessp, int media_type)
{
	int i;
	int ret;

	int rtp_fd = -1;
	int rtpport = -1;

	int rtcp_fd = -1;
	int rtcpport = -1;

	if (!sessp)
		return -1;

	RTSPTransportField *th = &sessp->transport[media_type];

	sessp->rtp[media_type].rtp_sock  = -1;
	sessp->rtp[media_type].rtcp_sock = -1;

#define TRY_GET_PORT_MAX	(100)
	for (i = 0; i < 100; i++) {
		rtpport = get_random_even_port();
		if (rtpport < 0)
			continue;

		rtcpport = rtpport + 1;

		rtp_fd = udp_socket_bind(NULL, rtpport);
		if (rtp_fd < 0)
			continue;

		/* Create second socket. */
		rtcp_fd = udp_socket_bind(NULL, rtcpport);
		if (rtcp_fd < 0) {
			close(rtp_fd);
			continue;
		}

		break;
	}

	if (i == TRY_GET_PORT_MAX)
		return -1;

	in_addr_t in_addr;
	if (socket_get_peeraddr(sessp->rtsp_snd_sock, &in_addr) < 0)
		goto fail;

	/* setup rtp socket */
	ret = socket_connect(rtp_fd, in_addr, th->client_port_min);
	if (ret < 0)
		goto fail;

	if (socket_set_nonblock(rtp_fd, 1) < 0)
		goto fail;

	/* setup rtcp socket */
	ret = socket_connect(rtcp_fd, in_addr, th->client_port_max);
	if (ret < 0)
		goto fail;

	if (socket_set_nonblock(rtcp_fd, 1) < 0)
		goto fail;

	sessp->rtp[media_type].rtp_sock  = rtp_fd;
	sessp->rtp[media_type].rtcp_sock = rtcp_fd;

	return 0;

fail:
	if (rtp_fd >= 0)
		close(rtp_fd);
	if (rtcp_fd >= 0)
		close(rtcp_fd);

	return -1;
}

#define TRY_GET_I_FRAME_MAX (256)

static int make_attribute_fmtp(rtsp_session_t *sessp, char *buf, int len)
{
	char b64_sps[512] = {0};
	char b64_pps[512] = {0};

	uint8_t sps[BUFSIZ]={0};           /* Begin of sequence parameter sets. */
	uint8_t pps[BUFSIZ]={0};           /* Begin of picture parameter sets. */
	int sps_len = 0;
	int pps_len = 0;
    int type = 0;
	uint8_t *data;
    int data_size ;
    frame_head_t head;

	int i;
	for (i = 0; i < TRY_GET_I_FRAME_MAX ; i++) 
    {
		if (sessp->abort)
			return -1;
        data = NULL;
        data_size = 0;
        //RTSP_INFO("[%s] fmtp call read_p to get sps & pps\n", __func__);
		int readlen = sessp->ops->read_p(sessp->handle, (void **)&data, (size_t *)&data_size, 0,&head);
		if (readlen < 0 )
        {
			return -1;
		} 
        else if (readlen == 0)
        {
			usleep(10 * 1000);
			continue;
		}

		if (head.frame_type != GOKE_I_FRAME)
			continue;
        else
            RTSP_WARNING("%s find the %d is I frame\n", sessp->url->path, i);
        
        if (head.payload != sessp->mdescs[MEDIA_TYPE_VIDEO].codec)
        {
            RTSP_ERR("payload:%d, codec: %d\n", head.payload, sessp->mdescs[MEDIA_TYPE_VIDEO].codec);
			continue;
        }

        if(RTP_CODEC_ID_H264 == sessp->mdescs[MEDIA_TYPE_VIDEO].codec)
        {
        	RTSP_INFO("rtsp data head payload is h264\n");
		    type = h264_get_frame_type(data, data_size);
            if (type != BLOCK_FLAG_TYPE_I)
            {
                RTSP_WARNING("type != BLOCK_FLAG_TYPE_I\n");
    			continue;
            }
            sps_len = h264_get_sps(data, data_size, sps, sizeof(sps));
    		if (sps_len <= 0)
            {
                RTSP_WARNING("sps_len <= 0\n");
    			continue;
            }
            pps_len = h264_get_pps(data, data_size, pps, sizeof(pps));
    		if (pps_len <= 0)
            {
                RTSP_WARNING("pps_len <= 0\n");
                continue;
            }
        }
        else if (RTP_CODEC_ID_H265 == sessp->mdescs[MEDIA_TYPE_VIDEO].codec)
        {
        	RTSP_INFO("rtsp data head payload is h265\n");
            type = h265_get_frame_type(data, data_size);
            if (type != BLOCK_FLAG_TYPE_I)
            {
                RTSP_WARNING("type != BLOCK_FLAG_TYPE_I\n");
    			continue;
            }
		    sps_len = h265_get_sps(data, data_size, sps, sizeof(sps));            
    		if (sps_len <= 0)
            {
                RTSP_WARNING("sps_len <= 0\n");
    			continue;
            }
            pps_len = h265_get_pps(data, data_size, pps, sizeof(pps));
    		if (pps_len <= 0)
            {
                RTSP_WARNING("pps_len <= 0\n");
                continue;
            }
        }

#if 0
		int j;
		printf("sps: ");
		for (j = 0; j < sps_len; j++)
			printf("0x%02x ", sps[j]);
		printf("\n");

		printf("pps: ");
		for (j = 0; j < pps_len; j++)
			printf("0x%02x ", pps[j]);
		printf("\n");
#endif

		if (sessp->ops->seek)
        {
			sessp->ops->seek(sessp->handle, 0, SEEK_SET);	/* reset to the very begining */
		} 
        else 
        {
			sessp->saved_keyframe = malloc(data_size);
			if (!sessp->saved_keyframe)
				return -1;

			memcpy(sessp->saved_keyframe, data, head.frame_size);
			sessp->saved_keyframe_length = head.frame_size;
			memcpy(&sessp->saved_head, &head, sizeof(frame_head_t));
		}
		break;
	}

	if (i == TRY_GET_I_FRAME_MAX)
    {
		RTSP_ERR("%s cannot find I frame!\n", sessp->url->path);
		return -1;
	}
    snprintf(buf, len,
				"a=fmtp:%d packetization-mode=1;"
				"profile-level-id=%02X%02X%02X;"
				"sprop-parameter-sets=%s,%s\r\n",
				sessp->rtp[MEDIA_TYPE_VIDEO].rtp_hdr.pt, sps[4 + 1], sps[4 + 2], sps[4 + 3],
				av_base64_encode(b64_sps, sizeof(b64_sps), sps + 4, sps_len - 4),
				av_base64_encode(b64_pps, sizeof(b64_pps), pps + 4, pps_len - 4));
	return 0;
}

/**
 * Return the size used actually.
 */
static int make_sdp_description(rtsp_session_t *sessp, char *buf, int size)
{
	char *p = NULL;

	p = buf;
	sprintf(p, "v=0\r\n");

	struct in_addr addr;
	memset(&addr, 0, sizeof(struct in_addr));
	socket_get_localaddr(sessp->rtsp_rcv_sock, &addr.s_addr);

	p += strlen(p);
	//sprintf(p, "o=- %ld %d IN IP4 %s\r\n", time(NULL), 1, inet_ntoa(addr));
	sprintf(p, "o=- 1418744474352605 1418744474352605 IN IP4 %s\r\n", inet_ntoa(addr));

	p += strlen(p);
	sprintf(p, "s=Media Presentation\r\n");

	p += strlen(p);
	sprintf(p, "e=NONE\r\n");

	p += strlen(p);
	sprintf(p, "b=AS:5100\r\n");

	p += strlen(p);
	sprintf(p, "i=mpeg4\r\n");

    RTSP_INFO("[%s] call describe\n", __func__);
	sessp->ndesc = sessp->ops->describe(sessp->url->path, sessp->url->query, sessp->mdescs, NELEMS(sessp->mdescs));
	if (sessp->ndesc <= 0) {
		RTSP_ERR("%s: get media describe failed\n", __func__);
		return -1;
	}
    
	if (sessp->multicast) {
        RTSP_INFO("add multicast\n");
		char addr[64];
		uint16_t port;
		if (rtp_get_multicast_addr(sessp, MEDIA_TYPE_VIDEO, addr, sizeof(addr), &port, NULL) < 0)
			return -1;
        
        p += strlen(p);
		sprintf(p, "c=IN IP4 %s\r\n", addr);
	}

	p += strlen(p);
	sprintf(p, "t=0 0\r\n");

	p += strlen(p);
	sprintf(p, "a=tool:LIVE555 Streaming Media v2009.09.28\r\n");

    p += strlen(p);
	sprintf(p, "a=type:broadcast\r\n");

    p += strlen(p);
	sprintf(p, "a=control:*\r\n");

	p += strlen(p);
	sprintf(p, "a=range:npt=0-\r\n");

    p += strlen(p);
	sprintf(p, "a=x-qt-text-nam:RTSP/RTP stream from Network Video Server\r\n");

    p += strlen(p);
	sprintf(p, "a=x-qt-text-inf:mpeg4\r\n");

    p += strlen(p);
	sprintf(p, "c=IN IP4 0.0.0.0\r\n");

    p += strlen(p);
	sprintf(p, "a=range:npt=now-\r\n");

    int rtp_pt;
	int i;
    const char *media_type;
    const char *codec_name;
    MediaDesc *desc;
	for (i = 0; i < sessp->ndesc; i++) 
    {
		desc = &sessp->mdescs[i];
        if (desc->media != MEDIA_TYPE_VIDEO && desc->media != MEDIA_TYPE_AUDIO)
        {
			RTSP_ERR("unknown media type:%d\n", desc->media);
			return -1;
		}
        rtp_pt = rtp_get_payload_type(desc->codec);
        codec_name = rtp_codec_name(desc->codec);
        switch (desc->media)
        {
            case MEDIA_TYPE_VIDEO : media_type = "video"      ; break;
            case MEDIA_TYPE_AUDIO : media_type = "audio"      ; break;
            default               : media_type = "application"; break;
        }

        p += strlen(p);
		sprintf(p, "m=%s 0 RTP/AVP %d\r\n", media_type, rtp_pt);

		if (sessp->multicast)
        {
			char addr[64];
			uint16_t port;
			if (rtp_get_multicast_addr(sessp, desc->media, addr, sizeof(addr), &port, NULL) < 0)
				return -1;
            
			p += strlen(p);
			sprintf(p, "c=IN IP4 %s\r\n", addr);
		}
        //p += strlen(p);
        //sprintf(p, "c=IN IP4 0.0.0.0\r\n");
       
		p += strlen(p);
		//sprintf(p, "a=control:track%d\r\n", desc->media);
		sprintf(p, "a=control:trackID=%d\r\n", desc->media);


		if (desc->media == MEDIA_TYPE_VIDEO)
        {
            p += strlen(p);
            sprintf(p, "b=AS:5000\r\n");
			p += strlen(p);
            sprintf(p, "a=rtpmap:%d %s/%d\r\n", rtp_pt, codec_name, desc->clockrate);

			if (desc->record) 
            {
				p += strlen(p);
				sprintf(p, "a=x-onvif-track:VIDEO001\r\n");
			}

			char fmtp[1024];
            RTSP_INFO("[%s] call make_attribute_fmtp\n", __func__);
			if (make_attribute_fmtp(sessp, fmtp, sizeof(fmtp)) < 0)
				return -1;

			p += strlen(p);
			sprintf(p, "%s", fmtp);
        
            p += strlen(p);
            sprintf(p, "a=framesize:%d %d-%d\r\n", rtp_pt, desc->video_width, desc->video_height);
			p += strlen(p);
			sprintf(p, "a=x-dimensions: %d, %d\r\n", desc->video_width, desc->video_height);
			p += strlen(p);
			sprintf(p, "a=x-framerate: %d\r\n", desc->video_fps);
		}
        else if (desc->media == MEDIA_TYPE_AUDIO) 
        {
            p += strlen(p);
            sprintf(p, "b=AS:50\r\n");
			p += strlen(p);
			//sprintf(p, "a=rtpmap:%d %s/%d/%d\r\n", rtp_pt, desc->codec_name, desc->audio_samplerate, desc->audio_channle_num);
            sprintf(p, "a=rtpmap:%d %s/%d\r\n", rtp_pt, codec_name, desc->audio_samplerate);

			if (desc->record) {
				p += strlen(p);
				sprintf(p, "a=x-onvif-track:AUDIO001\r\n");
			}
			//p += strlen(p);
			//sprintf(p, "a=range:npt=0-\r\n");
            p += strlen(p);
            //sprintf(p, "a=Media_header:MEDIAINFO=494D4B48010100000400010000000000000000000000000000000000000000000000000000000000;\r\n");
            sprintf(p, "a=Media_header:MEDIAINFO=494D4B48010100000400010010710110401F000000FA000000000000000000000000000000000000;\r\n");
            
            p += strlen(p);
            sprintf(p, "a=appversion:1.0\r\n");
		}
	}

	p += strlen(p);
	return (p - buf);           /* Return length of sdp description. */
}

static int make_sdp_description2(rtsp_session_t *sessp, char *buf, int size)
{
	char *p = NULL;

	p = buf;
	sprintf(p, "v=0\r\n");

	struct in_addr addr;
	memset(&addr, 0, sizeof(struct in_addr));
	socket_get_localaddr(sessp->rtsp_rcv_sock, &addr.s_addr);

	p += strlen(p);
	sprintf(p, "o=- %ld %d IN IP4 %s\r\n", time(NULL), 1, inet_ntoa(addr));

	p += strlen(p);
	sprintf(p, "s=Media Presentation\r\n");


	sessp->ndesc = sessp->ops->describe(sessp->url->path, sessp->url->query, sessp->mdescs, NELEMS(sessp->mdescs));
	if (sessp->ndesc <= 0) {
		printf("%s: get media describe failed\n", __func__);
		return -1;
	}

	if (sessp->multicast) {
		p += strlen(p);

		char addr[64];
		uint16_t port;
		if (rtp_get_multicast_addr(sessp, MEDIA_TYPE_VIDEO, addr, sizeof(addr), &port, NULL) < 0)
			return -1;

		sprintf(p, "c=IN IP4 %s\r\n", addr);
	}

	p += strlen(p);
	sprintf(p, "t=0 0\r\n");

	int rtp_pt_video;
	int rtp_pt_audio;

	int i;
	for (i = 0; i < sessp->ndesc; i++) {
		MediaDesc *desc = &sessp->mdescs[i];
		if (desc->media == MEDIA_TYPE_VIDEO) {
			if (desc->codec == RTP_CODEC_ID_H264)
				rtp_pt_video = RTP_PT_VIDEO_H264;
			else if (desc->codec == RTP_CODEC_ID_H265)
				rtp_pt_video = RTP_PT_VIDEO_H265;
			else
				return -1;
			p += strlen(p);
			sprintf(p, "m=video 0 RTP/AVP %d\r\n", rtp_pt_video);

			if (sessp->multicast) {
				p += strlen(p);

				char addr[64];
				uint16_t port;
				if (rtp_get_multicast_addr(sessp, MEDIA_TYPE_VIDEO, addr, sizeof(addr), &port, NULL) < 0)
					return -1;

				sprintf(p, "c=IN IP4 %s\r\n", addr);
			}

			p += strlen(p);
            sprintf(p, "c=IN IP4 0.0.0.0\r\n");
			p += strlen(p);
			if(desc->codec == RTP_CODEC_ID_H264)				
				sprintf(p, "a=rtpmap:%d H264/90000\r\n", rtp_pt_video);
			else if(desc->codec == RTP_CODEC_ID_H265)
				sprintf(p, "a=rtpmap:%d H265/90000\r\n", rtp_pt_video);
			p += strlen(p);
			sprintf(p, "a=control:trackID=%d\r\n", MEDIA_TYPE_VIDEO);
			if (desc->record) {
				p += strlen(p);
				sprintf(p, "a=x-onvif-track:VIDEO001\r\n");
			}

			char fmtp[1024];
			if (make_attribute_fmtp(sessp, fmtp, sizeof(fmtp)) < 0)
				return -1;

			p += strlen(p);
			sprintf(p, "%s", fmtp);
		} else if (desc->media == MEDIA_TYPE_AUDIO) {
			if (desc->codec == RTP_CODEC_ID_PCM_MULAW)
				rtp_pt_audio = RTP_PT_AUDIO_PCMU;
			else if (desc->codec == RTP_CODEC_ID_PCM_ALAW)
				rtp_pt_audio = RTP_PT_AUDIO_PCMA;
			else
				rtp_pt_audio = RTP_PT_AUDIO_PCMA;

			p += strlen(p);
			sprintf(p, "m=audio 0 RTP/AVP %d\r\n", rtp_pt_audio);

			if (sessp->multicast) {
				p += strlen(p);

				char addr[64];
				uint16_t port;
				if (rtp_get_multicast_addr(sessp, MEDIA_TYPE_AUDIO, addr, sizeof(addr), &port, NULL) < 0)
					return -1;

				sprintf(p, "c=IN IP4 %s\r\n", addr);
			}

			p += strlen(p);

            #if 1
			sprintf(p, "a=rtpmap:%d %s/%d/%d\r\n", rtp_pt_audio,
					(rtp_pt_audio == RTP_PT_AUDIO_PCMU) ? "PCMU" : "PCMA", desc->audio_samplerate, desc->audio_channle_num);
            #else
			sprintf(p, "a=rtpmap:%d %s/8000/1\r\n", rtp_pt_audio, "L16");
            #endif

			p += strlen(p);
			sprintf(p, "a=control:trackID=%d\r\n", MEDIA_TYPE_AUDIO);

			if (desc->record) {
				p += strlen(p);
				sprintf(p, "a=x-onvif-track:AUDIO001\r\n");
			}

			p += strlen(p);
			sprintf(p, "a=range:npt=0-\r\n");
		} else {
			printf("%s: unknown media type %d\n", __func__, __LINE__);
			return -1;
		}
	}

	p += strlen(p);
	return (p - buf);           /* Return length of sdp description. */
}

static int rtsp_datetime(char *buf, int len)
{
	time_t ti;
	struct tm tm;

	ti = time(NULL);
	gmtime_r(&ti, &tm);
	strftime(buf, len, "%a, %d %b %Y %H:%M:%S", &tm);

	return 0;
}

int rtsp_cmd_options(rtsp_session_t *sessp)
{
	char date[32];
	rtsp_datetime(date, sizeof(date));

	char buf[2048];
	snprintf(buf, sizeof(buf),
			"RTSP/1.0 200 %s\r\n"
			"CSeq: %d\r\n"
			"Date: %s GMT\r\n"
			"Public: %s\r\n"
			"\r\n", rtsp_state_message(200),
			sessp->cseq, date, supported_methods);

	/* Send response message. */
	if (rtsp_send_reply(sessp, buf, strlen(buf)) < 0)
		return -1;

	return 0;
}

int rtsp_cmd_describe(rtsp_session_t *sessp)
{
	char buf[2048] = {0};
	char date[32]  = {0};
	rtsp_datetime(date, sizeof(date));
	int secflag = 0;;
	users_s stUsers;

	memset(&stUsers, 0, sizeof(stUsers));
	if(sessp->ops->users_info)
		sessp->ops->users_info(&secflag, &stUsers);
	sessp->user.security = secflag;
	if(sessp->user.security && sessp->user.flag == 0)
	{
	    RTSP_INFO("[%s:%d] user need security\n", __func__, __LINE__);
        /////////// use md5 create nonce start
        char nonce[64]={0}, *nonce_p = nonce;
        int i, ret;
        MD5_CTX md5;
        MD5Init(&md5);
        unsigned char decrypt[16];
        MD5Update(&md5,date,strlen((char *)date));
        MD5Final(&md5,decrypt);
        memset(nonce, 0, sizeof(nonce));
        for(i=0;i<16;i++)
        {
            sprintf(nonce_p, "%02x",decrypt[i]);
            nonce_p += 2;
        }
        /////////// use md5 create nonce end
		snprintf(buf, sizeof(buf),
				"RTSP/1.0 401 %s\r\n"
				"CSeq: %d\r\n"
				"Date: %s GMT\r\n"
				"WWW-Authenticate:Basic realm=\"GK_RTSP_Server\"\r\n"
				//"WWW-Authenticate:Digest realm=\"GK_RTSP_Server,nonce=\"%s\"\r\n"
				"\r\n",
				rtsp_state_message(401), sessp->cseq, date);
				//rtsp_state_message(401), sessp->cseq, date, nonce);

		goto response;
	}
	char sdp[1024];
	int sdp_len = 0;

    bzero(sdp, sizeof(sdp));
	/* Send response message. */
	sdp_len = make_sdp_description(sessp, sdp, sizeof(sdp));
	if (sdp_len < 0) {
		RTSP_ERR("make sdp description failed\n");
		return -1;
	}

	snprintf(buf, sizeof(buf),
			"RTSP/1.0 200 %s\r\n"
			"CSeq: %d\r\n"
			"Date: %s GMT\r\n"
			"Content-Type: application/sdp\r\n"
			"Content-Length: %d\r\n"
			"\r\n"
			"%s", rtsp_state_message(200),
			sessp->cseq, date, sdp_len, sdp);

response:
	/* Send response message. */
	if (rtsp_send_reply(sessp, buf, strlen(buf)) < 0)
		return -1;

	return 0;
}

static int rtsp_make_transport(rtsp_session_t *sessp, int media_type, RTSPTransportField *th, char *transport, int len)
{
	int ttl;
	uint16_t rtp_port = 0;
	uint16_t rtcp_port = 0;
	char     rtp_addr[64];

	switch (th->lower_transport) {
	case RTSP_LOWER_TRANSPORT_TCP:
		snprintf(transport, len, "RTP/AVP/TCP;unicast;"
				"interleaved=%d-%d",
				th->interleaved_min, th->interleaved_max);
		break;
	case RTSP_LOWER_TRANSPORT_UDP:
		socket_get_port(sessp->rtp[media_type].rtp_sock, &rtp_port);
		socket_get_port(sessp->rtp[media_type].rtcp_sock, &rtcp_port);

		snprintf(transport, len, "RTP/AVP;unicast;"
				"client_port=%d-%d;server_port=%d-%d",
				th->client_port_min, th->client_port_max,
				rtp_port, rtcp_port);
		break;
	case RTSP_LOWER_TRANSPORT_UDP_MULTICAST:
		if (rtp_get_multicast_addr(sessp, media_type, rtp_addr,
					sizeof(rtp_addr), &rtp_port, &ttl) < 0)
			return -1;
		rtcp_port = rtp_port + 1;

		snprintf(transport, len, "RTP/AVP;multicast;destination=%s;"
				"port=%hu-%hu;ttl=%d", rtp_addr,
				rtp_port, rtcp_port, ttl);
		break;
	default:
		return -1;
	}

	return 0;
}

int rtsp_cmd_setup(rtsp_session_t *sessp)
{
	int media_type = rtsp_get_media_type(sessp->recv_buf);
	if (media_type < 0)
		return -1;

    RTSP_INFO("setup, trackID=%d", media_type);
	if (sessp->session_id == 0)
		sessp->session_id = (unsigned int)rtsp_random64();

	int i;
	for (i = 0; i < sessp->ndesc; i++) {
		MediaDesc *desc = &sessp->mdescs[i];

		if ((media_type == MEDIA_TYPE_VIDEO && desc->media == MEDIA_TYPE_VIDEO) ||
		    (media_type == MEDIA_TYPE_AUDIO && desc->media == MEDIA_TYPE_AUDIO)) {
			if (initRtpHdr(&sessp->rtp[media_type].rtp_hdr, desc->codec) < 0)
				return -1;
			break;
		}
	}

	RTSPTransportField *th = &sessp->transport[media_type];
	if (th->lower_transport == RTSP_LOWER_TRANSPORT_UDP) {
		if (setup_rtp_over_udp(sessp, media_type) < 0)
			return -1;
	}

	char transport[256];
	rtsp_make_transport(sessp, media_type, th, transport, sizeof(transport));

	char date[32];
	rtsp_datetime(date, sizeof(date));

	char buf[2048];
	snprintf(buf, sizeof(buf),
			"RTSP/1.0 200 %s\r\n"
			"CSeq: %d\r\n"
			"Date: %s GMT\r\n"
			"Session: %08X\r\n"
			"Transport: %s\r\n"
			"\r\n", rtsp_state_message(200),
			sessp->cseq, date, sessp->session_id, transport);

	if (rtsp_send_reply(sessp, buf, strlen(buf)) < 0) {
		RTSP_ERR("send replay failed: %s\n", buf);
		return -1;
	}

	sessp->rtp[media_type].enable = 1;

	return 0;
}

int rtsp_cmd_teardown(rtsp_session_t *sessp)
{
	char date[32];
	rtsp_datetime(date, sizeof(date));

	char buf[2048];
	snprintf(buf, sizeof(buf),
			"RTSP/1.0 200 %s\r\n"
			"CSeq: %d\r\n"
			"Date: %s GMT\r\n"
			"\r\n", rtsp_state_message(200),
			sessp->cseq, date);

	/* Send response message. */
	if (rtsp_send_reply(sessp, buf, strlen(buf)) < 0)
		return -1;

	sessp->abort = 1;	/* abort this session*/

	return 0;
}
static int64_t av_gettime_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
}
int rtsp_cmd_play(rtsp_session_t *sessp)
{
	char date[32];
	rtsp_datetime(date, sizeof(date));

	char buf[2048];
	snprintf(buf, sizeof(buf),
			"RTSP/1.0 200 %s\r\n"
			"CSeq: %d\r\n"
			"Date: %s GMT\r\n"
            "Range: npt=0.000-\r\n"
			"Session: %08X\r\n"
			"\r\n", rtsp_state_message(200),
			sessp->cseq, date, sessp->session_id);

    if (rtsp_send_reply(sessp, buf, strlen(buf)) < 0) {
		RTSP_ERR("send replay failed: %s\n", buf);
		return -1;
	}

	if (sessp->cmdq) {
		RTSPMessageHeader *h = malloc(sizeof(RTSPMessageHeader));
		if (!h)
			return -1;
		*h = sessp->message_headers;
		j_queue_put(sessp->cmdq, h);
	}

	if (sessp->state != RTSP_STATE_PLAYING && !sessp->pause) {
		if (sessp->mdescs[MEDIA_TYPE_VIDEO].record) {
			if (sessp->message_headers.scale >= 0) {
				sessp->range_start = sessp->message_headers.range_start;
				sessp->range_end   = sessp->message_headers.range_end;
			} else {
				sessp->range_start = sessp->message_headers.range_end;
				sessp->range_end   = sessp->message_headers.range_start;
			}

			sessp->cmdq = j_queue_new();
			if (!sessp->cmdq)
				return -1;

			RTSPMessageHeader *h = malloc(sizeof(RTSPMessageHeader));
			if (!h)
				return -1;
			*h = sessp->message_headers;
			j_queue_put(sessp->cmdq, h);

			if (pthread_create(&sessp->send_data_tid, NULL, rtsp_replay_thread, sessp) != 0) {
				void *d = j_queue_get(sessp->cmdq, 0);
				if (d)
					free(d);
				return -1;
			}
		} else {
			switch (sessp->transport[MEDIA_TYPE_VIDEO].lower_transport) {	/* FIXME */
			case RTSP_LOWER_TRANSPORT_TCP:
			case RTSP_LOWER_TRANSPORT_UDP:
			case RTSP_LOWER_TRANSPORT_SHMEM:
                sessp->rtp[MEDIA_TYPE_VIDEO].pts = sessp->rtp[MEDIA_TYPE_AUDIO].pts = av_gettime_ms();
				/* Create to prepare sending RTP & RTCP data. */
				if (pthread_create(&sessp->send_data_tid, NULL, rtsp_stream_thread, sessp) != 0)
					return -1;
				break;
			case RTSP_LOWER_TRANSPORT_UDP_MULTICAST:
				{
					rtsp_session_t *mcast_sessp;
					mcast_sessp = rtsp_find_multicast_session(sessp->url->path);
					if (!mcast_sessp) {
						mcast_sessp = rtsp_create_multicast_session(sessp->request_uri);
						if (!mcast_sessp) {
							RTSP_ERR("rtsp_create_multicast_session failed\n");
							return -1;
						}
					}

					if (rtsp_multicast_session_add_session(sessp) < 0) {
						RTSP_ERR("rtsp_multicast_session_add_session failed\n");
						return -1;
					}
				}
				break;
			default:
				RTSP_ERR("%s: unknown rtsp transport type: %d\n", __func__,
						sessp->transport[MEDIA_TYPE_VIDEO].lower_transport);
				return -1;
			}
		}
	}

	sessp->pause = 0;

	return 0;
}

int rtsp_cmd_pause(rtsp_session_t *sessp)
{
	char date[32];
	rtsp_datetime(date, sizeof(date));

	char buf[2048];
	snprintf(buf, sizeof(buf),
			"RTSP/1.0 200 %s\r\n"
			"CSeq: %d\r\n"
			"Date: %s GMT\r\n"
			"Session: %08X\r\n"
			"\r\n", rtsp_state_message(200),
			sessp->cseq, date, sessp->session_id);

	/* Send response message. */
	if (rtsp_send_reply(sessp, buf, strlen(buf)) < 0)
		return -1;

	sessp->pause = 1;

	return 0;
}

int rtsp_cmd_get_parameter(rtsp_session_t *sessp)
{
	char date[32];
	rtsp_datetime(date, sizeof(date));

	char buf[2048];
	snprintf(buf, sizeof(buf),
			"RTSP/1.0 200 %s\r\n"
			"CSeq: %d\r\n"
			"Date: %s GMT\r\n"
			"Session: %08X\r\n"
			"\r\n", rtsp_state_message(200),
			sessp->cseq, date, sessp->session_id);

	/* Send response message. */
	if (rtsp_send_reply(sessp, buf, strlen(buf)) < 0)
		return -1;

	return 0;
}

int rtsp_cmd_set_parameter(rtsp_session_t *sessp)
{
	char date[32];
	rtsp_datetime(date, sizeof(date));

	char buf[2048];
	snprintf(buf, sizeof(buf),
			"RTSP/1.0 200 %s\r\n"
			"CSeq: %d\r\n"
			"Date: %s GMT\r\n"
			"Session: %08X\r\n"
			"\r\n", rtsp_state_message(200),
			sessp->cseq, date, sessp->session_id);

	/* Send response message. */
	if (rtsp_send_reply(sessp, buf, strlen(buf)) < 0)
		return -1;

	return 0;
}
