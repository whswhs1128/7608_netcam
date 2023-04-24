
/*
 * hrtp api define.
 *
 * Author : Heyong
 * Date   : 2015.11.20
 * Copyright (c) 2015 Harric He(Heyong)
 *
 */

#include "stdafx.h"

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif


#include "utility_api.h"

#include "hrtp.h"

static unsigned int hrtp_random_num (void)
{
#if 0
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);
    return (1+(int) (300.0*rand()/(RAND_MAX+1.0)));
#else
    static int random_num = 1;
	random_num++;
	if(random_num == 256)
		random_num = 1;
	return random_num;
#endif
}

int hrtp_fragment_printf(hrtp_fragment_t * fragment)
{
    Y_INFO("fragment[random_num:%d type:%d seq:%d totle:%d payload_len:%d] data = %c%c\n",
                    fragment->hrtp_hdr.random_num,
                    fragment->hrtp_hdr.type,
                    fragment->hrtp_hdr.seq,
                    fragment->hrtp_hdr.totle,
                    fragment->hrtp_hdr.payload_len,
                    fragment->fm_data[0], fragment->fm_data[1]);
    return 0;
}

int fragment_equal(hrtp_fragment_t * fm1, hrtp_fragment_t * fm2)
{
    if((fm1->hrtp_hdr.random_num == fm2->hrtp_hdr.random_num)
            && (fm1->hrtp_hdr.type == fm2->hrtp_hdr.type)
            && (fm1->hrtp_hdr.seq == fm2->hrtp_hdr.seq)
            && (fm1->hrtp_hdr.totle == fm2->hrtp_hdr.totle)
            && (fm1->hrtp_hdr.payload_len == fm2->hrtp_hdr.payload_len))
    {
        return 1;//equal.
    }
    return 0;
}

int fragment_hrtp(hrtp_fragment_t * fm)
{
    if((fm->hrtp_hdr.flag[0] == 'H')
            && (fm->hrtp_hdr.flag[1] == 'R')
            && (fm->hrtp_hdr.flag[2] == 'T')
            && (fm->hrtp_hdr.flag[3] == 'P'))
    {
        return 1;//equal.
    }
    return 0;
}


hrtp_packet_t* hrtp_packet_create(int length)
{
    if(sizeof(hrtp_fragment_t) != HRTP_FRAGMENT_SIZE)
    {
        Y_ERROR("hrtp_packet_create: sizeof(hrtp_fragment_t)-%d != HRTP_FRAGMENT_SIZE-%d.\n", sizeof(hrtp_fragment_t), HRTP_FRAGMENT_SIZE);
        return NULL;
    }

    hrtp_packet_t* packet = NULL;
    Y_SMALLOC(packet, sizeof(hrtp_packet_t), hrtp_packet_t);

    packet->fm_count = (length + HRTP_MAX_PAYLOAD_SIZE - 1) / HRTP_MAX_PAYLOAD_SIZE;
    Y_INFO("hrtp_packet_create: length = %d fm_count = %d.\n", length, packet->fm_count);

    int new_fms_data_len = sizeof(hrtp_fragment_t)*packet->fm_count;

    Y_SMALLOC(packet->fms_data, new_fms_data_len, char);
    packet->fms_data_size = new_fms_data_len;
    packet->fms_data_length = 0;
	return packet;
}

void hrtp_packet_destroy(hrtp_packet_t* packet)
{
    Y_SFREE(packet->fms_data);
    Y_SFREE(packet);
}

int hrtp_packet_resize(hrtp_packet_t* packet, int new_length)
{
    packet->fm_count = (new_length + HRTP_MAX_PAYLOAD_SIZE - 1) / HRTP_MAX_PAYLOAD_SIZE;
    int new_fms_data_len = sizeof(hrtp_fragment_t)*packet->fm_count;

    //resize fms_data buffer.
    if(new_fms_data_len > packet->fms_data_size)
    {
        Y_SFREE(packet->fms_data);
        Y_SMALLOC(packet->fms_data, new_fms_data_len, char);
        Y_INFO("hrtp_packet_resize: resize data buffer %d-%d.\n",packet->fms_data_size, new_fms_data_len);
        packet->fms_data_size = new_fms_data_len;
    }

    packet->fms_data_length = 0;
    return 0;
}

