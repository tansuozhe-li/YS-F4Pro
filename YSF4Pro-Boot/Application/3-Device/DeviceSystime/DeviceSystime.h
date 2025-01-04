#ifndef __DEVICE_SYSTIME_H_
#define __DEVICE_SYSTIME_H_

#include "common.h"
#include <stdint.h>
#include "thread_cfg.h"
#include "I1Gw2015Protocol.h"

#define LOCAL_TIME_ZONE 8   /* 东八区 */

typedef struct _sys_time{
    uint16_t   sec;
    uint16_t   min;
    uint16_t   hour;
    uint16_t   day; 
    uint16_t   month;   
    uint16_t   year;
}sys_time_t;
extern sys_time_t system_time;



/* 获取当前系统时钟的世纪秒 */
uint32_t get_current_centurysec();
/* 获取当前的系统时间 */
void get_systime(sys_time_t *time);
/* 更新系统时间 */
void update_systime(sys_time_t systime);
/* 系统时钟校时 */
BOOL Systime_BroadcastMsgProc_TimingSystime(mq_brodcast_msg *pMsg);
/* 系统时钟维护线程 */
void Systime_run();
/* 初始化模块定时器、信号量 */
BOOL InitModuleOtherFunc();
/* 初始化系统时钟模块 , 为外部提供初始化接口 */
BOOL InitDeviceSystimeModule();

#endif


