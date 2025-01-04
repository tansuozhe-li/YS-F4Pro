/*******************************************************************************
*  版权所有 （c） 2023， 清蓉深瞳科技有限公司（清华四川能源研究院）
*  文件名称：  DeepSensingUpgrade.c
*  内容摘要：  DS模块初始化
*  其他说明：
*  当前版本：  EIT-L_V1.0.0
*  作    者：  
*  完成日期：  2023年12月3日
*  
*  修改记录 1：
*        修改日期：2023年12月3日
*        修 改 人：
*        修改内容：创建工程文件
*  修改记录 2：
*******************************************************************************/
#include "DeepSensingUpdate.h"
#include "DeepSensing.h"
#include "onchip_flash.h"
#include "w25q256.h"

rt_sem_t update_sem_data = RT_NULL;
static UpFrameState_t UpFrameStateTable[UPDATE_FRAME_MAX_NUM];
static uint16_t request_upgrade_frame_index = 0;
static DSUpgradeStateParam_t DSUpgradeStateParam = {0};
DSUpgradeOverTime_t DSUpgradeOverTime;
uint8_t Recv_Heartbeat_flag = 0;


///@brief 根据升级通知报文，判断是否需要升级，并将结果返回到服务器
BOOL DS_OTAUpgradeInformDataProcess(uint8_t *buff)
{
    DSOTAUpgradeInform_t OnchipFileHead;
    DSOTAUpgradeInform_t *pack = (DSOTAUpgradeInform_t *)buff;
    
    AssertError(pack->Flag == OTA_UPGRADE_INFORM_FLAG, return FALSE, "update inform flag error, except: %s", OTA_UPGRADE_INFORM_FLAG);
    
    AssertError(pack->File_Frame_Num < UPDATE_FRAME_MAX_NUM, return FALSE, "File_Frame_Num error, except less: %d", UPDATE_FRAME_MAX_NUM);
    switch (pack->File_Type)
    {
    case OTA_UPGRADE_FILE_TYPE_APP: //app应用程序
      //比较升级包中的程序版本与正在运行的程序版本，是否需要升级
      AssertError(pack->Software_Version[0] == 'S' || pack->Software_Version[0] == 's', return FALSE, "version flag error");
      
      //读取片内Flash的文件头信息
      OnChipFlashReadData((uint8_t *)&OnchipFileHead, UPDATE_APP_PKG_HEAD_BEGIN, sizeof(OnchipFileHead));
      //判断文件头是否一样，如一样则不需要升级
      AssertError(memcmp(pack, &OnchipFileHead, sizeof(DSOTAUpgradeInform_t)) != 0, return FALSE, "Version is Error");
      break;
      
    case OTA_UPGRADE_FILE_TYPE_BOOT: //bootloader程序
      
      break;
    case OTA_UPGRADE_FILE_TYPE_CFG: //配置文件升级
      AssertError(pack->Software_Version[0] == 'C' || pack->Software_Version[0] == 'c', return FALSE, "version flag error");
      //读取片外Flash的文件头信息
      FlashID_Read((uint8_t *)&OnchipFileHead, FLASH_ID_Config_File_Head, 0, sizeof(OnchipFileHead));
      //判断文件头是否一样，如一样则不需要升级
      AssertError(memcmp(pack, &OnchipFileHead, sizeof(DSOTAUpgradeInform_t)) != 0, return FALSE, "Version is Error");
      break;
    case OTA_UPGRADE_FILE_TYPE_KEY: //密钥升级
      
      break;
    case OTA_UPGRADE_FILE_TYPE_CERT: //加密证书升级
      
      break;
    default:
      break;        
    }
    
    //将升级通知中的升级文件头部信息保存至缓存地址
    FlashID_Erase_Sector(FLASH_ID_OTA_FILE_CACHE_HEAD);
    FlashID_Write_MorePage(buff, FLASH_ID_OTA_FILE_CACHE_HEAD, 0, sizeof(DSOTAUpgradeInform_t));
    return TRUE;
}

