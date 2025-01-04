/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeviceWdog_main.c
*  内容摘要：  看门狗模块初始化
*  其他说明：
*  当前版本：  EIT-L_V1.0.0
*  作    者：  
*  完成日期：  2023年12月15日
*  
*  修改记录 1：
*        修改日期：2023年12月15日
*        修 改 人：
*        修改内容：创建工程文件
*  修改记录 2：
*******************************************************************************/
#include "DeviceWdog.h"
#include "thread_cfg.h"

///@brief   任务运行，主线程
static void DS_RunTask(void *args)
{
    // 等待系统启动
    while (1)
    {
        WDOG_DONE_Toggle;
        mdelay(1000);
    }
}

/*   在此添加任务(线程)   */
static task_info_t tasks_info[] = {
  {"wdog_run",  TASK_STK_SIZE_1024, DEVICE_WDOG_RUN_TASK_PRIO, DS_RunTask, 0},
};

///@brief   创建任务（线程）
static BOOL InitCreateTask(int task_id, char *name, void (*start_routine)(void *), int stk_size, int prio)
{
  tasks_info[task_id].thread = rt_thread_create(name, start_routine, NULL, stk_size, prio, 20);
  AssertError(tasks_info[task_id].thread != RT_NULL, return FALSE, "create thread:%s failed", name);
  rt_thread_startup(tasks_info[task_id].thread);
  
  return TRUE;
}

///@brief  初始化通用模块
static BOOL InitModuleCommonFunc()
{    
    for (int i = 0; i < sizeof(tasks_info)/sizeof(task_info_t); ++i){
        LOG_D("starting : %s as task %d",tasks_info[i].name, i);
        AssertContinueNoPrint(tasks_info[i].threadFunc != NULL, {});
        InitCreateTask(i, tasks_info[i].name, tasks_info[i].threadFunc, tasks_info[i].stk_size, tasks_info[i].prio);
    }
    return TRUE;
}

///@brief  初始化微气象模块
BOOL InitDeviceWdogModule()
{   
    // 初始化模块通用功能
    InitModuleCommonFunc();
    
    return 0;
}