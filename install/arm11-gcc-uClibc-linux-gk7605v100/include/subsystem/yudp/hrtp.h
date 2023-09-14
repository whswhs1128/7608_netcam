
/*
 * hrtp api define.
 *
 * Author : Heyong
 * Date   : 2015.11.20
 * Copyright (c) 2015 Harric He(Heyong)
 *
 */
#ifndef _HRTP_H__
#define _HRTP_H__


// include
#include "yudp_utility.h"

#define HRTP_FRAGMENT_SIZE		    1472//must be even number - UDP.
#define HRTP_MAX_PAYLOAD_SIZE	    (HRTP_FRAGMENT_SIZE - 16)	/* minus sizeof(hrtp_hdr_t) */

#define HRTP_TYPE_AUDIO             0
#define HRTP_TYPE_VEDIO             1
#define HRTP_TYPE_VEDIO_I           2

#define HRTP_ENABLE_CRC32           0   /* 0->disable. 1->enable. */

typedef struct hrtp_hdr_s {
	unsigned char flag[4];              /* hrtp flag "H.R.T.P" */
	unsigned char random_num;           /* hrtp random num */
	unsigned char type;                 /* hrtp type */
	unsigned short seq;			        /* sequence number */
	unsigned short totle;		        /* totle sequence number */
	unsigned short payload_len;	        /* payload len (0~HRTP_MAX_PAYLOAD_SIZE)*/
    unsigned int  hrtp_crc32;           /* crc32 */
} hrtp_hdr_t;

#define HRTP_MAX_PACKET_SIZE

typedef struct hrtp_fragment_s {
	hrtp_hdr_t hrtp_hdr;
	char fm_data[HRTP_MAX_PAYLOAD_SIZE];/* malloc(fm_count*HRTP_MAX_SIZE) */
} hrtp_fragment_t;

typedef struct hrtp_packet_s {
	unsigned int fm_count;              /* fm = fragment, fms = fragments */
	unsigned int fms_data_length;       /* fragment_count*fragment's payload */
	unsigned int fms_data_size;         /* fragment_count*HRTP_MAX_SIZE */
	char *fms_data;                     /* malloc(fm_count*HRTP_MAX_SIZE) */
} hrtp_packet_t;


//utils functions.
int fragment_equal(hrtp_fragment_t * fm1, hrtp_fragment_t * fm2);
int fragment_hrtp(hrtp_fragment_t * fm);
int hrtp_fragment_printf(hrtp_fragment_t * fragment);


hrtp_packet_t* hrtp_packet_create(int length);
void hrtp_packet_destroy(hrtp_packet_t* packet);

int hrtp_packet_resize(hrtp_packet_t* packet, int new_length);
int hrtp_packet_convert_data(hrtp_packet_t* packet, char *data, int len, char hrtp_type);


#if 0
typedef struct hrtp_recombine_s {
	unsigned int length;                /* recombine data length */
	unsigned int data_size;             /* recombine data data_size */

	//unsigned int have_full_packet;      /* 1->you can call hrtp_recombine_get_data to get new packet. */

	unsigned char pkt_id;               /* packet fragment 's random_num */
	unsigned char pkt_type;             /* packet fragment 's type */
	unsigned short pkt_totle;           /* packet fragment 's totle */

	unsigned int pkt_fms_data_size;     /* array of packet fragments's size */
	char *pkt_fms_data;                 /* array of packet fragments */
	char *data;                         /* recombine data */
} hrtp_recombine_t;



hrtp_packet_t* hrtp_packet_create(int length);
void hrtp_packet_destroy(hrtp_packet_t* packet);

int hrtp_packet_resize(hrtp_packet_t* packet, int new_length);
int hrtp_packet_convert_data(hrtp_packet_t* packet, char *data, int len, char hrtp_type);

hrtp_recombine_t* hrtp_recombine_create();
void hrtp_recombine_destroy(hrtp_recombine_t* recombine);

int hrtp_recombine_add(hrtp_recombine_t* recombine, hrtp_fragment_t * fragment);
char * hrtp_recombine_get_data(hrtp_recombine_t *recombine, int *length, int *type);
int hrtp_recombine_print(hrtp_recombine_t *recombine);


#if 0

unsigned char rcv_buffer[YUDP_HOST_MAX_RECV_LENGTH];
char *data  = "hheeyyoonngg";//
char *data1 = "112233445566";//
int len = strlen(data);
hrtp_packet_t* packet = hrtp_packet_create(len);
hrtp_packet_convert_data(packet, data, len, 1);

hrtp_packet_t* packet1 = hrtp_packet_create(len);
hrtp_packet_convert_data(packet1, data1, len, 1);

hrtp_fragment_t * fragments = (hrtp_fragment_t *)packet->fms_data;
hrtp_fragment_t * fragments1 = (hrtp_fragment_t *)packet1->fms_data;


hrtp_recombine_t* recomb = hrtp_recombine_create(&fragments[3]);
hrtp_recombine_add(recomb, &fragments[5]);
hrtp_recombine_add(recomb, &fragments[1]);
hrtp_recombine_add(recomb, &fragments[4]);
hrtp_recombine_add(recomb, &fragments[0]);

hrtp_recombine_add(recomb, &fragments[4]);
//hrtp_recombine_add(recomb, &fragments1[4]);
hrtp_recombine_add(recomb, &fragments[2]);

hrtp_recombine_print(recomb);

int length = 0;
char *pkt = hrtp_recombine_get_data(recomb, &length);
if(pkt != NULL)
printf("data(%d): %s\n",length , pkt);

hrtp_recombine_add(recomb, &fragments1[0]);
hrtp_recombine_add(recomb, &fragments1[3]);
hrtp_recombine_add(recomb, &fragments1[5]);
hrtp_recombine_add(recomb, &fragments1[1]);
hrtp_recombine_add(recomb, &fragments1[4]);
hrtp_recombine_add(recomb, &fragments1[2]);

hrtp_recombine_print(recomb);

length = 0;
char *pkt1 = hrtp_recombine_get_data(recomb, &length);
if(pkt1 != NULL)
printf("data(%d): %s\n",length , pkt1);

#endif
#endif


#endif /* _HRTP_H__ */