///@brief 自定义数据响应报--OTA升级通知
BOOL DS_MsgProc_UpateInformDataResp(GW2015FrameHeader_t *pFrame)
{
    uint8_t inform_resp = 0;
    DSOTAUpgradeInform_t *pack = (DSOTAUpgradeInform_t *)pFrame->data;
    
    if (DS_OTAUpgradeInformDataProcess(pFrame->data)){
        inform_resp = 0xff;//设备可升级
        DS_SendGW2015DataPack(DS_FRAME_TYPE_21H_RESPONSE_DATA_REPORT, DS_PACK_TYPE_D1H_OTA_UPGRADE_INFORM, &inform_resp, 1);
        DSUpgradeStateParam.file_length  = pack->File_Length;
        DSUpgradeStateParam.file_frame_len = pack->File_Frame_Len;
        DSUpgradeStateParam.file_frame_num = pack->File_Frame_Num;
        LOG_D("File_Length: %d, File_Frame_Len: %d, File_Frame_Num: %d", pack->File_Length, pack->File_Frame_Len, pack->File_Frame_Num);
        request_upgrade_frame_index = 0;
        FlashID_Erase_Sector(FLASH_ID_OTA_FILE_CACHE_STATE);
        //擦除片外升级缓存空间
        int num = pack->File_Length / EXT_FLASH_SECTOR_SIZE;
        if (pack->File_Length % EXT_FLASH_SECTOR_SIZE != 0)
        {
           num += 1;
        }
        for (int i = 0; i < num; i++)
        {
            FlashID_Erase_Sector(FLASH_ID_OTA_FILE_CACHE_START + i);
        }
        memset(UpFrameStateTable, 0, sizeof(UpFrameStateTable));
        DSUpgradeStateParam.upgrade_state = UPDATE_STATE_INFORM;
        rt_sem_release(update_sem_data);
    }else{
        inform_resp = 0;//失败，设备不可升级
        DSUpgradeStateParam.upgrade_state = UPDATE_STATE_IDLE;
        memset(UpFrameStateTable, 0, sizeof(UpFrameStateTable));
        DS_SendGW2015DataPack(DS_FRAME_TYPE_21H_RESPONSE_DATA_REPORT, DS_PACK_TYPE_D1H_OTA_UPGRADE_INFORM, &inform_resp, 1); 
    } 
    return TRUE;
}

///@brief 将升级数据写入外部Flash文件缓存区
static BOOL DS_OTAUpgradeDataWriteToFlash(DSOTAUpgradeData_t *data)
{
    uint32_t offset = DSUpgradeStateParam.file_frame_len * data->File_Frame_Index;

    FlashID_Write_MorePage(data->File_Frame_Data, FLASH_ID_OTA_FILE_CACHE_START, offset, data->File_Frame_Length);
    return TRUE;
}

///@brief OTA数据响应报--OTA升级数据报
BOOL DS_MsgProc_UpdateFrameDataResp(GW2015FrameHeader_t *pFrame)
{
    DSOTAUpgradeData_t *pData = (DSOTAUpgradeData_t *)pFrame->data;
    
    //判断升级过程状态
    AssertReturnNoPrint(DSUpgradeStateParam.upgrade_state == UPDATE_STATE_TRANSMIT, return FALSE);
    //判断请求的升级数据与回复的是否是同一包
    AssertReturnNoPrint(pData->File_Frame_Index == request_upgrade_frame_index, return FALSE);
    
    UpFrameStateTable[pData->File_Frame_Index].upgrade_frame_index = pData->File_Frame_Index;
    UpFrameStateTable[pData->File_Frame_Index].upgrade_frame_lenth = pData->File_Frame_Length;
    
    //将升级数据写入外部Flash文件缓存区
    if (!DS_OTAUpgradeDataWriteToFlash(pData)){
        UpFrameStateTable[pData->File_Frame_Index].upgrade_frame_state = UPGRADE_DATA_ERR;
    }else{
        UpFrameStateTable[pData->File_Frame_Index].upgrade_frame_state = UPGRADE_DATA_OK;
        LOG_I("EITL Update Request Frame index : %d Success", pData->File_Frame_Index);
    }
    rt_sem_release(update_sem_data);
    return 0;
}

///@brief 发送升级数据报文请求
void DS_UpdateFrameDataRequest(uint16_t upgrade_frame_index)
{
    uint8_t index[2] = {0};
    index[0] = (uint8_t)(upgrade_frame_index & 0x00ff);
    index[1] = (uint8_t)(upgrade_frame_index >> 8);
    DS_SendGW2015DataPack(DS_FRAME_TYPE_21H_RESPONSE_DATA_REPORT, DS_PACK_TYPE_D2H_OTA_UPGRADE_DATA, index, 2);
}

///@brief 发送升级结束确认报文
void DS_UpdateFinishConfirmPack(uint8_t flag)
{
    DSOTAUpgradeInform_t DSOTAUpgradeFileHead;
    uint8_t pack_buff[20] = {0};
    DSOTAUpgradeFinish_t *pack = (DSOTAUpgradeFinish_t *)pack_buff;
    pack->Flag = flag;
    FlashID_Read((uint8_t *)&DSOTAUpgradeFileHead, FLASH_ID_OTA_FILE_CACHE_HEAD, 0, sizeof(DSOTAUpgradeFileHead));
    memcpy(pack->Software_Version, DSOTAUpgradeFileHead.Software_Version, sizeof(pack->Software_Version));
    memcpy(pack->Hardware_Version, DSOTAUpgradeFileHead.Hardware_Version, sizeof(pack->Hardware_Version));
    DS_SendGW2015DataPack(DS_FRAME_TYPE_21H_RESPONSE_DATA_REPORT, DS_PACK_TYPE_D3H_OTA_UPGRADE_FINISH, pack_buff, sizeof(DSOTAUpgradeFinish_t));
}

