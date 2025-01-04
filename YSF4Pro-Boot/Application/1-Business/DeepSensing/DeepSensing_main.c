/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeepSensing_main.c
*  内容摘要：  DS模块初始化
*  其他说明：
*  当前版本：  EIT-L_V1.0.0
*  作    者：  
*  完成日期：  2023年11月20日
*  
*  修改记录 1：
*        修改日期：2023年11月20日
*        修 改 人：
*        修改内容：创建工程文件
*  修改记录 2：
*******************************************************************************/
#include "common.h"
#include "thread_cfg.h"
#include "DeepSensing.h"
#include "DeepSensingUpdate.h"

/* 广播消息接收客户端 */
static struct rt_broadcast_client   g_Client;
/* 广播消息接收缓存 */
static uint8_t                      g_MsgPool[600];
/* 广播消息 */
static mq_brodcast_msg              g_Msg;
/* 来自清蓉深瞳后台的信息 */
uint8_t                             g_FrameBuff[1500];
#define CLIENT_NAME                 "business_deepsensing"

static void DS_RunTask(void *args);

///@brief  广播消息客户端初始化
static int InitBroadcastClient(void)
{
    rt_err_t ret = rt_broadcast_client_init(&g_Client, CLIENT_NAME, g_MsgPool, sizeof(mq_brodcast_msg), sizeof(g_MsgPool), RT_IPC_FLAG_FIFO);
    AssertError(RT_EOK == ret, return -1, "broadcast client init failed, error no %d", ret);
    rt_broadcast_client_regist(&g_mq_broadcast_server, &g_Client);
    return 0;
}

/*   任务(线程)间广播包消息处理表   */
static BroadcastMsgProcTable_t g_MsgTbl[] = {
    { "battery_data",  "设备电池数据",   DS_BroadcastMsgProc_BatteryData},
};

///@brief   任务(线程)间消息处理
static void BroadcastMsgHandler(mq_brodcast_msg *pMsg)
{
    for (uint32_t i = 0; i < sizeof(g_MsgTbl) / sizeof(BroadcastMsgProcTable_t); i++) {
      if (0 == strcmp((const char*)pMsg->msg_type, (const char*)g_MsgTbl[i].msgType)) {
          //LOG_D("process intertask msg %s:%s start", g_MsgTbl[i].msgType, g_MsgTbl[i].msgName);
          g_MsgTbl[i].pMsgProc(pMsg);
          //LOG_D("process intertask msg %s:%s end", g_MsgTbl[i].msgType, g_MsgTbl[i].msgName);
        }
    }
}

///@brief   监听其他线程发来的广播消息
static void DS_RecvBroadcastMsgTask(void *args)
{
    while(1) {
        memset(&g_Msg, 0, sizeof(mq_brodcast_msg));
        int ret = rt_broadcast_recv(&g_Client, &g_Msg, sizeof(mq_brodcast_msg), RT_WAITING_FOREVER);
        AssertContinueNoPrint(RT_EOK == ret, rt_thread_delay(5));
        /* TODO为了节约线程资源,不再单独起一个线程来处理消息 */
        BroadcastMsgHandler(&g_Msg);
    }
}

/*   与后台通讯协议处理   */
static DSMsgProcFrameFuncTable_t g_DSMsgProcTbl[] =
{
  { "工作状态响应报", "心跳数据报",  GW2015_PACK_TYPE_C1H_HEARTBEAT_DATA, DS_MsgProc_WorkHeartbeatDataResp},
  { "控制数据响应报", "状态监测装置复位", GW2015_PACK_TYPE_A6H_DEV_RESET, DS_MsgProc_ControlMonitorDevReboot},
  { "OTA数据响应报", "OTA升级通知报", DS_PACK_TYPE_D1H_OTA_UPGRADE_INFORM, DS_MsgProc_UpateInformDataResp},
  { "OTA数据响应报", "OTA升级数据报", DS_PACK_TYPE_D2H_OTA_UPGRADE_DATA, DS_MsgProc_UpdateFrameDataResp},
  { "OTA数据响应报", "OTA升级结束报", DS_PACK_TYPE_D3H_OTA_UPGRADE_FINISH, DS_MsgProc_UpdateFinishDataResp},
  { "自定义数据响应报", "设备状态数据报", DS_PACK_TYPE_D5H_DEV_WORK_STATUS_DATA, DS_Msgproc_DeviceStateDataResp},
};

///@brief   与后台通讯协议处理
static void DS_MsgProcHandler(GW2015FrameHeader_t *pstFrame)
{
    for (uint32_t i = 0; i < sizeof(g_DSMsgProcTbl) / sizeof(DSMsgProcFrameFuncTable_t); i++)
    {
        AssertContinueNoPrint((pstFrame->Frame_Type == GW2015_FRAME_TYPE_02H_MONITOR_DATA_RESP) ||  /* 数据响应报 */
                              (pstFrame->Frame_Type == GW2015_FRAME_TYPE_03H_CONTROL_DATA_REPORT) ||  /* 控制数据报 */
                              (pstFrame->Frame_Type == GW2015_FRAME_TYPE_0AH_WORK_STATUS_RESP) || /* 工作状态响应报 */
                              (pstFrame->Frame_Type == DS_FRAME_TYPE_20H_CMD_DATA_REPORT), {});  /* 自定义数据指令报 */
        AssertContinueNoPrint(pstFrame->Packet_Type == g_DSMsgProcTbl[i].msgType, {});
        AssertContinueNoPrint(g_DSMsgProcTbl[i].pMsgProc != NULL, {});
        LOG_D("process ds [%02X]%s %s start" , g_DSMsgProcTbl[i].msgType, g_DSMsgProcTbl[i].frameName, g_DSMsgProcTbl[i].packName);
        BOOL bRet = g_DSMsgProcTbl[i].pMsgProc(pstFrame);
        LOG_D("process ds [%02X]%s %s end" , g_DSMsgProcTbl[i].msgType, g_DSMsgProcTbl[i].frameName, g_DSMsgProcTbl[i].packName);
    }
}

