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
#include "NanWangGX.h"
#include "ec800k_atcmd.h"

///@brief   从环形队列中取出一帧数据
///@return  成功则返回数据长度，失败返回0
int GetNanWangFrame(uint8_t *buff, int buff_size, RingQueue *ring_queue)
{
    AssertReturnNoPrint(!RingQueueEmpty(ring_queue), return 0);
    uint32_t offset = 0;
    char c = 0;
    buff_size--; //预留一个'\0'位置 
    GW2015FrameHeader_t *pstFrame = (GW2015FrameHeader_t *)buff;
    rt_thread_delay(200);
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
uint16_t lte2_uart_read_pack(u_int8_t *buff, int buff_size, lte_device_handle lte_device, u_int8_t chnn)
{
    uint16_t pack_len = 0;
    lte_device_info_t *device = (lte_device_info_t *)lte_device;
    rt_sem_take(device->stlte_chn_table.stchn_table[chnn].sem_dist_data, 500);
    pack_len = GetNanWangFrame(buff, buff_size, &device->stlte_chn_table.stchn_table[chnn].data_in_buff);
    return pack_len;
}

///@brief 通过广播包发送消息
BOOL Send_Broadcast_Msg(char *msg_type, uint8_t *buff, uint16_t len)
{
    if (!send_mq_broadcast_msg((uint8_t *)msg_type, (uint32_t)buff, len, buff))
    {
        LOG_D("Broadcast_data send failed!");
        return FALSE;
    }
    return TRUE;
}

///@brief 广播包接收覆冰数据,通过4G发送
BOOL NW_BroadcastMsgProc_IceTensionData(mq_brodcast_msg *pMsg)
{
    NW_SendGW2015DataPack(GW2015_FRAME_TYPE_01H_MONITOR_DATA_REPORT, GW2015_PACK_TYPE_07H_ICING_DATA, pMsg->msg_body,pMsg->msg_len);
    return TRUE;
}
///@brief 工作状态响应报--心跳数据报
BOOL NW_MsgProc_WorkHeartbeatDataResp(GW2015FrameHeader_t *pFrame)
{
    return TRUE;
}

///@brief 设备发送心跳包
void NW_SendDeviceHeartbeatData()
{    
    static uint8_t heartbeat_buff[100] = {0}; 
    memset(heartbeat_buff, 0, sizeof(heartbeat_buff));
    GW2015HeartbeatDataPack_t *pack = (GW2015HeartbeatDataPack_t *)heartbeat_buff;
    
    pack->Time_Stamp = 0;
    pack->Battery_Voltage = 0;
    pack->Operation_Temperature = 0;
    pack->Battery_Capacity = 0;
    pack->FloatingCharge = 0;
    pack->Total_Working_Time = 0;
    pack->Working_Time = 0;
    pack->Connection_State = 0x01;
    pack->Send_Flow = 0;
    pack->Receive_Flow = 0;
    pack->Protocol_Version = I1GW2015PROTOCOL_VERSION;
    
    NW_SendGW2015DataPack(GW2015_FRAME_TYPE_09H_WORK_STATUS_REPORT, GW2015_PACK_TYPE_C1H_HEARTBEAT_DATA, heartbeat_buff, sizeof(GW2015HeartbeatDataPack_t));
}

///@brief 发送DS数据报,GW2015协议，添加报文头，报文尾，并调用数据发送；
void NW_SendGW2015DataPack(uint8_t frame_type, uint8_t pack_type, uint8_t *data, uint16_t data_len)
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
    LOG_HEX("buff", pack_buff, pack_len);
    
    lte_uart_send_pack(g_sub_lte_handle, LTE1_CHN_DS, pack_buff, pack_len);
}

