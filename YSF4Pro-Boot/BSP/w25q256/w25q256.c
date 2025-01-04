/*******************************************************************************
*  版权所有 (C) 2023 -2024 ，清蓉深瞳科技有限公司。 *
*  文件名称： w25q256.c
*  内容摘要： 片外Flash W25Q256驱动程序，采用软件模拟方式。
*  其它说明： 
*  当前版本： V1.0
*  作    者： 
*  完成日期： 2023 年 11 月 16  日
*
*  修改记录 1：
*        修改日期：2023年11月16日
*        修改内容：初稿V1.0
*  修改记录 2：
*******************************************************************************/
#include <string.h>
#include "w25q256.h"

///@brief   Flash相关引脚初始化
void Flash_GPIO_Init(void)
{
  FLASH_CS_DISABLE;
  FLASH_CLK_HIGH;
  FLASH_MOSI_LOW;
}

///@brief   读取一个字节数据
static uint8_t Flash_ReadOneByte(void)
{
    uint8_t retValue = 0;

    FLASH_CLK_HIGH; //时钟线拉高,恢复时钟线为高电平
    for(uint8_t i= 0; i < 8; i++)
    {
        retValue <<= 1;
        FLASH_CLK_HIGH; //时钟线拉高,恢复时钟线为高电平
        if(FLASH_MISO_READ)
        {
            retValue |= 0x01;
        }
        else
        {
            retValue &= 0xFE;
        }
        FLASH_CLK_LOW; //时钟线拉低,产生下降沿读出数据
    }
    FLASH_CLK_HIGH;
   
    return (retValue);
}

///@brief   写入一个字节
static void Flash_WriteOneByte(uint8_t DataBuffer)
{
    FLASH_CLK_LOW; //时钟线拉低，恢复时钟线为低电平
    for(uint8_t i = 0; i < 8; i++)
    {
        FLASH_CLK_LOW; //时钟线拉低，恢复时钟线为低电平
        if(DataBuffer & 0x80)
        {
            FLASH_MOSI_HIGH;
        }
        else
        {
            FLASH_MOSI_LOW;
        }
        DataBuffer <<= 1;
        FLASH_CLK_HIGH; //时钟线拉高，产生上升沿写入数据
    }
    FLASH_CLK_LOW;
    FLASH_MOSI_HIGH; //一字节数据传送完毕，MOSI数据线置高表示空闲状态
   
}

///@brief   写指令
static void Flash_Write_CMD(uint8_t *pCMD)
{
#ifdef FLASH_W25Q256  
     for(uint8_t i = 0; i < 4; i++)  //new add 
    {
        Flash_WriteOneByte(pCMD[i]);
    }
#endif
    
#ifdef FLASH_W25Q128  
     for(uint8_t i = 0; i < 3; i++)  //new add 
    {
        Flash_WriteOneByte(pCMD[i]);
    }
#endif        
}

///@brief   读取SR
static uint8_t Flash_ReadSR(void)
{
    uint8_t retValue = 0;

    FLASH_CS_ENABLE;
    Flash_WriteOneByte(Flash_ReadSR_CMD);
    retValue = Flash_ReadOneByte();
    FLASH_CS_DISABLE;

    return retValue;
}

///@brief   等待标志位
static void Flash_Wait_Busy(void)
{
   uint32_t i =0;

   while(((Flash_ReadSR() & FLASH_WRITE_BUSYBIT) == 0x01) && (i<0x1ffff))
   {
      i ++;
   }
}

///@brief   写使能
static void Flash_Write_Enable(void)
{
    FLASH_CS_ENABLE;
    Flash_WriteOneByte(Flash_WriteEnable_CMD);
    FLASH_CS_DISABLE;
}

#if 0
///@brief   进入4字节模式
void Enter4ByteAddrMode(void) 
{ 
    FLASH_CS_ENABLE;                                //使能器件    
    Flash_WriteOneByte(W25Q_4ByteAddrModeEnable);   //进入4Byte地址模式W25Q128以上使用 
    FLASH_CS_DISABLE; 
}

///@brief   退出4字节模式
void Exit4ByteAddrMode(void) 
{ 
    FLASH_CS_ENABLE;                                //使能器件    
    Flash_WriteOneByte(W25Q_Exit4ByteAddrModeEnable);   //进入4Byte地址模式W25Q128以上使用 
    FLASH_CS_DISABLE; 
}
#endif

///@brief   擦除区域
void Flash_Erase_Sector(uint8_t Block_Num,uint8_t Sector_Number)
{    
    uint8_t pcmd[3] = {0};
    Flash_Write_Enable();
    FLASH_CS_ENABLE;
    Flash_WriteOneByte(Flash_SecErase_CMD);

    pcmd[0] = Block_Num;
    pcmd[1] = Sector_Number<<4;
    pcmd[2] = 0;
    Flash_Write_CMD(pcmd);
    FLASH_CS_DISABLE;
    Flash_Wait_Busy();//每次擦除数据都要延时等待写入结束
   
    return ;
}

