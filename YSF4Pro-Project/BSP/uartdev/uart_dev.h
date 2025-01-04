#ifndef UART_DEV_H
#define UART_DEV_H

#include "comm_types.h"
#include "string.h"
#include "assertions.h"
#include "ring_buff.h"
#include "usart.h"
#include <rtthread.h>

typedef u_int32_t device_handle;
extern device_handle g_console_uart_handle;
extern device_handle g_mcu_lte1_uart_handle;
extern device_handle g_mcu_lte2_uart_handle;
extern device_handle g_rs485_uart_handle;
extern device_handle g_RS232_uart_handle;

/* UART 驱动数据结构，对应一个uart设备 */
typedef struct _uart_device_info{
  BOOL               init_status;               
  u_int8_t           uart_index;             
  u_int32_t          uart_baudrate;    
  RingQueue          uart_ring_queue;  
  rt_sem_t           ser_mutex;
  rt_sem_t           port_mutex;
  rt_sem_t           read_mutex;
  rt_sem_t           sem_recv_data;
  u_int8_t           inbuff[1500];
  device_handle      uartdev_handle;
}uart_device_info_t;
extern uart_device_info_t st_uart_devices[];

/* 与硬件对用的串口配置 */
typedef enum{ 
  CONSOLE_UART_INDEX        = 1, /* usart1 */
  MCU_LTE1_UART_INDEX       = 2, /* usart2 */
  RS485_UART_INDEX          = 3, /* usart3 */
  MCU_LTE2_UART_INDEX       = 4, /* usart4 */
  RT232_UART_INDEX          = 5, /* uart5 */
}uartIndex_e;

/* 串口波特率配置 */
typedef enum{ 
  CONSOLE_UART_BAUDRATE    = 115200,
  MCU_LTE1_UART_BAUDRATE   = 115200,
  MCU_LTE2_UART_BAUDRATE   = 115200,
  RS485_UART_BAUDRATE      = 115200,
  RS232_UART_BAUDRATE      = 115200,
}uartBaudate_e;

void Ctrl_UART_StopMode_WakeUp(UART_HandleTypeDef *huart,BOOL EnableNotDisable);

void HAL_UART_Error_ORE_Handle(UART_HandleTypeDef *huart);

device_handle uart_dev_init(u_int8_t uart_index, u_int32_t uart_baudrate);

void  uart_sendstr(device_handle device,char *str);

char uart_dev_in_char(device_handle device);

int ring_queue_dev_char_present(RingQueue *ring_queue);

char ring_queue_dev_in_char(RingQueue *ring_queue);

int uart_dev_char_present(device_handle device);

void uart_dev_write(device_handle device, void *data, int len);

void uart_enable_irq(u_int8_t uart_index);

void uart_disable_irq(u_int8_t uart_index);

int clean_data_buff(device_handle device);

/* 调试串口初始化 */
void init_console_uart();

/* RTK串口初始化 */
void init_RS485_uart();

/* LTE1串口初始化 */
void init_mcu_lte1_uart();

/* LTE2串口初始化 */
void init_mcu_lte2_uart();

/* tmp串口初始化 */
void init_tmp_uart();

/* RTK串口初始化 */
void init_RS232_uart();

#endif