///@brief OTA数据响应报--OTA升级数据结束报
BOOL DS_MsgProc_UpdateFinishDataResp(GW2015FrameHeader_t *pFrame)
{
    DSUpgradeStateParam.upgrade_state = UPDATE_STATE_IDLE;
    FlashID_Erase_Sector(FLASH_ID_OTA_FILE_CACHE_STATE);
    FlashID_Erase_Sector(FLASH_ID_OTA_FILE_CACHE_HEAD);
    DS_UpdateFinishConfirmPack(0);
    return 0;
}

///@brief 跳转至app应用程序
typedef  void (*pFunction)(void);
void DS_JumpToApp()
{
    pFunction Jump_To_Application;
  
    __IO uint32_t StackAddr;
    __IO uint32_t ResetVector;
    __set_PRIMASK(1);
    //if (((*(__IO uint32_t *)UPDATE_APP_ADDR_BEGIN) & 0x2FFE0000 ) == 0x20000000)
    {
      StackAddr = *(__IO uint32_t*)UPDATE_APP_ADDR_BEGIN;
      ResetVector = *(__IO uint32_t *)(UPDATE_APP_ADDR_BEGIN + 4);
      //HAL_ADC_MspDeInit(&hadc1);
      __HAL_RCC_GPIOE_CLK_DISABLE();
      __HAL_RCC_GPIOC_CLK_DISABLE();
      __HAL_RCC_GPIOH_CLK_DISABLE();
      __HAL_RCC_GPIOA_CLK_DISABLE();
      __HAL_RCC_GPIOB_CLK_DISABLE();
      __HAL_RCC_GPIOD_CLK_DISABLE();
      /* 设置所有时钟到默认状态，使用HSI时钟 */
      HAL_RCC_DeInit();
      
      /* 关闭所有中断，清除所有中断挂起标志 */
      for (int i = 0; i < 8; i++)		//@5
      {
          NVIC->ICER[i]=0xFFFFFFFF;
          NVIC->ICPR[i]=0xFFFFFFFF;
      }
      SysTick->CTRL = 0;		
      SysTick->LOAD = 0;
      SysTick->VAL = 0;
      
      __set_PRIMASK(0);
      __set_MSP(StackAddr); 
     
      SCB->VTOR = UPDATE_APP_ADDR_BEGIN;
      Jump_To_Application = (pFunction)ResetVector;
      __set_CONTROL(0); /* 设置为特权级模式，使用MSP指针 */
      Jump_To_Application(); 
    }
}

