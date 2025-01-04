/*******************************************************************************
*  ��Ȩ���� ��c�� 2023�� ������ͫ�Ƽ����޹�˾���廪�Ĵ���Դ�о�Ժ��
*  �ļ����ƣ�  DeviceTerm.c
*  ����ժҪ��  �ն˽���ģ�鹦��
*  ����˵����
*  ��ǰ�汾��  EIT-L_V1.0.0
*  ��    �ߣ�  
*  ������ڣ�  2024��9��10��
*  
*  �޸ļ�¼ 1��
*        �޸����ڣ�2024��9��10��
*        �� �� �ˣ�
*        �޸����ݣ����������ļ�
*  �޸ļ�¼ 2��
*******************************************************************************/
#include "common.h"
#include "DeviceTerm.h"
#include "uart_dev.h"
#include "DeviceCfg.h"
#include "DeepSensingUpdate.h"
///@brief  ��ȡ�ն˴�������
int term_uart_readln(u_int8_t *buff, int buff_size,u_int32_t timeout_ms)
{
    int offset = 0;
    char c = 0;
    uart_device_info_t *dev = (uart_device_info_t *)g_console_uart_handle;

    // Ԥ��һ��'\0'λ��
    buff_size--; 
    while(1) 
    {
        rt_sem_take(dev->sem_recv_data,timeout_ms);
        if (uart_dev_char_present(g_console_uart_handle))
        {
            // �ӻ��ζ��е���һ��char
            c = uart_dev_in_char(g_console_uart_handle);
            buff[offset++] = c;
            // 0x03Ϊctrl+c
            if ((c == '\n') || (c == '\r') || (c == 0x03))
            {
                buff[offset] = '\0';
                return offset;
            }
        }
        else 
        {
            // ��c/c++�й涨�ַ����Ľ�β��־Ϊ'\0' 
            // ������'\n','\r'��β������Ϊ����ʧ��
            buff[offset] = '\0';  
            return 0; 
        }	     
    }
}

///@brief  ��ȡ�����ļ�
int term_read_cfgfile(u_int8_t *buff, int buff_size,u_int8_t type)
{
    int offset = 0;
    char c = 0;
    int data_len = 0;
    DSOTAUpgradeFileHead_t *pack =  (DSOTAUpgradeFileHead_t*)buff; 
    if (uart_dev_char_present(g_console_uart_handle)){//���������������
        mdelay(200);
        for (offset = 0; offset<buff_size; )
        {
             c = uart_dev_in_char(g_console_uart_handle);//�ӻ��ζ��е���һ��char         
             buff[offset++] = c;
             if (offset == sizeof(pack->Flag)){ //������ʼ��־
                 if (pack->Flag != OTA_UPGRADE_INFORM_FLAG){//HYPK
                     memcpy(buff, buff+1, offset-1);
                     offset--;
                     buff_size--;
                 }
             }else if(offset == sizeof(DSOTAUpgradeFileHead_t)){ //��ͷ����
                 if (pack->File_Type != type){
                       LOG_D("�ļ���ʽ��ƥ��");
                       return 0; 
                 }
                 data_len = pack->File_Length + sizeof(DSOTAUpgradeFileHead_t);
                 if (type == OTA_UPGRADE_FILE_TYPE_CERT) {
                    FlashID_Erase_Sector(FLASH_ID_Cert_File_Head);//����Ƭ��flash�Ĵ洢�ռ�
                    mdelay(100);
                    FlashID_Write_MorePage(buff, FLASH_ID_Cert_File_Head, 0, offset);
                 } else if (type == OTA_UPGRADE_FILE_TYPE_KEY) {
                    FlashID_Erase_Sector(FLASH_ID_PublicKey_File_Head);//����Ƭ��flash�Ĵ洢�ռ�
                    mdelay(100);
                    FlashID_Write_MorePage(buff, FLASH_ID_PublicKey_File_Head, 0, offset);
                 }
             }
             else if(offset > sizeof(DSOTAUpgradeFileHead_t)){//��������
                 if (offset == data_len) {//���ݽ������
                     return offset;
                 }
             }   
        }
        return 1;
    }
    return 0;
}

