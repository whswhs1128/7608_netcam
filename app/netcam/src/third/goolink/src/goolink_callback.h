
#ifndef __GOOLINK_CALLBACK_H
#define __GOOLINK_CALLBACK_H

/*****************************************************************************
			 	 	 	 	回调函数接口

				回调接口以goolink返回客户实现的方式，
				再反馈给app或服务器，用来实现某种功能的接口
				成功必须return 1，失败必须return 0
 *****************************************************************************/

#include "goolink_apiv2.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************************************************************

 	 	 	 *****************************		以下为必须实现的接口	****************************************

**************************************************************************************************************************/

/*	打开实时视频接口
 *  device -->app
 *  channel ：		通道号
 *  ismainorsub：	主码流--0 次码流--1
 *  videoinfo：		提交给app的视频信息（见goolink_apiv2.h）	*/
int32_t GLNK_RTVideoOpen_Callback(unsigned char channel,unsigned char ismainorsub, GLNK_VideoDataFormat* videoinfo);


/*	关闭实时视频接口
 *  app -->device
 *  channel ：		通道号
 *  ismainorsub：	主码流--0 次码流--1 	*/
int32_t GLNK_RTVideoClose_Callback(unsigned char channel, unsigned char ismainorsub);


/*	打开实时音频接口
 *  device -->app
 *  channel ：		通道号
 *  audioinfo：		提交给app的音频信息（见goolink_apiv2.h）	*/
int32_t GLNK_AudioEncodeOpen_CallBack(unsigned char channel, GLNK_AudioDataFormat* audioinfo);


/*	关闭实时音频接口
 *  app -->device
 *  channel ：		通道号	*/
int32_t GLNK_AudioEncodeClose_CallBack(unsigned char channel);


/*	开启音频解码接口
 *  app -->device
 *  channel ：		通道号
 *  audioinfo：		app提交给设备的音频信息（见goolink_apiv2.h）*/
int32_t GLNK_AudioDecodeOpen_CallBack(unsigned char channel, GLNK_AudioDataFormat *audioinfo);


/*	音频解码接口
 *  app -->device
 *  channel ：		通道号
 *  buffer：		app提交给设备的音频数据
 *  length：		app提交给设备的音频数据长度	*/
int32_t GLNK_AudioDecode_CallBack(unsigned char channel, char* buffer, uint32_t length);


/*	关闭音频解码接口
 *  app -->device
 *  channel ：		通道号	*/
int32_t GLNK_AudioDecodeClose_CallBack(unsigned char channel);


/*	密码验证接口（局限，已不再使用）
 *  app -->device
 *  username ：		app请求的用户名
 *  pwd：			app请求的用户名密码	*/
unsigned char GLNK_PwdAuth_Callback(char* username, char* pwd);


/*	密码验证接口（新接口）
 *  app -->device
 *  username ：		app请求的用户名
 *  pwd：			app请求的用户名密码
 *  channel：		app请求的通道号
 *  sid：			当前连接的唯一标志，可用于glnk_CloseSession主动断开
 *  return：		掩码位置权限说明
					(取位0表示有这个功能，1表示没有这个功能，如
					return 1    (0000 0001) 则表示密码验证成功，且有所有的功能，
					return 113  (0111 0001) 则表示密码验证成功、有预览、对讲、回放功能，没有云台，获取配置，修改配置的权限
					0000 0000---用户名密码错误
					0000 0001---密码验证成功
					0000 0010---没有预览权限
					0000 0100---没有对讲权限
					0000 1000---没有回放权限
					0001 0000---没有云台权限
					0010 0000---没有获取配置权限
					0100 0000---没有修改配置权限
					。。。。保留） */
unsigned char GLNK_PwdAuthWithChannel_Callback(char* username, char* pwd,int32_t channel,int32_t sid);


/*	获取设备信息接口
 *  device -->app
 *  devinfo ：		设备提交给app的设备信息（见goolink_apiv2.h）	*/
void GLNK_GetDevInfo_Callback(GLNK_V_DeviceInfo* devinfo);


/*	重置密码接口
 *  app -->device
 *  username ：		app提交给设备的用户名
 *  oldpwd：		旧密码
 *  newpwd：		新密码	*/
int32_t GLNK_ResetUsrPsword_Callback(char* username, char* oldpwd, char* newpwd);


/*********************************************************************************************************************************************************************************************************************************************/
/*	app请求设备搜索wifi信息接口
 *  device -->app
 *  buf ：		设备搜索返回给app的数据（wifi信息）数据结构必须强转为GLNK_V_WifiInfo（见goolink_apiv2.h）	*/
int32_t GLNK_SearchWifi_Callback(char ** buf);


/*	app返回配置wifi信息接口
 *  app -->device
 *  Req ：		（见goolink_apiv2.h）	*/
int32_t GLNK_WifiConfig_Callback(GLNK_V_WifiConfigRequest *Req);


/*	app修改ap密码接口
 *  app -->device
 *  Req ：		（见goolink_apiv2.h）
 *  当TLV_V_ChangeAPPasswordResponse->isopenorclose == 0 1 2时
 *  retrun   : 	0 -- 修改失败
 *  			1 -- 修改成功，
 *  当TLV_V_ChangeAPPasswordResponse->isopenorclose == 3时，
 *  return 		2 -- 开
 *  			3 -- 关										*/
int32_t GLNK_WifiChangePassWord_Callback(TLV_V_ChangeAPPasswordRequest *Req);
/*********************************************************************************************************************************************************************************************************************************************/


