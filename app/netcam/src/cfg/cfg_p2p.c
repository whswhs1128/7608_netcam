#include <ctype.h>
#include "cfg_all.h"
#include "cfg_p2p.h"

GK_P2P_CFG runP2PCfg;
static GK_P2P_CFG defP2PCfg =
{
	.magic = CFG_P2P_MAGIC,
	.language = 0, //0 chinese, 1 english
	.timeZoneString = "People's Requblic of China",
	.msgPush = {0},
	.audioInEnable = 1,
	.audioOutEnable = 1,
	.PushKeyUpdateFlg = 0,
	.user_sign = {0},
	.scheduleTimeEnable = {0,0,0,0,0,0,0},
	.scheduleTimeSec = {
		{{0,59},{0,59},{0,59},{0,59}},
		{{0,59},{0,59},{0,59},{0,59}},
		{{0,59},{0,59},{0,59},{0,59}},
		{{0,59},{0,59},{0,59},{0,59}},
		{{0,59},{0,59},{0,59},{0,59}},
		{{0,59},{0,59},{0,59},{0,59}},
		{{0,59},{0,59},{0,59},{0,59}},
	},
	.is_beep = {0,0,0,0},
	.alarmAudioType = 1,
	.endMagic = CFG_P2P_MAGIC,
};

//字节流转换为十六进制字符串
static void Hex2Str(const unsigned char* source, char* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;

    for (i = 0; i < sourceLen; i++)
    {
        highByte = source[i] >> 4;
        lowByte = source[i] & 0x0f ;

        highByte += 0x30;

        if (highByte > 0x39)
                dest[i * 2] = highByte + 0x07;
        else
                dest[i * 2] = highByte;

        lowByte += 0x30;
        if (lowByte > 0x39)
            dest[i * 2 + 1] = lowByte + 0x07;
        else
            dest[i * 2 + 1] = lowByte;
    }
    return ;
}

//十六进制字符串转换为字节流
static void HexStrToByte(const char* source, unsigned char* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;

    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);

        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;

        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;

        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return ;
}

static void P2PCfg_SetDefValue(void)
{

	int i = 0;
	
	defP2PCfg.is_beep[0] = runMdCfg.handle.is_beep;

	if(!runRecordCfg.enable)
	{
		for(i=0; i<7; i++)
			defP2PCfg.scheduleTimeEnable[i] = 0;
	}
	else if(runRecordCfg.recordMode == 1)
	{
		for(i=0; i<7; i++)
			defP2PCfg.scheduleTimeEnable[i] = 1;
	}
}

int P2PCfgSave(void)
{
	char buff[1024];
	char *p2p = (char*)&runP2PCfg;

	memset(buff,0,sizeof(buff));
	Hex2Str((unsigned char*)p2p,buff,sizeof(runP2PCfg));

    int ret = CfgWriteToFile(P2P_CFG_FILE, buff);
    if (ret != 0) {
        PRINT_ERR("CfgWriteToFile %s error.\n", P2P_CFG_FILE);
        return -1;
    }
    return 0;
}

int P2PCfgLoad(void)
{
	GK_P2P_CFG *p2p_cfg = NULL;
	char p2p[1024] = {0};

	char *data = CfgReadFromFile(P2P_CFG_FILE);
	if (data != NULL)
	{
		HexStrToByte(data, (unsigned char*)p2p, 2*sizeof(GK_P2P_CFG));
		p2p_cfg = (GK_P2P_CFG *)p2p;

	}
	if(data == NULL || p2p_cfg == NULL || p2p_cfg->magic != CFG_P2P_MAGIC ||p2p_cfg->endMagic != CFG_P2P_MAGIC)
	{
		PRINT_ERR("p2p use default data\n");
		P2PCfg_SetDefValue();
		memcpy(&runP2PCfg,&defP2PCfg,sizeof(GK_P2P_CFG));
	}
	else
	{
		memcpy(&runP2PCfg,p2p_cfg,sizeof(GK_P2P_CFG));
	}

    return 0;
}

int P2PCfgLoadDefValue(void)
{
	P2PCfg_SetDefValue();
    memcpy(&runP2PCfg,&defP2PCfg,sizeof(GK_P2P_CFG));
    return 0;
}

int P2PCfg_get(void *dest)
{
	memcpy(dest, &runP2PCfg, sizeof(GK_P2P_CFG));
	return 0;
}
int P2PCfg_set(void *src)
{
	memcpy(&runP2PCfg, src, sizeof(GK_P2P_CFG));
	return 0;
}

void P2PCfg_init(void)
{
	PRIVETE_CFG_T P2PCfg;

	P2PCfg.CfgId = P2P_PARAM_ID;
	P2PCfg.PriveteCfgAddr = &runP2PCfg;
	P2PCfg.PriveteCfgSize = sizeof(GK_P2P_CFG);
	P2PCfg.privete_cfg_get_param = P2PCfg_get;
	P2PCfg.privete_cfg_set_param = P2PCfg_set;
	P2PCfg.privete_cfg_Load = P2PCfgLoad;
	P2PCfg.privete_cfg_Load_def_value = P2PCfgLoadDefValue;
	P2PCfg.privete_cfg_save = P2PCfgSave;
	reg_privete_cfg_api(&P2PCfg);
}

