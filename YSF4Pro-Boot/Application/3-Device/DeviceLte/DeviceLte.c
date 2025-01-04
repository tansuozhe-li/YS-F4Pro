#include "DeviceLte.h"
#include "ec800k.h"

///@brief 读取Lte主通道串口数据数据
static u_int8_t main_readBuff[800];
void DS_MainLteReadLnTask()
{
  while(1){
      rt_thread_delay(5); 
      memset(main_readBuff,0,sizeof(main_readBuff));
      int ret = ec800k_uart_readln(g_main_lte_handle, main_readBuff, sizeof(main_readBuff), RT_WAITING_FOREVER);
      if(ret > 0){
         ec800k_process_readln(g_main_lte_handle, main_readBuff, ret);     
      }
  }
}
   
///@brief 读取Lte副通道串口数据数据
static u_int8_t sub_readBuff[800];
void DS_SubLteReadLnTask()
{
  while(1){
       rt_thread_delay(5); 
      memset(sub_readBuff,0,sizeof(sub_readBuff));
      int ret = ec800k_uart_readln(g_sub_lte_handle, sub_readBuff, sizeof(sub_readBuff), RT_WAITING_FOREVER);
      if(ret > 0){
         ec800k_process_readln(g_sub_lte_handle, sub_readBuff, ret);     
      }
  }
}

