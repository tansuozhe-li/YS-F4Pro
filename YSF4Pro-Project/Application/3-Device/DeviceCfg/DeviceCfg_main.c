/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeviceCfg_main.c
*  内容摘要：  系统时钟
*  其他说明：
*  当前版本：  EIT-L_V1.0.0
*  作    者：  
*  完成日期：  2023年12月1日
*  
*  修改记录 1：
*        修改日期：2023年12月1日
*        修 改 人：
*        修改内容：创建工程文件
*  修改记录 2：
*******************************************************************************/
#include "DeviceCfg.h"
#include "common.h"
#include "thread_cfg.h"
#include "getcfg.h"
#include <stdlib.h>
#include "ec800k.h"
rt_mutex_t flash_mutex = RT_NULL;
static uint8_t config_buff[2048] = {0};
uint8_t cfg_proc_table_size = 0;
DeviceLteModuleCfg_t DeviceLteModuleCfg;
DsPlatformModuleCfg_t DsPlatformModuleCfg;
DeviceIceModuleCfg_t DeviceIceModuleCfg;

void GetCfgProc_LteMain_Enable(uint32_t cfgtalno, BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    if (!rt_strcmp((char *)buff, "true")){
        DeviceLteModuleCfg.LteMain_Enable = TRUE;
    }else if (!rt_strcmp((char *)buff, "false")){
        DeviceLteModuleCfg.LteMain_Enable = FALSE;
    }else {
        DeviceLteModuleCfg.LteMain_Enable = TRUE;
    }
}
void GetCfgProc_LteMain_APN(uint32_t cfgtalno, BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceLteModuleCfg.LteMain_APN, 0, sizeof(DeviceLteModuleCfg.LteMain_APN));
    memcpy(DeviceLteModuleCfg.LteMain_APN, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_LteMain_Ch0SrvIP(uint32_t cfgtalno, BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceLteModuleCfg.LteMain_Ch0SrvIP, 0, sizeof(DeviceLteModuleCfg.LteMain_Ch0SrvIP));
    memcpy(DeviceLteModuleCfg.LteMain_Ch0SrvIP, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_LteMain_Ch0SrvPort(uint32_t cfgtalno, BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    DeviceLteModuleCfg.LteMain_Ch0SrvPort = atoi((char *)buff);
}
void GetCfgProc_LteMain_Ch0NetType(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceLteModuleCfg.LteMain_Ch0NetType, 0, sizeof(DeviceLteModuleCfg.LteMain_Ch0NetType));
    memcpy(DeviceLteModuleCfg.LteMain_Ch0NetType, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_LteMain_Ch1SrvIP(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceLteModuleCfg.LteMain_Ch1SrvIP, 0, sizeof(DeviceLteModuleCfg.LteMain_Ch1SrvIP));
    memcpy(DeviceLteModuleCfg.LteMain_Ch1SrvIP, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_LteMain_Ch1SrvPort(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    DeviceLteModuleCfg.LteMain_Ch1SrvPort = atoi((char *)buff);
}
void GetCfgProc_LteMain_Ch1NetType(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceLteModuleCfg.LteMain_Ch1NetType, 0, sizeof(DeviceLteModuleCfg.LteMain_Ch1NetType));
    memcpy(DeviceLteModuleCfg.LteMain_Ch1NetType, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_LteSub_Enable(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    if (!rt_strcmp((char *)buff, "true")){
        DeviceLteModuleCfg.LteSub_Enable = TRUE;
    }else if (!rt_strcmp((char *)buff, "false")){
        DeviceLteModuleCfg.LteSub_Enable = FALSE;
    }else {
        DeviceLteModuleCfg.LteSub_Enable = TRUE;
    }
}
void GetCfgProc_LteSub_APN(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceLteModuleCfg.LteSub_APN, 0, sizeof(DeviceLteModuleCfg.LteSub_APN));
    memcpy(DeviceLteModuleCfg.LteSub_APN, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_LteSub_Ch0SrvIP(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceLteModuleCfg.LteSub_Ch0SrvIP, 0, sizeof(DeviceLteModuleCfg.LteSub_Ch0SrvIP));
    memcpy(DeviceLteModuleCfg.LteSub_Ch0SrvIP, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_LteSub_Ch0SrvPort(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    DeviceLteModuleCfg.LteSub_Ch0SrvPort = atoi((char *)buff);
}
void GetCfgProc_LteSub_Ch0NetType(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceLteModuleCfg.LteSub_Ch0NetType, 0, sizeof(DeviceLteModuleCfg.LteSub_Ch0NetType));
    memcpy(DeviceLteModuleCfg.LteSub_Ch0NetType, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_DsPlatform_Enable(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    if (!rt_strcmp((char *)buff, "true")){
        DsPlatformModuleCfg.Enable = TRUE;
    }else if (!rt_strcmp((char *)buff, "false")){
        DsPlatformModuleCfg.Enable = FALSE;
    }else {
        DsPlatformModuleCfg.Enable = TRUE;
    }
}
void GetCfgProc_DsPlatform_MainCh0(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    if (!rt_strcmp((char *)buff, "MainCh0")){
        DsPlatformModuleCfg.ChSelect = LTE1_CHN_DS;
    }
}
void GetCfgProc_DsPlatform_CmdId(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DsPlatformModuleCfg.CmdId, 0, sizeof(DsPlatformModuleCfg.CmdId));
    memcpy(DsPlatformModuleCfg.CmdId, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_DsPlatform_Heartbeat(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    DsPlatformModuleCfg.TimeInterval = atoi((char *)buff);
    if (DsPlatformModuleCfg.TimeInterval < 60) {
        DsPlatformModuleCfg.TimeInterval = 120;
    }
    if (DsPlatformModuleCfg.TimeInterval > 600) {
        DsPlatformModuleCfg.TimeInterval = 120;
    }
}
void GetCfgProc_DsPlatform_StatusData(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    DsPlatformModuleCfg.StatusInterval = atoi((char *)buff);
    if (DsPlatformModuleCfg.StatusInterval < 60) {
        DsPlatformModuleCfg.StatusInterval = 600;
    }
    if (DsPlatformModuleCfg.StatusInterval > 3600) {
        DsPlatformModuleCfg.StatusInterval = 600;
    }
}
/*
 ************************** 覆冰模块配置 ***************************************
 */
void GetCfgProc_Ice_Enable(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));  
    if (!rt_strcmp((char *)buff, "true")){
        DeviceIceModuleCfg.Enable = TRUE;
    }else if (!rt_strcmp((char *)buff, "false")){
        DeviceIceModuleCfg.Enable = FALSE;
    }else {
        DeviceIceModuleCfg.Enable = TRUE;
    }
}
void GetCfgProc_Ice_ID(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    DeviceIceModuleCfg.ID[0] = atoi((char *)buff);
    DeviceIceModuleCfg.ID[1] = atoi((char *)buff) +1;
}
void GetCfgProc_Ice_SetZeroOffsetX(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    DeviceIceModuleCfg.Angle_X = atoi((char *)buff);
}
void GetCfgProc_Ice_SetZeroOffsetY(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    DeviceIceModuleCfg.Angle_Y = atoi((char *)buff);
}
void GetCfgProc_Ice_Type(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceIceModuleCfg.sensor_type, 0, sizeof(DeviceIceModuleCfg.sensor_type));
    memcpy(DeviceIceModuleCfg.sensor_type, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_Ice_CompoentCmdId(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    memset(DeviceIceModuleCfg.CompoentCmdId, 0, sizeof(DeviceIceModuleCfg.CompoentCmdId));
    memcpy(DeviceIceModuleCfg.CompoentCmdId, (char *)buff, rt_strlen((char *)buff));
}
void GetCfgProc_Ice_TimeInterval(uint32_t cfgtalno,BOOL status, u_int8_t* buff)
{
    memcpy(g_SysCfgTbl[cfgtalno].cfgActualValue, buff, rt_strlen((char *)buff));
    DeviceIceModuleCfg.TimeInterval = atoi((char *)buff);
    if (DeviceIceModuleCfg.TimeInterval < 60) {
        DeviceIceModuleCfg.TimeInterval = 60;
    }
    if (DeviceIceModuleCfg.TimeInterval > 3600) {
        DeviceIceModuleCfg.TimeInterval = 3600;
    }
}


/* 系统配置参数获取处理表 根据需要填写回调函数，处理配置参数*/
SysCfgProcTable_t g_SysCfgTbl[] =
{    
  { "LTE主设备",     "LteMain",     "Enable",         "true",              "",  GetCfgProc_LteMain_Enable},
  { "LTE主设备",     "LteMain",     "APN",            "CMNET",             "",  GetCfgProc_LteMain_APN}, 
  { "LTE主设备",     "LteMain",     "Ch0SrvIP",       "deepsensing.cn",    "",  GetCfgProc_LteMain_Ch0SrvIP},
  { "LTE主设备",     "LteMain",     "Ch0SrvPort",     "30011",             "",  GetCfgProc_LteMain_Ch0SrvPort},
  { "LTE主设备",     "LteMain",     "Ch0NetType",     "UDP",               "",  GetCfgProc_LteMain_Ch0NetType},
  { "LTE主设备",     "LteMain",     "Ch1SrvIP",       "deepsensing.cn",      "",  GetCfgProc_LteMain_Ch1SrvIP},
  { "LTE主设备",     "LteMain",     "Ch1SrvPort",     "30011",              "",  GetCfgProc_LteMain_Ch1SrvPort},
  { "LTE主设备",     "LteMain",     "Ch1NetType",     "UDP",               "",  GetCfgProc_LteMain_Ch1NetType},
                                    
  { "LTE副设备",     "LteSub",      "Enable",         "true",             "",  GetCfgProc_LteSub_Enable},
  { "LTE副设备",     "LteSub",      "APN",            "CMNET",             "",  GetCfgProc_LteSub_APN},
  { "LTE副设备",     "LteSub",      "Ch0SrvIP",       "124.221.101.123",    "",  GetCfgProc_LteSub_Ch0SrvIP},
  { "LTE副设备",     "LteSub",      "Ch0SrvPort",     "6001",             "",  GetCfgProc_LteSub_Ch0SrvPort},
  { "LTE副设备",     "LteSub",      "Ch0NetType",     "UDP",               "",  GetCfgProc_LteSub_Ch0NetType},
                                   
  { "清蓉深瞳平台",  "DsPlatform",  "Enable",         "true",              "",  GetCfgProc_DsPlatform_Enable},
  { "清蓉深瞳平台",  "DsPlatform",  "ChSelect",       "MainCh0",           "",  GetCfgProc_DsPlatform_MainCh0},
  { "清蓉深瞳平台",  "DsPlatform",  "CmdId",          "DS_EITL_202400001", "",  GetCfgProc_DsPlatform_CmdId}, 
  { "清蓉深瞳平台",  "DsPlatform",  "TimeInterval",   "60",                "",  GetCfgProc_DsPlatform_Heartbeat}, 
                                   
  { "覆冰监测",      "Ice",         "Enable",         "true",             "",  GetCfgProc_Ice_Enable},
  { "覆冰监测",      "Ice",         "ID",             "1",                "",  GetCfgProc_Ice_ID},
  { "覆冰监测",      "Ice",         "SensorType",     "TianGuang",         "",  GetCfgProc_Ice_Type},
  { "覆冰监测",      "Ice",         "TimeInterval",   "600",               "",  GetCfgProc_Ice_TimeInterval},
  { "覆冰监测",      "Ice",         "CompoentCmdId",  "DS_EITL_202400004", "",  GetCfgProc_Ice_CompoentCmdId},
  { "覆冰监测",      "Ice",         "Angle_X",        "0",                 "",  GetCfgProc_Ice_SetZeroOffsetX},
  { "覆冰监测",      "Ice",         "Angle_Y",        "0",                 "",  GetCfgProc_Ice_SetZeroOffsetY},
};


///@brief   配置参数获取处理
void GetSysCfgHandler(u_int8_t *buff)   
{
    static u_int8_t read_buff[30]={0x00};
    int ret = 0;
    for (uint32_t i = 0; i < sizeof(g_SysCfgTbl) / sizeof(SysCfgProcTable_t); i++)
    {
        memset(read_buff,0,sizeof(read_buff));
        ret = GetConfigFromBuf((char *)g_SysCfgTbl[i].cfgSection, (char *)g_SysCfgTbl[i].cfgKeyName, (char *)g_SysCfgTbl[i].cfgDefaultValue, (char *)read_buff,sizeof(read_buff),(char *)buff);
        AssertError(RT_EOK == ret, {}, "Get CfgFile [%s][%s]:%sfailed",g_SysCfgTbl[i].cfgSection,g_SysCfgTbl[i].cfgKeyName, ret);
        //LOG_D("[%s][%s]: [%s]", g_SysCfgTbl[i].cfgSection, g_SysCfgTbl[i].cfgKeyName, read_buff);
        g_SysCfgTbl[i].pSysCfgParamProc(i, ret, read_buff);
        memcpy(g_SysCfgTbl[i].cfgActualValue, read_buff, rt_strlen((char *)read_buff));
    }
}

///@brief   初始化配置模块，获取相关配置参数
BOOL InitDeviceCfgModule()
{   
    LOG_D("Init Device Config, Please waiting ... ...");
    //创建一个动态互斥量
    flash_mutex = rt_mutex_create("flash_mutex", RT_IPC_FLAG_FIFO);
    AssertError(flash_mutex != RT_NULL, return FALSE, "create flash_mutex:%s failed", "flash_mutex");
    cfg_proc_table_size = sizeof(g_SysCfgTbl) / sizeof(SysCfgProcTable_t);
    memset(config_buff,0,sizeof(config_buff));
    FlashID_Read(config_buff, FLASH_ID_Config_File_Param, 0, sizeof(config_buff));
    GetSysCfgHandler(config_buff);
    return 0;
}


