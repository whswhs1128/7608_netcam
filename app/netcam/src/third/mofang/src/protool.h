//
// Created by 汪洋 on 2019-07-25.
//

#ifndef GWSDK_PROTOOL_H
#define GWSDK_PROTOOL_H

#include "common.h"

int get_link_status();
void new_protool();
void delete_protool();
int init_protool();
int destroy_protool();

int heart_beep();

int login_service();
int link_service();

void* login_thread(void *arg);
void  login_handle();

void* check_thread(void *arg);
void  check_handle();


int rec_cb(void *pOwner, PACKAGE_HEAD phead, DATA_HEAD dhead, char *optbuff, char *buff, short seq);
int cb_handle(PACKAGE_HEAD phead, DATA_HEAD dhead, char *optbuff, char *buff, short seq);

int send_up_data(unsigned char *optdata, unsigned char *data, PACKAGE_TYPE type, int isack, int optlen, int len, U8 checks);

int send_business_up_data(void *data, int len);
int send_management_up_data(void *data, int len);

int send_business_ack_data(short errcode, unsigned char *buff, int len);
int send_management_ack_data(short errcode, unsigned char *buff, int len);

int aes_decrypt(char* key, char* in, char* out, int inlen, int* outlen);
int aes_encrypt(char* enckey, char* encbuf, char* decbuf, int inlen, int* outlen);

int compute_file_md5(const char *file_path, char *md5_str);
int LogPlugStatus(ENUM_PLUG_LOG code);

#endif //GWSDK_PROTOOL_H
