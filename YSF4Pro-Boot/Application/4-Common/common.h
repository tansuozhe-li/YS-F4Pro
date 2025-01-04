#ifndef _COMMON_H_
#define _COMMON_H_
#include <rtthread.h>
#include "string.h"
#include "comm_types.h"
#include "common_broadcast.h"
#include "rtdbg.h"
#include "assertions.h"
#include "commlib.h"
#include "DeviceCfg.h"
#include "main.h"

/********************************* 设备重启 ***********************************/

#define EITL_REBOOT()   NVIC_SystemReset()


/************************** 通用的模块线程任务信息 ****************************/
/* 模块线程任务信息 */
typedef struct _task_info{
  char name[64];             /* 线程名字 */
  int  stk_size;             /* 堆栈大小 */
  int  prio;                 /*  优先级*/
  void (*threadFunc)(void*);
  rt_thread_t thread;
}task_info_t;


/************************** 通用的模块线程状态信息 ****************************/
/* 模块线程状态 */
#define MAX_TASK_ID  RT_THREAD_PRIORITY_MAX   //与RT_THREAD_PRIORITY_MAX值保持相同
typedef enum {
  TASK_READY =          0,        /* 准备 */
  TASK_WORKING =        1,        /* 处理中（默认） */
  TASK_IDLE =           2,        /* 空闲中 */
}task_status_e;
typedef enum {
  TASK_LOGIN_FINISH =   1,        /* 完成注册标志 */
  TASK_LOGIN_UNFINISH = 2,        /* 未完成注册 */
}task_flag_e;
typedef struct {
    u_int8_t login_total; 
    u_int8_t status[MAX_TASK_ID]; 
    u_int8_t login_flag[MAX_TASK_ID];
} task_status_t;
extern task_status_t stTaskStatus;
void LoginTaskId(u_int8_t task_id);
void SetTaskStatus(u_int8_t task_id, task_status_e status);


/************************** 通用的配置处理表及函数 ****************************/
/* 配置处理函数 */
typedef void (*SysCfgProcFunc)(uint32_t cfgtalno, BOOL status, u_int8_t *buff);
/* 通用的配置处理表 */
typedef struct _SysCfgProcTable {
  u_int8_t cfgmodulename[20];    /* 配置模块名 */     
  u_int8_t cfgSection[20];       /* 文件Section */
	u_int8_t cfgKeyName[20];       /* 文件Name名字 */
	u_int8_t cfgDefaultValue[20];  /* 文件默认值 */
  u_int8_t cfgActualValue[20];   /* 文件实际值 */
  SysCfgProcFunc  pSysCfgParamProc;  /* 配置参数处理函数 */
} SysCfgProcTable_t;
extern SysCfgProcTable_t g_SysCfgTbl[];
extern uint8_t cfg_proc_table_size;


/**************************** 各模块数据格式结构 ******************************/
#pragma pack(push,1)
/* DeviceBattery 电池模块数据结构 */
typedef struct _DeviceBatteryParam {
  uint8_t  FloatingCharge;        /* 浮充状态：①0x00 充电  ②0x01 放电 */
  uint8_t  Charge_Type;           /* 充电类型 */
  float    Battery_Temp;          /* 电池温度 */
  float    Battery_Voltage;       /* 电池工作电压 */
  float    Battery_Current;       /* 电池工作电流 */
  float    Charge_Current;        /* 充电电流 */
  float    Open_voltage;          /* 开路电压 */
  float    Battery_Capacity;      /* 电池剩余电量（浮点数，精确到小数点后 1 位，单位：Ah）*/
}DeviceBatteryParam_t;

/* DeviceSystime系统时钟模块数据结构 */
typedef struct _sys_work_time{
  u_int32_t Clocktime_Stamp;      /* 当前时间 */
  u_int32_t Total_Working_Time;   /* 工作总时间（无符号整数，单位：小时） */
  u_int32_t Working_Time;         /* 本次连续工作时间（无符号整数，单位：小时） */
}sys_work_time_t;
extern sys_work_time_t work_time;
extern rt_time_t DS_SysTime;

/* 升级超时监控 */
typedef struct _DSUpgradeOverTime {
  int  start_waiting_time;
  int  upgrade_failed_time;
}DSUpgradeOverTime_t;
extern DSUpgradeOverTime_t DSUpgradeOverTime;

#pragma pack(pop)

#endif