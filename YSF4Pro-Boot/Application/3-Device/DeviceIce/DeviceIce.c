/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeviceClimate.c
*  内容摘要：  覆冰模块主要功能实现
*  其他说明：
*  当前版本：  EIT-L_V1.0.0
*  作    者：  黎庆昌
*  创建日期：  2023年11月21日
*******************************************************************************/
#include "DeviceIce.h"
#include "uart_dev.h"
#include <math.h>

static DeviceIceParam_t DeviceIceParam;

///@brief  modbus crc16算法
static unsigned short Modbus_CRC16(unsigned char *arr_buff, unsigned char len)
{
    unsigned short crc=0xFFFF;
    unsigned char i, j;
    for ( j=0; j<len; j++){   
        crc=crc ^*arr_buff++;
        for ( i=0; i<8; i++){   
            if( ( crc&0x0001) >0){   
                crc=crc>>1;
                crc=crc^ 0xa001;
            }else{
                crc=crc>>1;
            }
        }   
    }   
    return crc;
}

///@brief  获取覆冰数据帧
static int GetIceFrame(uint8_t ID, uint8_t *buff, int buff_size)
{
    AssertReturnNoPrint(uart_dev_char_present(g_rs485_uart_handle), return 0);
    DeviceIceData_t *pFrame = (DeviceIceData_t *)buff;
    uint32_t offset = 0;
    char c = 0;
    buff_size--; //预留一个'\0'位置 
    mdelay(300);
    for (offset = 0; offset < buff_size;){ 
        c = uart_dev_in_char(g_rs485_uart_handle);//从环形队列弹出一个char
        buff[offset++] = c;
        if (offset == sizeof(pFrame->Sensor_ID)){//检查起始包标志
            if (pFrame->Sensor_ID != ID){
                LOG_E("Ice_ID error: %x", pFrame->Sensor_ID);
                memcpy(buff, buff+1, offset-1);
                offset--;
                buff_size--;//防止进入死循环
            }    
        }else if (offset == ICE_PACK_HEAD_SIZE(pFrame)){ //包头收完
            if((pFrame->Data_Len > buff_size))
            {
                LOG_E("packet size more than %d" , buff_size);
                offset = 0;
                break;
            }
        }else if (offset == ICE_PACK_SIZE(pFrame)){ //包接收完成
             return offset;
        }
    }   
    return 0; 
}

///@brief  发送覆冰数据
static void Send_MSG_IceData(uint8_t *buff, uint16_t len)
{
    if (!send_mq_broadcast_msg("ice_tension", (uint32_t)buff, len, buff))
    {
        LOG_D("ice_tension send failed!");
    }
}

///@brief  覆冰指令  
///  (寄存器地址 00 代表拉压力）
/// （寄存器地址 02 代表 X 轴倾角）
/// （寄存器地址 04 代表 Y 轴倾角）
/// （寄存器地址 06 代表 数字板环境温度）
static void TianGuangIceTensionGeneralCmd(uint8_t ID, uint8_t cmd_type, uint8_t *cmd_buff, uint8_t *cmd_len)
{
    DeviceIceCmd_t *pCmd = (DeviceIceCmd_t *)cmd_buff;
    pCmd->Sensor_ID = 0x01;
    pCmd->Cmd_Type = 0x03;
    pCmd->Register_Start_addr = 0x0000;
    pCmd->Register_No = 0x0200;
    pCmd->CRC16 = Modbus_CRC16(cmd_buff, sizeof(DeviceIceCmd_t)-2); 
    *cmd_len = sizeof(DeviceIceCmd_t);
    LOG_HEX("ice_cmd", cmd_buff, *cmd_len);
}

///@brief  发送覆冰采集指令，接收返回的覆冰数据
int SendCmdAndCollectIceData(uint8_t ice_ID, uint8_t *data_buff, uint16_t buff_size)
{  
    rt_mutex_take(RS485_Data_Mutex,RT_WAITING_FOREVER);
    uint8_t ice_cmd[8] = {0};
    uint8_t ice_cmd_len = 0;
    uart_device_info_t *uart_dev = (uart_device_info_t *)g_rs485_uart_handle;
    int recv_len = 0;
    
    //发送指令之前判断循环buff是否有数据，有数据先清除
    for (int i = 0; i < 100; i++) {
        clean_data_buff(g_rs485_uart_handle);
        mdelay(30);
    }

    if (!rt_strcmp(DeviceIceModuleCfg.sensor_type, "TianGuang"))
    {
        TianGuangIceTensionGeneralCmd(ice_ID, 0, ice_cmd, &ice_cmd_len);
    } 
    else 
    {
        LOG_E("ice type is error!");
    }

    for(int i = 0; i < 3; i++){
        //发送覆冰采集指令
        RS485_DE_HIGH;
        uart_dev_write(g_rs485_uart_handle, ice_cmd, ice_cmd_len);
        RS485_DE_LOW;
        rt_sem_take(uart_dev->sem_recv_data, 300);
        for (int j = 0; j < 100; j++) {
            mdelay(30);
            recv_len = GetIceFrame(ice_ID, data_buff, buff_size);
            AssertContinueNoPrint(recv_len != 0, {});
            if (recv_len > 0) {
                break;
            }
        }
        AssertBreakNoPrint(recv_len == 0, {});
    }
    if (recv_len == 0){
        LOG_D("Ice_data Recv Timeout!");
        rt_mutex_release(RS485_Data_Mutex);
        return 0;
    }
    rt_mutex_release(RS485_Data_Mutex);
    return recv_len;
}

