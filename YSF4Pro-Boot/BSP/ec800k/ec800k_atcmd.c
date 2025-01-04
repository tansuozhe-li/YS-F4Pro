/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  ec800k_atcmd.c
*  内容摘要：  移远ec800k模块驱动 
*  其他说明：
*  当前版本：  
*  作    者：  
*  完成日期：  
*  修改记录 ：
*******************************************************************************/
#include "stdio.h"
#include "ec800k_atcmd.h"
#include <rtthread.h>

/* 声明通道设备句柄 */
lte_device_handle g_main_lte_handle;
lte_device_handle g_sub_lte_handle;

lte_device_info_t stlte_device_table[2]={
  {0,0},
  {0,0}
};

lte_table_t g_stlte_table[MAX_LTE_MODEM_NUM]={0};

#define AT_LR               "\r"
#define AT_RESP_OK          "OK" 

///@brief 初始化lte模块接口函数
lte_device_handle lte_dev_init(u_int8_t index, u_int8_t uart_index, u_int32_t baud,
                                         u_int8_t *cmd_in_buff,      int cmd_in_buff_size,
                                         u_int8_t *ch0_data_in_buff, int ch0_data_in_buff_size,
                                         u_int8_t *ch1_data_in_buff, int ch1_data_in_buff_size)                                        
{
       if(stlte_device_table[index].init)
         return (lte_device_handle)(&stlte_device_table[index]);
      
       stlte_device_table[index].lte_uart_handle = uart_dev_init(uart_index,baud);
       InitRingQueue(&stlte_device_table[index].cmd_in_buff, cmd_in_buff, cmd_in_buff_size); 

       if((ch0_data_in_buff != NULL) && (ch0_data_in_buff_size >0)){
         InitRingQueue(&stlte_device_table[index].stlte_chn_table.stchn_table[0].data_in_buff, ch0_data_in_buff, ch0_data_in_buff_size); 
         stlte_device_table[index].stlte_chn_table.stchn_table[0].connected_stats = LTE_NET_CONNECTED_IDEL;
         stlte_device_table[index].stlte_chn_table.stchn_table[0].sem_dist_data = rt_sem_create("sem_dist_data",  0u, RT_IPC_FLAG_FIFO);
        }
       
        if((ch1_data_in_buff != NULL) && (ch1_data_in_buff_size >0)){
         InitRingQueue(&stlte_device_table[index].stlte_chn_table.stchn_table[1].data_in_buff, ch1_data_in_buff, ch1_data_in_buff_size);  
         stlte_device_table[index].stlte_chn_table.stchn_table[1].connected_stats = LTE_NET_CONNECTED_IDEL;
         stlte_device_table[index].stlte_chn_table.stchn_table[1].sem_dist_data = rt_sem_create("sem_dist_data",  0u, RT_IPC_FLAG_FIFO);
        }

        stlte_device_table[index].sem_dist_cmd           =  rt_sem_create( "sem_dist_cmd", 0u, RT_IPC_FLAG_FIFO);
        //lte_devices[index].sem_dist_data          =  rt_sem_create("sem_dist_data",  0u, RT_IPC_FLAG_FIFO);
        stlte_device_table[index].sem_mutex_send_data    = rt_sem_create("sem_mutex_send_data", 1u, RT_IPC_FLAG_FIFO);
        stlte_device_table[index].sem_mutex_read_at_lock = rt_sem_create("sem_mutex_read_at_lock", 1u, RT_IPC_FLAG_FIFO);
      
        stlte_device_table[index].index = index;
        stlte_device_table[index].init  = 1;
       
        return (lte_device_handle)(&stlte_device_table[index]);
}