/*********************************************************************************************************************************************************************************************************************************************/
/*	录像文件搜索接口（按录像文件回放（旧））
 *  device -->app
 *  SearchFileInfo ：		app请求录像文件搜索信息（见goolink_apiv2.h）
 *  ptr：					设备端返回给app的录像文件结构
 *  size：					设备端返回给app的录像文件个数 x sizeof(GLNK_V_FileInfo) */
int32_t GLNK_VideoFileSearch_CallBack(GLNK_V_SearchFileRequest *SearchFileInfo, GLNK_V_FileInfo **ptr, int32_t *size);

/*	录像文件打开接口（按录像文件回放（旧））
 *  app -->device
 *  recordname ：			app请求打开的录像文件名
 *  						/bin/vs/sd/rec/20150813/rec001/rec011_20150807115955_01_0000_0030.avi
							1./bin/vs/sd/  sd卡的路径
							2.rec/20150813/rec001/  录像文件的存储路径
							3.rec011_20150807115955_01_0118_0030.avi  录像的文件名

							录像的文件名的内容组成方式是
							1.rec011表示当前录像码流为主码流，rec012表示当前录像码流是次码流
							2.20150807115955 表示录像的开始时间(年月日时分秒)
							3.01表示是定时录像 02表示报警录像 03表示所有录像(包含定时和报警)
							4.0000保留字段 （如可以用来定义通道）
							5.0030录像的时长(30s)   0180表示180s   1800表示1800s
							6.avi 录像的格式，可为其它格式
 *  target：				码流缓存通道，发送码流时要发送到此通道 (glnk_sendplaybackdata 的参数channel对应这个值)
 *  recordfd：				按文件方式发送时返回文件的描述符（已废除）
 *  videoinfo：				设备端返回给app的视频录像信息（见goolink_apiv2.h）
 *  audioinfo：				设备端返回给app的音频录像信息（见goolink_apiv2.h）
 *  streamtype：			设备端返回给app的发送码流的形式（0--视频，1--音频，2--音视频三种）
 *  sendtype：				与streamtype相等即可（已废除）
 *  FileTotalTime：			返回录像文件的时间总长度（单位毫秒）*/
int32_t GLNK_PlayBackSendStreamOpen_CallBack( char*  recordname, unsigned char target, int32_t *recordfd,  GLNK_VideoDataFormat* videoinfo, GLNK_AudioDataFormat* audioinfo,unsigned char *streamtype, unsigned char * sendtype,  int* FileTotalTime);

/*	结束录像文件接口（按录像文件回放（旧））
 *  app -->device
 *  target ：				GLNK_PlayBackSendStreamOpen_CallBack对应的target
 *  recordfd：				已废除 */
int32_t GLNK_PlayBackSendStreamClose_CallBack(unsigned char target, int32_t recordfd);


/*	录像控制接口（按录像文件回放（旧））
 *  app -->device
 *  type ：					控制类型（0继续发送，1暂停发送，2停止发送，3快放，4慢放，5拖动播放）
 *  Value：					控制值（快放，慢放和拖动播放将用到此参数）【快放和慢放是播放数据的倍数，拖动播放时时一个百分比（拖动到当前时间/FileTotalTime）】
							快放：1x(=1)   2x(=2)  4x(=4)  8x(=8)
							慢放:  1x(=1)   2x(=2)  4x(=4)  8x(=8)
 *  target ：				GLNK_PlayBackSendStreamOpen_CallBack对应的target*/
int32_t GLNK_PlayBackSendStreamContrl_Callback(int32_t type, uint32_t value, int32_t target);
/*********************************************************************************************************************************************************************************************************************************************/


/*********************************************************************************************************************************************************************************************************************************************/
/*	新录像回放搜索接口
 * 		新录像搜索没有文件的概念，只有时间轴的概念，也就是说比如一天内有连续的录像（有多个文件），不需要返回每个文件的起始及结束时间通知app画轴，只需要返回首尾时间
 * 		可能有多个文件但是不要返回多个PlayBackResponse，可以约定文件间如果间隔少于10s，把他当成一个完整的轴，只要发流的时间戳是正确的，就不会出现时间轴跳动错误
 *  device -->app
 *  AppRequest ：			app请求录像文件搜索信息（见goolink_apiv2.h）
 *  PlayBackResponse：		设备端返回给app的录像文件结构
 *  Num：					设备端返回给app的录像文件个数 */
int32_t GLNK_PlaybackSearch_CallBack(GLNK_SearchPlaybackRequest *AppRequest,GLNK_SearchPlaybackReply **PlayBackResponse,int32_t *Num);

/*	打开录像时间接口
 * 		录像打开时只会通知打开的某个时间的录像而不会打开某个文件
 *  app -->device
 *  target ：				码流缓存通道，发送码流时要发送到此通道 (glnk_sendplaybackdata 的参数channel对应这个值)
 *  channel：				打开的通道
 *  StartTime：				打开的某个时间
 *  StreamTypeResponse：	发送码流的形式（0=视频，1=音频，2=音视频三种）（用户填写）
 *  VideoinfoResponse： 	设备端返回给app的视频录像信息（见goolink_apiv2.h）
 *  AudioinfoResponse：		设备端返回给app的音频录像信息（见goolink_apiv2.h）
 *  FileTotalTimeResponse：	此录像段的总时长（ms ）*/
int32_t GLNK_PlaybackOpen_CallBack(int8_t target,int8_t channel,GLNK_DateTime StartTime,uint8_t *StreamTypeResponse ,GLNK_VideoDataFormat* VideoinfoResponse, GLNK_AudioDataFormat* AudioinfoResponse,int32_t *FileTotalTimeResponse);

