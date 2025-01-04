/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeepSensing.c
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
#include "DeepSensing.h"
#include "common.h"

extern uint8_t Recv_Heartbeat_flag;
uint8_t ds_msgproc_frame_no = 0;
uint8_t GetFrameNo()
{
    if (ds_msgproc_frame_no >= 255) 
        ds_msgproc_frame_no = 0;
    return ++ds_msgproc_frame_no;
}

///@brief 通过广播包发送消息
static BOOL Send_Broadcast_Msg(char *msg_type, uint8_t *buff, uint16_t len)
{
    if (!send_mq_broadcast_msg((uint8_t *)msg_type, (uint32_t)buff, len, buff))
    {
        LOG_D("weather_data send failed!");
        return FALSE;
    }
    return TRUE;
}

///@brief   从环形队列中取出一帧数据
///@return  成功则返回数据长度，失败返回0
int GetFrame(uint8_t *buff, int buff_size, RingQueue *ring_queue)
{
    AssertReturnNoPrint(!RingQueueEmpty(ring_queue), return 0);
    uint32_t offset = 0;
    char c = 0;
    buff_size--; //预留一个'\0'位置 
    GW2015FrameHeader_t *pstFrame = (GW2015FrameHeader_t *)buff;
    rt_thread_delay(300);
    for (offset = 0; offset < buff_size;){ 
        int ret = OutRingQueue(ring_queue, (uint8_t*)&c);
        AssertBreakNoPrint(RQ_OK == ret, {}); /* 不在此函数阻塞 */
        buff[offset++] = c;
        if (offset == sizeof(pstFrame->Sync)){//检查起始包标志
            if (pstFrame->Sync != GW2015_RECV_FRAME_BEGIN_FLAG){
                LOG_E("Sync error: %x", pstFrame->Sync);
                memcpy(buff, buff+1, offset-1);
                offset--;
                buff_size--;//防止进入死循环
            }    
        }else if (offset == GW2015_PACK_HEAD_SIZE(pstFrame)){ //包头收完
            if((GW2015_PACK_SIZE(pstFrame) > buff_size))
            {
                LOG_E("packet size more than %d" , buff_size);
                offset = 0;
                break;
            }
        }else if (offset == GW2015_PACK_SIZE(pstFrame)){ //包接收完成
             return offset;
         }
    }   
    return 0;
}

///@brief 通过LTE获取设备串口数据
uint16_t lte_uart_read_pack(u_int8_t *buff, int buff_size, lte_device_handle lte_device, u_int8_t chnn)
{
    uint16_t pack_len = 0;
    lte_device_info_t *device = (lte_device_info_t *)lte_device;
    uart_device_info_t  *uart_dev = (uart_device_info_t *)device->lte_uart_handle;
    rt_sem_take(device->stlte_chn_table.stchn_table[chnn].sem_dist_data, 500);
    pack_len = GetFrame(buff, buff_size, &device->stlte_chn_table.stchn_table[chnn].data_in_buff);
    return pack_len;
}

DeviceBatteryParam_t *p_Battery = NULL;
///@brief 广播包接收电池数据
BOOL DS_BroadcastMsgProc_BatteryData(mq_brodcast_msg *pMsg)
{
    return TRUE;
}

///@brief 工作状态响应报--心跳数据报
BOOL DS_MsgProc_WorkHeartbeatDataResp(GW2015FrameHeader_t *pFrame)
{
    uint8_t buff[6] = {0};
    memcpy(buff, pFrame->data, pFrame->Packet_Length);
    GW2015HeartbeatResp_t *pResp = (GW2015HeartbeatResp_t *)buff;
    //当数据发送状态失败时，重新发送
    if (pResp->Command_Status != GW2015DataStateOK){
        DS_SendDeviceHeartbeatData();
    }
    //上位机当前时间不为零时，向系统时钟模块发送广播消息进行校时
    if (pResp->Clocktime_Stamp != 0){ 
        AssertError(Send_Broadcast_Msg("timing_systime", buff, pFrame->Packet_Length), return FALSE, "timing_systime send failed");
    }
    Recv_Heartbeat_flag = 1;
    return TRUE;
}

///@brief 控制数据响应报--状态监测装置重启
BOOL DS_MsgProc_ControlMonitorDevReboot(GW2015FrameHeader_t *pFrame)
{
    uint8_t Command_Status = 0xFF;
    DS_SendGW2015DataPack(GW2015_FRAME_TYPE_04H_CONTROL_DATA_RESP, GW2015_PACK_TYPE_A6H_DEV_RESET, &Command_Status, 1);
    delay(5);
    NVIC_SystemReset();
}

///@brief 自定义数据响应报--设备状态数据报
BOOL DS_Msgproc_DeviceStateDataResp(GW2015FrameHeader_t *pFrame)
{
    if (pFrame->data[0] != GW2015DataStateOK){
        LOG_E("StateData Response Failed");
    }
    return TRUE;
}


///@brief 填充TLV格式数据
void DS_TLVDataFillPack(uint8_t type, void *Sdata, uint16_t data_len, uint8_t *pack, uint16_t *pack_len)
{
    TLVData_t *pData = (TLVData_t *)pack;
    pData->Type = type;
    memcpy(pData->data, Sdata, data_len);
    pData->lenth = data_len + 3;
    *pack_len = pData->lenth;
}