///@brief nec800k 处理读取到的一行数据
/// 区别是命令回复 还是接收到的数据
#define MAX_CMD_LEN   300    
u_int8_t echo[MAX_CMD_LEN+1]={0};
int ec800k_process_readln(lte_device_handle lte_device, void const *data, int data_len)
{  
     int at_cmd_len = data_len;
     u_int8_t *pdata = (u_int8_t*)data;

     memset(echo,'0',sizeof(echo));
     
     lte_device_info_t *lte_dev = (lte_device_info_t *)lte_device;
      
     /* 1.判断长度，最大接收300字节数据 */
      AssertError(data_len < MAX_CMD_LEN, return 0, "Lte%d Recv data len  over buff len %d, len: %d", lte_dev->index+1, MAX_CMD_LEN,data_len);
     
      memset(echo,0,sizeof(echo));
      memcpy(echo, data,data_len); 
      rt_kprintf("%s",echo);
        
     /* 2.收到信号强度 */
     if(strstr((const char*)pdata,"+CSQ:")){
       if(pdata[8] != ',')
          return 0;
        char sg=StrToInt((char*)&pdata[6]);
        if (sg <= 31){
            g_stlte_table[lte_dev->index].signal_intensity = (unsigned char)(sg*3.2);
        }else{
            g_stlte_table[lte_dev->index].signal_intensity = 0; 
        } 

     /* 3.收到ccid号 */
     }else if(strstr((const char*)pdata,"+QCCID:")){
        memcpy(g_stlte_table[lte_dev->index].ccid,(char*)&pdata[8],sizeof(g_stlte_table[lte_dev->index].ccid)); 
        //LOG_D("LTE%d CCID: %s\r\n",lte_dev->index+1,g_stlte_table[lte_dev->index].ccid);
        
     /* 4.接收到副通道断开连接命令 */
     }else if(strstr((const char*)pdata,"+QIURC: \"closed\",1")){
         lte_dev->stlte_chn_table.stchn_table[1].connected_stats = LTE_NET_CONNECTED_FAILED;
     /* 5.接收命令 */   
     }else{
        while(at_cmd_len--){
           if(!RingQueueFull(&lte_dev->cmd_in_buff))
              InRingQueue(&lte_dev->cmd_in_buff, *(pdata++));
        } 
     }      
     rt_thread_mdelay(10);
     rt_sem_release(lte_dev->sem_dist_cmd); 
     return 0;
}          

///@brief 读取一行判断是命令还是数据
/// 查找+QIURC: "recv",0,4 代表接收到远程服务器端数据

int ec800k_uart_readln(lte_device_handle lte_device, u_int8_t *buff, int buff_size,u_int32_t timeout_ms)
{
  int curr_pos = 0;
  int recvfrom_chn = 0; //数据来的通道connectID
  int recvfrom_datalen=0;
  char ch=0;
  char temp[6] = {0x00};
  
  lte_device_info_t *lte_dev = (lte_device_info_t *)lte_device;
  uart_device_info_t  *uart_dev = (uart_device_info_t *)lte_dev->lte_uart_handle; 
  
  while(1){
    rt_sem_take(uart_dev->sem_recv_data, timeout_ms);
    
    if(uart_dev_char_present(lte_dev->lte_uart_handle)){
        ch = uart_dev_in_char(lte_dev->lte_uart_handle);//读取一个数据

        if(curr_pos < (buff_size-1)){
            buff[curr_pos++] = ch;
         }else{
            return -1;
        }
       
       /* 直吐模式下，可以通过 AT+QISEND 发送数据。从网络接收数据时，数据会以如下格式直接输出
          到 COM 口：+QIURC:"recv",<connectID>,<currectrecvlength><CR><LF><data>或+QIURC:
          "recv",<connectID>,<currentrecvlength>,<remoteIP>,<remote_port><CR><LF><data>*/
       if(strstr((const char*)buff,"+QIURC: \"recv\",")){
          rt_thread_mdelay(50);
          buff[curr_pos++] = uart_dev_in_char(lte_dev->lte_uart_handle);
          // <connectID>
          recvfrom_chn = buff[curr_pos-1] -'0';                
          buff[curr_pos] = uart_dev_in_char(lte_dev->lte_uart_handle);

          AssertError(recvfrom_chn < MAX_CHN_NUM, return -1, "Lte%d Recv ch error :%d", lte_dev->index+1, recvfrom_chn);
          AssertError((buff[curr_pos] == ','), return -1,);  
          
          for(char cnt  = 0; cnt < 5; cnt++){
             temp[cnt] = uart_dev_in_char(lte_dev->lte_uart_handle);
             if(temp[cnt] != '\n') continue;
             else break;
          } 
          // <currectrecvlength>
          recvfrom_datalen = StrToInt(&temp[0]);
          AssertError(recvfrom_datalen < 1460, return -1, "Lte%d Recv datalen error :%d", lte_dev->index+1, recvfrom_datalen);
          LOG_I("LTE%d ch%d recv %d bytes data",lte_dev->index+1, recvfrom_chn, recvfrom_datalen);
          // 获取副通道CORS平台数据
          if (recvfrom_chn == 1){
             lte_dev->stlte_chn_table.stchn_table[recvfrom_chn].recv_data_len = recvfrom_datalen;
          }
          // <data>
          while(recvfrom_datalen--){
            rt_sem_take(uart_dev->sem_recv_data, timeout_ms);
            if(uart_dev_char_present(lte_dev->lte_uart_handle)){
               ch = uart_dev_in_char(lte_dev->lte_uart_handle);
               if(!RingQueueFull(&lte_dev->stlte_chn_table.stchn_table[recvfrom_chn].data_in_buff)){
                 InRingQueue(&lte_dev->stlte_chn_table.stchn_table[recvfrom_chn].data_in_buff, ch);                 
               }
            }  
         }
         rt_sem_release(lte_dev->stlte_chn_table.stchn_table[recvfrom_chn].sem_dist_data);  
         return -1;
       }
      
       if((ch == '\n')||(ch == '>')){//'r' 'n' 不计入计数里
          buff[curr_pos] = '\0';
          return curr_pos;
       }
    }
   }
} 