///@brief   擦除块
void Flash_Erase_Block(uint8_t BlockNum)
{
    
    uint8_t pcmd[3] = {0};
    Flash_Write_Enable();   //写使能
    FLASH_CS_ENABLE;        //片选拉低
    Flash_WriteOneByte(Flash_BlockErase_CMD);  //传输Block擦除指令
    pcmd[0] = BlockNum ;     //传24位地址
    Flash_Write_CMD(pcmd);
    FLASH_CS_DISABLE;
    Flash_Wait_Busy();      //每次擦除数据都要延时等待写入结束
   
    return ;
}

///@brief   页写
void Flash_Write_Page(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t WriteBytesNum)
{
    uint8_t pcmd[4] = {0};    
                        
    Flash_Write_Enable();
    FLASH_CS_ENABLE;
    Flash_WriteOneByte(Flash_PageProgram_CMD);
#ifdef FLASH_W25Q256    
    pcmd[0] = (uint8_t)((WriteAddr&0xff000000)>>24);     //new add
    pcmd[1] = (uint8_t)((WriteAddr&0x00ff0000)>>16);
    pcmd[2] = (uint8_t)((WriteAddr&0x0000ff00)>>8);
    pcmd[3] = (uint8_t)WriteAddr;
#endif
    
#ifdef FLASH_W25Q128    
    pcmd[0] = (uint8_t)((WriteAddr&0x00ff0000)>>16);
    pcmd[1] = (uint8_t)((WriteAddr&0x0000ff00)>>8);
    pcmd[2] = (uint8_t)WriteAddr;
#endif    
    
    Flash_Write_CMD(pcmd);

    for(uint32_t  i = 0;i < WriteBytesNum; i++)
    {

        Flash_WriteOneByte(pBuffer[i]);  //向Flash中写入最大一页256bytes字节数据
    }
    FLASH_CS_DISABLE;
    Flash_Wait_Busy();                   //每次写入一定数量的数据都要延时等待写入结束
    return;
}

///@brief   连续写入多页
void Flash_Write_MorePage(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t WriteBytesNum)
{    
    uint16_t PageByteRemain = 0;
    PageByteRemain = Flash_PAGEBYTE_LENGTH - WriteAddr%Flash_PAGEBYTE_LENGTH;
    if(WriteBytesNum <= PageByteRemain)
    {
        PageByteRemain = WriteBytesNum;
    }
    while(1)
    {
        Flash_Write_Page(pBuffer,WriteAddr,PageByteRemain);
        if(WriteBytesNum == PageByteRemain)
        {
            break;
        }
        else
        {
            pBuffer       += PageByteRemain;
            WriteAddr     += PageByteRemain;
            WriteBytesNum -= PageByteRemain;
            if(WriteBytesNum > Flash_PAGEBYTE_LENGTH)
            {
                PageByteRemain = Flash_PAGEBYTE_LENGTH;
            }
            else
            {
                PageByteRemain = WriteBytesNum;
            }
        }
    }
    
    return;
}

///@brief   读取数据
void Flash_Read(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t ReadBytesNum)
{   
    uint8_t pcmd[4] = {0}; 
    FLASH_CS_ENABLE; //打开spiflash片选
    Flash_WriteOneByte(Flash_ReadData_CMD);
#ifdef FLASH_W25Q256
    pcmd[0] = (uint8_t)((ReadAddr&0xff000000)>>24); //new add
    pcmd[1] = (uint8_t)((ReadAddr&0x00ff0000)>>16);
    pcmd[2] = (uint8_t)((ReadAddr&0x0000ff00)>>8);
    pcmd[3] = (uint8_t)ReadAddr;
#endif
    
#ifdef FLASH_W25Q128  
    pcmd[0] = (uint8_t)((ReadAddr&0x00ff0000)>>16);
    pcmd[1] = (uint8_t)((ReadAddr&0x0000ff00)>>8);
    pcmd[2] = (uint8_t)ReadAddr;    
#endif   
    
    Flash_Write_CMD(pcmd);

    for(uint32_t i = 0;i < ReadBytesNum; i++)
    {
       pBuffer[i] = Flash_ReadOneByte();  //读取SPIflash中指定bytes字节数据

    }
    FLASH_CS_DISABLE;
    
    return ;
}

