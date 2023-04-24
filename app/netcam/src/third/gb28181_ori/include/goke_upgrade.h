#ifndef GOKE_MOJING_H
#define GOKE_MOJING_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    int upgradeStart;
    int upgradeProcess;
    int fileSize;
    int isFeedDog;
    char sdkName[32];
    char curVer[32];
}UPGRADE_SHARE_INFO;

typedef struct _UPGRAE_PARAM_S{
    char fileUrl[512]; //url
    char firmware[32];  //ver
}UPGRAE_PARAM_S;

int goke_upgrade_start(void);
int goke_upgrade_fresh_now(void);
void upgrad_set_default_dev_version(void);
int goke_upgrade_get_file_update_gb28181(void *arg);
/*!
*******************************************************************************
** \brief check device key is valid.
**
** \return
** - 0 ok
** - -1 and other error.
**
**
*******************************************************************************
*/
int goke_upgrade_key_status(void);
char *goke_upgrade_get_key(void);
#ifdef __cplusplus
    }
#endif

#endif