///@brief 比较字符串
static BOOL compare(u_int8_t *inbuff, u_int8_t *compare_msg)
{
    if(strstr((char*)inbuff,(char*)compare_msg)!=NULL)
       return TRUE;
    return FALSE;
}

///@brief 读取一行数据
static int lte_cmd_readln(lte_device_handle lte_device, u_int8_t *buff, int buff_size)
{

   lte_device_info_t *lte_dev = (lte_device_info_t *)lte_device;

   int offset;
   char c;
   buff_size--; 
   for(offset = 0; offset < buff_size; offset++){
       if(!RingQueueEmpty(&lte_dev->cmd_in_buff)){
           OutRingQueue(&lte_dev->cmd_in_buff,(u_int8_t*)&c);
           if ((c == '\n')||(c == '>')){
               buff[offset] = '\0';
               return offset;
          }
          buff[offset] = c;  
        }
   }
   return 0;  
}

///@brief 清空命令buff
static void clean_cmd_buff(lte_device_handle lte_dev)
{   
    u_int8_t read_buff[15]={0x00};
    for(int i = 0; i < 20; i++){
      int ret = lte_cmd_readln(lte_dev, (u_int8_t*)read_buff,sizeof(read_buff));
      if(ret <= 0)
         break;
     }       
}

///@brief 发送at命令，不处理回复指令
static BOOL ec800k_send_atcmd_noresp(lte_device_handle lte_dev, u_int8_t* cmd)
{   
    lte_device_info_t *device = (lte_device_info_t *)lte_dev;

    rt_sem_take(device->sem_mutex_read_at_lock,RT_WAITING_FOREVER);  

    clean_cmd_buff(lte_dev);
    uart_sendstr(device->lte_uart_handle, (char*)cmd);
    uart_sendstr(device->lte_uart_handle,AT_LR);
    rt_sem_release(device->sem_mutex_read_at_lock);
    return TRUE;      
}

///@brief 发送at命令，处理回复指令
static BOOL ec800k_send_atcmd_resp(lte_device_handle lte_dev, u_int8_t* cmd, u_int8_t* resp, int timeout_s)
{   
    u_int8_t read_buff[20]={0x00};
    
    lte_device_info_t *device = (lte_device_info_t *)lte_dev;

    rt_sem_take(device->sem_mutex_read_at_lock,RT_WAITING_FOREVER);  
    
    clean_cmd_buff(lte_dev);
    uart_sendstr(device->lte_uart_handle,(char*)cmd);
    uart_sendstr(device->lte_uart_handle,AT_LR);
    
    if(timeout_s == 0)
      timeout_s=5;
    int i = 10*timeout_s;
    while(i--){
       if(!rt_sem_take(device->sem_dist_cmd, 100u))
          continue;
       int ret = lte_cmd_readln(lte_dev, (u_int8_t*)read_buff,sizeof(read_buff));
       if(ret == 0)
          continue;
         
       if(!compare(read_buff, resp))
          continue;
       rt_sem_release(device->sem_mutex_read_at_lock);
       return TRUE;
     }
     rt_sem_release(device->sem_mutex_read_at_lock);
     return FALSE;      
}