///@brief   监听清蓉深瞳服务器发来的消息
void DS_RecvDSBackgroundMsgTask(void *args)
{   
    
    while (1)
    {
        rt_thread_delay(5);
        uint16_t recv_pack_len = 0;
        memset(g_FrameBuff, 0, sizeof(g_FrameBuff));
        GW2015FrameHeader_t *p_frame = (GW2015FrameHeader_t *)g_FrameBuff;
        // 获取数据
        recv_pack_len = lte_uart_read_pack(g_FrameBuff, sizeof(g_FrameBuff), g_main_lte_handle, LTE1_CHN_DS);
        AssertContinueNoPrint(recv_pack_len != 0, {});
        //LOG_HEX("g_FrameBuff", g_FrameBuff, recv_pack_len);
        AssertContinue(UnShortToHighLevelTransfer(RTU_CRC((uint8_t *)&p_frame->Packet_Length, recv_pack_len-5)) == GW2015_PACK_CRC16(p_frame), {}, 
                       "CRC16 Expect: %04x", UnShortToHighLevelTransfer(RTU_CRC((uint8_t *)&p_frame->Packet_Length, recv_pack_len-5)));
        AssertContinue(g_FrameBuff[recv_pack_len-1] == GW2015_FRAME_END_FLAG, {}, 
                       "Frame End Flag expect: %02x", GW2015_FRAME_END_FLAG);
        // 消息处理
        ds_msgproc_frame_no = p_frame->Frame_No;
        DS_MsgProcHandler(p_frame);
    }
}

///@brief   任务运行
static void DS_RunTask(void *args)
{    
    rt_time_t HeartbeatNowTime, HeartbeatLastTime=0;
    lte_device_info_t *device = (lte_device_info_t *)g_main_lte_handle; 
    // 等待系统启动
    while (1)
    {   
        mdelay(10);
        if (Recv_Heartbeat_flag == GW2015DataStateOK) {
            //LED_SYS_CLOSE;
        } else {
            //LED_SYS_TWINKLING();
        }
        
        //  Bootloader超时等待
        DS_OverTimeLoad();
        
        // 等待通道链路建立成功
        AssertContinueNoPrint(device->stlte_chn_table.dial_status == LTE_NET_CONNECTED_SUCCESS, {});
        // 发起Lte_main模块LTE1_CHN_DS通道的链接
        if (get_lte_chn_status(g_main_lte_handle, LTE1_CHN_DS) != LTE_NET_CONNECTED_SUCCESS) 
        {
            reconnection_lte_chn(g_main_lte_handle, LTE1_CHN_DS);
        }
        // 发送心跳数据报
        HeartbeatNowTime = DS_SysTime;
        if (HeartbeatNowTime-HeartbeatLastTime > DsPlatformModuleCfg.TimeInterval) 
        {
            HeartbeatLastTime = HeartbeatNowTime;
            LOG_D("Send ds 工作状态报 心跳数据报");
            DS_SendDeviceHeartbeatData();
            mdelay(3000);
            DS_SendDeviceBatteryData();
        }
        mdelay(10);
    }
}

/*   在此添加任务(线程)   */
static task_info_t tasks_info[] = {
  {"ds_background",  TASK_STK_SIZE_2048, BUSINESS_DS_RECV_BACKGROUND_MSG_TASK_PRIO, DS_RecvDSBackgroundMsgTask, 0},
  {"ds_broadcast",   TASK_STK_SIZE_2048, BUSINESS_DS_RECV_BROADCASE_MSG_TASK_PRIO, DS_RecvBroadcastMsgTask, 0},
  {"ds_update",      TASK_STK_SIZE_4096, BUSINESS_DS_UPDATE_TASK_PRIO, DS_UpdateTask, 0},
  {"ds_run",         TASK_STK_SIZE_2048, BUSINESS_DS_RUN_TASK_PRIO, DS_RunTask, 0},
};

///@brief   创建任务（线程）
static BOOL InitCreateTask(int task_id, char *name, void (*start_routine)(void *), int stk_size, int prio)
{
  tasks_info[task_id].thread = rt_thread_create(name, start_routine, NULL, stk_size, prio, 20);
  AssertError(tasks_info[task_id].thread != RT_NULL, return FALSE, "create thread:%s failed", name);
  rt_thread_startup(tasks_info[task_id].thread);
  
  return TRUE;
}

///@brief   初始化模块通用功能
static BOOL InitModuleCommonFunc()
{
    int ret = InitBroadcastClient();
    AssertErrorNoPrint(0 == ret, return FALSE);
    
    for (int i = 0; i < sizeof(tasks_info)/sizeof(task_info_t); ++i){
        LOG_D("starting : %s as task %d" , tasks_info[i].name, i);
        AssertContinueNoPrint(tasks_info[i].threadFunc != NULL, {});
        InitCreateTask(i, tasks_info[i].name, tasks_info[i].threadFunc, tasks_info[i].stk_size, tasks_info[i].prio);
    }
    return TRUE;
}

///@brief   初始化模块入口函数
BOOL InitBussinessDSModule()
{   
    // 初始化模块通用功能
    InitModuleCommonFunc();
    
    return 0;
}

