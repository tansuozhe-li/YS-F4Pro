/*******************************************************************************
*  版权所有 (C) 2023 -2028 ，清蓉深瞳科技有限公司。 *
*  文件名称： ds_finsh_cmd.c
*  内容摘要： finsh控制台自定义指令
*  其它说明： 
*  当前版本： V1.0
*  作    者： 
*  完成日期： 2023 年 11 月 27  日
*
*  修改记录 ：
*     2023年11月27日  cmd:"version_ds"  function: EITL版本信息
*                     
*
*******************************************************************************/

#include <rthw.h>
#include <rtthread.h>
#include "thread_cfg.h"
#include "rtdbg.h"
#ifdef RT_USING_FINSH
#include "finsh.h"
#include "common.h"
#include "w25q256.h"
#include "DeviceSystime.h"
#include "DeviceCfg.h"
#include "DeviceTerm.h"

extern uint8_t cfg_proc_table_size;

///@brief 重启
static void reboot(void)
{
    EITL_REBOOT();
}
//FINSH_FUNCTION_EXPORT(reboot, EITL reboot);
MSH_CMD_EXPORT(reboot, EITL reboot);

///@brief 显示EIT-L软件版本
static void version_ds(void)
{
    rt_kprintf("\n \\   |   /\n");
    rt_kprintf("-   EITL  -     DeepSensing EIT-L\n");
    rt_kprintf(" /   |   \\     V %d.%d.%d build %s\n",
               DS_SOFT_MAINVERSION, DS_SOFT_SUBVERSION, DS_SOFT_EDITEVERSION, __DATE__);
    rt_kprintf(" 2023 - 2028 Copyright by DeepSensing\n");
}
FINSH_FUNCTION_EXPORT(version_ds, show EITL version information);
MSH_CMD_EXPORT(version_ds, show EITL version information);

///@brief 显示设备当前的配置
static void config_list(int argc, char **argv)
{
  if (argc < 2){
    rt_kprintf("CfgSection            CfgKeyName            CfgDefaultValue       CfgActualValue        ModuleName\n"); 
    rt_kprintf("--------------------  --------------------  --------------------  --------------------  --------------------\n");
    for (int i = 0; i < cfg_proc_table_size; i++)
    {
        rt_kprintf("%-*.*s%-*.*s%-*.*s%-*.*s%-*.*s", 22,20, g_SysCfgTbl[i].cfgSection, 
                                                      22,20, g_SysCfgTbl[i].cfgKeyName, 
                                                      22,20, g_SysCfgTbl[i].cfgDefaultValue,
                                                      22,20, g_SysCfgTbl[i].cfgActualValue,
                                                      22,20, g_SysCfgTbl[i].cfgmodulename);
        rt_kprintf("\n");
    }
  }
}
FINSH_FUNCTION_EXPORT(config_list, show EITL config_list);
MSH_CMD_EXPORT(config_list, show EITL config_list);

///@brief 设置设备当前配置
static void config_set(int argc, char **argv)
{
    uint8_t cmd_flag = 0;
    if (argc < 2) {
        rt_kprintf("Please input'config_set CfgSection CfgKeyName Value'\n");
        return;
    }
    for (int i = 0; i < cfg_proc_table_size; i++)
    {
        if (!rt_strcmp(argv[1], (char *)g_SysCfgTbl[i].cfgSection)) {
            if (!rt_strcmp(argv[2], (char *)g_SysCfgTbl[i].cfgKeyName)) {
                cmd_flag = 1;
                memset(g_SysCfgTbl[i].cfgActualValue, 0, sizeof(g_SysCfgTbl[i].cfgActualValue));
                memcpy(g_SysCfgTbl[i].cfgActualValue, argv[3], strlen(argv[3]));
                g_SysCfgTbl[i].pSysCfgParamProc(i,0,g_SysCfgTbl[i].cfgActualValue);
            }
        }
    }
    AssertError(cmd_flag == 1, return, "config_set cmd error");
}
FINSH_FUNCTION_EXPORT(config_set, set EITL config);
MSH_CMD_EXPORT(config_set, set EITL config);

///@brief 将配置文件保存至外部flash中
static void config_save(int argc, char **argv)
{
    uint32_t offset = 0;
    FlashID_Erase_Sector(FLASH_ID_Config_File_Param);
    rt_thread_delay(100);
    char config_section[20] = {0};
    char config_buff[60] = {0};
    uint8_t save_strlen = 0;
    if (argc < 2) {
        for (int i = 0; i < cfg_proc_table_size; )
        {   
            memset(config_buff, 0, sizeof(config_buff));
            save_strlen = 0;
            if (rt_strcmp(config_section, (char *)g_SysCfgTbl[i].cfgSection))
            {
                memset(config_section, 0, sizeof(config_section));
                config_buff[0] = '[';
                save_strlen = 1;
                memcpy(config_section, g_SysCfgTbl[i].cfgSection, rt_strlen((char *)g_SysCfgTbl[i].cfgSection));
                memcpy(&config_buff[save_strlen], g_SysCfgTbl[i].cfgSection, rt_strlen((char *)g_SysCfgTbl[i].cfgSection));
                save_strlen = rt_strlen((char *)g_SysCfgTbl[i].cfgSection) + save_strlen;
                config_buff[save_strlen] = ']';
                save_strlen = save_strlen + 1;
                config_buff[save_strlen] = '\r';
                save_strlen = save_strlen + 1;
                config_buff[save_strlen] = '\n';
                save_strlen = save_strlen + 1;
            }
            memcpy(&config_buff[save_strlen], (char *)g_SysCfgTbl[i].cfgKeyName, rt_strlen((char *)g_SysCfgTbl[i].cfgKeyName));
            save_strlen = save_strlen + rt_strlen((char *)g_SysCfgTbl[i].cfgKeyName);
            config_buff[save_strlen] = '=';
            save_strlen += 1;
            memcpy(&config_buff[save_strlen], (char *)g_SysCfgTbl[i].cfgActualValue, rt_strlen((char *)g_SysCfgTbl[i].cfgActualValue));
            save_strlen = save_strlen + rt_strlen((char *)g_SysCfgTbl[i].cfgActualValue);
            config_buff[save_strlen] = '\r';
            save_strlen = save_strlen + 1;
            config_buff[save_strlen] = '\n';
            save_strlen = save_strlen + 1;
            FlashID_Write_MorePage((uint8_t *)config_buff, FLASH_ID_Config_File_Param, offset, save_strlen);
            i++;
            offset += save_strlen;
        } 
    }
}
FINSH_FUNCTION_EXPORT(config_save, save EITL config);
MSH_CMD_EXPORT(config_save, save EITL config);

///@brief 将保存在外部flash中的配置文件读出
static void config_erase(int argc, char **argv)
{
    FlashID_Erase_Sector(FLASH_ID_Config_File_Param);
    
}
FINSH_FUNCTION_EXPORT(config_erase, read EITL config);
MSH_CMD_EXPORT(config_erase, read EITL config);


#endif