#ifndef _ON_CHIP_FLASH_H
#define _ON_CHIP_FLASH_H
#include "stm32f4xx_hal.h"

/*******************************************************************************
*                              常量&宏定义                                     *
*******************************************************************************/         
#define REG64(addr)   (*(volatile uint64_t *)(uint32_t)(addr))
#define REG32(addr)   (*(volatile uint32_t *)(uint32_t)(addr))
#define REG16(addr)   (*(volatile uint16_t *)(uint32_t)(addr))
#define REG8(addr)    (*(volatile uint8_t *)(uint32_t)(addr))

#define STM_SECTOR_SIZE	2048

/*******************************************************************************
*                                 函数声明                                     *
*******************************************************************************/
///@brief  擦除片内指定flash扇区
int OnChipFlashEraseSector(uint32_t Eraseaddr, uint32_t EraseNb);
///@brief  读片内指定地址的数据，以单个字节(8bit)读取
void OnChipFlashReadData(uint8_t *read_buff, uint32_t addr, uint16_t len);
///@brief  写片内指定地址的数据，以单个字节(8bit)写入
int OnChipFlashWriteData(uint8_t *write_buff, uint32_t addr, uint16_t len);
#endif