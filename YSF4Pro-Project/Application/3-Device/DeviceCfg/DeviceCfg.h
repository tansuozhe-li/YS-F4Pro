#ifndef __DEVICE_Cfg_H_
#define __DEVICE_Cfg_H_

#include "common.h"
#include <stdint.h>
extern rt_mutex_t flash_mutex;
/* 使用flash快速分配的用户ID, 每个单位ID 等于1个EXT_FLASH_SECTOR_SIZE（1024*4 = 4KB）*/
typedef enum {
    FLASH_ID_Config_File_Head      = 0,    /* 配置文件的文件头存储ID */ 
    FLASH_ID_Config_File_Param     = 1,    /* 配置文件的内容参数存储ID  */
    FLASH_ID_Systime_Module        = 10,   /* Systime时钟模块参数存储ID */
             
    FLASH_ID_OTA_FILE_CACHE_STATE  = 960,  /* OTA文件缓存区升级状态存储ID */
    FLASH_ID_OTA_FILE_CACHE_HEAD   = 992,  /* OTA文件缓存区升级文件头存储ID */
    FLASH_ID_OTA_FILE_CACHE_START  = 1024, /* OTA文件缓存区升级内容存储ID */

    MAX_FLASH_ID           = 4096, /* FLASH存储ID的最大值 */
} FLASH_ID_USER_e;

#pragma pack(push,1)
/* DeviceLte模块配置 */
typedef struct _DeviceLteModuleCfg {
  BOOL     LteMain_Enable;
  char     LteMain_APN[10];
  char     LteMain_Ch0SrvIP[20];
  uint32_t LteMain_Ch0SrvPort;
  char     LteMain_Ch0NetType[20];
  char     LteMain_Ch1SrvIP[20];
  uint32_t LteMain_Ch1SrvPort;
  char     LteMain_Ch1NetType[20];
  BOOL     LteSub_Enable;
  char     LteSub_APN[10];
  char     LteSub_Ch0SrvIP[20];
  uint32_t LteSub_Ch0SrvPort;
  char     LteSub_Ch0NetType[20];
}DeviceLteModuleCfg_t;
extern DeviceLteModuleCfg_t DeviceLteModuleCfg;

/* DsPlatform清蓉深瞳平台配置 */
typedef struct _DsPlatformModuleCfg {
  BOOL      Enable;
  uint8_t   ChSelect;
  uint16_t  TimeInterval;
  uint16_t  StatusInterval;
  char      CmdId[17];
}DsPlatformModuleCfg_t;
extern DsPlatformModuleCfg_t DsPlatformModuleCfg;

/* 覆冰模块配置 */
typedef struct _DeviceIceModuleCfg {
  BOOL      Enable;
  uint8_t   ID[2];
  uint16_t  TimeInterval;
  float     Angle_X;
  float     Angle_Y;
  char      sensor_type[16];
  char      CompoentCmdId[17];
}DeviceIceModuleCfg_t;
extern DeviceIceModuleCfg_t DeviceIceModuleCfg;

#pragma pack(pop)


///@brief 通过ID返回片外FLASH的实际地址；
u_int32_t  IDToAddr(FLASH_ID_USER_e flash_id);
///@brief 按宏定义ID擦除对应的Sector
void FlashID_Erase_Sector(FLASH_ID_USER_e flash_id);
///@brief 按宏定义ID写入信息到flash
void  FlashID_Write_MorePage(u_int8_t *pBuffer, FLASH_ID_USER_e flash_id, uint32_t offset, u_int32_t WriteBytesNum);
///@brief 按宏定义ID读取flash的信息
void  FlashID_Read(u_int8_t *pBuffer,FLASH_ID_USER_e flash_id, uint32_t offset, u_int32_t ReadBytesNum);
///@brief   初始化配置模块，获取相关配置参数
BOOL InitDeviceCfgModule();

#endif