int hrtp_packet_convert_data(hrtp_packet_t* packet, char *data, int len, char hrtp_type)
{
    char *p_data = data;
    unsigned int fragment_number = 0;

    hrtp_packet_resize(packet, len);

    packet->fms_data_length = 0;
    hrtp_fragment_t * fragments = (hrtp_fragment_t *)packet->fms_data;
    unsigned int random_req = hrtp_random_num();

    unsigned int data_crc32;
#if HRTP_ENABLE_CRC32
    data_crc32 = utility_crc32(0, data, len);//enable.
#else
    data_crc32 = 0;//disable.
#endif

    //Y_INFO("packet id = %d, type = %s, len = %d\n\n",random_req, (hrtp_type==HRTP_TYPE_VEDIO)?"video":((hrtp_type==HRTP_TYPE_VEDIO_I)?"i-video":"audio"), len);
    for (fragment_number = 0; fragment_number < packet->fm_count; ++ fragment_number)
    {
        fragments[fragment_number].hrtp_hdr.flag[0]= 'H';
        fragments[fragment_number].hrtp_hdr.flag[1]= 'R';
        fragments[fragment_number].hrtp_hdr.flag[2]= 'T';
        fragments[fragment_number].hrtp_hdr.flag[3]= 'P';
        fragments[fragment_number].hrtp_hdr.random_num = random_req;
        fragments[fragment_number].hrtp_hdr.type = hrtp_type;
        fragments[fragment_number].hrtp_hdr.seq = fragment_number;
        fragments[fragment_number].hrtp_hdr.totle= packet->fm_count;
        fragments[fragment_number].hrtp_hdr.hrtp_crc32 = data_crc32;

        int copy_len = (len > HRTP_MAX_PAYLOAD_SIZE)?HRTP_MAX_PAYLOAD_SIZE:len;

        fragments[fragment_number].hrtp_hdr.payload_len = copy_len;

        memcpy(fragments[fragment_number].fm_data, p_data, copy_len);
        p_data += copy_len;
        len -= copy_len;

        //packet->fms_data_length += (sizeof(hrtp_hdr_t)+copy_len);
        packet->fms_data_length += sizeof(hrtp_fragment_t);

        //hrtp_fragment_printf(&fragments[fragment_number]);
    }
    return 0;
}

#if 0

hrtp_recombine_t* hrtp_recombine_create()
{
    hrtp_recombine_t *recombine = NULL;
    Y_SMALLOC(recombine, sizeof(hrtp_recombine_t), hrtp_recombine_t);
    recombine->length = 0;
    recombine->data_size = 0;

    recombine->pkt_id = 0;
    recombine->pkt_type = 0;
    recombine->pkt_totle = 0;
    recombine->pkt_fms_data_size = 0;

    recombine->pkt_fms_data = NULL;
    recombine->data = NULL;
    return recombine;
}



int hrtp_recombine_insert(hrtp_recombine_t* recombine, hrtp_fragment_t * fragment)
{
    hrtp_fragment_t * fragments = (hrtp_fragment_t *)recombine->pkt_fms_data;
    int seq = fragment->hrtp_hdr.seq;
    if(fragment_equal(&fragments[seq], fragment))
    {
        return -1;//exist, not insert.
    }

    int copy_len = sizeof(fragment->hrtp_hdr) + fragment->hrtp_hdr.payload_len;
    memcpy(&fragments[seq], fragment, copy_len);

    recombine->length += fragment->hrtp_hdr.payload_len;
    return 0;
}

int hrtp_recombine_print(hrtp_recombine_t *recombine)
{
    hrtp_fragment_t * fragments = (hrtp_fragment_t *)recombine->pkt_fms_data;
    Y_INFO("recombine length = %d\n",recombine->length);
    int i = 0;
    for (i = 0; i < recombine->pkt_totle; ++ i)
    {
        hrtp_fragment_printf(&fragments[i]);
    }
    return 0;
}

void hrtp_recombine_destroy(hrtp_recombine_t* recombine)
{
    Y_SFREE(recombine->data);
    Y_SFREE(recombine->pkt_fms_data);
    Y_SFREE(recombine);
}

