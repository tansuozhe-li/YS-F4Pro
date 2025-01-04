#ifndef __DEVICE_LTE_H_
#define __DEVICE_LTE_H_

#include "common.h"
#include "ec800k.h"

void DS_MainLteReadLnTask();
void DS_SubLteReadLnTask();
///@brief  初始化Lte模块
BOOL InitDeviceLteModule();
#endif


