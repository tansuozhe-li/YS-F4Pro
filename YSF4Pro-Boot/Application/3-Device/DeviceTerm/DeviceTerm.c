/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeviceTerm.c
*  内容摘要：  终端交互模块功能
*  其他说明：
*  当前版本：  EIT-L_V1.0.0
*  作    者：  
*  完成日期：  2024年9月10日
*  
*  修改记录 1：
*        修改日期：2024年9月10日
*        修 改 人：
*        修改内容：创建工程文件
*  修改记录 2：
*******************************************************************************/
#include "common.h"
#include "DeviceTerm.h"
#include "uart_dev.h"
#include "DeviceCfg.h"
#include "DeepSensingUpdate.h"
///@brief  读取终端串口命令
int term_uart_readln(u_int8_t *buff, int buff_size,u_int32_t timeout_ms)
{
    int offset = 0;
    char c = 0;
    uart_device_info_t *dev = (uart_device_info_t *)g_console_uart_handle;

    // 预留一个'\0'位置
    buff_size--; 
    while(1) 
    {
        rt_sem_take(dev->sem_recv_data,timeout_ms);
        if (uart_dev_char_present(g_console_uart_handle))
        {
            // 从环形队列弹出一个char
            c = uart_dev_in_char(g_console_uart_handle);
            buff[offset++] = c;
            // 0x03为ctrl+c
            if ((c == '\n') || (c == '\r') || (c == 0x03))
            {
                buff[offset] = '\0';
                return offset;
            }
        }
        else 
        {
            // 在c/c++中规定字符串的结尾标志为'\0' 
            // 不是以'\n','\r'结尾的则认为接收失败
            buff[offset] = '\0';  
            return 0; 
        }	     
    }
}

///@brief  读取配置文件
int term_read_cfgfile(u_int8_t *buff, int buff_size,u_int8_t type)
{
    int offset = 0;
    char c = 0;
    int data_len = 0;
    DSOTAUpgradeFileHead_t *pack =  (DSOTAUpgradeFileHead_t*)buff; 
    if (uart_dev_char_present(g_console_uart_handle)){//如果缓冲区有数据
        mdelay(200);
        for (offset = 0; offset<buff_size; )
        {
             c = uart_dev_in_char(g_console_uart_handle);//从环形队列弹出一个char         
             buff[offset++] = c;
             if (offset == sizeof(pack->Flag)){ //检查包起始标志
                 if (pack->Flag != OTA_UPGRADE_INFORM_FLAG){//HYPK
                     memcpy(buff, buff+1, offset-1);
                     offset--;
                     buff_size--;
                 }
             }else if(offset == sizeof(DSOTAUpgradeFileHead_t)){ //包头收完
                 if (pack->File_Type != type){
                       LOG_D("文件格式不匹配");
                       return 0; 
                 }
                 data_len = pack->File_Length + sizeof(DSOTAUpgradeFileHead_t);
                 if (type == OTA_UPGRADE_FILE_TYPE_CERT) {
                    FlashID_Erase_Sector(FLASH_ID_Cert_File_Head);//擦除片外flash的存储空间
                    mdelay(100);
                    FlashID_Write_MorePage(buff, FLASH_ID_Cert_File_Head, 0, offset);
                 } else if (type == OTA_UPGRADE_FILE_TYPE_KEY) {
                    FlashID_Erase_Sector(FLASH_ID_PublicKey_File_Head);//擦除片外flash的存储空间
                    mdelay(100);
                    FlashID_Write_MorePage(buff, FLASH_ID_PublicKey_File_Head, 0, offset);
                 }
             }
             else if(offset > sizeof(DSOTAUpgradeFileHead_t)){//接收数据
                 if (offset == data_len) {//数据接收完成
                     return offset;
                 }
             }   
        }
        return 1;
    }
    return 0;
}

