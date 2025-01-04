/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称： DeepSensing_main.c
*  内容摘要：  DeepSensing模块初始化
*  其他说明：
*  当前版本：  V1.0.000
*  作    者：  
*  开始日期：  2024年06月18日
*  
*  修改记录 1：
*        修改日期：2024年06月18日
*        修 改 人：黎庆昌
*        修改内容：创建工程文件
*******************************************************************************/
#include "common.h"
#include "thread_cfg.h"
#include "NanWangGX.h"
#include "DeviceCfg.h"

/* 广播消息接收客户端 */
static struct rt_broadcast_client   g_Client;
/* 广播消息接收缓存 */
static uint8_t                      g_MsgPool[600];
/* 广播消息 */
static mq_brodcast_msg              g_Msg;
/* 来自清蓉深瞳后台的信息 */
uint8_t                             g_NWFrameBuff[1500];
#define CLIENT_NAME                 "business_nanwang"

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
  { "ice_tension",   "覆冰拉力数据",   NW_BroadcastMsgProc_IceTensionData},
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

///@brief   监听其他线程发来的广播消息
static void NW_RecvDSBackgroundMsgTask(void *args)
{
    SetTaskStatus(BUSINESS_DS_RECV_BROADCASE_MSG_TASK_PRIO, TASK_IDLE);
    while(1) {
        memset(&g_Msg, 0, sizeof(mq_brodcast_msg));
        int ret = rt_broadcast_recv(&g_Client, &g_Msg, sizeof(mq_brodcast_msg), RT_WAITING_FOREVER);
        AssertContinueNoPrint(RT_EOK == ret, rt_thread_delay(5));
        SetTaskStatus(BUSINESS_DS_RECV_BROADCASE_MSG_TASK_PRIO, TASK_WORKING);
        /* TODO为了节约线程资源,不再单独起一个线程来处理消息 */
        BroadcastMsgHandler(&g_Msg);
        SetTaskStatus(BUSINESS_DS_RECV_BROADCASE_MSG_TASK_PRIO, TASK_IDLE);
    }
}

/*   与后台通讯协议处理   */
static NWMsgProcFrameFuncTable_t g_NWMsgProcTbl[] =
{
  { "工作状态响应报", "心跳数据报",  GW2015_PACK_TYPE_C1H_HEARTBEAT_DATA, NW_MsgProc_WorkHeartbeatDataResp},
};

///@brief   与后台通讯协议处理
static void NW_MsgProcHandler(GW2015FrameHeader_t *pstFrame)
{
    for (uint32_t i = 0; i < sizeof(g_NWMsgProcTbl) / sizeof(NWMsgProcFrameFuncTable_t); i++)
    {
        AssertContinueNoPrint((pstFrame->Frame_Type == GW2015_FRAME_TYPE_02H_MONITOR_DATA_RESP) ||  /* 数据响应报 */
                              (pstFrame->Frame_Type == GW2015_FRAME_TYPE_03H_CONTROL_DATA_REPORT) ||  /* 控制数据报 */
                              (pstFrame->Frame_Type == GW2015_FRAME_TYPE_0AH_WORK_STATUS_RESP) || /* 工作状态响应报 */
                              (pstFrame->Frame_Type == DS_FRAME_TYPE_20H_CMD_DATA_REPORT), {});  /* 自定义数据指令报 */
        AssertContinueNoPrint(pstFrame->Packet_Type == g_NWMsgProcTbl[i].msgType, {});
        AssertContinueNoPrint(g_NWMsgProcTbl[i].pMsgProc != NULL, {});
        LOG_D("process ds [%02X]%s %s start" , g_NWMsgProcTbl[i].msgType, g_NWMsgProcTbl[i].frameName, g_NWMsgProcTbl[i].packName);
        BOOL bRet = g_NWMsgProcTbl[i].pMsgProc(pstFrame);
        LOG_D("process ds [%02X]%s %s end" , g_NWMsgProcTbl[i].msgType, g_NWMsgProcTbl[i].frameName, g_NWMsgProcTbl[i].packName);
    }
}