///@brief 测试回显功能
BOOL ec800k_test_at_echo(lte_device_handle lte_dev)
{
    return ec800k_send_atcmd_resp(lte_dev, "AT", "AT", 0); 
}

///@brief 查询模块初始化状态
BOOL ec800k_check_init_status(lte_device_handle lte_dev)
{
    return ec800k_send_atcmd_resp(lte_dev, "", "RDY", 20); 
}

///@brief 打开模块故障显示功能
BOOL ec800k_open_echo_fault_code(lte_device_handle lte_dev)
{
    return ec800k_send_atcmd_resp(lte_dev, "AT+CMEE=2", "OK", 0); 
}

///@brief 打开飞行模式
BOOL ec800k_Turn_on_airplane_mode(lte_device_handle lte_dev)
{
    return ec800k_send_atcmd_resp(lte_dev, "AT+CFUN=0", "OK", 0); 
}

///@brief 关闭飞行模式
BOOL ec800k_Turn_off_airplane_mode(lte_device_handle lte_dev)
{
    return ec800k_send_atcmd_resp(lte_dev, "AT+CFUN=1", "OK", 0); 
}

///@brief 设置保存波特率
BOOL ec800k_set_baud(lte_device_handle lte_dev)
{
    return ec800k_send_atcmd_resp(lte_dev, "AT+IPR=115200;&W", "OK", 0); 
}

///@brief 关闭回显功能
BOOL ec800k_close_at_echo(lte_device_handle lte_dev)
{   
    return ec800k_send_atcmd_resp(lte_dev, "ATE0", "OK", 0); 
}

///@brief 打开休眠功能
/// DTR高是进入休眠，拉低退出休眠
BOOL ec800k_sleep_on(lte_device_handle lte_dev)
{   
    return ec800k_send_atcmd_resp(lte_dev, "AT+QSCLK=1", "OK", 0); 
}

///@brief 查询模块信号强度
BOOL ec800k_query_csq(lte_device_handle lte_dev)
{    
     return ec800k_send_atcmd_resp(lte_dev, "AT+CSQ","OK", 0);
}

///@brief 检查电话卡状态
BOOL ec800k_check_sim_card(lte_device_handle lte_dev)
{     
      return ec800k_send_atcmd_resp(lte_dev, "AT+CPIN?", "+CPIN: READY", 0);
}

///@brief 检查电话卡ccid
/// res: +QCCID:89860025128306012474
BOOL ec800k_query_sim_ccid(lte_device_handle lte_dev)
{     
      return ec800k_send_atcmd_resp(lte_dev, "AT+QCCID","OK", 0);   
}

///@brief 设置模块apn接入点及
///AT+QICSGP=<contextID>[,<context_type>,<APN>[,<username>,<password>)[,<authentication>]]]
BOOL ec800k_set_access_point(lte_device_handle lte_dev) 
{   
     uint8_t cmd[50]={0x00};
     if (lte_dev == g_main_lte_handle) {
        sprintf((char*)cmd, "AT+QICSGP=1,1,\"%s\","","",0", DeviceLteModuleCfg.LteMain_APN);
     }else{
        sprintf((char*)cmd, "AT+QICSGP=1,1,\"%s\","","",0", DeviceLteModuleCfg.LteSub_APN);
     }
     log_printf("%s", cmd);
     return ec800k_send_atcmd_resp(lte_dev, cmd, "OK", 0);   
}

///@brief 查询网络激活状态
BOOL ec800k_query_network_status(lte_device_handle lte_dev) 
{   
  return ec800k_send_atcmd_resp(lte_dev, "AT+CEREG?", "+CEREG:", 60); 
}

///@brief 激活场景
/// 激活场景 1，受网络状态影响，最大响应时间为 150 秒
BOOL ec800k_activate_the_scene(lte_device_handle lte_dev) 
{   
    return ec800k_send_atcmd_resp(lte_dev, "AT+QIACT=1","OK", 30); 
}