static DeviceBatteryParam_t DeviceBatteryData;
///@brief 广播包接收电池数据
void DS_SendDeviceBatteryData()
{
    memset((uint8_t*)&DeviceBatteryData, 0 , sizeof(DeviceBatteryParam_t));
    //memcpy((uint8_t*)&DeviceBatteryData.FloatingCharge, pMsg->msg_body, pMsg->msg_len);
    static uint8_t status_buff[100] = {0}; 
    uint16_t buff_len = 0;
    uint16_t data_len = 0;
    memset(status_buff, 0, sizeof(status_buff));
    
    DS_TLVDataFillPack(0x00, &DeviceBatteryData.Battery_Voltage, sizeof(DeviceBatteryData.Battery_Voltage), &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x01, &DeviceBatteryData.Battery_Current, sizeof(DeviceBatteryData.Battery_Current), &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x02, &DeviceBatteryData.Battery_Capacity, sizeof(DeviceBatteryData.Battery_Capacity), &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x03, &DeviceBatteryData.Battery_Temp, sizeof(DeviceBatteryData.Battery_Temp), &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x04, &DeviceBatteryData.Charge_Type, sizeof(DeviceBatteryData.Charge_Type), &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x05, &DeviceBatteryData.Open_voltage, sizeof(DeviceBatteryData.Open_voltage), &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x06, &DeviceBatteryData.Charge_Current, sizeof(DeviceBatteryData.Charge_Current), &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x07, &DeviceBatteryData.FloatingCharge, sizeof(DeviceBatteryData.FloatingCharge), &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x08, NULL, 4, &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x09, NULL, 4, &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x0a, NULL, 4, &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x0b, NULL, 4, &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x0c, NULL, 1, &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x0d, NULL, 4, &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x0e, NULL, 4, &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    DS_TLVDataFillPack(0x0f, NULL, 1, &status_buff[buff_len], &data_len);
    buff_len = buff_len + data_len;
    
    DS_SendGW2015DataPack(GW2015_FRAME_TYPE_09H_WORK_STATUS_REPORT, DS_PACK_TYPE_D5H_DEV_WORK_STATUS_DATA, status_buff, buff_len);
}

///@brief 设备发送心跳包
void DS_SendDeviceHeartbeatData()
{    
    static uint8_t heartbeat_buff[100] = {0}; 
    memset(heartbeat_buff, 0, sizeof(heartbeat_buff));
    GW2015HeartbeatDataPack_t *pack = (GW2015HeartbeatDataPack_t *)heartbeat_buff;
    
    pack->Time_Stamp = work_time.Clocktime_Stamp;
    pack->Battery_Voltage = p_Battery->Battery_Voltage;
    pack->Operation_Temperature =p_Battery->Battery_Temp;
    pack->Battery_Capacity = p_Battery->Battery_Capacity;
    pack->FloatingCharge = p_Battery->FloatingCharge;
    pack->Total_Working_Time = work_time.Clocktime_Stamp/3600;
    pack->Working_Time = work_time.Working_Time / 3600;
    pack->Connection_State = 0x01;
    pack->Send_Flow = 0;
    pack->Receive_Flow = 0;
    pack->Protocol_Version = I1GW2015PROTOCOL_VERSION;
    
    DS_SendGW2015DataPack(GW2015_FRAME_TYPE_09H_WORK_STATUS_REPORT, GW2015_PACK_TYPE_C1H_HEARTBEAT_DATA, heartbeat_buff, sizeof(GW2015HeartbeatDataPack_t));
}

///@brief 发送DS数据报,GW2015协议，添加报文头，报文尾，并调用数据发送；
void DS_SendGW2015DataPack(uint8_t frame_type, uint8_t pack_type, uint8_t *data, uint16_t data_len)
{
    uint16_t CRC16 = 0;
    static uint8_t pack_buff[256] = {0};
    memset(pack_buff, 0, sizeof(pack_buff));
    GW2015FrameHeader_t *pack = (GW2015FrameHeader_t *)pack_buff;
    
    pack->Sync = GW2015_FRAME_BEGIN_FLAG;
    pack->Packet_Length = data_len;
    memcpy(pack->CMD_ID, DsPlatformModuleCfg.CmdId, 17);
    pack->Frame_Type = frame_type;
    pack->Packet_Type = pack_type;
    pack->Frame_No = 0;
    memcpy(pack->data, data, data_len);
    CRC16 = RTU_CRC((uint8_t *)&pack->Packet_Length, data_len + GW2015_PACK_HEAD_SIZE(pack) - sizeof(uint16_t));
    pack->data[data_len] = (uint8_t)(CRC16 & 0x00FF);
    pack->data[data_len + 1] = (uint8_t)(CRC16 >> 8);
    pack->data[data_len + 2] = GW2015_FRAME_END_FLAG;
    uint16_t pack_len = GW2015_PACK_SIZE(pack);
    LOG_HEX("SendGW2015DataPack", pack_buff, pack_len);
    
    lte_uart_send_pack(g_main_lte_handle,LTE1_CHN_DS,pack_buff, pack_len);
}



