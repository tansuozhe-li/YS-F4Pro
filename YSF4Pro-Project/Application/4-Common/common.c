/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  common.c
*  内容摘要：  多模块共用内容，全局变量等
*  其他说明：
*  当前版本：  EIT-L_V1.0.0
*  作    者：  
*  完成日期：  2023年12月18日
*  
*  修改记录 1：
*        修改日期：2023年12月18日
*        修 改 人：
*        修改内容：创建工程文件
*  修改记录 2：
*******************************************************************************/
// 包含的头文件
#include "common.h"

rt_time_t DS_SysTime = 0;
task_status_t stTaskStatus={0};
DeviceBatteryParam_t DeviceBatteryParam;
///@brief  注册状态维护ID;最多支持MAX_TASK_ID个模块的状态维护;注册后模块状态默认为TASK_WORKING
void LoginTaskId(u_int8_t task_id)
{    
    if(task_id < (MAX_TASK_ID-1)){ 
       stTaskStatus.login_total++; 
       stTaskStatus.status[task_id] = TASK_WORKING;
       stTaskStatus.login_flag[task_id] = TASK_LOGIN_FINISH;
    } 
}

///@brief 更新任务状态
void SetTaskStatus(u_int8_t task_id, task_status_e status)
{
  if((task_id <= (MAX_TASK_ID-1)) && (stTaskStatus.login_flag[task_id] == TASK_LOGIN_FINISH)) {
	  stTaskStatus.status[task_id] = status;
  }
}

rt_mutex_t RS485_Data_Mutex = RT_NULL;
///@brief   根据不同的接口对RS485进行开电操作
void RS485_Power_Control(uint8_t JX, uint8_t status)
{
    rt_mutex_take(RS485_Data_Mutex,RT_WAITING_FOREVER);
    switch (JX){
      case RS485A_INTERFACE: 
        if (status == RS485_PWRER_ON){
          RS485A_PWRC_ON;
        }else{
          RS485A_PWRC_OFF;
        }
        break;
      case RS485B_INTERFACE: 
        if (status == RS485_PWRER_ON){
          RS485B_PWRC_ON;
        }else{
          RS485B_PWRC_OFF;
        }
        break;
    default:
      break;
    }
    rt_mutex_release(RS485_Data_Mutex);
}

///@brief   初始化模块入口函数
BOOL InitCommonFuntionModule()
{   
    //创建一个动态互斥量
    RS485_Data_Mutex = rt_mutex_create("RS485_Data_Mutex", RT_IPC_FLAG_FIFO);
    AssertError(RS485_Data_Mutex != RT_NULL, return FALSE, "create RS485_Data_Mutex:%s failed", "RS485_Data_Mutex");

    return 0;
}