///@brief 处理覆冰数据
void ProcessIceData(uint8_t ice_ID, uint8_t *data_buff, int data_len)
{
    uint8_t ice_data[125] = {0};
    memset(ice_data, 0, sizeof(ice_data));
    
    
    if (!rt_strcmp(DeviceIceModuleCfg.sensor_type, "TianGuang")) 
    {
        memcpy(ice_data, data_buff, data_len);
        // 将4字节的float型数据进行高低位调整
        for (int i = 0; i < 4; i++) {
            ice_data[3+i] = data_buff[6-i];
            ice_data[3+4+i] = data_buff[6+4-i];
            ice_data[3+8+i] = data_buff[6+8-i];
            ice_data[3+12+i] = data_buff[6+12-i];
        }
        TianGuangIceData_t *frame = (TianGuangIceData_t *)ice_data;
        //将串口接收的覆冰原始数据进行解析
        for (int j = 0; j < 2; j++) {
            if (DeviceIceParam.ID[j] == ice_ID) {
                memcpy((uint8_t *)&DeviceIceParam.IceData[j].Sensor_ID, ice_data, data_len);
            }
        }  
        LOG_D("ID: %d, Tension:%.2f, Incli_X: %.2f, Incli_Y: %.2f",ice_ID,frame->Tension, frame->Incli_X, frame->Incli_Y);
    } 
    else 
    {
        LOG_D("不支持的覆冰类型");
    }
}

///@brief  计算等值覆冰厚度，计算参考 “专利-覆冰算法（四川电科院）”
///  等值覆冰厚度Di计算公式：
///  Di = sqrt((F% x cosa x b)/(Pi x p x g) + Rc x Rc) - Rc
///  F% ： 输电线路悬垂绝缘子串覆冰前后拉力增长百分比
///  a  :  悬垂绝缘子串风偏角
///  b  :  导线线密度
///  Pi :  圆周率，3.1415926
///  p  :  冰密度, 0.9g/cm3
///  g  :  重力加速度 
///  Rc :  导线线径  
#define Pi 3.14159226
#define p  0.9    /// g/cm3
#define g  9.8   /// 
#define Rc 21.66 /// cm
#define b  8.259  /// g/cm
static float Last_Tension;
static float Calculate_Equal_IceThickness(float Tension, float a, float temp)
{
    float Equal_IceThickness = 0.0f; //等值覆冰厚度
    float Tension_Increased_Percentage = 0.0f; //拉力增长百分比
    
    // 判断环境温度是否小于零，如大于零则覆冰厚度为0
    if (temp >= 0) {
        return Equal_IceThickness;
    }
    // 计算拉力的增长百分比
    Tension_Increased_Percentage = (Tension - Last_Tension) / Last_Tension;
    
    // 计算等值覆冰厚度
    Equal_IceThickness = sqrt((Tension_Increased_Percentage * cos(a) * b)/(Pi * p * g) + Rc * Rc) - Rc;
    
    return Equal_IceThickness;
}

///@brief   发送覆冰数据
static void SendProcessIceData()
{
    static uint8_t buff_pack[256] = {0};
    memset(buff_pack, 0, sizeof(buff_pack));
    GW2015IceTensionDataPack_t *pack = (GW2015IceTensionDataPack_t *)buff_pack;
    
    //将串口接收的覆冰原始数据进行解析
    memcpy(pack->Componet_ID, DeviceIceModuleCfg.CompoentCmdId, sizeof(DeviceIceModuleCfg.CompoentCmdId));
    pack->Time_Stamp = work_time.Clocktime_Stamp;
    pack->Equal_IceThickness = Calculate_Equal_IceThickness(DeviceIceParam.IceData[0].Tension, DeviceIceParam.IceData[0].Incli_Y, DeviceIceParam.IceData[0].Temperature);
    pack->Tension = DeviceIceParam.IceData[0].Tension;
    pack->Tension_Difference = 0 /*(DeviceIceParam.IceData[0].Tension - Last_Tension) / Last_Tension*/;
    pack->T_Sensor_Num = 2;
    pack->Original_Tension1 = DeviceIceParam.IceData[0].Tension;
    pack->Windage_Yaw_Angle1 = DeviceIceParam.IceData[0].Incli_Y;
    pack->Deflection_Angle1 = DeviceIceParam.IceData[0].Incli_X;
    pack->Original_Tension2 = DeviceIceParam.IceData[1].Tension;
    pack->Windage_Yaw_Angle2 = DeviceIceParam.IceData[1].Incli_Y;
    pack->Deflection_Angle2 = DeviceIceParam.IceData[1].Incli_X;
    
    //发送MSG覆冰数据
    Send_MSG_IceData(buff_pack, sizeof(GW2015IceTensionDataPack_t));
}

