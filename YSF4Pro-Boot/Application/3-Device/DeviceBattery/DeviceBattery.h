#ifndef __DEVICE_BATTERY_H_
#define __DEVICE_BATTERY_H_

#include "common.h"
#include <stdint.h>
#include "adc.h"

typedef enum {
  BATTERY_ADC_CHANNEL_BAT_VDET   = ADC_CHANNEL_11,
  BATTERY_ADC_CHANNEL_VSO_VDET   = ADC_CHANNEL_12,
  BATTERY_ADC_CHANNEL_CHRG_DET   = ADC_CHANNEL_15,  
}Battery_Adc_Channel_e;

///@brief   广播获取维护模块数据
BOOL BroadcastMsgProc_GetBatteryData(mq_brodcast_msg *pMsg);
///@brief  设备维护模块主任务
void Collect_Battery_Data();
///@brief 初始化动态互斥锁
BOOL InitDataRTMutex();
///@brief  初始化MainTain模块 , 为外部提供初始化接口
BOOL InitDeviceBatteryModule();

#endif


