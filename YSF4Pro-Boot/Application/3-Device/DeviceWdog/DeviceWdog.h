#ifndef __DEVICE_WDOG_H_
#define __DEVICE_WDOG_H_

#include "common.h"
#include "main.h"

#define WDOG_DONE_Toggle HAL_GPIO_TogglePin(WDG_DONE_GPIO_Port, WDG_DONE_Pin)

/* 初始化看门狗模块 */
BOOL InitDeviceWdogModule();

#endif


