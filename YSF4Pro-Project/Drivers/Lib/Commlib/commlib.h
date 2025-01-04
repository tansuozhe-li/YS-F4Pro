#ifndef _COMMON_LIB_H_
#define _COMMON_LIB_H_

#include "comm_types.h"

///@brief 计算CRC
unsigned short RTU_CRC( unsigned char * puchMsg, unsigned short usDataLen);
///@brief 累加计算CRC
void RTU_CRC_update(u_int16_t *crc16, unsigned char * puchMsg, u_int16_t usDataLen);
///@brief 字符串转整数
int StrToInt(char *str);
///@brief 将uint16_t型的数据高低字节调整
u_int16_t UnShortToHighLevelTransfer(u_int16_t Num);
///@brief 将uint32_t型的数据高低字节调整
u_int32_t UnintToHighLevelTransfer(u_int32_t Num);
///@brief 将ASCII码转换成HEX
u_int8_t ASCII_To_Hex(u_int8_t number);
///@brief 去掉最大值与最小值求平均值 
u_int16_t DataRankCalculateAverage(u_int16_t *data, u_int16_t datalen);
///@brief float型数据 去掉最大值与最小值求平均值 
float FloatDataRankCalculateAverage(float *data, u_int16_t datalen);
///@brief float型数据，排序求中位数的值
float FloatDataCalculateMedian(float *data, u_int16_t datalen);
///@brief 累加和校验算法（CheckSum算法）
u_int8_t CS_CheckSum(u_int8_t *buf, u_int8_t len);
#endif