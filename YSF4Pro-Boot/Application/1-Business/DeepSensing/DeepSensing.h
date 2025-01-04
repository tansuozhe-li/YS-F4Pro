#ifndef _DEEP_SENSING_H
#define _DEEP_SENSING_H

#include "I1Gw2015Protocol.h"
#include "common.h"
#include "uart_dev.h"
#include "ec800k.h"

extern DsPlatformModuleCfg_t DsPlatformModuleCfg;

//帧序号
extern uint8_t ds_msgproc_frame_no;
/* 数据状态 */
typedef enum {
  GW2015DataStateERR = 0x00,
  GW2015DataStateOK  = 0xFF,
}GW2015DataState_e;


/* 帧、报文类型处理函数 */
typedef BOOL (*MsgFrameProcFunc)(GW2015FrameHeader_t *);
typedef struct {
  char             frameName[64];
  char             packName[64];
  int16_t          msgType;
  MsgFrameProcFunc pMsgProc;
}DSMsgProcFrameFuncTable_t;
unsigned short RTU_CRC( unsigned char * puchMsg, unsigned short usDataLen);
///@brief 通过LTE获取设备串口数据
uint16_t lte_uart_read_pack(u_int8_t *buff, int buff_size, lte_device_handle lte_device, u_int8_t chnn);
///@brief 广播包接收电池模块数据
BOOL DS_BroadcastMsgProc_BatteryData(mq_brodcast_msg *pMsg);
///@brief 控制数据响应报--状态监测装置重启
BOOL DS_MsgProc_ControlMonitorDevReboot(GW2015FrameHeader_t *pFrame);

///@brief 工作状态响应报--心跳数据报
BOOL DS_MsgProc_WorkHeartbeatDataResp(GW2015FrameHeader_t *pFrame);
///@brief 自定义数据响应报--设备状态数据报
BOOL DS_Msgproc_DeviceStateDataResp(GW2015FrameHeader_t *pFrame);

///@brief 设备发送心跳包
void DS_SendDeviceHeartbeatData();
///@brief 广播包接收电池数据
void DS_SendDeviceBatteryData();
///@brief 发送DS数据报,GW2015协议，添加报文头，报文尾，并调用数据发送；
void DS_SendGW2015DataPack(uint8_t frame_type, uint8_t pack_type, uint8_t *data, uint16_t data_len);
///@brief 初始化模块入口函数
BOOL InitBussinessDSModule();
#endif