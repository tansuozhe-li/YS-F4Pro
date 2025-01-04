#ifndef _W25Q256_H
#define _W25Q256_H

#include "gpio.h"

/*******************************************************************************
*                              常量&宏定义                                     *
*******************************************************************************/
#define FLASH_CS_ENABLE  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET) /*片选使能*/
#define FLASH_CS_DISABLE HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET) /*片选失能*/

#define FLASH_CLK_HIGH   HAL_GPIO_WritePin(FLASH_CLK_GPIO_Port, FLASH_CLK_Pin, GPIO_PIN_SET) /*时钟信号高*/
#define FLASH_CLK_LOW    HAL_GPIO_WritePin(FLASH_CLK_GPIO_Port, FLASH_CLK_Pin, GPIO_PIN_RESET) /*时钟信号低*/

#define FLASH_MISO_READ  HAL_GPIO_ReadPin(FLASH_MISO_GPIO_Port, FLASH_MISO_Pin) /*MISO数据输入*/

#define FLASH_MOSI_HIGH  HAL_GPIO_WritePin(FLASH_MOSI_GPIO_Port, FLASH_MOSI_Pin, GPIO_PIN_SET) /*MOSI数据引脚高电平*/
#define FLASH_MOSI_LOW   HAL_GPIO_WritePin(FLASH_MOSI_GPIO_Port, FLASH_MOSI_Pin, GPIO_PIN_RESET) /*MOSI数据引脚低电平*/

#define FLASH_W25Q128
#ifdef FLASH_W25Q128
#define    Flash_ReadData_CMD            0x03
#endif
#ifdef FLASH_W25Q256
#define    Flash_ReadData_CMD            0x13
#endif

#define    FLASH_WRITE_BUSYBIT           0X01
#define    Flash_ReadID                  0x90 /*读ID***16位*/
#define    Flash_ReadID_only             0x4b /*读唯一ID***64位*/
#define    Flash_Chip_Erase_CMD          0xC7 /*片擦除*/
#define    Flash_WriteEnable_CMD         0x06
#define    Flash_WriteDisable_CMD        0x04
#define    Flash_PageProgram_CMD         0x02
#define    Flash_WriteSR_CMD             0x01
#define    Flash_ReadSR_CMD              0x05
#define    Flash_SecErase_CMD            0x20
#define    Flash_BlockErase_CMD          0xD8
#define    W25Q_4ByteAddrModeEnable      0xB7 
#define    W25Q_Exit4ByteAddrModeEnable  0xE9 
#define    Flash_PAGEBYTE_LENGTH         256
#define    Flash_SECBYTE_LENGTH          (1024*4)
#define    Flash_BLOCKBYTE_LENGTH        (Flash_SECBYTE_LENGTH << 4) //Block大小
#define    EXT_FLASH_SECTOR_SIZE         (1024*4)
#define    EXT_FLASH_BLOCK_SIZE          (1024*64)

//配置文件地址信息--放在w25q128中
#define UPDATE_CFG_FILE_ADDR_BEGIN   (1024*1)//0block 6sector(4k) 包头放在5sector
#define UPDATE_CFG_FILE_ADDR_END     (1024*4*7-1)
#define UPDATE_CFG_FILE_SECTOR_BEGIN (UPDATE_CFG_FILE_ADDR_BEGIN / UPDATE_EXT_FLASH_SECTOR_SIZE)//配置文件
//计算FLASH存储的起始地址
#define FLASH_BLOCK_ADD(BLOCK) (BLOCK * 65536)
#define FLASH_SECTOR_ADD(BLOCK, SECTOR)  ((BLOCK * 65536) + (SECTOR * 4096))

#define W25Q128_BUFFER_LEN  256//一次读出的字节,且一定能被1024整除

/*******************************************************************************
*                                 函数声明                                     *
*******************************************************************************/
/* w25q256相关引脚初始化 */
void Flash_GPIO_Init(void);
/* 扇区擦除 */
void Flash_Erase_Block(uint8_t BlockNum);
/* 块擦除 */
void Flash_Erase_Sector(uint8_t Block_Num,uint8_t Sector_Number);
/* 写数据 */
void Flash_Write_Page(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t WriteBytesNum);
void Flash_Write_MorePage(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t WriteBytesNum);
/* 读数据 */
void Flash_Read(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t ReadBytesNum);

#endif