///@brief 读取片外Flash保存的升级缓存状态，判断是否需要继续升级
BOOL DS_CheckFlashUpdateState()
{
    static DSOTAUpgradeInform_t OTAUpgradeFileHead; //片外缓存区，文件头信息
    static DSOTAUpgradeInform_t OTACfgUpgradeFileHead; //片外配置文件头信息
    static DSOTAUpgradeInform_t OnchipUpgradeFileHead; //片内程序文件头信息
    static UpFrameState_t UpFrameState[UPDATE_FRAME_MAX_NUM];
    
    memset(&OTAUpgradeFileHead, 0, sizeof(OTAUpgradeFileHead));
    memset(&OTACfgUpgradeFileHead, 0, sizeof(OTACfgUpgradeFileHead));
    memset(&OnchipUpgradeFileHead, 0, sizeof(OnchipUpgradeFileHead));
    memset(&UpFrameState, 0, sizeof(UpFrameState));
    memset(UpFrameStateTable, 0, sizeof(UpFrameStateTable));
    
    LOG_D("BootLoader 检查片外缓存状态");
    //读取片外Flash缓存区文件头信息
    FlashID_Read((uint8_t *)&OTAUpgradeFileHead, FLASH_ID_OTA_FILE_CACHE_HEAD, 0, sizeof(OTAUpgradeFileHead));
    AssertError(OTAUpgradeFileHead.Flag == OTA_UPGRADE_INFORM_FLAG, return FALSE, "Flag is Error");
    
    if (OTAUpgradeFileHead.File_Type == OTA_UPGRADE_FILE_TYPE_APP) { //app应用程序
        //读取片内Flash的文件头信息
        OnChipFlashReadData((uint8_t *)&OnchipUpgradeFileHead, UPDATE_APP_PKG_HEAD_BEGIN, sizeof(OnchipUpgradeFileHead));
        //判断文件头是否相同，如相同则不需要升级
        AssertError(memcmp(&OTAUpgradeFileHead, &OnchipUpgradeFileHead, sizeof(OTAUpgradeFileHead)) != 0, return FALSE, "App Version is Same");

    } else if (OTAUpgradeFileHead.File_Type == OTA_UPGRADE_FILE_TYPE_BOOT) {
        
    } else if (OTAUpgradeFileHead.File_Type == OTA_UPGRADE_FILE_TYPE_CFG) {
        AssertError(OTAUpgradeFileHead.Software_Version[0] == 'C' || OTAUpgradeFileHead.Software_Version[0] == 'c', return FALSE, "version flag error");
        //读取片外Flash保存的配置文件的文件头信息
        FlashID_Read((uint8_t *)&OTACfgUpgradeFileHead, FLASH_ID_Config_File_Head, 0, sizeof(OTACfgUpgradeFileHead));
        //判断文件头是否一样，如一样则不需要升级
        AssertError(memcmp(&OTAUpgradeFileHead, &OTACfgUpgradeFileHead, sizeof(OTAUpgradeFileHead)) != 0, return FALSE, "Cfg Version is Same");
        
    } else if (OTAUpgradeFileHead.File_Type == OTA_UPGRADE_FILE_TYPE_KEY) {
      
    } else if (OTAUpgradeFileHead.File_Type == OTA_UPGRADE_FILE_TYPE_CERT) {
      
    } else {
        LOG_E("不支持的文件类型");
        return FALSE;
    }
    
    //读取OTA升级状态
    FlashID_Read((uint8_t *)&UpFrameState, FLASH_ID_OTA_FILE_CACHE_STATE, 0, sizeof(UpFrameState));
    //判断上次升级过程是否成功
    for (int i = 0; i < OTAUpgradeFileHead.File_Frame_Num; i++) {
        if (UpFrameState[i].upgrade_frame_state == UPGRADE_DATA_OK) {
            UpFrameStateTable[i].upgrade_frame_state = UPGRADE_DATA_OK;
            UpFrameStateTable[i].upgrade_frame_index = UpFrameState[i].upgrade_frame_index;
            UpFrameStateTable[i].upgrade_frame_lenth = UpFrameState[i].upgrade_frame_lenth;
            continue;
        } else {
            UpFrameStateTable[i].upgrade_frame_state = UPGRADE_DATA_ERR;
            DSUpgradeStateParam.file_length = OTAUpgradeFileHead.File_Length;
            DSUpgradeStateParam.file_frame_num = OTAUpgradeFileHead.File_Frame_Num;
            DSUpgradeStateParam.file_frame_len = OTAUpgradeFileHead.File_Frame_Len;
            request_upgrade_frame_index = i;
            LOG_E("上一次升级未完成，当前进度 ： %d / %d 继续升级", request_upgrade_frame_index, OTAUpgradeFileHead.File_Frame_Num);
            return TRUE; //需继续升级
        }
    }
    
    LOG_E("不需要继续升级");
    return FALSE;//不需要继续升级
}

///@brief 校验片外的升级缓存文件
static BOOL DS_UpdateFlashFileCheck(DSOTAUpgradeInform_t *file_head)
{
    uint16_t crc_ret = 0xFFFF;
    uint32_t read_len = 0;
    uint32_t offset = 0;
    int feeddog_num = 0;
    static uint8_t flash_read_buff[512];

    AssertError(file_head->Flag == OTA_UPGRADE_INFORM_FLAG, return FALSE, "Flag is Error");
    
    uint32_t len = file_head->File_Length;
    AssertError(len < 512*512, return FALSE, "File_Length is Error");
    //计算片外缓存文件的校验码
    while(len)
    {
      if(len > sizeof(flash_read_buff))
        read_len = sizeof(flash_read_buff);
      else
        read_len = len;
      
      memset(flash_read_buff,0,sizeof(flash_read_buff));
      FlashID_Read(flash_read_buff, FLASH_ID_OTA_FILE_CACHE_START, offset, read_len);
      RTU_CRC_update(&crc_ret, flash_read_buff, read_len);
      len -= read_len;
      offset += read_len;
      feeddog_num = feeddog_num + 1;
      if(feeddog_num % 20 == 0)
      {
        mdelay(5);
      }
    }
    AssertError(file_head->File_CRC == UnShortToHighLevelTransfer(crc_ret), return FALSE, "file_crc is Error");
    
    return TRUE;
}

