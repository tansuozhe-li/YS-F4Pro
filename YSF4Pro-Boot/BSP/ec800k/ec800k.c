 /************************************************************************
** File name:  ec800k.c
* Description: 包含ec800k cat1模块驱动
* Others： 
* Version： 
* Author： 
* Author: ID: 
* Modification:
***********************************************************************/
#include "ec800k.h"
#include "common.h"

#define LTE_MAIN_UART_IN_BUFF_SIZE        2048
#define LTE_UART_IN_BUFF_SIZE             1500
#define LTE_DATA_IN_BUFF_SIZE             1500
#define LTE_DATA_IN_BUFF_LITTLE_SIZE      500
#define LTE_CMD_IN_BUFF_SIZE              300

/* LTE模块驱动数据流向
USART IRQHandler  |   dist_lte_main_chn_msg |   
uart_in_buff     ->   lte_uart_readln       -> chx_data_in_buff
                                            -> ec800k_process_readln->cmd_in_buff
*/
//static u_int8_t main_uart_in_buff[LTE_MAIN_UART_IN_BUFF_SIZE];
static u_int8_t main_cmd_in_buff[LTE_CMD_IN_BUFF_SIZE];
static u_int8_t main_ch0_data_in_buff[LTE_DATA_IN_BUFF_SIZE];
static u_int8_t main_ch1_data_in_buff[LTE_DATA_IN_BUFF_SIZE];

//static u_int8_t sub_uart_in_buff[LTE_UART_IN_BUFF_SIZE];
static u_int8_t sub_cmd_in_buff[LTE_CMD_IN_BUFF_SIZE];
static u_int8_t sub_ch0_data_in_buff[LTE_DATA_IN_BUFF_SIZE];

/* 主通道和副通道链路状态 */
//u_int8_t g_lte_main_chn0_conected_status = NET_CONNECTED_IDEL;
//u_int8_t g_lte_main_chn1_conected_status = NET_CONNECTED_IDEL;
//u_int8_t g_lte_sub_chn0_dial_status = NET_CONNECTED_IDEL;


///@brief 获取lte模块具体通道的连接状态
u_int8_t get_lte_chn_status(lte_device_handle lte_device,u_int8_t chn)
{
   lte_device_info_t *device = (lte_device_info_t *)lte_device;  
   return device->stlte_chn_table.stchn_table[chn].connected_stats;
}

///@brief 手动更新lte模块具体通道的连接状态
void update_lte_chn_status(lte_device_handle lte_device,u_int8_t chn, u_int8_t status)
{
   lte_device_info_t *device = (lte_device_info_t *)lte_device; 
   device->stlte_chn_table.stchn_table[chn].connected_stats = status;
}

///@brief 发起Lte模块x通道的链接
BOOL reconnection_lte_chn(lte_device_handle lte_device,u_int8_t ch)
{
    lte_device_info_t *device = (lte_device_info_t *)lte_device; 

    if(device->stlte_chn_table.dial_status != LTE_NET_CONNECTED_SUCCESS)
       return FALSE;

    if(!ec800k_create_ip_channel(lte_device,ch)){
        LOG_E("LTE%d ch%d disconnect",device->index+1, ch);
        LOG_E("SrvIp:%s Port:%d NetType:%s", device->stlte_chn_table.stchn_table[ch].serv_ip, device->stlte_chn_table.stchn_table[ch].port, device->stlte_chn_table.stchn_table[ch].net_type);
        device->stlte_chn_table.stchn_table[ch].connected_stats = LTE_NET_CONNECTED_FAILED;    
        return FALSE;
     }else{
        LOG_I("LTE%d ch%d connected",device->index+1, ch);
        LOG_I("SrvIp:%s Port:%d NetType:%s",device->stlte_chn_table.stchn_table[ch].serv_ip, device->stlte_chn_table.stchn_table[ch].port, device->stlte_chn_table.stchn_table[ch].net_type);
        device->stlte_chn_table.stchn_table[ch].connected_stats = LTE_NET_CONNECTED_SUCCESS;    
        return TRUE;
     }
}

///@brief 开机
static BOOL ec800k_power_on(lte_device_handle lte_device)
{
   lte_device_info_t *device = (lte_device_info_t *)lte_device; 
   if(device->index == LTE_DEVICE_MAIN_INDEX){
       LTE1_PWRKEY_ON();
       LTE1_ENABLE();
       delay(10);
   }else if(device->index == LTE_DEVICE_SUB_INDEX){
       //LTE2_PWRKEY_ON();
       //LTE2_ENABLE();
       delay(10);
   }
   return TRUE;
}

