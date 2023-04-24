#ifndef DOORBELL_FUNC_H
#define DOORBELL_FUNC_H

#include "AVIOCTRLDEFs.h"


//gpio define
#define DBC_AUDIO_ENABLE	2
#define DBC_RESET			3
#define DBC_PIR				5
#define DBC_R_LED			7
#define DBC_B_LED			8
#define DBC_CALL			9
#define DBC_LOCK			16


void AlarmPush(SMsgAVIoctrlSetPushReq * pi, int type, time_t ts);
int PPDevSetupGPIO(int ActiveLow, int Direction, int Pin,int Value);
int PPDevGetGPIO(int Pin);
int PPDevSetGPIO(int Pin,int Value);
int door_open(void* arg);

int HandleGet(
	int 			FileHandle,
	char * 			Buffer, 
	int 			nBytesNeed
);

int HandlePut(
	int 			FileHandle,
	char * 			Buffer, 
	int 			nBytesNeed
);

#define LED_RED_ON()  // ({PPDevSetGPIO(DBC_R_LED,0);PPDevSetGPIO(DBC_B_LED,1);})
#define LED_BLUE_ON() // ({PPDevSetGPIO(DBC_R_LED,1);PPDevSetGPIO(DBC_B_LED,0);})


#endif //DOORBELL_FUNC_H