///@brief   采集覆冰数据
static BOOL AutoCollectIceData(uint8_t Ice_ID)
{
    static uint8_t read_buff[100] = {0};

    memset(read_buff, 0, sizeof(read_buff));
    //通过485串口发送覆冰指令，接收原始数据
    int ret = SendCmdAndCollectIceData(Ice_ID, read_buff, sizeof(read_buff));
    if (ret == 0){
        //使用上次的数据
        return FALSE;
    }
    LOG_HEX("read_buff", read_buff, ret);
    //ProcessIceData(Ice_ID, read_buff, ret);
    return TRUE;
}

///@brief   轮询485接口，确定覆冰接口位置
static BOOL InquireIceInterface()
{
   for (int j = 0; j < 2; j++) {
       if (DeviceIceParam.Flag[j] != 1) {
           DeviceIceParam.ID[j] = DeviceIceModuleCfg.ID[j];
           for (int i = 1; i < 3; i++) {
               RS485_Power_Control(i, RS485_PWRER_ON);
               BOOL ret = AutoCollectIceData(DeviceIceParam.ID[j]);
               if (ret == TRUE) 
               { 
                   DeviceIceParam.Interface[j] = i;
                   DeviceIceParam.Flag[j] = 1;
                   RS485_Power_Control(i, RS485_PWRER_OFF);
                   break;
               }
               delay(3);
           }
       }
   }
   return TRUE;
}

///@brief   广播获取覆冰数据
BOOL DS_BroadcastMsgProc_GetIceData()
{
    static uint8_t buff_pack[256] = {0};
    memset(buff_pack, 0, sizeof(buff_pack));
    GW2015IceTensionDataPack_t *pack = (GW2015IceTensionDataPack_t *)buff_pack;
    
    //将串口接收的覆冰原始数据进行解析
    memcpy(pack->Componet_ID, DeviceIceModuleCfg.CompoentCmdId, sizeof(DeviceIceModuleCfg.CompoentCmdId));
    pack->Time_Stamp = work_time.Clocktime_Stamp;
    pack->Equal_IceThickness = 0;
    pack->Tension = 0;
    pack->Tension_Difference = 0;
    pack->T_Sensor_Num = 2;
    pack->Original_Tension1 = DeviceIceParam.IceData[0].Tension;
    pack->Windage_Yaw_Angle1 = DeviceIceParam.IceData[0].Incli_X;
    pack->Deflection_Angle1 = DeviceIceParam.IceData[0].Incli_Y;
    pack->Original_Tension2 = DeviceIceParam.IceData[1].Tension;
    pack->Windage_Yaw_Angle2 = DeviceIceParam.IceData[1].Incli_X;
    pack->Deflection_Angle2 = DeviceIceParam.IceData[1].Incli_Y;
    //发送MSG覆冰数据
    Send_MSG_IceData(buff_pack, sizeof(GW2015IceTensionDataPack_t));
    return TRUE;
}

///@brief  覆冰主任务，轮询接口，采集覆冰数据
void DS_DeviceIce_Run()
{
    int RecvDataFlag = 0;
    int InquireInterfaceCnt = 0;
    int CollectDataCnt = 0;
    rt_time_t NowTime, LastTime=0;
    
    while(1)
    {
        mdelay(10);
        NowTime = DS_SysTime;
        if (CollectDataCnt < 3 && (NowTime - LastTime) > 60) {
            CollectDataCnt++;
            for (int j = 0; j < 2; j++) {
                RS485_Power_Control(j+1, RS485_PWRER_ON);
                LOG_D("Collect Ice Data");
                LastTime = NowTime;
                AutoCollectIceData(DeviceIceParam.ID[j]);
                //RS485_Power_Control(j+1, RS485_PWRER_OFF);
            }
            SendProcessIceData();
        } else if ((CollectDataCnt >= 3) && ((NowTime - LastTime) > DeviceIceModuleCfg.TimeInterval)) {
            for (int j = 0; j < 2; j++) {
                RS485_Power_Control(j+1, RS485_PWRER_ON);
                LOG_D("Collect Ice Data");
                LastTime = NowTime;
                AutoCollectIceData(DeviceIceParam.ID[j]);
                //RS485_Power_Control(j+1, RS485_PWRER_OFF);
            }
            SendProcessIceData();
        }
    }  
}

                              

