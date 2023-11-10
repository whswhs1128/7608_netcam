/*!
*****************************************************************************
** \file      $gkprjrtcp.h
**
** \version	$id: rtcp.h 15-08-04  8月:08:1438655248 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __RTCP_H_
#define __RTCP_H_

#include <stdint.h>

/* RTCP packet types */
enum RTCPType {
    RTCP_FIR    = 192,
    RTCP_NACK, // 193
    RTCP_SMPTETC,// 194
    RTCP_IJ,   // 195
    RTCP_SR     = 200,
    RTCP_RR,   // 201
    RTCP_SDES, // 202
    RTCP_BYE,  // 203
    RTCP_APP,  // 204
    RTCP_RTPFB,// 205
    RTCP_PSFB, // 206
    RTCP_XR,   // 207
    RTCP_AVB,  // 208
    RTCP_RSI,  // 209
    RTCP_TOKEN,// 210
};

#define RTP_PT_IS_RTCP(x) (((x) >= RTCP_FIR && (x) <= RTCP_IJ) || \
                           ((x) >= RTCP_SR  && (x) <= RTCP_TOKEN))

typedef struct rtcp_common_header {
#ifdef ST_BIGENDIAN
	uint16_t version:2;
	uint16_t padbit:1;
	uint16_t rc:5;
	uint16_t packet_type:8;
#else
	uint16_t rc:5;
	uint16_t padbit:1;
	uint16_t version:2;
	uint16_t packet_type:8;
#endif
	uint16_t length:16;
} rtcp_common_header_t;

typedef struct report_block {
	uint32_t ssrc;
	uint32_t fl_cnpl;/*fraction lost + cumulative number of packet lost*/
	uint32_t ext_high_seq_num_rec; /*extended highest sequence number received */
	uint32_t interarrival_jitter;
	uint32_t lsr; /*last SR */
	uint32_t delay_snc_last_sr; /*delay since last sr*/
} report_block_t;

typedef struct rtcp_rr {
	rtcp_common_header_t ch;
	uint32_t ssrc;
	report_block_t rb[1];
} rtcp_rr_t;

/* SR or RR  packets */

typedef struct sender_info {
	uint32_t ntp_timestamp_msw;
	uint32_t ntp_timestamp_lsw;
	uint32_t rtp_timestamp;
	uint32_t senders_packet_count;
	uint32_t senders_octet_count;
} sender_info_t;

typedef struct rtcp_sr {
	rtcp_common_header_t ch;
	uint32_t ssrc;
	sender_info_t si;
//	report_block_t rb[1];
} rtcp_sr_t;

#define RTP_VERSION (2)
#define RTCP_SDES_CNAME "goke"
typedef struct sdes_item {
	uint8_t item_type;
	uint8_t len;
	char content[1];	
} sdes_item_t;

typedef struct sdes_chunk {
	uint32_t ssrc;
	sdes_item_t sdes;
} sdes_chunk_t;

#define sdes_chunk_get_csrc(c)	ntohl((c)->csrc)

typedef struct rtcp_sdes {
	rtcp_common_header_t ch;
	sdes_chunk_t chunk;
} rtcp_sdes_t;

typedef struct rtcp_bye {
	rtcp_common_header_t ch;
	uint32_t ssrc;
} rtcp_bye_t;

typedef struct rtp_stat_s {
	uint32_t packet_count;
	uint32_t octet_count;

	int seq_cycles_count;

	uint16_t first_seq_number;
	uint16_t prev_seq_number;

	int64_t last_rtcp_ntp_time;
	int64_t first_rtcp_ntp_time;
	uint32_t last_rtcp_timestamp;
	int64_t last_rtp_seq_number;
	uint32_t ssrc;

	int has_sr;
	int has_sdes;
	uint8_t sdes_item_type;
	uint8_t sdes_item_len;
	char sdes_content[256];

	int bye;
} rtp_stat_t;

#endif
