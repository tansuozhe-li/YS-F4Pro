/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeviceLte_main.c
*  内容摘要：  Lte模块初始化
*  当前版本：  EIT-L_V1.0.014
*  作    者：  
*  完成日期：  2023年12月28日
*  
*  修改记录 1：
*        修改日期：2023年12月28日
*        修 改 人：黎庆昌
*        修改内容：创建工程文件
*  修改记录 2：
*        修改日期：2024年2月21日
*        修 改 人：黎庆昌
*        修改内容：掉线重播功能增加对4G模块的关机操作
*******************************************************************************/
#include "DeviceLte.h"

///@brief 获取拨号配置
static void GetMainLteDialConfig(lte_device_handle lte_device)
{
    lte_device_info_t *device = (lte_device_info_t *)lte_device;
    memcpy(device->stlte_chn_table.stchn_table[0].serv_ip,DeviceLteModuleCfg.LteMain_Ch0SrvIP, strlen(DeviceLteModuleCfg.LteMain_Ch0SrvIP));
    device->stlte_chn_table.stchn_table[0].port = DeviceLteModuleCfg.LteMain_Ch0SrvPort;
    memcpy(device->stlte_chn_table.stchn_table[0].net_type,DeviceLteModuleCfg.LteMain_Ch0NetType,strlen(DeviceLteModuleCfg.LteMain_Ch0NetType));
    memcpy(device->stlte_chn_table.stchn_table[1].serv_ip,DeviceLteModuleCfg.LteMain_Ch1SrvIP, strlen(DeviceLteModuleCfg.LteMain_Ch1SrvIP));
    device->stlte_chn_table.stchn_table[1].port = DeviceLteModuleCfg.LteMain_Ch1SrvPort;
    memcpy(device->stlte_chn_table.stchn_table[1].net_type,DeviceLteModuleCfg.LteMain_Ch1NetType,strlen(DeviceLteModuleCfg.LteMain_Ch1NetType));
}

///@brief 获取拨号配置
static void GetSubLteDialConfig(lte_device_handle lte_device)
{
    lte_device_info_t *device = (lte_device_info_t *)lte_device;
    memcpy(device->stlte_chn_table.stchn_table[0].serv_ip,DeviceLteModuleCfg.LteSub_Ch0SrvIP, strlen(DeviceLteModuleCfg.LteSub_Ch0SrvIP));
    device->stlte_chn_table.stchn_table[0].port = DeviceLteModuleCfg.LteSub_Ch0SrvPort;
    memcpy(device->stlte_chn_table.stchn_table[0].net_type,DeviceLteModuleCfg.LteSub_Ch0NetType,strlen(DeviceLteModuleCfg.LteSub_Ch0NetType));
}

///@brief  任务运行，主线程,实现网络监测，拨号，初始化等操作
static void Lte_RunTask(void *args)
{    
    // 根据配置拨号
    DeviceLteModuleCfg.LteMain_Enable = TRUE;
    if (DeviceLteModuleCfg.LteMain_Enable == TRUE){
        GetMainLteDialConfig(g_main_lte_handle);
        lte_chn_dial(g_main_lte_handle);
        LTE1_SLEEP();
    }

//    if (DeviceLteModuleCfg.LteSub_Enable == TRUE){
//        GetSubLteDialConfig(g_sub_lte_handle);
//        lte_chn_dial(g_sub_lte_handle);
//        LTE2_SLEEP();
//    }
    int delay_cnt = 0;
    while (1)
    {
        delay_cnt++;
        mdelay(1000); 
        // 每1个小时检查一次网络状态
        AssertContinueNoPrint(delay_cnt > 600, {});
        // 检测网络状态，发起重新拨号流程
        if ((DeviceLteModuleCfg.LteMain_Enable == TRUE) && (get_lte_chn_status(g_main_lte_handle, LTE1_CHN_DS) != LTE_NET_CONNECTED_SUCCESS)) {
            // 模块关机
            ec800k_power_down(g_main_lte_handle);
            delay(12);
            lte_chn_dial(g_main_lte_handle);
        }
        
        // 检测网络状态，发起重新拨号流程
//        if ((DeviceLteModuleCfg.LteSub_Enable == TRUE) && (get_lte_chn_status(g_sub_lte_handle, LTE1_CHN_DS) != LTE_NET_CONNECTED_SUCCESS)) {
//            // 模块关机
//            ec800k_power_down(g_sub_lte_handle);
//            delay(12);
//            lte_chn_dial(g_sub_lte_handle);
//        }
        delay_cnt = 0;
        mdelay(5); 
    }
}

/*   在此添加任务(线程)   */
static task_info_t tasks_info[] = {
 {"lte_RunTask", TASK_STK_SIZE_1024, DEVICE_LTE_RUN_TASK_PRIO, Lte_RunTask, 0}, 
 {"lte_main_readln", TASK_STK_SIZE_1024, DEVICE_LTE_MAIN_UART_READLN_TASK_PRIO, DS_MainLteReadLnTask, 0},
 {"lte_sub_readln", TASK_STK_SIZE_1024, DEVICE_LTE_SUB_UART_READLN_TASK_PRIO, DS_SubLteReadLnTask, 0},
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
    // 注册状态ID
    for (int i = 0; i < sizeof(tasks_info)/sizeof(task_info_t); ++i){
        LOG_D("starting: %s as task %d",tasks_info[i].name, i);
        AssertContinueNoPrint(tasks_info[i].threadFunc != NULL, {});
        InitCreateTask(i, tasks_info[i].name, tasks_info[i].threadFunc, tasks_info[i].stk_size, tasks_info[i].prio);
    }
    return TRUE;
}

///@brief  初始化Lte模块
BOOL InitDeviceLteModule()
{   
    lte_device_main_init();

    lte_device_sub_init();
    // 初始化模块通用功能
    InitModuleCommonFunc();
    
    return 0;
}