int hrtp_recombine_resize_by_fragment(hrtp_recombine_t* recombine, hrtp_fragment_t * fragment)
{
    recombine->length = 0;
    recombine->pkt_id = fragment->hrtp_hdr.random_num;
    recombine->pkt_type = fragment->hrtp_hdr.type;
    recombine->pkt_totle = fragment->hrtp_hdr.totle;
    unsigned int new_fms_data_len = sizeof(hrtp_fragment_t)*recombine->pkt_totle;

    //resize pkt_fms_data buffer.
    if(recombine->pkt_fms_data_size < new_fms_data_len)
    {
        Y_SFREE(recombine->pkt_fms_data);
        Y_SMALLOC(recombine->pkt_fms_data, new_fms_data_len, char);
        Y_INFO("hrtp_recombine_resize_by_fragment: resize data buffer %d-%d.\n",recombine->pkt_fms_data_size, new_fms_data_len);
        recombine->pkt_fms_data_size = new_fms_data_len;
    }
    hrtp_recombine_insert(recombine, fragment);
    return 0;
}

int hrtp_recombine_add(hrtp_recombine_t* recombine, hrtp_fragment_t * fragment)
{
    if(!fragment_hrtp(fragment))
    {
        Y_ERROR("hrtp_recombine_add: fragment is not 'HRTP'\n");
        return -1;//packet error.
    }

    if(recombine == NULL)
    {
        Y_ERROR("hrtp_recombine_add: recombine is NULL\n");
        return -1;//recombine error.
    }

    if((recombine->pkt_id == fragment->hrtp_hdr.random_num)
        && (recombine->pkt_type == fragment->hrtp_hdr.type)
        && (recombine->pkt_totle == fragment->hrtp_hdr.totle))
    {
        hrtp_recombine_insert(recombine, fragment);
        return -1;//exist, not insert.
    }

    if(recombine->pkt_fms_data != NULL)//not fist coming. check packet.
    {
       // int len = 0;
       // int type = 0;
       // hrtp_recombine_get_data(recombine, &len, &type);
      //  Y_INFO("old packet len(%d) type(%d)\n", len, type);
    }

    //fist or new packet coming. remalloc mem.
    Y_INFO("new packet(%d) coming, recombine it\n", fragment->hrtp_hdr.random_num);
    hrtp_recombine_resize_by_fragment(recombine, fragment);
    return 0;
}

char * hrtp_recombine_get_data(hrtp_recombine_t *recombine, int *length, int *type)
{
    if(recombine == NULL)
    {
        Y_ERROR("hrtp_recombine_get_data: recombine is NULL\n");
        return NULL;//PARAM error.
    }

    hrtp_fragment_t * fragments = (hrtp_fragment_t *)recombine->pkt_fms_data;
    int i = 0;
    for (i = 0; i < recombine->pkt_totle; ++ i)
    {
        if((recombine->pkt_id != fragments[i].hrtp_hdr.random_num)
            || (recombine->pkt_type != fragments[i].hrtp_hdr.type)
            || (recombine->pkt_totle != fragments[i].hrtp_hdr.totle))
        {
            Y_ERROR("hrtp_recombine_get_data: no complete packet. lost packet (%d) ..\n", recombine->pkt_id);
            return NULL;//packet fragment lost, not insert.
        }
    }


    //resize data buffer.
    if(recombine->data_size < recombine->length)
    {
        Y_SFREE(recombine->data);
        Y_SMALLOC(recombine->data, recombine->length, char);
        Y_INFO("hrtp_recombine_get_data: resize data buffer %d-%d.\n",recombine->data_size, recombine->length);
        recombine->data_size = recombine->length;
    }

    char *p_data = recombine->data;
    for (i = 0; i < recombine->pkt_totle; ++ i)
    {
        //hrtp_fragment_printf(&fragments[i]);
        memcpy(p_data, fragments[i].fm_data, fragments[i].hrtp_hdr.payload_len);
        p_data += fragments[i].hrtp_hdr.payload_len;
    }
    *type = fragments[0].hrtp_hdr.type;

    *length = recombine->length;
    return recombine->data;
}
#endif