///@brief 获取本地ip地址
BOOL ec800k_get_local_ip(lte_device_handle lte_dev)  
{ 
    return ec800k_send_atcmd_resp(lte_dev, "AT+QIACT?", "OK", 3); 
}

///@brief 关闭tcp连接
BOOL ec800k_tcpclose(lte_device_handle lte_dev,u_int8_t chnn)
{   
     u_int8_t cmd[20]={0x00};
     sprintf((char*)cmd,"AT+QICLOSE=%d",chnn);
     return ec800k_send_atcmd_resp(lte_dev, cmd, "OK", 3); 
}

///@brief  关机
BOOL ec800k_power_down(lte_device_handle lte_dev)
{
     return ec800k_send_atcmd_resp(lte_dev, "AT+QPOWD", "OK", 3); 
}

///@brief 发送指定长度数据
//AT+QISEND=<connectID>,<send_length>
BOOL ec800k_send_data(lte_device_handle lte_dev,u_int8_t chnn,u_int8_t *pdata,int data_len)
{     
    u_int8_t cmd[20]={0x00};
    //u_int8_t ctrl_z[1]={0x1A};
    
    lte_device_info_t *device = (lte_device_info_t *)lte_dev; 
    
    AssertError((device->stlte_chn_table.stchn_table[chnn].connected_stats == LTE_NET_CONNECTED_SUCCESS), return FALSE, "");
    AssertError((chnn < (MAX_CHN_NUM)), return FALSE, "Chnn error %d", chnn);
    AssertError(data_len < 1460, return FALSE, "Data len error %d", data_len);
     
    /* 发送成功响应 */
    sprintf((char*)cmd,"AT+QISEND=%d,%d",chnn,data_len); 

    ec800k_send_atcmd_noresp(lte_dev, cmd);
//    if(!ec800k_send_atcmd_resp(lte_dev,cmd, ">", 10)){
//       return FALSE;
//    }
    mdelay(100);
    
    rt_sem_take(device->sem_mutex_read_at_lock,RT_WAITING_FOREVER);
    clean_cmd_buff(lte_dev);
    uart_dev_write(device->lte_uart_handle, pdata, data_len);
    //uart_dev_write(device->lte_uart_handle, ctrl_z, sizeof(ctrl_z));
    rt_sem_release(device->sem_mutex_read_at_lock);
    
    if(!ec800k_send_atcmd_resp(lte_dev,"", "SEND OK", 10)){
       return FALSE;
    }
    return TRUE;      
}

 ///@brief 创建客户端网络通道   
BOOL ec800k_create_ip_channel(lte_device_handle lte_dev,u_int8_t chnn)
{   
     u_int8_t resp[20]={0x00};
     static u_int8_t cmd[55]={0x00};

     lte_device_info_t *device = (lte_device_info_t *)lte_dev;

     AssertError((chnn < (MAX_CHN_NUM)), return FALSE, "Chnn error %d", chnn);
       //检测网络状态
     AssertError((device->stlte_chn_table.dial_status == LTE_NET_CONNECTED_SUCCESS), return FALSE, "");
    
     sprintf((char*)resp,"+QIOPEN: %d,0",chnn);
     memset(cmd,0,sizeof(cmd));
     
 
     /* 执行AT+QIOPEN之前，Host需使用AT+QIACT激活场景 */    
     sprintf((char*)cmd,"AT+QIOPEN=%d,%d,\"%s\",\"%s\",%d,%d,%d",1, chnn,device->stlte_chn_table.stchn_table[chnn].net_type,\
                                                          device->stlte_chn_table.stchn_table[chnn].serv_ip,\
                                                          device->stlte_chn_table.stchn_table[chnn].port,0,1);
     if(!ec800k_send_atcmd_resp(lte_dev, cmd, resp, 10)){
          /* 若 150 秒内未响应，Host 可通过 AT+QICLOSE 断开 Socket */
          ec800k_send_atcmd_resp(lte_dev, "AT+QICLOSE", "OK", 0);
          return FALSE;
     }
     return TRUE;
     
}