/*	结束录像时间接口
 *  app -->device
 *  target ：				GLNK_PlaybackOpen_CallBack对应的target*/
int32_t GLNK_PlaybackClose_CallBack(int8_t target);

/*	录像控制接口
 *  app -->device
 *  type ：					控制类型（0继续发送，1暂停发送，2停止发送，3快放，4慢放，5拖动播放）
 *  Value：					控制值（快放，慢放将用到此参数）【快放和慢放是播放数据的倍数】
							快放：1x(=1)   2x(=2)  4x(=4)  8x(=8)
							慢放: 1x(=1)   2x(=2)  4x(=4)  8x(=8)
 *  JumpTime：				拖动播放的时间（拖动播放将用到此参数）
 *  target ：				GLNK_PlaybackOpen_CallBack对应的target*/
int32_t GLNK_PlaybackContrl_Callback(uint8_t type, uint8_t value, GLNK_DateTime JumpTime,int8_t target);

/*	APP获取设备录像类型接口
 *  device -->app
 *  FileorTime_Flags ：		0 ：按录像文件回放 1：按录像时间回放*/
int32_t GLNK_PlaybackByFileorTime_Callback(int32_t *FileorTime_Flags);
/*********************************************************************************************************************************************************************************************************************************************/


/*********************************************************************************************************************************************************************************************************************************************/
/*	服务器获取设备固件版本信息
 *  app -->device
 *  appbuf：		app名字不能超过20个字节（根据不同的app相应写死名字，如菲扬-FeiYang）
 *  solbuf：		方案商名字不能超过20个字节（用公司名字的全拼，如浪涛->LangTao）
 *  date：			设备软件版本更新的日期不能超过20个字节
 *  hardware：		硬件版本---方案商自己定义不能超过64个字节*/
int32_t GLNK_GetVersionFirmware_Callback(char* appbuf, char* solbuf, char* date, char* hardware);

/*	下载固件接口
 *  server -->device
 *  type：			1---请求升级判断flash空间大小，打开文件，返回文件描述符参数length是将要接收文件的总长度
					2---将接收到的数据写到flash中 buffer 数据开头指针，length 数据长度
					3---接收完成关闭文件，开始升级
					4---升级失败关闭文件
 *  buffer：		数据
 *  length：		数据长度*/
int32_t GLNK_DeviceUpDate_Callback(int32_t type, char *buffer, int32_t length);
/*********************************************************************************************************************************************************************************************************************************************/


/*	设备重启接口
 *  app -->device	*/
int32_t GLNK_DeviceReboot_Callback();


/*	设备恢复出厂设置接口
 *  app -->device	*/
int32_t GLNK_FactoryReset_Callback();


/*********************************************************************************************************************************************************************************************************************************************/
/*	App获取硬盘(SD卡)列表接口
 *  device -->app
 *  StorageList：	硬盘(SD卡)列表结构体，是一个变长结构体（见goolink_apiv2.h）*/
int32_t GLNK_GetStorageList_Callback(GLNK_DeviceStorageResponse **StorageList);

/*	格式化硬盘(SD卡)列表接口
 *  app -->device
 *  StorageID：		GLNK_DeviceStorageList->StorageID，硬盘(sd卡)ID，和获取时的id保持一致
 *  result：		格式化结果回复0:格式化失败，1:格式化成功，2:无权限*/
int32_t GLNK_FormatStorage_Callback(int32_t StorageID,int32_t *result);
/*********************************************************************************************************************************************************************************************************************************************/


/*	文件下载接口
 *  app -->device
 *  type：			有3种类型  下载的类型，断点续传和正常下载， 结束下载
					typedef enum _OpenDownLoadFIleType
					{
						NORMALDOWNLOAD = 1,   	//正常下载
						BREAKPOINTDOWNLOAD = 2, //断点续传
						OVERDOWNLOAD = 3,		// 结束下载
					}OpenDownLoadFIleType;
 *	SessionID：		发送码流ID（glnk_SendDownLoadFileData对应的 session id）
 *	filename：		下载文件名字
 *	offset ：		文件的偏移位置
 *	FileLen：		文件的长度 		*/
int32_t GLNK_DownLoadFileConfig_Callback(int32_t type, int32_t SessionID, char *filename, uint32_t offset, unsigned int *FileLen);

/*	该gid是否打开云存储功能接口
 *  server -->device
 *  Value：			1--打开，0--关闭*/
int32_t GLNK_bIsOpenCloudStorage_CallBack(int32_t Value);

/*************************************************************************************************************************

 	 	 	 *****************************		以下为可选的接口	****************************************

**************************************************************************************************************************/

/*	开锁上锁接口
 *  app -->device
 *  passwd：		操作密码
 *  channel：		通道（默认为0）
 *  type：			操作动作（0为上锁，1为开锁）
 *  lockdelay：		延时开锁时间（秒）*/
int32_t GLNK_SwitchOfDoor_Callback(char * passwd, int channel, int type,int lockdelay);

/*	获取锁状态回调接口
 *  device -->app
 *  channel：		通道（默认为0）
 *  action：		锁动作（0为未上锁，1为已开锁）返回给app的状态值*/
int32_t GLNK_GetSwitchOfDoor_Callback(unsigned char channel, char *action);

/*	修改开锁密码回调接口
 *  device -->app
 *  oldpwd：		旧密码
 *  newpwd：		新密码*/
int32_t GLNK_ResetUnlockPsword_Callback(char* oldpwd, char* newpwd);


/*********************************************************************************************************************************************************************************************************************************************/
/*	打开云台回调接口
 *  app -->device
 *  channel：		通道（默认为0）*/
int32_t GLNK_PTZOpen_Callback(uint32_t channel);

