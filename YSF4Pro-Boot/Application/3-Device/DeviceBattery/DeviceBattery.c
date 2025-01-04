/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeviceBattery_main.c
*  内容摘要：  电池模块，采集电池电压，电流，充电电压，电流，电池温度
*  其他说明：
*  当前版本：  EIT-L_V1.0.014
*  作    者：  
*  开始日期：  2024年06月18日
*  
*  修改记录 1：
*        修改日期：2024年06月18日
*        修 改 人：黎庆昌
*        修改内容：创建工程文件，调试电池数据
*******************************************************************************/
#include "DeviceBattery.h"
#include "adc.h"
#include "main.h"

rt_mutex_t Adc_Data_Mutex = RT_NULL;

///@brief 发送电池状态数据
static void Send_MSG_BatteryData(uint8_t *buff, uint16_t len)
{
    if (!send_mq_broadcast_msg("battery_data", (uint32_t)buff, len, buff))
    {
        LOG_D("battery_data send failed!");
    }
}

///@brief   广播获取维护模块数据
BOOL BroadcastMsgProc_GetBatteryData(mq_brodcast_msg *pMsg)
{
    return TRUE;
}

///@brief 获取开路电压的值
float Battery_Get_VSO_DET(uint32_t ch)
{   
    VSO_DET_CTR_ENABNEL;
    mdelay(1000);
    float volt_val = 0.0;
    uint16_t temp[12] = {0};
    uint16_t adc_value = 0;
    
    for (int i = 0; i < 12; i++)
    {
        temp[i] = Get_Adc(ch);
        mdelay(5);
    }
    adc_value = DataRankCalculateAverage(temp, 12);
    volt_val = (float)adc_value * 3.3f * 9.8f / 4096;
    VSO_DET_CTR_DISABNEL;
    mdelay(1000);
    return volt_val;
}

///@brief 获取充电电流的值
float Battery_Get_CHRG_DET(uint32_t ch)
{
    float chra_det = 0.0;
    uint16_t temp[12] = {0};
    uint16_t adc_value = 0;
    for (int i = 0; i < 12; i++)
    {
        temp[i] = Get_Adc(ch);
        mdelay(5);
    }
    adc_value = DataRankCalculateAverage(temp, 12);
    chra_det = (float)adc_value * 3.3f * 0.25f / 4096;

    return chra_det;
}
///@brief  获取电池电压
float Battery_Get_BAT_VDET(uint32_t ch)
{
    float volt_val = 0.0;
    static uint16_t vdet_temp[12] = {0};
    uint16_t adc_value = 0;
    
    for (int i = 0; i < 12; i++)
    {
        vdet_temp[i] = Get_Adc(ch);
        mdelay(5);
    }
    adc_value = DataRankCalculateAverage(vdet_temp, 12);
    volt_val = (float)adc_value * 3.3f * 6.09f / 4096 + 0.3f;
    return volt_val;
}

///@brief  获取电池充电状态
uint8_t Battery_Get_Floating_Charge()
{
    uint8_t floating_charge = HAL_GPIO_ReadPin(CHRG_INS_GPIO_Port, CHRG_INS_Pin);
    
    return floating_charge;
}

///@brief  采集电池数据
void Collect_Battery_Data()
{
    int delay_cnt = 0;
    uint8_t battery_data_buff[50] = {0};
    rt_time_t NowTime, LastTime=0;
    uint8_t sendcount = 0;
    while (1)
    {
        // 每分钟采集一次数据
        if (delay_cnt > 60) 
        {
            delay_cnt = 0;
            SetTaskStatus(DEVICE_BATTERY_RUN_TASK_PRIO, TASK_WORKING);
            
            DeviceBatteryParam_t *bat = (DeviceBatteryParam_t *)battery_data_buff;
            bat->Open_voltage = Battery_Get_VSO_DET(BATTERY_ADC_CHANNEL_VSO_VDET);
            bat->Charge_Current = Battery_Get_CHRG_DET(BATTERY_ADC_CHANNEL_CHRG_DET);
            bat->Battery_Voltage = Battery_Get_BAT_VDET(BATTERY_ADC_CHANNEL_BAT_VDET);
            bat->FloatingCharge = Battery_Get_Floating_Charge();
               
            SetTaskStatus(DEVICE_BATTERY_RUN_TASK_PRIO, TASK_IDLE);
        }
        delay_cnt++;
        if ((sendcount < 3) && ((NowTime - LastTime) > 60))
        {
            SetTaskStatus(DEVICE_BATTERY_RUN_TASK_PRIO, TASK_WORKING);
            sendcount++;
            if (LastTime != 0)
            {
                Send_MSG_BatteryData(battery_data_buff, sizeof(DeviceBatteryParam_t));
            }
            LastTime = NowTime;
            SetTaskStatus(DEVICE_BATTERY_RUN_TASK_PRIO, TASK_IDLE);
            
        }
        else if ((sendcount >= 3) && ((NowTime - LastTime) > 600))
        {
            SetTaskStatus(DEVICE_BATTERY_RUN_TASK_PRIO, TASK_WORKING);
            sendcount++;
            Send_MSG_BatteryData(battery_data_buff, sizeof(DeviceBatteryParam_t));
            LastTime = NowTime;
            SetTaskStatus(DEVICE_BATTERY_RUN_TASK_PRIO, TASK_IDLE);
        }
        mdelay(1000);
    } 
}

///@brief 初始化动态互斥锁
BOOL InitDataRTMutex()
{
    //创建一个动态互斥量
    Adc_Data_Mutex = rt_mutex_create("Adc_Data_Mutex", RT_IPC_FLAG_FIFO);
    AssertError(Adc_Data_Mutex != RT_NULL, return FALSE, "create Adc_Data_Mutex:%s failed", "Adc_Data_Mutex");

    return 0; 
}