///@brief 校验片内升级文件
static BOOL DS_UpdateOnchipFileCheck()
{
    static DSOTAUpgradeInform_t updata_onchip_file_head;
    memset((uint8_t *)&updata_onchip_file_head,0,sizeof(DSOTAUpgradeInform_t));
    OnChipFlashReadData((uint8_t *)&updata_onchip_file_head, UPDATE_APP_PKG_HEAD_BEGIN, sizeof(DSOTAUpgradeInform_t));
    AssertError(updata_onchip_file_head.Flag == OTA_UPGRADE_INFORM_FLAG, return FALSE, "Flag is Error");
    static uint8_t onchip_read_buff[512];
    uint32_t read_len = 0;
    uint16_t crc_ret = 0xFFFF;
    int feeddog_num = 0;
    u_int32_t write_addr = UPDATE_APP_ADDR_BEGIN;
    uint32_t len = updata_onchip_file_head.File_Length;
    AssertError(len < 512*512, return FALSE, "File_Length is Error");
    //文件校验
    while(len) 
    {
        if(len > sizeof(onchip_read_buff))
            read_len = sizeof(onchip_read_buff);
        else
            read_len = len;
  
        memset(onchip_read_buff,0,sizeof(onchip_read_buff));
        
        OnChipFlashReadData(onchip_read_buff, write_addr, read_len);
        RTU_CRC_update(&crc_ret, onchip_read_buff, read_len);
        
        len -= read_len;
        write_addr += read_len; 
        feeddog_num = feeddog_num + 1;
        if(feeddog_num % 20 == 0)
        {
          mdelay(5);
        }
    }
    
    //文件CRC校验
    AssertError(updata_onchip_file_head.File_CRC == UnShortToHighLevelTransfer(crc_ret), return FALSE, "file_crc is Error");
    return TRUE;
}

///@brief 擦除应用程序在片内的存储空间, 如果是配置文件，密钥，证书则擦除外部Flash
static BOOL DS_UpdateProgramOnchipFlashErase(DSOTAUpgradeInform_t *file_head)
{
    //uint16_t sector_num = 0;
    uint32_t file_total_len = file_head->File_Length;

    if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_APP) {
        if (OnChipFlashEraseSector(ADDR_FLASH_SECTOR_5, 1) != 1) {
            return FALSE;
        }
        //sector_num = file_total_len / STM_SECTOR_SIZE;
        //if (file_total_len % STM_SECTOR_SIZE != 0) {
            //sector_num++;
        //}
        if (OnChipFlashEraseSector(ADDR_FLASH_SECTOR_6, 1) != 1) {
            return FALSE;
        }
        if (OnChipFlashEraseSector(ADDR_FLASH_SECTOR_7, 1) != 1) {
            return FALSE;
        }
        if (OnChipFlashEraseSector(ADDR_FLASH_SECTOR_8, 1) != 1) {
            return FALSE;
        }
        if (OnChipFlashEraseSector(ADDR_FLASH_SECTOR_9, 1) != 1) {
            return FALSE;
        }
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_BOOT) {
        
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_CFG) {
        FlashID_Erase_Sector(FLASH_ID_Config_File_Head);
        FlashID_Erase_Sector(FLASH_ID_Config_File_Param);
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_KEY) {
      
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_CERT) {
      
    } else {
        LOG_E("不支持的文件类型");
        return FALSE;
    }
    return TRUE;
}

///@brief 将片外应用程序复制到片内
static BOOL DS_UpdateProgramCopyFileToOnchip(DSOTAUpgradeInform_t *file_head)
{
    uint32_t len = file_head->File_Length;
    AssertError(len < 512*512, return FALSE, "File_Length is Error");
    uint32_t read_len = 0;
    uint32_t offset = 0;
    u_int32_t write_addr = UPDATE_APP_ADDR_BEGIN;
    static uint8_t flash_read_buff[512];
    
    if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_APP) 
    {
        //复制文件头
        if (OnChipFlashWriteData((uint8_t *)file_head, UPDATE_APP_PKG_HEAD_BEGIN, sizeof(*file_head)) != 1) {
            return FALSE;
        }
        LOG_D("开始写入文件,地址write_addr = %d",write_addr);
        while(len) 
        {
            if(len > sizeof(flash_read_buff))
                read_len = sizeof(flash_read_buff);
            else
                read_len = len;
  
            memset(flash_read_buff,0,sizeof(flash_read_buff));
            FlashID_Read(flash_read_buff, FLASH_ID_OTA_FILE_CACHE_START, offset, read_len);
            
            if (OnChipFlashWriteData(flash_read_buff, write_addr, read_len) != 1) {
                LOG_D("数据保存到片内 失败");
                return FALSE;
            }
            len -= read_len;
            offset += read_len;
            write_addr += read_len; 
        }
        LOG_D("程序写入成功");
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_BOOT) {
        
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_CFG) {
        //复制文件头
        FlashID_Write_MorePage((uint8_t *)file_head, FLASH_ID_Config_File_Head, 0, sizeof(*file_head));
        while(len) 
        {
            if(len > sizeof(flash_read_buff))
                read_len = sizeof(flash_read_buff);
            else
                read_len = len;
  
            memset(flash_read_buff,0,sizeof(flash_read_buff));
            FlashID_Read(flash_read_buff, FLASH_ID_OTA_FILE_CACHE_START, offset, read_len);
            
            FlashID_Write_MorePage(flash_read_buff, FLASH_ID_Config_File_Param, offset, read_len);
            len -= read_len;
            offset += read_len;
            write_addr += read_len; 
        }
        LOG_D("配置文件写入成功");
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_KEY) {
      
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_CERT) {
      
    } else {
        LOG_E("不支持的文件类型");
        return FALSE;
    }
    
    return TRUE;
}