/*	云台控制回调接口
 *  app -->device
 *  ptzcmd：		云台命令
 *  channel：		通道（默认为0）
 *  arg：			额外参数（见goolink_apiv2.h）*/
int32_t GLNK_PTZCmd_Callback(GLNK_PTZControlCmd ptzcmd,uint32_t channel, GLNK_ControlArgData* arg);

/*	关闭云台回调接口
 *  app -->device
 *  channel：		通道（默认为0）*/
int32_t GLNK_PTZClose_Callback(uint32_t channel);
/*********************************************************************************************************************************************************************************************************************************************/

/*	设置通道报警开关回调接口
 *  app -->device
 *  channel：		32位IO通道掩码（0为关闭，1为打开）
 *  reserve: 		32位IO通道掩码（0为未启用，1为启用）*/
int32_t GLNK_SetAlarmSwtich_Callback( int32_t channel,int32_t reserve);


/*	获取通道报警开关回调接口
 *  device -->app
 *  channel：		32位IO通道掩码（0为关闭，1为打开）
 *  reserve: 		32位IO通道掩码（0为未启用，1为启用）
 *  channelnum:		有效通道数	*/
int32_t GLNK_GetAlarmSwtich_Callback( int32_t *channel,int32_t *reserve,int32_t *channelnum);


/*	时间同步回调接口
 *  app -->device
 *  sec：			当前时间的秒 统一标准的时间（UTC的时间）
 *  usec: 			精确到毫秒级（最准确的时间是sec+usec。不过一般很少用usec）
 *  zone:			时区，用秒来做单位，如+8h，sec+zone
 * 	dst：			夏令时，用秒来做单位
 * 	istwelve：		时间的表示方式，12 hours  or  24 hours*/
int32_t GLNK_TimeSyn_CallBack(int32_t sec, int32_t usec, int32_t zone, int32_t dst, char istwelve);


