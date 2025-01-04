#ifndef __DEVICE_LTE_H_
#define __DEVICE_LTE_H_

#include "common.h"
#include "ec800k.h"

BOOL InitDeviceLteModule();
void DS_MainLteReadLnTask();
void DS_SubLteReadLnTask();

#endif