///@brief 校验已写入片内rom的应用程序包  通过读出片外flash内容与片内内容完全比较的方式校验
static BOOL DSUpdateProgramAppVerify(DSOTAUpgradeInform_t *file_head)
{
    uint16_t crc_ret = 0xffff;
    uint32_t len = file_head->File_Length;
    AssertError(len < 512*512, return FALSE, "File_Length is Error");
    uint32_t read_len = 0;
    uint32_t offset = 0;
    u_int32_t write_addr = UPDATE_APP_ADDR_BEGIN;
    static uint8_t flash_read_buff[512];
    static uint8_t flash_read_buff2[512];
    static DSOTAUpgradeInform_t updata_file_head;
    if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_APP) //APP 
    {
        //头校验
        memset((uint8_t *)&updata_file_head,0,sizeof(updata_file_head));
        OnChipFlashReadData((uint8_t *)&updata_file_head, UPDATE_APP_PKG_HEAD_BEGIN, sizeof(DSOTAUpgradeInform_t));
        if(memcmp((uint8_t *)&updata_file_head, (u_int8_t *)file_head, sizeof(DSOTAUpgradeInform_t)) != 0)
            return FALSE;
        
        //文件校验
        while(len) 
        {
            if(len > sizeof(flash_read_buff))
                read_len = sizeof(flash_read_buff);
            else
                read_len = len;
  
            memset(flash_read_buff,0,sizeof(flash_read_buff));
            FlashID_Read(flash_read_buff, FLASH_ID_OTA_FILE_CACHE_START, offset, read_len);
            
            
            memset(flash_read_buff2,0,sizeof(flash_read_buff2));
            OnChipFlashReadData(flash_read_buff2, write_addr, read_len);
            RTU_CRC_update(&crc_ret, flash_read_buff2, read_len);
            
            if(memcmp(flash_read_buff, (u_int8_t *)flash_read_buff2, read_len) != 0)
                return FALSE;
            
            len -= read_len;
            offset += read_len;
            write_addr += read_len; 
        }
        
        //文件CRC校验
        AssertError(file_head->File_CRC == UnShortToHighLevelTransfer(crc_ret), return FALSE, "file_crc is Error");
        
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_BOOT) {
        
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_CFG) {
        //头校验
        memset((uint8_t *)&updata_file_head,0,sizeof(updata_file_head));
        FlashID_Read((uint8_t *)&updata_file_head, FLASH_ID_Config_File_Head, 0, sizeof(DSOTAUpgradeInform_t));
        if(memcmp((uint8_t *)&updata_file_head, (u_int8_t *)file_head, sizeof(DSOTAUpgradeInform_t)) != 0)
            return FALSE;

    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_KEY) {
      
    } else if (file_head->File_Type == OTA_UPGRADE_FILE_TYPE_CERT) {
      
    } else {
        LOG_E("不支持的文件类型");
        return FALSE;
    }
    return TRUE;
}