/****************************************************************************************
 * 									json形式配置接口（可见jsondemo.c）
*****************************************************************************************/
/*	设置移动侦测的回调函数
 *  app -->device
 *  pJsonData：		APP发送过来的json数据（字符串形式）
 *  				MotionDetectOpt
					{
						"CallBack":false	//回调失败	// If this object exist, there is a callback error.
						// or
						"MDetectClose": true,   //移动侦测是否关闭
						"MotionDetectList":[{...}]
					}

					MotionDetectList
					{
						"Channel": uint8_t,	//通道	号		// 1-255;
						"ReSuc":false,     //获取参数配置的返回结果,如果返回结果为失败,下面就不需要赋值
						// or
						"ChannelOpen": boolean,	//移动侦测是否打开			// 0 : close; 1: open;
						"EnablePush": boolean,	//移动侦测是否推送报警			// 0 : disable; 1 : enable;
						"Sensitivity": int8_t,	//灵敏度			// 1 -5;
						"PhotoNum": int8_t,	//抓拍照片的数量			// 1-255;
						"PhotoItvMs": int8_t,	//抓拍照片的时间间隔			// The interval of taking photo in Millisecond;
						"PreRecVideoSec": int32_t, //几秒后开始录像			// The duration of pre-recording. Default: 0 sec;
						"RecVideoDurSec": int32_t, //录像时间			// The duration of recording. Default: 60secs;
						"AudioAlarm":boolean,   //设备声音报警
						"LocalPhoto": boolean,//本地抓拍---抓拍的图片保存在设备上
						"LocalVideo": boolean,//本地录像--录像保存在设备上
						"CloudPhoto": boolean,//抓拍的图片上传云端
						"CloudVideo": boolean,//录像保存上传云端
						"IsAllTheTime": boolean,//此设置是否作用于每天		// 0: False; 1: True; if this value is true, ArmTime is not necessary.
						"ArmTime":[{...}]
					}

					ArmTime
					{
						"WeekDay": "Sun", //星期几				// "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
						"IsEnable": boolean,	//打开移动侦测			// 0 : disable； 1:enable;
						"DayTime":[{...}]
					}
					DayTime
					{
						"StartTime": "000000", //移动侦测有效开始时间
						"EndTime": "235959"  //移动侦测有效结束时间
					}
 *  JsonDataLen：	pJsonData的长度
 *  pResultData: 	返回的配置结果
 *  				MotionDetectRsp
					{
						"Channel":int8_t //通道号
						"ReSuc":true    //设置的返回的结果
						///////////// or exp
						"ReSuc":false
					}
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Set_MotionDetect_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	获取移动侦测的配置的回调函数
 *  device -->app
 *  pJsonData：		APP发送过来的json数据
			MotionDetectReq
			{
				"Channel":int8_t //通道号
			}
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据（将pJsonData里面的json数据填上即可）
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Get_MotionDetect_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	配置录像参数的回调函数
 *  app -->device
 *  pJsonData：		APP发送过来的json数据（字符串形式）
 *  				VideoRecordOpt
					{
						"CallBack":false //回调函数失败		// If this object exist, there is a callback error.
						"VideoRecClose": true, 				//录像是否关闭
						"VideoRecordList":[{...}]     		//这是一个数组，多少个通道需要设置就有多少个Class VideoRecordList
					}

					VideoRecordList
					{
						"Channel": uint8_t,					//通道号			// 1 - 255;
						"ChannelOpen": boolean, 			//该通道是否支持录像				// 0 : close ; 1 : open;
						"StreamLvl": [int8_t],  			//码流的等级(主次)				// 0: main stream; 1: substream； 2: subsubstream;
//						"VideoSetSDMode":uint8_t			// 1--循环录像  2--录完即停
						"ArmTime":[{...}]					// The same as MotionDetectOptList;
					}
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据
 *  				VideoRecordRsp
					{
						"Channel":int8_t//通道号
						"ReSuc":true   //设置的返回的结果
						///////////// or exp
						"ReSuc":false
					}
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Set_VideoRecord_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	获取录像的配置的回调函数
 *  device -->app
 *  pJsonData：		APP发送过来的json数据
			VideoRecordReq
			{
				"Channel":int8_t//通道号
			}
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据（将pJsonData里面的json数据填上即可）
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Get_VideoRecord_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	设置时间的回调函数
 *  app -->device
 *  pJsonData：		APP发送过来的json数据（字符串形式）
 *  				TimeSetOpt
					{
						"SetTimeType": "NetTime"
						"OpenNetSynTime":boolean  		//是否打开网络更新世家你
						"SvrAddr": string        		//时间服务器地址
						"SNTPfrequency"	:int32_t 		//网络时间协议时间间隔
						//or
						"SetTimeType":"ManualSetTime"
						"TimeZoneFlag":string   "east"  "west" //东西时区
						"TimeZone": int32_t  //时区值
						"SettingTime":   yyyy-mm-dd-hh-mm-ss //当前时间


						"DST": boolean  //是否打开夏令时
						"DSTValue": int32_t //夏令时的值
						"Auto-refreshTimes":boolean //自动更新时间
					}
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据
 *  				JSON Format
					{
						"FormatErr":true
						/// or
						"ReSuc":false
						/// or
						"ReSuc":True
					}
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Set_Time_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	获取时间设置回调函数
 *  device -->app
 *  pJsonData：		APP发送过来的json数据（字符串形式，见上json，里面是空的）
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据（将pJsonData里面的json数据填上即可）
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Get_Time_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	设置视频流的回调函数
 *  app -->device
 *  pJsonData：		APP发送过来的json数据（字符串形式）
 *  				VideoStreamOpt
					{
						"StreamType": "Main" or "Sub"  //码流类型：主码流，次码流，第三码流
						"Resolution": string   "720P" or "D1" ..... //分辨率
						"Height":int32_t          //视频显示的高度
						"Width":int32_t	          //视频显示的宽
						"bitrate":int32_t         //视频的比特率
						"framerate":int32_t       //视频的帧率
						"frameInterval":int32_t   //视频帧间隔
						"StreamControl":string "Control"
					}
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据
 *  				JSON Format
					{
						"FormatErr":true
						/// or
						"ReSuc":false
						/// or
						"ReSuc":True
					}
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Set_VideoStream_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	获取视频流的回调函数
 *  device -->app
 *  pJsonData：		APP发送过来的json数据（字符串形式，见上json，里面是空的）
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据（将pJsonData里面的json数据填上即可）
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Get_VideoStream_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	设置音频流的回调函数
 *  app -->device
 *  pJsonData：		APP发送过来的json数据（字符串形式）
 *  				AudioStreamOpt
					{
						"WaveFormat":"G711a" string  //编码格式
						"FrameInterval":int32_t      //帧间隔
						"FrameRate":int32_t        //采样率
						"SamplesRate":int32_t        //帧率
						"BitRate":int32_t		//比特率
						"InputGain":int32_t          //输入增益
						"Outputvolume "int32_t      //输出音量
						"MainStream":boolean        //
						"SubStream":boolean
					}
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据
 *  				JSON Format
					{
						"FormatErr":true
						/// or
						"ReSuc":false
						/// or
						"ReSuc":True
					}
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Set_AudioStream_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	获取音频流的回调函数
 *  device -->app
 *  pJsonData：		APP发送过来的json数据（字符串形式，见上json，里面是空的）
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据（将pJsonData里面的json数据填上即可）
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Get_AudioStream_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	设置图像配置参数的回调函数
 *  app -->device
 *  pJsonData：		APP发送过来的json数据（字符串形式）
 *  				ImageOpt
					{
						"VerticalRotation":boolean //垂直翻转是否打开
						"HorizontalRotation" boolean //水平翻转是否打开
						"Lightness":int32_t  //亮度  0--100
						"Color":int32_t     //色度
						"Contrast":int32_t   //对比度
						"Saturation":int32_t //饱和度
						"SceneMode":string auto, indoor, outdoor //场景模式,分为自动,室内,室外
						"Infrared":string  day&night, color, baw //红外,分为日夜, 色彩, 黑白
					}
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据
 *  				JSON Format
					{
						"FormatErr":true
						/// or
						"ReSuc":false
						/// or
						"ReSuc":True
					}
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Set_Image_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	获取图像配置参数的回调函数
 *  device -->app
 *  pJsonData：		APP发送过来的json数据（字符串形式，见上json，里面是空的）
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据（将pJsonData里面的json数据填上即可）
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Get_Image_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	设置OSD的回调函数
 *  app -->device
 *  pJsonData：		APP发送过来的json数据（字符串形式）
 *  				OSDOpt
					{
						"DisplayTimeStamp":boolean     //是否显示时间
						"DisplayDeviceName":boolean    //是否显示设备名
						"fontSize":int32_t             //字体的大小
						"DeviceName":string            //显示设备名字
					}
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据
 *  				JSON Format
					{
						"FormatErr":true
						/// or
						"ReSuc":false
						/// or
						"ReSuc":True
					}
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Set_OSD_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	获取OSD的回调函数
 *  device -->app
 *  pJsonData：		APP发送过来的json数据（字符串形式，见上json，里面是空的）
 *  JsonDataLen：	pJsonData的长度
 *  pResultData：	回复给app的json数据（将pJsonData里面的json数据填上即可）
 *  ResultDataLen: 	返回配置结果的长度*/
