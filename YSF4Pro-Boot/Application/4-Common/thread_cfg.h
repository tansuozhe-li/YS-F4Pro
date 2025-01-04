#ifndef THREAD_CFG_MODULE_PRESENT
#define THREAD_CFG_MODULE_PRESENT

#define ENABLE_MODULE    1
#define DISABLE_MODULE   0

/**************************************************************************
*    软件编译配置，编译文件输出
**************************************************************************/
#define DS_SOFT_REBUILD_TIME                     "20241111" /* 软件编译时间 */
#define DS_SOFT_REBUILD_FLAG                     "01"       /* 软件编译标志 */

/**************************************************************************
*    版本配置
**************************************************************************/
/* 产品型号 */
#define DS_PRODUCT_MODULE                        "DXFF"     /* 产品型号 */

/* 软件版本号 三段式命名规则*/
#define DS_SOFT_MAINVERSION                      "1"   /* 主版本号 */            
#define DS_SOFT_SUBVERSION                       "2"   /* 子版本号 */     
#define DS_SOFT_EDITEVERSION                     "001"  /* 阶段性修订版本号 */     

/* 硬件版本号 */
#define DS_HARDWARE_MAINVERSION                  "1"    /* 主版本号 */       
#define DS_HARDWARE_SUBVERSION                   "1"    /* 子版本号 */     
#define DS_HARDWARE_EDITEVERSION                 "2"    /* 阶段性版本号 */

/**************************************************************************
*    线程优先级配置  RT-Thread 最大支持 256 个线程优先级 (0~255)，数值越小的
*    优先级越高，0 为最高优先级。Default: 32
**************************************************************************/
/* Bussiness层线程优先级配置 */
typedef enum {
  BUSINESS_DS_RUN_TASK_PRIO                          = 1u, /* 清蓉深瞳平台模块主任务优先级 */
  BUSINESS_DS_RECV_BROADCASE_MSG_TASK_PRIO           = 2u, /* 清蓉深瞳平台模块广播包任务优先级 */
  BUSINESS_DS_RECV_BACKGROUND_MSG_TASK_PRIO          = 3u, /* 清蓉深瞳平台模块后台消息任务优先级 */
  BUSINESS_DS_UPDATE_TASK_PRIO                       = 4u, /* 清蓉深瞳平台模块OTA任务优先级 */
}BussinessPriorityValue_e;                                                                                         
                                                 
/* Device层线程优先级配置  */                    
typedef enum {                                   
  DEVICE_LTE_RUN_TASK_PRIO                           = 5u, /* LTE模块主任务优先级 */
  DEVICE_LTE_MAIN_UART_READLN_TASK_PRIO              = 6u, /* LTE主通道串口任务优先级 */
  DEVICE_LTE_SUB_UART_READLN_TASK_PRIO               = 7u, /* LTE副通道串口任务优先级 */
  DEVICE_SYSTIME_RUN_TASK_PRIO                       = 14u, /* 系统时钟维护任务优先级 */
  DEVICE_SYSTIME_RUN_BROADCASE_MSG_TASK_PRIO         = 15u, /* 系统时钟广播任务优先级 */
  DEVICE_BATTERY_RUN_TASK_PRIO                       = 16u, /* 电池数据任务优先级 */
  DEVICE_BATTERY_RUN_BROADCASE_MSG_TASK_PRIO         = 17u, /* 电池数据任务广播包优先级 */
  DEVICE_WDOG_RUN_TASK_PRIO                          = 30u, /* 看门狗模块任务优先级 */
}DevicePriorityValue_e;

/* Common层线程优先级配置  */

 
/* 操作系统初始化线程  */
#define RT_FINSH_THREAD_PRIORITY                  21u /* tshell */
#define RT_MAIN_THREAD_PRIORITY                   10u /* main */
#define RT_IDLE_THREAD_PRIORITY                   31u /* idle */
/**************************************************************************
*    线程堆栈配置
**************************************************************************/   
#define TASK_STK_SIZE_1024                        1024u
#define TASK_STK_SIZE_2048                        2048u
#define TASK_STK_SIZE_4096                        4096u

#endif

