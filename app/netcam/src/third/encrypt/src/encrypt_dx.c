/*!
*****************************************************************************
** \file        main.c
**
** \brief       GOKE application
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include "DX8_API.h"
#include "dx8_engineer.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define I2C_DEV_NODE "/dev/i2c-1"
//#define DX82_ENABLE

#define CHECK_RV(x) do { unsigned char y = x; if(y) {printf("Error: rv=%.2x,line:%d\n",y,__LINE__); return y;} } while(0)

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************

//------------------------------------------------------------------------------
//  Host CPU Random Number Generation
//------------------------------------------------------------------------------
static void GetSoftRandom(unsigned char *random, unsigned short len)
{
  unsigned short i;
  // Strongly recommended the seed involved by system time !!!!!
  srand((unsigned int)time(NULL) + srand_cnt++); 
  for (i=0; i<len; i++) random[i] = rand() % 256;
}

//------------------------------------------------------------------------------
//  PIN and Host Authentication
//------------------------------------------------------------------------------
static unsigned char AuthenticationTest()
{
   unsigned char random[32];
   unsigned char tmpBuf1[20] = { PPPPP };
   unsigned char tmpBuf2[20] = { KKKKK };   

   // Wakeup and Reset DX8
   CHECK_RV(DX8_Reset());

   // PIN Authentication
   GetSoftRandom(random,32); // Generate random for verify PIN
   CHECK_RV(DX8_VerifyPin(random,tmpBuf1));

   // Host Authentication
   memset(tmpBuf1,0x00,20);
   GetSoftRandom(random,32);
   CHECK_RV(DX8_HostAuth(random,32,tmpBuf1));
   Lib_HostAuth(random,32,tmpBuf2,tmpBuf2);
   CHECK_RV(memcmp(tmpBuf1,tmpBuf2,20));

   // DX8 Sleep to save power
   CHECK_RV(DX8_Sleep()); 
   
   printf("\n<<<<<<<<<AuthenticationTest OK!>>>>>>>>>\n");
   printf("<<<<<<<<< SN:%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx, MID:%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx >>>>>>>>>\n",
    dx8_info.SN[0], dx8_info.SN[1], dx8_info.SN[2], dx8_info.SN[3], dx8_info.SN[4], dx8_info.SN[5], dx8_info.SN[6], dx8_info.SN[7], 
    dx8_info.MID[0], dx8_info.MID[1], dx8_info.MID[2], dx8_info.MID[3], dx8_info.MID[4], dx8_info.MID[5]);

   return 0;
}

//------------------------------------------------------------------------------
//  Wrtie and Read Security EEPROM with random number
//------------------------------------------------------------------------------
static unsigned char SecurityEEPROMTest()
{
   unsigned char i;
   unsigned char rdata[128];
   unsigned char wdata[128];
   unsigned char zkey[4][16] = { {ZZZZZ0},{ZZZZZ1},{ZZZZZ2},{ZZZZZ3} };

   // Wakeup and Reset DX8
   CHECK_RV(DX8_Reset()); 

   // Security EEPROM write,read and check
   for (i=0; i<4; i++) {
     GetSoftRandom(wdata,128); // Generate the data for EEPROM writing
     if (dx8_info.ZoneMode[i] & 0x08) { // Not Lock
       if ((dx8_info.ZoneMode[i] & 0x03) != 0x03) { // EEPROM Encrypt Mode
          // Verify Zone
          GetSoftRandom(rdata,32); // Generate random for verify zone
          CHECK_RV(DX8_VerifyZone(i,rdata,zkey[i])); break;
          // write Zone
          CHECK_RV(DX8_WriteZone(i,0,(dx8_info.ZoneSize-16),wdata));
          // read Zone
          memset(rdata,0x00,128);
          CHECK_RV(DX8_ReadZone(i,0,(dx8_info.ZoneSize-16),rdata));
          // Check Zone
          CHECK_RV(memcmp(rdata,wdata,(dx8_info.ZoneSize-16)));
       }
       else { // EEPROM Normal Mode
          // write zone
          CHECK_RV(DX8_WriteZone(i,0,dx8_info.ZoneSize,wdata));
          // read zone
          memset(rdata,0x00,128);
          CHECK_RV(DX8_ReadZone(i,0,dx8_info.ZoneSize,rdata));
          // check zone
          CHECK_RV(memcmp(rdata,wdata,dx8_info.ZoneSize));
       }
     }
   }
   // DX8 Sleep to save power
   CHECK_RV(DX8_Sleep());
   printf("\n<<<<<<<<<SecurityEEPROMTest OK!>>>>>>>>>\n");

   return 0;  
}

#ifdef DX82_ENABLE
//------------------------------------------------------------------------------
//  Point 2 Point Data Encrypt (only for DX82)
//------------------------------------------------------------------------------
unsigned char DX82_Test()
{
   unsigned char random[32];
   unsigned char plaint[128];
   unsigned char cipher[128];
   unsigned char result[128];
   CRYPTO_PARA para;

   // Wakeup and Reset DX8
   CHECK_RV(DX8_Reset()); 

   GetSoftRandom(plaint,128); // Generate plaintext for encryption
   
   // Encryption
   // Input:   plaint, receiver's UID and receiver's SN
   // Output:  cipher, para.seed
   memcpy(para.UID,dx8_info.UID,7);
   memcpy(para.SN,dx8_info.SN,8);
   para.slen = 32;
   CHECK_RV(DX8_GenSessionKey(CRYPTO_ENC,CRYPTO_PRIVATE_IDSN,&para));
   CHECK_RV(DX8_DoCrypto(CRYPTO_ENC,plaint,cipher,128));
   memcpy(random,para.seed,32);

   // Decryption
   // Input:  cipher, para.seed
   // Output: result
   memcpy(para.seed,random,32);  // emulate receiving the random seed
   para.slen = 32;
   CHECK_RV(DX8_GenSessionKey(CRYPTO_DEC,CRYPTO_PRIVATE_IDSN,&para));
   CHECK_RV(DX8_DoCrypto(CRYPTO_DEC,cipher,result,128));

   // Compare:  plaint == result
   CHECK_RV(memcmp(plaint,result,128));

   // DX8 Sleep to save power
   CHECK_RV(DX8_Sleep());

   return 0;  
}
#endif

unsigned char dxif_transfer(unsigned char *buf, unsigned short len)
{
       int fd = -1;

       if (len < 1) return 1;

       if ((fd = open(I2C_DEV_NODE, O_RDWR)) < 0) {
          fprintf(stderr, "Fail to open %s", I2C_DEV_NODE);
          return 1;
       }

       if(ioctl(fd, I2C_SLAVE_FORCE, (buf[0]>>1)&0x7f) < 0) {
           fprintf(stderr, "ioctl set address failed!!\n");
           close(fd);
           return 1;
       }
 
       if (buf[0] & 0x01) { // i2c read
          if(read(fd, buf+1, len-1) != (len-1)) {
             fprintf(stderr, "i2c read failed!!\n");
             close(fd);
             return 1;
          }
       #ifdef DEBUG_ENABLE
          int i;
          printf("dxi2c read: %.2X",buf[0]);
          for (i = 1; i < len; i++) {
	      if (!((i-1) % 16)) printf("\n");
	      printf("%.2X ", buf[i]);
	  }
	  printf("\n");
       #endif
       }
       else {  // i2c write
       #ifdef DEBUG_ENABLE
          int j;
          printf("dxi2c write: %.2X %.2X %.2X %.2X",buf[0],buf[1],buf[2],buf[3]);
          for (j = 4; j < len; j++) {
	      if (!((j-4) % 16)) printf("\n");
	      printf("%.2X ", buf[j]);
	  }
	  printf("\n");
       #endif
          if (write(fd, buf+1, len-1) != (len-1)) {
             fprintf(stderr, "i2c write failed!!\n");
             close(fd);
             return 1;
          }
       }

      close(fd);
      return 0;
}

static int dx8_test(void)
{
    char *dx8Version;

    // Get Library Version
    dx8Version = DX8_Version();
    printf("Version = %s\n", dx8Version);

    // default is 0x60, DX8-C2 should change to 0x20
    //DX8_SetDevAddress(0x20);
    
    // Authention Test
    CHECK_RV(AuthenticationTest());

    // Security EEPROM Test
    CHECK_RV(SecurityEEPROMTest());

#ifdef DX82_ENABLE
    // Point to Point Data Encryption Test
    CHECK_RV(DX82_Test());
#endif

    printf ("\n<<<<<dx8 Demo Test OK!!!>>>>>>>\n");

    return 0;
}

int encrypt_authentication(void)
{
	return AuthenticationTest();
}

