/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeviceSystime.c
*  内容摘要：  微气象模块主要功能实现
*  其他说明：
*  当前版本：  EIT-L_V1.0.0
*  作    者：  
*  完成日期：  2023年11月21日
*  
*  修改记录 1：
*        修改日期：2023年11月21日
*        修 改 人：
*        修改内容：创建工程文件
*  修改记录 2：
*******************************************************************************/
#include "DeviceCfg.h"
#include "w25q256.h"

///@brief 通过ID返回FLASH的BLOCK
u_int16_t  IDtoBlock(FLASH_ID_USER_e flash_id)
{
  if(flash_id > MAX_FLASH_ID)
    return 256;
    
  u_int8_t Block_Num = flash_id / 16;
  return Block_Num;
}

///@brief 通过ID返回FLASH的Sector扇区，范围（0~15）
u_int8_t  IDToSector(FLASH_ID_USER_e flash_id)
{
  if(flash_id > MAX_FLASH_ID)
    return 0;
  
  u_int8_t Sector_Num = flash_id % 16;
  return Sector_Num;
}

///@brief 通过ID返回片外FLASH的实际地址；
u_int32_t  IDToAddr(FLASH_ID_USER_e flash_id)
{
    u_int16_t Block_Num = IDtoBlock(flash_id);
    u_int8_t Sector_Num = IDToSector(flash_id);
    u_int32_t Addr = EXT_FLASH_BLOCK_SIZE * Block_Num + EXT_FLASH_SECTOR_SIZE * Sector_Num;
    return Addr;
}

///@brief 按宏定义ID擦除对应的Sector
void FlashID_Erase_Sector(FLASH_ID_USER_e flash_id)
{
    rt_mutex_take(flash_mutex,RT_WAITING_FOREVER);
    u_int16_t Block_Num = IDtoBlock(flash_id);
    u_int8_t Sector_Num = IDToSector(flash_id);
    Flash_Erase_Sector(Block_Num,Sector_Num);
    rt_mutex_release(flash_mutex);
}

///@brief 按宏定义ID写入信息到flash
void  FlashID_Write_MorePage(u_int8_t *pBuffer, FLASH_ID_USER_e flash_id, uint32_t offset, u_int32_t WriteBytesNum)
{
    rt_mutex_take(flash_mutex,RT_WAITING_FOREVER);
    u_int32_t WriteAddr = IDToAddr(flash_id) + offset;
    Flash_Write_MorePage(pBuffer, WriteAddr, WriteBytesNum);
    rt_mutex_release(flash_mutex);
}

///@brief 按宏定义ID读取flash的信息
void  FlashID_Read(u_int8_t *pBuffer,FLASH_ID_USER_e flash_id, uint32_t offset, u_int32_t ReadBytesNum)
{
    rt_mutex_take(flash_mutex,RT_WAITING_FOREVER);
    u_int32_t ReadAddr = IDToAddr(flash_id) + offset;
    Flash_Read(pBuffer, ReadAddr, ReadBytesNum);
    rt_mutex_release(flash_mutex);
}

