/*******************************************************************************
*  版权所有 (C) 2023 -2024 ，清蓉深瞳科技有限公司。 *
*  文件名称： onchip_flash.c
*  内容摘要： 片内flash驱动程序，对片内flash进行读、写、擦除操作
*  其它说明： 
*  当前版本： V1.0
*  作    者： 
*  完成日期： 2023 年 11 月 22  日
*
*  修改记录 1：
*        修改日期：2023年11月22日
*        修改内容：初稿V1.0
*  修改记录 2：
*******************************************************************************/
#include <string.h>
#include "onchip_flash.h"

///@brief  读片内指定地址的一个字节数据
static uint8_t OnChipFlashReadOneByte(uint32_t addr)
{
    return REG8(addr);
}

// 获取要擦除扇区编号
static uint32_t flash_sector_number(uint32_t address)
{
  uint32_t sector = 0;
 
  if((address < ADDR_FLASH_SECTOR_1) && (address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((address < ADDR_FLASH_SECTOR_2) && (address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((address < ADDR_FLASH_SECTOR_3) && (address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((address < ADDR_FLASH_SECTOR_4) && (address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((address < ADDR_FLASH_SECTOR_5) && (address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((address < ADDR_FLASH_SECTOR_6) && (address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((address < ADDR_FLASH_SECTOR_7) && (address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((address < ADDR_FLASH_SECTOR_8) && (address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((address < ADDR_FLASH_SECTOR_9) && (address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;
  }
  else if((address < ADDR_FLASH_SECTOR_10) && (address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;
  }
  else if((address < ADDR_FLASH_SECTOR_11) && (address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;
  }
  
  return sector;
}


///@brief  擦除片内指定flash扇区,Eraseaddr需擦除的对应地址，EraseNb擦除扇区的数量
int OnChipFlashEraseSector(uint32_t Eraseaddr, uint32_t EraseNb)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t SectorError;
    HAL_FLASH_Unlock(); //解锁
    FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS; //擦除类型，扇区擦除
    FlashEraseInit.Sector = flash_sector_number(Eraseaddr);; //要擦除的页
    FlashEraseInit.NbSectors = 1;//要擦除的页数
    FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                         FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_BSY);
    
    if (HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
        return -1;
    
    FLASH_WaitForLastOperation(50000);
    HAL_FLASH_Lock(); //上锁
    return 1;
}

///@brief  读片内指定地址的数据，以单个字节(8bit)读取
void OnChipFlashReadData(uint8_t *read_buff, uint32_t readaddr, uint16_t len)
{
    for (int i = 0; i < len; i++)
    {
        read_buff[i] = OnChipFlashReadOneByte(readaddr);
        readaddr++;
    }
}

///@brief  从指定地址开始写入指定长度数据，以单个字节(8bit)写入
int OnChipFlashWriteData(uint8_t *write_buff, uint32_t writeaddr, uint16_t len)
{
    //判断地址合法性
    if ((writeaddr < FLASH_BASE) || (writeaddr % 8))
        return -1;
    //判断长度是否超过限制
    if (len == 0)
    {
        return -1;
    }
    
    uint16_t data = 0;

    HAL_FLASH_Unlock(); //解锁
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                         FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_BSY);
    
    for (uint16_t i = 0; i < len; i += 2)
    {      
        data = (*(write_buff + i + 1) << 8) + (*(write_buff + i));
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)(writeaddr + i), data);
      
    }
    
    HAL_FLASH_Lock(); //上锁
    return 1;
}