int32_t Glnk_Get_OSD_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);


/*	设备音频文件搜索接口
 *  device -->app
 *  SearchFileInfo：			音频文件搜索信息（见goolink_apiv2.h）
 *  ptr: 						搜索到的音频文件的信息（见goolink_apiv2.h）
 *  size:						ptr的大小（按字节计算）*/
int32_t GLNK_AudioFileSearch_CallBack(GLNK_V_SearchAudioFileRequest *SearchFileInfo, GLNK_V_AudioFileInfo **ptr, int32_t *size);


/*	播放和控制音乐文件接口
 *  device -->app
 *  Type：						控制类型，1----开始播放  2---暂停播放   3----恢复播放   4---结束播放（停止播放）
 *  PlayMode: 					播放模式
 *  FileName:					文件名字	*/
int32_t GLNK_Audio_Play_Contrl_CallBack(int32_t Type, int32_t PlayMode, char *FileName);


/*	控制设备音量大小接口
 *  app -->device
 *  action：					操作
 *  volume：  					增加或者减少音量值	*/
int32_t GLNK_SetVolume_Callback(char action, char volume);


/*	远程开关控制回调函数
 *  app -->device
 *  SType ：					开关类型（对应枚举SwitchType）（见goolink_apiv2.h）
 *  SValue：  					当SType为1或者2时，对应枚举SwtichOCValue, 3时对应枚举FishEyeModeValue）*/
int32_t GLNK_RCSwitch_Callback(int32_t SType, int32_t SValue);


/*	获取远程开关控制状态回调函数
 *  device -->app
 *  SType ：					开关类型（对应枚举SwitchType）（见goolink_apiv2.h）
 *  SValue：  					设备回复app的状态,当SType为1或者2时，对应枚举SwtichOCValue, 3时对应枚举FishEyeModeValue）*/
int32_t GLNK_Get_RCType_Callback(const int32_t SType, int32_t *SValue);


/*	获取报警日志回调函数
 *  device -->app
 *  LowChannelMask ：			低32位通道掩码：0到31通道
 *  HighChannelMask：  			高32位通道掩码：32到63通道
 *  AlarmType：					报警类型
 *  StartTime：					开始时间
 *  EndTime：					结束时间
 *  AlarmInfoStr：				报警日志信息结构数组指针
 *  Sum：						报警个数 （最大个数400） */
int32_t GLNK_GetAlarmLogs_Callback(uint32_t LowChannelMask, uint32_t HighChannelMask, int32_t AlarmType, GLNK_DateTime* StartTime, GLNK_DateTime* EndTime, GLNK_V_AlarmInfo *AlarmInfoStr,  int32_t *Sum);


/*	app设置分屏报警回调函数
 *  app -->device
 *  GLNK_SetScreenAlarmRequest ：		开关类型（对应枚举SwitchType）（见goolink_apiv2.h）
 *  如下示屏幕格数顺序为书写顺序，切为4分屏，1屏 4屏侦测报警，则
	GLNK_SetScreenAlarmRequest->ScreenListLen = sizeof(GLNK_SetScreenAlarmRequest) + sizeof(GLNK_ScreenList) * 4;
	GLNK_SetScreenAlarmReques->ScreenList[0].ifSetAlarm = 1;
	GLNK_SetScreenAlarmReques->ScreenList[3].ifSetAlarm = 1;
  						---------
  						| 1 | 2 |
  						---------
  						| 3 | 4 |
  						---------			*/
int32_t GLNK_SetScreenAlarm_Callback(GLNK_SetScreenAlarmRequest *req);


/*	app获取通道分屏报警回调函数
 *  device -->app
 *  channel: 需要获取分屏报警的通道号
 *  GLNK_SetScreenAlarmRequest ：		开关类型（对应枚举SwitchType）（见goolink_apiv2.h）
 *  如下示屏幕格数顺序为书写顺序，切为4分屏，1屏 4屏侦测报警，则
	GLNK_SetScreenAlarmRequest->ScreenListLen = sizeof(GLNK_SetScreenAlarmRequest) + sizeof(GLNK_ScreenList) * 4;
	GLNK_SetScreenAlarmReques->ScreenList[0].ifSetAlarm = 1;
	GLNK_SetScreenAlarmReques->ScreenList[3].ifSetAlarm = 1;*/
int32_t GLNK_GetScreenAlarm_Callback(int32_t channel, GLNK_SetScreenAlarmRequest** rsp);


/*	app获取设备信息的接口(当设备状态变化时需调用glnk_SendDeviceInfo通知sdk，见goolink_apiv2.h)
 *  device -->app
 *  Data：一个json字段
	SD卡				“SDCard”:			"int8_t"	(0--无sd卡，1--有sd卡)
	PTZ				“PTZ”:				"int8_t"	(0--无，1--摇头，2--带光学变焦)
	鱼眼				“FishEye” :			"int8_t"	(0--无，1--有)
	720全景			“Panorama”:			"int8_t"	(0--无，1--有)
	产品形态			“ProductType”:		"int8_t"	(1--IPC，2--NVR，3--门铃，4--报警网关)
	电池				“Battery”	:		"int8_t"	(0--无，1--有)
	码流情况			“StreamingClass”:	"int8_t"	(2--两种，3--三种)
	视频编码类型		“VideoEncoderType”:	"int8_t"	(4--h264，5--h265，0--两种都支持)
	是否支持回音消除	“EchoCancel”:		"int8_t"	(0--不支持，1--支持)
	是否支持音频全双工	“Full_Duplex”		"int8_t"	(0--不支持，1--支持)
	全景参数			“PanoData”：			“string” 	(全景参数，这串数据由GLNK_GetPanoData收到的值给他赋值 )
	通道数			“Channels”			"int8_t"	(通道数)
	版本日期			“SDKVersion”:		"string"	(sdk版本号，由glnk_get_version返回，是一个16进制数的日期号)
 	固件版本			“Firmware”:			"string"	(同GLNK_GetVersionFirmware_Callback,用|隔开)
 	全景模板版本		“PanoModelVersion”	"string"	（当GLNK_DownLoadPanoModel_Callback生成新模板时要更新此参数
 													是一个UTC时间，即模板生成时间)
 *  len:json字段的长度								*/
