#ifndef _NANWANG_GX_H
#define _NANWANG_GX_H

#include "I1Gw2015Protocol.h"
#include "common.h"
#include "uart_dev.h"
#include "ec800k.h"

/* 帧、报文类型处理函数 */
typedef BOOL (*MsgFrameProcFunc)(GW2015FrameHeader_t *);
typedef struct {
  char             frameName[64];
  char             packName[64];
  int16_t          msgType;
  MsgFrameProcFunc pMsgProc;
}NWMsgProcFrameFuncTable_t;

///@brief 通过LTE获取设备串口数据
uint16_t lte2_uart_read_pack(u_int8_t *buff, int buff_size, lte_device_handle lte_device, u_int8_t chnn);
BOOL NW_BroadcastMsgProc_IceTensionData(mq_brodcast_msg *pMsg);
void NW_SendGW2015DataPack(uint8_t frame_type, uint8_t pack_type, uint8_t *data, uint16_t data_len);
///@brief 工作状态响应报--心跳数据报
BOOL NW_MsgProc_WorkHeartbeatDataResp(GW2015FrameHeader_t *pFrame);
///@brief 设备发送心跳包
void NW_SendDeviceHeartbeatData();
///@brief 初始化模块入口函数
BOOL InitNanWangGxModule();
#endif