///@brief Bootloader启动时检查跳转或复位
BOOL DS_CheckAppFile()
{
    static DSOTAUpgradeInform_t EXT_UpgradeFileHead;
    memset(&EXT_UpgradeFileHead, 0, sizeof(DSOTAUpgradeInform_t));

    //读片外缓存区保存的文件头
    FlashID_Read((uint8_t *)&EXT_UpgradeFileHead, FLASH_ID_OTA_FILE_CACHE_HEAD, 0, sizeof(EXT_UpgradeFileHead));
    
    //校验片外flash的文件
    if (!DS_UpdateFlashFileCheck(&EXT_UpgradeFileHead)) 
        return FALSE; //设备复位   
    
    //将片外缓存文件与当前文件进行对比
    if (DSUpdateProgramAppVerify(&EXT_UpgradeFileHead)) 
        return TRUE; //加载应用程序
    
    //擦除应用程序在片内的存储空间
    if (!DS_UpdateProgramOnchipFlashErase(&EXT_UpgradeFileHead))
        return FALSE; //设备复位
    
    //将片外flash缓存的升级程序，复制到片内
    if (!DS_UpdateProgramCopyFileToOnchip(&EXT_UpgradeFileHead))
        return FALSE; //设备复位
    
    //将片外与片内的程序文件进行对比
    if (!DSUpdateProgramAppVerify(&EXT_UpgradeFileHead))
        return FALSE; //设备复位
    
    return TRUE; //加载应用程序
}

///@brief 升级结束，校验升级文件，将升级文件从片外复制到片内，发送结束报文，加载应用程序
BOOL DS_UpdateFinishCheckFileState()
{
    uint8_t finish_flag = 0;
    static DSOTAUpgradeInform_t UpgradeFileHead;
    memset(&UpgradeFileHead, 0, sizeof(DSOTAUpgradeInform_t));
    
    //读片外保存的文件头，校验片外Flash保存的程序文件是否正确
    FlashID_Read((uint8_t *)&UpgradeFileHead, FLASH_ID_OTA_FILE_CACHE_HEAD, 0, sizeof(DSOTAUpgradeInform_t));
    //校验片外flash的文件
    if (DS_UpdateFlashFileCheck(&UpgradeFileHead)) {
        finish_flag = 0xff;
        DS_UpdateFinishConfirmPack(finish_flag);
        LOG_I("升级成功");
    }else{
        finish_flag = 0;
        //DS_UpdateFinishConfirmPack(finish_flag);
        LOG_I("升级失败");
        return FALSE; //设备复位
    }
    
    //擦除应用程序在片内的存储空间
    if (!DS_UpdateProgramOnchipFlashErase(&UpgradeFileHead))
        return FALSE; //设备复位
    
    //将片外flash缓存的升级程序，复制到片内
    if (!DS_UpdateProgramCopyFileToOnchip(&UpgradeFileHead))
        return FALSE; //设备复位
    
    //将片外与片内的程序文件进行对比
    if (!DSUpdateProgramAppVerify(&UpgradeFileHead))
        return FALSE; //设备复位
    
    
    return TRUE; //加载应用程序 
}

///@brief 校验片内程序，成功加载，失败跳转
void OnchipFileCheckToLoad()
{
    if (DS_UpdateOnchipFileCheck()) { //升级失败，校验片内程序
        LOG_E("加载应用程序");
        DS_JumpToApp(); //校验成功，跳转片内程序
    } else {
        LOG_E("设备重启");
        NVIC_SystemReset(); //return FALSE 校验失败，设备复位
    }
}

///@brief Bootloader超时加载
void DS_OverTimeLoad()
{
    if ((DSUpgradeOverTime.start_waiting_time > 80) && (DSUpgradeStateParam.upgrade_state == UPDATE_STATE_DISCONNECT)) {
        DSUpgradeOverTime.start_waiting_time = 0;
        DS_CheckAppFile();
        OnchipFileCheckToLoad();
    }
    if ((DSUpgradeOverTime.start_waiting_time > 80) && (DSUpgradeStateParam.upgrade_state == UPDATE_STATE_IDLE)) {
        DSUpgradeOverTime.start_waiting_time = 0;
        DS_CheckAppFile();
        OnchipFileCheckToLoad();
    }
    if ((DSUpgradeOverTime.start_waiting_time > 100) && (DSUpgradeStateParam.upgrade_state == UPDATE_STATE_INFORM)) {
        DSUpgradeOverTime.start_waiting_time = 0;
        DS_CheckAppFile();
        OnchipFileCheckToLoad();
    }
    
    if (DSUpgradeOverTime.upgrade_failed_time > 100) {
        DSUpgradeOverTime.upgrade_failed_time = 0;
        DS_CheckAppFile();
        OnchipFileCheckToLoad();
    }
}