int32_t GLNK_GetDeviceInfo_Callback(char *Data,int32_t *len);


/*	获取全景参数的的接口
 *  Data:一个json字段
	全景参数			“PanoData”：			“string” 	（全景参数，由系统保持，讲这串数据给GLNK_GetDeviceInfo_Callback中的PanoData赋值 ）
	.....（可以拓展）
 *  len:json字段的长度								*/
int32_t GLNK_GetPanoData(char *Data,int32_t len);
/*********************************************************************************************************************************************************************************************************************************************/
/*	433设备类型   设备地址码(16进制)	备注
	0x01			0x01001				红外线探测器
	0x02			0x02001				水浸感应探测器
	0x03			0x03001				一键布撤防遥控器
	0x04			0x04001				烟雾感应探测器
	0x05			0x05001				气体感应探测器
	0x06			0x06001				门/窗磁探测器
	0x07			0x07001				紧急按钮
	0x08			0x08001				无线门铃
	0x09			0x09001				一氧化碳探测器
	0x0a			0x0a001				声光报警器
	0x0b			0x0b001				电源插座
	0x0c			0x0c001				网关遥控器
	0x0d			0x0d001				开合窗
	0x0e			0x0e001				晾衣架		*/
/*	433设备学习发送型回调函数
 *  device -->app
 *  sid ：						发送接口的第一个参数sid要和这个sid对应(glnk_Send433DeviceCodeAddr)
 *  OperateType：  				操作类型 0----结束433学习，  1----开始433学习
 *  DeviceType：				学习433设备类型*/
int32_t GLNK_433DeviceLearnCallback(int32_t Sid, int32_t OperateType, int32_t DeviceType);

/*	获取433设备列表回调函数
 *  device -->app
 *  AddrCode ：					433设备地址码，是一个二位数组
 *  _433Type：  				433设备类型
 *  _433DevSum：				433设备的个数 最大为64*/
int32_t GLNK_Get433DeviceAddrCodeListCallback(char (*AddrCode)[32],char (*_433Type)[32],int32_t _433DevSum);


/*	发送433信号回调函数
 *  device -->app
 *  AddrCode ：					接受型设备的地址码
 *  KeyNum：  					键值*/
int32_t GLNK_Send433Signal_Callback(char *AddrCode,int32_t KeyNum);
/*********************************************************************************************************************************************************************************************************************************************/


/********************************* 全景矫正模板接口 ************************************************************************************************************************************************************************************************/
/*	下载矫正模板回调函数
 *  app --> device
 *  filename ：					下载的文件名
 *  type：						1:开始下载  2：下载中 3：下载完成  4：错误
 *  size：						文件总长度
 *  data:						数据
 *  len:						当前数据长度*/
int32_t GLNK_DownLoadPanoModel_Callback(char *filename,int type,int size,char *data,int len);

/*	app请求下载矫正模板回函数
 *  app --> device
 *  ConnectionID:				下载的ID号，调用glnk_SendPanoModelData发送数据
 *  filename ：					下载的文件名
 *  filesize:					要下载的文件的大小（客户回填）*/
int32_t GLNK_UploadPanoModel_Callback(int32_t ConnectionID,char *filename,int *filesize);
/*********************************************************************************************************************************************************************************************************************************************/

/********************************* RTMP接口 ************************************************************************************************************************************************************************************************/
/*	直播流视频初始化接口
 *  maxchannel ：	最大支持通道数（最多16）
 *  netflag：		内网模式---0,外网模式---1
 *  netip:			内网模式下rtmp服务器的ip*/
int32_t GLNK_RtmpInit(int *maxchannel,int *netflag,char *netip);

/*	打开直播流视频接口
 *  device -->app
 *  channel ：		通道号
 *  ismainorsub：	主码流--0 次码流--1
 *  videoinfo：		提交给app的视频信息（见goolink_apiv2.h）	*/
int32_t GLNK_RtmpVideoOpen_Callback(unsigned char channel, unsigned char ismainorsub, GLNK_VideoDataFormat* videoinfo);

/*	关闭直播流视频接口
 *  app -->device
 *  channel ：		通道号
 *  ismainorsub：	主码流--0 次码流--1 	*/
int32_t GLNK_RtmpVideoClose_Callback(unsigned char channel, unsigned char ismainorsub);

/*	打开直播流音频接口
 *  device -->app
 *  channel ：		通道号
 *  audioinfo：		提交给app的音频信息（见goolink_apiv2.h）	*/
int32_t GLNK_RtmpAudioOpen_CallBack(unsigned char channel, GLNK_AudioDataFormat* audioinfo);

/*	关闭直播流音频接口
 *  app -->device
 *  channel ：		通道号	*/
int32_t GLNK_RtmpAudioClose_CallBack(unsigned char channel);
/*********************************************************************************************************************************************************************************************************************************************/