///@brief   监听清蓉深瞳服务器发来的消息
void NW_RecvBroadcastMsgTask(void *args)
{   
    while (1)
    {
        mdelay(10);
        uint16_t recv_pack_len = 0;
        memset(g_NWFrameBuff, 0, sizeof(g_NWFrameBuff));
        GW2015FrameHeader_t *p_frame = (GW2015FrameHeader_t *)g_NWFrameBuff;
        // 获取数据
        recv_pack_len = lte2_uart_read_pack(g_NWFrameBuff, sizeof(g_NWFrameBuff), g_main_lte_handle, LTE1_CHN_DS);
        AssertContinueNoPrint(recv_pack_len != 0, {});
        SetTaskStatus(BUSINESS_DS_RECV_BACKGROUND_MSG_TASK_PRIO, TASK_WORKING);
        LOG_HEX("g_NWFrameBuff", g_NWFrameBuff, recv_pack_len);
        AssertContinue(UnShortToHighLevelTransfer(RTU_CRC((uint8_t *)&p_frame->Packet_Length, recv_pack_len-5)) == GW2015_PACK_CRC16(p_frame), {}, 
                       "CRC16 Expect: %04x", UnShortToHighLevelTransfer(RTU_CRC((uint8_t *)&p_frame->Packet_Length, recv_pack_len-5)));
        AssertContinue(g_NWFrameBuff[recv_pack_len-1] == GW2015_FRAME_END_FLAG, {}, 
                       "Frame End Flag expect: %02x", GW2015_FRAME_END_FLAG);
        // 消息处理
        NW_MsgProcHandler(p_frame);
        SetTaskStatus(BUSINESS_DS_RECV_BACKGROUND_MSG_TASK_PRIO, TASK_IDLE);
    }
}

///@brief   任务运行
static void NW_RunTask(void *args)
{
    rt_time_t HeartbeatNowTime, HeartbeatLastTime=0;
    lte_device_info_t *device = (lte_device_info_t *)g_sub_lte_handle; 
    // 等待系统启动
    while (1)
    {   
        mdelay(10);
        // 等待通道链路建立成功
        AssertContinueNoPrint(device->stlte_chn_table.dial_status == LTE_NET_CONNECTED_SUCCESS, {});
        // 发起Lte_main模块LTE1_CHN_DS通道的链接
        if (get_lte_chn_status(g_sub_lte_handle, LTE1_CHN_DS) != LTE_NET_CONNECTED_SUCCESS) 
        {
            reconnection_lte_chn(g_sub_lte_handle, LTE1_CHN_DS);
        }
        // 发送心跳数据报
        HeartbeatNowTime = DS_SysTime;
        if (HeartbeatNowTime-HeartbeatLastTime > DsPlatformModuleCfg.TimeInterval) 
        {
            SetTaskStatus(BUSINESS_DS_RUN_TASK_PRIO, TASK_WORKING);
            HeartbeatLastTime = HeartbeatNowTime;
            LOG_D("Send ds 工作状态报 心跳数据报");
            NW_SendDeviceHeartbeatData();
            SetTaskStatus(BUSINESS_DS_RUN_TASK_PRIO, TASK_IDLE);
        }
        mdelay(10);
    }
}

/*   在此添加任务(线程)   */
static task_info_t tasks_info[] = {
  {"nw_background",  TASK_STK_SIZE_2048, BUSINESS_NW_RECV_BACKGROUND_MSG_TASK_PRIO, NW_RecvDSBackgroundMsgTask, 0},
  {"nw_broadcast",   TASK_STK_SIZE_1024, BUSINESS_NW_RECV_BROADCASE_MSG_TASK_PRIO, NW_RecvBroadcastMsgTask, 0},
  {"nw_run",         TASK_STK_SIZE_1024, BUSINESS_NW_RUN_TASK_PRIO, NW_RunTask, 0},
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
    // 初始化广播包客户端
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
BOOL InitNanWangGxModule()
{   
    // 初始化模块通用功能
    InitModuleCommonFunc();
    
    return 0;
}