///@brief 在主线程中循环判断OTA升级功能是否需要开启
void DS_UpdateTask()
{   
    int load_delay = 0;
    uint8_t update_state = 0;
    lte_device_info_t *device = (lte_device_info_t *)g_main_lte_handle; 
    //创建一个信号量
    update_sem_data = rt_sem_create("sem update data", 0u, RT_IPC_FLAG_FIFO);
    AssertError(update_sem_data != RT_NULL, return, "create update_sem_data:%s failed", "update_sem_data");
    
    //未连接到升级端
    DSUpgradeStateParam.upgrade_state = UPDATE_STATE_DISCONNECT; 
    while(1)
    {
        mdelay(10);
        request_upgrade_frame_index = 0;
        //等待拨号成功
        AssertContinueNoPrint(device->stlte_chn_table.stchn_table[LTE1_CHN_DS].connected_stats == LTE_NET_CONNECTED_SUCCESS, {});
        //连接到升级端，但无需升级,空闲状态
        if (DSUpgradeStateParam.upgrade_state == UPDATE_STATE_DISCONNECT) {
            DSUpgradeStateParam.upgrade_state = UPDATE_STATE_IDLE;
            LOG_D("BootLoader 拨号成功");
            
        }
        //等待心跳包成功接收  
        AssertContinueNoPrint(Recv_Heartbeat_flag == UPGRADE_DATA_OK, {});
        
        //建立通信后，读取保存到片外Flash中的升级状态，判断是否需要继续升级
        if ((load_delay == 0) && DS_CheckFlashUpdateState()){
            DSUpgradeStateParam.upgrade_state = UPDATE_STATE_INFORM;
        }
        load_delay = 1;
                
        //等待升级通知
        AssertContinueNoPrint(DSUpgradeStateParam.upgrade_state == UPDATE_STATE_INFORM, {});
        AssertContinueNoPrint(DSUpgradeStateParam.file_frame_num < UPDATE_FRAME_MAX_NUM, {});
        //接受到升级通知，进入升级传输过程
        DSUpgradeStateParam.upgrade_state = UPDATE_STATE_TRANSMIT;
        
        int delay_cnt = 0;
        for (; request_upgrade_frame_index < DSUpgradeStateParam.file_frame_num;) {
            if (DSUpgradeStateParam.upgrade_state == UPDATE_STATE_INFORM) {
                request_upgrade_frame_index = 0;
                memset(UpFrameStateTable, 0, sizeof(UpFrameStateTable));
                LOG_D("recv new OTA inform");
                delay_cnt = 0;
                DSUpgradeStateParam.upgrade_state = UPDATE_STATE_TRANSMIT;
                continue;
            }
            rt_sem_take(update_sem_data,5000 + delay_cnt*1000);
            if (UpFrameStateTable[request_upgrade_frame_index].upgrade_frame_state == UPGRADE_DATA_OK) {
                FlashID_Erase_Sector(FLASH_ID_OTA_FILE_CACHE_STATE);
                FlashID_Write_MorePage((uint8_t *)&UpFrameStateTable, FLASH_ID_OTA_FILE_CACHE_STATE, 0, sizeof(UpFrameState_t) * (request_upgrade_frame_index + 1));
                request_upgrade_frame_index++;
                delay_cnt = 0;
                DSUpgradeOverTime.upgrade_failed_time = 0;
                DSUpgradeOverTime.start_waiting_time = 0;
                rt_sem_release(update_sem_data);
            } else {
                delay_cnt++;
                LOG_I("EITL Update Request Frame index : %d / %d", request_upgrade_frame_index, DSUpgradeStateParam.file_frame_num);
                DS_UpdateFrameDataRequest(request_upgrade_frame_index);
            }
            
            if (delay_cnt > 5) {
                LOG_E("升级失败，当前进度 ：%d / %d", request_upgrade_frame_index, DSUpgradeStateParam.file_frame_num);
                FlashID_Erase_Sector(FLASH_ID_OTA_FILE_CACHE_STATE);
                FlashID_Write_MorePage((uint8_t *)&UpFrameStateTable, FLASH_ID_OTA_FILE_CACHE_STATE, 0, sizeof(UpFrameState_t) * (request_upgrade_frame_index + 1));
                delay_cnt = 0;
                break;
            }
            
            if (request_upgrade_frame_index == DSUpgradeStateParam.file_frame_num) {
                LOG_E("升级成功，总包数 ：%d", DSUpgradeStateParam.file_frame_num);
                update_state = UPDATE_STATE_FINISH;
                FlashID_Erase_Sector(FLASH_ID_OTA_FILE_CACHE_STATE);
                FlashID_Write_MorePage((uint8_t *)&UpFrameStateTable, FLASH_ID_OTA_FILE_CACHE_STATE, 0, sizeof(UpFrameState_t) * (request_upgrade_frame_index + 1));
                break;
            }
        }
        
        //升级结束，校验应用程序，发送升级结束报文，并加载应用程序
        DSUpgradeStateParam.upgrade_state = UPDATE_STATE_FINISH;
        if ((update_state == UPDATE_STATE_FINISH) && DS_UpdateFinishCheckFileState()) {
            OnchipFileCheckToLoad();
        } else {//升级失败校验加载片内程序
            OnchipFileCheckToLoad();
        }
    }      
}