/******************************* 获取设备图片接口 *********************************/
/*	获取设备图片接口
 *  Data ：			图片数据（续自己malloc内存，sdk会自动释放）
 *  len	 :			图片的大小
 *  return：			0--失败，1--成功	*/
int32_t GLNK_GetPicture(char **Data,int *len);

/******************************* 设置灯泡接口 *********************************/
/*	获取设备灯泡模式接口
 *  mode ：			（回填的数据）模式  0--	红外  1--	全彩  2--	智能  3--	手动
 *  key	 :			（回填的数据）亮度 0 - 100（仅手动模式下有效）
 *  return：			0--失败，1--成功	*/
int32_t Glnk_GetBulbMode(int *mode,int *key);

/*	app设置设备灯泡模式接口
 *  mode ：			模式 0--	红外  1--	全彩  2--	智能  3--	手动
 *  key	 :			亮度 0 - 100（仅手动模式下有效）
 *  return：			0--失败，1--成功	*/
int32_t Glnk_SetBulbMode(int mode,int key);

/*初版录像回放（已废除）*/
int32_t GLNK_RecordOpen_CallBack( char*  recordname, int32_t *recordfd,  GLNK_VideoDataFormat* videoinfo, GLNK_AudioDataFormat* audioinfo);
int32_t GLNK_RecordReadFrame_CallBack(int32_t recordfd, unsigned char *streamframetype, uint32_t *timestamp, int32_t *videoframeindex, void** streamdata, uint32_t *streamdatalen);
int32_t GLNK_RecordClose_CallBack(int32_t recordfd);

/*打开监听（已废除）*/
int32_t GLNK_OpenListening_Callback(GLNK_AudioDataFormat *audioinfo);
int32_t GLNK_CloseListening_Callback();

/*获取设备信息（已废除）*/
int32_t GLNK_GetDeviceServer_Callback(char* MDFlag, char* PushFlag, char* emailFlag , char* delayTime);
int32_t GLNK_SetDeviceServer_Callback(char  MDFlag, char  PushFlag, char emailFlag , char  delayTime);

/*获取用户信息（已废除）*/
int32_t GLNK_GetUserList_Callback(GLNK_V_GetUserListResponse *Rsp);
int32_t GLNK_SetUserList_Callback(GLNK_V_SetUserListRequest *Req);

/*自定义码流切换接口（已废除）*/
int32_t GLNK_RTVideoSwitching_Callback(unsigned char channel, unsigned char ismainorsub,GLNK_VideoDataFormat* Invideoinfo,GLNK_VideoDataFormat* Outvideoinfo);

/*获取sd卡接口（已废除）*/
int32_t Glnk_Get_Storage_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);

/*获取与设置智能开关接口（已废除）*/
int32_t Glnk_Set_Smart_Switch_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);
int32_t Glnk_Get_Smart_Switch_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen);

/*获取温湿度接口（已废除）*/
int32_t GLNK_Get_EnvOpt_Callback(int32_t *Temperature, int32_t *Humidity);

/*	上传图片数据的结果回调函数接口（已废除,不再支持上传云图片）
 *  server -->device
 *  DataPtr：		上传数据的指针（是glnk_UploadData的上传数据指针）
 *  Result：		上传数据结果（枚举UPLOADDATARESULT）(见goolink_apiv2.h)*/
int32_t GLNK_UpLoadData_Result_CallBack(char * DataPtr, int32_t Result);
#if 0
int32_t GLNK_RTVideoOpenEXT_Callback(int32_t connectionID,unsigned char channel, GLNK_VideoDataFormat* videoinfo);
int32_t GLNK_RTVideoCloseEXT_Callback(int32_t connectionID);
int32_t GLNK_AudioEncodeOpenEXT_CallBack(int32_t connectionID, unsigned char channel,  GLNK_AudioDataFormat* audioinfo);
int32_t GLNK_AudioEncodeCloseEXT_CallBack(int32_t connectionID);
int32_t GLNK_AudioDecodeOpenEXT_CallBack(int32_t connectionID, unsigned char channel, GLNK_AudioDataFormat *audioinfo);
int32_t GLNK_AudioDecodeEXT_CallBack(int32_t connectionID, char* buffer, unsigned int length, GLNK_V_AudioInfo* audioframeinfo, char* extdata, int32_t extdatalen);
int32_t GLNK_AudioDecodeCloseEXT_CallBack(int32_t connectionID);
int32_t GLNK_SetRecordConfigure_Callback(GLNK_V_RecordChgRequest *Req);
int32_t GLNK_OpenDownLoadRecord_Callback( char* recordname, int32_t *recordfd,  char mode, uint32_t offset);
int32_t GLNK_CloseDownLoadRecord_Callback(int32_t recordfd);
int32_t GLNK_ReadDownLoadRecord_Callback(int32_t recordfd, char* data, int32_t datalen, int32_t *start_pos, int32_t *end_pos);
void GLNK_DebugLog_CallBack(uint32_t sec, uint32_t usec, char* mdlname, char* threadname, int32_t code, int32_t threadid, int32_t logsize, int32_t logseq, char* log);
int32_t GLNK_GetVide_AudioConfig_Callback(int32_t channel, TLV_V_VA_GET_CONFIG_RSP *req);
int32_t GLNK_SetVide_AudioConfig_Callback(int32_t *result, TLV_V_VA_SET_CONFIG_REQ *req);
int32_t GLNK_SetNetWorkConfig_Callback(int32_t *result, TLV_V_Network *req);
int32_t GLNK_GetNetWorkConfig_Callback(TLV_V_Network *req);
void GLNK_UpLoadFileResult_CallBack(char *filename, int32_t result);
#endif

#ifdef __cplusplus
}
#endif

#endif