///@brief  下载配置文件
static int func_cmd_download_cfgfile(u_int8_t type)
{
    int ret = 0;
    int timeout_cnt = 0;
    static u_int8_t read_buff[1500] = {0};//3k
    DSOTAUpgradeFileHead_t *pack =  (DSOTAUpgradeFileHead_t*)read_buff;
    
    while(1) {
        mdelay(200);
        LOG_D(".");
        ret = term_read_cfgfile((u_int8_t*)read_buff,sizeof(read_buff),type);
        if (ret > 2) {
            LOG_HEX16("",(u_int8_t*)read_buff, ret);
            if (type == OTA_UPGRADE_FILE_TYPE_CERT) {
                FlashID_Erase_Sector(FLASH_ID_Cert_File_Param);//擦除片外flash的存储空间
                mdelay(100);
                FlashID_Write_MorePage(read_buff, FLASH_ID_Cert_File_Param, 0, ret);
            } else if (type == OTA_UPGRADE_FILE_TYPE_KEY) {
                FlashID_Erase_Sector(FLASH_ID_Publickey_File_Param);//擦除片外flash的存储空间
                mdelay(100);
                FlashID_Write_MorePage(read_buff, FLASH_ID_Publickey_File_Param, 0, ret);
            }
            break;
        }
        else if (ret == 1)
        {
            break;
        }
        else
        {
            timeout_cnt++;
            if (timeout_cnt >= 300)
            {
                LOG_D("等待超时");
                break;
            }
        }
    }
    
    return ret;
}

///@brief  终端交互任务
static void Task_Config_Term(void *args)
{
    int ret = 0;
    int timeout_cnt = 0;
    char temp = 0;
    char cmmd_buff[10] = {0};
    static u_int8_t read1_buff[1500] = {0};//3k
    LOG_D("请选择以下功能项");
    LOG_D("--> 1 下载配置文件 \r\n --> 2 下载加密证书 \r\n --> 3 下载平台公钥");
    while(1)
    {
        mdelay(200);
        ret = term_uart_readln((u_int8_t*)cmmd_buff,sizeof(cmmd_buff),RT_WAITING_FOREVER);
        if (ret > 0)
        {
            temp = cmmd_buff[0];
            if (temp == '1')
            {
                LOG_D("--> 下载配置文件");
                int Ret = func_cmd_download_cfgfile(OTA_UPGRADE_FILE_TYPE_CFG);
                if (Ret > 2){
                    LOG_D("配置文件下载成功");
                    FlashID_Read(read1_buff, OTA_UPGRADE_FILE_TYPE_CFG, 0, ret);
                    break;
                }else{
                    LOG_D("配置文件下载失败");
                    break;
                }
            } else if (temp == '2') {
                LOG_D("--> 下载加密证书");
                int Ret = func_cmd_download_cfgfile(OTA_UPGRADE_FILE_TYPE_CERT);
                
                if (Ret > 2){
                    LOG_D("加密证书下载成功");
                    FlashID_Read(read1_buff, FLASH_ID_Cert_File_Param, 0, Ret);
                    LOG_HEX16("", read1_buff, Ret);
                    break;
                }else{
                    LOG_D("加密证书下载失败");
                    break;
                }
            } else if (temp == '3') {
                LOG_D("--> 下载平台公钥");
                int Ret = func_cmd_download_cfgfile(OTA_UPGRADE_FILE_TYPE_KEY);
                
                if (Ret > 2){
                    LOG_D("下载平台公钥成功");
                    FlashID_Read(read1_buff, FLASH_ID_Publickey_File_Param, 0, Ret);
                    LOG_HEX16("", read1_buff, Ret);
                    break;
                }else{
                    LOG_D("下载平台公钥失败");
                    break;
                }
            }
        }
        else 
        {
            timeout_cnt++;
            if (timeout_cnt >= 300)
            {
                break;
            }
        }
    }
  
}

/*   在此添加任务(线程)   */
static task_info_t tasks_info[] = {
  {"term_run",  TASK_STK_SIZE_2048, DEVICE_TERM_RUN_TASK_PRIO, Task_Config_Term, 0},
};

///@brief   创建任务（线程）
static BOOL InitCreateTask(int task_id, char *name, void (*start_routine)(void *), int stk_size, int prio)
{
  tasks_info[task_id].thread = rt_thread_create(name, start_routine, NULL, stk_size, prio, 20);
  AssertError(tasks_info[task_id].thread != RT_NULL, return FALSE, "create thread:%s failed", name);
  rt_thread_startup(tasks_info[task_id].thread);
  
  return TRUE;
}

///@brief  初始化通用模块
static BOOL InitModuleCommonFunc()
{    
    for (int i = 0; i < sizeof(tasks_info)/sizeof(task_info_t); ++i){
        LOG_D("starting : %s as task %d",tasks_info[i].name, i);
        AssertContinueNoPrint(tasks_info[i].threadFunc != NULL, {});
        InitCreateTask(i, tasks_info[i].name, tasks_info[i].threadFunc, tasks_info[i].stk_size, tasks_info[i].prio);
    }
    return TRUE;
}

///@brief  初始化终端交互模块
BOOL InitDeviceTermModule()
{   
    // 初始化模块通用功能
    InitModuleCommonFunc();
    
    return 0;
}