///@brief ec800k8_dail
/// 通讯通道拨号，通过发送at指令，判断模块是否开机正常
int ec800k_dail(lte_device_handle lte_device)
{  
  lte_device_info_t *device = (lte_device_info_t *)lte_device;
  
   for(char i =0; i< MAX_CHN_NUM; i++){
      device->stlte_chn_table.stchn_table[i].connected_stats = LTE_NET_CONNECTED_FAILED;
   }
   device->stlte_chn_table.dial_status=LTE_NET_CONNECTED_FAILED;
   
   LOG_D("LTE%d begin to dail",device->index+1);
   if (!ec800k_power_on(lte_device)){
        LOG_E("LTE%d power on",device->index+1);
        return 0;
   }
   
    //关闭回显指令
    if(!ec800k_close_at_echo(lte_device)){
       LOG_E("LTE%d close at echo",device->index+1);
       return FALSE;
    }
     
    //打开飞行模式 
   if(!ec800k_Turn_on_airplane_mode(lte_device)){
      LOG_E("LTE%d turn on airplane mode",device->index+1);
      return FALSE;
   } 
   
    //设置apn接入点，用户名密码
    if(!ec800k_set_access_point(lte_device)){
        LOG_E("LTE%d set access point",device->index+1);
       return FALSE;
    }  
    
    //关闭飞行模式   
   if(!ec800k_Turn_off_airplane_mode(lte_device)){
      LOG_E("LTE%d turn off airplane mode",device->index+1);
      return FALSE;
   } 
      
	 //查询SIM卡是否被识别
   if(!ec800k_check_sim_card(lte_device)){
       LOG_E("LTE%d check sim card",device->index+1);
       return FALSE;
   }
   
   //查询sim卡ccid
   if(!ec800k_query_sim_ccid(lte_device)){
      LOG_E("LTE%d query sim ccid",device->index+1);
      return FALSE;
   }

   //查询信号强度
   if(!ec800k_query_csq(lte_device)){
       LOG_E("LTE%d query csq",device->index+1);
       return FALSE;
    }
      
    //激活网络  
    if(!ec800k_query_network_status(lte_device)){
         LOG_E("LTE%d query network status",device->index+1);
         return FALSE;
     }
 
    //激活场景,打开上网功能  
    if(!ec800k_activate_the_scene(lte_device)){
      LOG_E("LTE%d activate the scene",device->index+1);
      return FALSE;
    }
 
    //获取本地ip
    if(!ec800k_get_local_ip(lte_device)){
        LOG_E("LTE%d get local ip",device->index+1);
        return FALSE;
    }
     
    if(!ec800k_sleep_on(lte_device)){
        LOG_E("LTE%d set sleep mode",device->index+1);
     }
    
     device->stlte_chn_table.dial_status = LTE_NET_CONNECTED_SUCCESS;
     return TRUE;
}

///@brief 通过LTE发送设备串口数据
void lte_uart_send_pack(lte_device_handle lte_device, u_int8_t chnn, void *data, int len)
{
    lte_device_info_t *device = (lte_device_info_t *)lte_device;
    rt_sem_take(device->sem_mutex_send_data,RT_WAITING_FOREVER); 
    if (lte_device == g_main_lte_handle){ // 发送数据前唤醒模块
        LTE1_NOSLEEP();
    }else{
        //LTE2_NOSLEEP();
    }
    // 通过ec800k模块发送数据
    ec800k_send_data(lte_device, chnn, data, len);
    
    if (lte_device == g_main_lte_handle){ // 发送数据后模块休眠
        LTE1_SLEEP();
    }else{
        //LTE2_SLEEP();
    }
    rt_sem_release(device->sem_mutex_send_data);
}

///@brief lte模块拨号
BOOL lte_chn_dial(lte_device_handle lte_dev)
{
    for (u_int8_t loop = 0; loop < 3; loop++){  
        if (ec800k_dail(lte_dev)){
            return TRUE;           
        }else  
            continue;  
    }
    
    return FALSE;  
}

///@brief 初始化LTE主通道模块
void lte_device_main_init() 
{   
   g_main_lte_handle = lte_dev_init(LTE_DEVICE_MAIN_INDEX, MCU_LTE1_UART_INDEX, MCU_LTE1_UART_BAUDRATE,
                                    main_cmd_in_buff,  sizeof(main_cmd_in_buff),
                                    main_ch0_data_in_buff,sizeof(main_ch0_data_in_buff),
                                    main_ch1_data_in_buff,sizeof(main_ch1_data_in_buff)); 
}

///@brief 初始化LTE副通道模块
void lte_device_sub_init() 
{  
   g_sub_lte_handle = lte_dev_init(LTE_DEVICE_SUB_INDEX, MCU_LTE2_UART_INDEX, MCU_LTE2_UART_BAUDRATE,
                                    sub_cmd_in_buff,  sizeof(sub_cmd_in_buff),
                                    sub_ch0_data_in_buff,sizeof(sub_ch0_data_in_buff),
                                    NULL,0); 
}
