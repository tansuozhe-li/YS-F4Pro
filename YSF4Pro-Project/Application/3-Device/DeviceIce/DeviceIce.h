#ifndef __DEVICE_ICE_H_
#define __DEVICE_ICE_H_

#include "common.h"
#include <stdint.h>
#include "I1Gw2015Protocol.h"

#pragma pack(push,1)
/* 覆冰读出的原始数据 */
typedef struct _TianGuangIceData {
  uint8_t  Sensor_ID;            /* 地址码 */
  uint8_t  Cmd_Type;             /* 功能码 */
  uint8_t  Data_Len;             /* 数据长度 */
  float    Tension;              /* 拉压力（重量） */
  float    Incli_X;              /* X 轴倾角 */     
  float    Incli_Y;              /* Y 轴倾角 */  
  float    Temperature;          /* 环境温度 */  
  uint16_t CRC16;                /* CRC校验码 */
}TianGuangIceData_t;

/* 覆冰读出的原始数据 */
typedef struct _DT45IceData {
  uint8_t  Sensor_ID;            /* 地址码 */
  uint8_t  Cmd_Type;             /* 功能码 */
  uint8_t  Data_Len;             /* 数据长度 */
  int32_t  Tension;              /* 拉压力（重量） */
  uint16_t WorkStatus;              /* X 轴倾角 */     
  uint16_t CRC16;                /* CRC校验码 */
}DT45IceData_t;

/* 覆冰功能码 读指令：0x03 */
#define  WEATGHER_CMD_TYPE_READ_DATA   0x03
/* 覆冰命令 */
typedef struct _DeviceIceCmd {
  uint8_t  Sensor_ID;             /* 地址码 */
  uint8_t  Cmd_Type;              /* 功能码 */
  uint16_t Register_Start_addr;   /* 起始地址 */
  uint16_t Register_No;           /* 寄存器数量 */
  uint16_t CRC16;                 /* CRC校验码 */
}DeviceIceCmd_t;

/* 获取覆冰数据帧 */
typedef struct _DeviceIceData {
  uint8_t Sensor_ID;              /* 地址码 */
  uint8_t Cmd_Type;               /* 功能码 */
  uint8_t Data_Len;               /* 数据长度 */
  uint8_t data[2];
}DeviceIceData_t;
#define ICE_PACK_HEAD_SIZE(x)  (((char*)(x)->data) - (char*)(x))
#define ICE_PACK_SIZE(x)       ((char*)&((x)->data[(x)->Data_Len]) - (char*)(x) + sizeof(uint16_t))

/* 覆冰传感器信息，保存接口信息以及接收到的数据 */
typedef struct _DeviceIceParam {
  uint8_t ID[2];                    /* 传感器地址码，最多2个 */
  uint8_t Flag[2];                  /* 传感器是否轮询成功标志 */
  uint8_t Interface[2];             /* 传感器对应的接口标志 */
  TianGuangIceData_t IceData[2];    /* 传感器缓存的数据 */
}DeviceIceParam_t;
#pragma pack(pop)

/* 广播包处理，获取覆冰数据 */
BOOL DS_BroadcastMsgProc_GetIceData();
/* 主线程运行，采集覆冰数据，发送数据 */
void DS_DeviceIce_Run();
/* 初始化覆冰模块 , 为外部提供初始化接口 */
BOOL InitDeviceIceModule();

#endif


