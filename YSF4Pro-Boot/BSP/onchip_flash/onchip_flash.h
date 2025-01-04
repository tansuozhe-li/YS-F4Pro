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


#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

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