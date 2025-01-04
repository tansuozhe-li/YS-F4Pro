#include <string.h>
#include "onchip_flash.h"

///@brief  读片内指定地址的一个字节数据
static uint8_t OnChipFlashReadOneByte(uint32_t addr)
{
    return REG8(addr);
}

///@brief  识别FLASH地址所在BANK(该芯片有两个BANK)
static uint32_t GetOnChipFlashBank(uint32_t addr)
{
    uint32_t bank = 0;
//    if(addr < (FLASH_BASE + FLASH_BANK_SIZE))
//    {
//        bank = FLASH_BANK_1;
//    }
//    else
//    {
//        //bank = FLASH_BANK_2;
//    }
    return bank;
}

///@brief  擦除片内指定flash扇区,Eraseaddr需擦除的对应地址，EraseNb擦除扇区的数量
int OnChipFlashEraseSector(uint32_t Eraseaddr, uint32_t EraseNb)
{
//    HAL_FLASH_Unlock(); //解锁
//    FLASH_EraseInitTypeDef FlashEraseInit;
//    uint32_t SectorError;
//    FlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES; //擦除类型，扇区擦除
//    FlashEraseInit.Banks = GetOnChipFlashBank(Eraseaddr);//要擦除的bank
//    FlashEraseInit.Page = (Eraseaddr - (uint32_t)FLASH_BASE) / STM_SECTOR_SIZE; //要擦除的页
//    FlashEraseInit.NbPages = EraseNb;//要擦除的页数
//    
//    if (HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
//        return -1;
//    HAL_FLASH_Unlock(); //上锁
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
///        STM32L4片内flash写操作需要双字(double word，64bit)，需将uint8_t转换成uint64_t并对齐
static uint64_t write_data_buff[256] __attribute__( ( aligned ( 8 ) ) );
int OnChipFlashWriteData(uint8_t *write_buff, uint32_t writeaddr, uint16_t len)
{
    //判断地址合法性
    if ((writeaddr < FLASH_BASE) || (writeaddr % 8))
        return -1;
    //判断长度是否超过限制
    if (len > (uint16_t)STM_SECTOR_SIZE)
        return -1;
    
    memset(write_data_buff, 0, 256);
    memcpy(write_data_buff, write_buff, len);
    uint16_t u64_data_len = len / 8;
    if (len <= 8){
       u64_data_len = 1;
    }else{
      if ((len % 8) != 0){
       u64_data_len += 1; 
      }
    }
    
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t SectorError;

    HAL_FLASH_Unlock(); //解锁
    
    uint32_t erase_page = 0;
    uint32_t last_erase_page = 0;
    
    for (uint16_t i = 0; i < u64_data_len ;)
    {
        erase_page = (writeaddr - (uint32_t)FLASH_BASE) / STM_SECTOR_SIZE;
        if (erase_page > last_erase_page)
        {
            //写入数据前先执行擦除操作
//            FlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES; //擦除类型，扇区擦除
//            FlashEraseInit.Banks = GetOnChipFlashBank(writeaddr);//要擦除的bank
//            FlashEraseInit.Page = erase_page; //要擦除的页
//            FlashEraseInit.NbPages = 1;//要擦除的页数
            
            if (HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
                return -1;
            last_erase_page = erase_page;
        }
      
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, writeaddr, write_data_buff[i]) != HAL_OK)
            return -1;
        writeaddr += 8;
        i++;
    }
    
    HAL_FLASH_Unlock(); //上锁
    return 1;
}


