/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeviceBattery_main.c
*  内容摘要：  电池模块，采集电池电压，充电电压，电流
*  其他说明：
*  当前版本：  V1.0.000
*  作    者：  
*  开始日期：  2024年06月18日
*  
*  修改记录 1：
*        修改日期：2024年06月18日
*        修 改 人：黎庆昌
*        修改内容：创建工程文件，调试电池数据
*******************************************************************************/
#include "DeviceBattery.h"
#include "common.h"
#include "thread_cfg.h"

/* 广播消息接收客户端 */
static struct rt_broadcast_client   g_Client;
/* 广播消息接收缓存 */
static uint8_t                      g_MsgPool[600];
/* 广播消息 */
static mq_brodcast_msg              g_Msg;
/* 广播消息接收客户端名称 */
#define CLIENT_NAME                 "device_battery"

///@brief   广播消息客户端初始化
static int InitBroadcastClient()
{
    rt_err_t ret = rt_broadcast_client_init(&g_Client, CLIENT_NAME, g_MsgPool,\
                         sizeof(mq_brodcast_msg), sizeof(g_MsgPool), RT_IPC_FLAG_FIFO);
    AssertError(RT_EOK == ret, return -1, "broadcast client init failed, error no %d", ret);
    rt_broadcast_client_regist(&g_mq_broadcast_server, &g_Client);
    return 0;
}

/*   任务(线程)间广播包消息处理表   */
static BroadcastMsgProcTable_t g_MsgTbl[] = {
    { "get_battery",  "获取电池数据", BroadcastMsgProc_GetBatteryData},
};

///@brief   任务(线程)间消息处理
static void BroadcastMsgHandler(mq_brodcast_msg *pMsg)
{
    for (uint32_t i = 0; i < sizeof(g_MsgTbl) / sizeof(BroadcastMsgProcTable_t); i++) {
      if (0 == strcmp((const char*)pMsg->msg_type, (const char*)g_MsgTbl[i].msgType)) {
          LOG_D("process intertask msg %s:%s start", g_MsgTbl[i].msgType, g_MsgTbl[i].msgName);
          g_MsgTbl[i].pMsgProc(pMsg);
          LOG_D("process intertask msg %s:%s end", g_MsgTbl[i].msgType, g_MsgTbl[i].msgName);
        }
    }
}

///@brief   接收广播消息任务
static void DS_RecvBroadcastMsgTask(void *args)
{
    while(1)
    {
        memset(&g_Msg, 0, sizeof(mq_brodcast_msg));
        int ret = rt_broadcast_recv(&g_Client, &g_Msg, sizeof(mq_brodcast_msg), RT_WAITING_FOREVER);
        AssertContinueNoPrint(RT_EOK == ret, rt_thread_delay(100));
        SetTaskStatus(DEVICE_BATTERY_RUN_BROADCASE_MSG_TASK_PRIO, TASK_WORKING);
        /* TODO为了节约线程资源,不再单独起一个线程来处理消息 */
        BroadcastMsgHandler(&g_Msg);
        SetTaskStatus(DEVICE_BATTERY_RUN_BROADCASE_MSG_TASK_PRIO, TASK_IDLE);
        mdelay(5);
    }
}

///@brief   任务运行，主线程
static void DS_RunTask(void *args)
{
    while (1)
    {
        Collect_Battery_Data();
        mdelay(1000);
    }
}

/*   在此添加任务(线程)   */
static task_info_t tasks_info[] = {
  {"battery_broadcast", TASK_STK_SIZE_1024, DEVICE_BATTERY_RUN_BROADCASE_MSG_TASK_PRIO, DS_RecvBroadcastMsgTask, 0},
  {"battery_run",  TASK_STK_SIZE_1024, DEVICE_BATTERY_RUN_TASK_PRIO, DS_RunTask, 0},
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
    InitDataRTMutex();
    int ret = InitBroadcastClient();
    AssertErrorNoPrint(0 == ret, return FALSE);
    LoginTaskId(DEVICE_BATTERY_RUN_TASK_PRIO);
    LoginTaskId(DEVICE_BATTERY_RUN_BROADCASE_MSG_TASK_PRIO);
    
    for (int i = 0; i < sizeof(tasks_info)/sizeof(task_info_t); ++i){
        LOG_D("starting : %s as task %d",tasks_info[i].name, i);
        AssertContinueNoPrint(tasks_info[i].threadFunc != NULL, {});
        InitCreateTask(i, tasks_info[i].name, tasks_info[i].threadFunc, tasks_info[i].stk_size, tasks_info[i].prio);
    }
    return TRUE;
}

///@brief  初始化电池模块
BOOL InitDeviceBatteryModule()
{   
    // 初始化模块通用功能
    InitModuleCommonFunc();
    
    return 0;
}