///@brief  ���������ļ�
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
                FlashID_Erase_Sector(FLASH_ID_Cert_File_Param);//����Ƭ��flash�Ĵ洢�ռ�
                mdelay(100);
                FlashID_Write_MorePage(read_buff, FLASH_ID_Cert_File_Param, 0, ret);
            } else if (type == OTA_UPGRADE_FILE_TYPE_KEY) {
                FlashID_Erase_Sector(FLASH_ID_Publickey_File_Param);//����Ƭ��flash�Ĵ洢�ռ�
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
                LOG_D("�ȴ���ʱ");
                break;
            }
        }
    }
    
    return ret;
}

///@brief  �ն˽�������
static void Task_Config_Term(void *args)
{
    int ret = 0;
    int timeout_cnt = 0;
    char temp = 0;
    char cmmd_buff[10] = {0};
    static u_int8_t read1_buff[1500] = {0};//3k
    LOG_D("��ѡ�����¹�����");
    LOG_D("--> 1 ���������ļ� \r\n --> 2 ���ؼ���֤�� \r\n --> 3 ����ƽ̨��Կ");
    while(1)
    {
        mdelay(200);
        ret = term_uart_readln((u_int8_t*)cmmd_buff,sizeof(cmmd_buff),RT_WAITING_FOREVER);
        if (ret > 0)
        {
            temp = cmmd_buff[0];
            if (temp == '1')
            {
                LOG_D("--> ���������ļ�");
                int Ret = func_cmd_download_cfgfile(OTA_UPGRADE_FILE_TYPE_CFG);
                if (Ret > 2){
                    LOG_D("�����ļ����سɹ�");
                    FlashID_Read(read1_buff, OTA_UPGRADE_FILE_TYPE_CFG, 0, ret);
                    break;
                }else{
                    LOG_D("�����ļ�����ʧ��");
                    break;
                }
            } else if (temp == '2') {
                LOG_D("--> ���ؼ���֤��");
                int Ret = func_cmd_download_cfgfile(OTA_UPGRADE_FILE_TYPE_CERT);
                
                if (Ret > 2){
                    LOG_D("����֤�����سɹ�");
                    FlashID_Read(read1_buff, FLASH_ID_Cert_File_Param, 0, Ret);
                    LOG_HEX16("", read1_buff, Ret);
                    break;
                }else{
                    LOG_D("����֤������ʧ��");
                    break;
                }
            } else if (temp == '3') {
                LOG_D("--> ����ƽ̨��Կ");
                int Ret = func_cmd_download_cfgfile(OTA_UPGRADE_FILE_TYPE_KEY);
                
                if (Ret > 2){
                    LOG_D("����ƽ̨��Կ�ɹ�");
                    FlashID_Read(read1_buff, FLASH_ID_Publickey_File_Param, 0, Ret);
                    LOG_HEX16("", read1_buff, Ret);
                    break;
                }else{
                    LOG_D("����ƽ̨��Կʧ��");
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

/*   �ڴ��������(�߳�)   */
static task_info_t tasks_info[] = {
  {"term_run",  TASK_STK_SIZE_2048, DEVICE_TERM_RUN_TASK_PRIO, Task_Config_Term, 0},
};

///@brief   ���������̣߳�
static BOOL InitCreateTask(int task_id, char *name, void (*start_routine)(void *), int stk_size, int prio)
{
  tasks_info[task_id].thread = rt_thread_create(name, start_routine, NULL, stk_size, prio, 20);
  AssertError(tasks_info[task_id].thread != RT_NULL, return FALSE, "create thread:%s failed", name);
  rt_thread_startup(tasks_info[task_id].thread);
  
  return TRUE;
}

///@brief  ��ʼ��ͨ��ģ��
static BOOL InitModuleCommonFunc()
{    
    for (int i = 0; i < sizeof(tasks_info)/sizeof(task_info_t); ++i){
        LOG_D("starting : %s as task %d",tasks_info[i].name, i);
        AssertContinueNoPrint(tasks_info[i].threadFunc != NULL, {});
        InitCreateTask(i, tasks_info[i].name, tasks_info[i].threadFunc, tasks_info[i].stk_size, tasks_info[i].prio);
    }
    return TRUE;
}

///@brief  ��ʼ���ն˽���ģ��
BOOL InitDeviceTermModule()
{   
    // ��ʼ��ģ��ͨ�ù���
    InitModuleCommonFunc();
    
    return 0;
}