#if 0
///@brief   大量数据写入FLASH
void write_many_data(int n,unsigned char *p,unsigned int start_address)
{ /*n是数据总长度,*p是要写入数据的地址，start_address是写入Flash的地址，将除了帧头和动作之外的所有数据全部写入FLASH中*/
  unsigned char block;
 if((start_address%65536)==0)
  {
    block=(start_address/65536);
    Flash_Erase_Block(block);
  }
  Flash_Write_MorePage(p,start_address,n);//写入的数据是除开帧头和动作后剩下的所有数据
}

///@brief  falsh随机存储函数，即可以在FLASH任意一个地址存储数据，write_add写入的首地址
///        cache_add缓存地址，缓存大小默认为一个sector，即4KB。由于flash写入数据需要将整个sector擦出，
///        因此为了不影响sector其他不需要修改的数据，则需要一个缓存区。
///       *data需要写入数据的指针
///        len需要写入数据的长度
///        随机写入校验码计算
unsigned int w25q128_random_crc(char *data, unsigned int len)
{
  unsigned int i = 0;
  unsigned int crc = 0;
  for(i = 0; i < len; i++)
  {
    crc = crc + data[i];
  }
  return crc;
}

///@brief   根据BLOCK和SECTOR计算详细的地址
w25q128_add_inf w25q128_block_sector_to_add(unsigned char block, unsigned char sector)
{
  w25q128_add_inf inf;
  inf.block = block;
  inf.sector = sector;
  
  //计算所处block的首地址
  inf.block_first_add = 64 * 1024 * inf.block;
  //计算所处block的尾地址
  inf.block_tail_add  = 64 * 1024 * (inf.block + 1) - 1;
  
  //计算所处sector的首地址
  inf.sector_first_add = 64 * 1024 * inf.block + 4 * 1024 * inf.sector;
  //计算所处sector的尾地址
  inf.sector_tail_add  = 64 * 1024 * inf.block + 4 * 1024 * (inf.sector + 1) - 1;
  
  inf.add = inf.sector_first_add;//此处返回该sector的首地址
  
  return inf;
}

///@brief   通过某个地址，计算该地址对应的block和sector等详细的地址信息
w25q128_add_inf w25q128_add_to_block_sector(unsigned int add)
{
  w25q128_add_inf inf;
  
  inf.add = add;
  
  //计算处于哪个block
  if((add + 1) >= (64 * 1024))
  {
    if((add + 1) % (64 * 1024) == 0)
    {
      inf.block = (add + 1) / (64 * 1024) - 1;
    }
    else
    {
      inf.block = (add + 1) / (64 * 1024);
    }
  }
  else
  {
    inf.block = 0;
  }
  //计算所处block的首地址
  inf.block_first_add = 64 * 1024 * inf.block;
  //计算所处block的尾地址
  inf.block_tail_add  = 64 * 1024 * (inf.block + 1) - 1;
  
  //计算处于哪个sector
  if((add + 1 - inf.block * 64 * 1024) >= (4 * 1024))
  {
    if((add + 1 - inf.block * 64 * 1024) % (4 * 1024) == 0)
    {
      inf.sector = (add + 1 - inf.block * 64 * 1024) / (4 * 1024) - 1;
    }
    else
    {
      inf.sector = (add + 1 - inf.block * 64 * 1024) / (4 * 1024);
    }
  }
  else
  {
    inf.sector = 0;
  }
  
  //计算所处sector的首地址
  inf.sector_first_add = 64 * 1024 * inf.block + 4 * 1024 * inf.sector;
  //计算所处sector的尾地址
  inf.sector_tail_add  = 64 * 1024 * inf.block + 4 * 1024 * (inf.sector + 1) - 1;
  
  return inf;
}

