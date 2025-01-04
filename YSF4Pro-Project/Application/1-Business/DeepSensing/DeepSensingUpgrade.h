#ifndef _DEEP_SENSING_UPDATE_H
#define _DEEP_SENSING_UPDATE_H

#include "I1Gw2015Protocol.h"
#include "common.h"

#define UPGRADE_DATA_OK   1
#define UPGRADE_DATA_ERR  0
#define UPDATE_FRAME_MAX_NUM  512

#define OTA_UPGRADE_INFORM_FLAG          (*((uint32_t *)"EITL"))
#define OTA_UPGRADE_FILE_TYPE_APP        0x01  /* app应用程序 */
#define OTA_UPGRADE_FILE_TYPE_BOOT       0x02  /* bootloader程序 */
#define OTA_UPGRADE_FILE_TYPE_CFG        0x03  /* 配置文件 */
#define OTA_UPGRADE_FILE_TYPE_KEY        0x04  /* 密钥 */
#define OTA_UPGRADE_FILE_TYPE_CERT       0x05  /* 加密证书 */

/* 升级步骤进行的状态 */
typedef enum {
  UPDATE_STATE_DISCONNECT = 0, /* 初始状态，未连接到升级端 */
  UPDATE_STATE_IDLE       = 1, /* 空闲状态，系统正常运行，无需升级 */
  UPDATE_STATE_INFORM     = 2, /* 升级通知，后台已下发升级通知 */
  UPDATE_STATE_TRANSMIT   = 3, /* 升级数据正在传输 */
  UPDATE_STATE_FINISH     = 4, /* 升级数据传输完成，已向后台发送传输确认报文 */
  UPDATE_STATE_ERROR      = 5, /* 升级数据传输失败 */
}DSUpgradestate_e;

#pragma pack(push, 1)
/* 升级过程状态参数 */
typedef struct _DSUpgradeFrameInfo {
  uint8_t  upgrade_frame_state;
  uint16_t upgrade_frame_index;
  uint16_t upgrade_frame_lenth;
}UpFrameState_t;

/* 升级文件信息参数 */
typedef struct _DSUpgradeStateParam {
  uint8_t  upgrade_state;  /* 升级状态 */
  uint16_t file_length;
  uint16_t file_frame_len;
  uint16_t file_frame_num;
}DSUpgradeStateParam_t;
#pragma pack(pop)

/* bootloader地址信息--片内 */
#define UPDATE_BOOTLOADER_ADDR_BEGIN   (0x08000000)  //片内Bootloader起始地址
#define UPDATE_BOOTLOADER_ADDR_END     (0x0801BFFF)  //片内Bootloader结束地址，共112KB，Bank1: Page0-Page55
//应用程序包头存储地址信息--片内
#define UPDATE_APP_PKG_HEAD_BEGIN      (0x0801C000)  //片内应用程序包头起始地址
#define UPDATE_APP_PKG_HEAD_END        (0x0801FFFF)  //片内应用程序包头结束地址，共16KB, Bank1: Page56-Page63
#define UPDATE_APP_PKG_HEAD            ((UpdateFileHead*)UPDATE_APP_PKG_HEAD_BEGIN)
/* 应用程序地址信息--片内Flash */
#define UPDATE_APP_ADDR_BEGIN          (0x08020000)  //片内应用程序起始地址
#define UPDATE_APP_ADDR_END            (0x0807FFFF)  //片内应用程序结束地址，共384kB，Bank1:Page64 -Page255

///@brief OTA数据响应报--OTA升级通知
BOOL DS_MsgProc_UpateInformDataResp(GW2015FrameHeader_t *pFrame);
///@brief OTA数据响应报--OTA升级数据报
BOOL DS_MsgProc_UpdateFrameDataResp(GW2015FrameHeader_t *pFrame);
///@brief OTA数据响应报--OTA升级数据结束报
BOOL DS_MsgProc_UpdateFinishDataResp(GW2015FrameHeader_t *pFrame);
///@brief 在主线程中循环判断OTA升级功能是否需要开启
void DS_UpdateTask();

#endif