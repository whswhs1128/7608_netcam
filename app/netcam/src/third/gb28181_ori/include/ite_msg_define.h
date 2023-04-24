
#ifndef ITE_MSG_DEFINE_H
#define ITE_MSG_DEFINE_H

typedef struct ITE_Time_Struct_T
{
	int wMilliseconds;
	int wSecond;
	int wMinute;
	int wHour;
	int wDay;
	int wMonth;
	int wYear;	
}ITE_Time_Struct;

typedef struct ITE_OSAL_MSG_INFO
{
	int shmid;
}ITE_OSAL_MSG_INFO_T;


#endif //ITE_MSG_DEFINE_H