///@brief   随机写入数据write_add随机写入的地址；cache_add缓存区地址，必须是某个sector的首地址；
///         *data需要写入数据的指针；len需要写入数据的长度
char w25q128_random_write(unsigned int write_add, unsigned int cache_add, char *data, unsigned int len)
{

  unsigned int  save_write_add, save_read_add, cache_write_add, cache_read_add;
  
  unsigned char data_buffer[W25Q128_BUFFER_LEN];
  unsigned int i ,j;
  unsigned int total_byte, replace_len, replace_len_total; //需要从新写入的所有字节数,需要替换的长度
  w25q128_add_inf save_write_inf, save_first_sector_inf, save_tail_sector_inf, cache_inf;

  
  save_first_sector_inf = w25q128_add_to_block_sector(write_add);
  save_tail_sector_inf  = w25q128_add_to_block_sector(write_add + len - 1);
  cache_inf             = w25q128_add_to_block_sector(cache_add);
  //清除缓存区SECTOR的数据
  Flash_Erase_Sector(cache_inf.block, cache_inf.sector);
  
  total_byte = 
    (save_tail_sector_inf.block - save_first_sector_inf.block + 1) * 64 * 1024 - save_first_sector_inf.sector * 4 * 1024 - ( 15 - save_tail_sector_inf.sector) * 4 * 1024;
  
  save_read_add = 
    w25q128_block_sector_to_add(save_first_sector_inf.block, save_first_sector_inf.sector).sector_first_add;//计算需要搬运数据的第一个sector的首地址
  save_write_add = save_read_add;
  
  
  cache_read_add = cache_inf.sector_first_add;
  cache_write_add = cache_read_add;
  
  replace_len_total = 0;

  
  for(i = 0; i < total_byte; i = i + W25Q128_BUFFER_LEN)
  {
    memset(data_buffer, 0, sizeof(data_buffer));
    //读出256字节数据 
    Flash_Read((unsigned char *)data_buffer, save_read_add, W25Q128_BUFFER_LEN);
    
    //判断读出的256个字节有没有需要更改的地方
    if((write_add + replace_len_total) >= save_read_add && (write_add + replace_len_total) <= (save_read_add + W25Q128_BUFFER_LEN - 1))
    {
      //如果尾地址也是在该256个字节中
      if((write_add + len - 1) >= save_read_add && (write_add + len - 1) <= (save_read_add + W25Q128_BUFFER_LEN - 1))
      {
        //计算需要替换的数据长度
        replace_len = (write_add + len - 1) - (write_add + replace_len_total) + 1;
      }
      else
      {
        //计算需要替换的数据长度
        replace_len = (save_read_add + W25Q128_BUFFER_LEN - 1) - (write_add + replace_len_total) + 1;
      }
      //替换数据
      memcpy((data_buffer + (write_add + replace_len_total - save_read_add)), (data + replace_len_total), replace_len);//替换数据
      
      replace_len_total = replace_len_total + replace_len;
    }

    //向缓存区写入数据
    Flash_Write_MorePage(data_buffer, cache_write_add, W25Q128_BUFFER_LEN);
      
    cache_write_add = cache_write_add + W25Q128_BUFFER_LEN;
    save_read_add   = save_read_add + W25Q128_BUFFER_LEN;
    
    //当缓存区的数据写满时，则将数据复制写入到原来的数据存储区中
    if(cache_write_add == cache_inf.sector_tail_add + 1)
    {
      //擦出原来sector中的数据
      save_write_inf = w25q128_add_to_block_sector(save_write_add);
      Flash_Erase_Sector(save_write_inf.block, save_write_inf.sector);
      
      for(j = 0; j < 4 * 1024; j = j + W25Q128_BUFFER_LEN)
      {
        memset(data_buffer, 0, sizeof(data_buffer));
        //读出缓存区中的数据
        Flash_Read((unsigned char *)data_buffer, cache_read_add, W25Q128_BUFFER_LEN);
        //向存储区复写入数据
        Flash_Write_MorePage(data_buffer, save_write_add, W25Q128_BUFFER_LEN);
        
        cache_read_add = cache_read_add + W25Q128_BUFFER_LEN;
        save_write_add = save_write_add + W25Q128_BUFFER_LEN;
      }
      
      Flash_Erase_Sector(cache_inf.block, cache_inf.sector);
      cache_read_add = cache_inf.sector_first_add;
      cache_write_add = cache_read_add;
    }
  }
  return 1;
}

///@brief   整块擦除
void FLASH_BulkErase()
{    
	Flash_Write_Enable();   //写使能
	Flash_Wait_Busy();
	FLASH_CS_ENABLE;        //片选拉低
	Flash_WriteOneByte(Flash_Chip_Erase_CMD);
	// 发送完指令后，cs必须拉高，否则指令不执行
	FLASH_CS_DISABLE;
	Flash_Wait_Busy();
  return ;
}

///@brief   读系列ID，固定16位，用于测试
uint16_t Read_w25q128_ID()
{
  uint16_t ID=0;

  FLASH_CS_ENABLE;//片选使能
  Flash_WriteOneByte(Flash_ReadID);
  Flash_WriteOneByte(0x00);
  Flash_WriteOneByte(0x00);
  Flash_WriteOneByte(0x00);
  
  ID |= Flash_ReadOneByte() << 8;
  ID |= Flash_ReadOneByte();
  
  FLASH_CS_DISABLE;//片选失能
  return ID;
}

///@brief   读唯一ID,固定7字节，用于测试
void Read_w25q128_only_ID(uint8_t *only_id)
{
  FLASH_CS_ENABLE;//片选使能
  Flash_WriteOneByte(Flash_ReadID_only);
  Flash_WriteOneByte(0x00);
  Flash_WriteOneByte(0x00);
  Flash_WriteOneByte(0x00);
  Flash_WriteOneByte(0x00);
  for(int i = 0; i < 8; i++)
  {
    only_id[i] = Flash_ReadOneByte();
  }
  FLASH_CS_DISABLE;//片选失能
}
#endif
