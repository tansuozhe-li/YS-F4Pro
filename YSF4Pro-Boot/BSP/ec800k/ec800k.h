#ifndef _EC800K_ATCMD_H
#define _EC800K_ATCMD_H

#include "ec800k_atcmd.h"

/* 设备管脚定义 */
/* 开机管脚 */
#define LTE1_PWRKEY_HIGH HAL_GPIO_WritePin(LTE1_POWER_EN_GPIO_Port, LTE1_POWER_EN_Pin, GPIO_PIN_SET)
#define LTE1_PWRKEY_LOW  HAL_GPIO_WritePin(LTE1_POWER_EN_GPIO_Port, LTE1_POWER_EN_Pin, GPIO_PIN_RESET)
#define LTE1_EN_HIGH     HAL_GPIO_WritePin(LTE1_EN_GPIO_Port, LTE1_EN_Pin, GPIO_PIN_SET)
#define LTE1_EN_LOW      HAL_GPIO_WritePin(LTE1_EN_GPIO_Port, LTE1_EN_Pin, GPIO_PIN_RESET)

//#define LTE2_PWRKEY_HIGH HAL_GPIO_WritePin(LTE2_POWER_EN_GPIO_Port, LTE2_POWER_EN_Pin, GPIO_PIN_SET)
//#define LTE2_PWRKEY_LOW  HAL_GPIO_WritePin(LTE2_POWER_EN_GPIO_Port, LTE2_POWER_EN_Pin, GPIO_PIN_RESET)
//#define LTE2_EN_HIGH     HAL_GPIO_WritePin(LTE2_EN_GPIO_Port, LTE2_EN_Pin, GPIO_PIN_SET)
//#define LTE2_EN_LOW      HAL_GPIO_WritePin(LTE2_EN_GPIO_Port, LTE2_EN_Pin, GPIO_PIN_RESET)

/* 在开机状态下拉低 PWRKEY 至少 650 ms 后释放，模块将执行关机流程 */
//#define LTE1_PWRKEY_ON() do{LTE1_PWRKEY_LOW; mdelay(10);LTE1_PWRKEY_HIGH; mdelay(800);LTE1_PWRKEY_LOW;}while(0)
//#define LTE2_PWRKEY_ON() do{LTE2_PWRKEY_LOW; mdelay(10);LTE2_PWRKEY_HIGH; mdelay(800);LTE2_PWRKEY_LOW;}while(0)
#define LTE1_ENABLE() do{LTE1_EN_HIGH;mdelay(500);}while(0)
//#define LTE2_ENABLE() do{LTE2_EN_HIGH;mdelay(500);}while(0)
#define LTE1_PWRKEY_ON() do{LTE1_PWRKEY_LOW; mdelay(800);}while(0)
#define LTE1_PWRKEY_OFF() do{LTE1_PWRKEY_HIGH; mdelay(800);}while(0)
//#define LTE2_PWRKEY_ON() do{LTE2_PWRKEY_LOW; mdelay(800);}while(0)
//#define LTE2_PWRKEY_OFF() do{LTE2_PWRKEY_HIGH; mdelay(800);}while(0)
/* 复位管脚 */
//#define LTE1_REST_HIGH  HAL_GPIO_WritePin(LTE1_RESET_CTR_GPIO_Port, LTE1_RESET_CTR_Pin, GPIO_PIN_SET) 
//#define LTE1_REST_LOW   HAL_GPIO_WritePin(LTE1_RESET_CTR_GPIO_Port, LTE1_RESET_CTR_Pin, GPIO_PIN_RESET) 

//#define LTE2_REST_HIGH  HAL_GPIO_WritePin(LTE2_RESET_CTR_GPIO_Port, LTE2_RESET_CTR_Pin, GPIO_PIN_SET) 
//#define LTE2_REST_LOW   HAL_GPIO_WritePin(LTE2_RESET_CTR_GPIO_Port, LTE2_RESET_CTR_Pin, GPIO_PIN_RESET) 

/* 拉低 RESET_N 至少 300 ms 后释放可令模块复位 */
//#define LTE1_REST()   do{LTE1_REST_LOW;mdelay(10);LTE1_REST_HIGH;mdelay(400);LTE1_REST_LOW;}while(0)
//#define LTE2_REST()   do{LTE2_REST_LOW;mdelay(10);LTE2_REST_HIGH;mdelay(400);LTE2_REST_LOW;}while(0)

/* 休眠管脚 */
#define LTE1_SLEEP_HIGH  HAL_GPIO_WritePin(LTE1_DTR_GPIO_Port, LTE1_DTR_Pin, GPIO_PIN_SET) 
#define LTE1_SLEEP_LOW   HAL_GPIO_WritePin(LTE1_DTR_GPIO_Port, LTE1_DTR_Pin, GPIO_PIN_RESET) 

#define LTE1_NOSLEEP()   do{LTE1_SLEEP_LOW;mdelay(100);}while(0)
#define LTE1_SLEEP()     do{LTE1_SLEEP_HIGH;mdelay(100);}while(0)

//#define LTE2_SLEEP_HIGH  HAL_GPIO_WritePin(LTE2_DTR_GPIO_Port, LTE2_DTR_Pin, GPIO_PIN_SET) 
//#define LTE2_SLEEP_LOW   HAL_GPIO_WritePin(LTE2_DTR_GPIO_Port, LTE2_DTR_Pin, GPIO_PIN_RESET) 
//
//#define LTE2_NOSLEEP()   do{LTE2_SLEEP_LOW;mdelay(100);}while(0)
//#define LTE2_SLEEP()     do{LTE2_SLEEP_HIGH;mdelay(100);}while(0)

/* 模块定义 */
typedef enum{
    LTE_DEVICE_MAIN_INDEX = 0,
    LTE_DEVICE_SUB_INDEX = 1,
}lte_device_chn_e;

/* 通道号定义 */
typedef enum{
    LTE1_CHN_DS = 0,
    LTE1_CHN_CORS = 1,
}lte1_device_chn_e;

///@brief 获取lte模块具体通道的连接状态
u_int8_t get_lte_chn_status(lte_device_handle lte_device, u_int8_t chn);
///@brief 手动更新lte模块具体通道的连接状态
void update_lte_chn_status(lte_device_handle lte_device, u_int8_t chn, u_int8_t status);
///@brief 发起Lte模块x通道的链接
BOOL reconnection_lte_chn(lte_device_handle lte_device, u_int8_t chn);
BOOL lte_chn_dial(lte_device_handle lte_dev);
///@brief 通过LTE发送设备串口数据
void lte_uart_send_pack(lte_device_handle lte_device, u_int8_t chnn, void *data, int len);
void lte_device_main_init();
void lte_device_sub_init();
#endif 










