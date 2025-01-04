#ifndef THREAD_CFG_MODULE_PRESENT
#define THREAD_CFG_MODULE_PRESENT

/**************************************************************************
*    模块配置，未开启不编译
**************************************************************************/ 


/**************************************************************************
*    软件编译配置，编译文件输出:
*    1、使用Bootloader升级APP文件 打开USER_VECT_TAB_ADDRESS宏定义，否则关闭
*    2、在工程 Options->Linker->Config 选择stm32l496xx_flash_app.icf文件
*    3、在Output converter修改文件名内容，格式为：DS_DXFF_APP_Sv1.0.010_20240704_01.bin
**************************************************************************/
#define USER_VECT_TAB_ADDRESS                  /* 编译升级文件时 打开宏定义，仿真调试关闭 */

#define DS_SOFT_REBUILD_TIME                     "20241201" /* 软件编译时间 */
#define DS_SOFT_REBUILD_FLAG                     "01"       /* 软件编译标志 */

/**************************************************************************
*    版本配置
**************************************************************************/
/* 产品型号 */
#define DS_PRODUCT_MODULE                        "EITL"     /* 产品型号 */

/* 软件版本号 三段式命名规则*/
#define DS_SOFT_MAINVERSION                      "1"   /* 主版本号 */            
#define DS_SOFT_SUBVERSION                       "0"   /* 子版本号 */     
#define DS_SOFT_EDITEVERSION                     "600"  /* 阶段性修订版本号 */     

/* 硬件版本号 */
#define DS_HARDWARE_MAINVERSION                  "1"    /* 主版本号 */       
#define DS_HARDWARE_SUBVERSION                   "0"    /* 子版本号 */     
#define DS_HARDWARE_EDITEVERSION                 "0"    /* 阶段性版本号 */
                               
/**************************************************************************
*    线程优先级配置  RT-Thread 最大支持 256 个线程优先级 (0~255)，数值越小的
*    优先级越高，0 为最高优先级。Default: 32
**************************************************************************/
/* Bussiness层线程优先级配置 */
typedef enum {
  BUSINESS_DS_RUN_TASK_PRIO                       = 8u,  
  BUSINESS_DS_RECV_BROADCASE_MSG_TASK_PRIO        = 7u,  
  BUSINESS_DS_RECV_BACKGROUND_MSG_TASK_PRIO       = 6u,  
  BUSINESS_DS_UPDATE_TASK_PRIO                    = 4u, 
  BUSINESS_NW_RUN_TASK_PRIO                       = 5u,  
  BUSINESS_NW_RECV_BROADCASE_MSG_TASK_PRIO        = 15u,  
  BUSINESS_NW_RECV_BACKGROUND_MSG_TASK_PRIO       = 3u, 
}BussinessPriorityValue_e;                                            
                                                                                         
/* Device层线程优先级配置  */                    
typedef enum {
  DEVICE_LTE_RUN_TASK_PRIO                        = 9u, /* LTE模块主任务优先级 */
  DEVICE_LTE_MAIN_UART_READLN_TASK_PRIO           = 1u,  /* LTE主通道串口任务优先级 */
  DEVICE_LTE_SUB_UART_READLN_TASK_PRIO            = 2u, /* LTE副通道串口任务优先级 */
  DEVICE_ICE_RUN_TASK_PRIO                        = 11u, /* 覆冰模块主任务优先级 */
  DEVICE_ICE_RECV_BROADCASE_MSG_TASK_PRIO         = 12u, /* 覆冰模块广播包任务优先级 */
  DEVICE_SYSTIME_RUN_TASK_PRIO                    = 13u, /* 系统时钟维护任务优先级 */
  DEVICE_SYSTIME_RUN_BROADCASE_MSG_TASK_PRIO      = 14u, /* 系统时钟广播任务优先级 */
}DevicePriorityValue_e;

/* 操作系统初始化线程  */
#define RT_FINSH_THREAD_PRIORITY                  20u /* tshell */
#define RT_MAIN_THREAD_PRIORITY                   10u /* main */
/**************************************************************************
*    线程堆栈配置
**************************************************************************/   
#define TASK_STK_SIZE_1024                        1024u
#define TASK_STK_SIZE_2048                        2048u
#define TASK_STK_SIZE_4096                        4096u

#endif

