#ifndef _DEEP_SENSING_H
#define _DEEP_SENSING_H

#include "I1Gw2015Protocol.h"
#include "common.h"
#include "uart_dev.h"
#include "ec800k.h"

/* 心跳包接收标志 */
extern int Recv_Heartbeat_flag;
/* 心跳包接收回复失败计数 */
extern int Recv_Heartbeat_Failed_cnt;
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

///@brief 通过LTE获取设备串口数据
uint16_t lte_uart_read_pack(u_int8_t *buff, int buff_size, lte_device_handle lte_device, u_int8_t chnn);
///@brief 通过广播包发送消息
BOOL Send_Broadcast_Msg(char *msg_type, uint8_t *buff, uint16_t len);

///@brief 自定义数据响应报--设备状态数据报
BOOL DS_Msgproc_DeviceStateDataResp(GW2015FrameHeader_t *pFrame);
BOOL DS_BroadcastMsgProc_IceTensionData(mq_brodcast_msg *pMsg);
///@brief 设备发送状态数据 -- 自定义协议
void DS_SendDeviceStatusData();

///@brief 状态监测报--状态监测装置重启
BOOL DS_MsgProc_ControlMonitorDevReboot(GW2015FrameHeader_t *pFrame);
///@brief 工作状态响应报--心跳数据报
BOOL DS_MsgProc_WorkHeartbeatDataResp(GW2015FrameHeader_t *pFrame);
///@brief 自定义数据响应报--版本信息数据报
BOOL DS_Msgproc_DeviceVersionDataResp(GW2015FrameHeader_t *pFrame);

///@brief 设备发送心跳包
void DS_SendDeviceHeartbeatData();
///@brief 发送DS数据报,GW2015协议，添加报文头，报文尾，并调用数据发送；
void DS_SendGW2015DataPack(uint8_t frame_type, uint8_t pack_type, uint8_t *data, uint16_t data_len);

///@brief 初始化模块入口函数
BOOL InitBussinessDSModule();
#endif